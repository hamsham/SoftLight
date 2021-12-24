
#include "lightsky/setup/Types.h"

#include "lightsky/utils/Sort.hpp" // utils::sort_radix

#include "softlight/SL_Context.hpp"
#include "softlight/SL_LineRasterizer.hpp"
#include "softlight/SL_PointRasterizer.hpp"
#include "softlight/SL_Shader.hpp" // SL_Shader
#include "softlight/SL_ShaderUtil.hpp" // SL_BinCounter, SL_BinCounterAtomic
#include "softlight/SL_TriRasterizer.hpp"
#include "softlight/SL_VertexProcessor.hpp"
#include "softlight/SL_ViewportState.hpp"

namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * SL_VertexProcessor Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Execute the rasterizer
-------------------------------------*/
template <typename RasterizerType>
void SL_VertexProcessor::flush_rasterizer() const noexcept
{
    // Allow the other threads to know when they're ready for processing
    const int_fast64_t    numThreads   = (int_fast64_t)mNumThreads;
    const int_fast64_t    syncPoint1   = -numThreads - 1;
    const int_fast64_t    tileId       = mFragProcessors->count.fetch_add(1ll, std::memory_order_acq_rel);
    const SL_FragmentBin* pBins        = mFragBins;
    constexpr unsigned    maxIters     = 8;
    unsigned              currentIters = 1;
    uint_fast64_t         maxElements;
    int_fast64_t          syncPoint2;

    // Sort the bins based on their depth.
    if (LS_UNLIKELY(tileId == numThreads-1u))
    {
        maxElements = math::min<uint64_t>(mBinsUsed->count.load(std::memory_order_consume), SL_SHADER_MAX_BINNED_PRIMS);

        // Try to perform depth sorting once, and only once, per opaque draw
        // call to reduce depth-buffer access during rasterization. Sorting
        // primitives multiple times here in the vertex processor will
        // increase latency before invoking the fragment processor.
        bool shouldDepthSort = mMeshes[0].mode != SL_RenderMode::RENDER_MODE_POINTS && mMeshes[0].mode != SL_RenderMode::RENDER_MODE_INDEXED_POINTS;
        const bool canDepthSort = shouldDepthSort && (maxElements < SL_SHADER_MAX_BINNED_PRIMS);

        // Blended fragments get sorted by their primitive index for
        // consistency.
        if (LS_UNLIKELY(mShader->fragment_shader().blend != SL_BLEND_OFF))
        {
            utils::sort_radix<SL_BinCounter<uint32_t>>(mBinIds, mTempBinIds, maxElements, [&](const SL_BinCounter<uint32_t>& val) noexcept->unsigned long long
            {
                return (unsigned long long)pBins[val.count].primIndex;
            });
        }
        else if (canDepthSort)
        {
            utils::sort_radix<SL_BinCounter<uint32_t>>(mBinIds, mTempBinIds, maxElements, [&](const SL_BinCounter<uint32_t>& val) noexcept->unsigned long long
            {
                // flip sign, otherwise the sorting goes from back-to-front
                // due to the sortable nature of floats.
                return (unsigned long long) -(*reinterpret_cast<const int32_t*>(pBins[val.count].mScreenCoords[0].v+3));
            });
        }

        // Let all threads know they can process fragments.
        mFragProcessors->count.store(syncPoint1, std::memory_order_release);
    }
    else
    {
        while (mFragProcessors->count.load(std::memory_order_consume) > 0)
        {
            switch (currentIters)
            {
                case 8:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 4:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 2:
                    ls::setup::cpu_yield();
                default:
                    ls::setup::cpu_yield();
                    currentIters = ls::math::min(currentIters+currentIters, maxIters);
            }
        }

        maxElements = math::min<uint64_t>(mBinsUsed->count.load(std::memory_order_consume), SL_SHADER_MAX_BINNED_PRIMS);
    }

    static_assert(ls::setup::IsBaseOf<SL_FragmentProcessor, RasterizerType>::value, "Template parameter 'RasterizerType' must derive from SL_FragmentProcessor.");
    RasterizerType rasterizer;

    const SL_ViewportState& viewState = mContext->viewport_state();

    rasterizer.mThreadId = (uint16_t)mThreadId;
    rasterizer.mMode = mRenderMode;
    rasterizer.mNumProcessors = (uint32_t)mNumThreads;
    rasterizer.mNumBins = (uint32_t)maxElements;
    rasterizer.mShader = mShader;
    rasterizer.mFbo = mFbo;
    rasterizer.mViewState = &viewState;
    rasterizer.mBinIds = mBinIds;
    rasterizer.mBins = mFragBins;
    rasterizer.mQueues = mFragQueues + mThreadId;

    rasterizer.execute();

    // Indicate to all threads we can now process more vertices
    syncPoint2 = mFragProcessors->count.fetch_add(1, std::memory_order_acq_rel);

    if (syncPoint2 == -2)
    {
        mBinsUsed->count.store(0, std::memory_order_release);
        mFragProcessors->count.store(0, std::memory_order_release);
        return;
    }

    do
    {
        // wait until all fragments are rendered across the other threads
        switch (currentIters)
        {
            case 8:
                ls::setup::cpu_yield();
                ls::setup::cpu_yield();
                ls::setup::cpu_yield();
                ls::setup::cpu_yield();
            case 4:
                ls::setup::cpu_yield();
                ls::setup::cpu_yield();
            case 2:
                ls::setup::cpu_yield();
            default:
                ls::setup::cpu_yield();
                currentIters = ls::math::min(currentIters+currentIters, maxIters);
        }
    }
    while (mFragProcessors->count.load(std::memory_order_consume) < 0);
}



/*--------------------------------------
 * Perform a final sync
--------------------------------------*/
template <typename RasterizerType>
void SL_VertexProcessor::cleanup() noexcept
{
    static_assert(ls::setup::IsBaseOf<SL_FragmentProcessor, RasterizerType>::value, "Template parameter 'RasterizerType' must derive from SL_FragmentProcessor.");
    std::atomic<uint_fast64_t>& busyProcessors = mBusyProcessors->count;
    std::atomic<int_fast64_t>& fragProcessors = mFragProcessors->count;
    constexpr unsigned maxIters = 8;
    unsigned currentIters = 1;

    busyProcessors.fetch_sub(1, std::memory_order_acq_rel);
    while (busyProcessors.load(std::memory_order_consume))
    {
        if (LS_UNLIKELY(fragProcessors.load(std::memory_order_consume)))
        {
            flush_rasterizer<RasterizerType>();
        }
        else
        {
            switch (currentIters)
            {
                case 8:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 4:
                    ls::setup::cpu_yield();
                    ls::setup::cpu_yield();
                case 2:
                    ls::setup::cpu_yield();
                default:
                    ls::setup::cpu_yield();
                    currentIters = ls::math::min(currentIters+currentIters, maxIters);
            }
        }
    }

    if (LS_LIKELY(mBinsUsed->count.load(std::memory_order_consume)))
    {
        flush_rasterizer<RasterizerType>();
    }
}



/*--------------------------------------
 * Raster Specializations
--------------------------------------*/
template void SL_VertexProcessor::flush_rasterizer<SL_PointRasterizer>() const noexcept;
template void SL_VertexProcessor::flush_rasterizer<SL_LineRasterizer>() const noexcept;
template void SL_VertexProcessor::flush_rasterizer<SL_TriRasterizer>() const noexcept;

template void SL_VertexProcessor::cleanup<SL_PointRasterizer>() noexcept;
template void SL_VertexProcessor::cleanup<SL_LineRasterizer>() noexcept;
template void SL_VertexProcessor::cleanup<SL_TriRasterizer>() noexcept;
