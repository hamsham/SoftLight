
#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/fixed.h"

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
    }
}
