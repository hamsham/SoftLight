
#include "lightsky/math/scalar_utils.h"

#include "softlight/SL_BlitProcesor.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions and namespaces
-----------------------------------------------------------------------------*/
namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SL_BlitProcessor Class
-----------------------------------------------------------------------------*/
#if !defined(LS_COMPILER_MSC)
    template void SL_BlitProcessor::blit_src_r<uint8_t>();
    template void SL_BlitProcessor::blit_src_r<uint16_t>();
    template void SL_BlitProcessor::blit_src_r<uint32_t>();
    template void SL_BlitProcessor::blit_src_r<uint64_t>();
    template void SL_BlitProcessor::blit_src_r<float>();
    template void SL_BlitProcessor::blit_src_r<double>();
    template void SL_BlitProcessor::blit_src_rg<uint8_t>();
    template void SL_BlitProcessor::blit_src_rg<uint16_t>();
    template void SL_BlitProcessor::blit_src_rg<uint32_t>();
    template void SL_BlitProcessor::blit_src_rg<uint64_t>();
    template void SL_BlitProcessor::blit_src_rg<float>();
    template void SL_BlitProcessor::blit_src_rg<double>();
    template void SL_BlitProcessor::blit_src_rgb<uint8_t>();
    template void SL_BlitProcessor::blit_src_rgb<uint16_t>();
    template void SL_BlitProcessor::blit_src_rgb<uint32_t>();
    template void SL_BlitProcessor::blit_src_rgb<uint64_t>();
    template void SL_BlitProcessor::blit_src_rgb<float>();
    template void SL_BlitProcessor::blit_src_rgb<double>();
    template void SL_BlitProcessor::blit_src_rgba<uint8_t>();
    template void SL_BlitProcessor::blit_src_rgba<uint16_t>();
    template void SL_BlitProcessor::blit_src_rgba<uint32_t>();
    template void SL_BlitProcessor::blit_src_rgba<uint64_t>();
    template void SL_BlitProcessor::blit_src_rgba<float>();
    template void SL_BlitProcessor::blit_src_rgba<double>();
#endif



/*-------------------------------------
 * Nearest-neighbor filtering (RGB565)
-------------------------------------*/
template<>
void SL_BlitProcessor::blit_src_rgb<SL_ColorRGB565>() noexcept
{
    switch (mBackBuffer->type())
    {
        case SL_COLOR_R_8U:         blit_nearest<SL_Blit_RGB_to_R<SL_ColorRGB565, uint8_t>>();     break;
        case SL_COLOR_R_16U:        blit_nearest<SL_Blit_RGB_to_R<SL_ColorRGB565, uint16_t>>();    break;
        case SL_COLOR_R_32U:        blit_nearest<SL_Blit_RGB_to_R<SL_ColorRGB565, uint32_t>>();    break;
        case SL_COLOR_R_64U:        blit_nearest<SL_Blit_RGB_to_R<SL_ColorRGB565, uint64_t>>();    break;
        case SL_COLOR_R_FLOAT:      blit_nearest<SL_Blit_RGB_to_R<SL_ColorRGB565, float>>();       break;
        case SL_COLOR_R_DOUBLE:     blit_nearest<SL_Blit_RGB_to_R<SL_ColorRGB565, double>>();      break;

        case SL_COLOR_RG_8U:        blit_nearest<SL_Blit_RGB_to_RG<SL_ColorRGB565, uint8_t>>();    break;
        case SL_COLOR_RG_16U:       blit_nearest<SL_Blit_RGB_to_RG<SL_ColorRGB565, uint16_t>>();   break;
        case SL_COLOR_RG_32U:       blit_nearest<SL_Blit_RGB_to_RG<SL_ColorRGB565, uint32_t>>();   break;
        case SL_COLOR_RG_64U:       blit_nearest<SL_Blit_RGB_to_RG<SL_ColorRGB565, uint64_t>>();   break;
        case SL_COLOR_RG_FLOAT:     blit_nearest<SL_Blit_RGB_to_RG<SL_ColorRGB565, float>>();      break;
        case SL_COLOR_RG_DOUBLE:    blit_nearest<SL_Blit_RGB_to_RG<SL_ColorRGB565, double>>();     break;

        case SL_COLOR_RGB_8U:       blit_nearest<SL_Blit_RGB_to_RGB<SL_ColorRGB565, uint8_t>>();   break;
        case SL_COLOR_RGB_16U:      blit_nearest<SL_Blit_RGB_to_RGB<SL_ColorRGB565, uint16_t>>();  break;
        case SL_COLOR_RGB_32U:      blit_nearest<SL_Blit_RGB_to_RGB<SL_ColorRGB565, uint32_t>>();  break;
        case SL_COLOR_RGB_64U:      blit_nearest<SL_Blit_RGB_to_RGB<SL_ColorRGB565, uint64_t>>();  break;
        case SL_COLOR_RGB_FLOAT:    blit_nearest<SL_Blit_RGB_to_RGB<SL_ColorRGB565, float>>();     break;
        case SL_COLOR_RGB_DOUBLE:   blit_nearest<SL_Blit_RGB_to_RGB<SL_ColorRGB565, double>>();    break;

        case SL_COLOR_RGBA_8U:      blit_nearest<SL_Blit_RGB_to_RGBA<SL_ColorRGB565, uint8_t>>();  break;
        case SL_COLOR_RGBA_16U:     blit_nearest<SL_Blit_RGB_to_RGBA<SL_ColorRGB565, uint16_t>>(); break;
        case SL_COLOR_RGBA_32U:     blit_nearest<SL_Blit_RGB_to_RGBA<SL_ColorRGB565, uint32_t>>(); break;
        case SL_COLOR_RGBA_64U:     blit_nearest<SL_Blit_RGB_to_RGBA<SL_ColorRGB565, uint64_t>>(); break;
        case SL_COLOR_RGBA_FLOAT:   blit_nearest<SL_Blit_RGB_to_RGBA<SL_ColorRGB565, float>>();    break;
        case SL_COLOR_RGBA_DOUBLE:  blit_nearest<SL_Blit_RGB_to_RGBA<SL_ColorRGB565, double>>();   break;

        case SL_COLOR_RGB_565:     blit_nearest<SL_Blit_RGB_to_RGB<SL_ColorRGB565, SL_ColorRGB565>>();  break;
        case SL_COLOR_RGBA_5551:   blit_nearest<SL_Blit_RGB_to_RGBA<SL_ColorRGB565, SL_ColorRGB5551>>(); break;
        case SL_COLOR_RGBA_4444:   blit_nearest<SL_Blit_RGB_to_RGBA<SL_ColorRGB565, SL_ColorRGB4444>>(); break;

        default:
            break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA5551)
-------------------------------------*/
template<>
void SL_BlitProcessor::blit_src_rgba<SL_ColorRGB5551>() noexcept
{
    switch (mBackBuffer->type())
    {
        case SL_COLOR_R_8U:         blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB5551, uint8_t>>();     break;
        case SL_COLOR_R_16U:        blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB5551, uint16_t>>();    break;
        case SL_COLOR_R_32U:        blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB5551, uint32_t>>();    break;
        case SL_COLOR_R_64U:        blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB5551, uint64_t>>();    break;
        case SL_COLOR_R_FLOAT:      blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB5551, float>>();       break;
        case SL_COLOR_R_DOUBLE:     blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB5551, double>>();      break;

        case SL_COLOR_RG_8U:        blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB5551, uint8_t>>();    break;
        case SL_COLOR_RG_16U:       blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB5551, uint16_t>>();   break;
        case SL_COLOR_RG_32U:       blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB5551, uint32_t>>();   break;
        case SL_COLOR_RG_64U:       blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB5551, uint64_t>>();   break;
        case SL_COLOR_RG_FLOAT:     blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB5551, float>>();      break;
        case SL_COLOR_RG_DOUBLE:    blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB5551, double>>();     break;

        case SL_COLOR_RGB_8U:       blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB5551, uint8_t>>();   break;
        case SL_COLOR_RGB_16U:      blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB5551, uint16_t>>();  break;
        case SL_COLOR_RGB_32U:      blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB5551, uint32_t>>();  break;
        case SL_COLOR_RGB_64U:      blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB5551, uint64_t>>();  break;
        case SL_COLOR_RGB_FLOAT:    blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB5551, float>>();     break;
        case SL_COLOR_RGB_DOUBLE:   blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB5551, double>>();    break;

        case SL_COLOR_RGBA_8U:      blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB5551, uint8_t>>();  break;
        case SL_COLOR_RGBA_16U:     blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB5551, uint16_t>>(); break;
        case SL_COLOR_RGBA_32U:     blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB5551, uint32_t>>(); break;
        case SL_COLOR_RGBA_64U:     blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB5551, uint64_t>>(); break;
        case SL_COLOR_RGBA_FLOAT:   blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB5551, float>>();    break;
        case SL_COLOR_RGBA_DOUBLE:  blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB5551, double>>();   break;

        case SL_COLOR_RGB_565:     blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB5551, SL_ColorRGB565>>();  break;
        case SL_COLOR_RGBA_5551:   blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB5551, SL_ColorRGB5551>>(); break;
        case SL_COLOR_RGBA_4444:   blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB5551, SL_ColorRGB4444>>(); break;

        default:
            break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA4444)
-------------------------------------*/
template<>
void SL_BlitProcessor::blit_src_rgba<SL_ColorRGB4444>() noexcept
{
    switch (mBackBuffer->type())
    {
        case SL_COLOR_R_8U:         blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB4444, uint8_t>>();     break;
        case SL_COLOR_R_16U:        blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB4444, uint16_t>>();    break;
        case SL_COLOR_R_32U:        blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB4444, uint32_t>>();    break;
        case SL_COLOR_R_64U:        blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB4444, uint64_t>>();    break;
        case SL_COLOR_R_FLOAT:      blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB4444, float>>();       break;
        case SL_COLOR_R_DOUBLE:     blit_nearest<SL_Blit_RGBA_to_R<SL_ColorRGB4444, double>>();      break;

        case SL_COLOR_RG_8U:        blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB4444, uint8_t>>();    break;
        case SL_COLOR_RG_16U:       blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB4444, uint16_t>>();   break;
        case SL_COLOR_RG_32U:       blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB4444, uint32_t>>();   break;
        case SL_COLOR_RG_64U:       blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB4444, uint64_t>>();   break;
        case SL_COLOR_RG_FLOAT:     blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB4444, float>>();      break;
        case SL_COLOR_RG_DOUBLE:    blit_nearest<SL_Blit_RGBA_to_RG<SL_ColorRGB4444, double>>();     break;

        case SL_COLOR_RGB_8U:       blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB4444, uint8_t>>();   break;
        case SL_COLOR_RGB_16U:      blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB4444, uint16_t>>();  break;
        case SL_COLOR_RGB_32U:      blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB4444, uint32_t>>();  break;
        case SL_COLOR_RGB_64U:      blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB4444, uint64_t>>();  break;
        case SL_COLOR_RGB_FLOAT:    blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB4444, float>>();     break;
        case SL_COLOR_RGB_DOUBLE:   blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB4444, double>>();    break;

        case SL_COLOR_RGBA_8U:      blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB4444, uint8_t>>();  break;
        case SL_COLOR_RGBA_16U:     blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB4444, uint16_t>>(); break;
        case SL_COLOR_RGBA_32U:     blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB4444, uint32_t>>(); break;
        case SL_COLOR_RGBA_64U:     blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB4444, uint64_t>>(); break;
        case SL_COLOR_RGBA_FLOAT:   blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB4444, float>>();    break;
        case SL_COLOR_RGBA_DOUBLE:  blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB4444, double>>();   break;

        case SL_COLOR_RGB_565:     blit_nearest<SL_Blit_RGBA_to_RGB<SL_ColorRGB4444, SL_ColorRGB565>>();  break;
        case SL_COLOR_RGBA_5551:   blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB4444, SL_ColorRGB5551>>(); break;
        case SL_COLOR_RGBA_4444:   blit_nearest<SL_Blit_RGBA_to_RGBA<SL_ColorRGB4444, SL_ColorRGB4444>>(); break;

        default:
            break;
    }
}



/*-------------------------------------
 * Run the texture blitter
-------------------------------------*/
void SL_BlitProcessor::execute() noexcept
{
    switch (mTexture->type())
    {
        case SL_COLOR_R_8U:       blit_src_r<uint8_t>();     break;
        case SL_COLOR_R_16U:      blit_src_r<uint16_t>();    break;
        case SL_COLOR_R_32U:      blit_src_r<uint32_t>();    break;
        case SL_COLOR_R_64U:      blit_src_r<uint64_t>();    break;
        case SL_COLOR_R_FLOAT:    blit_src_r<float>();       break;
        case SL_COLOR_R_DOUBLE:   blit_src_r<double>();      break;

        case SL_COLOR_RG_8U:      blit_src_rg<uint8_t>();    break;
        case SL_COLOR_RG_16U:     blit_src_rg<uint16_t>();   break;
        case SL_COLOR_RG_32U:     blit_src_rg<uint32_t>();   break;
        case SL_COLOR_RG_64U:     blit_src_rg<uint64_t>();   break;
        case SL_COLOR_RG_FLOAT:   blit_src_rg<float>();      break;
        case SL_COLOR_RG_DOUBLE:  blit_src_rg<double>();     break;

        case SL_COLOR_RGB_8U:     blit_src_rgb<uint8_t>();   break;
        case SL_COLOR_RGB_16U:    blit_src_rgb<uint16_t>();  break;
        case SL_COLOR_RGB_32U:    blit_src_rgb<uint32_t>();  break;
        case SL_COLOR_RGB_64U:    blit_src_rgb<uint64_t>();  break;
        case SL_COLOR_RGB_FLOAT:  blit_src_rgb<float>();     break;
        case SL_COLOR_RGB_DOUBLE: blit_src_rgb<double>();    break;

        case SL_COLOR_RGBA_8U:     blit_src_rgba<uint8_t>();  break;
        case SL_COLOR_RGBA_16U:    blit_src_rgba<uint16_t>(); break;
        case SL_COLOR_RGBA_32U:    blit_src_rgba<uint32_t>(); break;
        case SL_COLOR_RGBA_64U:    blit_src_rgba<uint64_t>(); break;
        case SL_COLOR_RGBA_FLOAT:  blit_src_rgba<float>();    break;
        case SL_COLOR_RGBA_DOUBLE: blit_src_rgba<double>();   break;

        case SL_COLOR_RGB_565:     blit_src_rgb<SL_ColorRGB565>();   break;
        case SL_COLOR_RGBA_5551:   blit_src_rgba<SL_ColorRGB5551>(); break;
        case SL_COLOR_RGBA_4444:   blit_src_rgba<SL_ColorRGB4444>(); break;
    }
}
