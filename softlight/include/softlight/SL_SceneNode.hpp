
#ifndef SL_SCENE_NODE_HPP
#define SL_SCENE_NODE_HPP

#include <cstdlib> // size_t



/*-----------------------------------------------------------------------------
 * Enumerations
-----------------------------------------------------------------------------*/
enum SL_SceneNodeProp : size_t
{
    SCENE_NODE_ROOT_ID = (size_t)-1
};



enum SL_SceneNodeType : size_t
{
    NODE_TYPE_EMPTY,
    NODE_TYPE_MESH,
    NODE_TYPE_CAMERA,
    NODE_TYPE_BONE
};



/**----------------------------------------------------------------------------
 * A SL_SceneNode represents an atomic object in a visual scene. Scene nodes
 * can be used to render meshes in a scene, transform objects through a
 * hierarchy, assign render properties to a batched draw, and reference a
 * point in 3D space by name.
 * 
 * All properties in a SceneNode reference data in a SceneGraph using integer
 * handles to linearly allocated arrays of data. Keeping only an integer ID
 * helps to ensure that other objects contained within a SceneGraph can be
 * managed separately from the node.
-----------------------------------------------------------------------------*/
struct alignas(sizeof(size_t)) SL_SceneNode final
{
    /**
     * @brief Enumeration containing the type of scene node which *this
     * represents.
     */
    SL_SceneNodeType type;

    /**
     * @brief nodeId contains the index of a node's name, and transform within
     * a SceneGraph.
     * 
     * It is important that the nodeId is always equal to a node's index within
     * its parent scene graph. Animations and transformation updates rely on
     * this correlation for updates.
     * 
     * This member has a 1:1 relationship with the following members of a scene
     * graph:
     *      - bounds
     *      - baseTransforms
     *      - currentTransforms
     *      - modelMatrices
     *      - nodeNames
     */
    size_t nodeId;

    /**
     * @brief The dataId parameter contains the indexed location of data for a
     * SceneNode in a SceneGraph.
     * 
     * For empty transformations, this parameter will have a value of 0.
     * 
     * Mesh nodes will use this parameter as an index to a SceneGraph's
     * "mMeshes" and "mMeshCounts".
     * 
     * Camera Nodes will reference the "mCameras" member of a SceneGraph.
     *
     * Bone nodes can use this to access the "mBones" member of a scene graph.
     */
    size_t dataId;
};



/**------------------------------------
 * @brief sl_reset() assigns a default value of '0' to all internal members.
-------------------------------------*/
void sl_reset(SL_SceneNode& n) noexcept;



#endif /* SL_SCENE_NODE_HPP */
