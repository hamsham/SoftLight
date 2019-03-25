
#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/math/mat4.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_Framebuffer.hpp" // SR_Framebuffer
#include "soft_render/SR_Shader.hpp" // SR_FragmentShader
#include "soft_render/SR_ShaderProcessor.hpp" // SR_FragmentBin
#include "soft_render/SR_Texture.hpp"



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

    #if defined(LS_ARCH_X86)
        const __m128 bc0 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0x00);
        const __m128 bc1 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0x55);
        const __m128 bc2 = _mm_shuffle_ps(baryCoords.simd, baryCoords.simd, 0xAA);

        while (i --> 0u)
        {
            const __m128 v0 = _mm_mul_ps((inVaryings0++)->simd, bc0);
            const __m128 v1 = _mm_fmadd_ps((inVaryings1++)->simd, bc1, v0);
            const __m128 v2 = _mm_fmadd_ps((inVaryings2++)->simd, bc2, v1);
            (outVaryings++)->simd = v2;
        }
    #elif defined(LS_ARCH_ARM)
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
void SR_FragmentProcessor::render_point(
    const uint_fast64_t binId,
    SR_Framebuffer* const fbo,
    const ls::math::vec4_t<int32_t> dimens) noexcept
{
    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const bool              depthMask   = fragShader.depthMask == SR_DEPTH_MASK_ON;
    const auto              pShader     = fragShader.shader;
    const math::vec4        screenCoord = mBins[binId].mScreenCoords[0];
    const math::vec4        fragCoord   {screenCoord[0], screenCoord[1], screenCoord[2], 1.f};
    const math::vec4*       varyings    = mBins[binId].mVaryings;

    math::vec4 pOutputs[SR_SHADER_MAX_FRAG_OUTPUTS];

    if (fragCoord.v[0] >= dimens[0] && fragCoord.v[0] <= dimens[1] && fragCoord.v[1] >= dimens[2] && fragCoord.v[1] <= dimens[3])
    {
        const uint16_t x0    = (uint16_t)fragCoord[0];
        const uint16_t y0    = (uint16_t)fragCoord[1];
        const float    depth = fragCoord[2];

        if (fragShader.depthTest == SR_DEPTH_TEST_OFF || (fragShader.depthTest == SR_DEPTH_TEST_ON && fbo->test_depth_pixel(x0, y0, depth)))
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
            }

            if (depthMask)
            {
                fbo->put_depth_pixel<float>(x0, y0, z);
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
    const uint_fast64_t binId,
    SR_Framebuffer* fbo,
    const ls::math::vec4_t<int32_t> dimens) noexcept
{
    math::vec4              pOutputs[SR_SHADER_MAX_FRAG_OUTPUTS];
    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const uint32_t          numVaryings = fragShader.numVaryings;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const bool              noDepthTest = fragShader.depthTest == SR_DEPTH_TEST_OFF;
    const bool              depthMask   = fragShader.depthMask == SR_DEPTH_MASK_ON;
    const auto              shader      = fragShader.shader;
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();

    const math::vec4& screenCoord0 = mBins[binId].mScreenCoords[0];
    const math::vec4& screenCoord1 = mBins[binId].mScreenCoords[1];
    math::vec4* const inVaryings   = mBins[binId].mVaryings;

    math::vec4 outVaryings[SR_SHADER_MAX_VARYING_VECTORS];

    int32_t       pointX = (int32_t)screenCoord0[0];
    int32_t       pointY = (int32_t)screenCoord0[1];
    float         pointZ = screenCoord0[2];
    const float   dx     = screenCoord1[0] - screenCoord0[0];
    const float   dy     = screenCoord1[1] - screenCoord0[1];
    const float   dz     = screenCoord1[2] - screenCoord0[2];
    const int32_t l      = (int32_t)math::abs(dx);
    const int32_t m      = (int32_t)math::abs(dy);
    const int32_t n      = (int32_t)math::abs(dz);
    const int32_t dx2    = l << 1;
    const int32_t dy2    = m << 1;
    const int32_t dz2    = n << 1;
    const int32_t x_inc  = (dx < 0.f) ? -1 : 1;
    const int32_t y_inc  = (dy < 0.f) ? -1 : 1;

    int32_t err_1, err_2;

    if ((l >= m) && (l >= n))
    {
        const float percentEnd = math::abs(dx);
        const float z_inc = dz / percentEnd;
        err_1 = dy2 - l;
        err_2 = dz2 - l;

        for (int32_t i = 0; i < l; ++i)
        {
            interpolate_line_varyings(pointZ, numVaryings, inVaryings, outVaryings);

            if (noDepthTest || fbo->test_depth_pixel(pointX, pointY, pointZ))
            {
                const math::vec4 fragCoord{(float)pointX, (float)pointY, pointZ, 1.f};

                if (shader(fragCoord, pUniforms, outVaryings, pOutputs))
                {
                    if (depthMask)
                    {
                        fbo->put_depth_pixel<float>(pointX, pointY, pointZ);
                    }

                    for (std::size_t targetId = 0; targetId < numOutputs; ++targetId)
                    {
                        fbo->put_pixel(targetId, pointX, pointY, 0, pOutputs[targetId]);
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
        const float percentEnd = math::abs(dy);
        const float z_inc = dz / percentEnd;
        err_1 = dx2 - m;
        err_2 = dz2 - m;

        for (int32_t i = 0; i < m; ++i)
        {
            interpolate_line_varyings(pointZ, numVaryings, inVaryings, outVaryings);

            if (noDepthTest || fbo->test_depth_pixel(pointX, pointY, pointZ))
            {
                const math::vec4 fragCoord{(float)pointX, (float)pointY, pointZ, 1.f};

                if (shader(fragCoord, pUniforms, outVaryings, pOutputs))
                {
                    if (depthMask)
                    {
                        fbo->put_depth_pixel<float>(pointX, pointY, pointZ);
                    }

                    for (std::size_t targetId = 0; targetId < numOutputs; ++targetId)
                    {
                        fbo->put_pixel(targetId, pointX, pointY, 0, pOutputs[targetId]);
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
        const float percentEnd = math::abs(dz);
        const float z_inc = dz / percentEnd;
        err_1 = dy2 - n;
        err_2 = dx2 - n;

        for (int32_t i = 0; i < n; ++i)
        {
            interpolate_line_varyings(pointZ, numVaryings, inVaryings, outVaryings);

            if (noDepthTest || fbo->test_depth_pixel(pointX, pointY, pointZ))
            {
                const math::vec4 fragCoord{(float)pointX, (float)pointY, pointZ, 1.f};

                if (shader(fragCoord, pUniforms, outVaryings, pOutputs))
                {
                    if (depthMask)
                    {
                        fbo->put_depth_pixel<float>(pointX, pointY, pointZ);
                    }

                    for (std::size_t targetId = 0; targetId < numOutputs; ++targetId)
                    {
                        fbo->put_pixel(targetId, pointX, pointY, 0, pOutputs[targetId]);
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

    // Final bounds check
    if (pointX >= dimens[0] && pointX <= dimens[1] && pointY >= dimens[2] && pointY <= dimens[3])
    {
        if (noDepthTest || fbo->test_depth_pixel(pointX, pointY, pointZ))
        {
            const math::vec4 fragCoord{(float)pointX, (float)pointY, pointZ, 1.f};

            if (shader(fragCoord, pUniforms, outVaryings, pOutputs))
            {
                if (depthMask)
                {
                    fbo->put_depth_pixel<float>(pointX, pointY, pointZ);
                }

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
void SR_FragmentProcessor::render_triangle(const uint_fast64_t binId, const SR_Texture* depthBuffer) const noexcept
{
    uint32_t  numQueuedFrags = 0;
    const int32_t     yOffset      = (uint32_t)mThreadId;
    const int32_t     increment    = (int32_t)mNumProcessors;
    const math::vec4* screenCoords = mBins[binId].mScreenCoords;
    const math::vec4  depth        {screenCoords[0][2], screenCoords[1][2], screenCoords[2][2], 0.f};
    math::vec2&&      p0           = math::vec2_cast(screenCoords[0]);
    math::vec2&&      p1           = math::vec2_cast(screenCoords[1]);
    math::vec2&&      p2           = math::vec2_cast(screenCoords[2]);
    const math::vec4  persp        = mBins[binId].mPerspDivide;
    const bool        depthTesting = mShader->fragment_shader().depthTest == SR_DEPTH_TEST_ON;
    const int32_t     bboxMinX     = (int32_t)math::min(mFboW, math::max(0.f,   math::min(p0[0], p1[0], p2[0])));
    const int32_t     bboxMinY     = (int32_t)math::min(mFboH, math::max(0.f,   math::ceil(math::min(p0[1], p1[1], p2[1]))));
    const int32_t     bboxMaxX     = (int32_t)math::max(0.f,   math::min(mFboW, math::max(p0[0], p1[0], p2[0])));
    const int32_t     bboxMaxY     = (int32_t)math::max(0.f,   math::min(mFboH, math::max(p0[1], p1[1], p2[1])));
    const float       t0[3]        = {p2[0]-p0[0], p1[0]-p0[0], p0[0]};
    const float       t1[3]        = {p2[1]-p0[1], p1[1]-p0[1], p0[1]};
    const float       areaInv      = (t0[0] * t1[1]) - (t0[1] * t1[0]);
    const float       area         = math::rcp(areaInv);
    SR_FragCoord*     outCoords    = mQueues;

    // Don't render triangles which are too small to see
    if (math::abs(areaInv) < 0.1f)
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

    // Let each thread start rendering at whichever scanline it's assigned to
    const int32_t scanlineOffset = sr_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

    for (int32_t y = bboxMinY+scanlineOffset; y <= bboxMaxY; y += increment)
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
        if (xMin > xMax)
        {
            int32_t temp = xMin;
            xMin = xMax;
            xMax = temp;
        }

        // In this rasterizer, we're only rendering the absolute pixels
        // contained within the triangle edges. However this will serve as a
        // guard against any pixels we don't want to render.
        xMin = math::clamp<int32_t>(xMin, bboxMinX, bboxMaxX);
        xMax = math::clamp<int32_t>(xMax, bboxMinX, bboxMaxX);

        for (int32_t x = xMin; x <= xMax; ++x)
        {
            // calculate barycentric coordinates
            const float xf   = (float)x;
            const float tx   = t0[2] - xf;
            const float tx10 = tx * t1[0];
            const float tx11 = tx * t1[1];
            const float u0   = (t01y - tx11) * area;
            const float u1   = (tx10 - t00y) * area;
            math::vec4  bc   {1.f - (u0 + u1), u1, u0, 0.f};

            // Only render pixels within the triangle edges.
            // Ensure the current point is in a triangle by checking the sign
            // bits of all 3 barycentric coordinates.
            const float z = math::dot(depth, bc);
            const float oldDepth = depthBuffer->texel<float>(x, y);

            // We're only using the barycentric coordinates here to determine
            // if a pixel on the edge of a triangle should still be rendered.
            // This normally involves checking if the coordinate is negative,
            // but due to roundoff errors, we need to know if it's close enough
            // to a triangle edge to be rendered.
            if (depthTesting && (z < oldDepth))
            {
                continue;
            }

            // perspective correction
            bc *= persp;
            const float bW = math::sum_inv(bc);
            bc *= bW;

            outCoords[numQueuedFrags] = SR_FragCoord{bc, (uint16_t)x, (uint16_t)y, xf, yf, z};
            ++numQueuedFrags;

            if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
            {
                numQueuedFrags = 0;
                flush_fragments(binId, SR_SHADER_MAX_FRAG_QUEUES, outCoords);
            }
        }
    }

    // cleanup remaining fragments
    if (numQueuedFrags > 0)
    {
        flush_fragments(binId, numQueuedFrags, outCoords);
    }
}



/*--------------------------------------
 * Triangle Fragment Bin-Rasterization
--------------------------------------*/
void SR_FragmentProcessor::flush_fragments(
    const uint_fast64_t binId,
    uint32_t            numQueuedFrags,
    const SR_FragCoord* outCoords) const noexcept
{
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms.get();
    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const uint32_t          numVaryings = fragShader.numVaryings;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const SR_BlendMode      blendMode   = fragShader.blend;
    const bool              depthMask   = fragShader.depthMask == SR_DEPTH_MASK_ON;
    const auto              pShader     = fragShader.shader;
    SR_Framebuffer*         fbo         = mFbo;
    math::vec4* const       inVaryings  = mBins[binId].mVaryings;

    math::vec4 pOutputs[SR_SHADER_MAX_FRAG_OUTPUTS];
    math::vec4 outVaryings[SR_SHADER_MAX_VARYING_VECTORS];

    if (blendMode != SR_BLEND_OFF)
    {
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
                case 4: fbo->put_alpha_pixel(3, x, y, (uint16_t)zi, pOutputs[3], blendMode);
                case 3: fbo->put_alpha_pixel(2, x, y, (uint16_t)zi, pOutputs[2], blendMode);
                case 2: fbo->put_alpha_pixel(1, x, y, (uint16_t)zi, pOutputs[1], blendMode);
                case 1: fbo->put_alpha_pixel(0, x, y, (uint16_t)zi, pOutputs[0], blendMode);
            }

            if (depthMask)
            {
                fbo->put_depth_pixel<float>(x, y, fc[2]);
            }
        }
        return;
    }
    else
    {
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
                case 4: fbo->put_pixel(3, x, y, (uint16_t)zi, pOutputs[3]);
                case 3: fbo->put_pixel(2, x, y, (uint16_t)zi, pOutputs[2]);
                case 2: fbo->put_pixel(1, x, y, (uint16_t)zi, pOutputs[1]);
                case 1: fbo->put_pixel(0, x, y, (uint16_t)zi, pOutputs[0]);
            }

            if (depthMask)
            {
                fbo->put_depth_pixel<float>(x, y, fc[2]);
            }
        }
    }
}



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SR_FragmentProcessor::execute() noexcept
{
    switch(mMode)
    {
        case RENDER_MODE_POINTS:
        case RENDER_MODE_INDEXED_POINTS:
        {
            // divide the screen into equal parts which can then be rendered by all
            // available fragment threads.
            const int32_t w = mFbo->width();
            const int32_t h = mFbo->height();
            const math::vec4_t<int32_t> dimens = sr_subdivide_region<int32_t>(w, h, mNumProcessors, mThreadId);

            for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
            {
                render_point(numBinsProcessed, mFbo, dimens);
            }
            break;
        }

        case RENDER_MODE_LINES:
        case RENDER_MODE_INDEXED_LINES:
        case RENDER_MODE_TRI_WIRE:
        case RENDER_MODE_INDEXED_TRI_WIRE:
        {
            // divide the screen into equal parts which can then be rendered by all
            // available fragment threads.
            const int32_t w = mFbo->width();
            const int32_t h = mFbo->height();
            const math::vec4_t<int32_t> dimens = sr_subdivide_region<int32_t>(w, h, mNumProcessors, mThreadId);

            for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
            {
                render_line(numBinsProcessed, mFbo, dimens);
            }
            break;
        }

        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
            // Triangles assign scan-lines per thread for rasterization.
            // There's No need to subdivide the output framebuffer
            for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
            {
                render_triangle(numBinsProcessed, mFbo->get_depth_buffer());
            }
            break;
    }
}
