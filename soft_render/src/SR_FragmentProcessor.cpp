
#include <cstdlib> // qsort()

#include "lightsky/setup/Arch.h"

#include "lightsky/math/mat4.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_Color.hpp" // SR_Color... types
#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Namespace setup
-----------------------------------------------------------------------------*/
namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions
-----------------------------------------------------------------------------*/
namespace
{



/*--------------------------------------
 * Interpolate varying variables across a line
--------------------------------------*/
inline void interpolate_line_varyings(
    const float             percent,
    const uint32_t          numVaryings,
    const math::vec4* const inVaryings,
    math::vec4* const       outVaryings
) noexcept
{
    const uint32_t i2 = numVaryings;

    for (uint32_t i = numVaryings; i --> 0;)
    {
        const math::vec4& v0 = inVaryings[i];
        const math::vec4& v1 = inVaryings[i+i2];
        outVaryings[i] = math::mix(v0, v1, percent);
    }
}



/*--------------------------------------
 * Interpolate varying variables across a triangle
--------------------------------------*/
inline void interpolate_tri_varyings(
    const math::vec4&       baryCoords,
    const uint_fast32_t     numVaryings,
    const math::vec4* const inVaryings0,
    math::vec4* const       outVaryings) noexcept
{
    const math::vec4* const inVaryings1 = inVaryings0 + numVaryings;
    const math::vec4* const inVaryings2 = inVaryings1 + numVaryings;
    uint_fast32_t i = numVaryings;

    #ifdef LS_ARCH_X86
        const __m128 bc0 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0x00);
        const __m128 bc1 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0x55);
        const __m128 bc2 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0xAA);

        while (i --> 0u)
        {
            const __m128 v0 = _mm_mul_ps(inVaryings0[i].simd, bc0);
            const __m128 v1 = _mm_mul_ps(inVaryings1[i].simd, bc1);
            const __m128 v2 = _mm_mul_ps(inVaryings2[i].simd, bc2);
            outVaryings[i].simd = _mm_add_ps(_mm_add_ps(v0, v1), v2);
        }
    #elif defined(LS_ARCH_ARM)
        const float32x4_t bc0 = vdupq_lane_f32(vget_low_f32(baryCoords.simd),  0);
        const float32x4_t bc1 = vdupq_lane_f32(vget_low_f32(baryCoords.simd),  1);
        const float32x4_t bc2 = vdupq_lane_f32(vget_high_f32(baryCoords.simd), 0);

        while (i --> 0u)
        {
            const float32x4_t v0 = vmulq_f32(vld1q_f32(inVaryings0[i].v), bc0);
            const float32x4_t v1 = vmulq_f32(vld1q_f32(inVaryings1[i].v), bc1);
            const float32x4_t v2 = vmulq_f32(vld1q_f32(inVaryings2[i].v), bc2);

            outVaryings[i].simd = vaddq_f32(vaddq_f32(v0, v1), v2);
        }
    #else
        const float bc0 = baryCoords.v[0];
        const float bc1 = baryCoords.v[1];
        const float bc2 = baryCoords.v[2];

        while (i --> 0u)
        {
            const math::vec4&& v0 = inVaryings0[i] * bc0;
            const math::vec4&& v1 = inVaryings1[i] * bc1;
            const math::vec4&& v2 = inVaryings2[i] * bc2;
            outVaryings[i] = v0+v1+v2;
        }
    #endif
}



/*-----------------------------------------------------------------------------
 * Quick Sort for binning
-----------------------------------------------------------------------------*/
template <int binCoordIndex>
inline void _sr_frag_bin_qsort_impl(SR_FragmentBin* const bins, const long l, const long r)
{
    SR_FragmentBin temp;

    if (r <= l)
    {
        return;
    }
    else
    {
        const long pivotIndex = (l+r)/2l;
        temp = bins[pivotIndex];
        bins[pivotIndex] = bins[r];
        bins[r] = temp;
    }

    long m = l - 1;
    long n = r;
    const SR_FragmentBin& pivot = bins[r];

    do
    {
        while (bins[++m].mPerspDivide[binCoordIndex] > pivot.mPerspDivide[binCoordIndex]);
        while ((m < n) && (pivot.mPerspDivide[binCoordIndex] > bins[--n].mPerspDivide[binCoordIndex]));

        temp    = bins[m];
        bins[m] = bins[n];
        bins[n] = temp;
    } while (m < n);

    temp    = bins[m];
    bins[m] = bins[r];
    bins[r] = temp;

    _sr_frag_bin_qsort_impl<binCoordIndex>(bins, l, m - 1);
    _sr_frag_bin_qsort_impl<binCoordIndex>(bins, m + 1, r);
}



/* Quick Sort Interface */
inline void sr_frag_bin_qsort(SR_FragmentBin* const bins, long count)
{
    _sr_frag_bin_qsort_impl<0>(bins, 0, count-1);
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SR_FragmentProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Determine if a point can be rendered.
--------------------------------------*/
void SR_FragmentProcessor::render_point(
    SR_Framebuffer* const fbo,
    SR_ColorRGBAf*        pOutputs) noexcept
{
    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const auto              pShader     = fragShader.shader;
    const math::vec4        screenCoord = mBins[mBinId].mScreenCoords[0];
    const math::vec4        fragCoord   {screenCoord[0], screenCoord[1], screenCoord[2], 1.f};
    const math::vec4*       varyings    = mBins[mBinId].mVaryings;
    const float             fboX        = mFboX0;
    const float             fboY        = mFboY0;
    const float             w           = mFboX1;
    const float             h           = mFboY1;

    if (fragCoord.v[0] >= fboX && fragCoord.v[0] <= w && fragCoord.v[1] >= fboY && fragCoord.v[1] <= h)
    {
        const uint16_t x0    = (uint16_t)fragCoord[0];
        const uint16_t y0    = (uint16_t)fragCoord[1];
        const float    depth = fragCoord[2];

        if (fbo->test_depth_pixel(x0, y0, depth))
        {
            const uint16_t z = (uint16_t)depth;
            const uint_fast32_t haveOutputs = pShader(fragCoord, pUniforms, varyings, pOutputs);

            // branchless select
            switch (-haveOutputs & numOutputs)
            {
                case 4: fbo->put_pixel(3, x0, y0, z, pOutputs[3]);
                case 3: fbo->put_pixel(2, x0, y0, z, pOutputs[2]);
                case 2: fbo->put_pixel(1, x0, y0, z, pOutputs[1]);
                case 1: fbo->put_pixel(0, x0, y0, z, pOutputs[0]);
                //case 1: fbo->put_pixel(0, x0, y0, z, SR_ColorRGBAf{1.f, 0, 1.f, 1.f});
                    fbo->put_depth_pixel<float>(x0, y0, depth);
            }
        }
    }
}



/*--------------------------------------
 * Process the line fragments
 *
 * This method of Bresenham's line-drawing algorithm was developed by Will
 * Navidson Yamamushi:
 *     https://gist.github.com/yamamushi/5823518
 *
 * I believe he extracted his implementation from Anthony Thyssen:
 * http://www.ict.griffith.edu.au/anthony/info/graphics/bresenham.procs
 *
 * Anthony Thyssen received his implementation from Bob Pendelton's excerpt in
 * Graphics Gems I (1990):
 * ftp://ftp.isc.org/pub/usenet/comp.sources.unix/volume26/line3d
--------------------------------------*/
void SR_FragmentProcessor::render_line(
    SR_Framebuffer* fbo,
    SR_ColorRGBAf*  pOutputs,
    ls::math::vec4* outVaryings) noexcept
{
    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();
    const uint32_t          numVaryings = fragShader.numVaryings;
    const uint32_t          numOutputs  = fragShader.numOutputs;

    const math::vec4& screenCoord0 = mBins[mBinId].mScreenCoords[0];
    const math::vec4& screenCoord1 = mBins[mBinId].mScreenCoords[1];
    math::vec4* const inVaryings   = mBins[mBinId].mVaryings;

    int   pointX = (int)screenCoord0[0];
    int   pointY = (int)screenCoord0[1];
    float pointZ = screenCoord0[2];

    const float fboX  = mFboX0;
    const float fboY  = mFboY0;
    const float w     = mFboX1;
    const float h     = mFboY1;
    const float dx    = screenCoord1[0] - screenCoord0[0];
    const float dy    = screenCoord1[1] - screenCoord0[1];
    const float dz    = screenCoord1[2] - screenCoord0[2];
    const int   l     = (int)std::abs(dx);
    const int   m     = (int)std::abs(dy);
    const int   n     = (int)std::abs(dz);
    const int   dx2   = l << 1;
    const int   dy2   = m << 1;
    const int   dz2   = n << 1;
    const int   x_inc = (dx < 0.f) ? -1 : 1;
    const int   y_inc = (dy < 0.f) ? -1 : 1;

    int err_1, err_2;

    if ((l >= m) && (l >= n))
    {
        const float percentEnd = std::abs(dx);
        const float z_inc = dz / percentEnd;
        err_1 = dy2 - l;
        err_2 = dz2 - l;

        for (int i = 0; i < l; ++i)
        {
            const bool visible = (pointX >= fboX && pointX <= w) && (pointY >= fboY && pointY <= h);

            if (visible)
            {
                interpolate_line_varyings(pointZ, numVaryings, inVaryings, outVaryings);

                if (fbo->test_depth_pixel(pointX, pointY, pointZ))
                {
                    const math::vec4 fragCoord{(float)pointX, (float)pointY, pointZ, 1.f};

                    if (fragShader.shader(fragCoord, pUniforms, outVaryings, pOutputs))
                    {
                        fbo->put_depth_pixel(pointX, pointY, pointZ);

                        for (std::size_t targetId = 0; targetId < numOutputs; ++targetId)
                        {
                            fbo->put_pixel(targetId, pointX, pointY, 0, pOutputs[targetId]);
                        }
                    }
                }
            }

            if (err_1 > 0)
            {
                pointY += y_inc;
                err_1 -= dx2;
            }

            if (err_2 > 0)
            {
                pointZ += z_inc;
                err_2 -= dx2;
            }

            err_1 += dy2;
            err_2 += dz2;
            pointX += x_inc;
        }
    }
    else if ((m >= l) && (m >= n))
    {
        const float percentEnd = std::abs(dy);
        const float z_inc = dz / percentEnd;
        err_1 = dx2 - m;
        err_2 = dz2 - m;

        for (int i = 0; i < m; ++i)
        {
            const bool visible = (pointX >= fboX && pointX <= w) && (pointY >= fboY && pointY <= h);

            if (visible)
            {
                interpolate_line_varyings(pointZ, numVaryings, inVaryings, outVaryings);

                if (fbo->test_depth_pixel(pointX, pointY, pointZ))
                {
                    const math::vec4 fragCoord{(float)pointX, (float)pointY, pointZ, 1.f};

                    if (fragShader.shader(fragCoord, pUniforms, outVaryings, pOutputs))
                    {
                        fbo->put_depth_pixel(pointX, pointY, pointZ);

                        for (std::size_t targetId = 0; targetId < numOutputs; ++targetId)
                        {
                            fbo->put_pixel(targetId, pointX, pointY, 0, pOutputs[targetId]);
                        }
                    }
                }
            }

            if (err_1 > 0)
            {
                pointX += x_inc;
                err_1 -= dy2;
            }

            if (err_2 > 0)
            {
                pointZ += z_inc;
                err_2 -= dy2;
            }

            err_1 += dx2;
            err_2 += dz2;
            pointY += y_inc;
        }
    }
    else
    {
        const float percentEnd = std::abs(dz);
        const float z_inc = dz / percentEnd;
        err_1 = dy2 - n;
        err_2 = dx2 - n;

        for (int i = 0; i < n; ++i)
        {
            const bool visible = (pointX >= fboX && pointX <= w) && (pointY >= fboY && pointY <= h);

            if (visible)
            {
                interpolate_line_varyings(pointZ, numVaryings, inVaryings, outVaryings);

                if (fbo->test_depth_pixel(pointX, pointY, pointZ))
                {
                    const math::vec4 fragCoord{(float)pointX, (float)pointY, pointZ, 1.f};

                    if (fragShader.shader(fragCoord, pUniforms, outVaryings, pOutputs))
                    {
                        fbo->put_depth_pixel(pointX, pointY, pointZ);

                        for (std::size_t targetId = 0; targetId < numOutputs; ++targetId)
                        {
                            fbo->put_pixel(targetId, pointX, pointY, 0, pOutputs[targetId]);
                        }
                    }
                }
            }

            if (err_1 > 0)
            {
                pointY += y_inc;
                err_1 -= dz2;
            }

            if (err_2 > 0)
            {
                pointX += x_inc;
                err_2 -= dz2;
            }

            err_1 += dy2;
            err_2 += dx2;
            pointZ += z_inc;
        }
    }

    const bool visible = (pointX >= fboX && pointX <= w) && (pointY >= fboY && pointY <= h);

    if (visible)
    {
        interpolate_line_varyings(pointZ, numVaryings, inVaryings, outVaryings);

        if (fbo->test_depth_pixel(pointX, pointY, pointZ))
        {
            const math::vec4 fragCoord{(float)pointX, (float)pointY, pointZ, 1.f};

            if (fragShader.shader(fragCoord, pUniforms, outVaryings, pOutputs))
            {
                fbo->put_depth_pixel(pointX, pointY, pointZ);

                for (std::size_t targetId = 0; targetId < numOutputs; ++targetId)
                {
                    fbo->put_pixel(targetId, pointX, pointY, 0, pOutputs[targetId]);
                }
            }
        }
    }
}



/*--------------------------------------
 * Triangle Rasterization
--------------------------------------*/
#if defined(LS_ARCH_X86)

void SR_FragmentProcessor::render_triangle(
    const SR_Texture* depthBuffer,
    SR_ColorRGBAf*    pOutputs,
    ls::math::vec4*   outVaryings) noexcept
{
    const __m128      persp        = mBins[mBinId].mPerspDivide.simd;
    const math::vec4* screenCoords = mBins[mBinId].mScreenCoords;
    math::vec2        p0           = *reinterpret_cast<const math::vec2*>(screenCoords[0].v);
    math::vec2        p1           = *reinterpret_cast<const math::vec2*>(screenCoords[1].v);
    math::vec2        p2           = *reinterpret_cast<const math::vec2*>(screenCoords[2].v);
    const int32_t     bboxMinX     = math::min(mFboX1, math::max(mFboX0, math::min(p0[0], p1[0], p2[0])));
    const int32_t     bboxMinY     = math::min(mFboY1, math::max(mFboY0, math::min(p0[1], p1[1], p2[1])));
    const int32_t     bboxMaxX     = math::max(mFboX0, math::min(mFboX1, math::max(p0[0], p1[0], p2[0])));
    const int32_t     bboxMaxY     = math::max(mFboY0, math::min(mFboY1, math::max(p0[1], p1[1], p2[1])));
    const math::vec4  depth        {screenCoords[0][2], screenCoords[1][2], screenCoords[2][2], 0.f};
    const __m128      t0[3]        {_mm_set1_ps(p2[0]-p0[0]), _mm_set1_ps(p1[0]-p0[0]), _mm_set1_ps(p0[0])};
    const __m128      t1[3]        {_mm_set1_ps(p2[1]-p0[1]), _mm_set1_ps(p1[1]-p0[1]), _mm_set1_ps(p0[1])};
    const __m128      scaleInv     = _mm_sub_ps(_mm_mul_ps(t0[0], t1[1]), _mm_mul_ps(t0[1], t1[0]));
    const __m128      scale        = _mm_rcp_ps(scaleInv);

    // Don't render triangles which are too small to see
    if (std::fabs(_mm_cvtss_f32(scaleInv)) < 1.f)
    {
        return;
    }

    if (p0[1] < p1[1]) std::swap(p0, p1);
    if (p0[1] < p2[1]) std::swap(p0, p2);
    if (p1[1] < p2[1]) std::swap(p1, p2);

    const float p10x  = p1[0] - p0[0];
    const float p20x  = p2[0] - p0[0];
    const float p21x  = p2[0] - p1[0];
    const float p10y  = math::rcp(p1[1] - p0[1]);
    const float p21y  = math::rcp(p2[1] - p1[1]);
    const float p20y  = math::rcp(p2[1] - p0[1]);
    const float p10xy = p10x * p10y;
    const float p21xy = p21x * p21y;

    unsigned numQueuedFrags = 0;
    SR_FragCoord outCoords[SR_SHADER_MAX_FRAG_QUEUES];

    for (int32_t y = bboxMinY; y <= bboxMaxY; ++y)
    {
        //_mm_prefetch(reinterpret_cast<const char*>(pDepth + (bboxMinX + y * depthWidth)), _MM_HINT_T0);

        // calculate the bounds of the current scan-line
        const float  yf         = (float)y;
        const __m128 ty         = _mm_sub_ps(t1[2], _mm_set1_ps(yf));
        const __m128 t00y       = _mm_mul_ps(t0[0], ty);
        const __m128 t01y       = _mm_mul_ps(t0[1], ty);
        const float  d0         = yf - p0[1];
        const float  d1         = yf - p1[1];
        const float  alpha      = d0 * p20y;
        const int    secondHalf = _mm_movemask_ps(_mm_set_ss(d1));
        int32_t      xMin       = (int32_t)(p0[0] + (p20x * alpha));
        int32_t      xMax       = (int32_t)(secondHalf ? (p1[0] + p21xy * d1) : (p0[0] + p10xy * d0));

        // Get the beginning and end of the scan-line
        if (xMin > xMax) std::swap(xMin, xMax);
        xMin = math::clamp<int32_t>(xMin, bboxMinX, bboxMaxX);
        xMax = math::clamp<int32_t>(xMax, bboxMinX, bboxMaxX);

        for (int32_t x = xMin; x <= xMax; x += 4)
        {
            // calculate barycentric coordinates
            const math::vec4 xf{_mm_cvtepi32_ps(_mm_add_epi32(_mm_set1_epi32(x), _mm_set_epi32(3, 2, 1, 0)))};
            const math::vec4 tx{_mm_sub_ps(t0[2], xf.simd)};
            const math::vec4 u0{_mm_mul_ps(_mm_sub_ps(t01y, _mm_mul_ps(tx.simd, t1[1])), scale)};
            const math::vec4 u1{_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(tx.simd, t1[0]), t00y), scale)};

            //math::vec3   bc {1.f - (u0 + u1), u1, u0};
            const math::mat4&& bcF = math::transpose<float>(
                math::mat4{
                    math::vec4{_mm_sub_ps(_mm_set1_ps(1.f), _mm_add_ps(u0.simd, u1.simd))},
                    u1,
                    u0,
                    _mm_setzero_ps()
                }
            );

            // depth texture lookup will always be slow
            const math::vec4 z              = depth * bcF;
            const __m128     depthBufTexels = depthBuffer->raw_texel4<float>(x, y).simd;
            const int        depthTest      = _mm_movemask_ps(_mm_sub_ps(z.simd, depthBufTexels));
            const int        signBits       = depthTest
                | ((_mm_movemask_ps(bcF[0].simd) != 0) << 0x00)
                | ((_mm_movemask_ps(bcF[1].simd) != 0) << 0x01)
                | ((_mm_movemask_ps(bcF[2].simd) != 0) << 0x02)
                | ((_mm_movemask_ps(bcF[3].simd) != 0) << 0x03);

            for (int32_t i = 0; i < 4; ++i)
            {
                if (signBits & (1 << i))
                {
                    continue;
                }

                // prefetch
                const int32_t y2 = y;
                const float zf = z[i];

                // perspective correction
                __m128 bc4 = math::vec4{bcF[i] * persp}.simd;
                {
                    // horizontal add
                    const __m128 a = bc4;

                    // swap the words of each vector
                    const __m128 b = _mm_shuffle_ps(a, a, 0xB1);
                    const __m128 c = _mm_add_ps(a, b);

                    // swap each half of the vector
                    const __m128 d = _mm_shuffle_ps(c, c, 0x0F);
                    const __m128 e = _mm_add_ps(c, d);

                    bc4 = _mm_mul_ps(bc4, _mm_rcp_ps(e));
                }

                outCoords[numQueuedFrags] = SR_FragCoord{
                    bc4,
                    (uint16_t)(x+i),
                    (uint16_t)y2,
                    zf
                };
                ++numQueuedFrags;

                if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                {
                    numQueuedFrags = 0;
                    flush_fragments(SR_SHADER_MAX_FRAG_QUEUES, outCoords, pOutputs, outVaryings);
                }
            }
        }
    }

    if (numQueuedFrags)
    {
        flush_fragments(numQueuedFrags, outCoords, pOutputs, outVaryings);
    }
}



#elif defined(LS_ARCH_ARM) // Translating x86 into a NEON implementation.

void SR_FragmentProcessor::render_triangle(
    const SR_Texture* depthBuffer,
    SR_ColorRGBAf*    pOutputs,
    ls::math::vec4*   outVaryings) noexcept
{
    const math::vec4  persp        = mBins[mBinId].mPerspDivide;
    const math::vec4* screenCoords = mBins[mBinId].mScreenCoords;
    math::vec2        p0           = *reinterpret_cast<const math::vec2*>(screenCoords[0].v);
    math::vec2        p1           = *reinterpret_cast<const math::vec2*>(screenCoords[1].v);
    math::vec2        p2           = *reinterpret_cast<const math::vec2*>(screenCoords[2].v);
    const int32_t     bboxMinX     = math::min(mFboX1, math::max(mFboX0, math::min(p0[0], p1[0], p2[0])));
    const int32_t     bboxMinY     = math::min(mFboY1, math::max(mFboY0, math::min(p0[1], p1[1], p2[1])));
    const int32_t     bboxMaxX     = math::max(mFboX0, math::min(mFboX1, math::max(p0[0], p1[0], p2[0])));
    const int32_t     bboxMaxY     = math::max(mFboY0, math::min(mFboY1, math::max(p0[1], p1[1], p2[1])));
    const math::vec4  depth        {screenCoords[0][2], screenCoords[1][2], screenCoords[2][2], 0.f};
    const math::vec4  t0[3]        {math::vec4{p2[0]-p0[0]}, math::vec4{p1[0]-p0[0]}, math::vec4{p0[0]}};
    const math::vec4  t1[3]        {math::vec4{p2[1]-p0[1]}, math::vec4{p1[1]-p0[1]}, math::vec4{p0[1]}};
    const math::vec4  scaleInv     = {(t0[0] * t1[1]) - (t0[1] * t1[0])};
    const math::vec4  scale        = math::rcp(scaleInv);

    // Don't render triangles which are too small to see
    if (std::fabs(scaleInv[0]) < 1.f)
    {
        return;
    }

    if (p0[1] < p1[1]) std::swap(p0, p1);
    if (p0[1] < p2[1]) std::swap(p0, p2);
    if (p1[1] < p2[1]) std::swap(p1, p2);

    const float p10x  = p1[0] - p0[0];
    const float p20x  = p2[0] - p0[0];
    const float p21x  = p2[0] - p1[0];
    const float p10y  = math::rcp(p1[1] - p0[1]);
    const float p21y  = math::rcp(p2[1] - p1[1]);
    const float p20y  = math::rcp(p2[1] - p0[1]);
    const float p10xy = p10x * p10y;
    const float p21xy = p21x * p21y;

    unsigned numQueuedFrags = 0;
    SR_FragCoord outCoords[SR_SHADER_MAX_FRAG_QUEUES];

    for (int32_t y = bboxMinY; y <= bboxMaxY; ++y)
    {
        // calculate the bounds of the current scan-line
        const float      yf         = (float)y;
        const math::vec4 ty         = t1[2] - yf;
        const math::vec4 t00y       = t0[0] * ty;
        const math::vec4 t01y       = t0[1] * ty;
        const float      d0         = yf - p0[1];
        const float      d1         = yf - p1[1];
        const float      alpha      = d0 * p20y;
        const int        secondHalf = math::sign_bit(d1);
        int32_t          xMin       = (int32_t)(p0[0] + (p20x * alpha));
        int32_t          xMax       = (int32_t)(secondHalf ? (p1[0] + p21xy * d1) : (p0[0] + p10xy * d0));

        // Get the beginning and end of the scan-line
        if (xMin > xMax) std::swap(xMin, xMax);
        xMin = math::clamp<int32_t>(xMin, bboxMinX, bboxMaxX);
        xMax = math::clamp<int32_t>(xMax, bboxMinX, bboxMaxX);

        for (int32_t x = xMin; x <= xMax; x += 4)
        {
            // calculate barycentric coordinates
            const math::vec4&& xf = (math::vec4)(math::vec4i{x} + math::vec4i{0, 1, 2, 3});
            const math::vec4&& tx = t0[2] - xf;
            const math::vec4&& u0 = (t01y - (tx * t1[1])) * scale;
            const math::vec4&& u1 = ((tx * t1[0]) - t00y) * scale;

            //math::vec3   bc {1.f - (u0 + u1), u1, u0};
            const math::mat4&& bcF = math::transpose<float>(
                math::mat4{
                    math::vec4{math::vec4{1.f} - (u0 + u1)},
                    u1,
                    u0,
                    math::vec4{0.f}
                }
            );

            // depth texture lookup will always be slow
            const math::vec4 z              = depth * bcF;
            const math::vec4 depthBufTexels = depthBuffer->raw_texel4<float>(x, y);
            const int        depthTest      = math::sign_bits(z - depthBufTexels);
            const int        signBits       = depthTest
                                              | ((math::sign_bits(bcF[0]) != 0) << 0x00)
                                              | ((math::sign_bits(bcF[1]) != 0) << 0x01)
                                              | ((math::sign_bits(bcF[2]) != 0) << 0x02)
                                              | ((math::sign_bits(bcF[3]) != 0) << 0x03);

            for (int32_t i = 0; i < 4; ++i)
            {
                if (signBits & (1 << i))
                {
                    continue;
                }

                // prefetch
                const int32_t y2 = y;
                const float zf = z[i];

                // perspective correction
                math::vec4&& bc4 = bcF[i] * persp;
                const float bW = math::rcp(math::sum(bc4));
                bc4 *= bW;

                outCoords[numQueuedFrags] = SR_FragCoord{
                    bc4,
                    (uint16_t)(x+i),
                    (uint16_t)y2,
                    zf
                };
                ++numQueuedFrags;

                if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                {
                    numQueuedFrags = 0;
                    flush_fragments(SR_SHADER_MAX_FRAG_QUEUES, outCoords, pOutputs, outVaryings);
                }
            }
        }
    }

    if (numQueuedFrags)
    {
        flush_fragments(numQueuedFrags, outCoords, pOutputs, outVaryings);
    }
}



#else

void SR_FragmentProcessor::render_triangle(
    const SR_Texture* depthBuffer,
    SR_ColorRGBAf*    pOutputs,
    ls::math::vec4*   outVaryings) noexcept
{
    const math::vec4* screenCoords = mBins[mBinId].mScreenCoords;
    const math::vec4  depth        {screenCoords[0][2], screenCoords[1][2], screenCoords[2][2], 0.f};
    math::vec2        p0           {screenCoords[0][0], screenCoords[0][1]};
    math::vec2        p1           {screenCoords[1][0], screenCoords[1][1]};
    math::vec2        p2           {screenCoords[2][0], screenCoords[2][1]};
    const math::vec4  persp        = mBins[mBinId].mPerspDivide;
    const int32_t     bboxMinX     = math::min(mFboX1, math::max(mFboX0, math::min(p0[0], p1[0], p2[0])));
    const int32_t     bboxMinY     = math::min(mFboY1, math::max(mFboY0, math::min(p0[1], p1[1], p2[1])));
    const int32_t     bboxMaxX     = math::max(mFboX0, math::min(mFboX1, math::max(p0[0], p1[0], p2[0])));
    const int32_t     bboxMaxY     = math::max(mFboY0, math::min(mFboY1, math::max(p0[1], p1[1], p2[1])));
    const float       t0[3]        = {p2[0]-p0[0], p1[0]-p0[0], p0[0]};
    const float       t1[3]        = {p2[1]-p0[1], p1[1]-p0[1], p0[1]};
    const float       scaleInv     = (t0[0] * t1[1]) - (t0[1] * t1[0]);
    const float       scale        = math::rcp(scaleInv);

    // Don't render triangles which are too small to see
    if (std::fabs(scaleInv) < 1.f)
    {
        return;
    }

    if (p0[1] < p1[1]) std::swap(p0, p1);
    if (p0[1] < p2[1]) std::swap(p0, p2);
    if (p1[1] < p2[1]) std::swap(p1, p2);

    const float p10x  = p1[0] - p0[0];
    const float p20x  = p2[0] - p0[0];
    const float p21x  = p2[0] - p1[0];
    const float p10y  = math::rcp(p1[1] - p0[1]);
    const float p21y  = math::rcp(p2[1] - p1[1]);
    const float p20y  = math::rcp(p2[1] - p0[1]);
    const float p10xy = p10x * p10y;
    const float p21xy = p21x * p21y;

    unsigned numQueuedFrags = 0;
    SR_FragCoord outCoords[SR_SHADER_MAX_FRAG_QUEUES];

    for (int32_t y = bboxMinY; y <= bboxMaxY; ++y)
    {
        // calculate the bounds of the current scan-line
        const float   yf         = (float)y;
        const float   ty         = t1[2] - yf; // cached variables for barycentric calculation
        const float   t00y       = t0[0] * ty;
        const float   t01y       = t0[1] * ty;
        const float   d0         = yf - p0[1]; // scan-line start/end calculation
        const float   d1         = yf - p1[1];
        const float   alpha      = d0 * p20y;
        const int     secondHalf = d1 < 0.f;
        int32_t       xMin       = (int32_t)(p0[0] + (p20x * alpha));
        int32_t       xMax       = (int32_t)(secondHalf ? (p1[0] + p21xy * d1) : (p0[0] + p10xy * d0));

        // Get the beginning and end of the scan-line
        const int32_t increment = (xMin < xMax) ? 1 : -1;
        xMin = math::clamp<int32_t>(xMin, bboxMinX, bboxMaxX);
        xMax = math::clamp<int32_t>(xMax, bboxMinX, bboxMaxX) + increment;

        for (int32_t x = xMin; x != xMax; x += increment)
        {
            // calculate barycentric coordinates
            const float xf   = (float)x;
            const float tx   = t0[2] - xf;
            const float tx10 = tx * t1[0];
            const float tx11 = tx * t1[1];
            const float u0   = (t01y - tx11) * scale;
            const float u1   = (tx10 - t00y) * scale;
            math::vec4  bc   {1.f - (u0 + u1), u1, u0, 0.f};

            // Only render pixels within the triangle edges.
            // Ensure the current point is in a triangle by checking the sign
            // bits of all 3 barycentric coordinates.
            union
            {
                const float* f;
                const int32_t* i;
            } bcFtoI{bc.v};

            //if (bc.v[0] < 0.f || bc.v[1] < 0.f || bc.v[2] < 0.f)
            if ((bcFtoI.i[0] | bcFtoI.i[1] | bcFtoI.i[2]) & 0x80000000)
            {
                continue;
            }

            const float z = math::dot(depth, bc);
            const float oldDepth = depthBuffer->raw_texel<float>(x, y);

            // expensive
            if (z < oldDepth)
            {
                continue;
            }

            // perspective correction
            bc *= persp;
            const float bW = math::rcp(math::sum(bc));
            bc *= bW;

            outCoords[numQueuedFrags] = {bc, (uint16_t)(x), (uint16_t)y, z};
            ++numQueuedFrags;

            if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
            {
                numQueuedFrags = 0;
                flush_fragments(SR_SHADER_MAX_FRAG_QUEUES, outCoords, pOutputs, outVaryings);
            }
        }
    }

    // cleanup remaining fragments
    if (numQueuedFrags > 0)
    {
        flush_fragments(numQueuedFrags, outCoords, pOutputs, outVaryings);
    }
}



#endif



/*--------------------------------------
 * Triangle Fragment Bin-Rasterization
--------------------------------------*/
void SR_FragmentProcessor::flush_fragments(
    uint_fast32_t       numQueuedFrags,
    const SR_FragCoord* outCoords,
    SR_ColorRGBAf*      pOutputs,
    ls::math::vec4*     outVaryings) noexcept
{
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();
    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const auto              pShader     = fragShader.shader;
    const uint32_t          numVaryings = fragShader.numVaryings;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    SR_Framebuffer*         fbo         = mFbo;
    math::vec4* const       inVaryings  = mBins[mBinId].mVaryings;

    while (numQueuedFrags --> 0)
    {
        // Interpolate varying variables using the barycentric coordinates
        interpolate_tri_varyings(outCoords[numQueuedFrags].bc, numVaryings, inVaryings, outVaryings);

        uint_fast32_t  haveOutputs = pShader(outCoords[numQueuedFrags].bc, pUniforms, outVaryings, pOutputs);
        const uint16_t x           = outCoords[numQueuedFrags].x;
        const uint16_t y           = outCoords[numQueuedFrags].y;
        const float    zf          = outCoords[numQueuedFrags].zf;
        const int32_t  zi          = (int32_t)zf; // better to do the cast here after all candidate pixels have been rejected

        // branchless select
        switch (-haveOutputs & numOutputs)
        {
            case 4: fbo->put_pixel(3, x, y, (uint16_t)zi, pOutputs[3]);
            case 3: fbo->put_pixel(2, x, y, (uint16_t)zi, pOutputs[2]);
            case 2: fbo->put_pixel(1, x, y, (uint16_t)zi, pOutputs[1]);
            case 1: fbo->put_pixel(0, x, y, (uint16_t)zi, pOutputs[0]);
                fbo->put_depth_pixel<float>(x, y, zf);
        }
    }
}



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SR_FragmentProcessor::execute() noexcept
{
    SR_ColorRGBAf   pOutputs[SR_SHADER_MAX_FRAG_OUTPUTS];
    ls::math::vec4  outVaryings[SR_SHADER_MAX_VARYING_VECTORS];

    // Sort the bins based on their depth. Closer objects should be rendered
    // first to allow for fragment rejection during the depth test.
    #if 0
    std::sort(mBins, mBins+mNumBins, [](const SR_FragmentBin& a, const SR_FragmentBin& b)->bool
    {
        return a.mPerspDivide[0] > b.mPerspDivide[0];
    });
    #elif 0
    sr_frag_bin_qsort(mBins, mNumBins);
    #else
    std::qsort(mBins, mNumBins, sizeof(SR_FragmentBin), [](const void* pA, const void* pB)-> int
    {
        const SR_FragmentBin* a = reinterpret_cast<const SR_FragmentBin*>(pA);
        const SR_FragmentBin* b = reinterpret_cast<const SR_FragmentBin*>(pB);
        return a->mPerspDivide[0] < b->mPerspDivide[0];
    });
    #endif

    // local cache
    const SR_RenderMode mode = mMode;

    switch(mode)
    {
        case RENDER_MODE_POINTS:
        case RENDER_MODE_INDEXED_POINTS:
            while (mBinId < mNumBins)
            {
                render_point(mFbo, pOutputs);
                ++mBinId;
            }
            break;
        case RENDER_MODE_LINES:
        case RENDER_MODE_INDEXED_LINES:
            while (mBinId < mNumBins)
            {
                render_line(mFbo, pOutputs, outVaryings);
                ++mBinId;
            }
            break;
        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
            while (mBinId < mNumBins)
            {
                render_triangle(mFbo->get_depth_buffer(), pOutputs, outVaryings);
                ++mBinId;
            }
            break;
    }
}
