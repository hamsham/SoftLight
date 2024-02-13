/*
 * File:   SL_PostVertexTransform.hpp
 * Author: hammy
 * Created on February 13, 2024, at 11:31 AM
 */

#ifndef SL_POST_VERTEX_TRANSFORM_HPP
#define SL_POST_VERTEX_TRANSFORM_HPP

#include "lightsky/math/vec_utils.h"



/*-----------------------------------------------------------------------------
 * Enums
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Enum to help with determining face visibility
-------------------------------------*/
enum SL_ClipStatus
{
    SL_CLIP_STATUS_NOT_VISIBLE       = 0x00,
    SL_CLIP_STATUS_PARTIALLY_VISIBLE = 0x01,
    SL_CLIP_STATUS_FULLY_VISIBLE     = 0x03,
};



/*-----------------------------------------------------------------------------
 * Perspective Division
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Point Perspective Division
--------------------------------------*/
inline LS_INLINE void sl_perspective_divide(ls::math::vec4& LS_RESTRICT_PTR v0) noexcept
{
    namespace math = ls::math;

    #if defined(LS_X86_SSE4_1)
        const __m128 wInv0 = _mm_rcp_ps(_mm_shuffle_ps(v0.simd, v0.simd, 0xFF));
        const __m128 vMul0 = _mm_mul_ps(v0.simd, wInv0);
        v0.simd = _mm_blend_ps(wInv0, vMul0, 0x07);

    #elif defined(LS_X86_SSSE3)
        const __m128 wInv0 = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v0.simd), 0xFF)));
        const __m128 vMul0 = _mm_mul_ps(v0.simd, wInv0);
        const __m128i wi0  = _mm_shuffle_epi8(_mm_castps_si128(wInv0), _mm_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
        const __m128i vi0  = _mm_shuffle_epi8(_mm_castps_si128(vMul0), _mm_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
        v0.simd = _mm_or_ps(_mm_castsi128_ps(vi0), _mm_castsi128_ps(wi0));

    #elif defined(LS_ARCH_AARCH64)
        //const uint32x4_t blendMask{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
        const uint32x4_t blendMask = vsetq_lane_u32(0, vdupq_n_u32(0xFFFFFFFF), 3);

        const float32x4_t w0 = vdupq_n_f32(vgetq_lane_f32(v0.simd, 3));
        const float32x4_t wInv0 = vdivq_f32(vdupq_n_f32(1.f), w0);
        const float32x4_t vMul0 = vmulq_f32(v0.simd, wInv0);
        v0.simd = vbslq_f32(blendMask, vMul0, wInv0);

    #elif defined(LS_ARM_NEON)
        //const uint32x4_t blendMask{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
        const uint32x4_t blendMask = vsetq_lane_u32(0, vdupq_n_u32(0xFFFFFFFF), 3);

        const float32x4_t w0 = vdupq_n_f32(vgetq_lane_f32(v0.simd, 3));
        const float32x4_t wInv0 = vrecpeq_f32(w0);
        const float32x4_t vMul0 = vmulq_f32(v0.simd, vmulq_f32(vrecpsq_f32(w0, wInv0), wInv0));
        v0.simd = vbslq_f32(blendMask, vMul0, wInv0);

    #else
        const math::vec4&& wInv0 = math::rcp(v0[3]);
        const math::vec4&& vMul0 = v0 * wInv0;
        v0 = {vMul0[0], vMul0[1], vMul0[2], wInv0[0]};

    #endif
}



/*--------------------------------------
 * Line Perspective Division
--------------------------------------*/
inline LS_INLINE void sl_perspective_divide(
    ls::math::vec4& LS_RESTRICT_PTR v0,
    ls::math::vec4& LS_RESTRICT_PTR v1) noexcept
{
    namespace math = ls::math;

    #if defined(LS_X86_SSE4_1)
        const __m128 wInv0 = _mm_rcp_ps(_mm_shuffle_ps(v0.simd, v0.simd, 0xFF));
        const __m128 wInv1 = _mm_rcp_ps(_mm_shuffle_ps(v1.simd, v1.simd, 0xFF));

        const __m128 vMul0 = _mm_mul_ps(v0.simd, wInv0);
        const __m128 vMul1 = _mm_mul_ps(v1.simd, wInv1);

        v0.simd = _mm_blend_ps(wInv0, vMul0, 0x07);
        v1.simd = _mm_blend_ps(wInv1, vMul1, 0x07);

    #elif defined(LS_X86_SSSE3)
        const __m128 wInv0 = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v0.simd), 0xFF)));
        const __m128 wInv1 = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v1.simd), 0xFF)));

        const __m128 vMul0 = _mm_mul_ps(v0.simd, wInv0);
        const __m128 vMul1 = _mm_mul_ps(v1.simd, wInv1);

        const __m128i wi0  = _mm_shuffle_epi8(_mm_castps_si128(wInv0), _mm_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
        const __m128i wi1  = _mm_shuffle_epi8(_mm_castps_si128(wInv1), _mm_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));

        const __m128i vi0  = _mm_shuffle_epi8(_mm_castps_si128(vMul0), _mm_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
        const __m128i vi1  = _mm_shuffle_epi8(_mm_castps_si128(vMul1), _mm_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

        v0.simd = _mm_or_ps(_mm_castsi128_ps(vi0), _mm_castsi128_ps(wi0));
        v1.simd = _mm_or_ps(_mm_castsi128_ps(vi1), _mm_castsi128_ps(wi1));

    #elif defined(LS_ARCH_AARCH64)
        //const uint32x4_t blendMask{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
        const uint32x4_t blendMask = vsetq_lane_u32(0, vdupq_n_u32(0xFFFFFFFF), 3);

        const float32x4_t w0 = vdupq_n_f32(vgetq_lane_f32(v0.simd, 3));
        const float32x4_t wInv0 = vdivq_f32(vdupq_n_f32(1.f), w0);
        const float32x4_t vMul0 = vmulq_f32(v0.simd, wInv0);
        v0.simd = vbslq_f32(blendMask, vMul0, wInv0);

        const float32x4_t w1 = vdupq_n_f32(vgetq_lane_f32(v1.simd, 3));
        const float32x4_t wInv1 = vdivq_f32(vdupq_n_f32(1.f), w1);
        const float32x4_t vMul1 = vmulq_f32(v1.simd, wInv1);
        v1.simd = vbslq_f32(blendMask, vMul1, wInv1);

    #elif defined(LS_ARM_NEON)
        //const uint32x4_t blendMask{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
        const uint32x4_t blendMask = vsetq_lane_u32(0, vdupq_n_u32(0xFFFFFFFF), 3);

        const float32x4_t w0 = vdupq_n_f32(vgetq_lane_f32(v0.simd, 3));
        const float32x4_t wInv0 = vrecpeq_f32(w0);
        const float32x4_t vMul0 = vmulq_f32(v0.simd, vmulq_f32(vrecpsq_f32(w0, wInv0), wInv0));
        v0.simd = vbslq_f32(blendMask, vMul0, wInv0);

        const float32x4_t w1 = vdupq_n_f32(vgetq_lane_f32(v1.simd, 3));
        const float32x4_t wInv1 = vrecpeq_f32(w1);
        const float32x4_t vMul1 = vmulq_f32(v1.simd, vmulq_f32(vrecpsq_f32(w1, wInv1), wInv1));
        v1.simd = vbslq_f32(blendMask, vMul1, wInv1);

    #else
        const math::vec4&& wInv0 = math::rcp(v0[3]);
        const math::vec4&& wInv1 = math::rcp(v1[3]);

        const math::vec4&& vMul0 = v0 * wInv0;
        const math::vec4&& vMul1 = v1 * wInv1;

        v0 = {vMul0[0], vMul0[1], vMul0[2], wInv0[0]};
        v1 = {vMul1[0], vMul1[1], vMul1[2], wInv1[0]};

    #endif
}



/*--------------------------------------
 * Triangle Perspective Division
--------------------------------------*/
inline LS_INLINE void sl_perspective_divide(
    ls::math::vec4& LS_RESTRICT_PTR v0,
    ls::math::vec4& LS_RESTRICT_PTR v1,
    ls::math::vec4& LS_RESTRICT_PTR v2) noexcept
{
    namespace math = ls::math;

    #if defined(LS_X86_SSE4_1)
        const __m128 wInv0 = _mm_rcp_ps(_mm_shuffle_ps(v0.simd, v0.simd, 0xFF));
        const __m128 wInv1 = _mm_rcp_ps(_mm_shuffle_ps(v1.simd, v1.simd, 0xFF));
        const __m128 wInv2 = _mm_rcp_ps(_mm_shuffle_ps(v2.simd, v2.simd, 0xFF));

        const __m128 vMul0 = _mm_mul_ps(v0.simd, wInv0);
        const __m128 vMul1 = _mm_mul_ps(v1.simd, wInv1);
        const __m128 vMul2 = _mm_mul_ps(v2.simd, wInv2);

        v0.simd = _mm_blend_ps(wInv0, vMul0, 0x07);
        v1.simd = _mm_blend_ps(wInv1, vMul1, 0x07);
        v2.simd = _mm_blend_ps(wInv2, vMul2, 0x07);

    #elif defined(LS_X86_SSSE3)
        const __m128 wInv0 = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v0.simd), 0xFF)));
        const __m128 wInv1 = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v1.simd), 0xFF)));
        const __m128 wInv2 = _mm_rcp_ps(_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v2.simd), 0xFF)));

        const __m128 vMul0 = _mm_mul_ps(v0.simd, wInv0);
        const __m128 vMul1 = _mm_mul_ps(v1.simd, wInv1);
        const __m128 vMul2 = _mm_mul_ps(v2.simd, wInv2);

        const __m128i wi0  = _mm_shuffle_epi8(_mm_castps_si128(wInv0), _mm_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
        const __m128i wi1  = _mm_shuffle_epi8(_mm_castps_si128(wInv1), _mm_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
        const __m128i wi2  = _mm_shuffle_epi8(_mm_castps_si128(wInv2), _mm_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));

        const __m128i vi0  = _mm_shuffle_epi8(_mm_castps_si128(vMul0), _mm_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
        const __m128i vi1  = _mm_shuffle_epi8(_mm_castps_si128(vMul1), _mm_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
        const __m128i vi2  = _mm_shuffle_epi8(_mm_castps_si128(vMul2), _mm_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

        v0.simd = _mm_or_ps(_mm_castsi128_ps(vi0), _mm_castsi128_ps(wi0));
        v1.simd = _mm_or_ps(_mm_castsi128_ps(vi1), _mm_castsi128_ps(wi1));
        v2.simd = _mm_or_ps(_mm_castsi128_ps(vi2), _mm_castsi128_ps(wi2));

    #elif defined(LS_ARCH_AARCH64)
        //const uint32x4_t blendMask{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
        const uint32x4_t blendMask = vsetq_lane_u32(0, vdupq_n_u32(0xFFFFFFFF), 3);

        const float32x4_t w0 = vdupq_n_f32(vgetq_lane_f32(v0.simd, 3));
        const float32x4_t wInv0 = vdivq_f32(vdupq_n_f32(1.f), w0);
        const float32x4_t vMul0 = vmulq_f32(v0.simd, wInv0);
        v0.simd = vbslq_f32(blendMask, vMul0, wInv0);

        const float32x4_t w1 = vdupq_n_f32(vgetq_lane_f32(v1.simd, 3));
        const float32x4_t wInv1 = vdivq_f32(vdupq_n_f32(1.f), w1);
        const float32x4_t vMul1 = vmulq_f32(v1.simd, wInv1);
        v1.simd = vbslq_f32(blendMask, vMul1, wInv1);

        const float32x4_t w2 = vdupq_n_f32(vgetq_lane_f32(v2.simd, 3));
        const float32x4_t wInv2 = vdivq_f32(vdupq_n_f32(1.f), w2);
        const float32x4_t vMul2 = vmulq_f32(v2.simd, wInv2);
        v2.simd = vbslq_f32(blendMask, vMul2, wInv2);

    #elif defined(LS_ARM_NEON)
        //const uint32x4_t blendMask{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
        const uint32x4_t blendMask = vsetq_lane_u32(0, vdupq_n_u32(0xFFFFFFFF), 3);

        const float32x4_t w0 = vdupq_n_f32(vgetq_lane_f32(v0.simd, 3));
        const float32x4_t wInv0 = vrecpeq_f32(w0);
        const float32x4_t vMul0 = vmulq_f32(v0.simd, vmulq_f32(vrecpsq_f32(w0, wInv0), wInv0));
        v0.simd = vbslq_f32(blendMask, vMul0, wInv0);

        const float32x4_t w1 = vdupq_n_f32(vgetq_lane_f32(v1.simd, 3));
        const float32x4_t wInv1 = vrecpeq_f32(w1);
        const float32x4_t vMul1 = vmulq_f32(v1.simd, vmulq_f32(vrecpsq_f32(w1, wInv1), wInv1));
        v1.simd = vbslq_f32(blendMask, vMul1, wInv1);

        const float32x4_t w2 = vdupq_n_f32(vgetq_lane_f32(v2.simd, 3));
        const float32x4_t wInv2 = vrecpeq_f32(w2);
        const float32x4_t vMul2 = vmulq_f32(v2.simd, vmulq_f32(vrecpsq_f32(w2, wInv2), wInv2));
        v2.simd = vbslq_f32(blendMask, vMul2, wInv2);

    #else
        const math::vec4&& wInv0 = math::rcp(v0[3]);
        const math::vec4&& wInv1 = math::rcp(v1[3]);
        const math::vec4&& wInv2 = math::rcp(v2[3]);
        const math::vec4&& vMul0 = v0 * wInv0;
        const math::vec4&& vMul1 = v1 * wInv1;
        const math::vec4&& vMul2 = v2 * wInv2;
        v0 = {vMul0[0], vMul0[1], vMul0[2], wInv0[0]};
        v1 = {vMul1[0], vMul1[1], vMul1[2], wInv1[0]};
        v2 = {vMul2[0], vMul2[1], vMul2[2], wInv2[0]};

    #endif
}



/*-----------------------------------------------------------------------------
 * Convert world coordinates to screen coordinates
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * 1-Element NDC-to-Screen Space
--------------------------------------*/
inline LS_INLINE void sl_ndc_to_screen_coords(
    ls::math::vec4& LS_RESTRICT_PTR p0,
    const ls::math::vec4& viewportDims) noexcept
{
    namespace math = ls::math;

    #if defined(LS_X86_FMA)
        const __m128 dims = _mm_load_ps(&viewportDims);
        const __m128 halfDims = _mm_mul_ps(dims, _mm_set1_ps(0.5f));
        const __m128 one = _mm_set1_ps(1.f);
        const __m128 wh = _mm_shuffle_ps(halfDims, one, 0x0E);

        __m128 v0 = _mm_add_ps(one, p0.simd);
        v0 = _mm_floor_ps(_mm_fmadd_ps(v0, wh, dims));
        p0.simd = _mm_shuffle_ps(_mm_max_ps(v0, _mm_setzero_ps()), p0.simd, 0xE4);

    #elif defined(LS_X86_SSE)
        const __m128 dims = _mm_load_ps(&viewportDims);
        const __m128 halfDims = _mm_mul_ps(dims, _mm_set1_ps(0.5f));
        const __m128 one = _mm_set1_ps(1.f);
        const __m128 wh = _mm_shuffle_ps(halfDims, one, 0x0E);

        __m128 v0 = _mm_mul_ps(_mm_add_ps(one, p0.simd), wh);
        v0 = _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_add_ps(v0, dims)));
        p0.simd = _mm_shuffle_ps(_mm_max_ps(v0, _mm_setzero_ps()), p0.simd, 0xE4);

    #elif defined(LS_ARCH_AARCH64)
        const float32x4_t dims = vld1q_f32(&viewportDims);
        const float32x2_t offset = vget_low_f32(dims);
        const float32x2_t wh = vmul_f32(vdup_n_f32(0.5f), vget_high_f32(dims));
        const float32x2_t one = vdup_n_f32(1.f);

        float32x2_t v0 = vadd_f32(vld1_f32(&p0), one);
        v0 = vrndm_f32(vmla_f32(offset, wh, v0));
        vst1_f32(&p0, vmax_f32(v0, vdup_n_f32(0.f)));

    #elif defined(LS_ARM_NEON)
        const float32x4_t dims = vld1q_f32(&viewportDims);
        const float32x2_t offset = vget_low_f32(dims);
        const float32x2_t wh = vmul_f32(vdup_n_f32(0.5f), vget_high_f32(dims));
        const float32x2_t one = vdup_n_f32(1.f);

        float32x2_t v0 = vadd_f32(vld1_f32(&p0), one);
        v0 = vcvt_f32_s32(vcvt_s32_f32(vmla_f32(offset, wh, v0)));
        vst1_f32(&p0, vmax_f32(v0, vdup_n_f32(0.f)));

    #else
        // NDC->screen space transform is clamped to the framebuffer bounds.
        // For example:
        // x = math::clamp(x, 0.f, fboW);
        // w = min(w, fboW-x);
        const float w = viewportDims[2] * 0.5f;
        const float h = viewportDims[3] * 0.5f;

        p0[0] = math::max(math::floor(math::fmadd(p0[0]+1.f, w, viewportDims[0])), 0.f);
        p0[1] = math::max(math::floor(math::fmadd(p0[1]+1.f, h, viewportDims[1])), 0.f);

    #endif
}



/*--------------------------------------
 * 2-Element NDC-to-Screen Space
--------------------------------------*/
inline LS_INLINE void sl_ndc_to_screen_coords(
    ls::math::vec4& LS_RESTRICT_PTR p0,
    ls::math::vec4& LS_RESTRICT_PTR p1,
    const ls::math::vec4& viewportDims
) noexcept
{
    namespace math = ls::math;

    #if defined(LS_X86_FMA)
        const __m128 dims = _mm_load_ps(&viewportDims);
        const __m128 halfDims = _mm_mul_ps(dims, _mm_set1_ps(0.5f));
        const __m128 one = _mm_set1_ps(1.f);
        const __m128 wh = _mm_shuffle_ps(halfDims, one, 0x0E);

        __m128 v0 = _mm_add_ps(one, p0.simd);
        v0 = _mm_floor_ps(_mm_fmadd_ps(v0, wh, dims));
        p0.simd = _mm_shuffle_ps(_mm_max_ps(v0, _mm_setzero_ps()), p0.simd, 0xE4);

        __m128 v1 = _mm_add_ps(one, p1.simd);
        v1 = _mm_floor_ps(_mm_fmadd_ps(v1, wh, dims));
        p1.simd = _mm_shuffle_ps(_mm_max_ps(v1, _mm_setzero_ps()), p1.simd, 0xE4);

    #elif defined(LS_X86_SSE)
        const __m128 dims = _mm_load_ps(&viewportDims);
        const __m128 halfDims = _mm_mul_ps(dims, _mm_set1_ps(0.5f));
        const __m128 one = _mm_set1_ps(1.f);
        const __m128 wh = _mm_shuffle_ps(halfDims, one, 0x0E);

        __m128 v0 = _mm_mul_ps(_mm_add_ps(one, p0.simd), wh);
        v0 = _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_add_ps(v0, dims)));
        p0.simd = _mm_shuffle_ps(_mm_max_ps(v0, _mm_setzero_ps()), p0.simd, 0xE4);

        __m128 v1 = _mm_mul_ps(_mm_add_ps(one, p1.simd), wh);
        v1 = _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_add_ps(v1, dims)));
        p1.simd = _mm_shuffle_ps(_mm_max_ps(v1, _mm_setzero_ps()), p1.simd, 0xE4);

    #elif defined(LS_ARCH_AARCH64)
        const float32x4_t dims = vld1q_f32(&viewportDims);
        const float32x2_t offset = vget_low_f32(dims);
        const float32x2_t wh = vmul_f32(vdup_n_f32(0.5f), vget_high_f32(dims));
        const float32x2_t one = vdup_n_f32(1.f);

        float32x2_t v0 = vadd_f32(vld1_f32(&p0), one);
        v0 = vrndm_f32(vmla_f32(offset, wh, v0));
        vst1_f32(&p0, vmax_f32(v0, vdup_n_f32(0.f)));

        float32x2_t v1 = vadd_f32(vld1_f32(&p1), one);
        v1 = vrndm_f32(vmla_f32(offset, wh, v1));
        vst1_f32(&p1, vmax_f32(v1, vdup_n_f32(0.f)));

    #elif defined(LS_ARM_NEON)
        const float32x4_t dims = vld1q_f32(&viewportDims);
        const float32x2_t offset = vget_low_f32(dims);
        const float32x2_t wh = vmul_f32(vdup_n_f32(0.5f), vget_high_f32(dims));
        const float32x2_t one = vdup_n_f32(1.f);

        float32x2_t v0 = vadd_f32(vld1_f32(&p0), one);
        v0 = vcvt_f32_s32(vcvt_s32_f32(vmla_f32(offset, wh, v0)));
        vst1_f32(&p0, vmax_f32(v0, vdup_n_f32(0.f)));

        float32x2_t v1 = vadd_f32(vld1_f32(&p1), one);
        v1 = vcvt_f32_s32(vcvt_s32_f32(vmla_f32(offset, wh, v1)));
        vst1_f32(&p1, vmax_f32(v1, vdup_n_f32(0.f)));

    #else
        // NDC->screen space transform is clamped to the framebuffer bounds.
        // For example:
        // x = math::clamp(x, 0.f, fboW);
        // w = min(w, fboW-x);
        const float w = viewportDims[2] * 0.5f;
        const float h = viewportDims[3] * 0.5f;

        p0[0] = math::max(math::floor(math::fmadd(p0[0]+1.f, w, viewportDims[0])), 0.f);
        p0[1] = math::max(math::floor(math::fmadd(p0[1]+1.f, h, viewportDims[1])), 0.f);

        p1[0] = math::max(math::floor(math::fmadd(p1[0]+1.f, w, viewportDims[0])), 0.f);
        p1[1] = math::max(math::floor(math::fmadd(p1[1]+1.f, h, viewportDims[1])), 0.f);

    #endif
}



/*--------------------------------------
 * 3-Element NDC-to-Screen Space
--------------------------------------*/
inline LS_INLINE void sl_ndc_to_screen_coords(
    ls::math::vec4& LS_RESTRICT_PTR p0,
    ls::math::vec4& LS_RESTRICT_PTR p1,
    ls::math::vec4& LS_RESTRICT_PTR p2,
    const ls::math::vec4& viewportDims
) noexcept
{
    namespace math = ls::math;

    #if defined(LS_X86_FMA)
        const __m128 one = _mm_set1_ps(1.f);
        const __m128 dims = _mm_load_ps(&viewportDims);
        const __m128 halfDims = _mm_mul_ps(dims, _mm_set1_ps(0.5f));
        const __m128 wh = _mm_shuffle_ps(halfDims, one, 0x0E);

        __m128 v0 = _mm_add_ps(one, p0.simd);
        __m128 v1 = _mm_add_ps(one, p1.simd);
        __m128 v2 = _mm_add_ps(one, p2.simd);

        v0 = _mm_fmadd_ps(v0, wh, dims);
        v1 = _mm_fmadd_ps(v1, wh, dims);
        v2 = _mm_fmadd_ps(v2, wh, dims);

        v0 = _mm_floor_ps(v0);
        v1 = _mm_floor_ps(v1);
        v2 = _mm_floor_ps(v2);

        v0 = _mm_max_ps(v0, _mm_setzero_ps());
        v1 = _mm_max_ps(v1, _mm_setzero_ps());
        v2 = _mm_max_ps(v2, _mm_setzero_ps());

        v0 = _mm_blend_ps(p0.simd, v0, 0x03);
        v1 = _mm_blend_ps(p1.simd, v1, 0x03);
        v2 = _mm_blend_ps(p2.simd, v2, 0x03);

        _mm_store_ps(&p0, v0);
        _mm_store_ps(&p1, v1);
        _mm_store_ps(&p2, v2);

    #elif defined(LS_X86_SSE)
        const __m128 dims = _mm_load_ps(&viewportDims);
        const __m128 halfDims = _mm_mul_ps(dims, _mm_set1_ps(0.5f));
        const __m128 one = _mm_set1_ps(1.f);
        const __m128 wh = _mm_shuffle_ps(halfDims, one, 0x0E);

        __m128 v0 = _mm_mul_ps(_mm_add_ps(one, p0.simd), wh);
        v0 = _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_add_ps(v0, dims)));
        p0.simd = _mm_shuffle_ps(_mm_max_ps(v0, _mm_setzero_ps()), p0.simd, 0xE4);

        __m128 v1 = _mm_mul_ps(_mm_add_ps(one, p1.simd), wh);
        v1 = _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_add_ps(v1, dims)));
        p1.simd = _mm_shuffle_ps(_mm_max_ps(v1, _mm_setzero_ps()), p1.simd, 0xE4);

        __m128 v2 = _mm_mul_ps(_mm_add_ps(one, p2.simd), wh);
        v2 = _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_add_ps(v2, dims)));
        p2.simd = _mm_shuffle_ps(_mm_max_ps(v2, _mm_setzero_ps()), p2.simd, 0xE4);

    #elif defined(LS_ARCH_AARCH64)
        const float32x2_t one = vdup_n_f32(1.f);
        const float32x4_t dims = vld1q_f32(&viewportDims);
        const float32x2_t offset = vget_low_f32(dims);
        const float32x2_t wh = vmul_f32(vdup_n_f32(0.5f), vget_high_f32(dims));

        float32x2_t v0 = vadd_f32(vget_low_f32(p0.simd), one);
        v0 = vrndm_f32(vfma_f32(offset, wh, v0));
        v0 = vmax_f32(v0, vdup_n_f32(0.f));
        vst1_f32(&p0, v0);

        float32x2_t v1 = vadd_f32(vget_low_f32(p1.simd), one);
        v1 = vrndm_f32(vfma_f32(offset, wh, v1));
        v1 = vmax_f32(v1, vdup_n_f32(0.f));
        vst1_f32(&p1, v1);

        float32x2_t v2 = vadd_f32(vget_low_f32(p2.simd), one);
        v2 = vrndm_f32(vfma_f32(offset, wh, v2));
        v2 = vmax_f32(v2, vdup_n_f32(0.f));
        vst1_f32(&p2, v2);

    #elif defined(LS_ARM_NEON)
        const float32x4_t dims = vld1q_f32(&viewportDims);
        const float32x2_t offset = vget_low_f32(dims);
        const float32x2_t wh = vmul_f32(vdup_n_f32(0.5f), vget_high_f32(dims));
        const float32x2_t one = vdup_n_f32(1.f);

        float32x2_t v0 = vadd_f32(vld1_f32(&p0), one);
        v0 = vcvt_f32_s32(vcvt_s32_f32(vmla_f32(offset, wh, v0)));
        vst1_f32(&p0, vmax_f32(v0, vdup_n_f32(0.f)));

        float32x2_t v1 = vadd_f32(vld1_f32(&p1), one);
        v1 = vcvt_f32_s32(vcvt_s32_f32(vmla_f32(offset, wh, v1)));
        vst1_f32(&p1, vmax_f32(v1, vdup_n_f32(0.f)));

        float32x2_t v2 = vadd_f32(vld1_f32(&p2), one);
        v2 = vcvt_f32_s32(vcvt_s32_f32(vmla_f32(offset, wh, v2)));
        vst1_f32(&p2, vmax_f32(v2, vdup_n_f32(0.f)));

    #else
        // NDC->screen space transform is clamped to the framebuffer bounds.
        // For example:
        // x = math::clamp(x, 0.f, fboW);
        // w = min(w, fboW-x);
        const float w = viewportDims[2] * 0.5f;
        const float h = viewportDims[3] * 0.5f;

        p0[0] = math::max(math::floor(math::fmadd(p0[0]+1.f, w, viewportDims[0])), 0.f);
        p0[1] = math::max(math::floor(math::fmadd(p0[1]+1.f, h, viewportDims[1])), 0.f);

        p1[0] = math::max(math::floor(math::fmadd(p1[0]+1.f, w, viewportDims[0])), 0.f);
        p1[1] = math::max(math::floor(math::fmadd(p1[1]+1.f, h, viewportDims[1])), 0.f);

        p2[0] = math::max(math::floor(math::fmadd(p2[0]+1.f, w, viewportDims[0])), 0.f);
        p2[1] = math::max(math::floor(math::fmadd(p2[1]+1.f, h, viewportDims[1])), 0.f);

    #endif
}



/*-----------------------------------------------------------------------------
 * Determine Primitive Visibility
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Determine Line Clipping
--------------------------------------*/
inline LS_INLINE SL_ClipStatus sl_ndc_clip_status(
    const ls::math::vec4& LS_RESTRICT_PTR clip
) noexcept
{
    namespace math = ls::math;

    const float w0p = clip[3];
    const float w0n = -clip[3];

    SL_ClipStatus status = SL_CLIP_STATUS_NOT_VISIBLE;

    if (w0p > 0.f && clip <= w0p && clip >= w0n)
    {
        status = SL_CLIP_STATUS_FULLY_VISIBLE;
    }

    return status;
}



/*--------------------------------------
 * Determine Line Clipping
--------------------------------------*/
inline LS_INLINE SL_ClipStatus sl_ndc_clip_status(
    const ls::math::vec4& LS_RESTRICT_PTR clip0,
    const ls::math::vec4& LS_RESTRICT_PTR clip1
) noexcept
{
    namespace math = ls::math;

    const math::vec4 w0p = clip0[3];
    const math::vec4 w1p = clip1[3];

    const math::vec4 w1n = -clip1[3];
    const math::vec4 w0n = -clip0[3];

    int vis = SL_CLIP_STATUS_FULLY_VISIBLE & -(
        clip0 <= w0p &&
        clip1 <= w1p &&
        clip0 >= w0n &&
        clip1 >= w1n
    );

    int part = SL_CLIP_STATUS_PARTIALLY_VISIBLE & -(w0p > 0.f || w1p > 0.f);

    return (SL_ClipStatus)(vis | part);
}



/*--------------------------------------
 * Determine Line Clipping
--------------------------------------*/
inline LS_INLINE SL_ClipStatus sl_ndc_clip_status(
    const ls::math::vec4& LS_RESTRICT_PTR clip0,
    const ls::math::vec4& LS_RESTRICT_PTR clip1,
    const ls::math::vec4& LS_RESTRICT_PTR clip2
) noexcept
{
    namespace math = ls::math;

    #if defined(LS_X86_SSE)
        const __m128 w0p = _mm_shuffle_ps(clip0.simd, clip0.simd, 0xFF);
        const __m128 w1p = _mm_shuffle_ps(clip1.simd, clip1.simd, 0xFF);
        const __m128 w2p = _mm_shuffle_ps(clip2.simd, clip2.simd, 0xFF);

        const __m128 sign = _mm_set1_ps(-0.f);
        const __m128 ge0 = _mm_and_ps(_mm_cmple_ps(_mm_or_ps(w0p, sign), clip0.simd), _mm_cmpge_ps(w0p, clip0.simd));
        const __m128 ge1 = _mm_and_ps(_mm_cmple_ps(_mm_or_ps(w1p, sign), clip1.simd), _mm_cmpge_ps(w1p, clip1.simd));
        const __m128 ge2 = _mm_and_ps(_mm_cmple_ps(_mm_or_ps(w2p, sign), clip2.simd), _mm_cmpge_ps(w2p, clip2.simd));
        const __m128 vis = _mm_and_ps(_mm_and_ps(ge0, ge1), ge2);
        const int visI = SL_CLIP_STATUS_FULLY_VISIBLE & -(_mm_movemask_ps(vis) == 0x0F);

        const __m128 le0 = _mm_cmpgt_ps(w0p, _mm_setzero_ps());
        const __m128 le1 = _mm_cmpgt_ps(w1p, _mm_setzero_ps());
        const __m128 le2 = _mm_cmpgt_ps(w2p, _mm_setzero_ps());
        const __m128 part = _mm_or_ps(_mm_or_ps(le0, le1), le2);
        const int partI = SL_CLIP_STATUS_PARTIALLY_VISIBLE & -(_mm_movemask_ps(part) == 0x0F);

        return (SL_ClipStatus)(visI | partI);

    #elif defined(LS_ARM_NEON)
        #if defined(LS_ARCH_AARCH64)
            const float32x4_t w0p = vdupq_laneq_f32(clip0.simd, 3);
            const float32x4_t w1p = vdupq_laneq_f32(clip1.simd, 3);
            const float32x4_t w2p = vdupq_laneq_f32(clip2.simd, 3);
        #else
            const float32x4_t w0p = vdupq_lane_f32(vget_high_f32(clip0.simd), 1);
            const float32x4_t w1p = vdupq_lane_f32(vget_high_f32(clip1.simd), 1);
            const float32x4_t w2p = vdupq_lane_f32(vget_high_f32(clip2.simd), 1);
        #endif

        const uint32x4_t le0  = vcaleq_f32(clip0.simd, w0p);
        const uint32x4_t le1  = vcaleq_f32(clip1.simd, w1p);
        const uint32x4_t le2  = vcaleq_f32(clip2.simd, w2p);

        const uint32x4_t vis = vandq_u32(vandq_u32(le2, vandq_u32(le1, le0)), vdupq_n_u32(SL_CLIP_STATUS_FULLY_VISIBLE));
        const uint32x2_t vis2 = vand_u32(vget_low_u32(vis), vget_high_u32(vis));
        const unsigned   visI = vget_lane_u32(vand_u32(vis2, vrev64_u32(vis2)), 0);

        #if defined(LS_ARCH_AARCH64)
            const uint32x2_t gt0 = vcgtz_f32(vget_low_f32(w0p));
            const uint32x2_t gt1 = vcgtz_f32(vget_low_f32(w1p));
            const uint32x2_t gt2 = vcgtz_f32(vget_low_f32(w2p));
        #else
            const float32x2_t zero = vdup_n_f32(0.f);
            const uint32x2_t gt0 = vcgt_f32(vget_low_f32(w0p), zero);
            const uint32x2_t gt1 = vcgt_f32(vget_low_f32(w1p), zero);
            const uint32x2_t gt2 = vcgt_f32(vget_low_f32(w2p), zero);
        #endif

        const uint32x2_t part2 = vorr_u32(gt2, vorr_u32(gt1, gt0));
        const unsigned   partI = SL_CLIP_STATUS_PARTIALLY_VISIBLE & vget_lane_u32(part2, 0);

        return (SL_ClipStatus)(visI | partI);

    #else
        const math::vec4 w0p = clip0[3];
        const math::vec4 w1p = clip1[3];
        const math::vec4 w2p = clip2[3];

        const math::vec4 w1n = -clip1[3];
        const math::vec4 w0n = -clip0[3];
        const math::vec4 w2n = -clip2[3];

        int vis = SL_CLIP_STATUS_FULLY_VISIBLE & -(
            clip0 <= w0p &&
            clip1 <= w1p &&
            clip2 <= w2p &&
            clip0 >= w0n &&
            clip1 >= w1n &&
            clip2 >= w2n
        );

        int part = SL_CLIP_STATUS_PARTIALLY_VISIBLE & -(w0p > 0.f || w1p > 0.f || w2p > 0.f);

        return (SL_ClipStatus)(vis | part);
    #endif
}



#endif /* SL_POST_VERTEX_TRANSFORM_HPP */
