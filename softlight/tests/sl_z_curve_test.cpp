
#include <cstdint>
#include <iostream>
#include <iomanip>

#include "softlight/SL_Texture.hpp"



int main()
{
    constexpr unsigned w = 8;
    constexpr unsigned h = 16;
    constexpr unsigned d = 16;

    SL_Texture tex;
    tex.init(SL_COLOR_R_8U, w, h, d);

    for (unsigned z = 0; z < d; ++z)
    {
        for (unsigned y = 0; y < h; ++y)
        {
            for (unsigned x = 0; x < w; ++x)
            {
                std::cout << std::setw(4) << tex.map_coordinate<SL_TexelOrder::SL_TEXELS_SWIZZLED>(x, y, z) << ' ';
            }

            std::cout << std::endl;
        }

        std::cout << "---------------------------------------" << std::endl;
    }

    tex.terminate();

    return 0;
}

