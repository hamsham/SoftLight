
#include <iostream>
#include <string>

#include "softlight/SL_SpatialHierarchy.hpp"

typedef SL_SpatialHierarchy<std::string> SceneGraphType;
template class SL_SpatialHierarchy<std::string>;



std::ostream& operator << (std::ostream& ostr, const SceneGraphType& sceneGraph)
{
    for (size_t i = 0; i < sceneGraph.size(); ++i)
    {
        size_t parentId = sceneGraph.parent(i);
        const std::string& value = sceneGraph[i];
        size_t numTabs = 0;

        // Get the number of spaces which must be printed to represent the
        // scene hierarchy
        for (size_t p = parentId; p != SceneGraphType::ROOT_NODE_INDEX; ++numTabs)
        {
            p = sceneGraph.parent(p);
        }

        ostr << i << '-';
        for (size_t t = 0; t < numTabs; ++t)
        {
            std::cout << '-';
        }

        std::cout << parentId << ": " << value;

        if (i < sceneGraph.size()-1)
        {
            ostr << '\n';
        }
    }

    return ostr;
}



int main()
{
    SceneGraphType sceneGraph;
    sceneGraph.insert(SceneGraphType::ROOT_NODE_INDEX, "a");
    sceneGraph.insert(SceneGraphType::ROOT_NODE_INDEX, "b");
    sceneGraph.insert(SceneGraphType::ROOT_NODE_INDEX, "c");
    sceneGraph.insert(SceneGraphType::ROOT_NODE_INDEX, "d");
    sceneGraph.insert(SceneGraphType::ROOT_NODE_INDEX, "e");
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.insert(3, "d/0");
    sceneGraph.insert(2, "c/0");
    sceneGraph.insert(4, "d/1");
    sceneGraph.insert(7, "e/0");
    sceneGraph.insert(8, "e/1");
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.erase(4);
    std::cout << sceneGraph << '\n' << std::endl;

    /*
    sceneGraph.reparent(3, SceneGraphType::ROOT_NODE_INDEX);
    std::cout << sceneGraph << '\n' << std::endl;
    */

    sceneGraph.reparent(1, 2);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.reparent(3, SceneGraphType::ROOT_NODE_INDEX);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.reparent(4, 2);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.reparent(3, 5);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.reparent(1, 3);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.reparent(6, 2);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.duplicate(2);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.reparent(2, 0);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.reparent(0, 9);
    std::cout << sceneGraph << '\n' << std::endl;

    sceneGraph.reparent(6, SceneGraphType::ROOT_NODE_INDEX);
    std::cout << sceneGraph << '\n' << std::endl;

    return 0;
}
