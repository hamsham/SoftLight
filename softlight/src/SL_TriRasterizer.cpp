
#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/utils/Assertions.h" // LS_DEBUG_ASSERT

#include "lightsky/math/bits.h"
#include "lightsky/math/fixed.h"
#include "lightsky/math/half.h"
#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "softlight/SL_TriRasterizer.hpp"
#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_ScanlineBounds.hpp"
#include "softlight/SL_Shader.hpp" // SL_FragmentShader
#include "softlight/SL_ShaderProcessor.hpp" // SL_FragmentBin
#include "softlight/SL_ShaderUtil.hpp" // sl_scanline_offset(), SL_BinCounter
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_ViewportState.hpp"



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
 * Interpolate varying variables across a triangle
--------------------------------------*/
inline void LS_IMPERATIVE interpolate_tri_varyings(
    const float*      LS_RESTRICT_PTR baryCoords,
    uint_fast32_t     numVaryings,
    const math::vec4* LS_RESTRICT_PTR inVaryings0,
    math::vec4*       LS_RESTRICT_PTR outVaryings) noexcept
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

        switch (numVaryings)
        {
            case 4:
                v0 = vmulq_laneq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+3)), bc, 0);
                v1 = vfmaq_laneq_f32(v0, vld1q_f32(reinterpret_cast<const float*>(inVaryings1+3)), bc, 1);
                v2 = vfmaq_laneq_f32(v1, vld1q_f32(reinterpret_cast<const float*>(inVaryings2+3)), bc, 2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings+3), v2);

            case 3:
                v0 = vmulq_laneq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+2)), bc, 0);
                v1 = vfmaq_laneq_f32(v0, vld1q_f32(reinterpret_cast<const float*>(inVaryings1+2)), bc, 1);
                v2 = vfmaq_laneq_f32(v1, vld1q_f32(reinterpret_cast<const float*>(inVaryings2+2)), bc, 2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings+2), v2);

            case 2:
                v0 = vmulq_laneq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+1)), bc, 0);
                v1 = vfmaq_laneq_f32(v0, vld1q_f32(reinterpret_cast<const float*>(inVaryings1+1)), bc, 1);
                v2 = vfmaq_laneq_f32(v1, vld1q_f32(reinterpret_cast<const float*>(inVaryings2+1)), bc, 2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings+1), v2);

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

        switch (numVaryings)
        {
            case 4:
                v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+3)), bc0);
                v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1+3)), bc1);
                v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2+3)), bc2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings+3), vaddq_f32(v2, vaddq_f32(v1, v0)));

            case 3:
                v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+2)), bc0);
                v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1+2)), bc1);
                v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2+2)), bc2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings+2), vaddq_f32(v2, vaddq_f32(v1, v0)));

            case 2:
                v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0+1)), bc0);
                v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1+1)), bc1);
                v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2+1)), bc2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings+1), vaddq_f32(v2, vaddq_f32(v1, v0)));

            case 1:
                v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0)), bc0);
                v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1)), bc1);
                v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2)), bc2);
                vst1q_f32(reinterpret_cast<float*>(outVaryings), vaddq_f32(v2, vaddq_f32(v1, v0)));
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



/*--------------------------------------
 * Load and convert a depth texel from memory
--------------------------------------*/
template <typename depth_type>
inline LS_INLINE float _sl_get_depth_texel(const depth_type* pDepth)
{
    return (float)*pDepth;
}

#if defined(LS_ARCH_X86)
template <>
inline LS_INLINE float _sl_get_depth_texel<float>(const float* pDepth)
{
    return _mm_cvtss_f32(_mm_load_ss(pDepth));
}

#elif defined(LS_ARM_NEON)
template <>
inline LS_INLINE float _sl_get_depth_texel<math::half>(const math::half* pDepth)
{
    return (float)(*reinterpret_cast<const __fp16*>(pDepth));
}

#endif



/*--------------------------------------
 * Load and convert 4 depth texels from memory
--------------------------------------*/
template <typename depth_type>
inline LS_INLINE math::vec4 _sl_get_depth_texel4(const depth_type* pDepth);

#if defined(LS_X86_FP16)
template <>
inline LS_INLINE math::vec4 _sl_get_depth_texel4<math::half>(const math::half* pDepth)
{
    return math::vec4{_mm_cvtph_ps(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(pDepth)))};
}

template <>
inline LS_INLINE math::vec4 _sl_get_depth_texel4<float>(const float* pDepth)
{
    return math::vec4{_mm_loadu_ps(pDepth)};
}

#elif defined(LS_ARM_NEON)
template <>
inline LS_INLINE math::vec4 _sl_get_depth_texel4<math::half>(const math::half* pDepth)
{
    return math::vec4{vcvt_f32_f16(vld1_f16(reinterpret_cast<const __fp16*>(pDepth)))};
}

template <>
inline LS_INLINE math::vec4 _sl_get_depth_texel4<float>(const float* pDepth)
{
    return math::vec4{vld1q_f32(pDepth)};
}

#endif

template <typename depth_type>
inline LS_INLINE math::vec4 _sl_get_depth_texel4(const depth_type* pDepth)
{
    return (math::vec4)(*reinterpret_cast<const math::vec4_t<depth_type>*>(pDepth));
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_TriRasterizer Class
-----------------------------------------------------------------------------*/
#if SL_CONSERVE_MEMORY
/*--------------------------------------
 * Bin-Rasterization
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::flush_scanlines(const SL_FragmentBin* pBin, uint32_t xMin, uint32_t xMax, uint32_t y) const noexcept
{
    const math::vec4* pPoints     = pBin->mScreenCoords;
    const math::vec4* bcClipSpace = pBin->mBarycentricCoords;
    const math::vec4  depth       {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};
    const math::vec4  homogenous  {pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
    depth_type*       pDepthBuf   = mFbo->get_depth_buffer()->row_pointer<depth_type>((uint16_t)y);

    constexpr DepthCmpFunc   depthCmpFunc;
    const SL_FragmentShader& fragShader    = mShader->mFragShader;
    const SL_FboOutputMask   fboOutMask    = sl_calc_fbo_out_mask(fragShader.numOutputs, fragShader.blend != SL_BLEND_OFF);
    const bool               haveDepthMask = fragShader.depthMask == SL_DEPTH_MASK_ON;

    SL_FragmentParam fragParams;
    fragParams.pUniforms = mShader->mUniforms;

    const float yf = (float)y;
    const math::vec4&& bcY = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);

    math::vec4&& bcX = math::fmadd(bcClipSpace[0], math::vec4{(float)xMin}, bcY);

    for (uint32_t x = xMin; x < xMax; ++x, bcX += bcClipSpace[0])
    {
        // calculate barycentric coordinates
        const float          d         = _sl_get_depth_texel<depth_type>(pDepthBuf + x);
        math::vec4           bc        = bcX;
        const float          z         = math::dot(depth, bc);
        const int_fast32_t&& depthTest = depthCmpFunc(z, d);

        if (depthTest)
        {
            fragParams.coord.x = (uint16_t)x;
            fragParams.coord.y = (uint16_t)y;
            fragParams.coord.depth = z;

            // perspective correction
            const float persp = math::rcp(math::dot(bc, homogenous));
            bc *= homogenous;
            bc *= persp;

            interpolate_tri_varyings(bc.v, fragShader.numVaryings, pBin->mVaryings, fragParams.pVaryings);

            if (LS_LIKELY(fragShader.shader(fragParams)))
            {
                mFbo->put_pixel(fboOutMask, fragShader.blend, fragParams);

                if (LS_LIKELY(haveDepthMask))
                {
                    pDepthBuf[x] = (depth_type)fragParams.coord.depth;
                }
            }
        }
    }
}



template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncLT, ls::math::half>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncLT, float>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncLT, double>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;

template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncLE, ls::math::half>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncLE, float>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncLE, double>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;

template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncGT, ls::math::half>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncGT, float>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncGT, double>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;

template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncGE, ls::math::half>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncGE, float>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncGE, double>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;

template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncEQ, ls::math::half>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncEQ, float>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncEQ, double>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;

template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncNE, ls::math::half>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncNE, float>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncNE, double>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;

template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncOFF, ls::math::half>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncOFF, float>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;
template void SL_TriRasterizer::flush_scanlines<SL_DepthFuncOFF, double>(const SL_FragmentBin*, uint32_t, uint32_t, uint32_t) const noexcept;



/*--------------------------------------
 * Triangle Rasterization, scalar
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::iterate_tri_scanlines() const noexcept
{
    const SL_BinCounter<uint32_t>* pBinIds = mBinIds;
    const SL_FragmentBin* const    pBins   = mBins;

    const uint32_t    numBins   = (uint32_t)mNumBins;
    const int32_t     yOffset   = (int32_t)mThreadId;
    const int32_t     increment = (int32_t)mNumProcessors;
    SL_ScanlineBounds scanline;

    for (uint32_t i = 0; i < numBins; ++i)
    {
        const uint32_t        binId          = pBinIds[i].count;
        const SL_FragmentBin* pBin           = pBins+binId;
        const math::vec4*     pPoints        = pBin->mScreenCoords;
        const int32_t         bboxMinY       = (int32_t)math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t         bboxMaxY       = (int32_t)math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t         scanLineOffset = bboxMinY + sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        scanline.init(pPoints[0], pPoints[1], pPoints[2]);

        for (int32_t y = scanLineOffset; y < bboxMaxY; y += increment)
        {
            // calculate the bounds of the current scan-line
            const float yf = (float)y;

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as a
            // guard against any pixels we don't want to render.
            int32_t x;
            int32_t xMax;
            scanline.step(yf, x, xMax);

            if (LS_LIKELY(x < xMax))
            {
                flush_scanlines<DepthCmpFunc, depth_type>(pBin, x, xMax, y);
            }
        }
    }
}



template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncLT, ls::math::half>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncLT, float>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncLT, double>() const noexcept;

template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncLE, ls::math::half>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncLE, float>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncLE, double>() const noexcept;

template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncGT, ls::math::half>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncGT, float>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncGT, double>() const noexcept;

template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncGE, ls::math::half>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncGE, float>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncGE, double>() const noexcept;

template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncEQ, ls::math::half>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncEQ, float>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncEQ, double>() const noexcept;

template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncNE, ls::math::half>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncNE, float>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncNE, double>() const noexcept;

template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncOFF, ls::math::half>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncOFF, float>() const noexcept;
template void SL_TriRasterizer::iterate_tri_scanlines<SL_DepthFuncOFF, double>() const noexcept;

#endif



/*--------------------------------------
 * Bin-Rasterization
--------------------------------------*/
template <typename depth_type>
void SL_TriRasterizer::flush_fragments(
    const SL_FragmentBin* pBin,
    uint32_t              numQueuedFrags,
    const SL_FragCoord*   outCoords) const noexcept
{
    const SL_UniformBuffer*  pUniforms     = mShader->mUniforms;
    const SL_FragmentShader& fragShader    = mShader->mFragShader;
    const SL_FboOutputMask   fboOutMask    = sl_calc_fbo_out_mask(fragShader.numOutputs, (fragShader.blend != SL_BLEND_OFF));
    const int_fast32_t       haveDepthMask = fragShader.depthMask == SL_DEPTH_MASK_ON;
    SL_Texture*              pDepthBuf     = mFbo->get_depth_buffer();

    SL_FragmentParam fragParams;
    fragParams.pUniforms = pUniforms;

    for (uint32_t i = 0; i < numQueuedFrags; ++i)
    {
        const math::vec4& bc = outCoords->bc[i];
        fragParams.coord = outCoords->coord[i];

        interpolate_tri_varyings(bc.v, fragShader.numVaryings, pBin->mVaryings, fragParams.pVaryings);
        const bool haveOutputs = fragShader.shader(fragParams);

        if (LS_LIKELY(haveOutputs != false))
        {
            mFbo->put_pixel(fboOutMask, fragShader.blend, fragParams);

            if (LS_LIKELY(haveDepthMask != 0))
            {
                pDepthBuf->raw_texel<depth_type>(fragParams.coord.x, fragParams.coord.y) = (depth_type)fragParams.coord.depth;
            }
        }
    }
}



template void SL_TriRasterizer::flush_fragments<ls::math::half>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
template void SL_TriRasterizer::flush_fragments<float>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
template void SL_TriRasterizer::flush_fragments<double>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;



/*--------------------------------------
 * Wireframe Rasterization
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::render_wireframe(const SL_Texture* depthBuffer) const noexcept
{
    constexpr DepthCmpFunc depthCmpFunc;
    const SL_BinCounter<uint32_t>* pBinIds = mBinIds;
    const SL_FragmentBin* pBins = mBins;
    const uint32_t numBins = (uint32_t)mNumBins;

    SL_FragCoord*         outCoords    = mQueues;
    const int32_t         yOffset      = (int32_t)mThreadId;
    const int32_t         increment    = (int32_t)mNumProcessors;
    SL_ScanlineBounds     scanline;

    for (uint32_t i = 0; i < numBins; ++i)
    {
        const uint32_t binId = pBinIds[i].count;
        const SL_FragmentBin* pBin = pBins+binId;

        uint32_t          numQueuedFrags = 0;
        const math::vec4* pPoints        = pBin->mScreenCoords;
        const math::vec4* bcClipSpace    = pBin->mBarycentricCoords;
        const math::vec4  depth          {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};
        const math::vec4  homogenous     {pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
        const int32_t     bboxMinY       = (int32_t)math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     scanLineOffset = bboxMinY + sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);
        const int32_t     bboxMaxY       = (int32_t)math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]);

        scanline.init(pPoints[0], pPoints[1], pPoints[2]);

        for (int32_t y = scanLineOffset; y < bboxMaxY; y += increment)
        {
            // calculate the bounds of the current scan-line
            const float        yf     = (float)y;
            const math::vec4&& bcY    = math::fmadd(bcClipSpace[1], math::vec4{yf, yf, yf, 0.f}, bcClipSpace[2]);

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as a
            // guard against any pixels we don't want to render.
            int32_t xMinMax0[2], xMinMax1[2];
            scanline.step(yf, xMinMax0[0], xMinMax0[1]);
            scanline.step(yf-1.f, xMinMax1[0], xMinMax1[1]);
            const int32_t d0 = math::max(math::abs(xMinMax0[0]-xMinMax1[0]), 1);
            const int32_t d1 = math::max(math::abs(xMinMax0[1]-xMinMax1[1]), 1);

            const depth_type* const pDepth = depthBuffer->row_pointer<depth_type>(y);

            for (int32_t ix = 0, x = xMinMax0[0]; x < xMinMax0[1]; ++ix, ++x)
            {
                // skip to the start of the next horizontal edge
                if (LS_UNLIKELY(ix == d0))
                {
                    x = math::max(xMinMax0[0], xMinMax0[1]-d1-1);
                    continue;
                }

                // calculate barycentric coordinates
                const float   xf = (float)x;
                math::vec4&&  bc = math::fmadd(bcClipSpace[0], math::vec4{xf, xf, xf, 0.f}, bcY);
                const float   z  = math::dot(depth, bc);
                const float   d  = _sl_get_depth_texel<depth_type>(pDepth+x);

                const int_fast32_t&& depthTest = depthCmpFunc(z, d);

                if (LS_UNLIKELY(!depthTest))
                {
                    continue;
                }

                // perspective correction
                float persp = math::rcp(math::dot(bc, homogenous));
                outCoords->bc[numQueuedFrags]    = (bc * homogenous) * persp;
                outCoords->coord[numQueuedFrags] = {(uint16_t)x, (uint16_t)y, z};
                ++numQueuedFrags;

                if (numQueuedFrags == SL_SHADER_MAX_QUEUED_FRAGS)
                {
                    numQueuedFrags = 0;
                    flush_fragments<depth_type>(pBin, SL_SHADER_MAX_QUEUED_FRAGS, outCoords);

                    LS_PREFETCH(pBin+1, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_NONTEMPORAL);
                }
            }
        }

        // cleanup remaining fragments
        if (numQueuedFrags > 0)
        {
            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
        }
    }
}


 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, double>(const SL_Texture*) const noexcept;



/*--------------------------------------
 * Triangle Rasterization, scalar
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::render_triangle(const SL_Texture* depthBuffer) const noexcept
{
    constexpr DepthCmpFunc depthCmpFunc;
    const SL_BinCounter<uint32_t>* pBinIds = mBinIds;
    const SL_FragmentBin* pBins = mBins;
    const uint32_t numBins = (uint32_t)mNumBins;

    SL_FragCoord*         outCoords    = mQueues;
    const int32_t         yOffset      = (int32_t)mThreadId;
    const int32_t         increment    = (int32_t)mNumProcessors;
    SL_ScanlineBounds     scanline;

    for (uint32_t i = 0; i < numBins; ++i)
    {
        const uint32_t binId = pBinIds[i].count;
        const SL_FragmentBin* pBin = pBins+binId;

        uint32_t          numQueuedFrags = 0;
        const math::vec4* pPoints        = pBin->mScreenCoords;
        const math::vec4  depth          {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};
        const math::vec4  homogenous     {pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
        const int32_t     bboxMinY       = (int32_t)math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     bboxMaxY       = (int32_t)math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     scanLineOffset = bboxMinY + sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);
        const math::vec4* bcClipSpace    = pBin->mBarycentricCoords;

        int32_t y = scanLineOffset;
        if (y >= bboxMaxY)
        {
            continue;
        }

        scanline.init(pPoints[0], pPoints[1], pPoints[2]);

        do
        {
            // calculate the bounds of the current scan-line
            const float yf = (float)y;
            const math::vec4&& bcY = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as
            // a guard against any pixels we don't want to render.
            int32_t x;
            int32_t xMax;
            scanline.step(yf, x, xMax);

            if (LS_UNLIKELY(x >= xMax))
            {
                y += increment;
                continue;
            }

            math::vec4&& xf{(float)x};
            math::vec4&& bcX = math::fmadd(bcClipSpace[0], xf, bcY);
            const depth_type* pDepth = depthBuffer->row_pointer<depth_type>(y) + x;

            do
            {
                // calculate barycentric coordinates
                const float d  = _sl_get_depth_texel<depth_type>(pDepth);
                const float z  = math::dot(depth, bcX);
                const int_fast32_t&& depthTest = depthCmpFunc(z, d);

                if (LS_LIKELY(depthTest))
                {
                    // perspective correction
                    const math::vec4&& bc    = bcX * homogenous;
                    const math::vec4&& persp = {math::sum_inv(bc)};

                    outCoords->bc[numQueuedFrags]          = bc * persp;
                    outCoords->coord[numQueuedFrags].x     = (uint16_t)x;
                    outCoords->coord[numQueuedFrags].y     = (uint16_t)y;
                    outCoords->coord[numQueuedFrags].depth = z;

                    ++numQueuedFrags;

                    if (LS_UNLIKELY(numQueuedFrags == SL_SHADER_MAX_QUEUED_FRAGS))
                    {
                        numQueuedFrags = 0;
                        flush_fragments<depth_type>(pBin, SL_SHADER_MAX_QUEUED_FRAGS, outCoords);
                    }
                }

                bcX += bcClipSpace[0];
                ++x;
                ++pDepth;
            } while (LS_UNLIKELY(x < xMax));

            y += increment;
        } while (LS_UNLIKELY(y < bboxMaxY));

        // cleanup remaining fragments
        if (LS_LIKELY(numQueuedFrags > 0))
        {
            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
        }
    }
}



 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, double>(const SL_Texture*) const noexcept;



/*-------------------------------------
 * Render a triangle using 4 elements at a time
-------------------------------------*/
#if defined(LS_X86_AVX2)



inline LS_INLINE __m128 _sl_mul_vec4_mat4_ps(const __m128 v, const __m128 m[4]) noexcept
{
    const __m128 row0 = _mm_mul_ps(v, m[0]);
    const __m128 row1 = _mm_mul_ps(v, m[1]);
    const __m128 row2 = _mm_mul_ps(v, m[2]);
    const __m128 row3 = _mm_mul_ps(v, m[3]);

    // transpose, then add
    const __m128 t0 = _mm_unpacklo_ps(row0, row1);
    const __m128 t1 = _mm_unpacklo_ps(row2, row3);
    const __m128 t2 = _mm_unpackhi_ps(row0, row1);
    const __m128 t3 = _mm_unpackhi_ps(row2, row3);

    __m128 sum0 = _mm_add_ps(_mm_movehl_ps(t1, t0), _mm_movelh_ps(t0, t1));
    __m128 sum1 = _mm_add_ps(_mm_movehl_ps(t3, t2), _mm_movelh_ps(t2, t3));

    return _mm_add_ps(sum1, sum0);
}



inline LS_INLINE void _sl_vec4_outer_ps(const __m128 v1, const __m128 v2, __m128 ret[4]) noexcept
{
    const __m128 a = _mm_permute_ps(v1, 0x00);
    const __m128 b = _mm_permute_ps(v1, 0x55);
    const __m128 c = _mm_permute_ps(v1, 0xAA);
    const __m128 d = _mm_permute_ps(v1, 0xFF);

    ret[0] = _mm_mul_ps(a, v2);
    ret[1] = _mm_mul_ps(b, v2);
    ret[2] = _mm_mul_ps(c, v2);
    ret[3] = _mm_mul_ps(d, v2);
}



template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::render_triangle_simd(const SL_Texture* depthBuffer) const noexcept
{
    constexpr DepthCmpFunc         depthCmpFunc;
    const SL_BinCounter<uint32_t>* pBinIds = mBinIds;
    const SL_FragmentBin* const    pBins   = mBins;
    const uint32_t                 numBins = (uint32_t)mNumBins;

    SL_FragCoord*     outCoords    = mQueues;
    const int32_t     yOffset      = (int32_t)mThreadId;
    const int32_t     increment    = (int32_t)mNumProcessors;
    SL_ScanlineBounds scanline;

    for (uint32_t i = 0; i < numBins; ++i)
    {
        const uint32_t binId = pBinIds[i].count;
        const uint32_t binId2 = pBinIds[i+(i < numBins)].count;
        const SL_FragmentBin* const pBin = pBins+binId;
        LS_PREFETCH(pBins+binId2, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L3);

        const __m128 points0 = _mm_load_ps(reinterpret_cast<const float*>(pBin->mScreenCoords+0));
        const __m128 points1 = _mm_load_ps(reinterpret_cast<const float*>(pBin->mScreenCoords+1));
        const __m128 points2 = _mm_load_ps(reinterpret_cast<const float*>(pBin->mScreenCoords+2));

        const int32_t bboxMinY       = _mm_extract_epi32(_mm_cvtps_epi32(_mm_min_ps(_mm_min_ps(points0, points1), points2)), 1);
        const int32_t bboxMaxY       = _mm_extract_epi32(_mm_cvtps_epi32(_mm_max_ps(_mm_max_ps(points0, points1), points2)), 1);
        const int32_t scanLineOffset = bboxMinY + sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        int32_t y = scanLineOffset;
        if (LS_UNLIKELY(y >= bboxMaxY))
        {
            continue;
        }

        scanline.init(math::vec4{points0}, math::vec4{points1}, math::vec4{points2});

        const __m128 d01        = _mm_unpackhi_ps(points0, points1);
        const __m128 h01        = _mm_insert_ps(_mm_permute_ps(points0, 0xFF), points1, 0xD0);
        const __m128 depth      = _mm_insert_ps(d01, points2, 0xA8);
        const __m128 homogenous = _mm_insert_ps(h01, points2, 0xE8);

        const __m128 bcClipSpace0   = _mm_load_ps(reinterpret_cast<const float*>(pBin->mBarycentricCoords+0));
        const __m128 bcClipSpace1   = _mm_load_ps(reinterpret_cast<const float*>(pBin->mBarycentricCoords+1));
        const __m128 bcClipSpace2   = _mm_load_ps(reinterpret_cast<const float*>(pBin->mBarycentricCoords+2));
        unsigned     numQueuedFrags = 0;

        do
        {
            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as a
            // guard against any pixels we don't want to render.
            __m128i xMin;
            __m128i xMax;
            // calculate the bounds of the current scan-line
            const __m128 yf = _mm_cvtepi32_ps(_mm_set1_epi32(y));
            scanline.step(yf, xMin, xMax);

            if (LS_UNLIKELY(!_mm_test_all_ones(_mm_cmplt_epi32(xMin, xMax))))
            {
                y += increment;
                continue;
            }

            const int32_t     y16    = y << 16;
            const depth_type* pDepth = depthBuffer->row_pointer<depth_type>((uintptr_t)y) + _mm_cvtsi128_si32(xMin);
            const __m128      bcY    = _mm_fmadd_ps(bcClipSpace1, yf, bcClipSpace2);
            __m128i           x4     = _mm_add_epi32(_mm_set_epi32(3, 2, 1, 0), xMin);
            const __m128i     xMax4  = xMax;

            __m128 bc[4];
            _sl_vec4_outer_ps(_mm_cvtepi32_ps(x4), bcClipSpace0, bc);
            bc[0] = _mm_add_ps(bc[0], bcY);
            bc[1] = _mm_add_ps(bc[1], bcY);
            bc[2] = _mm_add_ps(bc[2], bcY);
            bc[3] = _mm_add_ps(bc[3], bcY);
            const __m128 bcX = _mm_mul_ps(bcClipSpace0, _mm_set1_ps(4.f));

            do
            {
                // calculate barycentric coordinates and perform a depth test
                const __m128  xBound    = _mm_castsi128_ps(_mm_cmplt_epi32(x4, xMax4));
                const __m128  z         = _sl_mul_vec4_mat4_ps(depth, bc);
                const __m128  d         = _sl_get_depth_texel4<depth_type>(pDepth).simd;
                const int32_t depthTest = _mm_movemask_ps(_mm_and_ps(xBound, depthCmpFunc(z, d)));

                if (LS_LIKELY(depthTest))
                {
                    const unsigned storeMask1  = (unsigned)_mm_popcnt_u32((unsigned)depthTest & 0x01u) + numQueuedFrags;
                    const unsigned storeMask2  = (unsigned)_mm_popcnt_u32((unsigned)depthTest & 0x03u) + numQueuedFrags;
                    const unsigned storeMask3  = (unsigned)_mm_popcnt_u32((unsigned)depthTest & 0x07u) + numQueuedFrags;
                    const unsigned rasterCount = (unsigned)_mm_popcnt_u32((unsigned)depthTest & 0x0Fu);

                    {
                        //const __m128 xy = _mm_castsi128_ps(_mm_or_si128(_mm_and_si128(x4, _mm_set1_epi32(0x0000FFFF)), _mm_slli_epi32(_mm_set1_epi32(y), 16)));
                        const __m128i xy   = _mm_or_si128(x4, _mm_set1_epi32(y16));
                        const __m128i xyz0 = _mm_unpacklo_epi32(xy, _mm_castps_si128(z));
                        const __m128i xyz1 = _mm_unpackhi_epi32(xy, _mm_castps_si128(z));

                        _mm_storel_pd(reinterpret_cast<double*>(outCoords->coord + numQueuedFrags), _mm_castsi128_pd(xyz0));
                        _mm_storeh_pd(reinterpret_cast<double*>(outCoords->coord + storeMask1),     _mm_castsi128_pd(xyz0));
                        _mm_storel_pd(reinterpret_cast<double*>(outCoords->coord + storeMask2),     _mm_castsi128_pd(xyz1));
                        _mm_storeh_pd(reinterpret_cast<double*>(outCoords->coord + storeMask3),     _mm_castsi128_pd(xyz1));
                    }

                    {
                        __m128 bc0 = _mm_mul_ps(homogenous, bc[0]);
                        __m128 bc1 = _mm_mul_ps(homogenous, bc[1]);
                        __m128 bc2 = _mm_mul_ps(homogenous, bc[2]);
                        __m128 bc3 = _mm_mul_ps(homogenous, bc[3]);

                        // transpose, then add
                        const __m128 t0 = _mm_unpacklo_ps(bc0, bc1);
                        const __m128 t1 = _mm_unpacklo_ps(bc2, bc3);
                        const __m128 t2 = _mm_unpackhi_ps(bc0, bc1);
                        const __m128 t3 = _mm_unpackhi_ps(bc2, bc3);

                        __m128 sum0 = _mm_movehl_ps(t1, t0);
                        __m128 sum1 = _mm_movehl_ps(t3, t2);
                        sum0 = _mm_add_ps(sum0, _mm_movelh_ps(t0, t1));
                        sum1 = _mm_add_ps(sum1, _mm_movelh_ps(t2, t3));

                        const __m128 persp4 = _mm_rcp_ps(_mm_add_ps(sum1, sum0));
                        const __m128 persp0 = _mm_permute_ps(persp4, 0x00);
                        const __m128 persp1 = _mm_permute_ps(persp4, 0x55);
                        const __m128 persp2 = _mm_permute_ps(persp4, 0xAA);
                        const __m128 persp3 = _mm_permute_ps(persp4, 0xFF);

                        bc0 = _mm_mul_ps(bc0, persp0);
                        bc1 = _mm_mul_ps(bc1, persp1);
                        bc2 = _mm_mul_ps(bc2, persp2);
                        bc3 = _mm_mul_ps(bc3, persp3);

                        _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + numQueuedFrags), bc0);
                        _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + storeMask1),     bc1);
                        _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + storeMask2),     bc2);
                        _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + storeMask3),     bc3);
                    }

                    numQueuedFrags += rasterCount;
                    if (LS_UNLIKELY(numQueuedFrags > SL_SHADER_MAX_QUEUED_FRAGS - 4))
                    {
                        flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
                        numQueuedFrags = 0;
                    }
                }

                bc[0] = _mm_add_ps(bc[0], bcX);
                bc[1] = _mm_add_ps(bc[1], bcX);
                bc[2] = _mm_add_ps(bc[2], bcX);
                bc[3] = _mm_add_ps(bc[3], bcX);

                x4 = _mm_add_epi32(x4, _mm_set1_epi32(4));

                pDepth += 4;
            }
            while (_mm_movemask_epi8(_mm_cmplt_epi32(x4, xMax4)));

            y += increment;
        }
        while (LS_UNLIKELY(y < bboxMaxY));

        if (LS_LIKELY(0 < numQueuedFrags))
        {
            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
        }
    }
}



#else



inline LS_INLINE ls::math::vec4_t<int> _sl_cmp_vec4_lt(const ls::math::vec4_t<int>& a, const ls::math::vec4_t<int>& b) noexcept
{
    return ls::math::vec4_t<int>{
        a[0] < b[0],
        a[1] < b[1],
        a[2] < b[2],
        a[3] < b[3]
    };
}



template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::render_triangle_simd(const SL_Texture* depthBuffer) const noexcept
{
    constexpr DepthCmpFunc         depthCmpFunc;
    const SL_BinCounter<uint32_t>* pBinIds = mBinIds;
    const SL_FragmentBin* const    pBins   = mBins;
    const uint32_t                 numBins = (uint32_t)mNumBins;

    SL_FragCoord*     outCoords    = mQueues;
    const int32_t     yOffset      = (int32_t)mThreadId;
    const int32_t     increment    = (int32_t)mNumProcessors;
    SL_ScanlineBounds scanline;

    for (uint32_t i = 0; i < numBins; ++i)
    {
        const uint32_t binId = pBinIds[i].count;
        const SL_FragmentBin* pBin = pBins+binId;

        unsigned          numQueuedFrags = 0;
        const math::vec4* pPoints        = pBin->mScreenCoords;
        const math::vec4  depth          {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};
        const math::vec4  homogenous     {pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
        const int32_t     bboxMinY       = (int32_t)math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     bboxMaxY       = (int32_t)math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     scanLineOffset = bboxMinY + sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);
        const math::vec4* bcClipSpace    = pBin->mBarycentricCoords;

        int32_t y = scanLineOffset;
        if (LS_UNLIKELY(y >= bboxMaxY))
        {
            continue;
        }

        scanline.init(pPoints[0], pPoints[1], pPoints[2]);

        do
        {
            // calculate the bounds of the current scan-line
            const float yf = (float)y;

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as a
            // guard against any pixels we don't want to render.
            int32_t xMin;
            int32_t xMax;
            scanline.step(yf, xMin, xMax);

            if (LS_UNLIKELY(xMin < xMax))
            {
                const depth_type*  pDepth = depthBuffer->row_pointer<depth_type>((uintptr_t)y) + xMin;
                const math::vec4&& bcY    = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);
                math::vec4i&&      x4     = math::vec4i{0, 1, 2, 3} + xMin;
                const math::vec4i  xMax4  {xMax};
                math::mat4&&       bc     = math::outer((math::vec4)x4, bcClipSpace[0]) + bcY;
                const math::vec4&& bcX    = bcClipSpace[0] * 4.f;

                do
                {
                    // calculate barycentric coordinates and perform a depth test
                    const math::vec4i&& xBound = _sl_cmp_vec4_lt(x4, xMax4);
                    const math::vec4&&  d      = _sl_get_depth_texel4<depth_type>(pDepth);
                    const math::vec4&&  z      = depth * bc;

                    math::vec4i&& storeMask4 = depthCmpFunc(z, d);
                    storeMask4[0] &= xBound[0];
                    storeMask4[1] &= xBound[1];
                    storeMask4[2] &= xBound[2];
                    storeMask4[3] &= xBound[3];

                    if (LS_LIKELY(storeMask4 != 0))
                    {
                        const unsigned storeMask0 = numQueuedFrags;
                        const unsigned storeMask1 = storeMask4[0]+storeMask0;
                        const unsigned storeMask2 = storeMask4[1]+storeMask1;
                        const unsigned storeMask3 = storeMask4[2]+storeMask2;

                        {
                            const uint16_t y16 = (uint16_t)y;

                            outCoords->coord[storeMask0] = SL_FragCoordXYZ{(uint16_t)x4.v[0], y16, z.v[0]};
                            outCoords->coord[storeMask1] = SL_FragCoordXYZ{(uint16_t)x4.v[1], y16, z.v[1]};
                            outCoords->coord[storeMask2] = SL_FragCoordXYZ{(uint16_t)x4.v[2], y16, z.v[2]};
                            outCoords->coord[storeMask3] = SL_FragCoordXYZ{(uint16_t)x4.v[3], y16, z.v[3]};
                        }

                        {
                            math::mat4&& bcH = {
                                homogenous * bc[0],
                                homogenous * bc[1],
                                homogenous * bc[2],
                                homogenous * bc[3]
                            };

                            // transpose, then add
                            math::mat4&& t = math::transpose(bcH);
                            math::vec4&& persp4 = math::rcp(math::sum<math::vec4>(t[0], t[1], t[2], t[3]));

                            t = math::mat_row_mul(bcH, persp4);

                            outCoords->bc[storeMask0] = t[0];
                            outCoords->bc[storeMask1] = t[1];
                            outCoords->bc[storeMask2] = t[2];
                            outCoords->bc[storeMask3] = t[3];
                        }

                        numQueuedFrags += math::sum(storeMask4);
                        if (LS_UNLIKELY(numQueuedFrags > SL_SHADER_MAX_QUEUED_FRAGS - 4))
                        {
                            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
                            numQueuedFrags = 0;
                        }
                    }

                    pDepth += 4;
                    bc += bcX;
                    x4 += 4;
                }
                while (x4.v[0] < xMax);
            }

            y += increment;
        }
        while (y < bboxMaxY);

        if (LS_LIKELY(0 < numQueuedFrags))
        {
            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
        }
    }
}



#endif



 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, double>(const SL_Texture*) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, ls::math::half>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, float>(const SL_Texture*) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, double>(const SL_Texture*) const noexcept;



/*-------------------------------------
 * Dispatch the fragment processor with the correct depth-comparison function
-------------------------------------*/
template <class DepthCmpFunc>
void SL_TriRasterizer::dispatch_bins() noexcept
{
    const uint16_t depthBpp = mFbo->get_depth_buffer()->bpp();

    switch(mMode)
    {
        case RENDER_MODE_TRI_WIRE:
        case RENDER_MODE_INDEXED_TRI_WIRE:
            if (depthBpp == sizeof(math::half))
            {
                render_wireframe<DepthCmpFunc, math::half>(mFbo->get_depth_buffer());
            }
            else if (depthBpp == sizeof(float))
            {
                render_wireframe<DepthCmpFunc, float>(mFbo->get_depth_buffer());
            }
            else if (depthBpp == sizeof(double))
            {
                render_wireframe<DepthCmpFunc, double>(mFbo->get_depth_buffer());
            }
            break;

        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
            // Triangles assign scan-lines per thread for rasterization.
            // There's No need to subdivide the output framebuffer
            if (depthBpp == sizeof(math::half))
            {
                #if SL_CONSERVE_MEMORY
                    iterate_tri_scanlines<DepthCmpFunc, math::half>();
                #else
                    //render_triangle<DepthCmpFunc, math::half>(mFbo->get_depth_buffer());
                    render_triangle_simd<DepthCmpFunc, math::half>(mFbo->get_depth_buffer());
                #endif
            }
            else if (depthBpp == sizeof(float))
            {
                #if SL_CONSERVE_MEMORY
                    iterate_tri_scanlines<DepthCmpFunc, float>();
                #else
                    //render_triangle<DepthCmpFunc, float>(mFbo->get_depth_buffer());
                    render_triangle_simd<DepthCmpFunc, float>(mFbo->get_depth_buffer());
                #endif
            }
            else if (depthBpp == sizeof(double))
            {
                #if SL_CONSERVE_MEMORY
                    iterate_tri_scanlines<DepthCmpFunc, double>();
                #else
                    //render_triangle<DepthCmpFunc, double>(mFbo->get_depth_buffer());
                    render_triangle_simd<DepthCmpFunc, double>(mFbo->get_depth_buffer());
                #endif
            }
            break;

        default:
            LS_DEBUG_ASSERT(false);
            LS_UNREACHABLE();
    }
}



template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncLT>() noexcept;
template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncLE>() noexcept;
template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncGT>() noexcept;
template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncGE>() noexcept;
template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncEQ>() noexcept;
template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncNE>() noexcept;
template void SL_TriRasterizer::dispatch_bins<SL_DepthFuncOFF>() noexcept;



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SL_TriRasterizer::execute() noexcept
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
