
#include "lightsky/math/bits.h"
#include "lightsky/math/half.h"

#include "softlight/SL_Config.hpp"
#include "softlight/SL_PointRasterizer.hpp"
#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_Shader.hpp" // SL_FragmentShader
#include "softlight/SL_Texture.hpp"



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
template <typename depth_type>
void SL_PointRasterizer::render_point(const uint32_t binId, SL_Framebuffer* const fbo) noexcept
{
    const SL_FragmentShader fragShader  = mShader->mFragShader;
    const SL_UniformBuffer* pUniforms   = mShader->mUniforms;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const bool              depthTest   = fragShader.depthTest == SL_DEPTH_TEST_ON;
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

    if (depthTest == SL_DEPTH_TEST_ON)
    {
#if SL_REVERSED_Z_RENDERING
        if (fragCoord[2] < (float)pDepthBuf->raw_texel<depth_type>(fragParams.coord.x, fragParams.coord.y))
#else
        if (fragCoord[2] > (float)pDepthBuf->raw_texel<depth_type>(fragParams.coord.x, fragParams.coord.y))
#endif
        {
            return;
        }
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
                //case 1: fbo->put_pixel(0, fragParams.coord.x, fragParams.coord.y, math::vec4{1.f, 0, 1.f, 1.f});
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
                //case 1: fbo->put_pixel(0, fragParams.coord.x, fragParams.coord.y, math::vec4{1.f, 0, 1.f, 1.f});
        }
    }

    if (haveOutputs && depthMask)
    {
        fbo->put_depth_pixel<depth_type>(fragParams.coord.x, fragParams.coord.y, (depth_type)fragParams.coord.depth);
    }
}



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SL_PointRasterizer::execute() noexcept
{
    const uint16_t depthBpp = mFbo->get_depth_buffer()->bpp();

    if (depthBpp == sizeof(math::half))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_point<math::half>(binId, mFbo);
        }
    }
    else if (depthBpp == sizeof(float))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_point<float>(binId, mFbo);
        }
    }
    else if (depthBpp == sizeof(double))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_point<double>(binId, mFbo);
        }
    }
}

template void SL_PointRasterizer::render_point<ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_PointRasterizer::render_point<double>(const uint32_t, SL_Framebuffer* const) noexcept;
