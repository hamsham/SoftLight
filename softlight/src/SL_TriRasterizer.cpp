
#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/utils/Assertions.h" // LS_DEBUG_ASSERT

#include "lightsky/math/half.h"
#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_ScanlineBounds.hpp"
#include "softlight/SL_Shader.hpp" // SL_FragmentShader
#include "softlight/SL_ShaderProcessor.hpp" // SL_FragmentBin
#include "softlight/SL_ShaderUtil.hpp" // sl_scanline_offset(), SL_BinCounter
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_TriRasterizer.hpp"
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
 * Load and convert a depth texel from memory
--------------------------------------*/
template <typename depth_type>
inline LS_INLINE float _sl_get_depth_texel(const depth_type* LS_RESTRICT_PTR pDepth)
{
    return (float)*pDepth;
}

#if defined(LS_ARCH_X86)
template <>
inline LS_INLINE float _sl_get_depth_texel<float>(const float* LS_RESTRICT_PTR pDepth)
{
    return _mm_cvtss_f32(_mm_load_ss(pDepth));
}

#elif defined(LS_ARM_NEON)
template <>
inline LS_INLINE float _sl_get_depth_texel<math::half>(const math::half* LS_RESTRICT_PTR pDepth)
{
    return (float)(*reinterpret_cast<const __fp16*>(pDepth));
}

#endif



/*--------------------------------------
 * Load and convert 4 depth texels from memory
--------------------------------------*/
template <typename depth_type>
inline LS_INLINE math::vec4 _sl_get_depth_texel4(const depth_type* LS_RESTRICT_PTR pDepth);

#if defined(LS_X86_FP16)
template <>
inline LS_INLINE math::vec4 _sl_get_depth_texel4<math::half>(const math::half* LS_RESTRICT_PTR pDepth)
{
    return math::vec4{_mm_cvtph_ps(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(pDepth)))};
}

template <>
inline LS_INLINE math::vec4 _sl_get_depth_texel4<float>(const float* LS_RESTRICT_PTR pDepth)
{
    return math::vec4{_mm_loadu_ps(pDepth)};
}

#elif defined(LS_ARM_NEON)
template <>
inline LS_INLINE math::vec4 _sl_get_depth_texel4<math::half>(const math::half* LS_RESTRICT_PTR pDepth)
{
    return math::vec4{vcvt_f32_f16(vld1_f16(reinterpret_cast<const __fp16*>(pDepth)))};
}

template <>
inline LS_INLINE math::vec4 _sl_get_depth_texel4<float>(const float* LS_RESTRICT_PTR pDepth)
{
    return math::vec4{vld1q_f32(pDepth)};
}

#endif

template <typename depth_type>
inline LS_INLINE math::vec4 _sl_get_depth_texel4(const depth_type* LS_RESTRICT_PTR pDepth)
{
    return (math::vec4)(*reinterpret_cast<const math::vec4_t<depth_type>*>(pDepth));
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_TriRasterizer Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Wireframe Rasterization
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::render_wireframe(const SL_TextureView& depthBuffer) const noexcept
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
        const SL_FragmentBin& bin = pBins[binId];

        uint32_t          numQueuedFrags = 0;
        const math::vec4* pPoints        = bin.mScreenCoords;
        const int32_t     bboxMinY       = (int32_t)math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     bboxMaxY       = (int32_t)math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     scanLineOffset = sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        scanline.init(pPoints[0], pPoints[1], pPoints[2]);

        const math::vec4* bcClipSpace = bin.mBarycentricCoords;
        const math::vec4  depth       {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};

        for (int32_t y = bboxMinY+scanLineOffset; y < bboxMaxY; y += increment)
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

            const depth_type* const pDepth = (depth_type*)depthBuffer.pTexels + (depthBuffer.width * y);

            for (int32_t ix = 0, x = xMinMax0[0]; (uint32_t)x < (uint32_t)xMinMax0[1]; ++ix, ++x)
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

                outCoords->bc[numQueuedFrags]    = bc;
                outCoords->coord[numQueuedFrags] = {(uint16_t)x, (uint16_t)y, z};
                ++numQueuedFrags;

                if (numQueuedFrags == SL_SHADER_MAX_QUEUED_FRAGS)
                {
                    numQueuedFrags = 0;
                    flush_tri_fragments<depth_type>(bin, SL_SHADER_MAX_QUEUED_FRAGS, outCoords);
                }
            }
        }

        // cleanup remaining fragments
        if (numQueuedFrags > 0)
        {
            flush_tri_fragments<depth_type>(bin, numQueuedFrags, outCoords);
        }
    }
}



 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLT, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncLE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGT, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncGE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncEQ, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncNE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_wireframe<SL_DepthFuncOFF, double>(const SL_TextureView&) const noexcept;



/*--------------------------------------
 * Triangle Rasterization, scalar
--------------------------------------*/
template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::render_triangle(const SL_TextureView& depthBuffer) const noexcept
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
        const SL_FragmentBin& bin = pBins[binId];

        uint32_t          numQueuedFrags = 0;
        const math::vec4* pPoints        = bin.mScreenCoords;
        const int32_t     bboxMinY       = (int32_t)math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     bboxMaxY       = (int32_t)math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     scanLineOffset = sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        int32_t y = bboxMinY + scanLineOffset;
        if (y >= bboxMaxY)
        {
            continue;
        }

        const math::vec4 depth{pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};

        scanline.init(pPoints[0], pPoints[1], pPoints[2]);

        const math::vec4* bcClipSpace = bin.mBarycentricCoords;

        do
        {
            // calculate the bounds of the current scan-line
            const float yf = (float)y;

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as
            // a guard against any pixels we don't want to render.
            int32_t x;
            int32_t xMax;
            scanline.step(yf, x, xMax);

            if (LS_UNLIKELY((uint32_t)x >= (uint32_t)xMax))
            {
                y += increment;
                continue;
            }

            math::vec4&& xf{(float)x};
            const math::vec4&& bcY = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);
            math::vec4&& bcX = math::fmadd(bcClipSpace[0], xf, bcY);
            const depth_type* pDepth = (depth_type*)depthBuffer.pTexels + (x + (int32_t)depthBuffer.width * y);

            do
            {
                // calculate barycentric coordinates
                const float d  = _sl_get_depth_texel<depth_type>(pDepth);
                const float z  = math::dot(depth, bcX);
                const int_fast32_t&& depthTest = depthCmpFunc(z, d);

                if (LS_LIKELY(depthTest))
                {
                    outCoords->bc[numQueuedFrags]          = bcX;
                    outCoords->coord[numQueuedFrags].x     = (uint16_t)x;
                    outCoords->coord[numQueuedFrags].y     = (uint16_t)y;
                    outCoords->coord[numQueuedFrags].depth = z;

                    ++numQueuedFrags;

                    if (LS_UNLIKELY(numQueuedFrags == SL_SHADER_MAX_QUEUED_FRAGS))
                    {
                        numQueuedFrags = 0;
                        flush_tri_fragments<depth_type>(bin, SL_SHADER_MAX_QUEUED_FRAGS, outCoords);
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
            flush_tri_fragments<depth_type>(bin, numQueuedFrags, outCoords);
        }
    }
}



 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLT, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncLE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGT, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncGE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncEQ, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncNE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle<SL_DepthFuncOFF, double>(const SL_TextureView&) const noexcept;



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
void SL_TriRasterizer::render_triangle_simd(const SL_TextureView& LS_RESTRICT_PTR depthBuffer) const noexcept
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
        const SL_FragmentBin& bin = pBins[binId];

        const __m128 points0 = _mm_load_ps(reinterpret_cast<const float*>(bin.mScreenCoords+0));
        const __m128 points1 = _mm_load_ps(reinterpret_cast<const float*>(bin.mScreenCoords+1));
        const __m128 points2 = _mm_load_ps(reinterpret_cast<const float*>(bin.mScreenCoords+2));

        const int32_t bboxMinY       = _mm_extract_epi32(_mm_cvtps_epi32(_mm_min_ps(_mm_min_ps(points0, points1), points2)), 1);
        const int32_t bboxMaxY       = _mm_extract_epi32(_mm_cvtps_epi32(_mm_max_ps(_mm_max_ps(points0, points1), points2)), 1);
        const int32_t scanLineOffset = sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        int32_t y = bboxMinY + scanLineOffset;
        if (LS_UNLIKELY(y >= bboxMaxY))
        {
            continue;
        }

        const __m128 d01   = _mm_unpackhi_ps(points0, points1);
        const __m128 depth = _mm_insert_ps(d01, points2, 0xA8);

        scanline.init(math::vec4{points0}, math::vec4{points1}, math::vec4{points2});

        const __m128 bcClipSpace0   = _mm_load_ps(reinterpret_cast<const float*>(bin.mBarycentricCoords+0));
        const __m128 bcClipSpace1   = _mm_load_ps(reinterpret_cast<const float*>(bin.mBarycentricCoords+1));
        const __m128 bcClipSpace2   = _mm_load_ps(reinterpret_cast<const float*>(bin.mBarycentricCoords+2));
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
            const depth_type* pDepth = (depth_type*)depthBuffer.pTexels + (_mm_cvtsi128_si32(xMin) + (int32_t)depthBuffer.width * y);
            const __m128      bcY    = _mm_fmadd_ps(bcClipSpace1, yf, bcClipSpace2);
            __m128i           x4     = _mm_add_epi32(_mm_set_epi32(3, 2, 1, 0), xMin);

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
                const __m128  xBound    = _mm_castsi128_ps(_mm_cmplt_epi32(x4, xMax));
                const __m128  z         = _sl_mul_vec4_mat4_ps(depth, bc);
                const __m128  d         = _sl_get_depth_texel4<depth_type>(pDepth).simd;
                const __m128  depthTestV = _mm_and_ps(xBound, depthCmpFunc(z, d));
                const int32_t depthTestI = _mm_movemask_ps(depthTestV);

                if (LS_LIKELY(depthTestI))
                {
                    {
                        bc[2] = _mm_blendv_ps(bc[3], bc[2], _mm_permute_ps(depthTestV, 0xAA));
                        bc[1] = _mm_blendv_ps(bc[2], bc[1], _mm_permute_ps(depthTestV, 0x55));
                        bc[0] = _mm_blendv_ps(bc[1], bc[0], _mm_permute_ps(depthTestV, 0x00));

                        _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + numQueuedFrags + 3), bc[3]);
                        _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + numQueuedFrags + 2), bc[2]);
                        _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + numQueuedFrags + 1), bc[1]);
                        _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + numQueuedFrags + 0), bc[0]);
                    }

                    {
                        unsigned storeMask1 = (unsigned)depthTestI & 0x01u;
                        unsigned storeMask2 = (unsigned)depthTestI & 0x03u;
                        unsigned storeMask3 = (unsigned)depthTestI & 0x07u;
                        unsigned storeMask4 = (unsigned)depthTestI & 0x0Fu;

                        //const __m128 xy = _mm_castsi128_ps(_mm_or_si128(_mm_and_si128(x4, _mm_set1_epi32(0x0000FFFF)), _mm_slli_epi32(_mm_set1_epi32(y), 16)));
                        const __m128i xy = _mm_or_si128(x4, _mm_set1_epi32(y16));
                        const __m128i xyz0 = _mm_unpacklo_epi32(xy, _mm_castps_si128(z));
                        const __m128i xyz1 = _mm_unpackhi_epi32(xy, _mm_castps_si128(z));

                        // Interleaving instructions here to help with pipelining
                        storeMask1 += numQueuedFrags;
                        storeMask2 = (unsigned)_mm_popcnt_u32(storeMask2) + numQueuedFrags;
                        storeMask3 = (unsigned)_mm_popcnt_u32(storeMask3) + numQueuedFrags;
                        storeMask4 = (unsigned)_mm_popcnt_u32(storeMask4);

                        _mm_storel_pi(reinterpret_cast<__m64*>(outCoords->coord + numQueuedFrags), _mm_castsi128_ps(xyz0));
                        _mm_storeh_pi(reinterpret_cast<__m64*>(outCoords->coord + storeMask1),     _mm_castsi128_ps(xyz0));
                        _mm_storel_pi(reinterpret_cast<__m64*>(outCoords->coord + storeMask2),     _mm_castsi128_ps(xyz1));
                        _mm_storeh_pi(reinterpret_cast<__m64*>(outCoords->coord + storeMask3),     _mm_castsi128_ps(xyz1));

                        numQueuedFrags += storeMask4;
                    }


                    if (LS_UNLIKELY(numQueuedFrags > SL_SHADER_MAX_QUEUED_FRAGS - 4))
                    {
                        flush_tri_fragments<depth_type>(bin, numQueuedFrags, outCoords);
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
            while (_mm_movemask_epi8(_mm_cmplt_epi32(x4, xMax)));

            y += increment;
        }
        while (LS_UNLIKELY(y < bboxMaxY));

        if (LS_LIKELY(0 < numQueuedFrags))
        {
            flush_tri_fragments<depth_type>(bin, numQueuedFrags, outCoords);
        }
    }
}



#elif defined(LS_ARM_NEON)



inline LS_INLINE int32_t _sl_bbox_min_y(const int32x4_t& points) noexcept
{
    int32x2_t lo   = vget_low_s32(points);
    int32x2_t hi   = vdup_lane_s32(vget_high_s32(points), 0);
    int32x2_t min0 = vmin_s32(lo, hi);
    int32x2_t min1 = vpmin_s32(min0, min0);
    return vget_lane_s32(min1, 0);
}



inline LS_INLINE int32_t _sl_bbox_max_y(const int32x4_t& points) noexcept
{
    int32x2_t lo   = vget_low_s32(points);
    int32x2_t hi   = vdup_lane_s32(vget_high_s32(points), 0);
    int32x2_t max0 = vmax_s32(lo, hi);
    int32x2_t max1 = vpmax_s32(max0, max0);
    return vget_lane_s32(max1, 0);
}



inline LS_INLINE float32x4_t _sl_mul_vec4_mat4_ps(const float32x4_t& v, const float32x4x4_t& m) noexcept
{
    #if defined(LS_ARCH_AARCH64)
        const float32x4_t aq = vmulq_f32(m.val[0], v);
        const float32x4_t bq = vmulq_f32(m.val[1], v);
        const float32x4_t cq = vmulq_f32(m.val[2], v);
        const float32x4_t dq = vmulq_f32(m.val[3], v);

        float32x4_t ret = vdupq_n_f32(vaddvq_f32(aq));
        ret = vsetq_lane_f32(vaddvq_f32(bq), ret, 1);
        ret = vsetq_lane_f32(vaddvq_f32(cq), ret, 2);
        ret = vsetq_lane_f32(vaddvq_f32(dq), ret, 3);

    #else
        const float32x4_t aq = vmulq_f32(m.val[0], v);
        const float32x4_t bq = vmulq_f32(m.val[1], v);
        const float32x4_t cq = vmulq_f32(m.val[2], v);
        const float32x4_t dq = vmulq_f32(m.val[3], v);

        const float32x2_t ad = vadd_f32(vget_high_f32(aq), vget_low_f32(aq));
        const float32x2_t bd = vadd_f32(vget_high_f32(bq), vget_low_f32(bq));
        const float32x2_t cd = vadd_f32(vget_high_f32(cq), vget_low_f32(cq));
        const float32x2_t dd = vadd_f32(vget_high_f32(dq), vget_low_f32(dq));

        float32x4_t ret = vcombine_f32(vpadd_f32(ad, ad), vpadd_f32(cd, cd));
        ret = vsetq_lane_f32(vget_lane_f32(vpadd_f32(bd, bd), 0), ret, 1);
        ret = vsetq_lane_f32(vget_lane_f32(vpadd_f32(dd, dd), 0), ret, 3);

    #endif

    return ret;
}



inline LS_INLINE void _sl_vec4_outer_ps(const float32x4_t& v1, const float32x4_t& v2, float32x4x4_t& ret) noexcept
{
    #if defined(LS_ARCH_AARCH64)
        ret.val[0] = vmulq_laneq_f32(v2, v1, 0);
        ret.val[1] = vmulq_laneq_f32(v2, v1, 1);
        ret.val[2] = vmulq_laneq_f32(v2, v1, 2);
        ret.val[3] = vmulq_laneq_f32(v2, v1, 3);

    #else
        const float32x4_t a = vdupq_lane_f32(vget_low_f32(v1), 0);
        const float32x4_t b = vdupq_lane_f32(vget_low_f32(v1), 1);
        const float32x4_t c = vdupq_lane_f32(vget_high_f32(v1), 0);
        const float32x4_t d = vdupq_lane_f32(vget_high_f32(v1), 1);
        ret.val[0] = vmulq_f32(a, v2);
        ret.val[1] = vmulq_f32(b, v2);
        ret.val[2] = vmulq_f32(c, v2);
        ret.val[3] = vmulq_f32(d, v2);
    #endif
}



template <class DepthCmpFunc, typename depth_type>
void SL_TriRasterizer::render_triangle_simd(const SL_TextureView& depthBuffer) const noexcept
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
        const SL_FragmentBin& bin = pBins[binId];
        unsigned numQueuedFrags = 0;

        const float32x4x4_t points         = vld4q_f32(reinterpret_cast<const float*>(bin.mScreenCoords));
        const int32x4_t     pointsY        = vcvtq_s32_f32(points.val[1]);
        const int32_t       bboxMinY       = _sl_bbox_min_y(pointsY);
        const int32_t       bboxMaxY       = _sl_bbox_max_y(pointsY);
        const int32_t       scanLineOffset = sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        int32_t y = bboxMinY + scanLineOffset;
        if (LS_UNLIKELY(y >= bboxMaxY))
        {
            continue;
        }

        const float32x4_t depth = vsetq_lane_f32(0.f, points.val[2], 3);

        scanline.init(points);

        const float32x4x3_t bcClipSpace = {
            vld1q_f32(reinterpret_cast<const float*>(bin.mBarycentricCoords + 0)),
            vld1q_f32(reinterpret_cast<const float*>(bin.mBarycentricCoords + 1)),
            vld1q_f32(reinterpret_cast<const float*>(bin.mBarycentricCoords + 2))
        };

        do
        {
            // calculate the bounds of the current scan-line
            const float32x4_t yf = vdupq_n_f32((float)y);

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as a
            // guard against any pixels we don't want to render.
            int32x4_t xMin;
            int32x4_t xMax;
            scanline.step(yf, xMin, xMax);

            if (LS_LIKELY(vgetq_lane_u32(vcltq_s32(xMin, xMax), 0)))
            {
                constexpr int32_t indices[4] = {0, 1, 2, 3};
                const depth_type* pDepth = (depth_type*)depthBuffer.pTexels + (vgetq_lane_s32(xMin, 0) + (int32_t)depthBuffer.width * y);
                const float32x4_t bcY    = vmlaq_f32(bcClipSpace.val[2], bcClipSpace.val[1], yf);
                int32x4_t         x4     = vaddq_s32(vld1q_s32(indices), xMin);
                const int32x4_t   xMax4  = xMax;
                const float32x4_t bcX    = vmulq_f32(bcClipSpace.val[0], vdupq_n_f32(4.f));

                float32x4x4_t bc;
                _sl_vec4_outer_ps(vcvtq_f32_s32(x4), bcClipSpace.val[0], bc);
                bc.val[0] = vaddq_f32(bc.val[0], bcY);
                bc.val[1] = vaddq_f32(bc.val[1], bcY);
                bc.val[2] = vaddq_f32(bc.val[2], bcY);
                bc.val[3] = vaddq_f32(bc.val[3], bcY);

                do
                {
                    // calculate barycentric coordinates and perform a depth test
                    const uint32x4_t  xBound     = vshrq_n_u32(vcltq_s32(x4, xMax4), 31);
                    const float32x4_t d          = _sl_get_depth_texel4<depth_type>(pDepth).simd;
                    const float32x4_t z          = _sl_mul_vec4_mat4_ps(depth, bc);
                    const uint32x4_t  storeMask4 = vandq_u32(xBound, vreinterpretq_u32_f32(depthCmpFunc(z, d)));
                    const uint32x2_t  boundsTest = vorr_u32(vget_low_u32(storeMask4), vget_high_u32(storeMask4));

                    if (LS_LIKELY(vget_lane_u64(vreinterpret_u64_u32(boundsTest), 0) != 0))
                    {
                        const unsigned storeMask0 = numQueuedFrags;
                        const unsigned storeMask1 = vgetq_lane_u32(storeMask4, 0) + storeMask0;
                        const unsigned storeMask2 = vgetq_lane_u32(storeMask4, 1) + storeMask1;
                        const unsigned storeMask3 = vgetq_lane_u32(storeMask4, 2) + storeMask2;

                        {
                            const int16x4x2_t xy16 = vtrn_s16(vmovn_s32(x4), vdup_n_s16((int16_t)y));
                            const int32x4_t   xy32 = vcombine_s32(vreinterpret_s32_s16(xy16.val[0]), vreinterpret_s32_s16(xy16.val[1]));
                            const int32x4x2_t xyz  = vtrnq_s32(xy32, vreinterpretq_s32_f32(z));
                            const int64x2_t   xyz0 = vreinterpretq_s64_s32(xyz.val[0]);
                            const int64x2_t   xyz1 = vreinterpretq_s64_s32(xyz.val[1]);

                            vst1_s64(reinterpret_cast<int64_t*>(outCoords->coord+storeMask0), vget_low_s64(xyz0));
                            vst1_s64(reinterpret_cast<int64_t*>(outCoords->coord+storeMask1), vget_high_s64(xyz0));
                            vst1_s64(reinterpret_cast<int64_t*>(outCoords->coord+storeMask2), vget_low_s64(xyz1));
                            vst1_s64(reinterpret_cast<int64_t*>(outCoords->coord+storeMask3), vget_high_s64(xyz1));
                        }

                        {
                            vst1q_f32(reinterpret_cast<float*>(outCoords->bc+storeMask0), bc.val[0]);
                            vst1q_f32(reinterpret_cast<float*>(outCoords->bc+storeMask1), bc.val[1]);
                            vst1q_f32(reinterpret_cast<float*>(outCoords->bc+storeMask2), bc.val[2]);
                            vst1q_f32(reinterpret_cast<float*>(outCoords->bc+storeMask3), bc.val[3]);
                        }

                        #if defined(LS_ARCH_AARCH64)
                            numQueuedFrags += vaddvq_u32(storeMask4);
                        #else
                        {
                            const uint32x2_t a = vadd_u32(vget_high_u32(storeMask4), vget_low_u32(storeMask4));
                            numQueuedFrags += vget_lane_u32(vpadd_u32(a, a), 0);
                        }
                        #endif

                        if (LS_UNLIKELY(numQueuedFrags > SL_SHADER_MAX_QUEUED_FRAGS - 4))
                        {
                            flush_tri_fragments<depth_type>(bin, numQueuedFrags, outCoords);
                            numQueuedFrags = 0;
                        }
                    }

                    pDepth += 4;
                    bc.val[0] = vaddq_f32(bc.val[0], bcX);
                    bc.val[1] = vaddq_f32(bc.val[1], bcX);
                    bc.val[2] = vaddq_f32(bc.val[2], bcX);
                    bc.val[3] = vaddq_f32(bc.val[3], bcX);
                    x4 = vaddq_s32(x4, vdupq_n_s32(4));
                }
                while (vgetq_lane_u32(vcltq_s32(x4, xMax4), 0));
            }

            y += increment;
        }
        while (y < bboxMaxY);

        if (LS_LIKELY(0 < numQueuedFrags))
        {
            flush_tri_fragments<depth_type>(bin, numQueuedFrags, outCoords);
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
void SL_TriRasterizer::render_triangle_simd(const SL_TextureView& depthBuffer) const noexcept
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
        const SL_FragmentBin& bin = pBins[binId];

        unsigned          numQueuedFrags = 0;
        const math::vec4* pPoints        = bin.mScreenCoords;
        const int32_t     bboxMinY       = (int32_t)math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     bboxMaxY       = (int32_t)math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     scanLineOffset = sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        int32_t y = bboxMinY + scanLineOffset;
        if (LS_UNLIKELY(y >= bboxMaxY))
        {
            continue;
        }

        const math::vec4 depth{pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};

        scanline.init(pPoints[0], pPoints[1], pPoints[2]);

        const math::vec4* bcClipSpace = bin.mBarycentricCoords;

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

            if (LS_LIKELY((uint32_t)xMin < (uint32_t)xMax))
            {
                const depth_type*  pDepth = (depth_type*)depthBuffer.pTexels + (xMin + (int32_t)depthBuffer.width * y);
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

                            outCoords->coord[storeMask0] = SL_FragCoordXYZ{(uint16_t)x4.v[0], y16, z[0]};
                            outCoords->coord[storeMask1] = SL_FragCoordXYZ{(uint16_t)x4.v[1], y16, z[1]};
                            outCoords->coord[storeMask2] = SL_FragCoordXYZ{(uint16_t)x4.v[2], y16, z[2]};
                            outCoords->coord[storeMask3] = SL_FragCoordXYZ{(uint16_t)x4.v[3], y16, z[3]};
                        }

                        {
                            outCoords->bc[storeMask0] = bc[0];
                            outCoords->bc[storeMask1] = bc[1];
                            outCoords->bc[storeMask2] = bc[2];
                            outCoords->bc[storeMask3] = bc[3];
                        }

                        numQueuedFrags += math::sum(storeMask4);
                        if (LS_UNLIKELY(numQueuedFrags > SL_SHADER_MAX_QUEUED_FRAGS - 4))
                        {
                            flush_tri_fragments<depth_type>(bin, numQueuedFrags, outCoords);
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
            flush_tri_fragments<depth_type>(bin, numQueuedFrags, outCoords);
        }
    }
}



#endif



 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLT, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncLE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGT, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncGE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncEQ, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncNE, double>(const SL_TextureView&) const noexcept;

 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, ls::math::half>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, float>(const SL_TextureView&) const noexcept;
 template void SL_TriRasterizer::render_triangle_simd<SL_DepthFuncOFF, double>(const SL_TextureView&) const noexcept;



/*-------------------------------------
 * Dispatch the fragment processor with the correct depth-comparison function
-------------------------------------*/
template <class DepthCmpFunc>
void SL_TriRasterizer::dispatch_bins() noexcept
{
    const uint16_t depthBpp = mFbo->get_depth_buffer().bytesPerTexel;

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
                //render_triangle<DepthCmpFunc, math::half>(mFbo->get_depth_buffer());
                render_triangle_simd<DepthCmpFunc, math::half>(mFbo->get_depth_buffer());
            }
            else if (depthBpp == sizeof(float))
            {
                //render_triangle<DepthCmpFunc, float>(mFbo->get_depth_buffer());
                render_triangle_simd<DepthCmpFunc, float>(mFbo->get_depth_buffer());
            }
            else if (depthBpp == sizeof(double))
            {
                //render_triangle<DepthCmpFunc, double>(mFbo->get_depth_buffer());
                render_triangle_simd<DepthCmpFunc, double>(mFbo->get_depth_buffer());
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
