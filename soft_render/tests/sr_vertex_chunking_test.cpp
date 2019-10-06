
#include <iostream>

#include "soft_render/SR_ShaderProcessor.hpp"

int main()
{
    constexpr size_t vertsPerPrim = 3;
    size_t totalVerts = 354u;
    size_t numThreads = 14u;
    size_t start;
    size_t end;

    for (size_t i = 0; i < numThreads; ++i)
    {
        std::cout << i << ": " << std::endl;

        sr_calc_indexed_parition<vertsPerPrim, false>(totalVerts, numThreads, i, start, end);
        
        for (size_t j = 0; start < end; ++start, ++j)
        {
            std::cout << "\t" << j << ": " << start << std::endl;
        }
    }

    sr_calc_indexed_parition<vertsPerPrim, false>(totalVerts, numThreads, 0, start, end);
    size_t stdChunkSize = end-start;

    std::cout << "Total Threads:  " << numThreads   << std::endl;
    std::cout << "Array Size:     " << totalVerts   << std::endl;
    std::cout << "Std Chunk Size: " << stdChunkSize << std::endl;

    return 0;
}
