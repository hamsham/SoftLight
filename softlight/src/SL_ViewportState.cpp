
#include "lightsky/math/mat4.h"
#include "lightsky/math/vec_utils.h"

#include "softlight/SL_ViewportState.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions
-----------------------------------------------------------------------------*/
namespace math = ls::math;

namespace
{

inline LS_INLINE math::mat4 sl_scissor(float x, float y, float w, float h) noexcept
{
    if (x < 0.f)
    {
        w += x;
        x = 0.f;
    }

    if (y < 0.f)
    {
        h += y;
        y = 0.f;
    }

    w = ls::math::min(1.f - x, w);
    h = ls::math::min(1.f - y, h);

    const float nm00 = math::rcp(w);
    const float nm11 = math::rcp(h);

    const float nm30 = nm00 - 1.f;
    const float nm31 = nm11 - 1.f;

    const float m30 = math::fmadd(x, (-2.f * nm00), nm30);
    const float m31 = math::fmadd(y, (-2.f * nm11), nm31);

    return math::mat4{
        nm00, 0.f,  0.f, 0.f,
        0.f,  nm11, 0.f, 0.f,
        0.f,  0.f,  1.f, 0.f,
        m30,  m31,  0.f, 1.f
    };
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * Rasterization State
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Viewport Rectangle
-------------------------------------*/
math::vec4 SL_ViewportState::viewport_rect(float fboW, float fboH) const noexcept
{
    // the viewport rectangle is both bounded by, and divided by the
    // framebuffer's dimensions.
    // x = math::clamp(x, 0.f, fboW);
    // w = min(w, fboW-x);
    const math::vec4&& scissorRect = (math::vec4)mScissor;
    math::vec4 viewportDims = (math::vec4)mViewport;

    viewportDims[0] = math::clamp(viewportDims[0]+scissorRect[0], 0.f, fboW);
    viewportDims[1] = math::clamp(viewportDims[1]+scissorRect[1], 0.f, fboH);
    //viewportDims[2] = math::min(viewportDims[2], (scissorRect[0]+scissorRect[2])-viewportDims[0], (fboX+fboW) - viewportDims[0]);
    //viewportDims[3] = math::min(viewportDims[3], (scissorRect[1]+scissorRect[3])-viewportDims[1], (fboY+fboH) - viewportDims[1]);
    viewportDims[2] = math::min(viewportDims[2], (scissorRect[0]+scissorRect[2])-viewportDims[0], fboW - viewportDims[0]);
    viewportDims[3] = math::min(viewportDims[3], (scissorRect[1]+scissorRect[3])-viewportDims[1], fboH - viewportDims[1]);

    return viewportDims;
}



/*-------------------------------------
 * Viewport Rectangle
-------------------------------------*/
math::vec4_t<int32_t> SL_ViewportState::viewport_rect(int32_t fboX, int32_t fboY, int32_t fboW, int32_t fboH) const noexcept
{
    const math::vec4_t<int32_t> scissorRect = mScissor;
    math::vec4_t<int32_t> viewportDims = mViewport;

    viewportDims[0] = math::clamp<int32_t>(viewportDims[0]+scissorRect[0], fboX, fboW);
    viewportDims[1] = math::clamp<int32_t>(viewportDims[1]+scissorRect[1], fboY, fboH);
    viewportDims[2] = math::min<int32_t>(viewportDims[2], (scissorRect[0]+scissorRect[2])-viewportDims[0], fboW - viewportDims[0]);
    viewportDims[3] = math::min<int32_t>(viewportDims[3], (scissorRect[1]+scissorRect[3])-viewportDims[1], fboH - viewportDims[1]);

    return viewportDims;
}



/*-------------------------------------
 * Scissor Matrix
-------------------------------------*/
math::mat4 SL_ViewportState::scissor_matrix(const float fboW, const float fboH) const noexcept
{
    const math::vec4 fboInv = math::rcp(math::vec4{fboW, fboH, fboW, fboH});
    const math::vec4&& scissorRect = (math::vec4)mScissor;
    const math::vec4&& scissorDims = scissorRect * fboInv;
    return sl_scissor(scissorDims[0], scissorDims[1], scissorDims[2], scissorDims[3]);
}