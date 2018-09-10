

#include <iostream>

#include "soft_render/SR_Color.hpp"



/*-----------------------------------------------------------------------------
 * Main (for testing only)
 *
 * g++ -std=c++11 -Wall -Werror -Wextra -pedantic -pedantic-errors SR_ColorType.cpp -o color_cvt
-----------------------------------------------------------------------------*/
int main()
{
    SR_ColorRGBType<uint8_t> c1 = {10, 93, 173};
    std::cout << (unsigned)c1.r  << ", " << (unsigned)c1.g  << ", " << (unsigned)c1.b << std::endl;

    SR_ColorRGBType<uint16_t> c2 = color_cast<uint16_t, uint8_t>(c1);
    std::cout << "RGB8 to RBG16: " << c2.r  << ", " << c2.g  << ", " << c2.b << std::endl;

    SR_ColorTypeHSV<float> c3 = hsv_cast<float>(c2);
    std::cout << "RBG16 to HSVf: "  << c3.h  << ", " << c3.s  << ", " << c3.v << std::endl;

    SR_ColorTypeHSL<float> c4 = hsl_cast<float>(c1);
    std::cout << "RGB8 to HSL: "  << c4.h  << ", " << c4.s  << ", " << c4.l << std::endl;

    SR_ColorRGBType<float> c5 = rgb_cast<float>(c4);
    std::cout << "HSLf to RBGf: "  << c5.r  << ", " << c5.g  << ", " << c5.b << std::endl;

    c1 = color_cast<uint8_t, float>(c5);
    std::cout << "RBG16 to RGB8: "  << (unsigned)c1.r  << ", " << (unsigned)c1.g  << ", " << (unsigned)c1.b << std::endl;


    return 0;
}

