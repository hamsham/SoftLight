
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
    #if defined(LS_X86_AVX2)
        (void)numVaryings;
        const __m256 p = _mm256_set1_ps(percent);
        __m256 v0, v1, v2, v3, v4, v5, o0, o1;

        v0 = _mm256_load_ps(reinterpret_cast<const float*>(inVaryings));
        v1 = _mm256_load_ps(reinterpret_cast<const float*>(inVaryings+SL_SHADER_MAX_VARYING_VECTORS));
        v3 = _mm256_load_ps(reinterpret_cast<const float*>(inVaryings+2));
        v4 = _mm256_load_ps(reinterpret_cast<const float*>(inVaryings+2+SL_SHADER_MAX_VARYING_VECTORS));

        v2 = _mm256_sub_ps(v1, v0);
        v5 = _mm256_sub_ps(v4, v3);
        o0 = _mm256_fmadd_ps(v2, p, v0);
        o1 = _mm256_fmadd_ps(v5, p, v3);

        _mm256_store_ps(reinterpret_cast<float*>(outVaryings), o0);
        _mm256_store_ps(reinterpret_cast<float*>(outVaryings+2), o1);

        _mm256_zeroupper();

    #elif defined(LS_X86_FMA)
        (void)numVaryings;
        const __m128 p = _mm_set1_ps(percent);
        __m128 v0, v1, v2, v3, v4, v5, o0, o1;

        v0 = _mm_load_ps(reinterpret_cast<const float*>(inVaryings));
        v1 = _mm_load_ps(reinterpret_cast<const float*>(inVaryings+SL_SHADER_MAX_VARYING_VECTORS));
        v3 = _mm_load_ps(reinterpret_cast<const float*>(inVaryings+1));
        v4 = _mm_load_ps(reinterpret_cast<const float*>(inVaryings+1+SL_SHADER_MAX_VARYING_VECTORS));

        v2 = _mm_sub_ps(v1, v0);
        v5 = _mm_sub_ps(v4, v3);
        o0 = _mm_fmadd_ps(v2, p, v0);
        o1 = _mm_fmadd_ps(v5, p, v3);

        _mm_store_ps(reinterpret_cast<float*>(outVaryings), o0);
        _mm_store_ps(reinterpret_cast<float*>(outVaryings+1), o1);

        v0 = _mm_load_ps(reinterpret_cast<const float*>(inVaryings+2));
        v1 = _mm_load_ps(reinterpret_cast<const float*>(inVaryings+2+SL_SHADER_MAX_VARYING_VECTORS));
        v3 = _mm_load_ps(reinterpret_cast<const float*>(inVaryings+3));
        v4 = _mm_load_ps(reinterpret_cast<const float*>(inVaryings+3+SL_SHADER_MAX_VARYING_VECTORS));

        v2 = _mm_sub_ps(v1, v0);
        v5 = _mm_sub_ps(v4, v3);
        o0 = _mm_fmadd_ps(v2, p, v0);
        o1 = _mm_fmadd_ps(v5, p, v3);

        _mm_store_ps(reinterpret_cast<float*>(outVaryings+2), o0);
        _mm_store_ps(reinterpret_cast<float*>(outVaryings+3), o1);

    #else
        const math::vec4* v0;
        const math::vec4* v1;

        switch (numVaryings)
        {
            case 4:
                v0 = inVaryings+3;
                v1 = inVaryings+3+SL_SHADER_MAX_VARYING_VECTORS;
                outVaryings[3] = math::mix(*v0, *v1, percent);

            case 3:
                v0 = inVaryings+2;
                v1 = inVaryings+2+SL_SHADER_MAX_VARYING_VECTORS;
                outVaryings[2] = math::mix(*v0, *v1, percent);

            case 2:
                v0 = inVaryings+1;
                v1 = inVaryings+1+SL_SHADER_MAX_VARYING_VECTORS;
                outVaryings[1] = math::mix(*v0, *v1, percent);

            case 1:
                v0 = inVaryings;
                v1 = inVaryings+SL_SHADER_MAX_VARYING_VECTORS;
                outVaryings[0] = math::mix(*v0, *v1, percent);
        }

    #endif
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
    const SL_FragmentBin& bin,
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
        const float interp = outCoords->lineInterp[i];

        interpolate_line_varyings(interp, numVaryings, bin.mVaryings, fragParams.pVaryings);

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



template void SL_LineRasterizer::flush_fragments<ls::math::half>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
template void SL_LineRasterizer::flush_fragments<float>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
template void SL_LineRasterizer::flush_fragments<double>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;



/*--------------------------------------
 * Process the line fragments using a simple DDA algorithm
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
    const SL_Texture*  depthBuf      = fbo->get_depth_buffer();

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
            const float currLen = math::length(p - p0);
            const float interp  = currLen * dist;
            const float z       = math::mix(z0, z1, interp);

            if (!depthCmp(z, (float)depthBuf->texel<depth_type>(x, y)))
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
                flush_fragments<depth_type>(bin, SL_SHADER_MAX_QUEUED_FRAGS, outCoords);
            }
        }
    );

    // cleanup remaining fragments
    if (LS_LIKELY(numQueuedFrags > 0))
    {
        flush_fragments<depth_type>(bin, numQueuedFrags, outCoords);
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
    const uint16_t depthBpp = mFbo->get_depth_buffer()->bpp();

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
