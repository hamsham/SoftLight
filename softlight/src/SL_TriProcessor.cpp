
#include "lightsky/math/mat_utils.h"

#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_ShaderUtil.hpp" // SL_BinCounter
#include "softlight/SL_TriProcessor.hpp"
#include "softlight/SL_TriRasterizer.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexCache.hpp"
#include "softlight/SL_ViewportState.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions
-----------------------------------------------------------------------------*/
namespace math = ls::math;

namespace
{



/*--------------------------------------
 * Convert world coordinates to screen coordinates
--------------------------------------*/
inline LS_INLINE void sl_perspective_divide3(math::vec4& LS_RESTRICT_PTR v0, math::vec4& LS_RESTRICT_PTR v1, math::vec4& LS_RESTRICT_PTR v2) noexcept
{
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



/*--------------------------------------
 * Convert world coordinates to screen coordinates
--------------------------------------*/
inline LS_INLINE void sl_world_to_screen_coords_divided3(
    math::vec4& LS_RESTRICT_PTR p0,
    math::vec4& LS_RESTRICT_PTR p1,
    math::vec4& LS_RESTRICT_PTR p2,
    const math::vec4& viewportDims
) noexcept
{
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



/*--------------------------------------
 * Triangle determinants for backface culling
--------------------------------------*/
inline LS_INLINE float face_determinant(
    const math::vec4& LS_RESTRICT_PTR p0,
    const math::vec4& LS_RESTRICT_PTR p1,
    const math::vec4& LS_RESTRICT_PTR p2
) noexcept
{
    // 3D homogeneous determinant of the 3 vertices of a triangle. The
    // Z-component of each 3D vertex is replaced by the 4D W-component.

    #if defined(LS_X86_SSE4_1)
        constexpr int shuffleMask120 = 0x8D; // indices: <base> + (2, 0, 3, 1): 10001101
        constexpr int shuffleMask201 = 0x93; // indices: <base> + (2, 1, 0, 3): 10010011

        // Swap the z and w components for each vector while placing the
        // remaining elements in a position optimal for calculating the
        // determinant.
        const __m128 col4   = _mm_insert_ps( p0.simd, p0.simd, 0xE8);
        const __m128 p1_201 = _mm_permute_ps(p1.simd, shuffleMask201);
        const __m128 p2_120 = _mm_permute_ps(p2.simd, shuffleMask120);
        const __m128 p1_120 = _mm_permute_ps(p1.simd, shuffleMask120);
        const __m128 p2_201 = _mm_permute_ps(p2.simd, shuffleMask201);

        const __m128 col3 = _mm_mul_ps(p1_201, p2_120);
        const __m128 col1 = _mm_mul_ps(p1_120, p2_201);
        const __m128 sub0 = _mm_sub_ps(col1,   col3);

        // Remove the Z component which was shuffled earlier
        const __m128 mul2 = _mm_mul_ps(sub0, col4);

        // horizontal add of only the first 3 elements. We're using the
        // W-component of each vector as the 3rd dimension for out determinant
        // calculation
        const __m128 hi = _mm_movehdup_ps(mul2);
        const __m128 lo = _mm_add_ss(hi, mul2);
        const __m128 w = _mm_permute_ps(mul2, 0xAA);
        return _mm_cvtss_f32(_mm_add_ss(w, lo));

    #elif defined(LS_ARM_NEON) // based on the AVX implementation
        const float32x4_t col4 = vcombine_f32(vget_low_f32(p0.simd), vrev64_f32(vget_high_f32(p0.simd)));

        const float32x2x2_t p1_120 = vzip_f32(vget_low_f32(p1.simd), vget_high_f32(p1.simd));
        const float32x4_t col0 = vcombine_f32(p1_120.val[1], p1_120.val[0]);

        const uint32x4_t p2_201 = vextq_u32(vreinterpretq_u32_f32(p2.simd), vreinterpretq_u32_f32(p2.simd), 3);
        const float32x4_t col1 = vmulq_f32(col0, vreinterpretq_f32_u32(p2_201));

        const float32x4_t col2 = vreinterpretq_f32_u32(vextq_u32(vreinterpretq_u32_f32(p1.simd), vreinterpretq_u32_f32(p1.simd), 3));

        const float32x2x2_t p2_120 = vzip_f32(vget_low_f32(p2.simd), vget_high_f32(p2.simd));
        const float32x4_t sub0 = vmlsq_f32(col1, col2, vcombine_f32(p2_120.val[1], p2_120.val[0]));

        // perform a dot product to get the determinant
        const float32x4_t mul2 = vmulq_f32(sub0, col4);

        #if 0//defined(LS_ARCH_AARCH64)
            return vaddvq_f32(mul2);
        #else
            const float32x2_t swap = vadd_f32(vget_high_f32(mul2), vget_low_f32(mul2));
            const float32x2_t sum = vpadd_f32(swap, swap);
            return vget_lane_f32(sum, 0);
        #endif

    #elif 0
        const math::mat3 det{
            math::vec3{p0[0], p0[1], p0[3]},
            math::vec3{p1[0], p1[1], p1[3]},
            math::vec3{p2[0], p2[1], p2[3]}
        };
        return math::determinant(det);

    #else
        // Alternative algorithm (requires division):
        // Get the normalized homogeneous normal of a triangle and determine
        // if the normal's Z-component is towards, or away from, the camera's
        // Z component.
        const float w0 = math::rcp(p0[3]);
        const float w1 = math::rcp(p1[3]);
        const float w2 = math::rcp(p2[3]);
        const math::vec4&& v0 = p0 * w0;
        const math::vec4&& v1 = p1 * w1;
        const math::vec4&& v2 = p2 * w2;
        const math::vec4 yCAB{v2[1], v0[1], v1[1], 0.f};
        const math::vec4 yBCA{v1[1], v2[1], v0[1], 0.f};
        const math::vec4 xABC{v0[0], v1[0], v2[0], 0.f};
        return math::dot(xABC, (yBCA - yCAB));

    #endif
}



/*--------------------------------------
 * Cull only triangle outside of the screen
--------------------------------------*/
inline LS_INLINE SL_ClipStatus face_visible(
    const math::vec4& LS_RESTRICT_PTR clip0,
    const math::vec4& LS_RESTRICT_PTR clip1,
    const math::vec4& LS_RESTRICT_PTR clip2
) noexcept
{
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



/*--------------------------------------
 * Load a grouping of vertex element IDs
--------------------------------------*/
inline LS_INLINE math::vec4_t<size_t> get_next_vertex3(const SL_IndexBuffer* LS_RESTRICT_PTR pIbo, size_t vId) noexcept
{
    // IBO's are padded so we are always aligned to size()+sizeof(int)*4. This
    // lets us avoid invalid reads at the end of a buffer.
    #if defined(LS_X86_SSE2)
        union
        {
            __m128i asNative;
            math::vec4_t<unsigned char> asBytes;
            math::vec4_t<unsigned short> asShorts;
            math::vec4_t<unsigned int> asInts;
        } ids;

        ids.asNative = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pIbo->element(vId)));

    #elif defined(LS_ARM_NEON)
        union
        {
            uint8x16_t asNative8;
            uint16x8_t asNative16;
            uint32x4_t asNative32;
            math::vec4_t<unsigned char> asBytes;
            math::vec4_t<unsigned short> asShorts;
            math::vec4_t<unsigned int> asInts;
        } ids;

        ids.asNative8 = vld1q_u8(reinterpret_cast<const uint8_t*>(pIbo->element(vId)));

    #else
        union
        {
            math::vec4_t<unsigned char> asBytes;
            math::vec4_t<unsigned short> asShorts;
            math::vec4_t<unsigned int> asInts;
        } ids;

        ids = *reinterpret_cast<const decltype(ids)*>(pIbo->element(vId));
    #endif

    #if defined(LS_X86_AVX)
        static_assert(sizeof(math::vec4_t<size_t>) == sizeof(__m256i), "No available conversion from math::vec4_t<size_t> to __m256i.");
        union
        {
            __m256i simd;
            math::vec4_t<size_t> u64;
        } ret;

        switch (pIbo->type())
        {
            case VERTEX_DATA_BYTE:
                ids.asNative = _mm_cvtepu8_epi16(ids.asNative);

            case VERTEX_DATA_SHORT:
                ids.asNative = _mm_cvtepu16_epi32(ids.asNative);

            case VERTEX_DATA_INT:
                _mm256_store_si256(&ret.simd, _mm256_cvtepu32_epi64(ids.asNative));
                break;

            default:
                LS_UNREACHABLE();
        }

        return ret.u64;

    #elif defined(LS_ARM_NEON)
        static_assert(sizeof(math::vec4_t<size_t>) == sizeof(uint64x2x2_t), "No available conversion from math::vec4_t<size_t> to __m256i.");
        union
        {
            uint64x2x2_t simd;
            math::vec4_t<size_t> u64;
        } ret;

        uint64x2_t lo, hi;

        switch (pIbo->type())
        {
            case VERTEX_DATA_BYTE:
                ids.asNative16 = vmovl_u8(vget_low_u8(ids.asNative8));

            case VERTEX_DATA_SHORT:
                ids.asNative32 = vmovl_u16(vget_low_u16(ids.asNative16));

            case VERTEX_DATA_INT:
                lo = vmovl_u32(vget_low_u32(ids.asNative32));
                hi = vmovl_u32(vget_high_u32(ids.asNative32));
                break;

            default:
                LS_UNREACHABLE();
        }

        ret.simd.val[0] = lo;
        ret.simd.val[1] = hi;
        return ret.u64;


    #else
        switch (pIbo->type())
        {
            case VERTEX_DATA_BYTE:  return (math::vec4_t<size_t>)ids.asBytes;
            case VERTEX_DATA_SHORT: return (math::vec4_t<size_t>)ids.asShorts;
            case VERTEX_DATA_INT:   return (math::vec4_t<size_t>)ids.asInts;
            default:                LS_UNREACHABLE();
        }

    #endif

    return math::vec4_t<size_t>{0, 0, 0, 0};
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_TriProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Publish a vertex to a fragment thread
--------------------------------------*/
void SL_TriProcessor::push_bin(size_t primIndex, const SL_TransformedVert& a, const SL_TransformedVert& b, const SL_TransformedVert& c) noexcept
{
    const uint_fast32_t numVaryings = mShader->pipelineState.num_varyings();

    const math::vec4& p0 = a.vert;
    const math::vec4& p1 = b.vert;
    const math::vec4& p2 = c.vert;

    // establish a bounding box to detect overlap with a thread's tiles
    const math::vec4&& bboxMin = math::min(math::min(p0, p1), p2);
    const math::vec4&& bboxMax = math::max(math::max(p0, p1), p2);

    const int isPrimHidden = (bboxMax[0]-bboxMin[0] < 1.f) || (bboxMax[1]-bboxMin[1] < 1.f);
    if (LS_UNLIKELY(isPrimHidden))
    {
        return;
    }

    // In case these formulas look unfamiliar, these are partial derivatives
    // used in the generation of barycentric coordinates. See the branch below
    // for the general formulas, and SL_TriRasterizer.cpp for their
    // application.
    //{
        const math::vec2&& xy = math::vec2_cast(p0 - p1);
        const math::vec2&& zy = math::vec2_cast(p2 - p1);
        const math::mat4&& pt = math::transpose(math::mat4{
            p0,
            p1,
            p2,
            math::vec4{0.f}
        });

        // A 2D cross product is simply the determinant of a 2x2 matrix
        const float denom = math::rcp(math::cross(zy, xy));
        //const float denom = math::rcp(math::determinant(math::mat2{zy, xy}));

        // cross-products
        const math::vec4&& ddx = math::cross(pt.m[1], math::vec4{1.f});
        const math::vec4&& ddy = math::cross(math::vec4{1.f}, pt.m[0]);
        const math::vec4&& ddz = math::cross(pt.m[0], pt.m[1]);

        // Check if the output bin is full
        uint_fast64_t binId;

        // Attempt to grab a bin index. Flush the bins if they've filled up.
        do
        {
            binId = active_num_bins_used().count.fetch_add(1, std::memory_order_acq_rel);
            if (LS_LIKELY(binId < SL_SHADER_MAX_BINNED_PRIMS))
            {
                break;
            }

            flush_rasterizer<SL_TriRasterizer>();
        }
        while (true);

        // place a triangle into the next available bin
        SL_FragmentBin* const pFragBins = active_frag_bins();
        SL_FragmentBin& bin = pFragBins[binId];
        bin.mScreenCoords[0] = p0;
        bin.mScreenCoords[1] = p1;
        bin.mScreenCoords[2] = p2;

        bin.mBarycentricCoords[0] = ddx * denom;
        bin.mBarycentricCoords[1] = ddy * denom;
        bin.mBarycentricCoords[2] = ddz * denom;
    //}

    switch (numVaryings)
    {
        case 4:
            bin.mVaryings[3+SL_SHADER_MAX_VARYING_VECTORS*0] = a.varyings[3];
            bin.mVaryings[3+SL_SHADER_MAX_VARYING_VECTORS*1] = b.varyings[3];
            bin.mVaryings[3+SL_SHADER_MAX_VARYING_VECTORS*2] = c.varyings[3];

        case 3:
            bin.mVaryings[2+SL_SHADER_MAX_VARYING_VECTORS*0] = a.varyings[2];
            bin.mVaryings[2+SL_SHADER_MAX_VARYING_VECTORS*1] = b.varyings[2];
            bin.mVaryings[2+SL_SHADER_MAX_VARYING_VECTORS*2] = c.varyings[2];

        case 2:
            bin.mVaryings[1+SL_SHADER_MAX_VARYING_VECTORS*0] = a.varyings[1];
            bin.mVaryings[1+SL_SHADER_MAX_VARYING_VECTORS*1] = b.varyings[1];
            bin.mVaryings[1+SL_SHADER_MAX_VARYING_VECTORS*2] = c.varyings[1];

        case 1:
            bin.mVaryings[0+SL_SHADER_MAX_VARYING_VECTORS*0] = a.varyings[0];
            bin.mVaryings[0+SL_SHADER_MAX_VARYING_VECTORS*1] = b.varyings[0];
            bin.mVaryings[0+SL_SHADER_MAX_VARYING_VECTORS*2] = c.varyings[0];
    }

    bin.primIndex = primIndex;
    active_bin_index(binId).count = binId;
}



/*-------------------------------------
 * Ensure only visible triangles get rendered. Triangles should have already
 * been tested for visibility within clip-space. Now we need to clip the
 * remaining tris and generate new ones.
 *
 * The basic clipping algorithm is as follows:
 *
 *  for each clipping edge do
 *      for (i = 0; i < Polygon.length; i++)
 *          Pi = Polygon.vertex[i];
 *          Pi+1 = Polygon.vertex[i+1];
 *          if (Pi is inside clipping region)
 *              if (Pi+1 is inside clipping region)
 *                  clippedPolygon.add(Pi+1)
 *              else
 *                  clippedPolygon.add(intersectionPoint(Pi, Pi+1, currentEdge)
 *          else
 *              if (Pi+1 is inside clipping region)
 *                  clippedPolygon.add(intersectionPoint(Pi, Pi+1, currentEdge)
 *                  clippedPolygon.add(Pi+1)
 *      end for
 *      Polygon = clippedPolygon     // keep on working with the new polygon
 *  end for
-------------------------------------*/
void SL_TriProcessor::clip_and_process_tris(
    size_t primIndex,
    const math::vec4& viewportDims,
    const SL_TransformedVert& a,
    const SL_TransformedVert& b,
    const SL_TransformedVert& c) noexcept
{
    const unsigned     numVarys      = (unsigned)mShader->pipelineState.num_varyings();
    constexpr unsigned numTempVerts  = 9; // at most 9 vertices should be generated
    unsigned           numTotalVerts = 3;
    math::vec4         tempVerts     [numTempVerts];
    math::vec4         newVerts      [numTempVerts];
    math::vec4         tempVarys     [numTempVerts * SL_SHADER_MAX_VARYING_VECTORS];
    math::vec4         newVarys      [numTempVerts * SL_SHADER_MAX_VARYING_VECTORS];
    const math::vec4   clipEdges[]  = {
        { 1.f,  0.f,  0.f, 1.f},
        {-1.f,  0.f,  0.f, 1.f},
        { 0.f,  1.f,  0.f, 1.f},
        { 0.f, -1.f,  0.f, 1.f},
#if SL_Z_CLIPPING_ENABLED
        { 0.f,  0.f,  1.f, 1.f},
        { 0.f,  0.f, -1.f, 1.f},
#endif
    };

    const auto _copy_verts = [](int maxVerts, const math::vec4* inVerts, math::vec4* outVerts) noexcept->void
    {
        while (maxVerts--)
        {
            outVerts[maxVerts] = inVerts[maxVerts];
        }
    };

    const auto _interpolate_varyings = [&numVarys](const math::vec4* inVarys, math::vec4* outVarys, int fromIndex, int toIndex, float amt) noexcept->void
    {
        const math::vec4* pV0 = inVarys + fromIndex * SL_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* pV1 = inVarys + toIndex * SL_SHADER_MAX_VARYING_VECTORS;

        for (unsigned i = numVarys; i--;)
        {
            *outVarys++ = math::mix(*pV0++, *pV1++, amt);
        }
    };

    newVerts[0] = a.vert;
    _copy_verts(numVarys, a.varyings, newVarys + 0 * SL_SHADER_MAX_VARYING_VECTORS);

    newVerts[1] = b.vert;
    _copy_verts(numVarys, b.varyings, newVarys + 1 * SL_SHADER_MAX_VARYING_VECTORS);

    newVerts[2] = c.vert;
    _copy_verts(numVarys, c.varyings, newVarys + 2 * SL_SHADER_MAX_VARYING_VECTORS);

    for (const math::vec4& edge : clipEdges)
    {
        // caching
        unsigned   numNewVerts = 0;
        unsigned   j           = numTotalVerts-1;
        math::vec4 p0          = newVerts[numTotalVerts-1];
        float      t0          = math::dot(p0, edge);
        int        visible0    = !math::sign_mask(t0);

        for (unsigned k = 0; k < numTotalVerts; ++k)
        {
            const math::vec4& p1 = newVerts[k];
            const float t1       = math::dot(p1, edge);
            const int   visible1 = !math::sign_mask(t1);

            if (visible0 ^ visible1)
            {
                const float t = t0 * math::rcp(t0-t1);//math::clamp(t0 / (t0-t1), 0.f, 1.f);
                tempVerts[numNewVerts] = math::mix(p0, p1, t);
                _interpolate_varyings(newVarys, tempVarys+(numNewVerts*SL_SHADER_MAX_VARYING_VECTORS), j, k, t);

                ++numNewVerts;
            }

            if (visible1)
            {
                tempVerts[numNewVerts] = p1;
                _copy_verts(numVarys, newVarys+(k*SL_SHADER_MAX_VARYING_VECTORS), tempVarys+(numNewVerts*SL_SHADER_MAX_VARYING_VECTORS));
                ++numNewVerts;
            }

            j           = k;
            p0          = p1;
            t0          = t1;
            visible0    = visible1;
        }

        if (LS_UNLIKELY(!numNewVerts))
        {
            return;
        }

        numTotalVerts = numNewVerts;
        _copy_verts(numNewVerts, tempVerts, newVerts);

        for (unsigned i = numNewVerts; i--;)
        {
            const unsigned offset = i*SL_SHADER_MAX_VARYING_VECTORS;
            _copy_verts(numNewVerts, tempVarys+offset, newVarys+offset);
        }
    }

    if (numTotalVerts < 3)
    {
        return;
    }

    LS_DEBUG_ASSERT(numTotalVerts <= numTempVerts);

    switch (numTotalVerts)
    {
        case 9:
        case 8:
        case 7:
            sl_perspective_divide3(newVerts[6], newVerts[7], newVerts[8]);
            sl_world_to_screen_coords_divided3(newVerts[6], newVerts[7], newVerts[8], viewportDims);

        case 6:
        case 5:
        case 4:
            sl_perspective_divide3(newVerts[3], newVerts[4], newVerts[5]);
            sl_world_to_screen_coords_divided3(newVerts[3], newVerts[4], newVerts[5], viewportDims);

        default:
            sl_perspective_divide3(newVerts[0], newVerts[1], newVerts[2]);
            sl_world_to_screen_coords_divided3(newVerts[0], newVerts[1], newVerts[2], viewportDims);
    }

    SL_TransformedVert p0, p1, p2;
    p0.vert = newVerts[0];
    _copy_verts(numVarys, newVarys, p0.varyings);

    for (unsigned i = numTotalVerts-2; i--;)
    {
        const unsigned j = i+1;
        const unsigned k = i+2;

        p1.vert = newVerts[j];
        _copy_verts(numVarys, newVarys+(j*SL_SHADER_MAX_VARYING_VECTORS), p1.varyings);

        p2.vert = newVerts[k];
        _copy_verts(numVarys, newVarys+(k*SL_SHADER_MAX_VARYING_VECTORS), p2.varyings);

        // clipped Tri's are coplanar so we give them the same sort index.
        push_bin(primIndex, p0, p1, p2);
    }
}



/*--------------------------------------
 * Process Points
--------------------------------------*/
template <bool usingIndices>
void SL_TriProcessor::process_verts(
    const SL_Mesh& m,
    size_t instanceId,
    const ls::math::mat4_t<float>& scissorMat,
    const ls::math::vec4_t<float>& viewportDims) noexcept
{
    SL_TransformedVert    pVert0;
    SL_TransformedVert    pVert1;
    SL_TransformedVert    pVert2;
    const auto            vertShader = mShader->pVertShader;
    const SL_CullMode     cullMode   = mShader->pipelineState.cull_mode();
    const SL_VertexArray& vao        = mContext->vao(m.vaoId);
    const SL_IndexBuffer* pIbo       = vao.has_index_buffer() ? &mContext->ibo(vao.get_index_buffer()) : nullptr;

    SL_VertexParam params;
    params.pUniforms  = mShader->pUniforms;
    params.instanceId = instanceId;
    params.pVao       = &vao;
    params.pVbo       = &mContext->vbo(vao.get_vertex_buffer());

    const size_t numElements = m.elementEnd - m.elementBegin;
    const size_t primOffset = numElements * instanceId;

    #if SL_VERTEX_CACHING_ENABLED
        size_t begin;
        size_t end;
        constexpr size_t step = 3;

        sl_calc_indexed_parition<3, true>(numElements, mNumThreads, mThreadId, begin, end);
        begin += m.elementBegin;
        end += m.elementBegin;

        SL_PTVCache ptvCache{};
        const auto&& vertTransform = [&](size_t key, SL_TransformedVert& tv) noexcept -> void
        {
            params.vertId = key;
            params.pVaryings = tv.varyings;
            tv.vert = scissorMat * vertShader(params);
        };

    #else
        //const size_t begin = m.elementBegin + mThreadId * 3u;
        const size_t begin = m.elementBegin + ((instanceId + mThreadId) % mNumThreads) * 3u;
        const size_t end   = m.elementEnd;
        const size_t step  = mNumThreads * 3u;
    #endif

    for (size_t i = begin; i < end; i += step)
    {
        const math::vec4_t<size_t>&& vertId = usingIndices ? get_next_vertex3(pIbo, i) : math::vec4_t<size_t>{i+0, i+1, i+2, i+3};

        #if SL_VERTEX_CACHING_ENABLED
            sl_cache_query_or_update(ptvCache, vertId[0], pVert0, vertTransform);
            sl_cache_query_or_update(ptvCache, vertId[1], pVert1, vertTransform);
            sl_cache_query_or_update(ptvCache, vertId[2], pVert2, vertTransform);

        #else
            params.vertId    = vertId.v[0];
            params.pVaryings = pVert0.varyings;
            pVert0.vert      = vertShader(params);

            params.vertId    = vertId.v[1];
            params.pVaryings = pVert1.varyings;
            pVert1.vert      = vertShader(params);

            params.vertId    = vertId.v[2];
            params.pVaryings = pVert2.varyings;
            pVert2.vert      = vertShader(params);
        #endif

        if (LS_LIKELY(cullMode != SL_CULL_OFF))
        {
            const float det = face_determinant(pVert0.vert, pVert1.vert, pVert2.vert);

            // Using bitwise magic to reduce time spent making comparisons.
            // We can cull the backface with (det < 0.f) or cull the front
            // face with (det > 0.f).

            const bool culled = (cullMode == SL_CULL_FRONT_FACE) ^ math::sign_mask(det);
            //if ((cullMode == SL_CULL_BACK_FACE && det < 0.f)
            //|| (cullMode == SL_CULL_FRONT_FACE && det > 0.f))
            if (culled)
            {
                continue;
            }
        }

        #if !SL_VERTEX_CACHING_ENABLED
            pVert0.vert = scissorMat * pVert0.vert;
            pVert1.vert = scissorMat * pVert1.vert;
            pVert2.vert = scissorMat * pVert2.vert;
        #endif

        // Clip-space culling
        const SL_ClipStatus visStatus = face_visible(pVert0.vert, pVert1.vert, pVert2.vert);
        if (visStatus == SL_CLIP_STATUS_FULLY_VISIBLE)
        {
            sl_perspective_divide3(pVert0.vert, pVert1.vert, pVert2.vert);
            sl_world_to_screen_coords_divided3(pVert0.vert, pVert1.vert, pVert2.vert, viewportDims);
            push_bin(primOffset+i, pVert0, pVert1, pVert2);
        }
        else if (visStatus == SL_CLIP_STATUS_PARTIALLY_VISIBLE)
        {
            clip_and_process_tris(primOffset+i, viewportDims, pVert0, pVert1, pVert2);
        }

        #if SL_VERTEX_CACHING_ENABLED
            if (LS_LIKELY(visStatus != SL_CLIP_STATUS_NOT_VISIBLE))
            {
                LS_PREFETCH(&ptvCache, LS_PREFETCH_ACCESS_RW, LS_PREFETCH_LEVEL_NONTEMPORAL);
            }
        #endif
    }
}



template void SL_TriProcessor::process_verts<true>(
    const SL_Mesh&,
    size_t,
    const ls::math::mat4_t<float>&,
    const ls::math::vec4_t<float>&
) noexcept;



template void SL_TriProcessor::process_verts<false>(
    const SL_Mesh&,
    size_t,
    const ls::math::mat4_t<float>&,
    const ls::math::vec4_t<float>&
) noexcept;



/*--------------------------------------
 * Execute the point rasterization
--------------------------------------*/
void SL_TriProcessor::execute() noexcept
{
    if (active_frag_processors().count.load(std::memory_order_consume))
    {
        flush_rasterizer<SL_TriRasterizer>();
    }

    const math::vec4&&      fboDims      = (math::vec4)math::vec4_t<int>{0, 0, mFbo->width(), mFbo->height()};
    const SL_ViewportState& viewState    = mContext->viewport_state();
    const math::mat4&&      scissorMat   = viewState.scissor_matrix(fboDims[2], fboDims[3]);
    const math::vec4&&      viewportDims = viewState.viewport_rect(fboDims[2], fboDims[3]);

    if (mNumInstances == 1)
    {
        for (size_t i = 0; i < mNumMeshes; ++i)
        {
            const SL_Mesh& m = mMeshes[i];
            const bool usingIndices = (m.mode == RENDER_MODE_INDEXED_TRIANGLES) || (m.mode == RENDER_MODE_INDEXED_TRI_WIRE);

            if (usingIndices)
            {
                process_verts<true>(m, 0, scissorMat, viewportDims);
            }
            else
            {
                process_verts<false>(m, 0, scissorMat, viewportDims);
            }
        }
    }
    else
    {
        const SL_Mesh& m = mMeshes[0];
        const bool usingIndices = (m.mode == RENDER_MODE_INDEXED_TRIANGLES) || (m.mode == RENDER_MODE_INDEXED_TRI_WIRE);

        if (usingIndices)
        {
            for (size_t i = 0; i < mNumInstances; ++i)
            {
                process_verts<true>(m, i, scissorMat, viewportDims);
            }
        }
        else
        {
            for (size_t i = 0; i < mNumInstances; ++i)
            {
                process_verts<false>(m, i, scissorMat, viewportDims);
            }
        }
    }

    this->cleanup<SL_TriRasterizer>();
}
