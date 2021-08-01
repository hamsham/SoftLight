
#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/fixed.h"

#include "softlight/SL_Texture.hpp"
#include "softlight/SL_ClearProcesor.hpp"
#include "softlight/SL_ShaderUtil.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions and namespaces
-----------------------------------------------------------------------------*/
namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SL_ClearProcessor Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Pixel clearing
-------------------------------------*/
template<class color_type>
void SL_ClearProcessor::clear_texture(const color_type& inColor) noexcept
{
    size_t begin;
    size_t end;

    sl_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    color_type* pOutBuf = mBackBuffer->texel_pointer<color_type>(0, 0) + begin;
    const color_type* pEnd = mBackBuffer->texel_pointer<color_type>(0, 0) + end;

    while (LS_LIKELY(pOutBuf != pEnd))
    {
        *pOutBuf++ = inColor;
    }
}



#if defined(LS_ARCH_X86)
template <>
void SL_ClearProcessor::clear_texture<SL_ColorRType<float>>(const SL_ColorRType<float>& c) noexcept
{
    const int32_t inColor = reinterpret_cast<const int32_t&>(c);
    size_t begin;
    size_t end;

    sl_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    int32_t* pOutBuf = mBackBuffer->texel_pointer<int32_t>(0, 0) + begin;
    const int32_t* pEnd = mBackBuffer->texel_pointer<int32_t>(0, 0) + end;

    while (LS_LIKELY(pOutBuf != pEnd))
    {
        _mm_stream_si32(pOutBuf++, inColor);
    }
}



template <>
void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint8_t>>(const SL_ColorRGBAType<uint8_t>& c) noexcept
{
    const int32_t inColor = reinterpret_cast<const int32_t&>(c);
    size_t begin;
    size_t end;

    sl_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    int32_t* pOutBuf = mBackBuffer->texel_pointer<int32_t>(0, 0) + begin;
    const int32_t* pEnd = mBackBuffer->texel_pointer<int32_t>(0, 0) + end;

    while (LS_LIKELY(pOutBuf != pEnd))
    {
        _mm_stream_si32(pOutBuf++, inColor);
    }
}




template <>
void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint32_t>>(const SL_ColorRGBAType<uint32_t>& c) noexcept
{
    const __m128i inColor = reinterpret_cast<const __m128i&>(c);
    size_t begin;
    size_t end;

    sl_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    __m128i* pOutBuf = mBackBuffer->texel_pointer<__m128i>(0, 0) + begin;
    const __m128i* pEnd = mBackBuffer->texel_pointer<__m128i>(0, 0) + end;

    while (LS_LIKELY(pOutBuf != pEnd))
    {
        _mm_stream_si128(pOutBuf++, inColor);
    }
}




template <>
void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<float>>(const SL_ColorRGBAType<float>& c) noexcept
{
    const __m128i inColor = reinterpret_cast<const __m128i&>(c);
    size_t begin;
    size_t end;

    sl_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    __m128i* pOutBuf = mBackBuffer->texel_pointer<__m128i>(0, 0) + begin;
    const __m128i* pEnd = mBackBuffer->texel_pointer<__m128i>(0, 0) + end;

    while (LS_LIKELY(pOutBuf != pEnd))
    {
        _mm_stream_si128(pOutBuf++, inColor);
    }
}

#endif



template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint8_t>>(const SL_ColorRType<uint8_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint16_t>>(const SL_ColorRType<uint16_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint32_t>>(const SL_ColorRType<uint32_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint64_t>>(const SL_ColorRType<uint64_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRType<double>>(const SL_ColorRType<double>&) noexcept;

template void SL_ClearProcessor::clear_texture<SL_ColorRGType<uint8_t>>(const SL_ColorRGType<uint8_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGType<uint16_t>>(const SL_ColorRGType<uint16_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGType<uint32_t>>(const SL_ColorRGType<uint32_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGType<uint64_t>>(const SL_ColorRGType<uint64_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGType<float>>(const SL_ColorRGType<float>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGType<double>>(const SL_ColorRGType<double>&) noexcept;

template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<uint8_t>>(const SL_ColorRGBType<uint8_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<uint16_t>>(const SL_ColorRGBType<uint16_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<uint32_t>>(const SL_ColorRGBType<uint32_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<uint64_t>>(const SL_ColorRGBType<uint64_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<float>>(const SL_ColorRGBType<float>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBType<double>>(const SL_ColorRGBType<double>&) noexcept;

template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint16_t>>(const SL_ColorRGBAType<uint16_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint64_t>>(const SL_ColorRGBAType<uint64_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<double>>(const SL_ColorRGBAType<double>&) noexcept;

#if !defined(LS_ARCH_X86)
    template void SL_ClearProcessor::clear_texture<SL_ColorRType<float>>(const SL_ColorRType<float>&) noexcept;

    template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint8_t>>(const SL_ColorRGBAType<uint8_t>&) noexcept;
    template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint32_t>>(const SL_ColorRGBAType<uint32_t>&) noexcept;
    template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<float>>(const SL_ColorRGBAType<float>&) noexcept;
#endif


/*-------------------------------------
 * Run the texture clearer
-------------------------------------*/
void SL_ClearProcessor::execute() noexcept
{
    switch (mBackBuffer->type())
    {
        case SL_COLOR_R_8U:       clear_texture<SL_ColorRType<uint8_t>>(*reinterpret_cast<const SL_ColorRType<uint8_t>*>(mTexture));     break;
        case SL_COLOR_R_16U:      clear_texture<SL_ColorRType<uint16_t>>(*reinterpret_cast<const SL_ColorRType<uint16_t>*>(mTexture));    break;
        case SL_COLOR_R_32U:      clear_texture<SL_ColorRType<uint32_t>>(*reinterpret_cast<const SL_ColorRType<uint32_t>*>(mTexture));    break;
        case SL_COLOR_R_64U:      clear_texture<SL_ColorRType<uint64_t>>(*reinterpret_cast<const SL_ColorRType<uint64_t>*>(mTexture));    break;
        case SL_COLOR_R_FLOAT:    clear_texture<SL_ColorRType<float>>(*reinterpret_cast<const SL_ColorRType<float>*>(mTexture));       break;
        case SL_COLOR_R_DOUBLE:   clear_texture<SL_ColorRType<double>>(*reinterpret_cast<const SL_ColorRType<double>*>(mTexture));      break;

        case SL_COLOR_RG_8U:      clear_texture<SL_ColorRGType<uint8_t>>(*reinterpret_cast<const SL_ColorRGType<uint8_t>*>(mTexture));    break;
        case SL_COLOR_RG_16U:     clear_texture<SL_ColorRGType<uint16_t>>(*reinterpret_cast<const SL_ColorRGType<uint16_t>*>(mTexture));   break;
        case SL_COLOR_RG_32U:     clear_texture<SL_ColorRGType<uint32_t>>(*reinterpret_cast<const SL_ColorRGType<uint32_t>*>(mTexture));   break;
        case SL_COLOR_RG_64U:     clear_texture<SL_ColorRGType<uint64_t>>(*reinterpret_cast<const SL_ColorRGType<uint64_t>*>(mTexture));   break;
        case SL_COLOR_RG_FLOAT:   clear_texture<SL_ColorRGType<float>>(*reinterpret_cast<const SL_ColorRGType<float>*>(mTexture));      break;
        case SL_COLOR_RG_DOUBLE:  clear_texture<SL_ColorRGType<double>>(*reinterpret_cast<const SL_ColorRGType<double>*>(mTexture));     break;

        case SL_COLOR_RGB_8U:     clear_texture<SL_ColorRGBType<uint8_t>>(*reinterpret_cast<const SL_ColorRGBType<uint8_t>*>(mTexture));   break;
        case SL_COLOR_RGB_16U:    clear_texture<SL_ColorRGBType<uint16_t>>(*reinterpret_cast<const SL_ColorRGBType<uint16_t>*>(mTexture));  break;
        case SL_COLOR_RGB_32U:    clear_texture<SL_ColorRGBType<uint32_t>>(*reinterpret_cast<const SL_ColorRGBType<uint32_t>*>(mTexture));  break;
        case SL_COLOR_RGB_64U:    clear_texture<SL_ColorRGBType<uint64_t>>(*reinterpret_cast<const SL_ColorRGBType<uint64_t>*>(mTexture));  break;
        case SL_COLOR_RGB_FLOAT:  clear_texture<SL_ColorRGBType<float>>(*reinterpret_cast<const SL_ColorRGBType<float>*>(mTexture));     break;
        case SL_COLOR_RGB_DOUBLE: clear_texture<SL_ColorRGBType<double>>(*reinterpret_cast<const SL_ColorRGBType<double>*>(mTexture));    break;

        case SL_COLOR_RGBA_8U:     clear_texture<SL_ColorRGBAType<uint8_t>>(*reinterpret_cast<const SL_ColorRGBAType<uint8_t>*>(mTexture));  break;
        case SL_COLOR_RGBA_16U:    clear_texture<SL_ColorRGBAType<uint16_t>>(*reinterpret_cast<const SL_ColorRGBAType<uint16_t>*>(mTexture)); break;
        case SL_COLOR_RGBA_32U:    clear_texture<SL_ColorRGBAType<uint32_t>>(*reinterpret_cast<const SL_ColorRGBAType<uint32_t>*>(mTexture)); break;
        case SL_COLOR_RGBA_64U:    clear_texture<SL_ColorRGBAType<uint64_t>>(*reinterpret_cast<const SL_ColorRGBAType<uint64_t>*>(mTexture)); break;
        case SL_COLOR_RGBA_FLOAT:  clear_texture<SL_ColorRGBAType<float>>(*reinterpret_cast<const SL_ColorRGBAType<float>*>(mTexture));    break;
        case SL_COLOR_RGBA_DOUBLE: clear_texture<SL_ColorRGBAType<double>>(*reinterpret_cast<const SL_ColorRGBAType<double>*>(mTexture));   break;
    }
}
