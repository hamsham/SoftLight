
#ifndef SR_CLEAR_PROCESSOR_HPP
#define SR_CLEAR_PROCESSOR_HPP

#include <cstdint>

#include "soft_render/SR_Color.hpp"



/**----------------------------------------------------------------------------
 * @brief The Clear Processor helps to assign all texels in a texture to a
 * single color. This helps distribute color clearing across multiple threads.
-----------------------------------------------------------------------------*/
struct SR_ClearProcessor
{
    using sr_fixed_type = ls::math::ulong_lowp_t;

    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 64-128 bits
    const void* mTexture;
    SR_Texture* mBackBuffer;

    // 96-160 bits total, 12-20 bytes

    // clear all 4 color components
    template<typename color_type>
    void clear_texture(const color_type& inColor) noexcept;

    void execute() noexcept;
};



#if defined(LS_ARCH_X86)

template <> void SR_ClearProcessor::clear_texture<SR_ColorRType<float>>(const SR_ColorRType<float>&) noexcept;

template <> void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint8_t>>(const SR_ColorRGBAType<uint8_t>&) noexcept;
template <> void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint32_t>>(const SR_ColorRGBAType<uint32_t>&) noexcept;
template <> void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<float>>(const SR_ColorRGBAType<float>&) noexcept;

#endif



extern template void SR_ClearProcessor::clear_texture<SR_ColorRType<uint8_t>>(const SR_ColorRType<uint8_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRType<uint16_t>>(const SR_ColorRType<uint16_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRType<uint32_t>>(const SR_ColorRType<uint32_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRType<uint64_t>>(const SR_ColorRType<uint64_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRType<double>>(const SR_ColorRType<double>&) noexcept;

extern template void SR_ClearProcessor::clear_texture<SR_ColorRGType<uint8_t>>(const SR_ColorRGType<uint8_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGType<uint16_t>>(const SR_ColorRGType<uint16_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGType<uint32_t>>(const SR_ColorRGType<uint32_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGType<uint64_t>>(const SR_ColorRGType<uint64_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGType<float>>(const SR_ColorRGType<float>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGType<double>>(const SR_ColorRGType<double>&) noexcept;

extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<uint8_t>>(const SR_ColorRGBType<uint8_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<uint16_t>>(const SR_ColorRGBType<uint16_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<uint32_t>>(const SR_ColorRGBType<uint32_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<uint64_t>>(const SR_ColorRGBType<uint64_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<float>>(const SR_ColorRGBType<float>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<double>>(const SR_ColorRGBType<double>&) noexcept;

extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint16_t>>(const SR_ColorRGBAType<uint16_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint64_t>>(const SR_ColorRGBAType<uint64_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<double>>(const SR_ColorRGBAType<double>&) noexcept;



#if !defined(LS_ARCH_X86)

extern template void SR_ClearProcessor::clear_texture<SR_ColorRType<float>>(const SR_ColorRType<float>&) noexcept;

extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint8_t>>(const SR_ColorRGBAType<uint8_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint32_t>>(const SR_ColorRGBAType<uint32_t>&) noexcept;
extern template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<float>>(const SR_ColorRGBAType<float>&) noexcept;

#endif



#endif /* SR_CLEAR_PROCESSOR_HPP */
