
#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/math/half.h"
#include "lightsky/math/vec_utils.h"

#include "softlight/SL_Geometry.hpp"
#include "softlight/SL_LineRasterizer.hpp"
#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_ViewportState.hpp"
#include "softlight/SL_Shader.hpp" // SL_FragmentShader
#include "softlight/SL_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Namespace setup
-----------------------------------------------------------------------------*/
namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions
-----------------------------------------------------------------------------*/
namespace
{



/*--------------------------------------
 * Interpolate varying variables across a line
--------------------------------------*/
inline void LS_IMPERATIVE interpolate_line_varyings(
    const float             percent,
    const uint32_t          numVaryings,
    const math::vec4* const inVaryings,
    math::vec4* const       outVaryings
) noexcept
{
    constexpr uint32_t i2 = SL_SHADER_MAX_VARYING_VECTORS;

    for (uint32_t i = numVaryings; i--;)
    {
        const math::vec4& v0 = inVaryings[i];
        const math::vec4& v1 = inVaryings[i+i2];
        outVaryings[i] = math::mix(v0, v1, percent);
    }
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_LineRasterizer Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Bin-Rasterization
--------------------------------------*/
template <typename depth_type>
void SL_LineRasterizer::flush_fragments(
    const SL_FragmentBin* pBin,
    uint_fast32_t         numQueuedFrags,
    SL_FragCoord* const   outCoords) const noexcept
{
    const SL_PipelineState  pipeline      = mShader->pipelineState;
    const SL_BlendMode      blendMode     = pipeline.blend_mode();
    const SL_FboOutputMask  fboOutMask    = sl_calc_fbo_out_mask((unsigned)pipeline.num_render_targets(), (blendMode != SL_BLEND_OFF));
    const uint32_t          numVaryings   = (unsigned)pipeline.num_varyings();
    const int_fast32_t      haveDepthMask = pipeline.depth_mask() == SL_DEPTH_MASK_ON;
    const SL_UniformBuffer* pUniforms     = mShader->pUniforms;
    const auto              fragShader    = mShader->pFragShader;
    SL_Texture* const       pDepthBuf     = mFbo->get_depth_buffer();

    SL_FragmentParam fragParams;
    fragParams.pUniforms = pUniforms;

    uint_fast32_t i;

    for (i = 0; i < numQueuedFrags; ++i)
    {
        const float interp = outCoords->bc[i][0];

        interpolate_line_varyings(interp, numVaryings, pBin->mVaryings, fragParams.pVaryings);

        fragParams.coord = outCoords->coord[i];

        const bool haveOutputs = fragShader(fragParams);

        if (LS_LIKELY(haveOutputs))
        {
            mFbo->put_pixel(fboOutMask, blendMode, fragParams);

            if (LS_LIKELY(haveDepthMask))
            {
                pDepthBuf->texel<depth_type>(fragParams.coord.x, fragParams.coord.y) = (depth_type)fragParams.coord.depth;
            }
        }
    }
}



template void SL_LineRasterizer::flush_fragments<ls::math::half>(const SL_FragmentBin*, uint_fast32_t, SL_FragCoord* const) const noexcept;
template void SL_LineRasterizer::flush_fragments<float>(const SL_FragmentBin*, uint_fast32_t, SL_FragCoord* const) const noexcept;
template void SL_LineRasterizer::flush_fragments<double>(const SL_FragmentBin*, uint_fast32_t, SL_FragCoord* const) const noexcept;



/*--------------------------------------
 * Process the line fragments using a simple DDA algorithm
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_LineRasterizer::render_line(const uint32_t binId, SL_Framebuffer* fbo) noexcept
{
    const math::vec4&  screenCoord0  = mBins[binId].mScreenCoords[0];
    const math::vec4&  screenCoord1  = mBins[binId].mScreenCoords[1];
    math::vec2         clipCoords[2] = {math::vec2_cast(screenCoord0), math::vec2_cast(screenCoord1)};
    const SL_Texture*  depthBuf      = fbo->get_depth_buffer();
    const float        dist          = math::inversesqrt(math::length_squared(math::vec2_cast(screenCoord1)-math::vec2_cast(screenCoord0)));
    const math::vec4&& p0            = math::vec4_cast(math::vec2_cast(screenCoord0), 0.f, 0.f);
    float              z0            = screenCoord0[2];
    float              z1            = screenCoord1[2];

    constexpr DepthCmpFunc depthCmp = {};

    SL_FragmentParam fragParams;
    fragParams.pUniforms = mShader->pUniforms;

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
            const float currLen = math::length(p - p0);
            const float interp  = currLen * dist;
            const float z       = math::mix(z0, z1, interp);

            if (!depthCmp(z, (float)depthBuf->texel<depth_type>(x, y)))
            {
                return;
            }

            outCoords->bc[numQueuedFrags][0]       = interp;
            outCoords->coord[numQueuedFrags].x     = x;
            outCoords->coord[numQueuedFrags].y     = y;
            outCoords->coord[numQueuedFrags].depth = z;

            ++numQueuedFrags;

            if (LS_UNLIKELY(numQueuedFrags == SL_SHADER_MAX_QUEUED_FRAGS))
            {
                numQueuedFrags = 0;
                flush_fragments<depth_type>(mBins+binId, SL_SHADER_MAX_QUEUED_FRAGS, outCoords);
            }
        }
    );

    // cleanup remaining fragments
    if (LS_LIKELY(numQueuedFrags > 0))
    {
        flush_fragments<depth_type>(mBins+binId, numQueuedFrags, outCoords);
    }
}



template void SL_LineRasterizer::render_line<SL_DepthFuncLT, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLT, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLT, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncLE, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLE, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLE, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncGT, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGT, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGT, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncGE, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGE, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGE, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncNE, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncNE, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncNE, double>(const uint32_t, SL_Framebuffer* const) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, ls::math::half>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, float>(const uint32_t, SL_Framebuffer* const) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, double>(const uint32_t, SL_Framebuffer* const) noexcept;



/*-------------------------------------
 * Dispatch the fragment processor with the correct depth-comparison function
-------------------------------------*/
template <class DepthCmpFunc>
void SL_LineRasterizer::dispatch_bins() noexcept
{
    const uint16_t depthBpp = mFbo->get_depth_buffer()->bpp();

    if (depthBpp == sizeof(math::half))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, math::half>(binId, mFbo);
        }
    }
    else if (depthBpp == sizeof(float))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, float>(binId, mFbo);
        }
    }
    else if (depthBpp == sizeof(double))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, double>(binId, mFbo);
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
