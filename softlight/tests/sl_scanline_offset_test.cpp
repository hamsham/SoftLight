
#include <iostream>

#include "softlight/SL_ScanlineBounds.hpp" // sl_scanline_offset()



int main()
{
    int numThreads = 7;
    int numScanlines = 33;
    for (int scanline = 0; scanline < numScanlines; ++scanline)
    {
        std::cout << "Scanline " << scanline << ':';
        for (int i = 0; i < numThreads; ++i)
        {
            int threadForScanline = sl_scanline_offset<int>(numThreads, i, scanline);
            std::cout << "\n\t" << scanline+i << " --- " << threadForScanline;
        }
        std::cout << '\n' << std::endl;
    }
    return 0;
}