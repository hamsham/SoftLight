
#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/fixed.h"

#include "soft_render/SR_BlitProcesor.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions and namespaces
-----------------------------------------------------------------------------*/
namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SR_BlitProcessor Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * External function declarations to keep compile times short
-------------------------------------*/
/*
template void SR_BlitProcessor::blit_nearest_r<uint8_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_r<uint16_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_r<uint32_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_r<uint64_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_r<float>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_r<double>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);

template void SR_BlitProcessor::blit_nearest_rg<uint8_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rg<uint16_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rg<uint32_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rg<uint64_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rg<float>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rg<double>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);

template void SR_BlitProcessor::blit_nearest_rgb<uint8_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgb<uint16_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgb<uint32_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgb<uint64_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgb<float>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgb<double>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);

template void SR_BlitProcessor::blit_nearest_rgba<uint8_t>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgba<uint16_t>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgba<uint32_t>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgba<uint64_t>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgba<float>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
template void SR_BlitProcessor::blit_nearest_rgba<double>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
*/



/*-------------------------------------
 * Run the texture blitter
-------------------------------------*/
void SR_BlitProcessor::execute() noexcept
{
    // calculate the bounds of the tile which a certain thread will be
    // responsible for
    const uint_fast32_t bufferW = mBackBuffer->width();
    const uint_fast32_t bufferH = mBackBuffer->height();
    const uint_fast32_t texW    = mTexture->width();
    const uint_fast32_t texH    = mTexture->height();
    SR_ColorRGBA8*      pDest   = mBackBuffer->buffer();

    switch (mTexture->type())
    {
        case SR_COLOR_R_8U:        blit_nearest_r<uint8_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH);  break;
        case SR_COLOR_R_16U:       blit_nearest_r<uint16_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_R_32U:       blit_nearest_r<uint32_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_R_64U:       blit_nearest_r<uint64_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_R_FLOAT:     blit_nearest_r<float>((unsigned char*)pDest, texW, texH, bufferW, bufferH);    break;
        case SR_COLOR_R_DOUBLE:    blit_nearest_r<double>((unsigned char*)pDest, texW, texH, bufferW, bufferH);   break;

        case SR_COLOR_RG_8U:       blit_nearest_rg<uint8_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH);  break;
        case SR_COLOR_RG_16U:      blit_nearest_rg<uint16_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RG_32U:      blit_nearest_rg<uint32_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RG_64U:      blit_nearest_rg<uint64_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RG_FLOAT:    blit_nearest_rg<float>((unsigned char*)pDest, texW, texH, bufferW, bufferH);    break;
        case SR_COLOR_RG_DOUBLE:   blit_nearest_rg<double>((unsigned char*)pDest, texW, texH, bufferW, bufferH);   break;

        case SR_COLOR_RGB_8U:      blit_nearest_rgb<uint8_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH);  break;
        case SR_COLOR_RGB_16U:     blit_nearest_rgb<uint16_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RGB_32U:     blit_nearest_rgb<uint32_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RGB_64U:     blit_nearest_rgb<uint64_t>((unsigned char*)pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RGB_FLOAT:   blit_nearest_rgb<float>((unsigned char*)pDest, texW, texH, bufferW, bufferH);    break;
        case SR_COLOR_RGB_DOUBLE:  blit_nearest_rgb<double>((unsigned char*)pDest, texW, texH, bufferW, bufferH);   break;

        case SR_COLOR_RGBA_8U:     blit_nearest_rgba<uint8_t>(pDest, texW, texH, bufferW, bufferH);  break;
        case SR_COLOR_RGBA_16U:    blit_nearest_rgba<uint16_t>(pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RGBA_32U:    blit_nearest_rgba<uint32_t>(pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RGBA_64U:    blit_nearest_rgba<uint64_t>(pDest, texW, texH, bufferW, bufferH); break;
        case SR_COLOR_RGBA_FLOAT:  blit_nearest_rgba<float>(pDest, texW, texH, bufferW, bufferH);    break;
        case SR_COLOR_RGBA_DOUBLE: blit_nearest_rgba<double>(pDest, texW, texH, bufferW, bufferH);   break;
    }
}
