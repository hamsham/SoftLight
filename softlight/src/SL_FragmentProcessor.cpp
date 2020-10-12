
#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/math/bits.h"
#include "lightsky/math/fixed.h"
#include "lightsky/math/half.h"
#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "softlight/SL_Config.hpp"
#include "softlight/SL_FragmentProcessor.hpp"
#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_ScanlineBounds.hpp"
#include "softlight/SL_Shader.hpp" // SL_FragmentShader
#include "softlight/SL_ShaderUtil.hpp" // sl_scanline_offset()
#include "softlight/SL_ShaderProcessor.hpp" // SL_FragmentBin
#include "softlight/SL_Texture.hpp"



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
    const float*      LS_RESTRICT_PTR baryCoords,
    uint_fast32_t     numVaryings,
    const math::vec4* LS_RESTRICT_PTR inVaryings0,
    math::vec4*       LS_RESTRICT_PTR outVaryings) noexcept
{
    static_assert(SL_SHADER_MAX_VARYING_VECTORS == 4, "Please update the varying interpolator.");

    #if defined(LS_ARCH_X86)
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
        const __m128 bc0 = _mm_broadcast_ss(baryCoords+0);
        v0 = _mm_mul_ps(bc0, a);
        v1 = _mm_mul_ps(bc0, b);
        v2 = _mm_mul_ps(bc0, c);
        v3 = _mm_mul_ps(bc0, d);

        a = _mm_load_ps(i1 + 0);
        b = _mm_load_ps(i1 + 4);
        c = _mm_load_ps(i1 + 8);
        d = _mm_load_ps(i1 + 12);
        const __m128 bc1 = _mm_broadcast_ss(baryCoords+1);
        v0 = _mm_fmadd_ps(bc1, a, v0);
        v1 = _mm_fmadd_ps(bc1, b, v1);
        v2 = _mm_fmadd_ps(bc1, c, v2);
        v3 = _mm_fmadd_ps(bc1, d, v3);

        a = _mm_load_ps(i2 + 0);
        b = _mm_load_ps(i2 + 4);
        c = _mm_load_ps(i2 + 8);
        d = _mm_load_ps(i2 + 12);
        const __m128 bc2 = _mm_broadcast_ss(baryCoords+2);
        v0 = _mm_fmadd_ps(bc2, a, v0);
        v1 = _mm_fmadd_ps(bc2, b, v1);
        v2 = _mm_fmadd_ps(bc2, c, v2);
        v3 = _mm_fmadd_ps(bc2, d, v3);

        _mm_store_ps(o + 0,  v0);
        _mm_store_ps(o + 4,  v1);
        _mm_store_ps(o + 8,  v2);
        _mm_store_ps(o + 12, v3);

    #elif defined(LS_ARCH_ARM)
        const math::vec4* LS_RESTRICT_PTR inVaryings1 = inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS;
        const math::vec4* LS_RESTRICT_PTR inVaryings2 = inVaryings0 + SL_SHADER_MAX_VARYING_VECTORS * 2;

        const float32x4_t bc  = vld1q_f32(baryCoords);
        const float32x4_t bc0 = vdupq_n_f32(vgetq_lane_f32(bc, 0));
        const float32x4_t bc1 = vdupq_n_f32(vgetq_lane_f32(bc, 1));
        const float32x4_t bc2 = vdupq_n_f32(vgetq_lane_f32(bc, 2));

        for (uint_fast32_t i = numVaryings; i--;)
        {
            const float32x4_t v0 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings0++)), bc0);
            const float32x4_t v1 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings1++)), bc1);
            const float32x4_t v2 = vmulq_f32(vld1q_f32(reinterpret_cast<const float*>(inVaryings2++)), bc2);
            vst1q_f32(reinterpret_cast<float*>(outVaryings++), vaddq_f32(v2, vaddq_f32(v1, v0)));
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

#endif



/*--------------------------------------
 * Load and convert 4 depth texels from memory
--------------------------------------*/
template <typename depth_type>
inline LS_INLINE math::vec4 _sl_get_depth_texel4(const depth_type* pDepth)
{
    return (math::vec4)(*reinterpret_cast<const math::vec4_t<depth_type>*>(pDepth));
}

#if defined(LS_ARCH_X86)
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

#elif defined(LS_ARCH_ARM)
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



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_FragmentProcessor Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Determine if a point can be rendered.
--------------------------------------*/
template <typename depth_type>
void SL_FragmentProcessor::render_point(
    const uint32_t binId,
    SL_Framebuffer* const fbo,
    const ls::math::vec4_t<int32_t> dimens) noexcept
{
    const SL_FragmentShader fragShader  = mShader->mFragShader;
    const SL_UniformBuffer* pUniforms   = mShader->mUniforms;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const bool              depthTest   = fragShader.depthTest == SL_DEPTH_TEST_ON;
    const bool              depthMask   = fragShader.depthMask == SL_DEPTH_MASK_ON;
    const auto              pShader     = fragShader.shader;
    const SL_BlendMode      blendMode   = fragShader.blend;
    const bool              blend       = blendMode != SL_BLEND_OFF;
    const math::vec4        screenCoord = mBins[binId].mScreenCoords[0];
    const math::vec4        fragCoord   {screenCoord[0], screenCoord[1], screenCoord[2], 1.f};
    const SL_Texture*       pDepthBuf   = fbo->get_depth_buffer();
    SL_FragmentParam        fragParams;

    if (fragCoord.v[0] < dimens[0] || fragCoord.v[0] > dimens[1] || fragCoord.v[1] < dimens[2] || fragCoord.v[1] > dimens[3])
    {
        return;
    }

    fragParams.coord.x = (uint16_t)fragCoord[0];
    fragParams.coord.y = (uint16_t)fragCoord[1];
    fragParams.coord.depth = fragCoord[2];
    fragParams.pUniforms = pUniforms;

    if (depthTest == SL_DEPTH_TEST_ON)
    {
#if SL_REVERSED_Z_RENDERING
        if (fragCoord[2] < (float)pDepthBuf->raw_texel<depth_type>(fragParams.coord.x, fragParams.coord.y))
#else
        if (fragCoord[2] > (float)pDepthBuf->raw_texel<depth_type>(fragParams.coord.x, fragParams.coord.y))
#endif
        {
            return;
        }
    }

    for (unsigned i = SL_SHADER_MAX_VARYING_VECTORS; i--;)
    {
        fragParams.pVaryings[i] = mBins[binId].mVaryings[i];
    }

    const uint_fast32_t haveOutputs = pShader(fragParams);

    if (blend)
    {
        // branchless select
        switch (-haveOutputs & numOutputs)
        {
            case 4: fbo->put_alpha_pixel(3, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3], blendMode);
            case 3: fbo->put_alpha_pixel(2, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2], blendMode);
            case 2: fbo->put_alpha_pixel(1, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1], blendMode);
            case 1: fbo->put_alpha_pixel(0, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0], blendMode);
                //case 1: fbo->put_pixel(0, x0, y0, math::vec4{1.f, 0, 1.f, 1.f});
        }

    }
    else
    {
        // branchless select
        switch (-haveOutputs & numOutputs)
        {
            case 4: fbo->put_pixel(3, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3]);
            case 3: fbo->put_pixel(2, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2]);
            case 2: fbo->put_pixel(1, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1]);
            case 1: fbo->put_pixel(0, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0]);
                //case 1: fbo->put_pixel(0, x0, y0, math::vec4{1.f, 0, 1.f, 1.f});
        }
    }

    if (haveOutputs && depthMask)
    {
        fbo->put_depth_pixel<depth_type>(fragParams.coord.x, fragParams.coord.y, (depth_type)fragParams.coord.depth);
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

bool sl_clip_liang_barsky(math::vec2& a, math::vec2& b, const math::vec4_t<int32_t>& dimens)
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

inline bool sl_clip_liang_barsky(math::vec2 screenCoords[2], const math::vec4_t<int32_t>& dimens)
{
    return sl_clip_liang_barsky(screenCoords[0], screenCoords[1], dimens);
}



/*--------------------------------------
 * Process the line fragments using a simple DDA algorithm
--------------------------------------*/
template <typename depth_type>
void SL_FragmentProcessor::render_line(
    const uint32_t binId,
    SL_Framebuffer* fbo,
    const math::vec4_t<int32_t> dimens) noexcept
{
    typedef math::long_medp_t fixed_type;
    constexpr fixed_type ZERO = fixed_type{0};

    const SL_FragmentShader fragShader  = mShader->mFragShader;
    const SL_BlendMode      blendMode   = fragShader.blend;
    const bool              blend       = blendMode != SL_BLEND_OFF;
    const uint32_t          numVaryings = fragShader.numVaryings;
    const uint32_t          numOutputs  = fragShader.numOutputs;
    const bool              noDepthTest = fragShader.depthTest == SL_DEPTH_TEST_OFF;
    const bool              depthMask   = fragShader.depthMask == SL_DEPTH_MASK_ON;
    const auto              shader      = fragShader.shader;
    const SL_UniformBuffer* pUniforms   = mShader->mUniforms;

    const math::vec4  screenCoord0  = mBins[binId].mScreenCoords[0];
    const math::vec4  screenCoord1  = mBins[binId].mScreenCoords[1];
    float             z0            = screenCoord0[2];
    float             z1            = screenCoord1[2];
    const math::vec4* inVaryings    = mBins[binId].mVaryings;
    math::vec2        clipCoords[2] = {math::vec2_cast(screenCoord0), math::vec2_cast(screenCoord1)};

    if (!sl_clip_liang_barsky(clipCoords, dimens))
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
    const SL_Texture* depthBuf = fbo->get_depth_buffer();

    SL_FragmentParam fragParams;
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

#if SL_REVERSED_Z_RENDERING
        if (noDepthTest || (float)depthBuf->raw_texel<depth_type>((uint16_t)xi, (uint16_t)yi) <= z)
#else
        if (noDepthTest || (float)depthBuf->raw_texel<depth_type>((uint16_t)xi, (uint16_t)yi) >= z)
#endif
        {
            interpolate_line_varyings(interp, numVaryings, inVaryings, fragParams.pVaryings);

            fragParams.coord.x     = (uint16_t)xi;
            fragParams.coord.y     = (uint16_t)yi;
            fragParams.coord.depth = z;
            const uint_fast32_t haveOutputs = shader(fragParams);

            if (blend)
            {
                // branchless select
                switch (-haveOutputs & numOutputs)
                {
                    case 4: fbo->put_alpha_pixel(3, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3], blendMode);
                    case 3: fbo->put_alpha_pixel(2, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2], blendMode);
                    case 2: fbo->put_alpha_pixel(1, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1], blendMode);
                    case 1: fbo->put_alpha_pixel(0, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0], blendMode);
                        //case 1: fbo->put_pixel(0, fragParams.x, fragParams.y, math::vec4{1.f, 0, 1.f, 1.f});
                }

            }
            else
            {
                // branchless select
                switch (-haveOutputs & numOutputs)
                {
                    case 4: fbo->put_pixel(3, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3]);
                    case 3: fbo->put_pixel(2, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2]);
                    case 2: fbo->put_pixel(1, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1]);
                    case 1: fbo->put_pixel(0, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0]);
                        //case 1: fbo->put_pixel(0,fragParams.x, fragParams.y, math::vec4{1.f, 0, 1.f, 1.f});
                }
            }

            if (haveOutputs && depthMask)
            {
                fbo->put_depth_pixel<depth_type>(xi, yi, (depth_type)z);
            }
        }

        x += dx;
        y += dy;
    }
}



/*--------------------------------------
 * Wireframe Rasterization
--------------------------------------*/
template <typename depth_type>
void SL_FragmentProcessor::render_wireframe(const SL_Texture* depthBuffer) const noexcept
{
    const SL_FragmentBin* pBin         = mBins;
    SL_FragCoord*         outCoords    = mQueues;
    const int32_t         yOffset      = (int32_t)mThreadId;
    const int32_t         increment    = (int32_t)mNumProcessors;
    const int32_t         depthTesting = mShader->fragment_shader().depthTest == SL_DEPTH_TEST_ON;
    SL_ScanlineBounds     scanline;

    for (uint64_t binId = 0; binId < mNumBins; ++binId, ++pBin)
    {
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
            int32_t xMin;
            int32_t xMax;
            scanline.step(yf, xMin, xMax);
            int32_t x = xMin;

            const depth_type* const pDepth = depthBuffer->row_pointer<depth_type>(y);

            for (; x < xMax; x += xMax-xMin)
            {
                // calculate barycentric coordinates
                const float  xf = (float)x;
                math::vec4&& bc = math::fmadd(bcClipSpace[0], math::vec4{xf, xf, xf, 0.f}, bcY);
                const float  z  = math::dot(depth, bc);
                const float  d  = _sl_get_depth_texel<depth_type>(pDepth+x);

                #if SL_REVERSED_Z_RENDERING
                    const int_fast32_t&& depthTest = math::sign_mask(d-z) | depthTesting;
                #else
                    const int_fast32_t&& depthTest = math::sign_mask(z-d) | depthTesting;
                #endif

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



/*--------------------------------------
 * Triangle Rasterization, scalar
--------------------------------------*/
template <typename depth_type>
void SL_FragmentProcessor::render_triangle(const SL_Texture* depthBuffer) const noexcept
{
    const SL_FragmentBin* pBin         = mBins;
    SL_FragCoord*         outCoords    = mQueues;
    const int32_t         yOffset      = (int32_t)mThreadId;
    const int32_t         increment    = (int32_t)mNumProcessors;
    const int32_t         depthTesting = mShader->fragment_shader().depthTest == SL_DEPTH_TEST_OFF;
    SL_ScanlineBounds     scanline;

    for (uint64_t binId = 0; binId < mNumBins; ++binId, ++pBin)
    {
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
            const float yf = (float)y;
            const math::vec4&& bcY = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);

            // In this rasterizer, we're only rendering the absolute pixels
            // contained within the triangle edges. However this will serve as a
            // guard against any pixels we don't want to render.
            int32_t x;
            int32_t xMax;
            scanline.step(yf, x, xMax);

            if (x < xMax)
            {
                const depth_type* pDepth = depthBuffer->row_pointer<depth_type>(y) + x;

                do
                {
                    // calculate barycentric coordinates
                    const math::vec4&& xf{(float)x};
                    const float d = _sl_get_depth_texel<depth_type>(pDepth);
                    math::vec4&& bc = math::fmadd(bcClipSpace[0], xf, bcY);
                    const float z = math::dot(depth, bc);

                    #if SL_REVERSED_Z_RENDERING
                        const int_fast32_t&& depthTest = math::sign_mask(d - z) | depthTesting;
                    #else
                        const int_fast32_t&& depthTest = math::sign_mask(z-d) | depthTesting;
                    #endif

                    if (LS_LIKELY(depthTest))
                    {
                        // perspective correction
                        const float persp = math::dot(bc, homogenous);
                        bc *= homogenous;
                        outCoords->bc[numQueuedFrags] = bc * math::rcp(persp);
                        outCoords->coord[numQueuedFrags] = {(uint16_t)x, (uint16_t)y, z};

                        ++numQueuedFrags;

                        if (LS_UNLIKELY(numQueuedFrags == SL_SHADER_MAX_QUEUED_FRAGS))
                        {
                            numQueuedFrags = 0;
                            flush_fragments<depth_type>(pBin, SL_SHADER_MAX_QUEUED_FRAGS, outCoords);
                        }
                    }

                    ++x;
                    ++pDepth;
                }
                while (x < xMax);
            }
        }

        // cleanup remaining fragments
        if (LS_LIKELY(numQueuedFrags > 0))
        {
            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
        }
    }
}



/*-------------------------------------
 * Render a triangle using 4 elements at a time
-------------------------------------*/
#if !defined(LS_ARCH_X86)

template <typename depth_type>
void SL_FragmentProcessor::render_triangle_simd(const SL_Texture* depthBuffer) const noexcept
{
    const SL_FragmentBin* pBin         = mBins;
    SL_FragCoord*         outCoords    = mQueues;
    const int32_t         yOffset      = (int32_t)mThreadId;
    const int32_t         increment    = (int32_t)mNumProcessors;
    const int32_t         depthTesting = -(mShader->fragment_shader().depthTest == SL_DEPTH_TEST_OFF) & 0x0F;
    SL_ScanlineBounds     scanline;

    for (uint64_t binId = 0; binId < mNumBins; ++binId, ++pBin)
    {
        unsigned          numQueuedFrags = 0;
        const math::vec4* pPoints        = pBin->mScreenCoords;
        const math::vec4  depth          {pPoints[0][2], pPoints[1][2], pPoints[2][2], 0.f};
        const math::vec4  homogenous     {pPoints[0][3], pPoints[1][3], pPoints[2][3], 0.f};
        const int32_t     bboxMinY       = (int32_t)math::min(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     bboxMaxY       = (int32_t)math::max(pPoints[0][1], pPoints[1][1], pPoints[2][1]);
        const int32_t     scanLineOffset = bboxMinY + sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);
        const math::vec4* bcClipSpace    = pBin->mBarycentricCoords;

        scanline.init(pPoints[0], pPoints[1], pPoints[2]);

        int32_t y = scanLineOffset;
        if (LS_UNLIKELY(y >= bboxMaxY))
        {
            continue;
        }

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
                const depth_type* pDepth = depthBuffer->row_pointer<depth_type>((uintptr_t)y) + xMin;
                const math::vec4&& bcY = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);
                math::vec4i       x4     = math::vec4i{0, 1, 2, 3} + xMin;
                const math::vec4i xMax4  {xMax};

                do
                {
                    // calculate barycentric coordinates and perform a depth test
                    const int32_t      xBound = math::sign_mask(x4-xMax4);
                    const math::vec4&& d      = _sl_get_depth_texel4<depth_type>(pDepth);
                    math::mat4&&       bc     = math::outer((math::vec4)x4, bcClipSpace[0]) + bcY;
                    const math::vec4&& z      = depth * bc;

                    #if SL_REVERSED_Z_RENDERING
                        const int32_t depthTest = xBound & (math::sign_mask(d-z) | depthTesting);
                    #else
                        const int32_t depthTest = xBound & (math::sign_mask(z-d) | depthTesting);
                    #endif

                    if (LS_LIKELY(0 != depthTest))
                    {
                        const unsigned rasterCount       = math::popcnt_u32(depthTest & 0x0F);
                        const unsigned storeMask1        = math::popcnt_u32((unsigned)depthTest & 0x01u) + numQueuedFrags;
                        const unsigned storeMask2        = math::popcnt_u32((unsigned)depthTest & 0x03u) + numQueuedFrags;
                        const unsigned storeMask3        = math::popcnt_u32((unsigned)depthTest & 0x07u) + numQueuedFrags;

                        outCoords->coord[numQueuedFrags] = SL_FragCoordXYZ{(uint16_t)x4.v[0], (uint16_t)y, z.v[0]};
                        outCoords->coord[storeMask1]     = SL_FragCoordXYZ{(uint16_t)x4.v[1], (uint16_t)y, z.v[1]};
                        outCoords->coord[storeMask2]     = SL_FragCoordXYZ{(uint16_t)x4.v[2], (uint16_t)y, z.v[2]};
                        outCoords->coord[storeMask3]     = SL_FragCoordXYZ{(uint16_t)x4.v[3], (uint16_t)y, z.v[3]};

                        math::vec4 persp4 = homogenous * bc;
                        bc[0] = bc[0] * homogenous;
                        bc[1] = bc[1] * homogenous;
                        bc[2] = bc[2] * homogenous;
                        bc[3] = bc[3] * homogenous;

                        persp4 = math::rcp(persp4);
                        bc[0] = bc[0] * persp4[0];
                        bc[1] = bc[1] * persp4[1];
                        bc[2] = bc[2] * persp4[2];
                        bc[3] = bc[3] * persp4[3];

                        outCoords->bc[numQueuedFrags] = bc[0];
                        outCoords->bc[storeMask1]     = bc[1];
                        outCoords->bc[storeMask2]     = bc[2];
                        outCoords->bc[storeMask3]     = bc[3];

                        numQueuedFrags += rasterCount;
                        if (LS_UNLIKELY(numQueuedFrags > SL_SHADER_MAX_QUEUED_FRAGS - 4))
                        {
                            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
                            numQueuedFrags = 0;
                        }
                    }

                    pDepth += 4;
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



#else



template <typename depth_type>
void SL_FragmentProcessor::render_triangle_simd(const SL_Texture* depthBuffer) const noexcept
{
    const SL_FragmentBin* pBin         = mBins;
    SL_FragCoord*         outCoords    = mQueues;
    const int32_t         yOffset      = (int32_t)mThreadId;
    const int32_t         increment    = (int32_t)mNumProcessors;
    const __m128          depthTesting = _mm_castsi128_ps(_mm_set1_epi32(-(mShader->fragment_shader().depthTest == SL_DEPTH_TEST_OFF)));
    SL_ScanlineBounds     scanline;

    for (uint64_t binId = 0; binId < mNumBins; ++binId, ++pBin)
    {
        unsigned numQueuedFrags = 0;

        const __m128 points0 = _mm_load_ps(reinterpret_cast<const float*>(pBin->mScreenCoords+0));
        const __m128 points1 = _mm_load_ps(reinterpret_cast<const float*>(pBin->mScreenCoords+1));
        const __m128 points2 = _mm_load_ps(reinterpret_cast<const float*>(pBin->mScreenCoords+2));

        const __m128 d01 = _mm_unpackhi_ps(points0, points1);
        const ls::math::vec4 depth{_mm_insert_ps(d01, points2, 0xA8)};

        const __m128 h01 = _mm_insert_ps(_mm_permute_ps(points0, 0xFF), points1, 0xD0);
        const ls::math::vec4 homogenous{_mm_insert_ps(h01, points2, 0xE8)};

        const int32_t     bboxMinY       = _mm_extract_epi32(_mm_cvtps_epi32(_mm_min_ps(_mm_min_ps(points0, points1), points2)), 1);
        const int32_t     bboxMaxY       = _mm_extract_epi32(_mm_cvtps_epi32(_mm_max_ps(_mm_max_ps(points0, points1), points2)), 1);
        const int32_t     scanLineOffset = bboxMinY + sl_scanline_offset<int32_t>(increment, yOffset, bboxMinY);
        const math::vec4* bcClipSpace    = pBin->mBarycentricCoords;

        scanline.init(math::vec4{points0}, math::vec4{points1}, math::vec4{points2});

        int32_t y = scanLineOffset;
        if (LS_UNLIKELY(y >= bboxMaxY))
        {
            continue;
        }

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

            if (LS_LIKELY(xMin < xMax))
            {
                const depth_type*  pDepth = depthBuffer->row_pointer<depth_type>((uintptr_t)y) + xMin;
                const math::vec4&& bcY    = math::fmadd(bcClipSpace[1], math::vec4{yf}, bcClipSpace[2]);
                __m128i            x4     = _mm_add_epi32(_mm_set_epi32(3, 2, 1, 0), _mm_set1_epi32(xMin));
                const __m128i      xMax4  = _mm_set1_epi32(xMax);

                do
                {
                    // calculate barycentric coordinates and perform a depth test
                    const math::vec4&& d      = _sl_get_depth_texel4<depth_type>(pDepth);
                    const __m128       xBound = _mm_castsi128_ps(_mm_cmplt_epi32(x4, xMax4));
                    math::mat4&&       bc     = math::outer(math::vec4{_mm_cvtepi32_ps(x4)}, bcClipSpace[0]) + bcY;
                    const math::vec4&& z      = depth * bc;

                    #if SL_REVERSED_Z_RENDERING
                        const int32_t depthTest = _mm_movemask_ps(_mm_and_ps(xBound, _mm_or_ps(_mm_cmplt_ps(d.simd, z.simd), depthTesting)));
                    #else
                        const int32_t depthTest = _mm_movemask_ps(_mm_and_ps(xBound, _mm_or_ps(_mm_cmplt_ps(z.simd, d.simd), depthTesting)));
                    #endif

                    if (LS_UNLIKELY(0 != depthTest))
                    {
                        __m128 persp4 = (homogenous * bc).simd;

                        const unsigned storeMask1  = math::popcnt_u32((unsigned)depthTest & 0x01u) + numQueuedFrags;
                        const unsigned storeMask2  = math::popcnt_u32((unsigned)depthTest & 0x03u) + numQueuedFrags;
                        const unsigned storeMask3  = math::popcnt_u32((unsigned)depthTest & 0x07u) + numQueuedFrags;
                        const unsigned rasterCount = math::popcnt_u32(depthTest & 0x0F);

                        persp4 = _mm_rcp_ps(persp4);
                        
                        //const __m128 xy = _mm_castsi128_ps(_mm_or_si128(_mm_and_si128(x4, _mm_set1_epi32(0x0000FFFF)), _mm_slli_epi32(_mm_set1_epi32(y), 16)));
                        const __m128 xy = _mm_castsi128_ps(_mm_or_si128(x4, _mm_slli_epi32(_mm_set1_epi32(y), 16)));
                        const __m128 xyz0 = _mm_unpacklo_ps(xy, z.simd);
                        const __m128 xyz1 = _mm_unpackhi_ps(xy, z.simd);

                        {
                            const __m128 persp0 = _mm_broadcastss_ps(persp4);
                            const __m128 bc0 = _mm_mul_ps(bc[0].simd, homogenous.simd);
                            bc[0].simd = _mm_mul_ps(bc0, persp0);
                            _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + numQueuedFrags), bc[0].simd);
                            _mm_storel_pi(reinterpret_cast<__m64*>(outCoords->coord + numQueuedFrags), xyz0);
                        }
                        {
                            const __m128 persp1 = _mm_permute_ps(persp4, 0x55);
                            const __m128 bc1 = _mm_mul_ps(bc[1].simd, homogenous.simd);
                            bc[1].simd = _mm_mul_ps(bc1, persp1);
                            _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + storeMask1), bc[1].simd);
                            _mm_storeh_pi(reinterpret_cast<__m64*>(outCoords->coord + storeMask1), xyz0);
                        }
                        {
                            const __m128 persp2 = _mm_permute_ps(persp4, 0xAA);
                            const __m128 bc2 = _mm_mul_ps(bc[2].simd, homogenous.simd);
                            bc[2].simd = _mm_mul_ps(bc2, persp2);
                            _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + storeMask2), bc[2].simd);
                            _mm_storel_pi(reinterpret_cast<__m64*>(outCoords->coord + storeMask2), xyz1);
                        }
                        {
                            const __m128 persp3 = _mm_permute_ps(persp4, 0xFF);
                            const __m128 bc3 = _mm_mul_ps(bc[3].simd, homogenous.simd);
                            bc[3].simd = _mm_mul_ps(bc3, persp3);
                            _mm_store_ps(reinterpret_cast<float*>(outCoords->bc + storeMask3), bc[3].simd);
                            _mm_storeh_pi(reinterpret_cast<__m64*>(outCoords->coord + storeMask3), xyz1);
                        }

                        numQueuedFrags += rasterCount;
                        if (LS_UNLIKELY(numQueuedFrags > SL_SHADER_MAX_QUEUED_FRAGS - 4))
                        {
                            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
                            numQueuedFrags = 0;
                        }
                    }

                    x4 = _mm_add_epi32(x4, _mm_set1_epi32(4));
                    pDepth += 4;
                }
                while (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpgt_epi32(xMax4, x4))));
            }

            y += increment;
        }
        while (LS_UNLIKELY(y < bboxMaxY));

        if (LS_LIKELY(0 < numQueuedFrags))
        {
            flush_fragments<depth_type>(pBin, numQueuedFrags, outCoords);
        }
    }
}



#endif



/*--------------------------------------
 * Triangle Fragment Bin-Rasterization
--------------------------------------*/
template <typename depth_type>
void SL_FragmentProcessor::flush_fragments(
    const SL_FragmentBin* pBin,
    uint32_t              numQueuedFrags,
    const SL_FragCoord*   outCoords) const noexcept
{
    const SL_UniformBuffer* pUniforms   = mShader->mUniforms;
    const SL_FragmentShader fragShader  = mShader->mFragShader;
    SL_Texture*             pDepthBuf   = mFbo->get_depth_buffer();

    SL_FragmentParam fragParams;
    fragParams.pUniforms = pUniforms;

    if (LS_LIKELY(fragShader.blend == SL_BLEND_OFF))
    {
        for (uint32_t i = 0; i < numQueuedFrags; ++i)
        {
            const math::vec4& bc = outCoords->bc[i];
            fragParams.coord = outCoords->coord[i];

            interpolate_tri_varyings(bc.v, fragShader.numVaryings, pBin->mVaryings, fragParams.pVaryings);

            uint_fast32_t haveOutputs = fragShader.shader(fragParams);

            // branchless select
            switch (-haveOutputs & fragShader.numOutputs)
            {
                case 4: mFbo->put_pixel(3, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3]);
                case 3: mFbo->put_pixel(2, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2]);
                case 2: mFbo->put_pixel(1, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1]);
                case 1: mFbo->put_pixel(0, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0]);

                    if (fragShader.depthMask == SL_DEPTH_MASK_ON)
                    {
                        pDepthBuf->raw_texel<depth_type>(fragParams.coord.x, fragParams.coord.y) = (depth_type)fragParams.coord.depth;
                    }
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < numQueuedFrags; ++i)
        {
            const math::vec4& bc = outCoords->bc[i];
            fragParams.coord = outCoords->coord[i];

            interpolate_tri_varyings(bc.v, fragShader.numVaryings, pBin->mVaryings, fragParams.pVaryings);

            uint_fast32_t haveOutputs = fragShader.shader(fragParams);

            // branchless select
            switch (-haveOutputs & fragShader.numOutputs)
            {
                case 4: mFbo->put_alpha_pixel(3, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[3], fragShader.blend);
                case 3: mFbo->put_alpha_pixel(2, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[2], fragShader.blend);
                case 2: mFbo->put_alpha_pixel(1, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[1], fragShader.blend);
                case 1: mFbo->put_alpha_pixel(0, fragParams.coord.x, fragParams.coord.y, fragParams.pOutputs[0], fragShader.blend);

                    if (fragShader.depthMask == SL_DEPTH_MASK_ON)
                    {
                        pDepthBuf->raw_texel<depth_type>(fragParams.coord.x, fragParams.coord.y) = (depth_type)fragParams.coord.depth;
                    }
            }
        }
    }
}



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SL_FragmentProcessor::execute() noexcept
{
    const uint16_t depthBpp = mFbo->get_depth_buffer()->bpp();

    switch(mMode)
    {
        case RENDER_MODE_POINTS:
        case RENDER_MODE_INDEXED_POINTS:
        {
            // divide the screen into equal parts which can then be rendered by all
            // available fragment threads.
            const int32_t w = mFbo->width();
            const int32_t h = mFbo->height();
            const math::vec4_t<int32_t> dimens = sl_subdivide_region<int32_t>(w, h, mNumProcessors, mThreadId);

            if (depthBpp == sizeof(math::half))
            {
                for (uint64_t binId = 0; binId < mNumBins; ++binId)
                {
                    render_point<math::half>(binId, mFbo, dimens);
                }
            }
            else if (depthBpp == sizeof(float))
            {
                for (uint64_t binId = 0; binId < mNumBins; ++binId)
                {
                    render_point<float>(binId, mFbo, dimens);
                }
            }
            else if (depthBpp == sizeof(double))
            {
                for (uint64_t binId = 0; binId < mNumBins; ++binId)
                {
                    render_point<double>(binId, mFbo, dimens);
                }
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
            const math::vec4_t<int32_t> dimens = sl_subdivide_region<int32_t>(w, h, mNumProcessors, mThreadId);

            if (depthBpp == sizeof(math::half))
            {
                for (uint64_t binId = 0; binId < mNumBins; ++binId)
                {
                    render_line<math::half>(binId, mFbo, dimens);
                }
            }
            else if (depthBpp == sizeof(float))
            {
                for (uint64_t binId = 0; binId < mNumBins; ++binId)
                {
                    render_line<float>(binId, mFbo, dimens);
                }
            }
            else if (depthBpp == sizeof(double))
            {
                for (uint64_t binId = 0; binId < mNumBins; ++binId)
                {
                    render_line<double>(binId, mFbo, dimens);
                }
            }
            break;
        }

        case RENDER_MODE_TRI_WIRE:
        case RENDER_MODE_INDEXED_TRI_WIRE:
            if (depthBpp == sizeof(math::half))
            {
                render_wireframe<math::half>(mFbo->get_depth_buffer());
            }
            else if (depthBpp == sizeof(float))
            {
                render_wireframe<float>(mFbo->get_depth_buffer());
            }
            else if (depthBpp == sizeof(double))
            {
                render_wireframe<double>(mFbo->get_depth_buffer());
            }
            break;

        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
            // Triangles assign scan-lines per thread for rasterization.
            // There's No need to subdivide the output framebuffer
            if (depthBpp == sizeof(math::half))
            {
                //render_triangle<math::half>(mFbo->get_depth_buffer());
                render_triangle_simd<math::half>(mFbo->get_depth_buffer());
            }
            else if (depthBpp == sizeof(float))
            {
                //render_triangle<float>(mFbo->get_depth_buffer());
                render_triangle_simd<float>(mFbo->get_depth_buffer());
            }
            else if (depthBpp == sizeof(double))
            {
                render_triangle<double>(mFbo->get_depth_buffer());
                //render_triangle_simd<double>(mFbo->get_depth_buffer());
            }
            break;

        default:
            LS_UNREACHABLE();
    }
}

template void SL_FragmentProcessor::render_point<ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
template void SL_FragmentProcessor::render_point<float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
template void SL_FragmentProcessor::render_point<double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;

template void SL_FragmentProcessor::render_line<ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
template void SL_FragmentProcessor::render_line<float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;
template void SL_FragmentProcessor::render_line<double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>) noexcept;

template void SL_FragmentProcessor::render_wireframe<ls::math::half>(const SL_Texture*) const noexcept;
template void SL_FragmentProcessor::render_wireframe<float>(const SL_Texture*) const noexcept;
template void SL_FragmentProcessor::render_wireframe<double>(const SL_Texture*) const noexcept;

template void SL_FragmentProcessor::render_triangle<ls::math::half>(const SL_Texture*) const noexcept;
template void SL_FragmentProcessor::render_triangle<float>(const SL_Texture*) const noexcept;
template void SL_FragmentProcessor::render_triangle<double>(const SL_Texture*) const noexcept;

template void SL_FragmentProcessor::render_triangle_simd<ls::math::half>(const SL_Texture*) const noexcept;
template void SL_FragmentProcessor::render_triangle_simd<float>(const SL_Texture*) const noexcept;
template void SL_FragmentProcessor::render_triangle_simd<double>(const SL_Texture*) const noexcept;

template void SL_FragmentProcessor::flush_fragments<ls::math::half>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
template void SL_FragmentProcessor::flush_fragments<float>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
template void SL_FragmentProcessor::flush_fragments<double>(const SL_FragmentBin*, uint32_t, const SL_FragCoord*) const noexcept;
