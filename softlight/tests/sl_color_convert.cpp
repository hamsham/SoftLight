

#include <iostream>

#include "lightsky/math/half.h"
#include "softlight/SL_Color.hpp"
#include "softlight/SL_ColorCompressed.hpp"
#include "softlight/SL_ColorHSX.hpp"
#include "softlight/SL_ColorYCoCg.hpp"



template <typename float_type>
void run_tests() noexcept
{
    SL_ColorRGBType<float_type> cf;

    SL_ColorRGBType<uint8_t> c1 = {10, 93, 173};
    std::cout << (unsigned)c1[0]  << ", " << (unsigned)c1[1]  << ", " << (unsigned)c1[2] << std::endl;

    SL_ColorRGBType<uint16_t> c2 = color_cast<uint16_t, uint8_t>(c1);
    std::cout << "RGB8 to RBG16: " << c2[0]  << ", " << c2[1]  << ", " << c2[2] << std::endl;

    {
        SL_ColorTypeHSV<float_type> c3 = hsv_cast<float_type>(color_cast<float_type, uint16_t>(c2));
        std::cout << "RBG16 to HSVf: " << c3.h << ", " << c3.s << ", " << c3.v << std::endl;

        SL_ColorTypeHSL<float_type> c4 = hsl_cast<float_type>(color_cast<float_type, uint8_t>(c1));
        std::cout << "RGB8 to HSL: " << c4.h << ", " << c4.s << ", " << c4.l << std::endl;

        cf = rgb_cast<float_type>(hsv_cast<float_type>(hsl_cast<float_type>(c3)));
        std::cout << "HSLf to RBGf: " << cf[0] << ", " << cf[1] << ", " << cf[2] << std::endl;
    }

    c1 = color_cast<uint8_t, uint16_t>(c2);
    std::cout << "RBG16 to RGB8: "  << (unsigned)c1[0]  << ", " << (unsigned)c1[1]  << ", " << (unsigned)c1[2] << std::endl;

    {
        SL_ColorYCoCg8u c6 = ycocg_cast<uint8_t>(c1);
        std::cout << "RGB8 to YCoCg: " << (unsigned)c6.y << ", " << (unsigned)c6.co << ", " << (unsigned)c6.cg << std::endl;

        c1 = rgb_cast<uint8_t>(c6);
        std::cout << "YCoCg to RGB8: " << (unsigned)c1[0] << ", " << (unsigned)c1[1] << ", " << (unsigned)c1[2] << std::endl;
    }

    {
        SL_ColorRGB565 c565 = rgb_cast<SL_ColorRGB565, float_type>(cf);
        std::cout << "RGBf to RGB565: " << (unsigned)c565.r << ", " << (unsigned)c565.g << ", " << (unsigned)c565.b << std::endl;

        c565 = rgb_cast<SL_ColorRGB565, uint8_t>(c1);
        std::cout << "RGB8 to RGB565: " << (unsigned)c565.r << ", " << (unsigned)c565.g << ", " << (unsigned)c565.b << std::endl;

        cf = rgb_cast<float_type, SL_ColorRGB565>(c565);
        std::cout << "RGB565 to RBGf: " << cf[0] << ", " << cf[1] << ", " << cf[2] << std::endl;

        c1 = rgb_cast<uint8_t, SL_ColorRGB565>(c565);
        std::cout << "RGB565 to RBG8: " << (unsigned)c1[0] << ", " << (unsigned)c1[1] << ", " << (unsigned)c1[2] << std::endl;
    }

    {
        SL_ColorRGB5551 c5551 = rgba_cast<SL_ColorRGB5551, float_type>(ls::math::vec4_cast<float_type>(cf, float_type{1.0}));
        std::cout << "RGBAf to RGB5551: " << (unsigned)c5551.r << ", " << (unsigned)c5551.g << ", " << (unsigned)c5551.b << ", " << (unsigned)c5551.a << std::endl;

        c5551 = rgba_cast<SL_ColorRGB5551, uint8_t>(ls::math::vec4_cast<uint8_t>(c1, 255));
        std::cout << "RGBA8 to RGB5551: " << (unsigned)c5551.r << ", " << (unsigned)c5551.g << ", " << (unsigned)c5551.b << ", " << (unsigned)c5551.a << std::endl;

        SL_ColorRGBAType<float_type> cf2 = rgba_cast<float_type, SL_ColorRGB5551>(c5551);
        std::cout << "RGB5551 to RBGAf: " << cf2[0] << ", " << cf2[1] << ", " << cf2[2] << ", " << cf2[3] << std::endl;

        SL_ColorRGBAType<uint8_t> c8 = rgba_cast<uint8_t, SL_ColorRGB5551>(c5551);
        std::cout << "RGB5551 to RBGA8: " << (unsigned)c8[0] << ", " << (unsigned)c8[1] << ", " << (unsigned)c8[2] << ", " << (unsigned)c8[3] << std::endl;
    }

    {
        SL_ColorRGB4444 c4444 = rgba_cast<SL_ColorRGB4444, float_type>(ls::math::vec4_cast<float_type>(cf, float_type{1.0}));
        std::cout << "RGBAf to RGB4444: " << (unsigned)c4444.r << ", " << (unsigned)c4444.g << ", " << (unsigned)c4444.b << ", " << (unsigned)c4444.a << std::endl;

        c4444 = rgba_cast<SL_ColorRGB4444, uint8_t>(ls::math::vec4_cast<uint8_t>(c1, 255));
        std::cout << "RGBA8 to RGB4444: " << (unsigned)c4444.r << ", " << (unsigned)c4444.g << ", " << (unsigned)c4444.b << ", " << (unsigned)c4444.a << std::endl;

        SL_ColorRGBAType<float_type> cf2 = rgba_cast<float_type, SL_ColorRGB4444>(c4444);
        std::cout << "RGB4444 to RBGAf: " << cf2[0] << ", " << cf2[1] << ", " << cf2[2] << ", " << cf2[3] << std::endl;

        SL_ColorRGBAType<uint8_t> c8 = rgba_cast<uint8_t, SL_ColorRGB4444>(c4444);
        std::cout << "RGB4444 to RBGA8: " << (unsigned)c8[0] << ", " << (unsigned)c8[1] << ", " << (unsigned)c8[2] << ", " << (unsigned)c8[3] << std::endl;
    }

    std::cout << std::endl;
}



/*-----------------------------------------------------------------------------
 * Main (for testing only)
 *
 * g++ -std=c++11 -Wall -Werror -Wextra -pedantic -pedantic-errors SL_ColorType.cpp -o color_cvt
-----------------------------------------------------------------------------*/
int main()
{
    run_tests<ls::math::half>();
    run_tests<float>();
    run_tests<double>();

    return 0;
}

