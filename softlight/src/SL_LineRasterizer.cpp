
#include "lightsky/math/half.h"
#include "lightsky/math/vec_utils.h"

#include "softlight/SL_Geometry.hpp" // sl_draw_line_bresenham
#include "softlight/SL_LineRasterizer.hpp"
#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_Shader.hpp" // SL_FragmentShader
#include "softlight/SL_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Namespace setup
-----------------------------------------------------------------------------*/
namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * SL_LineRasterizer Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Enqueue line fragments for shading
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_LineRasterizer::render_line(const SL_FragmentBin& bin, SL_Framebuffer* fbo) noexcept
{
    const ls::math::vec4* coords     = bin.mScreenCoords;
    const math::vec4&  screenCoord0  = coords[0];
    const math::vec4&  screenCoord1  = coords[1];
    const float        z0            = screenCoord0[2];
    const float        z1            = screenCoord1[2];
    const math::vec2&& sc0           = math::vec2_cast(screenCoord0);
    const math::vec2&& sc1           = math::vec2_cast(screenCoord1);
    const math::vec4&& p0            = math::vec4_cast(sc0, 0.f, 0.f);
    math::vec2         clipCoords[2] = {sc0, sc1};
    const float        dist          = math::inversesqrt(math::length_squared(sc1-sc0));

    const SL_TextureView& depthBuf = fbo->get_depth_buffer();
    constexpr DepthCmpFunc depthCmp = {};

    SL_FragCoord* outCoords = mQueues;
    uint32_t numQueuedFrags = 0;

    sl_draw_line_bresenham(
        (uint16_t)clipCoords[0][0],
        (uint16_t)clipCoords[0][1],
        (uint16_t)clipCoords[1][0],
        (uint16_t)clipCoords[1][1],
        [&](uint16_t x, uint16_t y) noexcept->void
        {
            if ((LS_LIKELY(y % mNumProcessors) != mThreadId))
            {
                return;
            }

            const math::vec4&& p = (math::vec4)math::vec4_t<uint16_t>{x, y, 0, 0};
            const float currLen  = math::length(p - p0);
            const float interp   = currLen * dist;
            const float z        = math::mix(z0, z1, interp);

            const depth_type d = ((depth_type*)depthBuf.pTexels)[x + depthBuf.width * y];
            if (!depthCmp(z, (float)d))
            {
                return;
            }

            outCoords->lineInterp[numQueuedFrags]  = interp;
            outCoords->coord[numQueuedFrags].x     = x;
            outCoords->coord[numQueuedFrags].y     = y;
            outCoords->coord[numQueuedFrags].depth = z;

            ++numQueuedFrags;

            if (LS_UNLIKELY(numQueuedFrags == SL_SHADER_MAX_QUEUED_FRAGS))
            {
                numQueuedFrags = 0;
                flush_line_fragments<depth_type>(bin, SL_SHADER_MAX_QUEUED_FRAGS, outCoords);
            }
        }
    );

    // cleanup remaining fragments
    if (LS_LIKELY(numQueuedFrags > 0))
    {
        flush_line_fragments<depth_type>(bin, numQueuedFrags, outCoords);
    }
}



template void SL_LineRasterizer::render_line<SL_DepthFuncLT, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLT, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLT, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncLE, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLE, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLE, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncGT, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGT, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGT, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncGE, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGE, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGE, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncNE, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncNE, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncNE, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, ls::math::half>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, float>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, double>(const SL_FragmentBin&, SL_Framebuffer* const) noexcept;



/*-------------------------------------
 * Dispatch the fragment processor with the correct depth-comparison function
-------------------------------------*/
template <class DepthCmpFunc>
void SL_LineRasterizer::dispatch_bins() noexcept
{
    const uint16_t depthBpp = mFbo->get_depth_buffer().bytesPerTexel;

    if (depthBpp == sizeof(math::half))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, math::half>(mBins[binId], mFbo);
        }
    }
    else if (depthBpp == sizeof(float))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, float>(mBins[binId], mFbo);
        }
    }
    else if (depthBpp == sizeof(double))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, double>(mBins[binId], mFbo);
        }
    }
}



template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncLT>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncLE>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncGT>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncGE>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncEQ>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncNE>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncOFF>() noexcept;



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SL_LineRasterizer::execute() noexcept
{
    const SL_DepthTest depthTestType = mShader->pipelineState.depth_test();

    switch (depthTestType)
    {
        case SL_DEPTH_TEST_OFF:
            dispatch_bins<SL_DepthFuncOFF>();
            break;

        case SL_DEPTH_TEST_LESS_THAN:
            dispatch_bins<SL_DepthFuncLT>();
            break;

        case SL_DEPTH_TEST_LESS_EQUAL:
            dispatch_bins<SL_DepthFuncLE>();
            break;

        case SL_DEPTH_TEST_GREATER_THAN:
            dispatch_bins<SL_DepthFuncGT>();
            break;

        case SL_DEPTH_TEST_GREATER_EQUAL:
            dispatch_bins<SL_DepthFuncGE>();
            break;

        case SL_DEPTH_TEST_EQUAL:
            dispatch_bins<SL_DepthFuncEQ>();
            break;

        case SL_DEPTH_TEST_NOT_EQUAL:
            dispatch_bins<SL_DepthFuncNE>();
            break;

        default:
            LS_DEBUG_ASSERT(false);
            LS_UNREACHABLE();
    }
}
