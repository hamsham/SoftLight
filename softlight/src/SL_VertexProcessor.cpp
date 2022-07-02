
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

    // For once, I'm incredibly surprised by Clang. A simple switch statement
    // would blow up in the generated assembly. Coercing Clang to generate
    // simpler assembly required a bit more explicit code than I expected.

    // The combination of x < 1, x == 1, and x > 1 allow clang to compile
    // these branches into an optimal jump table using a single comparison and
    // 3 jump instructions
    if (yieldCount > 1)
    {
        goto yield3;
    }
    else if (yieldCount == 1)
    {
        goto yield2;
    }
    else if (yieldCount < 1)
    {
        goto yield1;
    }
    else
    {
        LS_UNREACHABLE();
    }

    yield3:
    ls::setup::cpu_yield();
    ls::setup::cpu_yield();
    ls::setup::cpu_yield();
    ls::setup::cpu_yield();

    yield2:
    ls::setup::cpu_yield();
    ls::setup::cpu_yield();

    yield1:
    ls::setup::cpu_yield();

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
void SL_VertexProcessor::flush_rasterizer() const noexcept
{
    // Allow the other threads to know when they're ready for processing
    const int_fast64_t    numThreads   = (int_fast64_t)mNumThreads;
    const int_fast64_t    syncPoint1   = -numThreads - 1;
    const int_fast64_t    tileId       = mFragProcessors->count.fetch_add(1ll, std::memory_order_acq_rel);
    const SL_FragmentBin* pBins        = mFragBins;
    uint_fast64_t         maxElements;
    int_fast64_t          syncPoint2;

    // Sort the bins based on their depth.
    if (LS_UNLIKELY(tileId == numThreads-1u))
    {
        maxElements = math::min<uint_fast64_t>(mBinsUsed->count.load(std::memory_order_consume), SL_SHADER_MAX_BINNED_PRIMS);

        // Try to perform depth sorting once, and only once, per opaque draw
        // call to reduce depth-buffer access during rasterization. Sorting
        // primitives multiple times here in the vertex processor will
        // increase latency before invoking the fragment processor.
        bool shouldDepthSort = ls::setup::IsSame<RasterizerType, SL_TriRasterizer>::value;
        const bool canDepthSort = shouldDepthSort && (maxElements < SL_SHADER_MAX_BINNED_PRIMS);

        // Blended fragments get sorted by their primitive index for
        // consistency.
        if (LS_UNLIKELY(mShader->pipelineState.blend_mode() != SL_BLEND_OFF))
        {
            utils::sort_radix<SL_BinCounter<uint32_t>>(mBinIds, mTempBinIds, (uint64_t)maxElements, [&](const SL_BinCounter<uint32_t>& val) noexcept->unsigned long long
            {
                return (unsigned long long)pBins[val.count].primIndex;
            });
        }
        else if (canDepthSort)
        {
            utils::sort_radix<SL_BinCounter<uint32_t>>(mBinIds, mTempBinIds, (uint64_t)maxElements, [&](const SL_BinCounter<uint32_t>& val) noexcept->unsigned long long
            {
                // flip sign, otherwise the sorting goes from back-to-front
                // due to the sortable nature of floats.
                static_assert(sizeof(float) == sizeof(int32_t), "Current architecture doesn't have similar float & integer sizes.");
                union
                {
                    float f;
                    int32_t i;
                } w{pBins[val.count].mScreenCoords[0].v[3]};
                return (unsigned long long) -w.i;
            });
        }

        // Let all threads know they can process fragments.
        mFragProcessors->count.store(syncPoint1, std::memory_order_release);
    }
    else
    {
        _sl_cpu_yield_loop([&]()->bool {
            return mFragProcessors->count.load(std::memory_order_consume) > 0;
        });

        maxElements = math::min<uint_fast64_t>(mBinsUsed->count.load(std::memory_order_consume), SL_SHADER_MAX_BINNED_PRIMS);
    }

    static_assert(ls::setup::IsBaseOf<SL_FragmentProcessor, RasterizerType>::value, "Template parameter 'RasterizerType' must derive from SL_FragmentProcessor.");

    RasterizerType rasterizer;
    rasterizer.mThreadId = (uint16_t)mThreadId;
    rasterizer.mMode = mRenderMode;
    rasterizer.mNumProcessors = (uint32_t)mNumThreads;
    rasterizer.mNumBins = maxElements;
    rasterizer.mShader = mShader;
    rasterizer.mFbo = mFbo;
    rasterizer.mBinIds = mBinIds;
    rasterizer.mBins = pBins;
    rasterizer.mQueues = mFragQueues + mThreadId;

    rasterizer.execute();

    // Indicate to all threads we can now process more vertices
    syncPoint2 = mFragProcessors->count.fetch_add(1, std::memory_order_acq_rel);

    if (syncPoint2 == -2)
    {
        mBinsUsed->count.store(0, std::memory_order_release);
        mFragProcessors->count.store(0, std::memory_order_release);
    }
    else
    {
        if (LS_UNLIKELY(!mAmDone))
        {
            _sl_cpu_yield_loop([&]()->bool {
                return mFragProcessors->count.load(std::memory_order_consume) < 0;
            });
        }
    }
}



/*--------------------------------------
 * Perform a final sync
--------------------------------------*/
template <typename RasterizerType>
void SL_VertexProcessor::cleanup() noexcept
{
    static_assert(ls::setup::IsBaseOf<SL_FragmentProcessor, RasterizerType>::value, "Template parameter 'RasterizerType' must derive from SL_FragmentProcessor.");
    uint_fast32_t currentIters = 0;

    mBusyProcessors->count.fetch_sub(1, std::memory_order_acq_rel);
    while (mBusyProcessors->count.load(std::memory_order_acquire))
    {
        if (LS_LIKELY(mFragProcessors->count.load(std::memory_order_acquire) > 0))
        {
            flush_rasterizer<RasterizerType>();

            _sl_cpu_yield_loop([&]()->bool {
                return mFragProcessors->count.load(std::memory_order_consume) < 0;
            });
        }
        else
        {
            currentIters = _sl_cpu_yield_exponential<uint_fast32_t>(currentIters);
        }
    }

    if (LS_LIKELY(mBinsUsed->count.load(std::memory_order_acquire)))
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
