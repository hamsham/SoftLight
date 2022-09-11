
#ifndef SL_SCENE_GRAPH_HPP
#define SL_SCENE_GRAPH_HPP

#include <vector>

#include "lightsky/utils/AlignedAllocator.hpp"

#include "lightsky/script/Script.h"

#include "softlight/SL_Context.hpp"
#include "softlight/SL_SpatialHierarchy.hpp"
#include "softlight/SL_SceneNode.hpp"
#include "softlight/SL_Setup.hpp"



/*-----------------------------------------------------------------------------
 * Forward declarations
-----------------------------------------------------------------------------*/
namespace ls
{
    namespace math
    {
        template<typename num_t>
        struct mat4_t;
    } // end math namespace
} // end ls namespace



class SL_Animation;
struct SL_AnimationChannel;
class SL_Camera;
struct SL_Mesh;
struct SL_Material;
class SL_Transform;
class SL_BoundingBox;

namespace slscript
{
    template <ls::script::hash_t, typename var_type>
    class SL_SceneGraphScript;

} // end slscript namespace



struct SL_SceneNodeCommonData
{
    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::nodeId" member. Base Transformations are not expected to
     * maintain a reference to their parent transform.
     *
     * This member is unique to all nodes.
     */
    SL_AlignedVector<ls::math::mat4_t<float>> mBaseTransforms;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::nodeId" member. The current transformation for a scene
     * node is expected to keep track of its parent transformation.
     *
     * This member is unique to all nodes.
     */
    SL_AlignedVector<SL_Transform> mCurrentTransforms;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::nodeId" member.
     *
     * This member is unique to all nodes.
     */
    SL_AlignedVector<ls::math::mat4_t<float>> mModelMatrices;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::nodeId" member.
     *
     * This member is unique to all nodes.
     */
    SL_AlignedVector<std::string> mNodeNames;
};



struct SL_SceneGraphMeshData
{
    /**
     * Referenced by mesh-type scene nodes using their
     * "SL_SceneNode::dataId" member.
     *
     * No two nodes should be able to reference the same mesh count index.
     * Doing so will cause a crash when deleting nodes.
     *
     * This member is unique to all mesh nodes.
     */
    SL_AlignedVector<size_t> mNumNodeMeshes;

    /**
     * Referenced by mesh-type scene nodes using their
     * "SL_SceneNode::dataId" member.
     *
     * Some mesh nodes may use more than one mesh. This member, along with the
     * "mNumNodeMeshes" member, will allow several mesh objects or their
     * bounding boxes to be indexed by a single node.
     *
     * This member is unique to all mesh nodes.
     */
    SL_AlignedVector<ls::utils::Pointer<size_t[]>> mNodeMeshes;

    /**
     * Array to contain all meshes indexed by mNodeMeshes. These mesh objects
     * can be re-used by multiple meshes.
     */
    SL_AlignedVector<SL_Mesh> mMeshes;

    /**
     * Shared by all mesh objects using the following relationship:
     *      "this->mMeshes[SL_SceneNode::dataId]->materialId"
     */
    SL_AlignedVector<SL_Material> mMaterials;

    /**
     * Bounding boxes for meshes indexed by mNodeMeshes.
     *
     * This member is unique to all mesh objects in mMeshes.
     */
    SL_AlignedVector<SL_BoundingBox> mMeshBounds;
};



struct SL_SceneNodeCameraData
{
    /**
     * Referenced by camera-type scene nodes using their "SL_SceneNode::dataId"
     * member.
     *
     * No two nodes should be able to reference a single camera. Doing so
     * will cause a crash when deleting nodes.
     *
     * This member is unique to all camera nodes.
     */
    SL_AlignedVector<SL_Camera> mCameras;
};



struct SL_SceneNodeBoneData
{
    /**
     * @brief Referenced by all bone nodes using the "SL_SceneNode::dataId"
     * member. This contains inverse transform matrices.
     *
     * This member is unique to all bone nodes.
     */
    SL_AlignedVector<ls::math::mat4_t<float>> mInvBoneTransforms;

    /**
     * @brief Referenced by all bone nodes using the "SL_SceneNode::dataId"
     * member. This contains offset matrix data.
     *
     * This member is unique to all bone nodes.
     */
    SL_AlignedVector<ls::math::mat4_t<float>> mBoneOffsets;
};



struct SL_SceneNodeAnimData
{
    /**
     * Contains all animations available in the current scene graph.
     *
     * Animations reference only the nodes they modify and are not shared among
     * nodes directly within the scene graph.
     */
    SL_AlignedVector<SL_Animation> mAnimations;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::animTrackId" member.
     */
    SL_AlignedVector<SL_AlignedVector<SL_AnimationChannel>> mNodeAnims;
};

/*
    mContext(),
    mNodeParentIds(),
    mNodes(),
    mNodeNames(),
    mBaseTransforms(),
    mCurrentTransforms(),
    mModelMatrices(),
    mNumNodeMeshes(),
    mNodeMeshes(),
    mMeshes(),
    mMaterials(),
    mMeshBounds(),
    mInvBoneTransforms(),
    mBoneOffsets(),
    mCameras(),
    mAnimations(),
    mNodeAnims()
*/



/**----------------------------------------------------------------------------
 * @brief the SceneGraph object contains all of the data necessary to either
 * instantiate or render SceneNodes in an OpenGL context.
-----------------------------------------------------------------------------*/
class SL_SceneGraph
{
    friend class slscript::SL_SceneGraphScript<LS_SCRIPT_HASH_FUNC("SL_SceneGraph"), SL_SceneGraph*>;

  public: // member objects
    /**
     * @brief Graphical context & resources required for rendering all data in
     * *this.
     */
    SL_Context mContext;

    /**
     * This vector contains the IDs of all parent nodes. It maps 1:1 with all
     * nodes in mNodes and their transformations.
     */
    SL_AlignedVector<std::size_t> mNodeParentIds;

    /**
     * Contains all empty, camera, mesh, and bode nodes in a scene graph.
     *
     * @note Parent nodes must always have a lower array index value than
     * their children. This allows for the transformation update routines
     * to reduce the number of recursive iterations required to update
     * child nodes.
     *
     * Child nodes are always expected to be grouped sequentially after
     * their parent nodes.
     */
    SL_AlignedVector<SL_SceneNode> mNodes;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::nodeId" member.
     *
     * This member is unique to all nodes.
     */
    SL_AlignedVector<std::string> mNodeNames;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::nodeId" member. Base Transformations are not expected to
     * maintain a reference to their parent transform.
     *
     * This member is unique to all nodes.
     */
    SL_AlignedVector<ls::math::mat4_t<float>> mBaseTransforms;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::nodeId" member. The current transformation for a scene
     * node is expected to keep track of its parent transformation.
     *
     * This member is unique to all nodes.
     */
    SL_AlignedVector<SL_Transform> mCurrentTransforms;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::nodeId" member.
     *
     * This member is unique to all nodes.
     */
    SL_AlignedVector<ls::math::mat4_t<float>> mModelMatrices;

    /**
     * Referenced by mesh-type scene nodes using their
     * "SL_SceneNode::dataId" member.
     *
     * No two nodes should be able to reference the same mesh count index.
     * Doing so will cause a crash when deleting nodes.
     *
     * This member is unique to all mesh nodes.
     */
    SL_AlignedVector<size_t> mNumNodeMeshes;

    /**
     * Referenced by mesh-type scene nodes using their
     * "SL_SceneNode::dataId" member.
     *
     * Some scene nodes may use more than one mesh. This member, along with the
     * "mNodeMeshCounts" member, will allow several mesh indices to be
     * referenced by a single node.
     *
     * This member is unique to all mesh nodes.
     */
    SL_AlignedVector<ls::utils::Pointer<size_t[]>> mNodeMeshes;

    /**
     * Array to contain all meshes referenced by mesh node draw commands.
     */
    SL_AlignedVector<SL_Mesh> mMeshes;

    /**
     * Shared by all mesh objects using the following relationship:
     *      "SceneGraph::mMeshes[SL_SceneNode::dataId]->materialId"
     */
    SL_AlignedVector<SL_Material> mMaterials;

    /**
     * Bounding boxes for meshes.
     *
     * This member is unique to all mesh objects.
     */
    SL_AlignedVector<SL_BoundingBox> mMeshBounds;

    /**
     * @brief Indices to the location of skeleton nodes used by skinned meshes.
     * This member essentially associates mesh nodes with bone node
     * transformations. This member should be updated if any node's
     * transformation within the scene graph is added, removed, or reparented.
     *
     * If a mesh is skinned, these indices will point to the first and last
     * bones in a skeleton. Non-skinned meshes will have indices pointing to
     * the root node ID.
     *
     * This member is unique to all mesh objects.
     */
    SL_AlignedVector<SL_SkeletonIndex> mMeshSkeletons;

    /**
     * @brief Referenced by all bone nodes using the "SL_SceneNode::dataId"
     * member. This contains inverse transform matrices.
     *
     * This member is unique to all bone nodes.
     */
    SL_AlignedVector<ls::math::mat4_t<float>> mInvBoneTransforms;

    /**
     * @brief Referenced by all bone nodes using the "SL_SceneNode::dataId"
     * member. This contains offset matrix data.
     *
     * This member is unique to all bone nodes.
     */
    SL_AlignedVector<ls::math::mat4_t<float>> mBoneOffsets;

    /**
     * Referenced by camera-type scene nodes using their
     * "SL_SceneNode::dataId" member.
     *
     * No two nodes should be able to reference a single camera. Doing so
     * will cause a crash when deleting nodes.
     *
     * This member is unique to all camera nodes.
     */
    SL_AlignedVector<SL_Camera> mCameras;

    /**
     * Contains all animations available in the current scene graph.
     *
     * Animations reference only the nodes they modify and are not shared among
     * nodes directly within the scene graph.
     */
    SL_AlignedVector<SL_Animation> mAnimations;

    /**
     * Referenced by all scene node types using their
     * "SL_SceneNode::animTrackId" member.
     */
    SL_AlignedVector<SL_AlignedVector<SL_AnimationChannel>> mNodeAnims;

  private: // member functions
    /**
     * Update the transformation of a single node in the transformation
     * hierarchy.
     *
     * @param transformId
     * An array index which will determine which transform is currently being
     */
    void update_node_transform(const size_t transformId) noexcept;

    /**
     * Remove all data specific to mesh nodes.
     *
     * @param nodeDataId
     * An unsigned integer, containing the data index of a node being
     * deleted.
     */
    void delete_mesh_node_data(const size_t nodeDataId) noexcept;

    /**
     * @brief Remove all data specific to a bone node.
     *
     * @param nodeDataId
     * An unsigned integer, containing the data index of a node being
     * deleted.
     */
    void delete_bone_node_data(const size_t nodeDataId) noexcept;

    /**
     * Remove all data specific to camera nodes.
     *
     * @param nodeDataId
     * An unsigned integer, containing the data index of a node being
     * deleted.
     */
    void delete_camera_node_data(const size_t nodeDataId) noexcept;

    /**
     * Remove all animation data pertaining to the current node.
     *
     * @param nodeId
     * The array index of a node being deleted.
     *
     * @param includeChildren
     * Determine if child animations should also be deleted
     */
    void delete_node_animation_data(const size_t nodeId, bool includeChildren = false) noexcept;

  public: // member functions
    /**
     * @brief Destructor
     *
     * Calls 'terminate()' to delete all CPU and GPU-side resources.
     */
    ~SL_SceneGraph() noexcept;

    /**
     * Constructor
     *
     * Initializes all members in *this to their default values.
     */
    SL_SceneGraph() noexcept;

    /**
     * Copy Constructor
     *
     * @param s
     * A constant reference to another scene graph which will be used to
     * initialize *this.
     *
     * This function will incur a large runtime overhead in order to copy all
     * dynamically allocated resources on both the CPU and GPU.
     */
    SL_SceneGraph(const SL_SceneGraph& s) noexcept;

    /**
     * Move Constructor
     *
     * Initializes *this using data from the input parameter. No copies or data
     * reallocations will be performed.
     *
     * @param s
     * An r-value reference to another scene graph object who's data will be
     * moved into *this.
     */
    SL_SceneGraph(SL_SceneGraph&& s) noexcept;

    /**
     * Copy Operator
     *
     * @param s
     * A constant reference to another scene graph which will be used to
     * initialize *this.
     *
     * This function will incur a large runtime overhead in order to copy all
     * dynamically allocated resources on both the CPU and GPU.
     *
     * @return A reference to *this.
     */
    SL_SceneGraph& operator=(const SL_SceneGraph& s) noexcept;

    /**
     * Move Operator
     *
     * Moves all data from the input parameter into *this. No copies or data
     * reallocations will be performed.
     *
     * @param s
     * An r-value reference to another scene graph object who's data will be
     * moved into *this.
     *
     * @return A reference to *this.
     */
    SL_SceneGraph& operator=(SL_SceneGraph&& s) noexcept;

    /**
     * @brief Terminate A scene graph by cleaning up all CPu and GPU-side
     * resources.
     */
    void terminate() noexcept;

    /**
     * Remove all data related to scene nodes. This includes:
     * cameras, transformations, node names, animations, node meshes.
     *
     * All render data and bounding boxes will remain intact.
     */
    void clear_node_data() noexcept;

    /**
     * Update all scene nodes in *this scene graph.
     *
     * All nodes in the scene graph will have their transformations updated and
     * placed into the modelMatrices array.
     */
    void update() noexcept;

    /**
     * Remove a node from the scene graph.
     *
     * This function will remove all children related to the current node.
     *
     * @param nodeIndex
     * An unsigned integral type, containing the array-index of the node to
     * remove from the graph.
     *
     * @return The total number of nodes which were deleted.
     */
    size_t delete_node(const size_t nodeIndex) noexcept;

    /**
     * Reassign a node to a different parent.
     *
     * This method will move a node and all of its children. Large node
     * hierarchies will cause a large reallocation of the internal node and
     * transform arrays.
     *
     * @param nodeIndex
     * An unsigned integral type, containing the array-index of the node to
     * re-parent within the graph.
     *
     * @param parentIndex
     * An unsigned integral type, containing the array-index of the node to
     * serve as the node's new parent.
     *
     * @return TRUE if the node could be reparented, FALSE if the node is
     * currently an ancestor of the requested parent.
     */
    bool reparent_node(const size_t nodeIndex, const size_t parentIndex = SCENE_NODE_ROOT_ID) noexcept;

    /**
     * Copy a node.
     *
     * This method will duplicate a node and all of its children. Large node
     * hierarchies will cause a large reallocation of the internal node and
     * transform arrays. The duplicated node will share the same parent as its
     * original counterpart.
     *
     * @param nodeIndex
     * An unsigned integral type, containing the array-index of the node to
     * copy within the graph.
     *
     * @return TRUE if the node at "nodeIndex" could be copied, FALSE if not.
     */
    bool copy_node(const size_t nodeIndex) noexcept;

    /**
     * Search for a node by its name and return its index.
     *
     * @param nameQuery
     * A constant reference to an std::string object, containing the name
     * of the node to search for.
     *
     * @return The array-index of the node being searched for, or
     * SCENE_GRAPH_ROOT_ID if the node was not found.
     */
    size_t find_node_id(const std::string& nameQuery) const noexcept;

    /**
     * Retrieve the total number of children hierarchially attached to a
     * SL_SceneNode.
     *
     * @param nodeIndex
     * An unsigned integral type, containing the array-index of the node to
     * query.
     *
     * @return An unsigned integral type, containing the total number of
     * child nodes that are recursively attached to the queried node.
     */
    size_t num_total_children(const size_t nodeIndex) const noexcept;

    /**
     * Retrieve the number of children immediately attached to a SL_SceneNode.
     *
     * @param nodeIndex
     * An unsigned integral type, containing the array-index of the node to
     * query.
     *
     * @return An unsigned integral type, containing the number of child
     * nodes that are attached to the queried node.
     */
    size_t num_immediate_children(const size_t nodeIndex) const noexcept;

    /**
     * Determine if a node is a hierarchal child of another node.
     *
     * @param nodeIndex
     * An unsigned integral type, containing the array-index of the node to
     * query.
     *
     * @param parentId
     * An unsigned integral type, containing the array-index of the
     * possible parent ID.
     */
    bool node_is_child(const size_t nodeIndex, const size_t parentId) const noexcept;

    /**
     * @brief Import data loaded from a mesh file loader.
     *
     * @param inGraph
     * A movable reference to a SL_SceneGraph object who's data will be moved
     * into *this.
     *
     * @return The new index of the imported graph's root node if the import
     * succeeded, SCENE_NODE_ROOT_ID if not.
     */
    size_t import(SL_SceneGraph& inGraph) noexcept;

    /**
     * @brief Insert an empty node object.
     *
     * @param parentId
     * The parent ID of *this node.
     *
     * @param name
     * A name to provide this node. This should be unique.
     *
     * @param transform
     * An initial transformation which will be concatenated with the parent
     * node's transformation.
     *
     * @return The index within the scene graph of the new mesh node.
     */
    size_t insert_empty_node(
        size_t parentId,
        const char* name,
        const SL_Transform& transform) noexcept;

    /**
     * @brief Place mesh data into the scene graph.
     *
     * @param m
     * The mesh to be inserted. This mesh will not have any association with a
     * mesh node unless explicitly specified.
     *
     * @param meshBounds
     * The bounding box to use for the mesh.
     *
     * @return The index within the scene graph which can be used by a mesh
     * node.
     */
    size_t insert_mesh(const SL_Mesh& m, const SL_BoundingBox& meshBounds) noexcept;

    /**
     * @brief Insert a mesh node and have it use currently existing mesh data.
     *
     * @param parentId
     * The parent ID of *this node.
     *
     * @param name
     * A name to provide this node. This should be unique.
     *
     * @param numSubMeshes
     * The number of SL_Mesh objects used by the inserted node.
     *
     * @param subMeshIds
     * The index of all sub-mesh (a.k.a. SL_Mesh) ids currently within the
     * scene graph to be used by the inserted node.
     *
     * @param transform
     * An initial transformation which will be concatenated with the parent
     * node's transformation.
     *
     * @return The index within the scene graph of the new mesh node.
     */
    size_t insert_mesh_node(
        size_t parentId,
        const char* name,
        size_t numSubMeshes,
        const size_t* subMeshIds,
        const SL_Transform& transform) noexcept;

    /**
     * @brief Insert a bone node, with transformation data, into the scene
     * graph
     *
     * @param parentId
     * The parent ID of *this node.
     *
     * @param name
     * A name to provide this node. This should be unique.
     *
     * @param inverseTransform
     * The root inverse transform of this bone's skeleton
     *
     * @param boneOffset
     * An offset matrix relative to the current bone's transformation.
     *
     * @param transform
     * An initial transformation which will be concatenated with the parent
     * node's transformation.
     *
     * @return The index within the scene graph of the new mesh node.
     */
    size_t insert_bone_node(
        size_t parentId,
        const char* name,
        const ls::math::mat4_t<float>& inverseTransform,
        const ls::math::mat4_t<float>& boneOffset,
        const SL_Transform& transform) noexcept;

    /**
     * @brief Insert a bone node, with transformation data, into the scene
     * graph
     *
     * @param parentId
     * The parent ID of *this node.
     *
     * @param name
     * A name to provide this node. This should be unique.
     *
     * @param cam
     * The current node's projection data.
     *
     * @param transform
     * An initial transformation which will be concatenated with the parent
     * node's transformation.
     *
     * @return The index within the scene graph of the new mesh node.
     */
    size_t insert_camera_node(
        size_t parentId,
        const char* name,
        const SL_Camera& cam,
        const SL_Transform& transform) noexcept;
};



#endif    /* SL_SCENE_GRAPH_HPP */
