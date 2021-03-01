
#ifndef SL_RASTER_STATE_HPP
#define SL_RASTER_STATE_HPP

#include <cstdint>

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
    SL_DEPTH_TEST_ON,

    //SL_DEPTH_TEST_LESS_THAN,
    //SL_DEPTH_TEST_LESS_EQUAL,
    //SL_DEPTH_TEST_GREATER_THAN,
    //SL_DEPTH_TEST_GREATER_EQUAL,
    //SL_DEPTH_TEST_EQUAL,
    //SL_DEPTH_TEST_NOT_EQUAL,
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
-----------------------------------------------------------------------------*/
class SL_RasterState
{
  public:
    typedef uint16_t value_type;

  private:
    enum SL_RasterStateShifts : SL_RasterState::value_type
    {
        SL_CULL_MODE_SHIFTS = 0,
        SL_DEPTH_TEST_SHIFTS = 2,
        SL_DEPTH_MASK_SHIFTS = 5,
        SL_BLEND_MODE_SHIFTS = 6
    };

    enum SL_RasterStateMask : SL_RasterState::value_type
    {
        SL_CULL_MODE_MASK  = 0x0003,
        SL_DEPTH_TEST_MASK = 0x001C,
        SL_DEPTH_MASK_MASK = 0x0020,
        SL_BLEND_MODE_MASK = 0x01C0
    };

    static constexpr value_type cull_mode_to_bits(SL_CullMode cm) noexcept;

    static constexpr SL_CullMode cull_mode_from_bits(value_type bits) noexcept;

    static constexpr value_type depth_test_to_bits(SL_DepthTest dt) noexcept;

    static constexpr SL_DepthTest depth_test_from_bits(value_type bits) noexcept;

    static constexpr value_type depth_mask_to_bits(SL_DepthMask dm) noexcept;

    static constexpr SL_DepthMask depth_mask_from_bits(value_type bits) noexcept;

    static constexpr value_type blend_mode_to_bits(SL_BlendMode bm) noexcept;

    static constexpr SL_BlendMode blend_mode_from_bits(value_type bits) noexcept;

  public:
    ~SL_RasterState() noexcept = default;

    constexpr SL_RasterState() noexcept;

    constexpr SL_RasterState(const SL_RasterState& rs) noexcept;

    constexpr SL_RasterState(SL_RasterState&& rs) noexcept;

    SL_RasterState& operator=(const SL_RasterState& rs) noexcept;

    SL_RasterState& operator=(SL_RasterState&& rs) noexcept;

    void reset() noexcept;

    constexpr value_type bits() const noexcept;

    void cull_mode(SL_CullMode cm) noexcept;

    constexpr SL_CullMode cull_mode() const noexcept;

    void depth_test(SL_DepthTest dt) noexcept;

    constexpr SL_DepthTest depth_test() const noexcept;

    void depth_mask(SL_DepthMask dm) noexcept;

    constexpr SL_DepthMask depth_mask() const noexcept;

    void blend_mode(SL_BlendMode bm) noexcept;

    constexpr SL_BlendMode blend_mode() const noexcept;

    void viewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept;

    constexpr ls::math::vec4_t<uint16_t> viewport() const noexcept;

    void scissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept;

    constexpr ls::math::vec4_t<uint16_t> scissor() const noexcept;

  private:
    value_type mStates;

    ls::math::vec4_t<uint16_t> mViewport;

    ls::math::vec4_t<uint16_t> mScissor;
};



/*-----------------------------------------------------------------------------
 * Rasterization State Storage (Implementations)
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Cull Mode to bits
-------------------------------------*/
constexpr SL_RasterState::value_type SL_RasterState::cull_mode_to_bits(SL_CullMode cm) noexcept
{
    return static_cast<SL_RasterState::value_type>(cm << SL_RasterState::SL_CULL_MODE_SHIFTS);
}



/*-------------------------------------
 * Cull Mode from bits
-------------------------------------*/
constexpr SL_CullMode SL_RasterState::cull_mode_from_bits(value_type bits) noexcept
{
    return static_cast<SL_CullMode>((bits & SL_RasterState::SL_CULL_MODE_MASK) >> SL_RasterState::SL_CULL_MODE_SHIFTS);
}



/*-------------------------------------
 * Depth Test to bits
-------------------------------------*/
constexpr SL_RasterState::value_type SL_RasterState::depth_test_to_bits(SL_DepthTest dt) noexcept
{
    return static_cast<SL_RasterState::value_type>(dt << SL_RasterState::SL_DEPTH_TEST_SHIFTS);
}



/*-------------------------------------
 * Depth Test from bits
-------------------------------------*/
constexpr SL_DepthTest SL_RasterState::depth_test_from_bits(value_type bits) noexcept
{
    return static_cast<SL_DepthTest>((bits & SL_RasterState::SL_DEPTH_TEST_MASK) >> SL_RasterState::SL_DEPTH_TEST_SHIFTS);
}



/*-------------------------------------
 * Depth Mask to bits
-------------------------------------*/
constexpr SL_RasterState::value_type SL_RasterState::depth_mask_to_bits(SL_DepthMask dm) noexcept
{
    return static_cast<SL_RasterState::value_type>(dm << SL_RasterState::SL_DEPTH_MASK_SHIFTS);
}



/*-------------------------------------
 * Depth Mask from bits
-------------------------------------*/
constexpr SL_DepthMask SL_RasterState::depth_mask_from_bits(value_type bits) noexcept
{
    return static_cast<SL_DepthMask>((bits & SL_RasterState::SL_DEPTH_MASK_MASK) >> SL_RasterState::SL_DEPTH_MASK_SHIFTS);
}



/*-------------------------------------
 * Blend mode to bits
-------------------------------------*/
constexpr SL_RasterState::value_type SL_RasterState::blend_mode_to_bits(SL_BlendMode bm) noexcept
{
    return static_cast<SL_RasterState::value_type>(bm << SL_RasterState::SL_BLEND_MODE_SHIFTS);
}



/*-------------------------------------
 * blend mode from bits
-------------------------------------*/
constexpr SL_BlendMode SL_RasterState::blend_mode_from_bits(value_type bits) noexcept
{
    return static_cast<SL_BlendMode>((bits & SL_RasterState::SL_BLEND_MODE_MASK) >> SL_RasterState::SL_BLEND_MODE_SHIFTS);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
constexpr SL_RasterState::SL_RasterState() noexcept :
    mStates{
        SL_RasterState::cull_mode_to_bits(SL_CullMode::SL_CULL_BACK_FACE) |
        SL_RasterState::depth_test_to_bits(SL_DepthTest::SL_DEPTH_TEST_ON) |
        SL_RasterState::depth_mask_to_bits(SL_DepthMask::SL_DEPTH_MASK_ON) |
        SL_RasterState::blend_mode_to_bits(SL_BlendMode::SL_BLEND_OFF)
    },
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
    mStates = 0
        | SL_RasterState::cull_mode_to_bits(SL_CullMode::SL_CULL_BACK_FACE)
        | SL_RasterState::depth_test_to_bits(SL_DepthTest::SL_DEPTH_TEST_ON)
        | SL_RasterState::depth_mask_to_bits(SL_DepthMask::SL_DEPTH_MASK_ON)
        | SL_RasterState::blend_mode_to_bits(SL_BlendMode::SL_BLEND_OFF)
        | 0;
    mViewport = ls::math::vec4_t<uint16_t>{0, 0, 65535, 65535};
    mScissor = ls::math::vec4_t<uint16_t>{0, 0, 65535, 65535};
}



/*-------------------------------------
 * Get the internal state
-------------------------------------*/
constexpr SL_RasterState::value_type SL_RasterState::bits() const noexcept
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
inline void SL_RasterState::viewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept
{
    mViewport = ls::math::vec4_t<uint16_t>{x, y, w, h};
}



/*-------------------------------------
 * viewport getter
-------------------------------------*/
constexpr ls::math::vec4_t<uint16_t> SL_RasterState::viewport() const noexcept
{
    return mViewport;
}



/*-------------------------------------
 * scissor setter
-------------------------------------*/
inline void SL_RasterState::scissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h) noexcept
{
    mScissor = ls::math::vec4_t<uint16_t>{x, y, w, h};
}



/*-------------------------------------
 * scissor getter
-------------------------------------*/
constexpr ls::math::vec4_t<uint16_t> SL_RasterState::scissor() const noexcept
{
    return mScissor;
}



#endif /* SL_RASTER_STATE_HPP */
