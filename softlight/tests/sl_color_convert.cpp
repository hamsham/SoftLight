

#include <iostream>

#include "lightsky/math/half.h"
#include "softlight/SL_Color.hpp"
#include "softlight/SL_ColorHSX.hpp"
#include "softlight/SL_ColorYCoCg.hpp"



template <typename float_type>
void run_tests() noexcept
{
    SL_ColorRGBType<uint8_t> c1 = {10, 93, 173};
    std::cout << (unsigned)c1[0]  << ", " << (unsigned)c1[1]  << ", " << (unsigned)c1[2] << std::endl;

    SL_ColorRGBType<uint16_t> c2 = color_cast<uint16_t, uint8_t>(c1);
    std::cout << "RGB8 to RBG16: " << c2[0]  << ", " << c2[1]  << ", " << c2[2] << std::endl;

    SL_ColorTypeHSV<float_type> c3 = hsv_cast<float_type>(color_cast<float_type, uint16_t>(c2));
    std::cout << "RBG16 to HSVf: "  << c3.h  << ", " << c3.s  << ", " << c3.v << std::endl;

    SL_ColorTypeHSL<float_type> c4 = hsl_cast<float_type>(color_cast<float_type, uint8_t>(c1));
    std::cout << "RGB8 to HSL: "  << c4.h  << ", " << c4.s  << ", " << c4.l << std::endl;

    SL_ColorRGBType<float_type> c5 = rgb_cast<float_type>(hsv_cast<float_type>(hsl_cast<float_type>(c3)));
    std::cout << "HSLf to RBGf: "  << c5[0]  << ", " << c5[1]  << ", " << c5[2] << std::endl;

    c1 = color_cast<uint8_t, uint16_t>(c2);
    std::cout << "RBG16 to RGB8: "  << (unsigned)c1[0]  << ", " << (unsigned)c1[1]  << ", " << (unsigned)c1[2] << std::endl;

    SL_ColorYCoCg8u c6 = ycocg_cast<uint8_t>(c1);
    std::cout << "RGB8 to YCoCg: "  << (unsigned)c6.y  << ", " << (unsigned)c6.co  << ", " << (unsigned)c6.cg << std::endl;

    c1 = rgb_cast<uint8_t>(c6);
    std::cout << "YCoCg to RGB8: "  << (unsigned)c1[0]  << ", " << (unsigned)c1[1]  << ", " << (unsigned)c1[2] << std::endl;

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

