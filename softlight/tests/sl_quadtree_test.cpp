
#include <iostream>

#include "softlight/SL_Quadtree.hpp"


int main()
{
    typedef SL_Quadtree<int, 16> Quadtree;
    Quadtree quadtree{ls::math::vec2{0.f, 0.f}, 512.f};

    // insert the world node
    quadtree.insert(ls::math::vec2{0.f, 0.f}, 512.f, 0);

    quadtree.insert(ls::math::vec2{-25.f,   3.f},  3.f, 1);
    quadtree.insert(ls::math::vec2{ 25.f,   3.f},  2.f, 2);
    quadtree.insert(ls::math::vec2{-6.f,   -64.f}, 3.f, 3);
    quadtree.insert(ls::math::vec2{ 9.f,    426.f},  5.f, 4);
    quadtree.insert(ls::math::vec2{-100.f, -129.f},  3.f, 5);
    quadtree.insert(ls::math::vec2{-6.f,   -37.f},  1.f, 6);
    quadtree.insert(ls::math::vec2{-52.f,   3.f},  3.f, 7);
    quadtree.insert(ls::math::vec2{-25.f,   4.f},  1.f, 8);

    std::cout
        << "\nTree breadth: " << quadtree.breadth()
        << "\nTree depth: " << quadtree.depth()
        << '\n'
        << std::endl;

    const  Quadtree* const* subNodes = quadtree.sub_nodes();
    size_t numSubNodes = 0;
    for (size_t i = 0; i < 8; ++i)
    {
        numSubNodes += subNodes[i] != nullptr;
    }
    std::cout << "Found " << numSubNodes << " top-level sub-nodes." << std::endl;

    const ls::math::vec2 subTreePos{-4.f, -36.f};
    SL_Quadtree<int, 16>* pSubtree = quadtree.find(subTreePos);
    std::cout
        << "Found sub-tree:"
        << "\n\tLocation: " << subTreePos[0] << ',' << subTreePos[1]
        << "\n\tDepth:    " << pSubtree->depth()
        << "\n\tElements: " << pSubtree->size()
        << std::endl;

    for (int data : pSubtree->data())
    {
        std::cout << "\t" << data << std::endl;
    }

    std::cout << "\nIterating: " << std::endl;

    quadtree.iterate_bottom_up([](const Quadtree* pTree, size_t depth)->bool {
        const ls::math::vec2& pos = pTree->origin();
        bool amPositive = 0x07 != (0x07 & ls::math::sign_mask(pos));

        if (!amPositive)
        {
            return false;
        }

        if (pTree->size())
        {
            std::cout << "\tFound objects at depth " << depth << " with position: (" << pos[0] << ',' << pos[1] << ')' << std::endl;
        }

        for (int data : pTree->data())
        {
            std::cout << "\t\tObject: " << data << std::endl;
        }

        return true;
    });

    return 0;
}
