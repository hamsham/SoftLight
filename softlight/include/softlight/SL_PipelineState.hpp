
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



/*-------------------------------------
 * Varying Calculation
-------------------------------------*/
enum SL_VaryingCount : uint8_t
{
    SL_VARYING_COUNT_0,
    SL_VARYING_COUNT_1,
    SL_VARYING_COUNT_2,
    SL_VARYING_COUNT_3,
    SL_VARYING_COUNT_4,
};  // 5 states = 3 bits



/*-------------------------------------
 * Pipeline Outputs
-------------------------------------*/
enum SL_RenderTargetCount : uint8_t
{
    SL_RENDER_TARGET_COUNT_1,
    SL_RENDER_TARGET_COUNT_2,
    SL_RENDER_TARGET_COUNT_3,
    SL_RENDER_TARGET_COUNT_4,
};  // 4 states = 2 bits



/*-------------------------------------
 * Helper structures to ensure compile-time validation of bit masks
-------------------------------------*/
namespace sl_detail
{
    typedef uint16_t value_type;

    template <typename enum_type>
    struct PipelineEnumBits;

    // Currently 14/16 bits are used. "value_type" can be updated to uint32_t
    // if more bits are needed in the future, along with the class's alignment.
    template <> struct PipelineEnumBits<SL_CullMode>          { enum : sl_detail::value_type {mask = 0x0003, shifts = 0}; };
    template <> struct PipelineEnumBits<SL_DepthTest>         { enum : sl_detail::value_type {mask = 0x001C, shifts = 2}; };
    template <> struct PipelineEnumBits<SL_DepthMask>         { enum : sl_detail::value_type {mask = 0x0020, shifts = 5}; };
    template <> struct PipelineEnumBits<SL_BlendMode>         { enum : sl_detail::value_type {mask = 0x01C0, shifts = 6}; };
    template <> struct PipelineEnumBits<SL_VaryingCount>      { enum : sl_detail::value_type {mask = 0x0E00, shifts = 9}; };
    template <> struct PipelineEnumBits<SL_RenderTargetCount> { enum : sl_detail::value_type {mask = 0x3000, shifts = 12}; };

} // end SL_PipelineBitDetail namespace



/*-----------------------------------------------------------------------------
 * Render Pipeline State Storage
 *
 * This class should be lightweight so any overhead of copying shouldn't have
 * more overhead than assigning an __m128 or float32x4_t type. The reason is
 * the SL_PipelineState is copied into the software rasterizer, which should be
 * as freakishly fast as possible.
-----------------------------------------------------------------------------*/
class alignas(alignof(uint16_t)) SL_PipelineState
{
  public:
    typedef sl_detail::value_type value_type;

  private:
    value_type mStates;

    template <typename enum_type>
    static constexpr enum_type enum_value_from_bits(value_type bits) noexcept;

    template <typename enum_type>
    static constexpr value_type enum_value_to_bits(enum_type value) noexcept;

    template <typename enum_type>
    static constexpr value_type set_enum_bits(value_type bits, enum_type value) noexcept;

  public:
    ~SL_PipelineState() noexcept = default;

    constexpr SL_PipelineState() noexcept;

    constexpr SL_PipelineState(const SL_PipelineState& rs) noexcept;

    constexpr SL_PipelineState(SL_PipelineState&& rs) noexcept;

    SL_PipelineState& operator=(const SL_PipelineState& rs) noexcept;

    SL_PipelineState& operator=(SL_PipelineState&& rs) noexcept;

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

    void num_varyings(SL_VaryingCount vc) noexcept;

    constexpr SL_VaryingCount num_varyings() const noexcept;

    void num_render_targets(SL_RenderTargetCount rt) noexcept;

    constexpr SL_RenderTargetCount num_render_targets() const noexcept;
};



/*-----------------------------------------------------------------------------
 * Rasterization State Storage (Implementations)
-----------------------------------------------------------------------------*/
void sl_reset(SL_PipelineState& state) noexcept;



/*-------------------------------------
 * Enumerations to & from bits
-------------------------------------*/
template <typename enum_type>
constexpr enum_type SL_PipelineState::enum_value_from_bits(SL_PipelineState::value_type bits) noexcept
{
    using namespace sl_detail;
    return static_cast<enum_type>((bits & PipelineEnumBits<enum_type>::mask) >> PipelineEnumBits<enum_type>::shifts);
}

template <typename enum_type>
constexpr SL_PipelineState::value_type SL_PipelineState::enum_value_to_bits(enum_type value) noexcept
{
    using namespace sl_detail;
    return static_cast<SL_PipelineState::value_type>(value << PipelineEnumBits<enum_type>::shifts) & static_cast<SL_PipelineState::value_type>(PipelineEnumBits<enum_type>::mask);
}

template <typename enum_type>
constexpr SL_PipelineState::value_type SL_PipelineState::set_enum_bits(SL_PipelineState::value_type bits, enum_type value) noexcept
{
    using namespace sl_detail;
    return (bits | PipelineEnumBits<enum_type>::mask) & enum_value_to_bits<enum_type>(value);
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
constexpr SL_PipelineState::SL_PipelineState() noexcept :
    mStates{(SL_PipelineState::value_type)(
        SL_PipelineState::enum_value_to_bits<SL_CullMode>(SL_CullMode::SL_CULL_BACK_FACE) |
        SL_PipelineState::enum_value_to_bits<SL_DepthTest>(SL_DepthTest::SL_DEPTH_TEST_LESS_THAN) |
        SL_PipelineState::enum_value_to_bits<SL_DepthMask>(SL_DepthMask::SL_DEPTH_MASK_ON) |
        SL_PipelineState::enum_value_to_bits<SL_BlendMode>(SL_BlendMode::SL_BLEND_OFF) |
        SL_PipelineState::enum_value_to_bits<SL_VaryingCount>(SL_VaryingCount::SL_VARYING_COUNT_0) |
        SL_PipelineState::enum_value_to_bits<SL_RenderTargetCount>(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_1)
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
    return *this;
}



/*-------------------------------------
 * Get the internal state
-------------------------------------*/
constexpr SL_PipelineState::value_type SL_PipelineState::bits() const noexcept
{
    return mStates;
}



/*-------------------------------------
 * cull mode setter
-------------------------------------*/
inline void SL_PipelineState::cull_mode(SL_CullMode cm) noexcept
{
    mStates = SL_PipelineState::set_enum_bits<SL_CullMode>(mStates, cm);
}



/*-------------------------------------
 * cull mode getter
-------------------------------------*/
constexpr SL_CullMode SL_PipelineState::cull_mode() const noexcept
{
    return SL_PipelineState::enum_value_from_bits<SL_CullMode>(mStates);
}



/*-------------------------------------
 * depth test setter
-------------------------------------*/
inline void SL_PipelineState::depth_test(SL_DepthTest dt) noexcept
{
    mStates = SL_PipelineState::set_enum_bits<SL_DepthTest>(mStates, dt);
}



/*-------------------------------------
 * depth test getter
-------------------------------------*/
constexpr SL_DepthTest SL_PipelineState::depth_test() const noexcept
{
    return SL_PipelineState::enum_value_from_bits<SL_DepthTest>(mStates);
}



/*-------------------------------------
 * depth mask setter
-------------------------------------*/
inline void SL_PipelineState::depth_mask(SL_DepthMask dm) noexcept
{
    mStates = SL_PipelineState::set_enum_bits<SL_DepthMask>(mStates, dm);
}



/*-------------------------------------
 * depth mask getter
-------------------------------------*/
constexpr SL_DepthMask SL_PipelineState::depth_mask() const noexcept
{
    return SL_PipelineState::enum_value_from_bits<SL_DepthMask>(mStates);
}



/*-------------------------------------
 * blend mode setter
-------------------------------------*/
inline void SL_PipelineState::blend_mode(SL_BlendMode bm) noexcept
{
    mStates = SL_PipelineState::set_enum_bits<SL_BlendMode>(mStates, bm);
}



/*-------------------------------------
 * blend mode getter
-------------------------------------*/
constexpr SL_BlendMode SL_PipelineState::blend_mode() const noexcept
{
    return SL_PipelineState::enum_value_from_bits<SL_BlendMode>(mStates);
}

/*-------------------------------------
 * varying count setter
-------------------------------------*/
inline void SL_PipelineState::num_varyings(SL_VaryingCount vc) noexcept
{
    mStates = SL_PipelineState::set_enum_bits<SL_VaryingCount>(mStates, vc);
}



/*-------------------------------------
 * varying count getter
-------------------------------------*/
constexpr SL_VaryingCount SL_PipelineState::num_varyings() const noexcept
{
    return SL_PipelineState::enum_value_from_bits<SL_VaryingCount>(mStates);
}

/*-------------------------------------
 * render target count setter
-------------------------------------*/
inline void SL_PipelineState::num_render_targets(SL_RenderTargetCount rt) noexcept
{
    mStates = SL_PipelineState::set_enum_bits<SL_RenderTargetCount>(mStates, rt);
}



/*-------------------------------------
 * render target count getter
-------------------------------------*/
constexpr SL_RenderTargetCount SL_PipelineState::num_render_targets() const noexcept
{
    return SL_PipelineState::enum_value_from_bits<SL_RenderTargetCount>(mStates);
}



#endif /* SL_PIPELINE_STATE_HPP */
