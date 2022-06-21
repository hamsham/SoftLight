
#ifndef SL_CLEAR_PROCESSOR_HPP
#define SL_CLEAR_PROCESSOR_HPP

#include <cstdint>

#include "softlight/SL_Color.hpp"
#include "softlight/SL_ColorCompressed.hpp"



class SL_Texture;



/**----------------------------------------------------------------------------
 * @brief The Clear Processor helps to assign all texels in a texture to a
 * single color. This helps distribute color clearing across multiple threads.
-----------------------------------------------------------------------------*/
struct SL_ClearProcessor
{
    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 64-128 bits
    const void* mTexture;
    SL_Texture* mBackBuffer;

    // 96-160 bits total, 12-20 bytes

    // clear all 4 color components
    template<typename color_type>
    void clear_texture(const color_type& inColor) noexcept;

    void execute() noexcept;
};



#if defined(LS_ARCH_X86)

template <> void SL_ClearProcessor::clear_texture<SL_ColorRType<float>>(const SL_ColorRType<float>&) noexcept;

template <> void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint8_t>>(const SL_ColorRGBAType<uint8_t>&) noexcept;
template <> void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint32_t>>(const SL_ColorRGBAType<uint32_t>&) noexcept;
template <> void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<float>>(const SL_ColorRGBAType<float>&) noexcept;

#endif



extern template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint8_t>>(const SL_ColorRType<uint8_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint16_t>>(const SL_ColorRType<uint16_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint32_t>>(const SL_ColorRType<uint32_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint64_t>>(const SL_ColorRType<uint64_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRType<double>>(const SL_ColorRType<double>&) noexcept;

extern template void SL_ClearProcessor::clear_texture<SL_ColorRGType<uint8_t>>(const SL_ColorRGType<uint8_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGType<uint16_t>>(const SL_ColorRGType<uint16_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGType<uint32_t>>(const SL_ColorRGType<uint32_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGType<uint64_t>>(const SL_ColorRGType<uint64_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGType<float>>(const SL_ColorRGType<float>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGType<double>>(const SL_ColorRGType<double>&) noexcept;

extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<uint8_t>>(const SL_ColorRGBType<uint8_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<uint16_t>>(const SL_ColorRGBType<uint16_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<uint32_t>>(const SL_ColorRGBType<uint32_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<uint64_t>>(const SL_ColorRGBType<uint64_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<float>>(const SL_ColorRGBType<float>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<double>>(const SL_ColorRGBType<double>&) noexcept;

extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint16_t>>(const SL_ColorRGBAType<uint16_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint64_t>>(const SL_ColorRGBAType<uint64_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<double>>(const SL_ColorRGBAType<double>&) noexcept;

extern template void SL_ClearProcessor::clear_texture<SL_ColorRGB332>(const SL_ColorRGB332&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGB565>(const SL_ColorRGB565&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGB5551>(const SL_ColorRGB5551&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGB4444>(const SL_ColorRGB4444&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGB1010102>(const SL_ColorRGB1010102&) noexcept;



#if !defined(LS_ARCH_X86)

extern template void SL_ClearProcessor::clear_texture<SL_ColorRType<float>>(const SL_ColorRType<float>&) noexcept;

extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint8_t>>(const SL_ColorRGBAType<uint8_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint32_t>>(const SL_ColorRGBAType<uint32_t>&) noexcept;
extern template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<float>>(const SL_ColorRGBAType<float>&) noexcept;

#endif



#endif /* SL_CLEAR_PROCESSOR_HPP */
