
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
void SL_PointRasterizer::render_point(const uint32_t binId, SL_Framebuffer* const fbo) noexcept
{
    constexpr DepthCmpFunc  depthCmp    = {};
    const SL_FragmentShader fragShader  = mShader->mFragShader;
    const SL_UniformBuffer* pUniforms   = mShader->mUniforms;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const bool              depthMask   = fragShader.depthMask == SL_DEPTH_MASK_ON;
    const auto              pShader     = fragShader.shader;
    const SL_BlendMode      blendMode   = fragShader.blend;
    const bool              blend       = blendMode != SL_BLEND_OFF;
    const math::vec4        screenCoord = mBins[binId].mScreenCoords[0];
    const math::vec4        fragCoord   {screenCoord[0], screenCoord[1], screenCoord[2], 1.f};
    const SL_Texture*       pDepthBuf   = fbo->get_depth_buffer();
    SL_FragmentParam        fragParams;

    if ((uint16_t)fragCoord.v[1] % mNumProcessors != mThreadId)
    {
        return;
    }

    fragParams.coord.x = (uint16_t)fragCoord[0];
    fragParams.coord.y = (uint16_t)fragCoord[1];
    fragParams.coord.depth = fragCoord[2];
    fragParams.pUniforms = pUniforms;

    if (!depthCmp(fragCoord[2], (float)pDepthBuf->texel<depth_type>(fragParams.coord.x, fragParams.coord.y)))
    {
        return;
    }

    for (unsigned i = SL_SHADER_MAX_VARYING_VECTORS; i--;)
    {
        fragParams.pVaryings[i] = mBins[binId].mVaryings[i];
    }

    const uint_fast32_t haveOutputs = pShader(fragParams);

    if (blend)
    {
        // branchless select
        switch (-haveOutputs & numOutputs)
        {
            case 4: fbo->put_alpha_pixel(3, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3], blendMode);
            case 3: fbo->put_alpha_pixel(2, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2], blendMode);
            case 2: fbo->put_alpha_pixel(1, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1], blendMode);
            case 1: fbo->put_alpha_pixel(0, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0], blendMode);
            //case 1: fbo->put_pixel(0, fragParams.coord.x, fragParams.coord.y, math::vec4{1.f, 0.f, 1.f, 1.f});
        }

    }
    else
    {
        // branchless select
        switch (-haveOutputs & numOutputs)
        {
            case 4: fbo->put_pixel(3, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3]);
            case 3: fbo->put_pixel(2, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2]);
            case 2: fbo->put_pixel(1, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1]);
            case 1: fbo->put_pixel(0, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0]);
            //case 1: fbo->put_pixel(0, fragParams.coord.x, fragParams.coord.y, math::vec4{1.f, 0.f, 1.f, 1.f});
        }
    }

    if (haveOutputs && depthMask)
    {
        fbo->put_depth_pixel<depth_type>(fragParams.coord.x, fragParams.coord.y, (depth_type)fragParams.coord.depth);
    }
}



template void SL_PointRasterizer::render_point<SL_DepthFuncLT, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLT, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLT, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncLE, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLE, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLE, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncGT, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGT, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGT, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncGE, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGE, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGE, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncNE, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncNE, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncNE, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, double>(const uint32_t, SL_Framebuffer* const) noexcept;



/*-------------------------------------
 * Dispatch the fragment processor with the correct depth-comparison function
-------------------------------------*/
template <class DepthCmpFunc>
void SL_PointRasterizer::dispatch_bins() noexcept
{
    const uint16_t depthBpp = mFbo->get_depth_buffer()->bpp();

    if (depthBpp == sizeof(math::half))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_point<DepthCmpFunc, math::half>(binId, mFbo);
        }
    }
    else if (depthBpp == sizeof(float))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_point<DepthCmpFunc, float>(binId, mFbo);
        }
    }
    else if (depthBpp == sizeof(double))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_point<DepthCmpFunc, double>(binId, mFbo);
        }
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
