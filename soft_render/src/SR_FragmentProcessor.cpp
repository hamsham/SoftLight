
#include <iostream>

#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/math/bits.h"
#include "lightsky/math/fixed.h"
#include "lightsky/math/mat4.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_Framebuffer.hpp" // SR_Framebuffer
#include "soft_render/SR_Shader.hpp" // SR_FragmentShader
#include "soft_render/SR_ShaderProcessor.hpp" // SR_FragmentBin
#include "soft_render/SR_Texture.hpp"



#ifndef SR_REVERSED_Z_BUFFER
    #define SR_REVERSED_Z_BUFFER 1
#endif



#ifndef SR_VERTEX_CLIPPING_ENABLED
    #define SR_VERTEX_CLIPPING_ENABLED 1
#endif /* SR_VERTEX_CLIPPING_ENABLED */



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

        const __m128 bc  = _mm_load_ps(baryCoords);
        const __m128 bc0 = _mm_permute_ps(bc, 0x00);
        const __m128 bc1 = _mm_permute_ps(bc, 0x55);
        const __m128 bc2 = _mm_permute_ps(bc, 0xAA);

        for (uint_fast32_t i = numVaryings; i--;)
        {
            const __m128 v0 = _mm_mul_ps(_mm_load_ps(reinterpret_cast<const float*>(inVaryings0++)), bc0);
            const __m128 v1 = _mm_fmadd_ps(_mm_load_ps(reinterpret_cast<const float*>(inVaryings1++)), bc1, v0);
            _mm_store_ps(reinterpret_cast<float*>(outVaryings++), _mm_fmadd_ps(_mm_load_ps(reinterpret_cast<const float*>(inVaryings2++)), bc2, v1));
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



/*-----------------------------------------------------------------------------
 * Common method to get the beginning and end of a scanline.
-----------------------------------------------------------------------------*/
struct SR_ScanlineBounds
{

    const float p10x;
    const float p20x;
    const float p21x;
    const float p10y;
    const float p21y;
    const float p20y;
    const float p10xy;
    const float p21xy;

    const int32_t bboxMinX;
    const int32_t bboxMaxX;

    int32_t xMin;
    int32_t xMax;

    #if SR_VERTEX_CLIPPING_ENABLED == 0
    SR_ScanlineBounds(const math::vec2& p0, const math::vec2& p1, const math::vec2& p2, const float fboW) noexcept :
        p10x{p1[0] - p0[0]},
        p20x{p2[0] - p0[0]},
        p21x{p2[0] - p1[0]},
        p10y{math::rcp(p1[1] - p0[1])},
        p21y{math::rcp(p2[1] - p1[1])},
        p20y{math::rcp(p2[1] - p0[1])},
        p10xy{p10x * p10y},
        p21xy{p21x * p21y},
        bboxMinX{(int32_t)math::min(fboW, math::max(0.f,  math::min(p0[0], p1[0], p2[0])))},
        bboxMaxX{(int32_t)math::max(0.f,  math::min(fboW, math::max(p0[0], p1[0], p2[0]))+0.5f)}
    {}
    #else
    SR_ScanlineBounds(const math::vec2& p0, const math::vec2& p1, const math::vec2& p2) noexcept :
        p10x{p1[0] - p0[0]},
        p20x{p2[0] - p0[0]},
        p21x{p2[0] - p1[0]},
        p10y{math::rcp(p1[1] - p0[1])},
        p21y{math::rcp(p2[1] - p1[1])},
        p20y{math::rcp(p2[1] - p0[1])},
        p10xy{p10x * p10y},
        p21xy{p21x * p21y},
        bboxMinX{(int32_t)math::min(p0[0], p1[0], p2[0])},
        bboxMaxX{(int32_t)math::max(p0[0], p1[0], p2[0])}
    {}
    #endif

    inline void step(const math::vec2& p0, const math::vec2& p1, const float yf) noexcept
    {
        const float d0         = yf - p0[1];
        const float d1         = yf - p1[1];
        const float alpha      = d0 * p20y;
        const int   secondHalf = -(d1 < 0.f);

        xMin = (int32_t)math::fmadd(p20x, alpha, p0[0]);

        const float a = math::fmadd(p21xy, d1, p1[0]);
        const float b = math::fmadd(p10xy, d0, p0[0]);
        xMax = (int32_t)(*(const float*)((secondHalf & (uintptr_t)&a) | (~secondHalf & (uintptr_t)&b)));

        // Get the beginning and end of the scan-line
        const int32_t temp = math::max(xMin, xMax);
        xMin = math::clamp<int32_t>(math::min(xMin, xMax), bboxMinX, bboxMaxX);
        xMax = temp;

        /*
        xMin = math::clamp(xMin, bboxMinX, bboxMaxX);
        xMax = math::clamp(xMax, bboxMinX, bboxMaxX);
        */
    }
};



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
    fragParams.depth = fragCoord[2];
    fragParams.pUniforms = pUniforms;

    for (unsigned i = SR_SHADER_MAX_VARYING_VECTORS; i--;)
    {
        mVaryings[i] = mBins[binId].mVaryings[i];
    }

    if (depthTest == SR_DEPTH_TEST_ON)
    {
#if SR_REVERSED_Z_BUFFER
        if (fragParams.depth < pDepthBuf->raw_texel<float>(fragParams.x, fragParams.y))
#else
        if (fragParams.depth > pDepthBuf->raw_texel<float>(fragParams.x, fragParams.y))
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
        fbo->put_depth_pixel<float>(fragParams.x, fragParams.y, fragParams.depth);
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
        const int32_t    xi      = math::integer_cast<int32_t, fixed_type>(math::ceil(x));
        const int32_t    yi      = math::integer_cast<int32_t, fixed_type>(math::ceil(y));

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
            fragParams.z = z;
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
void SR_FragmentProcessor::render_wireframe(const SR_FragmentBin* pBins, const SR_Texture* depthBuffer) const noexcept
{
    uint32_t          numQueuedFrags = 0;
    const int32_t     yOffset      = (uint32_t)mThreadId;
    const int32_t     increment    = (int32_t)mNumProcessors;
    const math::vec4* screenCoords = pBins->mScreenCoords;
    const math::vec4  depth        {screenCoords[0][2], screenCoords[1][2], screenCoords[2][2], 0.f};
    const math::vec4  homogenous   {screenCoords[0][3], screenCoords[1][3], screenCoords[2][3], 0.f};
    math::vec2        p0           = math::vec2_cast(screenCoords[0]);
    math::vec2        p1           = math::vec2_cast(screenCoords[1]);
    math::vec2        p2           = math::vec2_cast(screenCoords[2]);
    const math::vec4* bcClipSpace  = pBins->mBarycentricCoords;
    const int32_t     depthTesting = mShader->fragment_shader().depthTest == SR_DEPTH_TEST_ON;
    const int32_t     bboxMinY     = (int32_t)math::ceil(math::min(p0[1], p1[1], p2[1]));
    const int32_t     bboxMaxY     = (int32_t)math::max(p0[1], p1[1], p2[1]);
    SR_FragCoord*     outCoords    = mQueues;

    if (p0[1] < p1[1]) std::swap(p0, p1);
    if (p0[1] < p2[1]) std::swap(p0, p2);
    if (p1[1] < p2[1]) std::swap(p1, p2);

    #if SR_VERTEX_CLIPPING_ENABLED == 0
    SR_ScanlineBounds scanline{p0, p1, p2, mFboW};
    #else
    SR_ScanlineBounds scanline{p0, p1, p2};
    #endif

    // Let each thread start rendering at whichever scanline it's assigned to
    const int32_t scanlineOffset = sr_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

    for (int32_t y = bboxMinY+scanlineOffset; y <= bboxMaxY; y += increment)
    {
        // calculate the bounds of the current scan-line
        const float   yf         = (float)y;
        const math::vec4 bcY     = (bcClipSpace[1] * yf) + bcClipSpace[2];

        // In this rasterizer, we're only rendering the absolute pixels
        // contained within the triangle edges. However this will serve as a
        // guard against any pixels we don't want to render.
        scanline.step(p0, p1, yf);
        const int32_t xVals[2] = {scanline.xMin, scanline.xMax};

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
                if (depthTesting && (z < oldDepth))
                {
                    continue;
                }
            #else
                if (depthTesting && (z > oldDepth))
                {
                    continue;
                }
            #endif

            // perspective correction
            float persp = math::dot(homogenous, bc);
            outCoords->bc[numQueuedFrags] = bc * homogenous / persp;

            const math::vec4 outXYZ{xf, yf, z, persp};
            outCoords->xyzw[numQueuedFrags] = outXYZ;

            const uint32_t outXY = (uint32_t)((0xFFFF & x) | y16);
            outCoords->xy[numQueuedFrags] = outXY;
            ++numQueuedFrags;

            if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
            {
                numQueuedFrags = 0;
                flush_fragments(pBins, SR_SHADER_MAX_FRAG_QUEUES, outCoords);
            }
        }
    }

    // cleanup remaining fragments
    if (numQueuedFrags > 0)
    {
        flush_fragments(pBins, numQueuedFrags, outCoords);
    }
}



/*--------------------------------------
 * Triangle Rasterization, scalar
--------------------------------------*/
void SR_FragmentProcessor::render_triangle(const SR_FragmentBin* pBin, const SR_Texture* depthBuffer) const noexcept
{
    uint32_t  numQueuedFrags = 0;
    const int32_t     yOffset      = (uint32_t)mThreadId;
    const int32_t     increment    = (int32_t)mNumProcessors;
    const math::vec4* screenCoords = pBin->mScreenCoords;
    const math::vec4  depth        {screenCoords[0][2], screenCoords[1][2], screenCoords[2][2], 0.f};
    const math::vec4  homogenous   {screenCoords[0][3], screenCoords[1][3], screenCoords[2][3], 0.f};
    math::vec2        p0           = math::vec2_cast(screenCoords[0]);
    math::vec2        p1           = math::vec2_cast(screenCoords[1]);
    math::vec2        p2           = math::vec2_cast(screenCoords[2]);
    const math::vec4* bcClipSpace  = pBin->mBarycentricCoords;
    const int32_t     depthTesting = mShader->fragment_shader().depthTest == SR_DEPTH_TEST_ON;
    #if SR_VERTEX_CLIPPING_ENABLED == 0
        const int32_t     bboxMinY     = (int32_t)math::min(mFboH, math::max(0.f,   math::min(p0[1], p1[1], p2[1]))+0.5f);
        const int32_t     bboxMaxY     = (int32_t)math::max(0.f,   math::min(mFboH, math::max(p0[1], p1[1], p2[1])));
    #else
        const int32_t     bboxMinY     = (int32_t)math::ceil(math::min(p0[1], p1[1], p2[1]));
        const int32_t     bboxMaxY     = (int32_t)math::max(p0[1], p1[1], p2[1]);
    #endif
    SR_FragCoord*     outCoords    = mQueues;

    if (p0[1] < p1[1]) std::swap(p0, p1);
    if (p0[1] < p2[1]) std::swap(p0, p2);
    if (p1[1] < p2[1]) std::swap(p1, p2);

    #if SR_VERTEX_CLIPPING_ENABLED == 0
    SR_ScanlineBounds scanline{p0, p1, p2, mFboW};
    #else
    SR_ScanlineBounds scanline{p0, p1, p2};
    #endif

    // Let each thread start rendering at whichever scanline it's assigned to
    const int32_t scanlineOffset = sr_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

    for (int32_t y = bboxMinY+scanlineOffset; y <= bboxMaxY; y += increment)
    {
        // calculate the bounds of the current scan-line
        const float   yf         = (float)y;
        const math::vec4 bcY     = (bcClipSpace[1] * yf) + bcClipSpace[2];

        // In this rasterizer, we're only rendering the absolute pixels
        // contained within the triangle edges. However this will serve as a
        // guard against any pixels we don't want to render.
        scanline.step(p0, p1, yf);
        const int32_t xMin = scanline.xMin;
        const int32_t xMax = scanline.xMax;

        for (int32_t x = xMin, y16 = y << 16; x <= xMax; ++x)
        {
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
                if (depthTesting && (z < oldDepth))
                {
                    continue;
                }
            #else
                if (depthTesting && (z > oldDepth))
                {
                    continue;
                }
            #endif

            // perspective correction
            float persp = 1.f / math::dot(homogenous, bc);
            outCoords->bc[numQueuedFrags]   = bc * homogenous * persp;
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



/*-------------------------------------
 * Render a triangle using 4 elements at a time
-------------------------------------*/
void SR_FragmentProcessor::render_triangle_simd(const SR_FragmentBin* pBin, const SR_Texture* depthBuffer) const noexcept
{
    const uint32_t    yOffset      = (uint32_t)mThreadId;
    const uint32_t    increment    = (uint32_t)mNumProcessors;
    const math::vec4* screenCoords = pBin->mScreenCoords;
    const math::vec4  depth        {screenCoords[0][2], screenCoords[1][2], screenCoords[2][2], 0.f};
    const math::vec4  homogenous   {screenCoords[0][3], screenCoords[1][3], screenCoords[2][3], 0.f};
    math::vec2&&      p0           = math::vec2_cast(screenCoords[0]);
    math::vec2&&      p1           = math::vec2_cast(screenCoords[1]);
    math::vec2&&      p2           = math::vec2_cast(screenCoords[2]);
    const math::vec4* bcClipSpace  = pBin->mBarycentricCoords;
    const int32_t     depthTesting = -(mShader->fragment_shader().depthTest == SR_DEPTH_TEST_ON);
    #if SR_VERTEX_CLIPPING_ENABLED == 0
        const int32_t bboxMinY     = (int32_t)math::min(mFboH, math::max(0.f,   math::min(p0[1], p1[1], p2[1]))+0.5f);
        const int32_t bboxMaxY     = (int32_t)math::max(0.f,   math::min(mFboH, math::max(p0[1], p1[1], p2[1])));
    #else
        const int32_t bboxMinY     = (int32_t)math::ceil(math::min(p0[1], p1[1], p2[1]));
        const int32_t bboxMaxY     = (int32_t)math::max(p0[1], p1[1], p2[1]);
    #endif
    SR_FragCoord*     outCoords    = mQueues;

    if (p0[1] < p1[1]) std::swap(p0, p1);
    if (p0[1] < p2[1]) std::swap(p0, p2);
    if (p1[1] < p2[1]) std::swap(p1, p2);

    #if SR_VERTEX_CLIPPING_ENABLED == 0
    SR_ScanlineBounds scanline{p0, p1, p2, mFboW};
    #else
    SR_ScanlineBounds scanline{p0, p1, p2};
    #endif

    unsigned numQueuedFrags = 0;
    const int32_t scanlineOffset = sr_scanline_offset<int32_t>(increment, yOffset, bboxMinY);

    for (int32_t y = bboxMinY+scanlineOffset; y <= bboxMaxY; y += increment)
    {
        // calculate the bounds of the current scan-line
        const float yf = (float)y;
        const math::vec4&& bcY = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);

        scanline.step(p0, p1, yf);
        const int32_t xMin = scanline.xMin;
        const int32_t xMax = scanline.xMax;
        math::vec4i&& x    = math::vec4i{0, 1, 2, 3} + xMin;
        math::vec4&&  xf   = math::vec4{0.f, 1.f, 2.f, 3.f} + (float)xMin;

        // I'm pretty sure Z-ordering has been ruled out at this point.
        const float* pDepth = (const float*)depthBuffer->raw_texel_pointer<float>((uint16_t)xMin, (uint16_t)y);

        //for (uint16_t y16 = (uint16_t)y; x[0] <= xMax;)
        for (uint32_t y16 = (uint32_t)(y << 16); x[0] <= xMax;)
        {
            // calculate barycentric coordinates and perform a depth test
            const math::mat4&& bc          = math::outer(xf, bcClipSpace[0]) + bcY;
            const math::vec4&& z           = depth * bc;
            const math::vec4&& depthTexels = depthTesting ? *reinterpret_cast<const math::vec4*>(pDepth) : math::vec4{0.f};
            //const math::vec4&& depthTexels = depthTesting ? depthBuffer->texel4<float>(x0, y) : math::vec4{0.f};
    #if SR_REVERSED_Z_BUFFER
            const int_fast32_t      depthTest   = depthTesting ? math::sign_mask(z-depthTexels) : 0x00;
    #else
            const int_fast32_t      depthTest   = depthTesting ? math::sign_mask(depthTexels-z) : 0x00;
    #endif
            const int_fast32_t      end         = -(depthTest != 0x0F) & math::min<int_fast32_t>(xMax-x[0], 4);

            for (int32_t i = 0; i < end; ++i)
            {
                if (depthTest & (0x01 << i))
                {
                    continue;
                }

                // perspective correction
                const float persp = math::rcp(math::dot(homogenous, bc[i]));
                outCoords->bc[numQueuedFrags] = (bc[i] * homogenous) * persp;
                outCoords->xyzw[numQueuedFrags] = math::vec4{xf[i], yf, z[i], persp};
                outCoords->xy[numQueuedFrags] = (uint32_t)((0xFFFF & x[i]) | y16);
                //outCoords->coord[numQueuedFrags].x = (uint16_t)x[i];
                //outCoords->coord[numQueuedFrags].y = y16;

                ++numQueuedFrags;

                if (numQueuedFrags == SR_SHADER_MAX_FRAG_QUEUES)
                {
                    numQueuedFrags = 0;
                    flush_fragments(pBin, SR_SHADER_MAX_FRAG_QUEUES, outCoords);
                }
            }

            pDepth += 4;
            x += 4;
            xf += 4.f;
        }
    }

    if (numQueuedFrags)
    {
        flush_fragments(pBin, numQueuedFrags, outCoords);
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
        const math::vec4 bc = outCoords->bc[i];
        interpolate_tri_varyings(bc.v, numVaryings, inVaryings, pVaryings);
    }

    if (blendMode != SR_BLEND_OFF)
    {
        for (uint_fast32_t i = 0; i < numQueuedFrags; ++i)
        {
            fragParams.fragCoord = outCoords->xyzw[i];
            fragParams.depth     = outCoords->xyzw[i][2];
            fragParams.x         = outCoords->coord[i].x;
            fragParams.y         = outCoords->coord[i].y;
            fragParams.z         = 0;

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
                pDepthBuf->raw_texel<float>(fragParams.x, fragParams.y) = fragParams.depth;
            }

            fragParams.pVaryings += SR_SHADER_MAX_VARYING_VECTORS;
        }
    }
    else
    {
        for (uint_fast32_t i = 0; i < numQueuedFrags; ++i)
        {
            fragParams.fragCoord = outCoords->xyzw[i];
            fragParams.depth     = outCoords->xyzw[i][2];
            fragParams.x         = outCoords->coord[i].x;
            fragParams.y         = outCoords->coord[i].y;
            fragParams.z         = 0;

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
                pDepthBuf->raw_texel<float>(fragParams.x, fragParams.y) = fragParams.depth;
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
        {
            for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
            {
                render_wireframe(mBins+mBinIds[numBinsProcessed], mFbo->get_depth_buffer());
            }
            break;
        }

        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
            // Triangles assign scan-lines per thread for rasterization.
            // There's No need to subdivide the output framebuffer
            for (uint64_t numBinsProcessed = 0; numBinsProcessed < mNumBins; ++numBinsProcessed)
            {
                const SR_FragmentBin* pBin = mBins+mBinIds[numBinsProcessed];
                //render_triangle(pBin, mFbo->get_depth_buffer());
                render_triangle_simd(pBin, mFbo->get_depth_buffer());
            }
            break;
    }
}
