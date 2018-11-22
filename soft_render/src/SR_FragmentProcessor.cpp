
 #include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/utils/Sort.hpp" // utils::sort_quick

#include "lightsky/math/mat4.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_ShaderProcessor.hpp"
#include "soft_render/SR_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Rendering setup
-----------------------------------------------------------------------------*/
//#define SR_RENDER_4_PIXELS



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
inline void LS_IMPERATIVE interpolate_tri_varyings(
    const math::vec4&   baryCoords,
    const uint_fast32_t numVaryings,
    const math::vec4*   inVaryings0,
    math::vec4*         outVaryings) noexcept
{
    const math::vec4* inVaryings1 = inVaryings0 + numVaryings;
    const math::vec4* inVaryings2 = inVaryings1 + numVaryings;
    uint_fast32_t i = numVaryings;

    #if defined(SR_RENDER_4_PIXELS) && defined(LS_ARCH_X86)
        const __m128 bc0 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0x00);
        const __m128 bc1 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0x55);
        const __m128 bc2 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0xAA);
        //const __m128 bc0 = _mm_set1_ps(baryCoords.v[0]);
        //const __m128 bc1 = _mm_set1_ps(baryCoords.v[1]);
        //const __m128 bc2 = _mm_set1_ps(baryCoords.v[2]);

        while (i --> 0u)
        {
            const __m128 v0 = _mm_mul_ps((inVaryings0++)->simd, bc0);
            const __m128 v1 = _mm_mul_ps((inVaryings1++)->simd, bc1);
            const __m128 v2 = _mm_mul_ps((inVaryings2++)->simd, bc2);
            (outVaryings++)->simd = _mm_add_ps(_mm_add_ps(v0, v1), v2);
            //_mm_store_ps((outVaryings++)->v, _mm_add_ps(_mm_add_ps(v0, v1), v2));
        }
    #elif defined(SR_RENDER_4_PIXELS) && defined(LS_ARCH_ARM)
        const float32x4_t bc0 = vdupq_lane_f32(vget_low_f32(baryCoords.simd),  0);
        const float32x4_t bc1 = vdupq_lane_f32(vget_low_f32(baryCoords.simd),  1);
        const float32x4_t bc2 = vdupq_lane_f32(vget_high_f32(baryCoords.simd), 0);

        while (i --> 0u)
        {
            const float32x4_t v0 = vmulq_f32(vld1q_f32((inVaryings0++)->v), bc0);
            const float32x4_t v1 = vmulq_f32(vld1q_f32((inVaryings1++)->v), bc1);
            const float32x4_t v2 = vmulq_f32(vld1q_f32((inVaryings2++)->v), bc2);
            (outVaryings++)->simd = vaddq_f32(vaddq_f32(v0, v1), v2);
        }
    #else
        const float bc0 = baryCoords.v[0];
        const float bc1 = baryCoords.v[1];
        const float bc2 = baryCoords.v[2];

        while (i --> 0u)
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
 * SR_FragmentProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Determine if a point can be rendered.
--------------------------------------*/
void SR_FragmentProcessor::render_point(SR_Framebuffer* const fbo) noexcept
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

    math::vec4 pOutputs[SR_SHADER_MAX_FRAG_OUTPUTS];

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
                //case 1: fbo->put_pixel(0, x0, y0, z, math::vec4{1.f, 0, 1.f, 1.f});
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
void SR_FragmentProcessor::render_line(SR_Framebuffer* fbo) noexcept
{
    math::vec4              pOutputs[SR_SHADER_MAX_FRAG_OUTPUTS];
    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();
    const uint32_t          numVaryings = fragShader.numVaryings;
    const uint32_t          numOutputs  = fragShader.numOutputs;

    const math::vec4& screenCoord0 = mBins[mBinId].mScreenCoords[0];
    const math::vec4& screenCoord1 = mBins[mBinId].mScreenCoords[1];
    math::vec4* const inVaryings   = mBins[mBinId].mVaryings;
    alignas(sizeof(math::vec4)) math::vec4 outVaryings[SR_SHADER_MAX_VARYING_VECTORS];

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
#if defined(SR_RENDER_4_PIXELS) && defined(LS_ARCH_X86)

void SR_FragmentProcessor::render_triangle(const SR_Texture* depthBuffer) const noexcept
{
    const __m128      persp        = mBins[mBinId].mPerspDivide.simd;
    const math::vec4* screenCoords = mBins[mBinId].mScreenCoords;
    math::vec4        p0           = screenCoords[0];
    math::vec4        p1           = screenCoords[1];
    math::vec4        p2           = screenCoords[2];
    const int32_t     bboxMinX     = math::min(mFboX1, math::max(mFboX0, math::min(p0[0], p1[0], p2[0])));
    const int32_t     bboxMinY     = math::min(mFboY1, math::max(mFboY0, math::min(p0[1], p1[1], p2[1])));
    const int32_t     bboxMaxX     = math::max(mFboX0, math::min(mFboX1, math::max(p0[0], p1[0], p2[0])));
    const int32_t     bboxMaxY     = math::max(mFboY0, math::min(mFboY1, math::max(p0[1], p1[1], p2[1])));
    const math::vec4  depth        {p0[2], p1[2], p2[2], 0.f};
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
    SR_FragCoord* outCoords = mQueues;

    for (int32_t y = bboxMinY; y <= bboxMaxY; ++y)
    {
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
            const math::vec4 xf {_mm_cvtepi32_ps(_mm_add_epi32(_mm_set1_epi32(x), _mm_set_epi32(3, 2, 1, 0)))};
            const __m128     tx = _mm_sub_ps(t0[2], xf.simd);
            const __m128     u0 = _mm_mul_ps(_mm_sub_ps(t01y, _mm_mul_ps(tx, t1[1])), scale);
            const __m128     u1 = _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(tx, t1[0]), t00y), scale);

            math::mat4 bcF{
                math::vec4{_mm_sub_ps(_mm_set1_ps(1.f), _mm_add_ps(u0, u1))},
                math::vec4{u1},
                math::vec4{u0},
                math::vec4{_mm_setzero_ps()}
            };
            bcF = math::transpose(bcF);

            // depth texture lookup will always be slow
            const math::vec4      z           = depth * bcF;
            const __m128          depthTexels = depthBuffer->texel4<float>(x, y).simd;
            alignas(16) const int depthTest   = _mm_movemask_ps(_mm_sub_ps(z.simd, depthTexels));

            for (int32_t i = 0; i < 4; ++i)
            {
                if ((depthTest & (1 << i)) || _mm_movemask_ps(bcF[i].simd))
                {
                    continue;
                }

                // perspective correction
                __m128 bc4 = _mm_mul_ps(bcF[i].simd, persp);

                // horizontal add
                {
                    const __m128 a = bc4;

                    // swap the words of each vector
                    const __m128 b = _mm_shuffle_ps(a, a, 0xB1);
                    const __m128 c = _mm_add_ps(a, b);

                    // swap each half of the vector
                    const __m128 d = _mm_shuffle_ps(c, c, 0x0F);
                    const __m128 e = _mm_add_ps(c, d);

                    bc4 = _mm_mul_ps(a, _mm_rcp_ps(e));
                }

                outCoords[numQueuedFrags] = SR_FragCoord{
                    math::vec4{bc4},
                    (uint16_t)(x+i),
                    (uint16_t)y,
                    xf[i],
                    yf,
                    z[i]
                };
                ++numQueuedFrags;

                if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                {
                    numQueuedFrags = 0;
                    flush_fragments(SR_SHADER_MAX_FRAG_QUEUES, outCoords);
                }
            }
        }
    }

    if (numQueuedFrags)
    {
        flush_fragments(numQueuedFrags, outCoords);
    }
}



#elif defined(SR_RENDER_4_PIXELS) && defined(LS_ARCH_ARM) // Translating x86 into a NEON implementation.

void SR_FragmentProcessor::render_triangle(const SR_Texture* depthBuffer) const noexcept
{
    const float32x4_t persp        = mBins[mBinId].mPerspDivide.simd;
    const math::vec4* screenCoords = mBins[mBinId].mScreenCoords;
    math::vec4        p0           = screenCoords[0];
    math::vec4        p1           = screenCoords[1];
    math::vec4        p2           = screenCoords[2];
    const int32_t     bboxMinX     = math::min(mFboX1, math::max(mFboX0, math::min(p0[0], p1[0], p2[0])));
    const int32_t     bboxMinY     = math::min(mFboY1, math::max(mFboY0, math::min(p0[1], p1[1], p2[1])));
    const int32_t     bboxMaxX     = math::max(mFboX0, math::min(mFboX1, math::max(p0[0], p1[0], p2[0])));
    const int32_t     bboxMaxY     = math::max(mFboY0, math::min(mFboY1, math::max(p0[1], p1[1], p2[1])));
    const math::vec4  depth       {p0[2], p1[2], p2[2], 0.f};
    const float32x4_t t0[3]       {vdupq_n_f32(p2[0]-p0[0]), vdupq_n_f32(p1[0]-p0[0]), vdupq_n_f32(p0[0])};
    const float32x4_t t1[3]       {vdupq_n_f32(p2[1]-p0[1]), vdupq_n_f32(p1[1]-p0[1]), vdupq_n_f32(p0[1])};
    const float32x4_t scaleInv    = vsubq_f32(vmulq_f32(t0[0], t1[1]), vmulq_f32(t0[1], t1[0]));
    const float32x4_t scale       = vrecpeq_f32(scaleInv);

    // Don't render triangles which are too small to see
    if (std::fabs(vgetq_lane_f32(scaleInv, 0)) < 1.f)
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
    SR_FragCoord* outCoords = mQueues;

    for (int32_t y = bboxMinY; y <= bboxMaxY; ++y)
    {
        // calculate the bounds of the current scan-line
        const float       yf         = (float)y;
        const float32x4_t ty         = vsubq_f32(t1[2], vdupq_n_f32(yf));
        const float32x4_t t00y       = vmulq_f32(t0[0], ty);
        const float32x4_t t01y       = vmulq_f32(t0[1], ty);
        const float       d0         = yf - p0[1];
        const float       d1         = yf - p1[1];
        const float       alpha      = d0 * p20y;
        const int         secondHalf = math::sign_bit(d1);
        int32_t           xMin       = (int32_t)(p0[0] + (p20x * alpha));
        int32_t           xMax       = (int32_t)(secondHalf ? (p1[0] + p21xy * d1) : (p0[0] + p10xy * d0));

        // Get the beginning and end of the scan-line
        if (xMin > xMax) std::swap(xMin, xMax);
        xMin = math::clamp<int32_t>(xMin, bboxMinX, bboxMaxX);
        xMax = math::clamp<int32_t>(xMax, bboxMinX, bboxMaxX);

        for (int32_t x = xMin; x <= xMax; x += 4)
        {
            // calculate barycentric coordinates
            constexpr int32_t lanes[4] = {0, 1, 2, 3};
            const int32x4_t   xi       = vaddq_s32(vdupq_n_s32(x), vld1q_s32(lanes));
            const math::vec4  xf       {vcvtq_f32_s32(xi)};
            const float32x4_t tx       = vsubq_f32(t0[2], xf.simd);
            const float32x4_t u0       = vmulq_f32(vsubq_f32(t01y, vmulq_f32(tx, t1[1])), scale);
            const float32x4_t u1       = vmulq_f32(vsubq_f32(vmulq_f32(tx, t1[0]), t00y), scale);

            math::mat4 bcF{
                math::vec4{vsubq_f32(vdupq_n_f32(1.f), vaddq_f32(u0, u1))},
                math::vec4{u1},
                math::vec4{u0},
                math::vec4{vdupq_n_f32(0.f)}
            };
            bcF = math::transpose(bcF);

            // depth texture lookup will always be slow
            const math::vec4&& z           = depth * bcF;
            const math::vec4&& depthTexels = depthBuffer->texel4<float>(x, y);
            const int          depthTest   = math::sign_bits(z - depthTexels);

            for (int32_t i = 0; i < 4; ++i)
            {
                if ((depthTest & (1 << i)) || math::sign_bits(bcF[i]))
                {
                    continue;
                }

                // perspective correction
                float32x4_t bc4 = vmulq_f32(bcF[i].simd, persp);

                // horizontal add
                {
                    const float32x4_t a = bc4;
                    const float32x2_t b = vadd_f32(vget_high_f32(a), vget_low_f32(a));
                    const float32x2_t c = vpadd_f32(b, b);
                    const float32x4_t d = vcombine_f32(c, c);
                    const float32x4_t recip0 = vrecpeq_f32(d);
                    const float32x4_t recip1 = vmulq_f32(vrecpsq_f32(d, recip0), recip0);

                    bc4 = vmulq_f32(a, recip1);
                }

                outCoords[numQueuedFrags] = SR_FragCoord{
                    math::vec4{bc4},
                    (uint16_t)(x+i),
                    (uint16_t)y,
                    xf[i],
                    yf,
                    z[i]
                };
                ++numQueuedFrags;

                if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                {
                    numQueuedFrags = 0;
                    flush_fragments(SR_SHADER_MAX_FRAG_QUEUES, outCoords);
                }
            }
        }
    }

    if (numQueuedFrags)
    {
        flush_fragments(numQueuedFrags, outCoords);
    }
}



#elif defined(SR_RENDER_4_PIXELS)

void SR_FragmentProcessor::render_triangle(const SR_Texture* depthBuffer) const noexcept
{
    const math::vec4  persp        = mBins[mBinId].mPerspDivide;
    const math::vec4* screenCoords = mBins[mBinId].mScreenCoords;
    math::vec4        p0           = screenCoords[0];
    math::vec4        p1           = screenCoords[1];
    math::vec4        p2           = screenCoords[2];
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
    SR_FragCoord* outCoords = mQueues;

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
            const math::mat4&& bcF = math::transpose(
                math::mat4{
                    math::vec4{math::vec4{1.f} - (u0 + u1)},
                    u1,
                    u0,
                    math::vec4{0.f}
                }
            );

            // depth texture lookup will always be slow
            const math::vec4&& z              = depth * bcF;
            const math::vec4&& depthBufTexels = depthBuffer->texel4<float>(x, y);
            const int          depthTest      = math::sign_bits(z - depthBufTexels);

            for (int32_t i = 0; i < 4; ++i)
            {
                if ((depthTest & (0x01 << i)) || math::sign_bits(bcF[i]))
                {
                    continue;
                }

                // prefetch
                const float zf = z[i];

                // perspective correction
                math::vec4&& bc4 = bcF[i] * persp;
                const float bW = math::sum_inv(bc4);
                bc4 *= bW;

                outCoords[numQueuedFrags] = SR_FragCoord{
                    bc4,
                    (uint16_t)(x+i),
                    (uint16_t)y,
                    xf[i],
                    yf,
                    zf
                };
                ++numQueuedFrags;

                if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                {
                    numQueuedFrags = 0;
                    flush_fragments(SR_SHADER_MAX_FRAG_QUEUES, outCoords);
                }
            }
        }
    }

    if (numQueuedFrags)
    {
        flush_fragments(numQueuedFrags, outCoords);
    }
}



#else

void SR_FragmentProcessor::render_triangle(const SR_Texture* depthBuffer) const noexcept
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
    SR_FragCoord* outCoords = mQueues;

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
        const int     secondHalf = math::sign_bit(d1);
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
            const float z = math::dot(depth, bc);
            const float oldDepth = depthBuffer->texel<float>(x, y);

            //if (bc.v[0] < 0.f || bc.v[1] < 0.f || bc.v[2] < 0.f)
            if (math::sign_bits(bc) || math::sign_bit(z - oldDepth))
            {
                continue;
            }

            // perspective correction
            bc *= persp;
            const float bW = math::rcp(math::sum(bc));
            bc *= bW;

            outCoords[numQueuedFrags] = SR_FragCoord{
                bc,
                (uint16_t)x,
                (uint16_t)y,
                xf,
                yf,
                z
            };
            ++numQueuedFrags;

            if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
            {
                numQueuedFrags = 0;
                flush_fragments(SR_SHADER_MAX_FRAG_QUEUES, outCoords);
            }
        }
    }

    // cleanup remaining fragments
    if (numQueuedFrags > 0)
    {
        flush_fragments(numQueuedFrags, outCoords);
    }
}



#endif



/*--------------------------------------
 * Triangle Fragment Bin-Rasterization
--------------------------------------*/
void SR_FragmentProcessor::flush_fragments(
    uint_fast32_t       numQueuedFrags,
    const SR_FragCoord* outCoords) const noexcept
{
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();
    alignas(sizeof(math::vec4)) const SR_FragmentShader fragShader  = mShader->mFragShader;
    const auto              pShader     = fragShader.shader;
    const uint32_t          numVaryings = fragShader.numVaryings;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    SR_Framebuffer*         fbo         = mFbo;
    math::vec4* const       inVaryings  = mBins[mBinId].mVaryings;

    alignas(sizeof(math::vec4)) math::vec4 pOutputs[SR_SHADER_MAX_FRAG_OUTPUTS];
    alignas(sizeof(math::vec4)) math::vec4 outVaryings[SR_SHADER_MAX_VARYING_VECTORS];

    while (numQueuedFrags --> 0)
    {
        const math::vec4 bc = outCoords[numQueuedFrags].bc;
        const uint16_t   x  = outCoords[numQueuedFrags].x;
        const uint16_t   y  = outCoords[numQueuedFrags].y;
        const math::vec4 fc {outCoords[numQueuedFrags].xf, outCoords[numQueuedFrags].yf, outCoords[numQueuedFrags].zf, 1.f};
        const int32_t    zi = (int32_t)outCoords[numQueuedFrags].zf; // better to do the cast here after all candidate pixels have been rejected

        // Interpolate varying variables using the barycentric coordinates
        interpolate_tri_varyings(bc, numVaryings, inVaryings, outVaryings);

        uint_fast32_t  haveOutputs = pShader(fc, pUniforms, outVaryings, pOutputs);
        // branchless select
        switch (-haveOutputs & numOutputs)
        {
            case 4: /*zi < fbo->depth() && */fbo->put_pixel(3, x, y, (uint16_t)zi, pOutputs[3]);
            case 3: /*zi < fbo->depth() && */fbo->put_pixel(2, x, y, (uint16_t)zi, pOutputs[2]);
            case 2: /*zi < fbo->depth() && */fbo->put_pixel(1, x, y, (uint16_t)zi, pOutputs[1]);
            case 1: /*zi < fbo->depth() && */fbo->put_pixel(0, x, y, (uint16_t)zi, pOutputs[0]);
                fbo->put_depth_pixel<float>(x, y, fc[2]);
        }
    }
}



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SR_FragmentProcessor::execute() noexcept
{
    // Sort the bins based on their depth. Closer objects should be rendered
    // first to allow for fragment rejection during the depth test.
    utils::sort_quick<SR_FragmentBin, utils::IsGreater<SR_FragmentBin>>(mBins, mNumBins);
    //utils::sort_quick_iterative<SR_FragmentBin, utils::IsGreater<SR_FragmentBin>>(mBins, mNumBins);

    switch(mMode)
    {
        case RENDER_MODE_POINTS:
        case RENDER_MODE_INDEXED_POINTS:
            while (mBinId < mNumBins)
            {
                render_point(mFbo);
                ++mBinId;
            }
            break;
        case RENDER_MODE_LINES:
        case RENDER_MODE_INDEXED_LINES:
            while (mBinId < mNumBins)
            {
                render_line(mFbo);
                ++mBinId;
            }
            break;
        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
            while (mBinId < mNumBins)
            {
                render_triangle(mFbo->get_depth_buffer());
                ++mBinId;
            }
            break;
    }
}
