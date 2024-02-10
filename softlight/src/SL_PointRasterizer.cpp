
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
void SL_PointRasterizer::render_point() noexcept
{
    constexpr DepthCmpFunc  depthCmp    {};
    const SL_PipelineState  pipeline    = mShader->pipelineState;
    const SL_BlendMode      blendMode   = pipeline.blend_mode();
    const SL_FboOutputMask  fboOutMask  = sl_calc_fbo_out_mask((unsigned)pipeline.num_render_targets(), (blendMode != SL_BLEND_OFF));
    const uint32_t          numVaryings = (unsigned)pipeline.num_varyings();
    const bool              depthMask   = pipeline.depth_mask() == SL_DEPTH_MASK_ON;
    const auto              fragShader  = mShader->pFragShader;
    const SL_UniformBuffer* pUniforms   = mShader->pUniforms;
    SL_FboOutputFunctions&  fboOutFuncs = *mFragFuncs;
    SL_TextureView* const   pColorBufs  = fboOutFuncs.pColorAttachments;
    SL_TextureView&         pDepthBuf   = *fboOutFuncs.pDepthAttachment;
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

        const depth_type d = ((depth_type*)pDepthBuf.pTexels)[fragParams.coord.x + pDepthBuf.width * fragParams.coord.y];
        if (LS_UNLIKELY(!depthCmp(fragParams.coord.depth, (float)d)))
        {
            continue;
        }

        for (unsigned i = numVaryings; i--;)
        {
            fragParams.pVaryings[i] = bin.mVaryings[i];
        }

        const bool haveOutputs = fragShader(fragParams);
        if (LS_LIKELY(haveOutputs))
        {
            switch (fboOutMask)
            {
                case SL_FBO_OUTPUT_ATTACHMENT_0_1_2_3: (*fboOutFuncs.pOutFunc[3])(fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3], pColorBufs[3]);
                case SL_FBO_OUTPUT_ATTACHMENT_0_1_2:   (*fboOutFuncs.pOutFunc[2])(fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2], pColorBufs[2]);
                case SL_FBO_OUTPUT_ATTACHMENT_0_1:     (*fboOutFuncs.pOutFunc[1])(fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1], pColorBufs[1]);
                case SL_FBO_OUTPUT_ATTACHMENT_0:       (*fboOutFuncs.pOutFunc[0])(fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0], pColorBufs[0]);
                    break;

                case SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1_2_3: (*fboOutFuncs.pOutBlendedFunc[3])(fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3], pColorBufs[3], blendMode);
                case SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1_2:   (*fboOutFuncs.pOutBlendedFunc[2])(fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2], pColorBufs[2], blendMode);
                case SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1:     (*fboOutFuncs.pOutBlendedFunc[1])(fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1], pColorBufs[1], blendMode);
                case SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0:       (*fboOutFuncs.pOutBlendedFunc[0])(fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0], pColorBufs[0], blendMode);
                    break;

                default:
                    LS_UNREACHABLE();
            }
        }

        if (LS_LIKELY(depthMask))
        {
            ((depth_type*)pDepthBuf.pTexels)[fragParams.coord.x + pDepthBuf.width * fragParams.coord.y] = (depth_type)fragParams.coord.depth;
        }
    }
}



template void SL_PointRasterizer::render_point<SL_DepthFuncLT, ls::math::half>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLT, float>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLT, double>() noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncLE, ls::math::half>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLE, float>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncLE, double>() noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncGT, ls::math::half>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGT, float>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGT, double>() noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncGE, ls::math::half>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGE, float>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncGE, double>() noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, ls::math::half>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, float>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncEQ, double>() noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncNE, ls::math::half>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncNE, float>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncNE, double>() noexcept;

template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, ls::math::half>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, float>() noexcept;
template void SL_PointRasterizer::render_point<SL_DepthFuncOFF, double>() noexcept;



/*-------------------------------------
 * Dispatch the fragment processor with the correct depth-comparison function
-------------------------------------*/
template <class DepthCmpFunc>
void SL_PointRasterizer::dispatch_bins() noexcept
{
    const uint16_t depthBpp = mFragFuncs->pDepthAttachment->bytesPerTexel;

    if (depthBpp == sizeof(math::half))
    {
        render_point<DepthCmpFunc, math::half>();
    }
    else if (depthBpp == sizeof(float))
    {
        render_point<DepthCmpFunc, float>();
    }
    else if (depthBpp == sizeof(double))
    {
        render_point<DepthCmpFunc, double>();
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
