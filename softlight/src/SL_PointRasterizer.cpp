
#include "lightsky/math/half.h"

#include "softlight/SL_PointRasterizer.hpp"
#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_Shader.hpp" // SL_FragmentShader
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_ViewportState.hpp"



/*-----------------------------------------------------------------------------
 * Namespace setup
-----------------------------------------------------------------------------*/
namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * SL_FragmentProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Determine if a point can be rendered.
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_PointRasterizer::render_point(SL_Framebuffer* const fbo) noexcept
{
    constexpr DepthCmpFunc  depthCmp    {};
    const SL_Texture*       pDepthBuf   = fbo->get_depth_buffer();
    const SL_FragmentShader fragShader  = mShader->mFragShader;
    const SL_UniformBuffer* pUniforms   = mShader->mUniforms;
    const bool              depthMask   = fragShader.depthMask == SL_DEPTH_MASK_ON;
    const auto              pShader     = fragShader.shader;
    const SL_FboOutputMask  fboOutMask  = sl_calc_fbo_out_mask(fragShader.numOutputs, (fragShader.blend != SL_BLEND_OFF));
    SL_FragmentParam        fragParams;

    fragParams.pUniforms = pUniforms;

    for (uint64_t binId = 0; binId < mNumBins; ++binId)
    {
        const SL_FragmentBin& bin = mBins[binId];
        const math::vec4& screenCoord = bin.mScreenCoords[0];

        fragParams.coord.x     = (uint16_t)screenCoord[0];
        fragParams.coord.y     = (uint16_t)screenCoord[1];
        fragParams.coord.depth = screenCoord[2];
        if (LS_LIKELY(fragParams.coord.y % mNumProcessors != mThreadId))
        {
            continue;
        }

        if (LS_UNLIKELY(!depthCmp(fragParams.coord.depth, (float)pDepthBuf->texel<depth_type>(fragParams.coord.x, fragParams.coord.y))))
        {
            continue;
        }

        for (unsigned i = fragShader.numVaryings; i--;)
        {
            fragParams.pVaryings[i] = bin.mVaryings[i];
        }

        const bool haveOutputs = pShader(fragParams);
        if (LS_LIKELY(haveOutputs))
        {
            mFbo->put_pixel(fboOutMask, fragShader.blend, fragParams);

            if (depthMask)
            {
                fbo->put_depth_pixel<depth_type>(fragParams.coord.x, fragParams.coord.y, (depth_type)fragParams.coord.depth);
            }
        }
    }
}



template void SL_PointRasterizer::render_point<SL_DepthFuncLT, ls::math::half>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLT, float>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLT, double>(SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncLE, ls::math::half>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLE, float>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLE, double>(SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncGT, ls::math::half>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGT, float>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGT, double>(SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncGE, ls::math::half>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGE, float>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGE, double>(SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, ls::math::half>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, float>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, double>(SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncNE, ls::math::half>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncNE, float>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncNE, double>(SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, ls::math::half>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, float>(SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, double>(SL_Framebuffer* const) noexcept;



/*-------------------------------------
 * Dispatch the fragment processor with the correct depth-comparison function
-------------------------------------*/
template <class DepthCmpFunc>
void SL_PointRasterizer::dispatch_bins() noexcept
{
    const uint16_t depthBpp = mFbo->get_depth_buffer()->bpp();

    if (depthBpp == sizeof(math::half))
    {
        render_point<DepthCmpFunc, math::half>(mFbo);
    }
    else if (depthBpp == sizeof(float))
    {
        render_point<DepthCmpFunc, float>(mFbo);
    }
    else if (depthBpp == sizeof(double))
    {
        render_point<DepthCmpFunc, double>(mFbo);
    }
}



template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncLT>() noexcept;
template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncLE>() noexcept;
template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncGT>() noexcept;
template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncGE>() noexcept;
template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncEQ>() noexcept;
template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncNE>() noexcept;
template void SL_PointRasterizer::dispatch_bins<SL_DepthFuncOFF>() noexcept;



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SL_PointRasterizer::execute() noexcept
{
    const SL_DepthTest depthTestType = mShader->fragment_shader().depthTest;

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
