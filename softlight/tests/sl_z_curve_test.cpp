
#include <cstdint>
#include <iostream>
#include <iomanip>

#include "softlight/SL_Swizzle.hpp"
#include "softlight/SL_Texture.hpp"



int main()
{
    constexpr uint_fast32_t w = 8;
    constexpr uint_fast32_t h = 16;
    constexpr uint_fast32_t d = 8;
    constexpr uint_fast32_t tpc = SL_TexChunkInfo::SL_TEXELS_PER_CHUNK;
    constexpr uint_fast32_t spc = SL_TexChunkInfo::SL_TEXEL_SHIFTS_PER_CHUNK;

    for (uint_fast32_t z = 0; z < d; ++z)
    {
        for (uint_fast32_t y = 0; y < h; ++y)
        {
            for (uint_fast32_t x = 0; x < w; ++x)
            {
                std::cout << std::setw(4) << sl_swizzle_3d_index<tpc, spc>(x, y, z, w, h) << ' ';
            }

            std::cout << std::endl;
        }

        std::cout << "---------------------------------------" << std::endl;
    }

    return 0;
}

