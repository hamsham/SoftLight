
#ifndef SL_SCANLINEBOUNDS_HPP
#define SL_SCANLINEBOUNDS_HPP

#include <cstdint>

#include "lightsky/setup/Api.h"

#include "lightsky/math/vec2.h"
#include "lightsky/math/vec4.h"
#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec_utils.h"



/**------------------------------------
 * @brief Retrieve the offset to a threads first renderable scanline.
 *
 * @tparam data_t
 * The requested data type.
 *
 * @param numThreads
 * The number of threads which are currently being used for rendering.
 *
 * @param threadId
 * The current thread's ID (0-based index).
 *
 * @param fragmentY
 * The initial scanline for a line or triangle being rendered.
-------------------------------------*/
template <typename data_t>
constexpr LS_INLINE data_t sl_scanline_offset(
    const data_t numThreads,
    const data_t threadId,
    const data_t fragmentY) noexcept
{
    //return numThreads - 1 - (((fragmentY % numThreads) + threadId) % numThreads);
    return numThreads - 1 - ((fragmentY + threadId) % numThreads);
}



/*-------------------------------------
 * Branchless vertex swap for SSE
-------------------------------------*/
#ifdef LS_ARCH_X86

inline LS_INLINE void sl_sort_minmax(__m128& a, __m128& b)  noexcept
{
    #if defined(LS_X86_AVX)
        const __m128 masks = _mm_permute_ps(_mm_cmplt_ps(a, b), 0x55);
        const __m128 at   = a;
        const __m128 bt   = b;
        a = _mm_blendv_ps(at, bt, masks);
        b = _mm_blendv_ps(bt, at, masks);
    #else
        const __m128 cmp  = _mm_cmplt_ps(a, b);
        const __m128i abx = _mm_xor_ps(a, b);
        const __m128 mask = _mm_and_ps(abx, _mm_shuffle_ps(cmp, cmp, 0x55));
        const __m128 at   = _mm_xor_ps(mask, a);
        const __m128 bt   = _mm_xor_ps(mask, b);
        a = at;
        b = bt;
    #endif
}

#elif defined(LS_ARM_NEON)

inline LS_INLINE void sl_sort_minmax(float32x4_t& a, float32x4_t& b)  noexcept
{
    #if defined(LS_ARCH_AARCH64)
        const uint32x4_t mask = vdupq_laneq_u32(vcltq_f32(a, b), 1);
    #else
        const uint32x4_t mask = vdupq_lane_u32(vget_low_u32(vcltq_f32(a, b)), 1);
    #endif

    const float32x4_t at = a;
    const float32x4_t bt = b;

    a = vbslq_f32(mask, bt, at);
    b = vbslq_f32(mask, at, bt);
}

#endif



inline LS_INLINE void sl_sort_minmax(int32_t a, int32_t b, int32_t& outA, int32_t& outB)  noexcept
{
    const int32_t mask = -(a >= b);
    const int32_t al   = ~mask & a;
    const int32_t bg   = ~mask & b;
    const int32_t ag   = mask & a;
    const int32_t bl   = mask & b;

    outA = al | bl;
    outB = ag | bg;
}



/*-----------------------------------------------------------------------------
 * Common method to get the beginning and end of a scanline.
-----------------------------------------------------------------------------*/
struct alignas(sizeof(float)*4) SL_ScanlineBounds
{
    float v0y;
    float v1y;
    float v0x;
    float v1x;

    float p20y;
    float p20x;
    float p21xy;
    float p10xy;

    inline void LS_INLINE init(ls::math::vec4 p0, ls::math::vec4 p1, ls::math::vec4 p2) noexcept
    {
        #if defined(LS_ARCH_X86) || defined(LS_ARM_NEON)
            sl_sort_minmax(p0.simd, p1.simd);
            sl_sort_minmax(p0.simd, p2.simd);
            sl_sort_minmax(p1.simd, p2.simd);
        #else
            if (p0[1] < p1[1]) std::swap(p0, p1);
            if (p0[1] < p2[1]) std::swap(p0, p2);
            if (p1[1] < p2[1]) std::swap(p1, p2);
        #endif

        v0y = p0[1];
        v1y = p1[1];
        v0x = p0[0];
        v1x = p1[0];

        p20y  = ls::math::rcp(p2[1] - p0[1]);
        p20x  = p2[0] - p0[0];
        p21xy = (p2[0] - p1[0]) * ls::math::rcp(p2[1] - p1[1]);
        p10xy = (p1[0] - p0[0]) * ls::math::rcp(p1[1] - p0[1]);
    }

    inline void LS_INLINE step(const float yf, int32_t& xMin, int32_t& xMax) const noexcept
    {
        #if defined(LS_X86_AVX)
            const __m128 yv         = _mm_set_ss(yf);
            const __m128 v01        = _mm_set_ss(v0y);
            const __m128 v11        = _mm_set_ss(v1y);
            const __m128 p201       = _mm_set_ss(p20y);
            const __m128 d0         = _mm_sub_ss(yv, v01);
            const __m128 alpha      = _mm_mul_ss(d0, p201);
            const __m128 d1         = _mm_sub_ss(yv, v11);

            const __m128 v00        = _mm_set_ss(v0x);
            const __m128 v10        = _mm_set_ss(v1x);
            const __m128 p1001      = _mm_set_ss(p10xy);
            const __m128 p200       = _mm_set_ss(p20x);
            const __m128 p2101      = _mm_set_ss(p21xy);
            const __m128 lo         = _mm_fmadd_ss( p200,  alpha, v00);
            const __m128 pdv0       = _mm_fmadd_ss( p1001, d0,    v00);
            const __m128 pdv1       = _mm_fmadd_ss( p2101, d1,    v10);
            const __m128 hi         = _mm_blendv_ps(pdv0,  pdv1,  d1);

            xMin = _mm_cvtss_si32(_mm_min_ss(lo, hi));
            xMax = _mm_cvtss_si32(_mm_max_ss(lo, hi));

        #elif defined(LS_X86_SSE2)
            const __m128 yv         = _mm_set_ss(yf);
            const __m128 v01        = _mm_set_ss(v0y);
            const __m128 v11        = _mm_set_ss(v1y);
            const __m128 p201       = _mm_set_ss(p20y);
            const __m128 d0         = _mm_sub_ss(yv, v01);
            const __m128 alpha      = _mm_mul_ss(d0, p201);
            const __m128 d1         = _mm_sub_ss(yv, v11);

            const __m128 v00        = _mm_set_ss(v0x);
            const __m128 p200       = _mm_set_ss(p20x);
            const __m128 lo         = _mm_add_ss(_mm_mul_ss(p200,  alpha), v00);
            const __m128 p1001      = _mm_set_ss(p10xy);
            const __m128 pdv0       = _mm_add_ss(_mm_mul_ss(p1001, d0),    v00);
            const __m128 cmpMask    = _mm_srai_epi32(_mm_castps_si128(d1), 31);
            const __m128 v10        = _mm_set_ss(v1x);
            const __m128 p2101      = _mm_set_ss(p21xy);
            const __m128 pdv1       = _mm_add_ss(_mm_mul_ss(p2101, d1),    v10);
            const __m128 hi         = _mm_or_ps(_mm_andnot_ps(cmpMask, pdv0), _mm_and_ps(cmpMask, pdv1));

            xMin = _mm_cvtss_si32(_mm_min_ss(lo, hi));
            xMax = _mm_cvtss_si32(_mm_max_ss(lo, hi));

        #elif defined(LS_ARM_NEON)
            const float32x2_t yv         = vdup_n_f32(yf);
            const float32x2_t v01        = vdup_n_f32(v0y);
            const float32x2_t v11        = vdup_n_f32(v1y);
            const float32x2_t p201       = vdup_n_f32(p20y);
            const float32x2_t d0         = vsub_f32(yv, v01);
            const float32x2_t alpha      = vmul_f32(d0, p201);
            const float32x2_t d1         = vsub_f32(yv, v11);
            const uint32x2_t  secondHalf = vreinterpret_u32_s32(vshr_n_s32(vreinterpret_s32_f32(d1), 31));

            const float32x2_t v00        = vdup_n_f32(v0x);
            const float32x2_t v10        = vdup_n_f32(v1x);
            const float32x2_t p1001      = vdup_n_f32(p10xy);
            const float32x2_t p200       = vdup_n_f32(p20x);
            const float32x2_t p2101      = vdup_n_f32(p21xy);
            const float32x2_t lo         = vmla_f32(v00, p200,  alpha);
            const float32x2_t pdv0       = vmla_f32(v00, p1001, d0);
            const float32x2_t pdv1       = vmla_f32(v10, p2101, d1);
            const float32x2_t hi         = vbsl_f32(secondHalf, pdv1, pdv0);

            xMin = vget_lane_s32(vcvt_s32_f32(vmin_f32(lo, hi)), 0);
            xMax = vget_lane_s32(vcvt_s32_f32(vmax_f32(lo, hi)), 0);

        #else
            const float d0 = yf - v0y;
            const float d1 = yf - v1y;
            const float alpha = d0 * p20y;

            const int32_t lo         = (int32_t)(p20x * alpha + v0x);
            const int32_t secondHalf = -(d1 < 0);
            const int32_t a          = (int32_t)(p21xy * d1 + v1x);
            const int32_t b          = (int32_t)(p10xy * d0 + v0x);
            const int32_t hi         = (secondHalf & a) | (~secondHalf & b);

            sl_sort_minmax(lo, hi, xMin, xMax);
        #endif
    }
};


#endif /* SL_SCANLINEBOUNDS_HPP */
