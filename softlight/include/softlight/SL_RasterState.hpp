
#ifndef SL_RASTER_STATE_HPP
#define SL_RASTER_STATE_HPP

#include <cstdint>

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec4.h"



/*-----------------------------------------------------------------------------
 * Rasterization State
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Triangle Cull Mode
-------------------------------------*/
enum SL_CullMode : uint8_t
{
    SL_CULL_BACK_FACE,
    SL_CULL_FRONT_FACE,
    SL_CULL_OFF
}; // 3 states = 2 bits



/*-------------------------------------
 * Depth Test Configuration
-------------------------------------*/
enum SL_DepthTest : uint8_t
{
    SL_DEPTH_TEST_OFF,

    SL_DEPTH_TEST_LESS_THAN,
    SL_DEPTH_TEST_LESS_EQUAL,
    SL_DEPTH_TEST_GREATER_THAN,
    SL_DEPTH_TEST_GREATER_EQUAL,
    SL_DEPTH_TEST_EQUAL,
    SL_DEPTH_TEST_NOT_EQUAL,
}; // 7 states = 3 bits



/*-------------------------------------
 * Depth-Write Configuration
-------------------------------------*/
enum SL_DepthMask : uint8_t
{
    SL_DEPTH_MASK_OFF,
    SL_DEPTH_MASK_ON
}; // 2 states = 1 bit



/*-------------------------------------
 * Fragment Blending
-------------------------------------*/
enum SL_BlendMode : uint8_t
{
    SL_BLEND_OFF,
    SL_BLEND_ALPHA,
    SL_BLEND_PREMULTIPLED_ALPHA,
    SL_BLEND_ADDITIVE,
    SL_BLEND_SCREEN,
}; // 5 states = 3 bits



/*-----------------------------------------------------------------------------
 * Rasterization State Storage
 *
 * This class should be lightweight so any overhead of copying shouldn't have
 * more overhead than assigning an __m128 or float32x4_t type. The reason is
 * the SL_RasterState is copied into the software rasterizer, which should be
 * as freakishly fast as possible.
-----------------------------------------------------------------------------*/
class SL_RasterState
{
  private:
    enum SL_RasterStateShifts : uint32_t
    {
        SL_CULL_MODE_SHIFTS  = 0,
        SL_DEPTH_TEST_SHIFTS = 2,
        SL_DEPTH_MASK_SHIFTS = 5,
        SL_BLEND_MODE_SHIFTS = 6
    };

    enum SL_RasterStateMask : uint32_t
    {
        SL_CULL_MODE_MASK  = 0x00000003,
        SL_DEPTH_TEST_MASK = 0x0000001C,
        SL_DEPTH_MASK_MASK = 0x00000020,
        SL_BLEND_MODE_MASK = 0x000001C0
    };

    static constexpr uint32_t cull_mode_to_bits(SL_CullMode cm) noexcept;

    static constexpr SL_CullMode cull_mode_from_bits(uint32_t bits) noexcept;

    static constexpr uint32_t depth_test_to_bits(SL_DepthTest dt) noexcept;

    static constexpr SL_DepthTest depth_test_from_bits(uint32_t bits) noexcept;

    static constexpr uint32_t depth_mask_to_bits(SL_DepthMask dm) noexcept;

    static constexpr SL_DepthMask depth_mask_from_bits(uint32_t bits) noexcept;

    static constexpr uint32_t blend_mode_to_bits(SL_BlendMode bm) noexcept;

    static constexpr SL_BlendMode blend_mode_from_bits(uint32_t bits) noexcept;

  public:
    ~SL_RasterState() noexcept = default;

    constexpr SL_RasterState() noexcept;

    constexpr SL_RasterState(const SL_RasterState& rs) noexcept;

    constexpr SL_RasterState(SL_RasterState&& rs) noexcept;

    SL_RasterState& operator=(const SL_RasterState& rs) noexcept;

    SL_RasterState& operator=(SL_RasterState&& rs) noexcept;

    void reset() noexcept;

    constexpr uint32_t bits() const noexcept;

    void cull_mode(SL_CullMode cm) noexcept;

    constexpr SL_CullMode cull_mode() const noexcept;

    void depth_test(SL_DepthTest dt) noexcept;

    constexpr SL_DepthTest depth_test() const noexcept;

    void depth_mask(SL_DepthMask dm) noexcept;

    constexpr SL_DepthMask depth_mask() const noexcept;

    void blend_mode(SL_BlendMode bm) noexcept;

    constexpr SL_BlendMode blend_mode() const noexcept;

    void viewport(int32_t x, int32_t y, uint16_t w, uint16_t h) noexcept;

    constexpr ls::math::vec4_t<int32_t> viewport() const noexcept;

    void scissor(int32_t x, int32_t y, uint16_t w, uint16_t h) noexcept;

    constexpr ls::math::vec4_t<int32_t> scissor() const noexcept;

  private:
    uint32_t mStates;

    ls::math::vec4_t<int32_t> mViewport;

    ls::math::vec4_t<int32_t> mScissor;
};



/*-----------------------------------------------------------------------------
 * Rasterization State Storage (Implementations)
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Cull Mode to bits
-------------------------------------*/
constexpr uint32_t SL_RasterState::cull_mode_to_bits(SL_CullMode cm) noexcept
{
    return static_cast<uint32_t>(cm << SL_RasterState::SL_CULL_MODE_SHIFTS);
}



/*-------------------------------------
 * Cull Mode from bits
-------------------------------------*/
constexpr SL_CullMode SL_RasterState::cull_mode_from_bits(uint32_t bits) noexcept
{
    return static_cast<SL_CullMode>((bits & SL_RasterState::SL_CULL_MODE_MASK) >> SL_RasterState::SL_CULL_MODE_SHIFTS);
}



/*-------------------------------------
 * Depth Test to bits
-------------------------------------*/
constexpr uint32_t SL_RasterState::depth_test_to_bits(SL_DepthTest dt) noexcept
{
    return static_cast<uint32_t>(dt << SL_RasterState::SL_DEPTH_TEST_SHIFTS);
}



/*-------------------------------------
 * Depth Test from bits
-------------------------------------*/
constexpr SL_DepthTest SL_RasterState::depth_test_from_bits(uint32_t bits) noexcept
{
    return static_cast<SL_DepthTest>((bits & SL_RasterState::SL_DEPTH_TEST_MASK) >> SL_RasterState::SL_DEPTH_TEST_SHIFTS);
}



/*-------------------------------------
 * Depth Mask to bits
-------------------------------------*/
constexpr uint32_t SL_RasterState::depth_mask_to_bits(SL_DepthMask dm) noexcept
{
    return static_cast<uint32_t>(dm << SL_RasterState::SL_DEPTH_MASK_SHIFTS);
}



/*-------------------------------------
 * Depth Mask from bits
-------------------------------------*/
constexpr SL_DepthMask SL_RasterState::depth_mask_from_bits(uint32_t bits) noexcept
{
    return static_cast<SL_DepthMask>((bits & SL_RasterState::SL_DEPTH_MASK_MASK) >> SL_RasterState::SL_DEPTH_MASK_SHIFTS);
}



/*-------------------------------------
 * Blend mode to bits
-------------------------------------*/
constexpr uint32_t SL_RasterState::blend_mode_to_bits(SL_BlendMode bm) noexcept
{
    return static_cast<uint32_t>(bm << SL_RasterState::SL_BLEND_MODE_SHIFTS);
}



/*-------------------------------------
 * blend mode from bits
-------------------------------------*/
constexpr SL_BlendMode SL_RasterState::blend_mode_from_bits(uint32_t bits) noexcept
{
    return static_cast<SL_BlendMode>((bits & SL_RasterState::SL_BLEND_MODE_MASK) >> SL_RasterState::SL_BLEND_MODE_SHIFTS);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
constexpr SL_RasterState::SL_RasterState() noexcept :
    mStates{(uint32_t)(
        SL_RasterState::cull_mode_to_bits(SL_CullMode::SL_CULL_BACK_FACE) |
        SL_RasterState::depth_test_to_bits(SL_DepthTest::SL_DEPTH_TEST_LESS_THAN) |
        SL_RasterState::depth_mask_to_bits(SL_DepthMask::SL_DEPTH_MASK_ON) |
        SL_RasterState::blend_mode_to_bits(SL_BlendMode::SL_BLEND_OFF)
    )},
    mViewport{0, 0, 65535, 65535},
    mScissor{0, 0, 65535, 65535}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
constexpr SL_RasterState::SL_RasterState(const SL_RasterState& rs) noexcept :
    mStates{rs.mStates},
    mViewport{rs.mViewport},
    mScissor{rs.mScissor}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
constexpr SL_RasterState::SL_RasterState(SL_RasterState&& rs) noexcept :
    mStates{rs.mStates},
    mViewport{rs.mViewport},
    mScissor{rs.mScissor}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
inline SL_RasterState& SL_RasterState::operator=(const SL_RasterState& rs) noexcept
{
    mStates = rs.mStates;
    mViewport = rs.mViewport;
    mScissor = rs.mScissor;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
inline SL_RasterState& SL_RasterState::operator=(SL_RasterState&& rs) noexcept
{
    mStates = rs.mStates;
    mViewport = rs.mViewport;
    mScissor = rs.mScissor;
    rs.reset();

    return *this;
}



/*-------------------------------------
 * Reset Internal State
-------------------------------------*/
inline void SL_RasterState::reset() noexcept
{
    mStates = (uint32_t)(0
        | SL_RasterState::cull_mode_to_bits(SL_CullMode::SL_CULL_BACK_FACE)
        | SL_RasterState::depth_test_to_bits(SL_DepthTest::SL_DEPTH_TEST_LESS_THAN)
        | SL_RasterState::depth_mask_to_bits(SL_DepthMask::SL_DEPTH_MASK_ON)
        | SL_RasterState::blend_mode_to_bits(SL_BlendMode::SL_BLEND_OFF)
        | 0);
    mViewport = ls::math::vec4_t<int32_t>{0, 0, 65535, 65535};
    mScissor = ls::math::vec4_t<int32_t>{0, 0, 65535, 65535};
}



/*-------------------------------------
 * Get the internal state
-------------------------------------*/
constexpr uint32_t SL_RasterState::bits() const noexcept
{
    return mStates;
}



/*-------------------------------------
 * cull mode setter
-------------------------------------*/
inline void SL_RasterState::cull_mode(SL_CullMode cm) noexcept
{
    mStates = (mStates & ~SL_CULL_MODE_MASK) | cull_mode_to_bits(cm);
}



/*-------------------------------------
 * cull mode getter
-------------------------------------*/
constexpr SL_CullMode SL_RasterState::cull_mode() const noexcept
{
    return cull_mode_from_bits(mStates);
}



/*-------------------------------------
 * depth test setter
-------------------------------------*/
inline void SL_RasterState::depth_test(SL_DepthTest dt) noexcept
{
    mStates = (mStates & ~SL_DEPTH_TEST_MASK) | depth_test_to_bits(dt);
}



/*-------------------------------------
 * depth test getter
-------------------------------------*/
constexpr SL_DepthTest SL_RasterState::depth_test() const noexcept
{
    return depth_test_from_bits(mStates);
}



/*-------------------------------------
 * depth mask setter
-------------------------------------*/
inline void SL_RasterState::depth_mask(SL_DepthMask dm) noexcept
{
    mStates = (mStates & ~SL_DEPTH_MASK_MASK) | depth_mask_to_bits(dm);
}



/*-------------------------------------
 * depth mask getter
-------------------------------------*/
constexpr SL_DepthMask SL_RasterState::depth_mask() const noexcept
{
    return depth_mask_from_bits(mStates);
}



/*-------------------------------------
 * blend mode setter
-------------------------------------*/
inline void SL_RasterState::blend_mode(SL_BlendMode bm) noexcept
{
    mStates = (mStates & ~SL_BLEND_MODE_MASK) | blend_mode_to_bits(bm);
}



/*-------------------------------------
 * blend mode getter
-------------------------------------*/
constexpr SL_BlendMode SL_RasterState::blend_mode() const noexcept
{
    return blend_mode_from_bits(mStates);
}



/*-------------------------------------
 * viewport setter
-------------------------------------*/
inline void SL_RasterState::viewport(int32_t x, int32_t y, uint16_t w, uint16_t h) noexcept
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
constexpr ls::math::vec4_t<int32_t> SL_RasterState::viewport() const noexcept
{
    return mViewport;
}



/*-------------------------------------
 * scissor setter
-------------------------------------*/
inline void SL_RasterState::scissor(int32_t x, int32_t y, uint16_t w, uint16_t h) noexcept
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
constexpr ls::math::vec4_t<int32_t> SL_RasterState::scissor() const noexcept
{
    return mScissor;
}



#endif /* SL_RASTER_STATE_HPP */
