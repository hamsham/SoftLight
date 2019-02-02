
#include <cstdint>
#include <iostream>
#include <iomanip>

#ifndef SR_TEXTURE_Z_ORDERING
    #define SR_TEXTURE_Z_ORDERING
#endif
#include "soft_render/SR_Texture.hpp"



int main()
{
    constexpr unsigned w = 8;
    constexpr unsigned h = 16;
    constexpr unsigned d = 16;

    SR_Texture tex;
    tex.init(SR_COLOR_R_8U, w, h, d);

    for (unsigned z = 0; z < d; ++z)
    {
        for (unsigned y = 0; y < h; ++y)
        {
            for (unsigned x = 0; x < w; ++x)
            {
                std::cout << std::setw(4) << tex.map_coordinate(x, y, z) << ' ';
            }

            std::cout << std::endl;
        }

        std::cout << "---------------------------------------" << std::endl;
    }

    tex.terminate();

    return 0;
}

