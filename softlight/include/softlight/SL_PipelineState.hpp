
#ifndef SL_PIPELINE_STATE_HPP
#define SL_PIPELINE_STATE_HPP

#include <cstdint>



/*-----------------------------------------------------------------------------
 * Fixed-Function Pipeline State
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
 * Render Pipeline State Storage
 *
 * This class should be lightweight so any overhead of copying shouldn't have
 * more overhead than assigning an __m128 or float32x4_t type. The reason is
 * the SL_PipelineState is copied into the software rasterizer, which should be
 * as freakishly fast as possible.
-----------------------------------------------------------------------------*/
class SL_PipelineState
{
  private:
    enum SL_PipelineStateShifts : uint32_t
    {
        SL_CULL_MODE_SHIFTS  = 0,
        SL_DEPTH_TEST_SHIFTS = 2,
        SL_DEPTH_MASK_SHIFTS = 5,
        SL_BLEND_MODE_SHIFTS = 6
    };

    enum SL_PipelineStateMask : uint32_t
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
    ~SL_PipelineState() noexcept = default;

    constexpr SL_PipelineState() noexcept;

    constexpr SL_PipelineState(const SL_PipelineState& rs) noexcept;

    constexpr SL_PipelineState(SL_PipelineState&& rs) noexcept;

    SL_PipelineState& operator=(const SL_PipelineState& rs) noexcept;

    SL_PipelineState& operator=(SL_PipelineState&& rs) noexcept;

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

  private:
    uint32_t mStates;
};



/*-----------------------------------------------------------------------------
 * Rasterization State Storage (Implementations)
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Cull Mode to bits
-------------------------------------*/
constexpr uint32_t SL_PipelineState::cull_mode_to_bits(SL_CullMode cm) noexcept
{
    return static_cast<uint32_t>(cm << SL_PipelineState::SL_CULL_MODE_SHIFTS);
}



/*-------------------------------------
 * Cull Mode from bits
-------------------------------------*/
constexpr SL_CullMode SL_PipelineState::cull_mode_from_bits(uint32_t bits) noexcept
{
    return static_cast<SL_CullMode>((bits & SL_PipelineState::SL_CULL_MODE_MASK) >> SL_PipelineState::SL_CULL_MODE_SHIFTS);
}



/*-------------------------------------
 * Depth Test to bits
-------------------------------------*/
constexpr uint32_t SL_PipelineState::depth_test_to_bits(SL_DepthTest dt) noexcept
{
    return static_cast<uint32_t>(dt << SL_PipelineState::SL_DEPTH_TEST_SHIFTS);
}



/*-------------------------------------
 * Depth Test from bits
-------------------------------------*/
constexpr SL_DepthTest SL_PipelineState::depth_test_from_bits(uint32_t bits) noexcept
{
    return static_cast<SL_DepthTest>((bits & SL_PipelineState::SL_DEPTH_TEST_MASK) >> SL_PipelineState::SL_DEPTH_TEST_SHIFTS);
}



/*-------------------------------------
 * Depth Mask to bits
-------------------------------------*/
constexpr uint32_t SL_PipelineState::depth_mask_to_bits(SL_DepthMask dm) noexcept
{
    return static_cast<uint32_t>(dm << SL_PipelineState::SL_DEPTH_MASK_SHIFTS);
}



/*-------------------------------------
 * Depth Mask from bits
-------------------------------------*/
constexpr SL_DepthMask SL_PipelineState::depth_mask_from_bits(uint32_t bits) noexcept
{
    return static_cast<SL_DepthMask>((bits & SL_PipelineState::SL_DEPTH_MASK_MASK) >> SL_PipelineState::SL_DEPTH_MASK_SHIFTS);
}



/*-------------------------------------
 * Blend mode to bits
-------------------------------------*/
constexpr uint32_t SL_PipelineState::blend_mode_to_bits(SL_BlendMode bm) noexcept
{
    return static_cast<uint32_t>(bm << SL_PipelineState::SL_BLEND_MODE_SHIFTS);
}



/*-------------------------------------
 * blend mode from bits
-------------------------------------*/
constexpr SL_BlendMode SL_PipelineState::blend_mode_from_bits(uint32_t bits) noexcept
{
    return static_cast<SL_BlendMode>((bits & SL_PipelineState::SL_BLEND_MODE_MASK) >> SL_PipelineState::SL_BLEND_MODE_SHIFTS);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
constexpr SL_PipelineState::SL_PipelineState() noexcept :
    mStates{(uint32_t)(
        SL_PipelineState::cull_mode_to_bits(SL_CullMode::SL_CULL_BACK_FACE) |
        SL_PipelineState::depth_test_to_bits(SL_DepthTest::SL_DEPTH_TEST_LESS_THAN) |
        SL_PipelineState::depth_mask_to_bits(SL_DepthMask::SL_DEPTH_MASK_ON) |
        SL_PipelineState::blend_mode_to_bits(SL_BlendMode::SL_BLEND_OFF)
    )}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
constexpr SL_PipelineState::SL_PipelineState(const SL_PipelineState& rs) noexcept :
    mStates{rs.mStates}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
constexpr SL_PipelineState::SL_PipelineState(SL_PipelineState&& rs) noexcept :
    mStates{rs.mStates}
{}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
inline SL_PipelineState& SL_PipelineState::operator=(const SL_PipelineState& rs) noexcept
{
    mStates = rs.mStates;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
inline SL_PipelineState& SL_PipelineState::operator=(SL_PipelineState&& rs) noexcept
{
    mStates = rs.mStates;
    rs.reset();

    return *this;
}



/*-------------------------------------
 * Reset Internal State
-------------------------------------*/
inline void SL_PipelineState::reset() noexcept
{
    mStates = (uint32_t)(0
                         | SL_PipelineState::cull_mode_to_bits(SL_CullMode::SL_CULL_BACK_FACE)
                         | SL_PipelineState::depth_test_to_bits(SL_DepthTest::SL_DEPTH_TEST_LESS_THAN)
                         | SL_PipelineState::depth_mask_to_bits(SL_DepthMask::SL_DEPTH_MASK_ON)
                         | SL_PipelineState::blend_mode_to_bits(SL_BlendMode::SL_BLEND_OFF)
                         | 0);
}



/*-------------------------------------
 * Get the internal state
-------------------------------------*/
constexpr uint32_t SL_PipelineState::bits() const noexcept
{
    return mStates;
}



/*-------------------------------------
 * cull mode setter
-------------------------------------*/
inline void SL_PipelineState::cull_mode(SL_CullMode cm) noexcept
{
    mStates = (mStates & ~SL_CULL_MODE_MASK) | cull_mode_to_bits(cm);
}



/*-------------------------------------
 * cull mode getter
-------------------------------------*/
constexpr SL_CullMode SL_PipelineState::cull_mode() const noexcept
{
    return cull_mode_from_bits(mStates);
}



/*-------------------------------------
 * depth test setter
-------------------------------------*/
inline void SL_PipelineState::depth_test(SL_DepthTest dt) noexcept
{
    mStates = (mStates & ~SL_DEPTH_TEST_MASK) | depth_test_to_bits(dt);
}



/*-------------------------------------
 * depth test getter
-------------------------------------*/
constexpr SL_DepthTest SL_PipelineState::depth_test() const noexcept
{
    return depth_test_from_bits(mStates);
}



/*-------------------------------------
 * depth mask setter
-------------------------------------*/
inline void SL_PipelineState::depth_mask(SL_DepthMask dm) noexcept
{
    mStates = (mStates & ~SL_DEPTH_MASK_MASK) | depth_mask_to_bits(dm);
}



/*-------------------------------------
 * depth mask getter
-------------------------------------*/
constexpr SL_DepthMask SL_PipelineState::depth_mask() const noexcept
{
    return depth_mask_from_bits(mStates);
}



/*-------------------------------------
 * blend mode setter
-------------------------------------*/
inline void SL_PipelineState::blend_mode(SL_BlendMode bm) noexcept
{
    mStates = (mStates & ~SL_BLEND_MODE_MASK) | blend_mode_to_bits(bm);
}



/*-------------------------------------
 * blend mode getter
-------------------------------------*/
constexpr SL_BlendMode SL_PipelineState::blend_mode() const noexcept
{
    return blend_mode_from_bits(mStates);
}



#endif /* SL_PIPELINE_STATE_HPP */
