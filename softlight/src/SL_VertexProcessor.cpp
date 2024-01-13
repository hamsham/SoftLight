
#include "lightsky/setup/Types.h"

#include "lightsky/utils/Sort.hpp" // utils::sort_radix

#include "softlight/SL_Context.hpp"
#include "softlight/SL_LineRasterizer.hpp"
#include "softlight/SL_PointRasterizer.hpp"
#include "softlight/SL_Shader.hpp" // SL_Shader
#include "softlight/SL_TriRasterizer.hpp"
#include "softlight/SL_VertexProcessor.hpp"
#include "softlight/SL_ViewportState.hpp"

namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace
{



/*-------------------------------------
 * Exponential CPU-yielding to reduce spinlock overhead
-------------------------------------*/
template <typename IntegralType>
inline LS_INLINE IntegralType _sl_cpu_yield_exponential(IntegralType yieldCount) noexcept
{
    static_assert(ls::setup::IsIntegral<IntegralType>::value, "Need an integer type to count exponential yielding.");
    constexpr IntegralType maxIters = 2;

    switch (yieldCount)
    {
        case 2:
            ls::setup::cpu_yield();
            ls::setup::cpu_yield();
            ls::setup::cpu_yield();
            ls::setup::cpu_yield();

        case 1:
            ls::setup::cpu_yield();
            ls::setup::cpu_yield();

        default:
            ls::setup::cpu_yield();
    }

    return ls::math::min<IntegralType>(yieldCount+1, maxIters);
}



/*-------------------------------------
 * Exponential CPU-yielding to reduce spinlock overhead
-------------------------------------*/
template <typename WaitCondition>
inline LS_INLINE void _sl_cpu_yield_loop(const WaitCondition& cond) noexcept
{
    uint_fast32_t yieldCount = 0;

    while (cond())
    {
        yieldCount = _sl_cpu_yield_exponential<uint_fast32_t>(yieldCount);
    }
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_VertexProcessor Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Execute the rasterizer
-------------------------------------*/
template <typename RasterizerType>
void SL_VertexProcessor::flush_rasterizer() noexcept
{
    static_assert(ls::setup::IsBaseOf<SL_FragmentProcessor, RasterizerType>::value, "Template parameter 'RasterizerType' must derive from SL_FragmentProcessor.");

    // Allow the other threads to know when they're ready for processing
    const int_fast64_t    numThreads   = (int_fast64_t)mNumThreads;
    const int_fast64_t    syncPoint1   = -numThreads - 1;
    const int_fast64_t    tileId       = active_frag_processors().count.fetch_add(1ll, std::memory_order_acq_rel);
    const SL_FragmentBin* pBins        = active_frag_bins();
    uint_fast64_t         maxElements;
    int_fast64_t          syncPoint2;

    // Sort the bins based on their depth.
    if (LS_UNLIKELY(tileId == numThreads-1u))
    {
        maxElements = math::min<uint_fast64_t>(active_num_bins_used().count.load(std::memory_order_consume), SL_SHADER_MAX_BINNED_PRIMS);

        // Try to perform depth sorting once, and only once, per opaque draw
        // call to reduce depth-buffer access during rasterization. Sorting
        // primitives multiple times here in the vertex processor will
        // increase latency before invoking the fragment processor.
        const bool shouldDepthSort = ls::setup::IsSame<RasterizerType, SL_TriRasterizer>::value && (active_buffer_index() == next_buffer_index());
        const bool canDepthSort = shouldDepthSort && (maxElements < SL_SHADER_MAX_BINNED_PRIMS);

        // Blended fragments get sorted by their primitive index for
        // consistency.
        if (LS_UNLIKELY(mShader->pipelineState.blend_mode() != SL_BLEND_OFF))
        {
            SL_BinCounter<uint32_t>* const pActiveBinIds = active_bin_indices();
            SL_BinCounter<uint32_t>* const pTempBinIds = active_temp_bin_indices();

            utils::sort_radix<SL_BinCounter<uint32_t>>(pActiveBinIds, pTempBinIds, (uint64_t)maxElements, [&](const SL_BinCounter<uint32_t>& val) noexcept->unsigned long long
            {
                return (unsigned long long)pBins[val.count].primIndex;
            });
        }
        else if (canDepthSort)
        {
            SL_BinCounter<uint32_t>* const pActiveBinIds = active_bin_indices();
            SL_BinCounter<uint32_t>* const pTempBinIds = active_temp_bin_indices();

            utils::sort_radix<SL_BinCounter<uint32_t>>(pActiveBinIds, pTempBinIds, (uint64_t)maxElements, [&](const SL_BinCounter<uint32_t>& val) noexcept->unsigned long long
            {
                // flip sign, otherwise the sorting goes from back-to-front
                // due to the sortable nature of floats.
                static_assert(sizeof(float) == sizeof(int32_t), "Current architecture doesn't have similar float & integer sizes.");
                union
                {
                    float f;
                    int32_t i;
                } w{pBins[val.count].mScreenCoords[0][3]};
                return (unsigned long long) -w.i;
            });
        }

        // Let all threads know they can process fragments.
        active_frag_processors().count.store(syncPoint1, std::memory_order_release);
    }
    else
    {
        _sl_cpu_yield_loop([&]() noexcept -> bool
        {
            return active_frag_processors().count.load(std::memory_order_consume) > 0;
        });

        maxElements = math::min<uint_fast64_t>(active_num_bins_used().count.load(std::memory_order_consume), SL_SHADER_MAX_BINNED_PRIMS);
    }

    RasterizerType rasterizer;
    rasterizer.mThreadId = (uint16_t)mThreadId;
    rasterizer.mMode = mRenderMode;
    rasterizer.mNumProcessors = (uint32_t)mNumThreads;
    rasterizer.mNumBins = maxElements;
    rasterizer.mShader = mShader;
    rasterizer.mFbo = mFbo;
    rasterizer.mBinIds = active_bin_indices();
    rasterizer.mBins = pBins;
    rasterizer.mQueues = mFragQueues + mThreadId;

    rasterizer.execute();

    // Indicate to all threads we can now process more vertices
    syncPoint2 = active_frag_processors().count.fetch_add(1, std::memory_order_acq_rel);

    if (syncPoint2 == -2)
    {
        active_num_bins_used().count.store(0, std::memory_order_release);
        active_frag_processors().count.store(0, std::memory_order_release);
    }
    else if (active_buffer_index() == next_buffer_index())
    {
        _sl_cpu_yield_loop([&]() noexcept -> bool {
            return active_frag_processors().count.load(std::memory_order_consume) < 0;
        });
    }

    flip_process_buffers();
}



/*--------------------------------------
 * Perform a final sync
--------------------------------------*/
template <typename RasterizerType>
void SL_VertexProcessor::cleanup() noexcept
{
    static_assert(ls::setup::IsBaseOf<SL_FragmentProcessor, RasterizerType>::value, "Template parameter 'RasterizerType' must derive from SL_FragmentProcessor.");

    uint_fast64_t numActiveFragProcessors;
    uint_fast32_t currentIters = 0;

    mBusyProcessors->count.fetch_sub(1, std::memory_order_acq_rel);

    do
    {
        if (LS_LIKELY(active_frag_processors().count.load(std::memory_order_acquire) > 0))
        {
            flush_rasterizer<RasterizerType>();
        }
        else
        {
            currentIters = _sl_cpu_yield_exponential<uint_fast32_t>(currentIters);
        }

        numActiveFragProcessors = mBusyProcessors->count.load(std::memory_order_acquire);
    }
    while (numActiveFragProcessors);

    if (LS_LIKELY(active_num_bins_used().count.load(std::memory_order_acquire)))
    {
        flush_rasterizer<RasterizerType>();
    }
}



/*--------------------------------------
 * Raster Specializations
--------------------------------------*/
template void SL_VertexProcessor::flush_rasterizer<SL_PointRasterizer>() noexcept;
template void SL_VertexProcessor::flush_rasterizer<SL_LineRasterizer>() noexcept;
template void SL_VertexProcessor::flush_rasterizer<SL_TriRasterizer>() noexcept;

template void SL_VertexProcessor::cleanup<SL_PointRasterizer>() noexcept;
template void SL_VertexProcessor::cleanup<SL_LineRasterizer>() noexcept;
template void SL_VertexProcessor::cleanup<SL_TriRasterizer>() noexcept;
