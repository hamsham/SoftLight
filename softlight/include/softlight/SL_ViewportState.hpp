
#ifndef SL_RASTER_STATE_HPP
#define SL_RASTER_STATE_HPP

#include <cstdint>

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec4.h"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace math
{
    template <typename num_type>
    struct mat4_t;
}
}



/*-----------------------------------------------------------------------------
 * Rasterization State Storage
 *
 * This class should be lightweight so any overhead of copying shouldn't have
 * more overhead than assigning an __m128 or float32x4_t type. The reason is
 * the SL_ViewportState is copied into the software rasterizer, which should be
 * as freakishly fast as possible.
-----------------------------------------------------------------------------*/
class SL_ViewportState
{
  public:
    ~SL_ViewportState() noexcept = default;

    constexpr SL_ViewportState() noexcept;

    constexpr SL_ViewportState(const SL_ViewportState& rs) noexcept;

    constexpr SL_ViewportState(SL_ViewportState&& rs) noexcept;

    SL_ViewportState& operator=(const SL_ViewportState& rs) noexcept;

    SL_ViewportState& operator=(SL_ViewportState&& rs) noexcept;

    void reset() noexcept;

    void viewport(int32_t x, int32_t y, uint16_t w, uint16_t h) noexcept;

    constexpr ls::math::vec4_t<int32_t> viewport() const noexcept;

    ls::math::vec4 viewport_rect(float fboW, float fboH) const noexcept;

    ls::math::vec4_t<int32_t> viewport_rect(int32_t fboX, int32_t fboY, int32_t fboW, int32_t fboH) const noexcept;

    void scissor(int32_t x, int32_t y, uint16_t w, uint16_t h) noexcept;

    constexpr ls::math::vec4_t<int32_t> scissor() const noexcept;

    ls::math::mat4_t<float> scissor_matrix(const float fboW, const float fboH) const noexcept;

  private:
    ls::math::vec4_t<int32_t> mViewport;

    ls::math::vec4_t<int32_t> mScissor;
};



/*-----------------------------------------------------------------------------
 * Rasterization State Storage (Implementations)
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Constructor
-------------------------------------*/
constexpr SL_ViewportState::SL_ViewportState() noexcept :
    mViewport{0, 0, 65535, 65535},
    mScissor{0, 0, 65535, 65535}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
constexpr SL_ViewportState::SL_ViewportState(const SL_ViewportState& rs) noexcept :
    mViewport{rs.mViewport},
    mScissor{rs.mScissor}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
constexpr SL_ViewportState::SL_ViewportState(SL_ViewportState&& rs) noexcept :
    mViewport{rs.mViewport},
    mScissor{rs.mScissor}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
inline SL_ViewportState& SL_ViewportState::operator=(const SL_ViewportState& rs) noexcept
{
    mViewport = rs.mViewport;
    mScissor = rs.mScissor;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
inline SL_ViewportState& SL_ViewportState::operator=(SL_ViewportState&& rs) noexcept
{
    mViewport = rs.mViewport;
    mScissor = rs.mScissor;

    rs.reset();

    return *this;
}



/*-------------------------------------
 * Reset Internal State
-------------------------------------*/
inline void SL_ViewportState::reset() noexcept
{
    mViewport = ls::math::vec4_t<int32_t>{0, 0, 65535, 65535};
    mScissor = ls::math::vec4_t<int32_t>{0, 0, 65535, 65535};
}



/*-------------------------------------
 * viewport setter
-------------------------------------*/
inline void SL_ViewportState::viewport(int32_t x, int32_t y, uint16_t w, uint16_t h) noexcept
{
    mViewport = ls::math::vec4_t<int32_t>{
        ls::math::clamp<int32_t>(x, -65536, 65535),
        ls::math::clamp<int32_t>(y, -65536, 65536),
        ls::math::min((int32_t)w, 65535-x),
        ls::math::min((int32_t)h, 65535-y)
    };
}



/*-------------------------------------
 * viewport getter
-------------------------------------*/
constexpr ls::math::vec4_t<int32_t> SL_ViewportState::viewport() const noexcept
{
    return mViewport;
}



/*-------------------------------------
 * scissor setter
-------------------------------------*/
inline void SL_ViewportState::scissor(int32_t x, int32_t y, uint16_t w, uint16_t h) noexcept
{
    mScissor = ls::math::vec4_t<int32_t>{
        ls::math::clamp<int32_t>(x, -65536, 65535),
        ls::math::clamp<int32_t>(y, -65536, 65536),
        ls::math::min((int32_t)w, 65535-x),
        ls::math::min((int32_t)h, 65535-y)
    };
}



/*-------------------------------------
 * scissor getter
-------------------------------------*/
constexpr ls::math::vec4_t<int32_t> SL_ViewportState::scissor() const noexcept
{
    return mScissor;
}



#endif /* SL_RASTER_STATE_HPP */
