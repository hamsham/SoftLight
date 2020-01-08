
#include <iostream>

#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/math/bits.h"
#include "lightsky/math/fixed.h"
#include "lightsky/math/mat4.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_Framebuffer.hpp" // SR_Framebuffer
#include "soft_render/SR_ScanlineBounds.hpp"
#include "soft_render/SR_Shader.hpp" // SR_FragmentShader
#include "soft_render/SR_ShaderProcessor.hpp" // SR_FragmentBin
#include "soft_render/SR_Texture.hpp"

#ifndef SR_REVERSED_Z_BUFFER
    #define SR_REVERSED_Z_BUFFER 1
#endif



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

    for (uint32_t i = numVaryings; i--;)
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
    const float*        baryCoords,
    const uint_fast32_t numVaryings,
    const math::vec4*   inVaryings0,
    math::vec4*         outVaryings) noexcept
{
    #if defined(LS_ARCH_X86)
        const math::vec4* inVaryings1  = inVaryings0 + SR_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* inVaryings2  = inVaryings0 + (SR_SHADER_MAX_VARYING_VECTORS * 2);

        const __m128 bc  = _mm_castsi128_ps(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(baryCoords)));
        const __m128 bc0 = _mm_permute_ps(bc, 0x00);
        const __m128 bc1 = _mm_permute_ps(bc, 0x55);
        const __m128 bc2 = _mm_permute_ps(bc, 0xAA);

        for (uint_fast32_t i = numVaryings; i--;)
        {
            const __m128 v0 = _mm_mul_ps(_mm_castsi128_ps(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(inVaryings0++))), bc0);
            const __m128 v1 = _mm_fmadd_ps(_mm_castsi128_ps(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(inVaryings1++))), bc1, v0);
            _mm_store_ps(reinterpret_cast<float*>(outVaryings++), _mm_fmadd_ps(_mm_castsi128_ps(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(inVaryings2++))), bc2, v1));
        }
    #elif defined(LS_ARCH_AARCH64)
        const math::vec4* inVaryings1  = inVaryings0 + SR_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* inVaryings2  = inVaryings0 + SR_SHADER_MAX_VARYING_VECTORS * 2;

        const float32x4_t bc  = vld1q_f32(baryCoords);
        const float32x4_t bc0 = vdupq_lane_f32(vget_low_f32(bc),  0);
        const float32x4_t bc1 = vdupq_lane_f32(vget_low_f32(bc),  1);
        const float32x4_t bc2 = vdupq_lane_f32(vget_high_f32(bc), 0);

        for (uint_fast32_t i = numVaryings; i--;)
        {
            const float32x4_t v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0++)),     bc0);
            const float32x4_t v1 = vfmaq_f32(v0, vld1q_f32(reinterpret_cast<const float*>(inVaryings1++)), bc1);
            vst1q_f32(reinterpret_cast<float*>(outVaryings++), vfmaq_f32(v1, vld1q_f32(reinterpret_cast<const float*>(inVaryings2++)), bc2));
        }
    #elif defined(LS_ARCH_ARM)
        const math::vec4* inVaryings1  = inVaryings0 + SR_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* inVaryings2  = inVaryings0 + SR_SHADER_MAX_VARYING_VECTORS * 2;

        const float32x4_t bc  = vld1q_f32(baryCoords);
        const float32x4_t bc0 = vdupq_lane_f32(vget_low_f32(bc),  0);
        const float32x4_t bc1 = vdupq_lane_f32(vget_low_f32(bc),  1);
        const float32x4_t bc2 = vdupq_lane_f32(vget_high_f32(bc), 0);

        for (uint_fast32_t i = numVaryings; i--;)
        {
            const float32x4_t v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0++)), bc0);
            const float32x4_t v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1++)), bc1);
            vst1q_f32(reinterpret_cast<float*>(outVaryings++), vaddq_f32(vaddq_f32(v0, v1), vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2++)), bc2)));
        }
    #else
        const math::vec4* inVaryings1 = inVaryings0 + SR_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* inVaryings2 = inVaryings0 + SR_SHADER_MAX_VARYING_VECTORS * 2;

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
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const bool              depthTest   = fragShader.depthTest == SR_DEPTH_TEST_ON;
    const bool              depthMask   = fragShader.depthMask == SR_DEPTH_MASK_ON;
    const auto              pShader     = fragShader.shader;
    const SR_BlendMode      blendMode   = fragShader.blend;
    const bool              blend       = blendMode != SR_BLEND_OFF;
    const math::vec4        screenCoord = mBins[binId].mScreenCoords[0];
    const math::vec4        fragCoord   {screenCoord[0], screenCoord[1], screenCoord[2], 1.f};
    const SR_Texture*       pDepthBuf   = fbo->get_depth_buffer();
    SR_FragmentParam        fragParams;

    if (fragCoord.v[0] < dimens[0] || fragCoord.v[0] > dimens[1] || fragCoord.v[1] < dimens[2] || fragCoord.v[1] > dimens[3])
    {
        return;
    }

    fragParams.x = (uint16_t)fragCoord[0];
    fragParams.y = (uint16_t)fragCoord[1];
    fragParams.pUniforms = pUniforms;

    for (unsigned i = SR_SHADER_MAX_VARYING_VECTORS; i--;)
    {
        mVaryings[i] = mBins[binId].mVaryings[i];
    }

    if (depthTest == SR_DEPTH_TEST_ON)
    {
#if SR_REVERSED_Z_BUFFER
        if (fragCoord[2] < pDepthBuf->raw_texel<float>(fragParams.x, fragParams.y))
#else
        if (fragCoord[2] > pDepthBuf->raw_texel<float>(fragParams.x, fragParams.y))
#endif
        {
            return;
        }
    }

    fragParams.pVaryings = mVaryings;
    const uint_fast32_t haveOutputs = pShader(fragParams);

    if (blend)
    {
        // branchless select
        switch (-haveOutputs & numOutputs)
        {
            case 4: fbo->put_alpha_pixel(3, fragParams.x, fragParams.y, fragParams.pOutputs[3], blendMode);
            case 3: fbo->put_alpha_pixel(2, fragParams.x, fragParams.y, fragParams.pOutputs[2], blendMode);
            case 2: fbo->put_alpha_pixel(1, fragParams.x, fragParams.y, fragParams.pOutputs[1], blendMode);
            case 1: fbo->put_alpha_pixel(0, fragParams.x, fragParams.y, fragParams.pOutputs[0], blendMode);
                //case 1: fbo->put_pixel(0, x0, y0, math::vec4{1.f, 0, 1.f, 1.f});
        }

    }
    else
    {
        // branchless select
        switch (-haveOutputs & numOutputs)
        {
            case 4: fbo->put_pixel(3, fragParams.x, fragParams.y, fragParams.pOutputs[3]);
            case 3: fbo->put_pixel(2, fragParams.x, fragParams.y, fragParams.pOutputs[2]);
            case 2: fbo->put_pixel(1, fragParams.x, fragParams.y, fragParams.pOutputs[1]);
            case 1: fbo->put_pixel(0, fragParams.x, fragParams.y, fragParams.pOutputs[0]);
                //case 1: fbo->put_pixel(0, x0, y0, math::vec4{1.f, 0, 1.f, 1.f});
        }
    }

    if (depthMask)
    {
        fbo->put_depth_pixel<float>(fragParams.x, fragParams.y, fragParams.fragCoord[2]);
    }
}



/*--------------------------------------
 * Methods for clipping a line segment
 *
 * This method was adapted from Stephen M. Cameron's implementation of the
 * Liang-Barsky line clipping algorithm:
 *     https://github.com/smcameron/liang-barsky-in-c
 *
 * His method was also adapted from Hin Jang's C++ implementation:
 *     http://hinjang.com/articles/04.html#eight
--------------------------------------*/
bool clip_segment(float num, float denom, float& tE, float& tL)
{
    if (math::abs(denom) < LS_EPSILON)
        return num < 0.f;

    const float t = num / denom;

    if (denom > 0.f)
    {
        if (t > tL)
        {
            return false;
        }

        if (t > tE)
        {
            tE = t;
        }
    }
    else
    {
        if (t < tE)
        {
            return false;
        }

        if (t < tL)
        {
            tL = t;
        }
    }

    return true;
}

bool sr_clip_liang_barsky(math::vec2& a, math::vec2& b, const math::vec4_t<int32_t>& dimens)
{
    float tE, tL;
    float& x1 = a[0];
    float& y1 = a[1];
    float& x2 = b[0];
    float& y2 = b[1];
    const float dx = x2 - x1;
    const float dy = y2 - y1;

    float xMin = (float)dimens[0];
    float xMax = (float)dimens[1];
    float yMin = (float)dimens[2];
    float yMax = (float)dimens[3];

    if (math::abs(dx) < LS_EPSILON && math::abs(dy) < LS_EPSILON)
    {
        if (x1 >= xMin && x1 <= xMax && y1 >= yMin && y1 <= yMax)
        {
            return true;
        }
    }

    tE = 0.f;
    tL = 1.f;

    if (clip_segment(xMin-x1,  dx, tE, tL) &&
        clip_segment(x1-xMax, -dx, tE, tL) &&
        clip_segment(yMin-y1,  dy, tE, tL) &&
        clip_segment(y1-yMax, -dy, tE, tL))
    {
        if (tL < 1.f)
        {
            x2 = x1 + tL * dx;
            y2 = y1 + tL * dy;
        }

        if (tE > 0.f)
        {
            x1 += tE * dx;
            y1 += tE * dy;
        }

        return true;
    }

    return false;
}

inline bool sr_clip_liang_barsky(math::vec2 screenCoords[2], const math::vec4_t<int32_t>& dimens)
{
    return sr_clip_liang_barsky(screenCoords[0], screenCoords[1], dimens);
}



/*--------------------------------------
 * Process the line fragments using a simple DDA algorithm
--------------------------------------*/
void SR_FragmentProcessor::render_line(
    const uint_fast64_t binId,
    SR_Framebuffer* fbo,
    const math::vec4_t<int32_t> dimens) noexcept
{
    typedef math::long_medp_t fixed_type;
    constexpr fixed_type ZERO = fixed_type{0};

    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const SR_BlendMode      blendMode   = fragShader.blend;
    const bool              blend       = blendMode != SR_BLEND_OFF;
    const uint32_t          numVaryings = fragShader.numVaryings;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const bool              noDepthTest = fragShader.depthTest == SR_DEPTH_TEST_OFF;
    const bool              depthMask   = fragShader.depthMask == SR_DEPTH_MASK_ON;
    const auto              shader      = fragShader.shader;
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms;

    const math::vec4  screenCoord0  = mBins[binId].mScreenCoords[0];
    const math::vec4  screenCoord1  = mBins[binId].mScreenCoords[1];
    float             z0            = screenCoord0[2];
    float             z1            = screenCoord1[2];
    math::vec4* const inVaryings    = mBins[binId].mVaryings;
    math::vec2        clipCoords[2] = {math::vec2_cast(screenCoord0), math::vec2_cast(screenCoord1)};

    if (!sr_clip_liang_barsky(clipCoords, dimens))
    {
        return;
    }

    const float      x1f   = screenCoord0[0];
    const float      y1f   = screenCoord0[1];
    const fixed_type x1    = math::fixed_cast<fixed_type, float>(clipCoords[0][0]);
    const fixed_type y1    = math::fixed_cast<fixed_type, float>(clipCoords[0][1]);
    const fixed_type x2    = math::fixed_cast<fixed_type, float>(clipCoords[1][0]);
    const fixed_type y2    = math::fixed_cast<fixed_type, float>(clipCoords[1][1]);
    fixed_type       x     = x1;
    fixed_type       y     = y1;
    fixed_type       dx    = x2 - x1;
    fixed_type       dy    = y2 - y1;
    const fixed_type step  = math::abs((dx >= dy ? dx : dy));
    const int32_t    istep = math::integer_cast<int32_t, fixed_type>(step);

    dx = (step > ZERO) ? (dx/step) : ZERO;
    dy = (step > ZERO) ? (dy/step) : ZERO;

    const float       dist     = 1.f / math::length(math::vec2_cast(screenCoord1)-math::vec2_cast(screenCoord0));
    const SR_Texture* depthBuf = fbo->get_depth_buffer();

    SR_FragmentParam fragParams;
    fragParams.pUniforms = pUniforms;

    for (int32_t i = 0; i <= istep; ++i)
    {
        const float      xf      = math::float_cast<float, fixed_type>(x);
        const float      yf      = math::float_cast<float, fixed_type>(y);
        const int32_t    xi      = math::integer_cast<int32_t, fixed_type>(math::floor(x));
        const int32_t    yi      = math::integer_cast<int32_t, fixed_type>(math::floor(y));

        const float      currLen = math::length(math::vec2{xf-x1f, yf-y1f});
        const float      interp  = (currLen*dist);
        const float      z       = math::mix(z0, z1, interp);

#if SR_REVERSED_Z_BUFFER
        if (noDepthTest || depthBuf->raw_texel<float>((uint16_t)xi, (uint16_t)yi) <= z)
#else
        if (noDepthTest || depthBuf->raw_texel<float>((uint16_t)xi, (uint16_t)yi) >= z)
#endif
        {
            interpolate_line_varyings(interp, numVaryings, inVaryings, mVaryings);

            fragParams.fragCoord = {xf, yf, z, 1.f};
            fragParams.x = (uint16_t)xi;
            fragParams.y = (uint16_t)yi;
            fragParams.pVaryings = mVaryings;
            const uint_fast32_t haveOutputs = shader(fragParams);

            if (blend)
            {
                // branchless select
                switch (-haveOutputs & numOutputs)
                {
                    case 4: fbo->put_alpha_pixel(3, fragParams.x, fragParams.y, fragParams.pOutputs[3], blendMode);
                    case 3: fbo->put_alpha_pixel(2, fragParams.x, fragParams.y, fragParams.pOutputs[2], blendMode);
                    case 2: fbo->put_alpha_pixel(1, fragParams.x, fragParams.y, fragParams.pOutputs[1], blendMode);
                    case 1: fbo->put_alpha_pixel(0, fragParams.x, fragParams.y, fragParams.pOutputs[0], blendMode);
                        //case 1: fbo->put_pixel(0, fragParams.x, fragParams.y, math::vec4{1.f, 0, 1.f, 1.f});
                }

            }
            else
            {
                // branchless select
                switch (-haveOutputs & numOutputs)
                {
                    case 4: fbo->put_pixel(3, fragParams.x, fragParams.y, fragParams.pOutputs[3]);
                    case 3: fbo->put_pixel(2, fragParams.x, fragParams.y, fragParams.pOutputs[2]);
                    case 2: fbo->put_pixel(1, fragParams.x, fragParams.y, fragParams.pOutputs[1]);
                    case 1: fbo->put_pixel(0, fragParams.x, fragParams.y, fragParams.pOutputs[0]);
                        //case 1: fbo->put_pixel(0,fragParams.x, fragParams.y, math::vec4{1.f, 0, 1.f, 1.f});
                }
            }

            if (depthMask)
            {
                fbo->put_depth_pixel<float>(xi, yi, z);
            }
        }

        x += dx;
        y += dy;
    }
}



/*--------------------------------------
 * Wireframe Rasterization
--------------------------------------*/
void SR_FragmentProcessor::render_wireframe(const SR_Texture* depthBuffer) const noexcept
{
    SR_FragCoord*     outCoords    = mQueues;
    const int32_t     yOffset      = (uint32_t)mThreadId;
    const int32_t     increment    = (int32_t)mNumProcessors;
    const int32_t     depthTesting = mShader->fragment_shader().depthTest == SR_DEPTH_TEST_ON;
    SR_ScanlineBounds scanline;

    for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
    {
        const SR_FragmentBin* pBin = mBins+mBinIds[numBinsProcessed];

        uint_fast32_t     numQueuedFrags = 0;
        const math::vec4* pPoints      = pBin->mScreenCoords;
        const math::vec4  depth        {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};
        const math::vec4  homogenous   {pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
        const int32_t     bboxMinY     = (int32_t)math::max(0.f,   math::ceil(math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1])));
        const int32_t     bboxMaxY     = (int32_t)math::min(mFboH, math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]));
        const math::vec4* bcClipSpace  = pBin->mBarycentricCoords;

        // Let each thread start rendering at whichever scanline it's assigned to
        const int32_t scanlineOffset = bboxMinY+sr_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        #if SR_VERTEX_CLIPPING_ENABLED == 0
            scanline.init(pPoints[0], pPoints[1], pPoints[2], mFboW);
        #else
            scanline.init(pPoints[0], pPoints[1], pPoints[2]);
        #endif

        for (int32_t y = bboxMinY+scanlineOffset; y <= bboxMaxY; y += increment)
        {
            // calculate the bounds of the current scan-line
            const float   yf         = (float)y;
            const math::vec4 bcY     = (bcClipSpace[1] * yf) + bcClipSpace[2];

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as a
            // guard against any pixels we don't want to render.
            int32_t xVals[2];
            scanline.step(yf, xVals[0], xVals[1]);

            for (int32_t i = 0, y16 = y << 16; i < 2; ++i)
            {
                const int32_t x = xVals[i];

                // calculate barycentric coordinates
                const float xf   = (float)x;
                math::vec4  bc   = (bcClipSpace[0]*xf) + bcY;

                // Only render pixels within the triangle edges.
                // Ensure the current point is in a triangle by checking the sign
                // bits of all 3 barycentric coordinates.
                const float z = math::dot(depth, bc);
                const float oldDepth = depthBuffer->raw_texel<float>(x, y);

                // We're only using the barycentric coordinates here to determine
                // if a pixel on the edge of a triangle should still be rendered.
                // This normally involves checking if the coordinate is negative,
                // but due to roundoff errors, we need to know if it's close enough
                // to a triangle edge to be rendered.
                #if SR_REVERSED_Z_BUFFER
                    if (depthTesting & (z < oldDepth))
                    {
                        continue;
                    }
                #else
                    if (depthTesting & (z > oldDepth))
                    {
                        continue;
                    }
                #endif

                // perspective correction
                float persp = 1.f / math::dot(homogenous, bc);
                outCoords->bc[numQueuedFrags] = bc * homogenous * persp;

                const math::vec4 outXYZ{xf, yf, z, persp};
                outCoords->xyzw[numQueuedFrags] = outXYZ;

                const uint32_t outXY = (uint32_t)((0xFFFF & x) | y16);
                outCoords->xy[numQueuedFrags] = outXY;
                ++numQueuedFrags;

                if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                {
                    numQueuedFrags = 0;
                    flush_fragments(pBin, SR_SHADER_MAX_FRAG_QUEUES, outCoords);
                }
            }
        }

        // cleanup remaining fragments
        if (numQueuedFrags > 0)
        {
            flush_fragments(pBin, numQueuedFrags, outCoords);
        }
    }
}



/*--------------------------------------
 * Triangle Rasterization, scalar
--------------------------------------*/
void SR_FragmentProcessor::render_triangle(const SR_Texture* depthBuffer) const noexcept
{
    SR_FragCoord*     outCoords    = mQueues;
    const int32_t     yOffset      = (uint32_t)mThreadId;
    const int32_t     increment    = (int32_t)mNumProcessors;
    const int32_t     depthTesting = mShader->fragment_shader().depthTest == SR_DEPTH_TEST_ON;
    SR_ScanlineBounds scanline;

    for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
    {
        const SR_FragmentBin* pBin = mBins+mBinIds[numBinsProcessed];

        uint_fast32_t     numQueuedFrags = 0;
        const math::vec4* pPoints      = pBin->mScreenCoords;
        const math::vec4  depth        {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};
        const math::vec4  homogenous   {pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
        const int32_t     bboxMinY     = (int32_t)math::max(0.f,   math::ceil(math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1])));
        const int32_t     bboxMaxY     = (int32_t)math::min(mFboH, math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]));
        const math::vec4* bcClipSpace  = pBin->mBarycentricCoords;

        // Let each thread start rendering at whichever scanline it's assigned to
        const int32_t scanlineOffset = bboxMinY+sr_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        #if SR_VERTEX_CLIPPING_ENABLED == 0
            scanline.init(pPoints[0], pPoints[1], pPoints[2], mFboW);
        #else
            scanline.init(pPoints[0], pPoints[1], pPoints[2]);
        #endif

        for (int32_t y = scanlineOffset; y <= bboxMaxY; y += increment)
        {
            // calculate the bounds of the current scan-line
            const float        yf  = (float)y;
            const math::vec4&& bcY = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as a
            // guard against any pixels we don't want to render.
            int32_t xMin;
            int32_t xMax;
            scanline.step(yf, xMin, xMax);

            const float* pDepth = depthBuffer->texel_pointer<float>(xMin, y);

            for (int32_t x = xMin, y16 = y << 16; x < xMax; ++x, ++pDepth)
            {
                // calculate barycentric coordinates
                const float  xf = (float)x;
                math::vec4&& bc = math::fmadd(bcClipSpace[0], math::vec4{xf}, bcY);

                // Only render pixels within the triangle edges.
                // Ensure the current point is in a triangle by checking the sign
                // bits of all 3 barycentric coordinates.
                const float z = depthTesting ? math::dot(depth, bc) : pDepth[0];

                // We're only using the barycentric coordinates here to determine
                // if a pixel on the edge of a triangle should still be rendered.
                // This normally involves checking if the coordinate is negative,
                // but due to roundoff errors, we need to know if it's close enough
                // to a triangle edge to be rendered.
                #if SR_REVERSED_Z_BUFFER
                    if (z < pDepth[0])
                    {
                        continue;
                    }
                #else
                    if (z > pDepth[0])
                    {
                        continue;
                    }
                #endif

                // perspective correction
                float persp = 1.f / math::dot(bc, homogenous);
                outCoords->bc[numQueuedFrags]   = (bc * homogenous) * persp;
                outCoords->xyzw[numQueuedFrags] = math::vec4{xf, yf, z, persp};
                outCoords->xy[numQueuedFrags]   = (uint32_t)((0xFFFF & x) | y16);
                ++numQueuedFrags;

                if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                {
                    numQueuedFrags = 0;
                    flush_fragments(pBin, SR_SHADER_MAX_FRAG_QUEUES, outCoords);
                }
            }
        }

        // cleanup remaining fragments
        if (numQueuedFrags > 0)
        {
            flush_fragments(pBin, numQueuedFrags, outCoords);
        }
    }
}



/*-------------------------------------
 * Render a triangle using 4 elements at a time
-------------------------------------*/
void SR_FragmentProcessor::render_triangle_simd(const SR_Texture* depthBuffer) const noexcept
{
    SR_FragCoord*     outCoords    = mQueues;
    const int32_t     yOffset      = (uint32_t)mThreadId;
    const int32_t     increment    = (int32_t)mNumProcessors;
    const int32_t     depthTesting = mShader->fragment_shader().depthTest == SR_DEPTH_TEST_ON;
    SR_ScanlineBounds scanline;

    for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
    {
        const SR_FragmentBin* pBin = mBins+mBinIds[numBinsProcessed];

        uint_fast32_t     numQueuedFrags = 0;
        const math::vec4* pPoints      = pBin->mScreenCoords;
        const math::vec4  depth        {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};
        const math::vec4  homogenous   {pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
        const int32_t     bboxMinY     = (int32_t)math::max(0.f,   math::ceil(math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1])));
        const int32_t     bboxMaxY     = (int32_t)math::min(mFboH, math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]));
        const math::vec4* bcClipSpace  = pBin->mBarycentricCoords;

        // Let each thread start rendering at whichever scanline it's assigned to
        const int32_t scanlineOffset = bboxMinY+sr_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

        #if SR_VERTEX_CLIPPING_ENABLED == 0
            scanline.init(pPoints[0], pPoints[1], pPoints[2], mFboW);
        #else
            scanline.init(pPoints[0], pPoints[1], pPoints[2]);
        #endif

        for (int32_t y = scanlineOffset; y <= bboxMaxY; y += increment)
        {
            // calculate the bounds of the current scan-line
            const float yf = (float)y;
            const math::vec4&& bcY = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);

            int32_t xMin;
            int32_t xMax;
            scanline.step(yf, xMin, xMax);
            math::vec4i&& x    = math::vec4i{0, 1, 2, 3} + xMin;

            // I'm pretty sure Z-ordering has been ruled out at this point.
            const float* pDepth = depthBuffer->row_pointer<float>((uintptr_t)y) + xMin;

            for (uint32_t y16 = (uint32_t)(y << 16u); x[0] <= xMax; x += 4, pDepth += 4)
            {
                const math::vec4&& xf = (math::vec4)x;

                // calculate barycentric coordinates and perform a depth test
                const math::mat4&& bc  = math::outer(xf, bcClipSpace[0]) + bcY;
                const math::vec4&& z   = depthTesting ? (depth * bc) : math::vec4{pDepth[0], pDepth[1], pDepth[2], pDepth[3]};
                const int_fast32_t end = math::min<int_fast32_t>(xMax-x[0], 4);

                for (int32_t i = 0; i < end; ++i)
                {
                    #if SR_REVERSED_Z_BUFFER
                        if (z[i] < pDepth[i])
                        {
                            continue;
                        }
                    #else
                        if (z[i] > pDepth[i])
                        {
                            continue;
                        }
                    #endif

                    // perspective correction
                    const float persp = 1.f / math::dot(homogenous, bc[i]);
                    outCoords->bc[numQueuedFrags]   = (bc[i] * homogenous) * persp;
                    outCoords->xyzw[numQueuedFrags] = math::vec4{xf[i], yf, z[i], persp};
                    outCoords->xy[numQueuedFrags]   = (uint32_t)((0xFFFF & x[i]) | y16);

                    ++numQueuedFrags;

                    if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                    {
                        numQueuedFrags = 0;
                        flush_fragments(pBin, SR_SHADER_MAX_FRAG_QUEUES, outCoords);
                    }
                }
            }
        }

        if (numQueuedFrags)
        {
            flush_fragments(pBin, numQueuedFrags, outCoords);
        }
    }
}



/*--------------------------------------
 * Triangle Fragment Bin-Rasterization
--------------------------------------*/
void SR_FragmentProcessor::flush_fragments(
    const SR_FragmentBin* pBin,
    uint_fast32_t         numQueuedFrags,
    const SR_FragCoord*   outCoords) const noexcept
{
    LS_PREFETCH(pBin->mVaryings, LS_PREFETCH_ACCESS_R, LS_PREFETCH_LEVEL_L1);
    const SR_UniformBuffer* pUniforms   = mShader->mUniforms;
    const SR_FragmentShader fragShader  = mShader->mFragShader;
    const uint_fast32_t     numVaryings = fragShader.numVaryings;
    const uint_fast32_t     numOutputs  = fragShader.numOutputs;
    const SR_BlendMode      blendMode   = fragShader.blend;
    const uint_fast32_t     depthMask   = fragShader.depthMask == SR_DEPTH_MASK_ON;
    const auto              pShader     = fragShader.shader;
    SR_Framebuffer*         fbo         = mFbo;
    SR_Texture*             pDepthBuf   = fbo->get_depth_buffer();
    const math::vec4* const inVaryings  = pBin->mVaryings;

    SR_FragmentParam fragParams;
    fragParams.pUniforms = pUniforms;
    fragParams.pVaryings = mVaryings;

    // Interpolate varying variables using the barycentric coordinates. I'm
    // interpolating here to maintain cache coherence.
    math::vec4* pVaryings = mVaryings;
    for (uint_fast32_t i = 0; i < numQueuedFrags; ++i, pVaryings += SR_SHADER_MAX_VARYING_VECTORS)
    {
        const math::vec4& bc = outCoords->bc[i];
        interpolate_tri_varyings(bc.v, numVaryings, inVaryings, pVaryings);
    }

    if (blendMode != SR_BLEND_OFF)
    {
        for (uint_fast32_t i = 0; i < numQueuedFrags; ++i)
        {
            fragParams.fragCoord = outCoords->xyzw[i];
            fragParams.x         = outCoords->coord[i].x;
            fragParams.y         = outCoords->coord[i].y;
            
            uint_fast32_t haveOutputs = pShader(fragParams);

            // branchless select
            switch (-haveOutputs & numOutputs)
            {
                case 4: fbo->put_alpha_pixel(3, fragParams.x, fragParams.y, fragParams.pOutputs[3], blendMode);
                case 3: fbo->put_alpha_pixel(2, fragParams.x, fragParams.y, fragParams.pOutputs[2], blendMode);
                case 2: fbo->put_alpha_pixel(1, fragParams.x, fragParams.y, fragParams.pOutputs[1], blendMode);
                case 1: fbo->put_alpha_pixel(0, fragParams.x, fragParams.y, fragParams.pOutputs[0], blendMode);
            }

            if (depthMask)
            {
                pDepthBuf->raw_texel<float>(fragParams.x, fragParams.y) = fragParams.fragCoord[2];
            }

            fragParams.pVaryings += SR_SHADER_MAX_VARYING_VECTORS;
        }
    }
    else
    {
        for (uint_fast32_t i = 0; i < numQueuedFrags; ++i)
        {
            fragParams.fragCoord = outCoords->xyzw[i];
            fragParams.x         = outCoords->coord[i].x;
            fragParams.y         = outCoords->coord[i].y;

            uint_fast32_t haveOutputs = pShader(fragParams);

            // branchless select
            switch (-haveOutputs & numOutputs)
            {
                case 4: fbo->put_pixel(3, fragParams.x, fragParams.y, fragParams.pOutputs[3]);
                case 3: fbo->put_pixel(2, fragParams.x, fragParams.y, fragParams.pOutputs[2]);
                case 2: fbo->put_pixel(1, fragParams.x, fragParams.y, fragParams.pOutputs[1]);
                case 1: fbo->put_pixel(0, fragParams.x, fragParams.y, fragParams.pOutputs[0]);
            }

            if (depthMask)
            {
                pDepthBuf->raw_texel<float>(fragParams.x, fragParams.y) = fragParams.fragCoord[2];
            }

            fragParams.pVaryings += SR_SHADER_MAX_VARYING_VECTORS;
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
                render_point(mBinIds[numBinsProcessed], mFbo, dimens);
            }
            break;
        }

        case RENDER_MODE_LINES:
        case RENDER_MODE_INDEXED_LINES:
        {
            // divide the screen into equal parts which can then be rendered by all
            // available fragment threads.
            const int32_t w = mFbo->width();
            const int32_t h = mFbo->height();
            const math::vec4_t<int32_t> dimens = sr_subdivide_region<int32_t>(w, h, mNumProcessors, mThreadId);

            for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
            {
                render_line(mBinIds[numBinsProcessed], mFbo, dimens);
            }
            break;
        }
        case RENDER_MODE_TRI_WIRE:
        case RENDER_MODE_INDEXED_TRI_WIRE:
            render_wireframe(mFbo->get_depth_buffer());
            break;

        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
            // Triangles assign scan-lines per thread for rasterization.
            // There's No need to subdivide the output framebuffer
            render_triangle(mFbo->get_depth_buffer());
            //render_triangle_simd(mFbo->get_depth_buffer());
            break;
    }
}
