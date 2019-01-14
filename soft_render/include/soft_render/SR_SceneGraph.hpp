
#ifndef SR_SCENE_GRAPH_HPP
#define SR_SCENE_GRAPH_HPP

#include <vector>

#include "soft_render/SR_Animation.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_SceneNode.hpp"



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



class SR_Camera;
struct SR_Mesh;
struct SR_Material;
class SR_Transform;
class SR_BoundingBox;



/**----------------------------------------------------------------------------
 * @brief the SceneGraph object contains all of the data necessary to either
 * instantiate or render SceneNodes in an OpenGL context.
-----------------------------------------------------------------------------*/
class SR_SceneGraph
{
  public: // member objects
    /**
     * Referenced by camera-type scene nodes using their
     * "SR_SceneNode::dataId" member.
     *
     * No two nodes should be able to reference a single camera. Doing so
     * will cause a crash when deleting nodes.
     */
    std::vector<SR_Camera> mCameras;

    /**
     * Array to contain all meshes referenced by mesh node draw commands.
     */
    std::vector<SR_Mesh> mMeshes;

    /**
     * Bounding boxes for meshes
     */
    std::vector<SR_BoundingBox> mMeshBounds;

    /**
     * Referenced by all mesh node types using the following relationship:
     *      "SceneGraph::nodeMeshes[SR_SceneNode::nodeId]->materialId"
     */
    std::vector<SR_Material> mMaterials;

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
    std::vector<SR_SceneNode> mNodes;

    /**
     * Referenced by all scene node types using their
     * "SR_SceneNode::nodeId" member. Base Transformations are not expected to
     * maintain a reference to their parent transform.
     */
    std::vector<ls::math::mat4_t<float>> mBaseTransforms;

    /**
     * Referenced by all scene node types using their
     * "SR_SceneNode::nodeId" member. The current transformation for a scene
     * node is expected to keep track of its parent transformation.
     */
    std::vector<SR_Transform> mCurrentTransforms;

    /**
     * Referenced by all scene node types using their
     * "SR_SceneNode::nodeId" member.
     */
    std::vector<ls::math::mat4_t<float>> mModelMatrices;

    /**
     * Referenced by all scene node types using their
     * "SR_SceneNode::nodeId" member.
     */
    std::vector<std::string> mNodeNames;

    /**
     * Contains all animations available in the current scene graph.
     */
    std::vector<SR_Animation> mAnimations;

    /**
     * Referenced by all scene node types using their
     * "SR_SceneNode::animTrackId" member.
     */
    std::vector<std::vector<SR_AnimationChannel>> mNodeAnims;

    /**
     * Referenced by mesh-type scene nodes using their
     * "SR_SceneNode::dataId" member.
     *
     * Some scene nodes may use more than one mesh. This member, along with the
     * "mNodeMeshCounts" member, will allow several mesh indices to be
     * referenced by a single node.
     */
    std::vector<ls::utils::Pointer<size_t[]>> mNodeMeshes;

    /**
     * Referenced by mesh-type scene nodes using their
     * "SR_SceneNode::dataId" member.
     *
     * No two nodes should be able to reference the same mesh count index.
     * Doing so will cause a crash when deleting nodes.
     */
    std::vector<size_t> mNumNodeMeshes;

    /**
     * @brief Graphical context & resources required for rendering all data in
     * *this.
     */
    SR_Context mContext;

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
     * @param animId
     * The array index of the animation being deleted.
     */
    void delete_node_animation_data(const size_t nodeId, const size_t animId) noexcept;

  public: // member functions
    /**
     * @brief Destructor
     *
     * Calls 'terminate()' to delete all CPU and GPU-side resources.
     */
    ~SR_SceneGraph() noexcept;

    /**
     * Constructor
     *
     * Initializes all members in *this to their default values.
     */
    SR_SceneGraph() noexcept;

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
    SR_SceneGraph(const SR_SceneGraph& s) noexcept;

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
    SR_SceneGraph(SR_SceneGraph&& s) noexcept;

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
    SR_SceneGraph& operator=(const SR_SceneGraph& s) noexcept;

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
    SR_SceneGraph& operator=(SR_SceneGraph&& s) noexcept;

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
     * SR_SceneNode.
     *
     * @param nodeIndex
     * An unsigned integral type, containing the array-index of the node to
     * query.
     *
     * @return An unsigned integral type, containing the total number of
     * child nodes that are recursively attached to the queried node.
     */
    size_t get_num_total_children(const size_t nodeIndex) const noexcept;

    /**
     * Retrieve the number of children immediately attached to a SR_SceneNode.
     *
     * @param nodeIndex
     * An unsigned integral type, containing the array-index of the node to
     * query.
     *
     * @return An unsigned integral type, containing the number of child
     * nodes that are attached to the queried node.
     */
    size_t get_num_immediate_children(const size_t nodeIndex) const noexcept;

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
     * A movable reference to a SR_SceneGraph object who's data will be moved
     * into *this.
     *
     * @return 0 if the import succeeded, non-zero if not.
     */
    int import(SR_SceneGraph& inGraph) noexcept;
};



#endif    /* SR_SCENE_GRAPH_HPP */
