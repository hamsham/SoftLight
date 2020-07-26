
#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/fixed.h"

#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_ClearProcesor.hpp"
#include "soft_render/SR_ShaderUtil.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions and namespaces
-----------------------------------------------------------------------------*/
namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SR_ClearProcessor Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Pixel clearing
-------------------------------------*/
template<class color_type>
void SR_ClearProcessor::clear_texture(const color_type& inColor) noexcept
{
    size_t begin;
    size_t end;

    sr_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    color_type* pOutBuf = mBackBuffer->texel_pointer<color_type>(0, 0) + begin;
    const color_type* pEnd = mBackBuffer->texel_pointer<color_type>(0, 0) + end;

    while (LS_UNLIKELY(pOutBuf != pEnd))
    {
        *pOutBuf++ = inColor;
    }
}



#if defined(LS_ARCH_X86)
template <>
void SR_ClearProcessor::clear_texture<SR_ColorRType<float>>(const SR_ColorRType<float>& c) noexcept
{
    const int32_t inColor = reinterpret_cast<const int32_t&>(c);
    size_t begin;
    size_t end;

    sr_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    int32_t* pOutBuf = mBackBuffer->texel_pointer<int32_t>(0, 0) + begin;
    const int32_t* pEnd = mBackBuffer->texel_pointer<int32_t>(0, 0) + end;

    while (LS_UNLIKELY(pOutBuf != pEnd))
    {
        _mm_stream_si32(pOutBuf++, inColor);
    }
}



template <>
void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint8_t>>(const SR_ColorRGBAType<uint8_t>& c) noexcept
{
    const int32_t inColor = reinterpret_cast<const int32_t&>(c);
    size_t begin;
    size_t end;

    sr_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    int32_t* pOutBuf = mBackBuffer->texel_pointer<int32_t>(0, 0) + begin;
    const int32_t* pEnd = mBackBuffer->texel_pointer<int32_t>(0, 0) + end;

    while (LS_UNLIKELY(pOutBuf != pEnd))
    {
        _mm_stream_si32(pOutBuf++, inColor);
    }
}




template <>
void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint32_t>>(const SR_ColorRGBAType<uint32_t>& c) noexcept
{
    const __m128i inColor = reinterpret_cast<const __m128i&>(c);
    size_t begin;
    size_t end;

    sr_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    __m128i* pOutBuf = mBackBuffer->texel_pointer<__m128i>(0, 0) + begin;
    const __m128i* pEnd = mBackBuffer->texel_pointer<__m128i>(0, 0) + end;

    while (LS_UNLIKELY(pOutBuf != pEnd))
    {
        _mm_stream_si128(pOutBuf++, inColor);
    }
}




template <>
void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<float>>(const SR_ColorRGBAType<float>& c) noexcept
{
    const __m128i inColor = reinterpret_cast<const __m128i&>(c);
    size_t begin;
    size_t end;

    sr_calc_indexed_parition<1, true>((size_t)mBackBuffer->width()*(size_t)mBackBuffer->height(), (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    __m128i* pOutBuf = mBackBuffer->texel_pointer<__m128i>(0, 0) + begin;
    const __m128i* pEnd = mBackBuffer->texel_pointer<__m128i>(0, 0) + end;

    while (LS_UNLIKELY(pOutBuf != pEnd))
    {
        _mm_stream_si128(pOutBuf++, inColor);
    }
}

#endif



template void SR_ClearProcessor::clear_texture<SR_ColorRType<uint8_t>>(const SR_ColorRType<uint8_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRType<uint16_t>>(const SR_ColorRType<uint16_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRType<uint32_t>>(const SR_ColorRType<uint32_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRType<uint64_t>>(const SR_ColorRType<uint64_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRType<double>>(const SR_ColorRType<double>&) noexcept;

template void SR_ClearProcessor::clear_texture<SR_ColorRGType<uint8_t>>(const SR_ColorRGType<uint8_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGType<uint16_t>>(const SR_ColorRGType<uint16_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGType<uint32_t>>(const SR_ColorRGType<uint32_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGType<uint64_t>>(const SR_ColorRGType<uint64_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGType<float>>(const SR_ColorRGType<float>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGType<double>>(const SR_ColorRGType<double>&) noexcept;

template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<uint8_t>>(const SR_ColorRGBType<uint8_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<uint16_t>>(const SR_ColorRGBType<uint16_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<uint32_t>>(const SR_ColorRGBType<uint32_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<uint64_t>>(const SR_ColorRGBType<uint64_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<float>>(const SR_ColorRGBType<float>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGBType<double>>(const SR_ColorRGBType<double>&) noexcept;

template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint16_t>>(const SR_ColorRGBAType<uint16_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint64_t>>(const SR_ColorRGBAType<uint64_t>&) noexcept;
template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<double>>(const SR_ColorRGBAType<double>&) noexcept;

#if !defined(LS_ARCH_X86)
    template void SR_ClearProcessor::clear_texture<SR_ColorRType<float>>(const SR_ColorRType<float>&) noexcept;

    template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint8_t>>(const SR_ColorRGBAType<uint8_t>&) noexcept;
    template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<uint32_t>>(const SR_ColorRGBAType<uint32_t>&) noexcept;
    template void SR_ClearProcessor::clear_texture<SR_ColorRGBAType<float>>(const SR_ColorRGBAType<float>&) noexcept;
#endif


/*-------------------------------------
 * Run the texture clearer
-------------------------------------*/
void SR_ClearProcessor::execute() noexcept
{
    switch (mBackBuffer->type())
    {
        case SR_COLOR_R_8U:       clear_texture<SR_ColorRType<uint8_t>>(*reinterpret_cast<const SR_ColorRType<uint8_t>*>(mTexture));     break;
        case SR_COLOR_R_16U:      clear_texture<SR_ColorRType<uint16_t>>(*reinterpret_cast<const SR_ColorRType<uint16_t>*>(mTexture));    break;
        case SR_COLOR_R_32U:      clear_texture<SR_ColorRType<uint32_t>>(*reinterpret_cast<const SR_ColorRType<uint32_t>*>(mTexture));    break;
        case SR_COLOR_R_64U:      clear_texture<SR_ColorRType<uint64_t>>(*reinterpret_cast<const SR_ColorRType<uint64_t>*>(mTexture));    break;
        case SR_COLOR_R_FLOAT:    clear_texture<SR_ColorRType<float>>(*reinterpret_cast<const SR_ColorRType<float>*>(mTexture));       break;
        case SR_COLOR_R_DOUBLE:   clear_texture<SR_ColorRType<double>>(*reinterpret_cast<const SR_ColorRType<double>*>(mTexture));      break;

        case SR_COLOR_RG_8U:      clear_texture<SR_ColorRGType<uint8_t>>(*reinterpret_cast<const SR_ColorRGType<uint8_t>*>(mTexture));    break;
        case SR_COLOR_RG_16U:     clear_texture<SR_ColorRGType<uint16_t>>(*reinterpret_cast<const SR_ColorRGType<uint16_t>*>(mTexture));   break;
        case SR_COLOR_RG_32U:     clear_texture<SR_ColorRGType<uint32_t>>(*reinterpret_cast<const SR_ColorRGType<uint32_t>*>(mTexture));   break;
        case SR_COLOR_RG_64U:     clear_texture<SR_ColorRGType<uint64_t>>(*reinterpret_cast<const SR_ColorRGType<uint64_t>*>(mTexture));   break;
        case SR_COLOR_RG_FLOAT:   clear_texture<SR_ColorRGType<float>>(*reinterpret_cast<const SR_ColorRGType<float>*>(mTexture));      break;
        case SR_COLOR_RG_DOUBLE:  clear_texture<SR_ColorRGType<double>>(*reinterpret_cast<const SR_ColorRGType<double>*>(mTexture));     break;

        case SR_COLOR_RGB_8U:     clear_texture<SR_ColorRGBType<uint8_t>>(*reinterpret_cast<const SR_ColorRGBType<uint8_t>*>(mTexture));   break;
        case SR_COLOR_RGB_16U:    clear_texture<SR_ColorRGBType<uint16_t>>(*reinterpret_cast<const SR_ColorRGBType<uint16_t>*>(mTexture));  break;
        case SR_COLOR_RGB_32U:    clear_texture<SR_ColorRGBType<uint32_t>>(*reinterpret_cast<const SR_ColorRGBType<uint32_t>*>(mTexture));  break;
        case SR_COLOR_RGB_64U:    clear_texture<SR_ColorRGBType<uint64_t>>(*reinterpret_cast<const SR_ColorRGBType<uint64_t>*>(mTexture));  break;
        case SR_COLOR_RGB_FLOAT:  clear_texture<SR_ColorRGBType<float>>(*reinterpret_cast<const SR_ColorRGBType<float>*>(mTexture));     break;
        case SR_COLOR_RGB_DOUBLE: clear_texture<SR_ColorRGBType<double>>(*reinterpret_cast<const SR_ColorRGBType<double>*>(mTexture));    break;

        case SR_COLOR_RGBA_8U:     clear_texture<SR_ColorRGBAType<uint8_t>>(*reinterpret_cast<const SR_ColorRGBAType<uint8_t>*>(mTexture));  break;
        case SR_COLOR_RGBA_16U:    clear_texture<SR_ColorRGBAType<uint16_t>>(*reinterpret_cast<const SR_ColorRGBAType<uint16_t>*>(mTexture)); break;
        case SR_COLOR_RGBA_32U:    clear_texture<SR_ColorRGBAType<uint32_t>>(*reinterpret_cast<const SR_ColorRGBAType<uint32_t>*>(mTexture)); break;
        case SR_COLOR_RGBA_64U:    clear_texture<SR_ColorRGBAType<uint64_t>>(*reinterpret_cast<const SR_ColorRGBAType<uint64_t>*>(mTexture)); break;
        case SR_COLOR_RGBA_FLOAT:  clear_texture<SR_ColorRGBAType<float>>(*reinterpret_cast<const SR_ColorRGBAType<float>*>(mTexture));    break;
        case SR_COLOR_RGBA_DOUBLE: clear_texture<SR_ColorRGBAType<double>>(*reinterpret_cast<const SR_ColorRGBAType<double>*>(mTexture));   break;
    }
}
