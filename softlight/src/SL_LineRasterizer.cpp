
#include "lightsky/setup/Api.h" // LS_IMPERATIVE

#include "lightsky/math/bits.h"
#include "lightsky/math/fixed.h"
#include "lightsky/math/half.h"
#include "lightsky/math/vec_utils.h"

#include "softlight/SL_LineRasterizer.hpp"
#include "softlight/SL_Framebuffer.hpp" // SL_Framebuffer
#include "softlight/SL_ViewportState.hpp"
#include "softlight/SL_Shader.hpp" // SL_FragmentShader
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
    constexpr uint32_t i2 = SL_SHADER_MAX_VARYING_VECTORS;

    for (uint32_t i = numVaryings; i--;)
    {
        const math::vec4& v0 = inVaryings[i];
        const math::vec4& v1 = inVaryings[i+i2];
        outVaryings[i] = math::mix(v0, v1, percent);
    }
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_LineRasterizer Class
-----------------------------------------------------------------------------*/
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
template <class DepthCmpFunc, typename depth_type>
void SL_LineRasterizer::render_line(const uint32_t binId, SL_Framebuffer* fbo, const math::vec4_t<int32_t>& dimens) noexcept
{
    const math::vec4& screenCoord0  = mBins[binId].mScreenCoords[0];
    const math::vec4& screenCoord1  = mBins[binId].mScreenCoords[1];
    float             z0            = screenCoord0[2];
    float             z1            = screenCoord1[2];
    const math::vec4* inVaryings    = mBins[binId].mVaryings;
    math::vec2        clipCoords[2] = {math::vec2_cast(screenCoord0), math::vec2_cast(screenCoord1)};

    if (!sl_clip_liang_barsky(clipCoords, dimens))
    {
        return;
    }

    constexpr DepthCmpFunc  depthCmp    = {};
    const SL_PipelineState  pipeline    = mShader->pipelineState;
    const SL_BlendMode      blendMode   = pipeline.blend_mode();
    const SL_FboOutputMask  fboOutMask  = sl_calc_fbo_out_mask((unsigned)pipeline.num_render_targets(), (blendMode != SL_BLEND_OFF));
    const uint32_t          numVaryings = (unsigned)pipeline.num_varyings();
    const bool              depthMask   = pipeline.depth_mask() == SL_DEPTH_MASK_ON;
    const auto              shader      = mShader->pFragShader;
    const SL_UniformBuffer* pUniforms   = mShader->pUniforms;

    const SL_Texture*  depthBuf = fbo->get_depth_buffer();
    const float        dist     = math::inversesqrt(math::length_squared(math::vec2_cast(screenCoord1)-math::vec2_cast(screenCoord0)));
    const math::vec4&& p0       = math::vec4_cast(math::vec2_cast(screenCoord0), 0.f, 0.f);
    const math::vec4&& p1       = math::vec4_cast(clipCoords[0], 0.f, 0.f);
    const math::vec4&& p2       = math::vec4_cast(clipCoords[1], 0.f, 0.f);
    math::vec4         p        = p1;
    math::vec4&&       d        = p2 - p1;
    const float        step     = math::abs(math::max(d[0], d[1]));
    const int32_t      istep    = (int32_t)step;

    d /= step;

    SL_FragmentParam fragParams;
    fragParams.pUniforms = pUniforms;

    for (int32_t i = 0; i <= istep; ++i, p += d)
    {
        const math::vec4_t<uint16_t>&& pi = (math::vec4_t<uint16_t>)math::max(math::vec4{0.f}, p);
        const float currLen = math::length(p - p0);
        const float interp  = currLen * dist;
        const float z       = math::mix(z0, z1, interp);

        if (!depthCmp(z, (float)depthBuf->texel<depth_type>(pi[0], pi[1])))
        {
            continue;
        }

        interpolate_line_varyings(interp, numVaryings, inVaryings, fragParams.pVaryings);

        fragParams.coord.x     = pi[0];
        fragParams.coord.y     = pi[1];
        fragParams.coord.depth = z;
        const bool haveOutputs = shader(fragParams);

        if (LS_LIKELY(haveOutputs))
        {
            mFbo->put_pixel(fboOutMask, blendMode, fragParams);

            if (depthMask)
            {
                fbo->put_depth_pixel<depth_type>(fragParams.coord.x, fragParams.coord.y, (depth_type)fragParams.coord.depth);
            }
        }
    }
}



template void SL_LineRasterizer::render_line<SL_DepthFuncLT, ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLT, float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLT, double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncLE, ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLE, float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncLE, double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncGT, ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGT, float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGT, double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncGE, ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGE, float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncGE, double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncEQ, double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncNE, ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncNE, float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncNE, double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;

template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, ls::math::half>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, float>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;
template void SL_LineRasterizer::render_line<SL_DepthFuncOFF, double>(const uint32_t, SL_Framebuffer* const, const ls::math::vec4_t<int32_t>&) noexcept;



/*-------------------------------------
 * Dispatch the fragment processor with the correct depth-comparison function
-------------------------------------*/
template <class DepthCmpFunc>
void SL_LineRasterizer::dispatch_bins() noexcept
{
    // divide the screen into equal parts which can then be rendered by all
    // available fragment threads.
    const math::vec4_t<int32_t>&& clipRect = mViewState->viewport_rect(0, 0, mFbo->width(), mFbo->height());
    const math::vec4_t<int32_t>&& dimens = sl_subdivide_region<int32_t>(clipRect, mNumProcessors, mThreadId);

    const uint16_t depthBpp = mFbo->get_depth_buffer()->bpp();

    if (depthBpp == sizeof(math::half))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, math::half>(binId, mFbo, dimens);
        }
    }
    else if (depthBpp == sizeof(float))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, float>(binId, mFbo, dimens);
        }
    }
    else if (depthBpp == sizeof(double))
    {
        for (uint64_t binId = 0; binId < mNumBins; ++binId)
        {
            render_line<DepthCmpFunc, double>(binId, mFbo, dimens);
        }
    }
}



template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncLT>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncLE>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncGT>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncGE>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncEQ>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncNE>() noexcept;
template void SL_LineRasterizer::dispatch_bins<SL_DepthFuncOFF>() noexcept;



/*-------------------------------------
 * Run the fragment processor
-------------------------------------*/
void SL_LineRasterizer::execute() noexcept
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
