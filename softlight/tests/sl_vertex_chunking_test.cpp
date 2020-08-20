
#include <cassert>
#include <iostream>

#include "softlight/SL_ShaderUtil.hpp" // sl_calc_indexed_parition()

int main()
{
    constexpr size_t vertsPerPrim = 3;
    constexpr size_t totalVerts = 66u;
    constexpr size_t numThreads = 5u;
    constexpr bool lastThreadHasLess = true;
    size_t start = 0;
    size_t end = 0;
    size_t stdChunkSize = 0;
    size_t endChunkSize = 0;

    static_assert(totalVerts % vertsPerPrim == 0, "Vertices per primitive is not a multiple of the total vertex count.");

    for (size_t i = 0; i < numThreads; ++i)
    {
        sl_calc_indexed_parition<vertsPerPrim, lastThreadHasLess>(totalVerts, numThreads, i, start, end);
        endChunkSize = (end == start) ? endChunkSize : end-start;

        std::cout << i << ": " << start << '-' << end << std::endl;

        for (size_t j = 0; start < end; ++start, ++j)
        {
            std::cout << "\t" << j << ": " << start << std::endl;
        }
    }

    sl_calc_indexed_parition<vertsPerPrim, lastThreadHasLess>(totalVerts, numThreads, 0, start, end);
    stdChunkSize = end;

    std::cout << "Total Threads:  " << numThreads   << std::endl;
    std::cout << "Array Size:     " << totalVerts   << std::endl;
    std::cout << "Std Chunk Size: " << stdChunkSize << std::endl;
    std::cout << "Std Prim Count: " << stdChunkSize/vertsPerPrim << std::endl;
    std::cout << "End Chunk Size: " << endChunkSize << std::endl;
    std::cout << "End Prim Count: " << endChunkSize/vertsPerPrim << std::endl;

    std::cout << "-------------------------------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < numThreads; ++i)
    {
        sl_calc_indexed_parition2<vertsPerPrim>(totalVerts, numThreads, i, start, end);
        endChunkSize = (end == start) ? endChunkSize : end-start;

        std::cout << i << ": " << start << '-' << end << std::endl;

        for (size_t j = 0; start < end; ++start, ++j)
        {
            std::cout << "\t" << j << ": " << start << std::endl;
        }
    }

    sl_calc_indexed_parition2<vertsPerPrim>(totalVerts, numThreads, 0, start, end);
    stdChunkSize = end;

    std::cout << "Total Threads:  " << numThreads   << std::endl;
    std::cout << "Array Size:     " << totalVerts   << std::endl;
    std::cout << "Std Chunk Size: " << stdChunkSize << std::endl;
    std::cout << "Std Prim Count: " << stdChunkSize/vertsPerPrim << std::endl;
    std::cout << "End Chunk Size: " << endChunkSize << std::endl;
    std::cout << "End Prim Count: " << endChunkSize/vertsPerPrim << std::endl;

    return 0;
}
