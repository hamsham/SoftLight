
/**
 * @brief Basic SL_Octree interface
 *
 * @todo Finish implementing proper allocator usage
 *
 * @todo Permit the use of a secondary allocator for nodes only.
 */
#ifndef SL_OCTREE_HPP
#define SL_OCTREE_HPP

#include <vector>

#include "lightsky/setup/Types.h" // ls::setup::forward

#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"
#include "lightsky/math/vec_utils.h"




template <typename T, size_t MaxDepth, class Allocator = std::allocator<T>>
class SL_Octree;



/**
 * @brief Enum to describe which of the 8 directions an object can be placed
 * within an SL_Octree node.
 */
enum SL_OctreeDirection : int
{
    INSIDE = -1,

    FRONT_TOP_LEFT = 0,
    FRONT_TOP_RIGHT,
    FRONT_BOTTOM_LEFT,
    FRONT_BOTTOM_RIGHT,
    BACK_TOP_LEFT,
    BACK_TOP_RIGHT,
    BACK_BOTTOM_LEFT,
    BACK_BOTTOM_RIGHT,

    MAX_DIRECTIONS = 8
};



ls::math::vec4 sl_octree_direction_vector(SL_OctreeDirection direction) noexcept;



SL_OctreeDirection sl_octree_vector_direction(const ls::math::vec4& direction) noexcept;
SL_OctreeDirection sl_octree_vector_direction(const ls::math::vec3& direction) noexcept;



/**
 * @brief A Generic Octree node for spatial partitioning of general 3D
 * data.
 *
 * This octree will perform a best-fit of data into sub-trees. If an object
 * overlaps one or more sub-trees, it will be stored in the parent tree.
 *
 * @tparam T
 * The type of data to store. Must be copy-constructable.
 *
 * @tparam Allocator
 * An std::allocator-compatible object which will be responsible for managing
 * the memory of all data stored within the octree's subdivisions.
 */
template <typename T, class Allocator = std::allocator<T>>
class SL_OctreeNode
{
    template <typename, size_t, class>
    friend class Octree;

  protected:
    // contains origin using elements X, Y, & Z. Bounding box half-extent
    // (from origin) is contained in the W-component.
    ls::math::vec4 mOrigin;

    SL_OctreeNode* mParent;

    SL_OctreeNode<T, Allocator>* mNodes[8];

    std::vector<T, Allocator> mData;

    template <typename ConstIterCallbackType>
    void iterate_from_bottom_internal(ConstIterCallbackType&& iterCallback, size_t currDepth) const noexcept;

    template <typename IterCallbackType>
    void iterate_from_bottom_internal(IterCallbackType&& iterCallback, size_t currDepth) noexcept;

    template <typename ConstIterCallbackType>
    void iterate_from_top_internal(ConstIterCallbackType&& iterCallback, size_t currDepth) const noexcept;

    template <typename IterCallbackType>
    void iterate_from_top_internal(IterCallbackType&& iterCallback, size_t currDepth) noexcept;

  public:
    /**
     * @brief Destructor.
     *
     * Frees all internal memory.
     */
    virtual ~SL_OctreeNode() noexcept;

    /**
     * @brief Constructor
     *
     * @param pParent
     * Non-owning pointer to the current node's parent.
     *
     * @param origin
     * The center of the octree in 3D space.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     */
    SL_OctreeNode(
        SL_OctreeNode* pParent = nullptr,
        const ls::math::vec3& origin = ls::math::vec3{0.f, 0.f, 0.f},
        float extent = 1.f) noexcept;

    /**
     * @brief Constructor
     *
     * @param origin
     * The center of the octree in 3D space. The last (fourth) element of the
     * input vector is unused.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     */
    SL_OctreeNode(
        SL_OctreeNode* pParent,
        const ls::math::vec4& origin,
        float extent) noexcept;

    /**
     * @brief Copy Constructor
     *
     * Initializes internal data to its defaults, then calls the copy
     * operator.
     *
     * @param tree
     * The input tree to be copied into *this.
     */
    SL_OctreeNode(const SL_OctreeNode& tree) noexcept;

    /**
     * @brief Move Constructor
     *
     * Initializes and moves all data from the input tree into *this. No
     * copies are performed.
     *
     * @param tree
     * The input tree to be moved into *this.
     */
    SL_OctreeNode(SL_OctreeNode&& tree) noexcept;

    /**
     * @brief Copy Operator
     *
     * No copy will be performed if storage could not be allocated for the new
     * input data.
     *
     * @param tree
     * The input tree to be copied into *this.
     *
     * @return  A reference to *this.
     */
    SL_OctreeNode& operator=(const SL_OctreeNode& tree) noexcept;

    /**
     * @brief Move Operator
     *
     * Moves all data from the input tree into *this.
     *
     * @param tree
     * The input tree to be moved into *this.
     *
     * @return A reference to *this.
     */
    SL_OctreeNode& operator=(SL_OctreeNode&& tree) noexcept;

    /**
     * @brief Determine the direction of an object, relative to *this in 3D
     * space.
     *
     * @param location
     * A 3D point which indicates where an object exists.
     *
     * @param extent
     * The distance from \p location to a corner of the object's axis-aligned
     * bounding box.
     *
     * @return An enum which determines where an object can be placed in a
     * sub-node, relative to the origin of *this parent node.
     */
    SL_OctreeDirection relative_direction_for_object(
        const ls::math::vec4& location,
        float extent) const noexcept;

    /**
     * @brief Determine the direction of an object, relative to *this in 3D
     * space.
     *
     * @param location
     * A 3D point which indicates where an object exists.
     *
     * @param extent
     * The distance from \p location to a corner of the object's axis-aligned
     * bounding box.
     *
     * @return An enum which determines where an object can be placed in a
     * sub-node, relative to the origin of *this parent node.
     */
    SL_OctreeDirection relative_direction_for_object(
        const ls::math::vec3& location,
        float extent) const noexcept;

    /**
     * @brief Retrieve the user-defined origin of the top-level octree.
     *
     * Sub-trees will return their origin with respect to, and subdivided by,
     * the top-level octree.
     *
     * @return A reference to the 3D (4D-padded) octree's origin.
     */
    ls::math::vec4 origin() const noexcept;

    /**
     * @brief Retrieve the extent of an internal bounding-sphere's 3D space.
     *
     * Sub-trees will return their extent with respect to, and subdivided by,
     * the top-level octree.
     *
     * @return A floating-point value representing the distance from the node's
     * origin to its bounding box corners.
     */
    float extent() const noexcept;

    /**
     * @brief Retrieve the current node's parent node.
     *
     * @return A pointer to a parent SL_OctreeNode, if one exists.
     */
    const SL_OctreeNode<T, Allocator>* parent() const noexcept;

    /**
     * @brief Retrieve the current node's parent node.
     *
     * @return A pointer to a parent SL_OctreeNode, if one exists.
     */
    SL_OctreeNode<T, Allocator>* parent() noexcept;

    /**
     * @brief Retrieve the internal sub-trees.
     *
     * The returned list of pointers will not be null, but the objects
     * contained within the returned list may be null.
     *
     * @return A pointer to a list of 8 constant sub-trees.
     */
    const SL_OctreeNode<T, Allocator>* const* sub_nodes() const noexcept;

    /**
     * @brief Retrieve the internal sub-trees.
     *
     * The returned list of pointers will not be null, but the objects
     * contained within the returned list may be null.
     *
     * @return A pointer to a list of 8 sub-trees.
     */
    SL_OctreeNode<T, Allocator>** sub_nodes() noexcept;

    /**
     * @brief Retrieve a pointer to a sub-node.
     *
     * @return A const pointer to a single octree node, if one exists at the
     * requested direction.
     */
    const SL_OctreeNode<T, Allocator>* sub_node(SL_OctreeDirection direction) const noexcept;

    /**
     * @brief Retrieve a pointer to a sub-node.
     *
     * @return A pointer to a single octree node, if one exists at the
     * requested direction.
     */
    SL_OctreeNode<T, Allocator>* sub_node(SL_OctreeDirection direction) noexcept;

    /**
     * @brief Retrieve the constant list of objects contained directly within
     * *this top-level tree node.
     *
     * @return All data contained at *this tree's bounds. This list does not
     * include sub-tree data.
     */
    const std::vector<T, Allocator>& data() const noexcept;

    /**
     * @brief Retrieve the list of objects contained directly within *this
     * top-level tree node.
     *
     * @return All data contained at *this tree's bounds. This list does not
     * include sub-tree data.
     */
    std::vector<T, Allocator>& data() noexcept;

    /**
     * @brief Determine if this node contains any data which is not in a sub-tree.
     *
     * @return TRUE if this specific node contains any data, FALSE if not.
     */
    bool empty() const noexcept;

    /**
     * @brief Retrieve the number of objects contained at *this tree's 3D
     * space.
     *
     * @return An unsigned integral type, representing the number of objects
     * contained directly within this current node, not including sub-nodes.
     */
    size_t size() const noexcept;

    /**
     * @brief Retrieve the number of local partitions occupied by *this.
     *
     * @return An unsigned integral type, representing the number of direct
     * partitions created by *this node..
     */
    size_t breadth() const noexcept;

    /**
     * @brief Retrieve the depth of all sub-trees contained within *this.
     *
     * @return A zero-based integral, representing the distance to the most
     * distant sub-tree.
     */
    size_t depth() const noexcept;

    /**
     * @brief Clear all memory, data, and sub-trees occupied by *this.
     */
    void clear() noexcept;

    /**
     * @brief Locate the closest sub-partition referenced by a point in 3D
     * space (const).
     *
     * @param location
     * A 3D point where data should be referenced.
     *
     * @return The closes sub-partition which contains the requested point.
     */
    const SL_OctreeNode<T, Allocator>* find(const ls::math::vec4& location) const noexcept;

    /**
     * @brief Locate the closest sub-partition referenced by a point in 3D
     * space.
     *
     * @param location
     * A 3D point where data should be referenced.
     *
     * @return The closes sub-partition which contains the requested point.
     */
    SL_OctreeNode<T, Allocator>* find(const ls::math::vec4& location) noexcept;

    /**
     * @brief Locate the closest sub-partition referenced by a point in 3D
     * space (const).
     *
     * @param location
     * A 3D point where data should be referenced.
     *
     * @return The closes sub-partition which contains the requested point.
     */
    const SL_OctreeNode<T, Allocator>* find(const ls::math::vec3& location) const noexcept;

    /**
     * @brief Locate the closest sub-partition referenced by a point in 3D
     * space.
     *
     * @param location
     * A 3D point where data should be referenced.
     *
     * @return The closes sub-partition which contains the requested point.
     */
    SL_OctreeNode<T, Allocator>* find(const ls::math::vec3& location) noexcept;

    /**
     * @brief Perform a depth-first iteration over all sub-trees in *this
     * (const).
     *
     * @param iterCallback
     * A callback function to be invoked at every sub-node in *this. The
     * function should return false if no further iteration is needed at a
     * sub-node or its children. It should return true to continue the
     * depth-first iteration into a node's sub-tree.
     */
    template <typename ConstIterCallbackType>
    void iterate_bottom_up(ConstIterCallbackType&& iterCallback) const noexcept;

    /**
     * @brief Perform a depth-first iteration over all sub-trees in *this.
     *
     * @param iterCallback
     * A callback function to be invoked at every sub-node in *this. The
     * function should return false if no further iteration is needed at a
     * sub-node or its children. It should return true to continue the
     * depth-first iteration into a node's sub-tree.
     */
    template <typename IterCallbackType>
    void iterate_bottom_up(IterCallbackType&& iterCallback) noexcept;

    /**
     * @brief Perform a top-down iteration over all sub-trees in *this
     * (const).
     *
     * @param iterCallback
     * A callback function to be invoked at every sub-node in *this. The
     * function should return false if no further iteration is needed at a
     * sub-node or its children. It should return true to continue the
     * depth-first iteration into a node's sub-tree.
     */
    template <typename ConstIterCallbackType>
    void iterate_top_down(ConstIterCallbackType&& iterCallback) const noexcept;

    /**
     * @brief Perform a top-down iteration over all sub-trees in *this.
     *
     * @param iterCallback
     * A callback function to be invoked at every sub-node in *this. The
     * function should return false if no further iteration is needed at a
     * sub-node or its children. It should return true to continue the
     * depth-first iteration into a node's sub-tree.
     */
    template <typename IterCallbackType>
    void iterate_top_down(IterCallbackType&& iterCallback) noexcept;
};



/*-----------------------------------------------------------------------------
 * Octree Helper Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Direction vectors for SL_OctreeDirection enums
-------------------------------------*/
inline ls::math::vec4 sl_octree_direction_vector(SL_OctreeDirection direction) noexcept
{
    constexpr float x = 1.f;
    ls::math::vec4 result;

    switch (direction)
    {
        case FRONT_TOP_LEFT:     result = ls::math::vec4{ x,  x,  x, 0.f}; break;
        case FRONT_TOP_RIGHT:    result = ls::math::vec4{-x,  x,  x, 0.f}; break;
        case FRONT_BOTTOM_LEFT:  result = ls::math::vec4{ x, -x,  x, 0.f}; break;
        case FRONT_BOTTOM_RIGHT: result = ls::math::vec4{-x, -x,  x, 0.f}; break;
        case BACK_TOP_LEFT:      result = ls::math::vec4{ x,  x, -x, 0.f}; break;
        case BACK_TOP_RIGHT:     result = ls::math::vec4{-x,  x, -x, 0.f}; break;
        case BACK_BOTTOM_LEFT:   result = ls::math::vec4{ x, -x, -x, 0.f}; break;
        case BACK_BOTTOM_RIGHT:  result = ls::math::vec4{-x, -x, -x, 0.f}; break;
        default:
            result = ls::math::vec4{0.f};
            break;
    }

    return result;
}



/*-------------------------------------
 * SL_OctreeDirection from Direction vector (4D)
-------------------------------------*/
inline SL_OctreeDirection sl_octree_vector_direction(const ls::math::vec4& direction) noexcept
{
    if (ls::math::length(direction) == 0.f)
    {
        return SL_OctreeDirection::INSIDE;
    }

    const int mask = ls::math::sign_mask(direction);
    return static_cast<SL_OctreeDirection>(mask);
}



/*-------------------------------------
 * SL_OctreeDirection from Direction vector (3D)
-------------------------------------*/
inline SL_OctreeDirection sl_octree_vector_direction(const ls::math::vec3& direction) noexcept
{
    if (ls::math::length(direction) == 0.f)
    {
        return SL_OctreeDirection::INSIDE;
    }

    const int mask = ls::math::sign_mask(direction);
    return static_cast<SL_OctreeDirection>(mask);
}



/*-----------------------------------------------------------------------------
 * Octree Node Member Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeNode<T, Allocator>::~SL_OctreeNode() noexcept
{
    clear();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeNode<T, Allocator>::SL_OctreeNode(
    SL_OctreeNode* pParent,
    const ls::math::vec3& origin,
    float extent) noexcept :
    mOrigin{origin[0], origin[1], origin[2], extent},
    mParent{pParent},
    mNodes{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
    mData{}
{}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeNode<T, Allocator>::SL_OctreeNode(
    SL_OctreeNode* pParent,
    const ls::math::vec4& origin,
    float extent) noexcept :
    mOrigin{origin[0], origin[1], origin[2], extent},
    mParent{pParent},
    mNodes{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
    mData{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeNode<T, Allocator>::SL_OctreeNode(const SL_OctreeNode& tree) noexcept :
    SL_OctreeNode{}
{
    *this = tree;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeNode<T, Allocator>::SL_OctreeNode(SL_OctreeNode&& tree) noexcept :
    mOrigin{tree.mOrigin},
    mParent{tree.mParent},
    mNodes{
        tree.mNodes[0],
        tree.mNodes[1],
        tree.mNodes[2],
        tree.mNodes[3],
        tree.mNodes[4],
        tree.mNodes[5],
        tree.mNodes[6],
        tree.mNodes[7],
    },
    mData{std::move(tree.mData)}
{
    tree.mOrigin = ls::math::vec4{0.f};
    tree.mParent = nullptr;

    for (unsigned i = 0; i < 8; ++i)
    {
        tree.mNodes[i] = nullptr;
    }
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeNode<T, Allocator>& SL_OctreeNode<T, Allocator>::operator=(const SL_OctreeNode& tree) noexcept
{
    if (this == &tree)
    {
        return *this;
    }

    clear();

    mOrigin = tree.mOrigin;
    mParent = nullptr;
    mData = tree.mData;

    for (unsigned i = 0; i < 8; ++i)
    {
        SL_OctreeNode<T, Allocator>* pNode = tree.mNodes[i];
        if (pNode && pNode->mNodes[i] && !mNodes[i])
        {
            mNodes[i] = new(std::nothrow) SL_OctreeNode<T, Allocator>{};
            *mNodes[i] = *(pNode->mNodes[i]); // recurse
            mNodes[i]->mParent = this;
        }
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeNode<T, Allocator>& SL_OctreeNode<T, Allocator>::operator=(SL_OctreeNode&& tree) noexcept
{
    if (this == &tree)
    {
        return *this;
    }

    clear();

    mOrigin = tree.mOrigin;
    tree.mOrigin = ls::math::vec4{0.f};

    mParent = tree.mParent;
    tree.mParent = nullptr;

    for (unsigned i = 0; i < 8; ++i)
    {
        if (mNodes[i])
        {
            delete mNodes[i];
            mNodes[i] = nullptr;
        }

        mNodes[i] = tree.mNodes[i];
        tree.mNodes[i] = nullptr;

        if (mNodes[i])
        {
            mNodes[i]->mParent = this;
        }
    }

    mData = std::move(tree.mData);

    return *this;
}



/*-------------------------------------
 * Determine the direction of an object, relative to *this.
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeDirection SL_OctreeNode<T, Allocator>::relative_direction_for_object(
    const ls::math::vec4& location,
    float extent) const noexcept
{
    const ls::math::vec4&& localSpace = this->origin() - location;
    const ls::math::vec4&& ls0 = localSpace - extent;
    const ls::math::vec4&& ls1 = localSpace + extent;

    // Calculate a three-bit mask from the object's position and size. This
    // mask will be used as the index of a sub-node in the tree. Sign-masks
    // determine if data should be placed into a locally positive or negative
    // direction for each X/Y/Z component.
    // This will simultaneously check all sub-quadrants to determine which
    // individual sub-node can properly contain data.
    const int locations[8] = {
        ls::math::sign_mask(ls::math::vec4{ls0[0], ls0[1], ls0[2], 0.f}), // left,  top,    front
        ls::math::sign_mask(ls::math::vec4{ls1[0], ls0[1], ls0[2], 0.f}), // right, top,    front
        ls::math::sign_mask(ls::math::vec4{ls0[0], ls1[1], ls0[2], 0.f}), // left,  bottom, front
        ls::math::sign_mask(ls::math::vec4{ls1[0], ls1[1], ls0[2], 0.f}), // right, bottom, front
        ls::math::sign_mask(ls::math::vec4{ls0[0], ls0[1], ls1[2], 0.f}), // left,  top,    back
        ls::math::sign_mask(ls::math::vec4{ls1[0], ls0[1], ls1[2], 0.f}), // right, top,    back
        ls::math::sign_mask(ls::math::vec4{ls0[0], ls1[1], ls1[2], 0.f}), // left,  bottom, back
        ls::math::sign_mask(ls::math::vec4{ls1[0], ls1[1], ls1[2], 0.f}), // right, bottom, back
    };

    // determine if all the calculated masks match. unique masks mean the
    // object overlaps sub-nodes
    const int nodeId   = locations[0] | locations[1] | locations[2] | locations[3] | locations[4] | locations[5] | locations[6] | locations[7];
    const int overlaps = locations[0] & locations[1] & locations[2] & locations[3] & locations[4] & locations[5] & locations[6] & locations[7];

    // If an object intersects multiple sub-nodes, keep it in the current node
    // rather than split the object across the intersecting sub-nodes
    if (nodeId ^ overlaps)
    {
        return SL_OctreeDirection::INSIDE;
    }

    return static_cast<SL_OctreeDirection>(nodeId);
}



/*-------------------------------------
 * Determine the direction of an object, relative to *this.
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeDirection SL_OctreeNode<T, Allocator>::relative_direction_for_object(
    const ls::math::vec3& location,
    float extent) const noexcept
{
    return relative_direction_for_object(ls::math::vec4_cast(location, 0.f), extent);
}



/*-------------------------------------
 * Get the origin
-------------------------------------*/
template <typename T, class Allocator>
inline ls::math::vec4 SL_OctreeNode<T, Allocator>::origin() const noexcept
{
    return ls::math::vec4{mOrigin[0], mOrigin[1], mOrigin[2], 0.f};
}



/*-------------------------------------
 * Get the bounding-box extent
-------------------------------------*/
template <typename T, class Allocator>
inline float SL_OctreeNode<T, Allocator>::extent() const noexcept
{
    return mOrigin[3];
}



/*-------------------------------------
 * Get the parent node (const)
-------------------------------------*/
template <typename T, class Allocator>
inline const SL_OctreeNode<T, Allocator>* SL_OctreeNode<T, Allocator>::parent() const noexcept
{
    return mParent;
}



/*-------------------------------------
 * Get the parent node
-------------------------------------*/
template <typename T, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_OctreeNode<T, Allocator>::parent() noexcept
{
    return mParent;
}



/*-------------------------------------
 * Get the sub-nodes (const)
-------------------------------------*/
template <typename T, class Allocator>
inline const SL_OctreeNode<T, Allocator>* const* SL_OctreeNode<T, Allocator>::sub_nodes() const noexcept
{
    return mNodes;
}



/*-------------------------------------
 * Get the sub-nodes
-------------------------------------*/
template <typename T, class Allocator>
inline SL_OctreeNode<T, Allocator>** SL_OctreeNode<T, Allocator>::sub_nodes() noexcept
{
    return mNodes;
}



/*-------------------------------------
 * Get the sub-nodes (const)
-------------------------------------*/
template <typename T, class Allocator>
inline const SL_OctreeNode<T, Allocator>* SL_OctreeNode<T, Allocator>::sub_node(SL_OctreeDirection dir) const noexcept
{
    const int index = static_cast<int>(dir);
    if (index < 0 || index >= SL_OctreeDirection::MAX_DIRECTIONS)
    {
        return nullptr;
    }

    return mNodes[index];
}



/*-------------------------------------
 * Get the sub-nodes
-------------------------------------*/
template <typename T, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_OctreeNode<T, Allocator>::sub_node(SL_OctreeDirection dir) noexcept
{
    const int index = static_cast<int>(dir);
    if (index < 0 || index >= SL_OctreeDirection::MAX_DIRECTIONS)
    {
        return nullptr;
    }

    return mNodes[index];
}



/*-------------------------------------
 * Get the internal objects (const)
-------------------------------------*/
template <typename T, class Allocator>
inline const std::vector<T, Allocator>& SL_OctreeNode<T, Allocator>::data() const noexcept
{
    return mData;
}



/*-------------------------------------
 * Get the internal objects
-------------------------------------*/
template <typename T, class Allocator>
inline std::vector<T, Allocator>& SL_OctreeNode<T, Allocator>::data() noexcept
{
    return mData;
}



/*-------------------------------------
 * Determine if there's any data
-------------------------------------*/
template <typename T, class Allocator>
inline bool SL_OctreeNode<T, Allocator>::empty() const noexcept
{
    return mData.empty();
}



/*-------------------------------------
 * Get the current data size
-------------------------------------*/
template <typename T, class Allocator>
inline size_t SL_OctreeNode<T, Allocator>::size() const noexcept
{
    return mData.size();
}



/*-------------------------------------
 * Get the number of local partitions
-------------------------------------*/
template <typename T, class Allocator>
size_t SL_OctreeNode<T, Allocator>::breadth() const noexcept
{
    size_t count = 0;

    for (const SL_OctreeNode<T, Allocator>* pNode : mNodes)
    {
        count += pNode != nullptr;
    }

    return count;
}



/*-------------------------------------
 * Get the overall depth from *this
-------------------------------------*/
template <typename T, class Allocator>
inline size_t SL_OctreeNode<T, Allocator>::depth() const noexcept
{
    size_t d = 0;
    for (const SL_OctreeNode<T, Allocator>* pNode : mNodes)
    {
        if (pNode)
        {
            d = ls::math::max(d, 1+pNode->depth());
        }
    }

    return d;
}



/*-------------------------------------
 * Clear all data & memory
-------------------------------------*/
template <typename T, class Allocator>
void SL_OctreeNode<T, Allocator>::clear() noexcept
{
    for (unsigned i = 0; i < 8; ++i)
    {
        if (mNodes[i])
        {
            delete mNodes[i];
            mNodes[i] = nullptr;
        }
    }

    mData.clear();
}



/*-------------------------------------
 * Find a sub-node at a location (const)
-------------------------------------*/
template <typename T, class Allocator>
const SL_OctreeNode<T, Allocator>* SL_OctreeNode<T, Allocator>::find(const ls::math::vec4& location) const noexcept
{
    const int nodeId = ls::math::sign_mask(location - this->origin());

    if (!this->mNodes[nodeId])
    {
        return this;
    }

    return this->mNodes[nodeId]->find(location);
}



/*-------------------------------------
 * Find a sub-node at a location
-------------------------------------*/
template <typename T, class Allocator>
SL_OctreeNode<T, Allocator>* SL_OctreeNode<T, Allocator>::find(const ls::math::vec4& location) noexcept
{
    const int nodeId = ls::math::sign_mask(location - this->origin());

    if (!this->mNodes[nodeId])
    {
        return this;
    }

    return this->mNodes[nodeId]->find(location);
}



/*-------------------------------------
 * Find a sub-node at a location (const)
-------------------------------------*/
template <typename T, class Allocator>
inline const SL_OctreeNode<T, Allocator>* SL_OctreeNode<T, Allocator>::find(const ls::math::vec3& location) const noexcept
{
    return this->find(ls::math::vec4_cast(location, 0.f));
}



/*-------------------------------------
 * Find a sub-node at a location
-------------------------------------*/
template <typename T, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_OctreeNode<T, Allocator>::find(const ls::math::vec3& location) noexcept
{
    return this->find(ls::math::vec4_cast(location, 0.f));
}



/*-------------------------------------
 * Internal Depth-first iteration (const)
-------------------------------------*/
template <typename T, class Allocator>
template <typename ConstIterCallbackType>
void SL_OctreeNode<T, Allocator>::iterate_from_bottom_internal(ConstIterCallbackType&& iterCallback, size_t currDepth) const noexcept
{
    for (const SL_OctreeNode<T, Allocator>* pNode : this->mNodes)
    {
        if (pNode)
        {
            pNode->iterate_from_bottom_internal<ConstIterCallbackType>(std::forward<ConstIterCallbackType>(iterCallback), currDepth+1);
        }
    }

    iterCallback(this, currDepth);
}



/*-------------------------------------
 * Internal Depth-first iteration
-------------------------------------*/
template <typename T, class Allocator>
template <typename IterCallbackType>
void SL_OctreeNode<T, Allocator>::iterate_from_bottom_internal(IterCallbackType&& iterCallback, size_t currDepth) noexcept
{
    for (SL_OctreeNode<T, Allocator>* pNode : this->mNodes)
    {
        if (pNode)
        {
            pNode->iterate_from_bottom_internal<IterCallbackType>(std::forward<IterCallbackType>(iterCallback), currDepth+1);
        }
    }

    iterCallback(this, currDepth);
}



/*-------------------------------------
 * Depth-first iteration (const)
-------------------------------------*/
template <typename T, class Allocator>
template <typename ConstIterCallbackType>
inline void SL_OctreeNode<T, Allocator>::iterate_bottom_up(ConstIterCallbackType&& iterCallback) const noexcept
{
    this->iterate_from_bottom_internal<ConstIterCallbackType>(std::forward<ConstIterCallbackType>(iterCallback), 0);
}



/*-------------------------------------
 * Depth-first iteration
-------------------------------------*/
template <typename T, class Allocator>
template <typename IterCallbackType>
inline void SL_OctreeNode<T, Allocator>::iterate_bottom_up(IterCallbackType&& iterCallback) noexcept
{
    this->iterate_from_bottom_internal<IterCallbackType>(std::forward<IterCallbackType>(iterCallback), 0);
}



/*-------------------------------------
 * Internal Breadth-first iteration (const)
-------------------------------------*/
template <typename T, class Allocator>
template <typename ConstIterCallbackType>
void SL_OctreeNode<T, Allocator>::iterate_from_top_internal(ConstIterCallbackType&& iterCallback, size_t currDepth) const noexcept
{
    if (!iterCallback(this, currDepth))
    {
        return;
    }

    for (const SL_OctreeNode<T, Allocator>* pNode : this->mNodes)
    {
        if (pNode)
        {
            pNode->iterate_from_top_internal<ConstIterCallbackType>(std::forward<ConstIterCallbackType>(iterCallback), currDepth+1);
        }
    }
}



/*-------------------------------------
 * Internal Top-down iteration
-------------------------------------*/
template <typename T, class Allocator>
template <typename IterCallbackType>
void SL_OctreeNode<T, Allocator>::iterate_from_top_internal(IterCallbackType&& iterCallback, size_t currDepth) noexcept
{
    if (!iterCallback(this, currDepth))
    {
        return;
    }

    for (SL_OctreeNode<T, Allocator>* pNode : this->mNodes)
    {
        if (pNode)
        {
            pNode->iterate_from_top_internal<IterCallbackType>(std::forward<IterCallbackType>(iterCallback), currDepth+1);
        }
    }
}



/*-------------------------------------
 * Top-down iteration (const)
-------------------------------------*/
template <typename T, class Allocator>
template <typename ConstIterCallbackType>
inline void SL_OctreeNode<T, Allocator>::iterate_top_down(ConstIterCallbackType&& iterCallback) const noexcept
{
    this->iterate_from_top_internal<ConstIterCallbackType>(std::forward<ConstIterCallbackType>(iterCallback), 0);
}



/*-------------------------------------
 * Top-down iteration
-------------------------------------*/
template <typename T, class Allocator>
template <typename IterCallbackType>
inline void SL_OctreeNode<T, Allocator>::iterate_top_down(IterCallbackType&& iterCallback) noexcept
{
    this->iterate_from_top_internal<IterCallbackType>(std::forward<IterCallbackType>(iterCallback), 0);
}



/**
 * @brief A Generic SL_Octree container for spatial partitioning of general 3D
 * data.
 *
 * This octree will perform a best-fit of data into sub-trees. If an object
 * overlaps one or more sub-trees, it will be stored in the parent tree.
 *
 * @tparam T
 * The type of data to store. Must be copy-constructable.
 *
 * @tparam MaxDepth
 * The maximum depth (subdivisions) of the octree.
 *
 * @tparam Allocator
 * An std::allocator-compatible object which will be responsible for managing
 * the memory of all data stored within the octree's subdivisions.
 */
template <typename T, size_t MaxDepth, class Allocator>
class SL_Octree final : public SL_OctreeNode<T, Allocator>
{
  public:
    enum Limits : size_t
    {
        DEFAULT_DEPTH_LIMIT = std::numeric_limits<size_t>::max()
    };

    SL_OctreeNode<T, Allocator>* _insert_node(const ls::math::vec4& location, float extent, size_t depthLimit) noexcept;

  public:
    /**
     * @brief Destructor.
     *
     * Frees all internal memory.
     */
    virtual ~SL_Octree() noexcept override = default;

    /**
     * @brief Constructor
     *
     * @param origin
     * The center of the octree in 3D space.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     */
    SL_Octree(
        const ls::math::vec3& origin = ls::math::vec3{0.f, 0.f, 0.f},
        float extent = 1.f) noexcept;

    /**
     * @brief Constructor
     *
     * @param origin
     * The center of the octree in 3D space. The last (fourth) element of the
     * input vector is unused.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     */
    SL_Octree(
        const ls::math::vec4& origin,
        float extent) noexcept;

    /**
     * @brief Copy Constructor
     *
     * Initializes internal data to its defaults, then calls the copy
     * operator.
     *
     * @param tree
     * The input tree to be copied into *this.
     */
    SL_Octree(const SL_Octree& tree) noexcept;

    /**
     * @brief Move Constructor
     *
     * Initializes and moves all data from the input tree into *this. No
     * copies are performed.
     *
     * @param tree
     * The input tree to be moved into *this.
     */
    SL_Octree(SL_Octree&& tree) noexcept;

    /**
     * @brief Copy Operator
     *
     * No copy will be performed if storage could not be allocated for the new
     * input data.
     *
     * @param tree
     * The input tree to be copied into *this.
     *
     * @return  A reference to *this.
     */
    SL_Octree& operator=(const SL_Octree& tree) noexcept;

    /**
     * @brief Move Operator
     *
     * Moves all data from the input tree into *this.
     *
     * @param tree
     * The input tree to be moved into *this.
     *
     * @return A reference to *this.
     */
    SL_Octree& operator=(SL_Octree&& tree) noexcept;

    /**
     * @brief Retrieve the maximum allowable depth possible in this tree.
     *
     * @return The template parameter "MaxDepth".
     */
    constexpr size_t max_depth() const noexcept;

    /**
     * @brief Insert an empty node into *this, creating sub-tree partitions if
     * needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     *
     * @return A pointer to the node which exists (or is created at) at \p
     * location.
     */
    SL_OctreeNode<T, Allocator>* insert_empty(
        const ls::math::vec4& location,
        float extent,
        size_t depthLimit = DEFAULT_DEPTH_LIMIT) noexcept;

    /**
     * @brief Insert an empty node into *this, creating sub-tree partitions if
     * needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     *
     * @return A pointer to the node which exists (or is created at) at \p
     * location.
     */
    SL_OctreeNode<T, Allocator>* insert_empty(
        const ls::math::vec3& location,
        float extent,
        size_t depthLimit = DEFAULT_DEPTH_LIMIT) noexcept;

    /**
     * @brief Insert an object into *this, creating sub-tree partitions if
     * needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     *
     * @param value
     * The data to be stored. This object must be copy-constructable,
     * copy-assignable, move-constructable, or move-assignable.
     *
     * @return A pointer to the node which will contain \p args.
     */
    SL_OctreeNode<T, Allocator>* insert(
        const ls::math::vec4& location,
        float extent,
        const T& value,
        size_t depthLimit = DEFAULT_DEPTH_LIMIT) noexcept;

    /**
     * @brief Insert (copy) an object into *this, creating sub-tree partitions
     * if needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     *
     * @param value
     * The data to be stored. This object must be move-constructable, and
     * move-assignable.
     *
     * @return A pointer to the node which will contain \p args.
     */
    SL_OctreeNode<T, Allocator>* insert(
        const ls::math::vec3& location,
        float extent,
        const T& value,
        size_t depthLimit = DEFAULT_DEPTH_LIMIT) noexcept;

    /**
     * @brief Insert an object into *this, creating sub-tree partitions if
     * needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     *
     * @param value
     * The data to be stored. This object must be copy-constructable,
     * copy-assignable, move-constructable, or move-assignable.
     *
     * @return A pointer to the node which will contain \p args.
     */
    SL_OctreeNode<T, Allocator>* insert(
        const ls::math::vec4& location,
        float extent,
        T&& value,
        size_t depthLimit = DEFAULT_DEPTH_LIMIT) noexcept;

    /**
     * @brief Insert (copy) an object into *this, creating sub-tree partitions
     * if needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     *
     * @param value
     * The data to be stored. This object must be move-constructable, and
     * move-assignable.
     *
     * @return A pointer to the node which will contain \p args.
     */
    SL_OctreeNode<T, Allocator>* insert(
        const ls::math::vec3& location,
        float extent,
        T&& value,
        size_t depthLimit = DEFAULT_DEPTH_LIMIT) noexcept;

    /**
     * @brief Insert an object into *this, creating sub-tree partitions if
     * needed.
     *
     * @tparam Args
     * Arguments for constructing a type T in-place
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     *
     * @param args
     * The data to construct a type T in-place.
     *
     * @return A pointer to the node which will contain \p args.
     */
     template <typename... Args>
    SL_OctreeNode<T, Allocator>* emplace(
        const ls::math::vec4& location,
        float extent,
        Args&&... args,
        size_t depthLimit = DEFAULT_DEPTH_LIMIT) noexcept;

    /**
     * @brief Insert an object into *this, creating sub-tree partitions if
     * needed.
     *
     * @tparam Args
     * Arguments for constructing a type T in-place
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param extent
     * The distance from the node's origin to its bounding box corners.
     *
     * @param args
     * The data to construct a type T in-place.
     *
     * @return A pointer to the node which will contain \p args.
     */
    template <typename... Args>
    SL_OctreeNode<T, Allocator>* emplace(
        const ls::math::vec3& location,
        float exetent,
        Args&&... args,
        size_t depthLimit = DEFAULT_DEPTH_LIMIT) noexcept;
};



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::SL_Octree(
    const ls::math::vec3& origin,
    float extent) noexcept :
    SL_OctreeNode<T, Allocator>{nullptr, origin, extent}
{}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::SL_Octree(
    const ls::math::vec4& origin,
    float extent) noexcept :
    SL_OctreeNode<T, Allocator>{nullptr, origin, extent}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::SL_Octree(const SL_Octree& tree) noexcept :
    SL_OctreeNode<T, Allocator>{tree}
{
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::SL_Octree(SL_Octree&& tree) noexcept :
    SL_OctreeNode<T, Allocator>{std::move(tree)}
{
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>& SL_Octree<T, MaxDepth, Allocator>::operator=(const SL_Octree& tree) noexcept
{
    if (this != &tree)
    {
        this->SL_OctreeNode<T, Allocator>::operator=(std::forward<const SL_OctreeNode<T, Allocator>&>(tree));
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>& SL_Octree<T, MaxDepth, Allocator>::operator=(SL_Octree&& tree) noexcept
{
    if (this != &tree)
    {
        this->SL_OctreeNode<T, Allocator>::operator=(std::move(tree));
    }

    return *this;
}



/*-------------------------------------
 * Place a new node into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::_insert_node(const ls::math::vec4& location, float extent, size_t depthLimit) noexcept
{
    typedef SL_Octree<T, MaxDepth, Allocator> OctreeType;
    SL_OctreeNode<T, Allocator>* pParent = nullptr;
    SL_OctreeNode<T, Allocator>* pTree = this;
    size_t currDepth = 0;

    do
    {
        // Don't bother placing an object into sub-nodes if it can't fit
        const float subExtent = pTree->extent() * 0.5f;
        if (extent > subExtent
        || (MaxDepth && currDepth >= MaxDepth)
        || (depthLimit != OctreeType::DEFAULT_DEPTH_LIMIT && currDepth >= depthLimit))
        {
            break;
        }

        // Don't bother placing an object into sub-nodes if it can't fit
        const int nodeId = (int)pTree->relative_direction_for_object(location, extent);
        if (nodeId < 0)
        {
            break;
        }

        // Using the sign-mask, bucket the data into one of 8 sub-nodes
        const float xSign = (nodeId & 0x01) ? -subExtent : subExtent;
        const float ySign = (nodeId & 0x02) ? -subExtent : subExtent;
        const float zSign = (nodeId & 0x04) ? -subExtent : subExtent;

        // Don't bother placing an object into sub-nodes if it can't fit
        const ls::math::vec4&& nodeExtents = ls::math::vec4{xSign, ySign, zSign, 0.f};
        const ls::math::vec4&& nodeLocation = pTree->origin() - nodeExtents;

        // Data can still be bucketed into a smaller lead. Add a sub-node and
        // continue iterating
        SL_OctreeNode<T, Allocator>** pSubNodes = pTree->sub_nodes();
        if (!pSubNodes[nodeId])
        {
            OctreeType* const pNewSubNode = new(std::nothrow) OctreeType{nodeLocation, subExtent};
            if (!pNewSubNode)
            {
                pTree = nullptr; // crash
                break;
            }

            pNewSubNode->mParent = pParent;
            pSubNodes[nodeId] = pNewSubNode;
        }

        ++currDepth;
        pParent = pTree;
        pTree = pSubNodes[nodeId];
    }
    while (true);

    return pTree;
}



/*-------------------------------------
 * Get the maximum allowable depth
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
constexpr size_t SL_Octree<T, MaxDepth, Allocator>::max_depth() const noexcept
{
    return MaxDepth;
}



/*-------------------------------------
 * Insert an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::insert_empty(const ls::math::vec4& location, float extent, size_t depthLimit) noexcept
{
    return this->_insert_node(location, extent, depthLimit);
}



/*-------------------------------------
 * Insert an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::insert_empty(const ls::math::vec3& location, float extent, size_t depthLimit) noexcept
{
    return this->_insert_node(ls::math::vec4_cast(location, 0.f), extent, depthLimit);
}



/*-------------------------------------
 * Insert an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::insert(const ls::math::vec4& location, float extent, const T& value, size_t depthLimit) noexcept
{
    SL_OctreeNode<T, Allocator>* pNode = this->_insert_node(location, extent, depthLimit);
    pNode->data().push_back(value);
    return pNode;
}



/*-------------------------------------
 * Insert an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::insert(const ls::math::vec3& location, float extent, const T& value, size_t depthLimit) noexcept
{
    SL_OctreeNode<T, Allocator>* pNode = this->_insert_node(ls::math::vec4_cast(location, 0.f), extent, depthLimit);
    pNode->data().push_back(value);
    return pNode;
}



/*-------------------------------------
 * Insert an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::insert(const ls::math::vec4& location, float extent, T&& value, size_t depthLimit) noexcept
{
    SL_OctreeNode<T, Allocator>* pNode = this->_insert_node(location, extent, depthLimit);
    pNode->data().push_back(std::forward<T>(value));
    return pNode;
}



/*-------------------------------------
 * Insert an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::insert(const ls::math::vec3& location, float extent, T&& value, size_t depthLimit) noexcept
{
    SL_OctreeNode<T, Allocator>* pNode = this->_insert_node(ls::math::vec4_cast(location, 0.f), extent, depthLimit);
    pNode->data().push_back(std::forward<T>(value));
    return pNode;
}



/*-------------------------------------
 * Emplace an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename... Args>
inline SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::emplace(const ls::math::vec4& location, float extent, Args&&... args, size_t depthLimit) noexcept
{
    SL_OctreeNode<T, Allocator>* pNode = this->_insert_node(location, extent, depthLimit);
    pNode->data().emplace_back(std::forward<Args>(args)...);
    return pNode;
}



/*-------------------------------------
 * Emplace an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename... Args>
inline SL_OctreeNode<T, Allocator>* SL_Octree<T, MaxDepth, Allocator>::emplace(const ls::math::vec3& location, float exetent, Args&&... args, size_t depthLimit) noexcept
{
    SL_OctreeNode<T, Allocator>* pNode = this->_insert_node(ls::math::vec4_cast(location, 0.f), exetent, depthLimit);
    pNode->data().emplace_back(std::forward<Args>(args)...);
    return pNode;
}



#endif /* SL_OCTREE_HPP */
