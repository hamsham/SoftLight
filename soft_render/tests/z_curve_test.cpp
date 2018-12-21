
#include <cstdint>
#include <iostream>
#include <iomanip>

#ifndef SR_TEXTURE_Z_ORDERING
    #define SR_TEXTURE_Z_ORDERING
#endif
#include "soft_render/SR_Texture.hpp"



int main()
{
    constexpr unsigned w = 24;
    constexpr unsigned h = 16;

    SR_Texture tex;
    tex.init(SR_COLOR_R_8U, w, h);

    for (unsigned y = 0; y < h; ++y)
    {
        for (unsigned x = 0; x < w; ++x)
        {
            std::cout << std::setw(4) << tex.map_coordinate(x, y) << ' ';
        }

        std::cout << std::endl;
    }

    tex.terminate();

    return 0;
}

