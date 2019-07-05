
#include <iostream>
#include <iomanip>
#include <memory> // std::move()

#include "lightsky/utils/Pointer.h"

#include "soft_render/SR_Animation.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_SceneGraph.hpp"
#include "soft_render/SR_Transform.hpp"



namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SR_SceneGraph> load_scene(const std::string& fileName)
{
    int retCode = 0;
    (void)retCode;

    SR_SceneFileLoader meshLoader;
    utils::Pointer<SR_SceneGraph> pGraph{new SR_SceneGraph{}};

    retCode = meshLoader.load(fileName);
    assert(retCode != 0);

    retCode = pGraph->import(meshLoader.data());
    assert(retCode == 0);

    return pGraph;
}



/*-----------------------------------------------------------------------------
 * print scene graph
-----------------------------------------------------------------------------*/
void print_scene_info(const utils::Pointer<SR_SceneGraph>& pGraph) noexcept
{
    std::cout << "-------------------------------------------------------------------------------" << std::endl;

    for (const SR_SceneNode& n : pGraph->mNodes)
    {
        const size_t nodeId = n.nodeId;
        const SR_Transform& transform = pGraph->mCurrentTransforms[nodeId];
        const size_t parentId = transform.mParentId;
        size_t numTabs = 0;

        // Get the number of spaces which must be printed to represent the
        // scene hierarchy
        for (size_t p = parentId; p != SCENE_NODE_ROOT_ID; ++numTabs)
        {
            p = pGraph->mCurrentTransforms[p].mParentId;
        }

        std::ios initialFmt{nullptr};
        initialFmt.copyfmt(std::cout);

        std::cout << std::left << std::setw(20) << nodeId << ' ' << std::left << std::setw(20) << parentId;
        std::cout.copyfmt(initialFmt);
        std::cout << ": ";

        switch (n.type)
        {
            case NODE_TYPE_EMPTY:
                std::cout << "Empty  ";
                break;

            case NODE_TYPE_MESH:
                std::cout << "Mesh   ";
                break;

            case NODE_TYPE_CAMERA:
                std::cout << "Camera ";
                break;

            case NODE_TYPE_BONE:
                std::cout << "Bone   ";
                break;
        }

        for (size_t i = 0; i < numTabs; ++i)
        {
            std::cout << '-';
        }

        std::cout << ' ' << pGraph->mNodeNames[nodeId] << std::endl;
    }

    std::cout << std::endl;

    // Animations need love too
    for (size_t animId = 0; animId < pGraph->mAnimations.size(); ++animId)
    {
        const SR_Animation& anim = pGraph->mAnimations[animId];

        std::cout
            << "Animation "       << animId
            << "\n\tId:         " << anim.id()
            << "\n\tName:       " << anim.name()
            << "\n\tDuration:   " << (anim.duration()/anim.ticks_per_sec()) << " seconds."
            << "\n\tMonotonic:  " << anim.have_monotonic_transforms()
            << "\n\tTransforms: ";

        for (size_t i = 0; i < anim.size(); ++i)
        {
            const size_t transformId = anim.transforms()[i];

            if (i < anim.size()-1)
            {
                std::cout << transformId << ", ";
            }
            else
            {
                std::cout << transformId << std::endl;
            }
        }
    }

    std::cout << "-------------------------------------------------------------------------------" << std::endl;
}


/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    std::string sceneFile;
    utils::Pointer<SR_SceneGraph> pGraph0, pGraph1;

    sceneFile = "testdata/bob/Bob.md5mesh";
    pGraph0 = std::move(load_scene(sceneFile));
    print_scene_info(pGraph0);

    sceneFile = "testdata/rover/testmesh.dae";
    pGraph1 = std::move(load_scene(sceneFile));
    pGraph0->import(*pGraph1);
    print_scene_info(pGraph0);

    pGraph0->reparent_node(36, 1);
    print_scene_info(pGraph0);

    pGraph0->reparent_node(2, SCENE_NODE_ROOT_ID);
    print_scene_info(pGraph0);

    return 0;
}
