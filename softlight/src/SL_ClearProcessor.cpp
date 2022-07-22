
#include "lightsky/utils/Copy.h"

#include "softlight/SL_ClearProcesor.hpp"
#include "softlight/SL_ShaderUtil.hpp"
#include "softlight/SL_Texture.hpp"



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
    size_t w = (size_t)mBackBuffer->width();
    size_t h = (size_t)mBackBuffer->height();
    size_t numBytes = w * h;
    size_t begin;
    size_t end;

    sl_calc_indexed_parition2<1>(numBytes, (size_t)mNumThreads, (size_t)mThreadId, begin, end);
    color_type* pOutBuf = (color_type*)mBackBuffer->data() + begin;
    size_t count = end - begin;

    LS_PREFETCH(pOutBuf, LS_PREFETCH_ACCESS_RW, LS_PREFETCH_LEVEL_NONTEMPORAL);

    switch (sizeof(inColor))
    {
        case sizeof(uint8_t):
            ls::utils::fast_memset(pOutBuf, *reinterpret_cast<const uint8_t*>(&inColor), count*sizeof(uint8_t));
            break;

        case sizeof(uint16_t):
            ls::utils::fast_memset_2(pOutBuf, *reinterpret_cast<const uint16_t*>(&inColor), count*sizeof(uint16_t));
            break;

        case sizeof(uint32_t):
            ls::utils::fast_memset_4(pOutBuf, *reinterpret_cast<const uint32_t*>(&inColor), count*sizeof(uint32_t));
            break;

        case sizeof(uint64_t):
            ls::utils::fast_memset_8(pOutBuf, *reinterpret_cast<const uint64_t*>(&inColor), count*sizeof(uint64_t));
            break;

        default:
            while (LS_LIKELY(count--))
            {
                *pOutBuf++ = inColor;
            }
            break;
    }
}



template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint8_t>>(const SL_ColorRType<uint8_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint16_t>>(const SL_ColorRType<uint16_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint32_t>>(const SL_ColorRType<uint32_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRType<uint64_t>>(const SL_ColorRType<uint64_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRType<float>>(const SL_ColorRType<float>&) noexcept;
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

template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint8_t>>(const SL_ColorRGBAType<uint8_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint16_t>>(const SL_ColorRGBAType<uint16_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint32_t>>(const SL_ColorRGBAType<uint32_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<uint64_t>>(const SL_ColorRGBAType<uint64_t>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<float>>(const SL_ColorRGBAType<float>&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGBAType<double>>(const SL_ColorRGBAType<double>&) noexcept;

template void SL_ClearProcessor::clear_texture<SL_ColorRGB332>(const SL_ColorRGB332&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGB565>(const SL_ColorRGB565&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGB5551>(const SL_ColorRGB5551&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGB4444>(const SL_ColorRGB4444&) noexcept;
template void SL_ClearProcessor::clear_texture<SL_ColorRGB1010102>(const SL_ColorRGB1010102&) noexcept;



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

        case SL_COLOR_RGB_332:      clear_texture<SL_ColorRGB332>(*reinterpret_cast<const SL_ColorRGB332*>(mTexture)); break;
        case SL_COLOR_RGB_565:      clear_texture<SL_ColorRGB565>(*reinterpret_cast<const SL_ColorRGB565*>(mTexture)); break;
        case SL_COLOR_RGBA_5551:    clear_texture<SL_ColorRGB5551>(*reinterpret_cast<const SL_ColorRGB5551*>(mTexture)); break;
        case SL_COLOR_RGBA_4444:    clear_texture<SL_ColorRGB4444>(*reinterpret_cast<const SL_ColorRGB4444*>(mTexture)); break;
        case SL_COLOR_RGBA_1010102: clear_texture<SL_ColorRGB1010102>(*reinterpret_cast<const SL_ColorRGB1010102*>(mTexture)); break;
    }
}
