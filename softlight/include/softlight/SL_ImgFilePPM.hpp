
#ifndef SL_IMAGE_FILE_PPM_HPP
#define SL_IMAGE_FILE_PPM_HPP

#include <fstream>

#include "softlight/SL_Color.hpp"
#include "softlight/SL_Setup.hpp"



/*------------------------------------------------------------------------------
 * Save Images
------------------------------------------------------------------------------*/
int sl_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SL_ColorRGB8* const colors, const char* const pFilename);



/*------------------------------------------------------------------------------
 * Save R Images
------------------------------------------------------------------------------*/
template <typename colorType>
int sl_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SL_ColorRType<colorType>* const colors, const char* const pFilename)
{
    if (w <= 0)
    {
        return -1;
    }

    if (h <= 0)
    {
        return -2;
    }

    std::ofstream f(pFilename, std::ofstream::out | std::ofstream::binary);
    if (!f.good())
    {
        return -3;
    }

    // Print the header
    // PPM images support color components with values up to 65535. We're
    // simple, so we only use 255 colors per pixel component.
    f << "P6\n" << w << ' ' << h << '\n' << 255 << '\n';

    const size_t numPixels = (size_t)w * (size_t)h;

    // iterate through the image height, then the width
    for (coord_shrt_t i = 0; i < h; ++i)
    {
        for(coord_shrt_t j = 0; j < w; ++j)
        {
            const SL_ColorR8&& c = color_cast<uint8_t, colorType>(colors[numPixels - (w * i + j)]);
            const SL_ColorRGB8 temp{c.r, c.r, c.r};
            f.write(reinterpret_cast<const char*>(&temp), sizeof(SL_ColorRGB8));
        }
    }

    f.close();

    return 0;
}



/*------------------------------------------------------------------------------
 * Save RG Images
------------------------------------------------------------------------------*/
template <typename colorType>
int sl_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SL_ColorRGType<colorType>* const colors, const char* const pFilename)
{
    if (w <= 0)
    {
        return -1;
    }

    if (h <= 0)
    {
        return -2;
    }

    std::ofstream f(pFilename, std::ofstream::out | std::ofstream::binary);
    if (!f.good())
    {
        return -3;
    }

    // Print the header
    // PPM images support color components with values up to 65535. We're
    // simple, so we only use 255 colors per pixel component.
    f << "P6\n" << w << ' ' << h << '\n' << 255 << '\n';

    const size_t numPixels = (size_t)w * (size_t)h;

    // iterate through the image height, then the width
    for (coord_shrt_t i = 0; i < h; ++i)
    {
        coord_shrt_t i2 = h - i - 1;

        for(coord_shrt_t j = 0; j < w; ++j)
        {
            const SL_ColorRG8&& c = color_cast<uint8_t, colorType>(colors[numPixels - (w * i2 + j)]);
            const SL_ColorRGB8 o = {c[2], c[1], c[0]};
            f.write(reinterpret_cast<const char*>(&o), sizeof(SL_ColorRGB8));
        }
    }

    f.close();

    return 0;
}



/*------------------------------------------------------------------------------
 * Save RGB Images
------------------------------------------------------------------------------*/
template <typename colorType>
int sl_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SL_ColorRGBType<colorType>* const colors, const char* const pFilename)
{
    if (w <= 0)
    {
        return -1;
    }

    if (h <= 0)
    {
        return -2;
    }

    std::ofstream f(pFilename, std::ofstream::out | std::ofstream::binary);
    if (!f.good())
    {
        return -3;
    }

    // Print the header
    // PPM images support color components with values up to 65535. We're
    // simple, so we only use 255 colors per pixel component.
    f << "P6\n" << w << ' ' << h << '\n' << 255 << '\n';

    // iterate through the image height, then the width
    for (coord_shrt_t i = 0; i < h; ++i)
    {
        coord_shrt_t i2 = h - i - 1;

        for(coord_shrt_t j = 0; j < w; ++j)
        {
            const SL_ColorRGB8&& c = color_cast<uint8_t, colorType>(colors[w * i2 + j]);
            const SL_ColorRGB8 o = {c[2], c[1], c[0]};
            f.write(reinterpret_cast<const char*>(&o), sizeof(SL_ColorRGB8));
        }
    }

    f.close();

    return 0;
}



/*------------------------------------------------------------------------------
 * Save RGB Images
------------------------------------------------------------------------------*/
template <typename colorType>
int sl_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SL_ColorRGBAType<colorType>* const colors, const char* const pFilename)
{
    if (w <= 0)
    {
        return -1;
    }

    if (h <= 0)
    {
        return -2;
    }

    std::ofstream f(pFilename, std::ofstream::out | std::ofstream::binary);
    if (!f.good())
    {
        return -3;
    }

    // Print the header
    // PPM images support color components with values up to 65535. We're
    // simple, so we only use 255 colors per pixel component.
    f << "P6\n" << w << ' ' << h << '\n' << 255 << '\n';

    // iterate through the image height, then the width
    for (coord_shrt_t i = 0; i < h; ++i)
    {
        coord_shrt_t i2 = h - i - 1;

        for(coord_shrt_t j = 0; j < w; ++j)
        {
            const SL_ColorRGBA8&& c = color_cast<uint8_t, colorType>(colors[w * i2 + j]);
            const SL_ColorRGB8 o = {c[2], c[1], c[0]};
            f.write(reinterpret_cast<const char*>(&o), sizeof(SL_ColorRGB8));
        }
    }

    f.close();

    return 0;
}



/*------------------------------------------------------------------------------
 * Load Images
------------------------------------------------------------------------------*/
SL_ColorRGB8* sl_img_load_ppm(coord_shrt_t& w, coord_shrt_t& h, const char* const pFilename);



#endif /* SL_IMAGE_FILE_PPM_HPP */

