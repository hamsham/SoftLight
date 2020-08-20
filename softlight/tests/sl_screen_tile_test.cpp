
#include <iostream>

#include "softlight/SL_ShaderUtil.hpp" // sl_subdivide_region()


int main()
{
    constexpr unsigned numThreads = 33;
    constexpr unsigned fboWidth = 1280;
    constexpr unsigned fboHeight = 768;

    for (unsigned i = 1; i <= numThreads; ++i)
    {
        unsigned w, h;
        sl_calc_frag_tiles<unsigned>(i, w, h);
        std::cout << i << " threads " << " can be placed into a " << w << 'x' << h << " grid:";

        for (unsigned j = 0; j < numThreads; ++j)
        {
            const ls::math::vec4_t<unsigned> dimens = sl_subdivide_region(fboWidth, fboHeight, numThreads, j);
            std::cout
                << "\n\tRegion " << j << ':'
                << "\n\t\tX0: " << dimens[0]
                << "\n\t\tX1: " << dimens[1]
                << "\n\t\tY0: " << dimens[2]
                << "\n\t\tY1: " << dimens[3]
                << std::endl;
        }

        std::cout << std::endl;
    }

    return 0;
}

