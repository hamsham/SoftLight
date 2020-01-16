
#include <iostream>

#include "soft_render/SR_FragmentProcessor.hpp"



int main()
{
    int numThreads = 7;
    int numScanlines = 33;
    for (int scanline = 0; scanline < numScanlines; ++scanline)
    {
        std::cout << "Scanline " << scanline << ':';
        for (int i = 0; i < numThreads; ++i)
        {
            std::cout << "\n\t" << scanline + sr_scanline_offset<int>(numThreads, i, scanline) << " --- " << sr_scanline_offset<int>(numThreads, i, scanline);
        }
        std::cout << '\n' << std::endl;
    }
    return 0;
}