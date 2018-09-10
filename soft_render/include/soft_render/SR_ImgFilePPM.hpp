
#ifndef SR_IMAGE_FILE_PPM_HPP
#define SR_IMAGE_FILE_PPM_HPP

#include <fstream>

#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_Setup.hpp"



/*------------------------------------------------------------------------------
 * Save Images
------------------------------------------------------------------------------*/
int sr_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SR_ColorRGB8* const colors, const char* const pFilename);



/*------------------------------------------------------------------------------
 * Save R Images
------------------------------------------------------------------------------*/
template <typename colorType>
int sr_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SR_ColorRType<colorType>* const colors, const char* const pFilename)
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
        for(coord_shrt_t j = 0; j < w; ++j)
        {
            const SR_ColorR8&& c = color_cast<uint8_t, colorType>(colors[w * i + j]);
            const SR_ColorRGB8 temp{c.r, c.r, c.r};
            f.write(reinterpret_cast<const char*>(&temp), sizeof(SR_ColorRGB8));
        }
    }

    f.close();

    return 0;
}



/*------------------------------------------------------------------------------
 * Save RG Images
------------------------------------------------------------------------------*/
template <typename colorType>
int sr_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SR_ColorRGType<colorType>* const colors, const char* const pFilename)
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
        for(coord_shrt_t j = 0; j < w; ++j)
        {
            const SR_ColorRG8&& c = color_cast<uint8_t, colorType>(colors[w * i + j]);
            const SR_ColorRGB8 temp{c.r, c.g, 0};
            f.write(reinterpret_cast<const char*>(&temp), sizeof(SR_ColorRGB8));
        }
    }

    f.close();

    return 0;
}



/*------------------------------------------------------------------------------
 * Save RGB Images
------------------------------------------------------------------------------*/
template <typename colorType>
int sr_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SR_ColorRGBType<colorType>* const colors, const char* const pFilename)
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
        for(coord_shrt_t j = 0; j < w; ++j)
        {
            const SR_ColorRGB8&& c = color_cast<uint8_t, colorType>(colors[w * i + j]);
            f.write(reinterpret_cast<const char*>(&c), sizeof(SR_ColorRGB8));
        }
    }

    f.close();

    return 0;
}



/*------------------------------------------------------------------------------
 * Save RGB Images
------------------------------------------------------------------------------*/
template <typename colorType>
int sr_img_save_ppm(const coord_shrt_t w, const coord_shrt_t h, const SR_ColorRGBAType<colorType>* const colors, const char* const pFilename)
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
        for(coord_shrt_t j = 0; j < w; ++j)
        {
            const SR_ColorRGBA8&& c = color_cast<uint8_t, colorType>(colors[w * i + j]);
            f.write(reinterpret_cast<const char*>(&c), sizeof(SR_ColorRGB8));
        }
    }

    f.close();

    return 0;
}



/*------------------------------------------------------------------------------
 * Load Images
------------------------------------------------------------------------------*/
SR_ColorRGB8* sr_img_load_ppm(coord_shrt_t& w, coord_shrt_t& h, const char* const pFilename);



#endif /* SR_IMAGE_FILE_PPM_HPP */

