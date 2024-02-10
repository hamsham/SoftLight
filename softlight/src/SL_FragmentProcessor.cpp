
#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "softlight/SL_FragmentProcessor.hpp"
#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_PipelineState.hpp"
#include "softlight/SL_Shader.hpp" // SL_FragmentShader



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions & Namespace Setup
-----------------------------------------------------------------------------*/
namespace math = ls::math;
namespace utils = ls::utils;



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

    #elif defined(LS_ARM_NEON)
        const float32x4_t p = vdupq_n_f32(percent);
        float32x4_t v0, v1, v2, o;

        switch (numVaryings)
        {
            case 4:
                v0 = vld1q_f32(reinterpret_cast<const float*>(inVaryings + 3));
                v1 = vld1q_f32(reinterpret_cast<const float*>(inVaryings + 3 + SL_SHADER_MAX_VARYING_VECTORS));
                v2 = vsubq_f32(v1, v0);
                o = vmlaq_f32(v0, p, v2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings + 3), o);

            case 3:
                v0 = vld1q_f32(reinterpret_cast<const float*>(inVaryings + 2));
                v1 = vld1q_f32(reinterpret_cast<const float*>(inVaryings + 2 + SL_SHADER_MAX_VARYING_VECTORS));
                v2 = vsubq_f32(v1, v0);
                o = vmlaq_f32(v0, p, v2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings + 2), o);

            case 2:
                v0 = vld1q_f32(reinterpret_cast<const float*>(inVaryings + 1));
                v1 = vld1q_f32(reinterpret_cast<const float*>(inVaryings + 1 + SL_SHADER_MAX_VARYING_VECTORS));
                v2 = vsubq_f32(v1, v0);
                o = vmlaq_f32(v0, p, v2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings + 1), o);

            case 1:
                v0 = vld1q_f32(reinterpret_cast<const float*>(inVaryings + 0));
                v1 = vld1q_f32(reinterpret_cast<const float*>(inVaryings + 0 + SL_SHADER_MAX_VARYING_VECTORS));
                v2 = vsubq_f32(v1, v0);
                o = vmlaq_f32(v0, p, v2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings + 0), o);
        }

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



/*--------------------------------------
 * Interpolate varying variables across a triangle
--------------------------------------*/
inline void LS_IMPERATIVE interpolate_tri_varyings(
    const float*      LS_RESTRICT_PTR baryCoords,
    uint_fast32_t     numVaryings,
    const math::vec4* inVaryings0,
    math::vec4*       outVaryings) noexcept
{
    static_assert(SL_SHADER_MAX_VARYING_VECTORS == 4, "Please update the varying interpolator.");

    #if defined(LS_X86_AVX2)
        (void)numVaryings;

        const float* LS_RESTRICT_PTR i0 = reinterpret_cast<const float*>(inVaryings0);
        const float* LS_RESTRICT_PTR i1 = reinterpret_cast<const float*>(inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS);
        const float* LS_RESTRICT_PTR i2 = reinterpret_cast<const float*>(inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS * 2);
        float* const LS_RESTRICT_PTR o = reinterpret_cast<float*>(outVaryings);

        __m256 a, c, v0, v2;

        a = _mm256_load_ps(i0 + 0);
        c = _mm256_load_ps(i0 + 8);
        const __m256 bc0 = _mm256_broadcast_ss(baryCoords+0);
        v0 = _mm256_mul_ps(bc0, a);
        v2 = _mm256_mul_ps(bc0, c);

        a = _mm256_load_ps(i1 + 0);
        c = _mm256_load_ps(i1 + 8);
        const __m256 bc1 = _mm256_broadcast_ss(baryCoords+1);
        v0 = _mm256_fmadd_ps(bc1, a, v0);
        v2 = _mm256_fmadd_ps(bc1, c, v2);

        a = _mm256_load_ps(i2 + 0);
        c = _mm256_load_ps(i2 + 8);
        const __m256 bc2 = _mm256_broadcast_ss(baryCoords+2);
        v0 = _mm256_fmadd_ps(bc2, a, v0);
        v2 = _mm256_fmadd_ps(bc2, c, v2);

        _mm256_store_ps(o + 0,  v0);
        _mm256_store_ps(o + 8,  v2);
        _mm256_zeroupper();

    #elif defined(LS_X86_SSE)
        (void)numVaryings;

        const float* LS_RESTRICT_PTR i0 = reinterpret_cast<const float*>(inVaryings0);
        const float* LS_RESTRICT_PTR i1 = reinterpret_cast<const float*>(inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS);
        const float* LS_RESTRICT_PTR i2 = reinterpret_cast<const float*>(inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS * 2);
        float* const LS_RESTRICT_PTR o = reinterpret_cast<float*>(outVaryings);

        __m128 a, b, c, d, v0, v1, v2, v3;

        a = _mm_load_ps(i0 + 0);
        b = _mm_load_ps(i0 + 4);
        c = _mm_load_ps(i0 + 8);
        d = _mm_load_ps(i0 + 12);
        const __m128 bc0 = _mm_load1_ps(baryCoords+0);
        v0 = _mm_mul_ps(bc0, a);
        v1 = _mm_mul_ps(bc0, b);
        v2 = _mm_mul_ps(bc0, c);
        v3 = _mm_mul_ps(bc0, d);

        a = _mm_load_ps(i1 + 0);
        b = _mm_load_ps(i1 + 4);
        c = _mm_load_ps(i1 + 8);
        d = _mm_load_ps(i1 + 12);
        const __m128 bc1 = _mm_load1_ps(baryCoords+1);
        v0 = _mm_add_ps(_mm_mul_ps(bc1, a), v0);
        v1 = _mm_add_ps(_mm_mul_ps(bc1, b), v1);
        v2 = _mm_add_ps(_mm_mul_ps(bc1, c), v2);
        v3 = _mm_add_ps(_mm_mul_ps(bc1, d), v3);

        a = _mm_load_ps(i2 + 0);
        b = _mm_load_ps(i2 + 4);
        c = _mm_load_ps(i2 + 8);
        d = _mm_load_ps(i2 + 12);
        const __m128 bc2 = _mm_load1_ps(baryCoords+2);
        v0 = _mm_add_ps(_mm_mul_ps(bc2, a), v0);
        v1 = _mm_add_ps(_mm_mul_ps(bc2, b), v1);
        v2 = _mm_add_ps(_mm_mul_ps(bc2, c), v2);
        v3 = _mm_add_ps(_mm_mul_ps(bc2, d), v3);

        _mm_store_ps(o + 0,  v0);
        _mm_store_ps(o + 4,  v1);
        _mm_store_ps(o + 8,  v2);
        _mm_store_ps(o + 12, v3);

    #elif defined(LS_ARCH_AARCH64)
        const math::vec4* LS_RESTRICT_PTR inVaryings1 = inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* LS_RESTRICT_PTR inVaryings2 = inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS * 2;

        const float32x4_t bc  = vld1q_f32(baryCoords);
        float32x4_t v0, v1, v2;

        float* const LS_RESTRICT_PTR o = reinterpret_cast<float*>(outVaryings);

        switch (numVaryings)
        {
            case 4:
                v0 = vmulq_laneq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+3)), bc, 0);
                v1 = vfmaq_laneq_f32(v0, vld1q_f32(reinterpret_cast<const float*>(inVaryings1+3)), bc, 1);
                v2 = vfmaq_laneq_f32(v1, vld1q_f32(reinterpret_cast<const float*>(inVaryings2+3)), bc, 2);
                vst1q_f32(reinterpret_cast<float*>(o+12), v2);

            case 3:
                v0 = vmulq_laneq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+2)), bc, 0);
                v1 = vfmaq_laneq_f32(v0, vld1q_f32(reinterpret_cast<const float*>(inVaryings1+2)), bc, 1);
                v2 = vfmaq_laneq_f32(v1, vld1q_f32(reinterpret_cast<const float*>(inVaryings2+2)), bc, 2);
                vst1q_f32(reinterpret_cast<float*>(o+8), v2);

            case 2:
                v0 = vmulq_laneq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+1)), bc, 0);
                v1 = vfmaq_laneq_f32(v0, vld1q_f32(reinterpret_cast<const float*>(inVaryings1+1)), bc, 1);
                v2 = vfmaq_laneq_f32(v1, vld1q_f32(reinterpret_cast<const float*>(inVaryings2+1)), bc, 2);
                vst1q_f32(reinterpret_cast<float*>(o+4), v2);

            case 1:
                v0 = vmulq_laneq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0)), bc, 0);
                v1 = vfmaq_laneq_f32(v0, vld1q_f32(reinterpret_cast<const float*>(inVaryings1)), bc, 1);
                v2 = vfmaq_laneq_f32(v1, vld1q_f32(reinterpret_cast<const float*>(inVaryings2)), bc, 2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings), v2);
        }

    #elif defined(LS_ARM_NEON)
        const math::vec4* LS_RESTRICT_PTR inVaryings1 = inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* LS_RESTRICT_PTR inVaryings2 = inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS * 2;

        const float32x4_t bc  = vld1q_f32(baryCoords);
        const float32x4_t bc0 = vdupq_n_f32(vgetq_lane_f32(bc, 0));
        const float32x4_t bc1 = vdupq_n_f32(vgetq_lane_f32(bc, 1));
        const float32x4_t bc2 = vdupq_n_f32(vgetq_lane_f32(bc, 2));
        float32x4_t v0, v1, v2;

        float* const LS_RESTRICT_PTR o = reinterpret_cast<float*>(outVaryings);

        switch (numVaryings)
        {
            case 4:
                v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+3)), bc0);
                v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1+3)), bc1);
                v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2+3)), bc2);
                vst1q_f32(reinterpret_cast<float*>(o+12), vaddq_f32(v2, vaddq_f32(v1, v0)));

            case 3:
                v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+2)), bc0);
                v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1+2)), bc1);
                v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2+2)), bc2);
                vst1q_f32(reinterpret_cast<float*>(o+8), vaddq_f32(v2, vaddq_f32(v1, v0)));

            case 2:
                v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+1)), bc0);
                v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1+1)), bc1);
                v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2+1)), bc2);
                vst1q_f32(reinterpret_cast<float*>(o+4), vaddq_f32(v2, vaddq_f32(v1, v0)));

            case 1:
                v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0)), bc0);
                v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1)), bc1);
                v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2)), bc2);
                vst1q_f32(reinterpret_cast<float*>(o), vaddq_f32(v2, vaddq_f32(v1, v0)));
        }

    #else
        const math::vec4* LS_RESTRICT_PTR inVaryings1 = inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* LS_RESTRICT_PTR inVaryings2 = inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS * 2;

        const float bc0 = baryCoords[0];
        const float bc1 = baryCoords[1];
        const float bc2 = baryCoords[2];

        for (uint_fast32_t i = numVaryings; i--;)
        {
            const math::vec4&& v0 = (*inVaryings0++) * bc0;
            const math::vec4&& v1 = (*inVaryings1++) * bc1;
            const math::vec4&& v2 = (*inVaryings2++) * bc2;
            (*outVaryings++) = v0+v1+v2;
        }
    #endif
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_FragmentProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Bin-Rasterization
--------------------------------------*/
template <typename depth_type>
void SL_FragmentProcessor::flush_line_fragments(
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
    SL_FboOutputFunctions&  fboOutFuncs   = *mFragFuncs;
    SL_TextureView* const   pColorBufs    = fboOutFuncs.pColorAttachments;
    SL_TextureView&         pDepthBuf     = *fboOutFuncs.pDepthAttachment;
    SL_FragmentParam        fragParams;

    fragParams.pUniforms = pUniforms;

    for (uint_fast32_t i = 0; i < numQueuedFrags; ++i)
    {
        const float interp = outCoords->lineInterp[i];

        interpolate_line_varyings(interp, numVaryings, bin.mVaryings, fragParams.pVaryings);

        fragParams.coord = outCoords->coord[i];

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

        if (LS_LIKELY(haveDepthMask))
        {
            ((depth_type*)pDepthBuf.pTexels)[fragParams.coord.x + pDepthBuf.width * fragParams.coord.y] = (depth_type)fragParams.coord.depth;
        }
    }
}



template void SL_FragmentProcessor::flush_line_fragments<ls::math::half>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
template void SL_FragmentProcessor::flush_line_fragments<float>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
template void SL_FragmentProcessor::flush_line_fragments<double>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;



/*--------------------------------------
 * Bin-Rasterization
--------------------------------------*/
template <typename depth_type>
void SL_FragmentProcessor::flush_tri_fragments(
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
    SL_FboOutputFunctions&  fboOutFuncs   = *mFragFuncs;
    SL_TextureView* const   pColorBufs    = fboOutFuncs.pColorAttachments;
    SL_TextureView&         pDepthBuf     = *fboOutFuncs.pDepthAttachment;
    SL_FragmentParam        fragParams;

    fragParams.pUniforms = pUniforms;
    const math::vec4* pPoints = bin.mScreenCoords;

    // perspective correction
    #if defined(LS_X86_AVX)
        #if defined(LS_X86_AVX2)
            const __m256 mask       = _mm256_castsi256_ps(_mm256_set_epi32(0, -1, -1, -1, 0, -1, -1, -1));
            const __m256i idx       = _mm256_set_epi32(-1, 11, 7, 3, -1, 11, 7, 3);
            const __m256 homogenous = _mm256_mask_i32gather_ps(_mm256_setzero_ps(), reinterpret_cast<const float*>(pPoints), idx, mask, sizeof(float));
        #else
            const __m256 homogenous = _mm256_set_ps(0.f, pPoints[2][3], pPoints[1][3], pPoints[0][3], 0.f, pPoints[2][3], pPoints[1][3], pPoints[0][3]);
        #endif

        for (uint_fast32_t i = 0; i < numQueuedFrags; i += 2)
        {
            float* const pBc = reinterpret_cast<float*>(outCoords->bc + i);
            const __m256 bc = _mm256_mul_ps(_mm256_load_ps(pBc), homogenous);

            // horizontal add
            const __m256 a     = _mm256_permute_ps(bc, 0xB1);
            const __m256 b     = _mm256_add_ps(bc, a);
            const __m256 c     = _mm256_permute_ps(b, 0x0F);
            const __m256 d     = _mm256_add_ps(c, b);
            const __m256 persp = _mm256_rcp_ps(d);

            _mm256_store_ps(pBc, _mm256_mul_ps(bc, persp));
        }

    #elif defined(LS_ARM_NEON)
        const float32x4_t homogenous = vld4q_f32(reinterpret_cast<const float*>(pPoints)).val[3];

        for (uint_fast32_t i = 0; i < numQueuedFrags; ++i)
        {
            float* const pBc = reinterpret_cast<float*>(outCoords->bc + i);
            const float32x4_t bc = vmulq_f32(vld1q_f32(pBc), homogenous);

            // horizontal add
            #if defined(LS_ARCH_AARCH64)
                const float32x4_t a = vdupq_n_f32(vaddvq_f32(bc));
                vst1q_f32(pBc, vdivq_f32(bc, a));
            #else
                const float32x4_t a     = vrev64q_f32(bc);
                const float32x4_t b     = vaddq_f32(bc, a);
                const float32x2_t c     = vdup_lane_f32(vget_high_f32(b), 3);
                const float32x2_t d     = vadd_f32(vget_low_f32(b), c);
                const float32x4_t e     = vdupq_lane_f32(d, 0);
                const float32x4_t f     = vrecpeq_f32(e);
                const float32x4_t persp = vmulq_f32(vrecpsq_f32(e, f), f);
                vst1q_f32(pBc, vmulq_f32(bc, persp));
            #endif
        }

    #else
        const math::vec4 homogenous{pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
        for (uint_fast32_t i = 0; i < numQueuedFrags; ++i)
        {
            const math::vec4&& bc = outCoords->bc[i] * homogenous;
            const math::vec4&& persp = {math::sum_inv(bc)};
            outCoords->bc[i] = bc * persp;
        }
    #endif

    for (uint_fast32_t i = 0; i < numQueuedFrags; ++i)
    {
        interpolate_tri_varyings(&outCoords->bc[i], numVaryings, bin.mVaryings, fragParams.pVaryings);
        fragParams.coord = outCoords->coord[i];

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

        if (LS_LIKELY(haveDepthMask))
        {
            ((depth_type*)pDepthBuf.pTexels)[fragParams.coord.x + pDepthBuf.width * fragParams.coord.y] = (depth_type)fragParams.coord.depth;
        }
    }
}



template void SL_FragmentProcessor::flush_tri_fragments<ls::math::half>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
template void SL_FragmentProcessor::flush_tri_fragments<float>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
template void SL_FragmentProcessor::flush_tri_fragments<double>(const SL_FragmentBin&, uint_fast32_t, SL_FragCoord* const) const noexcept;
