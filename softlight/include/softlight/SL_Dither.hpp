//
// Created by hammy on 2022-03-17.
//

#ifndef SOFTLIGHT_DITHER_HPP
#define SOFTLIGHT_DITHER_HPP

#include "lightsky/setup/Api.h"

#include "lightsky/math/scalar_utils.h"



/*-----------------------------------------------------------------------------
 * 2x2 Ordered Dithering
-----------------------------------------------------------------------------*/
inline LS_INLINE float sl_bayer_dither_2x2(const float color, const unsigned x, const unsigned y)
{
    constexpr unsigned bayerMatrix16[4] = {
        0, 2,
        3, 1
    };
    return ls::math::step(color, (float)bayerMatrix16[(x % 2u) + (y % 2u) * 2u] / 4.f);
}



template <unsigned numBits = 8>
inline float sl_dither2(float c, const unsigned x, const unsigned y)
{
    const float paletteColor = c * (float)((1u << numBits)-1u);
    const float bayer = paletteColor + sl_bayer_dither_2x2(c, x, y);
    return ls::math::clamp(bayer/(float)((1u << numBits)-1u), 0.f, 1.f);
}



template <>
inline float sl_dither2<1u>(float c, const unsigned x, const unsigned y)
{
    return 1.f-sl_bayer_dither_2x2(c, x, y);
}



/*-----------------------------------------------------------------------------
 * 4x4 Ordered Dithering
-----------------------------------------------------------------------------*/
inline LS_INLINE float sl_bayer_dither_4x4(const float color, const unsigned x, const unsigned y)
{
    constexpr unsigned bayerMatrix16[16] = {
        0,  8,  2,  10,
        12, 4,  14, 6,
        3,  11, 1,  9,
        15, 7,  13, 5
    };
    return ls::math::step(color, (float)bayerMatrix16[(x % 4u) + (y % 4u) * 4u] / 16.f);
}



template <unsigned numBits = 8>
inline float sl_dither4(float c, const unsigned x, const unsigned y)
{
    const float paletteColor = c * (float)((1u << numBits)-1u);
    const float bayer = paletteColor + sl_bayer_dither_4x4(c, x, y);
    return ls::math::clamp(bayer/(float)((1u << numBits)-1u), 0.f, 1.f);
}



template <>
inline float sl_dither4<1u>(float c, const unsigned x, const unsigned y)
{
    return 1.f-sl_bayer_dither_4x4(c, x, y);
}



/*-----------------------------------------------------------------------------
 * 8x8 Ordered Dithering
-----------------------------------------------------------------------------*/
inline LS_INLINE float sl_bayer_dither_8x8(const float color, const unsigned x, const unsigned y)
{
    constexpr unsigned bayerMatrix16[64] = {
        0,  32, 8,  40, 2,  34, 10, 42,
        48, 16, 56, 24, 50, 18, 58, 26,
        12, 44, 4,  36, 14, 46, 6,  38,
        60, 28, 52, 20, 62, 30, 54, 22,
        3,  35, 11, 43, 1,  33, 9,  41,
        51, 19, 59, 27, 49, 17, 57, 25,
        15, 47, 7,  39, 13, 45, 5,  37,
        63, 31, 55, 23, 61, 29, 53, 21
    };
    return ls::math::step(color, (float)bayerMatrix16[(x % 8u) + (y % 8u) * 8u] / 64.f);
}



template <unsigned numBits = 8>
inline float sl_dither8(float c, const unsigned x, const unsigned y)
{
    const float paletteColor = c * (float)((1u << numBits)-1u);
    const float bayer = paletteColor + sl_bayer_dither_8x8(c, x, y);
    return ls::math::clamp(bayer/(float)((1u << numBits)-1u), 0.f, 1.f);
}



template <>
inline float sl_dither8<1u>(float c, const unsigned x, const unsigned y)
{
    return 1.f-sl_bayer_dither_8x8(c, x, y);
}



#endif /* SOFTLIGHT_DITHER_HPP */
