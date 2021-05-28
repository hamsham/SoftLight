
/**
 * @brief Basic Octree interface
 *
 * @todo Finish implementing proper allocator usage
 *
 * @todo Permit the use of a secondary allocator for nodes only.
 */
#ifndef SL_OCTREE_HPP
#define SL_OCTREE_HPP

#include <vector>

#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"
#include "lightsky/math/vec_utils.h"



/**
 * @brief A Generic Octree container for spatial partitioning of general 3D
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
template <typename T, size_t MaxDepth, class Allocator = std::allocator<T>>
class SL_Octree
{
  private:
    ls::math::vec4 mOrigin;

    float mRadius;

    SL_Octree<T, MaxDepth, Allocator>* mNodes[8];

    std::vector<T, Allocator> mData;

  private:
    bool emplace_internal(const ls::math::vec4& location, float radius, T&& value, size_t currDepth) noexcept;

    template <typename ConstIterCallbackType>
    void iterate_from_bottom_internal(ConstIterCallbackType iterCallback, size_t currDepth) const noexcept;

    template <typename IterCallbackType>
    void iterate_from_bottom_internal(IterCallbackType iterCallback, size_t currDepth) noexcept;

    template <typename ConstIterCallbackType>
    void iterate_from_top_internal(ConstIterCallbackType iterCallback, size_t currDepth) const noexcept;

    template <typename IterCallbackType>
    void iterate_from_top_internal(IterCallbackType iterCallback, size_t currDepth) noexcept;

  public:
    /**
     * @brief Destructor.
     *
     * Frees all internal memory.
     */
    ~SL_Octree() noexcept;

    /**
     * @brief Constructor
     *
     * @param origin
     * The center of the octree in 3D space.
     *
     * @param radius
     * The radius of the top-level octree.
     */
    SL_Octree(const ls::math::vec3& origin = ls::math::vec3{0.f, 0.f, 0.f}, float radius = 1.f) noexcept;

    /**
     * @brief Constructor
     *
     * @param origin
     * The center of the octree in 3D space. The last (fourth) element of the
     * input vector is unused.
     *
     * @param radius
     * The radius of the top-level octree.
     */
    SL_Octree(const ls::math::vec4& origin, float radius) noexcept;

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
     * @brief Retrieve the user-defined origin of the top-level octree.
     *
     * Sub-trees will return their origin with respect to, and subdivided by,
     * the top-level octree.
     *
     * @return A reference to the 3D (4D-padded) octree's origin.
     */
    const ls::math::vec4& origin() const noexcept;

    /**
     * @brief Retrieve the radius of an internal bounding-sphere's 3D space.
     *
     * Sub-trees will return their radius with respect to, and subdivided by,
     * the top-level octree.
     *
     * @return A floating-point value representing the size of the current
     * octree node.
     */
    float radius() const noexcept;

    /**
     * @brief Retrieve the internal sub-trees.
     *
     * The returned list of pointers will not be null, but the objects
     * contained within the returned list may be null.
     *
     * @return A pointer to a list of 8 constant sub-trees.
     */
    const SL_Octree<T, MaxDepth, Allocator>* const* sub_nodes() const noexcept;

    /**
     * @brief Retrieve the internal sub-trees.
     *
     * The returned list of pointers will not be null, but the objects
     * contained within the returned list may be null.
     *
     * @return A pointer to a list of 8 sub-trees.
     */
    SL_Octree<T, MaxDepth, Allocator>** sub_nodes() noexcept;

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
     * @brief Retrieve the maximum allowable depth possible in this tree.
     *
     * @return The template parameter "MaxDepth".
     */
    constexpr size_t max_depth() const noexcept;

    /**
     * @brief Clear all memory, data, and sub-trees occupied by *this.
     */
    void clear() noexcept;

    /**
     * @brief Insert (copy) an object into *this, creating sub-tree partitions
     * if needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param radius
     * The maximum bounding-radius occupied by the input object.
     *
     * @param value
     * The data to be stored. This object must be copy-constructable,
     * copy-assignable, move-constructable, and move-assignable.
     *
     * @return TRUE if the data was successfully inserted into *this or a
     * sub-tree, FALSE if an error occurred.
     */
    bool insert(const ls::math::vec4& location, float radius, const T& value) noexcept;

    /**
     * @brief Insert (move) an object into *this, creating sub-tree partitions
     * if needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param radius
     * The maximum bounding-radius occupied by the input object.
     *
     * @param value
     * The data to be stored. This object must be copy-constructable,
     * copy-assignable, move-constructable, and move-assignable.
     *
     * @return TRUE if the data was successfully inserted into *this or a
     * sub-tree, FALSE if an error occurred.
     */
    bool emplace(const ls::math::vec4& location, float radius, T&& value) noexcept;

    /**
     * @brief Insert (copy) an object into *this, creating sub-tree partitions
     * if needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param radius
     * The maximum bounding-radius occupied by the input object.
     *
     * @param value
     * The data to be stored. This object must be copy-constructable,
     * copy-assignable, move-constructable, and move-assignable.
     *
     * @return TRUE if the data was successfully inserted into *this or a
     * sub-tree, FALSE if an error occurred.
     */
    bool insert(const ls::math::vec3& location, float radius, const T& value) noexcept;

    /**
     * @brief Insert (move) an object into *this, creating sub-tree partitions
     * if needed.
     *
     * @param location
     * The 3D spatial location of the object to be stored.
     *
     * @param radius
     * The maximum bounding-radius occupied by the input object.
     *
     * @param value
     * The data to be stored. This object must be copy-constructable,
     * copy-assignable, move-constructable, and move-assignable.
     *
     * @return TRUE if the data was successfully inserted into *this or a
     * sub-tree, FALSE if an error occurred.
     */
    bool emplace(const ls::math::vec3& location, float radius, T&& value) noexcept;

    /**
     * @brief Locate the closest sub-partition referenced by a point in 3D
     * space (const).
     *
     * @param location
     * A 3D point where data should be referenced.
     *
     * @return The closes sub-partition which contains the requested point.
     */
    const SL_Octree* find(const ls::math::vec4& location) const noexcept;

    /**
     * @brief Locate the closest sub-partition referenced by a point in 3D
     * space.
     *
     * @param location
     * A 3D point where data should be referenced.
     *
     * @return The closes sub-partition which contains the requested point.
     */
    SL_Octree* find(const ls::math::vec4& location) noexcept;

    /**
     * @brief Locate the closest sub-partition referenced by a point in 3D
     * space (const).
     *
     * @param location
     * A 3D point where data should be referenced.
     *
     * @return The closes sub-partition which contains the requested point.
     */
    const SL_Octree* find(const ls::math::vec3& location) const noexcept;

    /**
     * @brief Locate the closest sub-partition referenced by a point in 3D
     * space.
     *
     * @param location
     * A 3D point where data should be referenced.
     *
     * @return The closes sub-partition which contains the requested point.
     */
    SL_Octree* find(const ls::math::vec3& location) noexcept;

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
    void iterate_bottom_up(ConstIterCallbackType iterCallback) const noexcept;

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
    void iterate_bottom_up(IterCallbackType iterCallback) noexcept;

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
    void iterate_top_down(ConstIterCallbackType iterCallback) const noexcept;

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
    void iterate_top_down(IterCallbackType iterCallback) noexcept;
};



/*-------------------------------------
 * Destructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::~SL_Octree() noexcept
{
    clear();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::SL_Octree(const ls::math::vec3& origin, float radius) noexcept :
    mOrigin{ls::math::vec4_cast(origin, 0.f)},
    mRadius{radius},
    mNodes{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
    mData{}
{}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::SL_Octree(const ls::math::vec4& origin, float radius) noexcept :
    mOrigin{origin},
    mRadius{radius},
    mNodes{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
    mData{}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::SL_Octree(const SL_Octree& tree) noexcept :
    SL_Octree{}
{
    *this = tree;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>::SL_Octree(SL_Octree&& tree) noexcept :
    mOrigin{tree.mOrigin},
    mRadius{tree.mRadius},
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
    tree.mRadius = 0.f;

    for (unsigned i = 0; i < 8; ++i)
    {
        tree.mNodes[i] = nullptr;
    }
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>& SL_Octree<T, MaxDepth, Allocator>::operator=(const SL_Octree& tree) noexcept
{
    if (this == &tree)
    {
        return *this;
    }

    SL_Octree<T, MaxDepth, Allocator>* tempNodes[8] = {
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

    for (unsigned i = 0; i < 8; ++i)
    {
        if (!tree.mNodes[i])
        {
            continue;
        }

        tempNodes[i] = new(std::nothrow) SL_Octree<T, MaxDepth, Allocator>{*(tree.mNodes[i])};
        if (!tempNodes[i])
        {
            for (SL_Octree<T, MaxDepth, Allocator>* pTemp : tempNodes)
            {
                delete pTemp;
            }

            return *this;
        }

        tempNodes[i]->mData = tree.mNodes[i]->mData;
    }

    mOrigin = tree.mOrigin;
    mRadius = tree.mRadius;

    for (unsigned i = 0; i < 8; ++i)
    {
        mNodes[i] = tempNodes[i];
    }

    mData = tree.mData;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>& SL_Octree<T, MaxDepth, Allocator>::operator=(SL_Octree&& tree) noexcept
{
    if (this == &tree)
    {
        return *this;
    }

    mOrigin = tree.mOrigin;
    mRadius = tree.mRadius;

    for (unsigned i = 0; i < 8; ++i)
    {
        delete mNodes[i];
        mNodes[i] = tree.mNodes[i];
        tree.mNodes[i] = nullptr;
    }

    mData = std::move(tree.mData);

    return *this;
}



/*-------------------------------------
 * Get the origin
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline const ls::math::vec4& SL_Octree<T, MaxDepth, Allocator>::origin() const noexcept
{
    return mOrigin;
}



/*-------------------------------------
 * Get the radius
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline float SL_Octree<T, MaxDepth, Allocator>::radius() const noexcept
{
    return mRadius;
}



/*-------------------------------------
 * Get the sub-nodes (const)
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline const SL_Octree<T, MaxDepth, Allocator>* const* SL_Octree<T, MaxDepth, Allocator>::sub_nodes() const noexcept
{
    return mNodes;
}



/*-------------------------------------
 * Get the sub-nodes
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline SL_Octree<T, MaxDepth, Allocator>** SL_Octree<T, MaxDepth, Allocator>::sub_nodes() noexcept
{
    return mNodes;
}



/*-------------------------------------
 * Get the internal objects (const)
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline const std::vector<T, Allocator>& SL_Octree<T, MaxDepth, Allocator>::data() const noexcept
{
    return mData;
}



/*-------------------------------------
 * Get the internal objects
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline std::vector<T, Allocator>& SL_Octree<T, MaxDepth, Allocator>::data() noexcept
{
    return mData;
}



/*-------------------------------------
 * Emplace (move) an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
bool SL_Octree<T, MaxDepth, Allocator>::emplace_internal(const ls::math::vec4& location, float radius, T&& value, size_t currDepth) noexcept
{
    SL_Octree<T, MaxDepth, Allocator>* pTree = this;

    while (true)
    {
        // Don't even bother placing an object into sub-nodes if it can't fit
        const float r2 = pTree->mRadius * 0.5f;
        if (radius > r2 || currDepth == MaxDepth)
        {
            pTree->mData.push_back(value);
            return true;
        }

        // calculate a three-bit mask from the object's position and size. This
        // mask will be used as the index of a sub-node in the tree
        const ls::math::vec4&& localSpace = location - pTree->mOrigin;
        const ls::math::vec4&& ls0 = localSpace + radius;
        const ls::math::vec4&& ls1 = localSpace - radius;

        const int locations[8] = {
            ls::math::sign_mask(ls::math::vec4{ls0[0], ls0[1], ls0[2], 0.f}),
            ls::math::sign_mask(ls::math::vec4{ls1[0], ls0[1], ls0[2], 0.f}),
            ls::math::sign_mask(ls::math::vec4{ls0[0], ls1[1], ls0[2], 0.f}),
            ls::math::sign_mask(ls::math::vec4{ls1[0], ls1[1], ls0[2], 0.f}),
            ls::math::sign_mask(ls::math::vec4{ls0[0], ls0[1], ls1[2], 0.f}),
            ls::math::sign_mask(ls::math::vec4{ls1[0], ls0[1], ls1[2], 0.f}),
            ls::math::sign_mask(ls::math::vec4{ls0[0], ls1[1], ls1[2], 0.f}),
            ls::math::sign_mask(ls::math::vec4{ls1[0], ls1[1], ls1[2], 0.f}),
        };

        // determine if all the calculated masks match. unique masks mean the
        // object overlaps sub-nodes
        const int nodeId   = locations[0] | locations[1] | locations[2] | locations[3] | locations[4] | locations[5] | locations[6] | locations[7];
        const int overlaps = locations[0] & locations[1] & locations[2] & locations[3] & locations[4] & locations[5] & locations[6] & locations[7];

        // If an object intersects multiple sub-nodes, keep it in the current node
        // rather than split the object across the intersecting sub-nodes
        if (nodeId ^ overlaps)
        {
            pTree->mData.push_back(value);
            return true;
        }

        const float xSign = (nodeId & 0x01) ? -1.f : 1.f;
        const float ySign = (nodeId & 0x02) ? -1.f : 1.f;
        const float zSign = (nodeId & 0x04) ? -1.f : 1.f;

        const ls::math::vec4&& nodeLocation = pTree->mOrigin + ls::math::vec4{r2} * ls::math::vec4{xSign, ySign, zSign, 0.f};

        if (!pTree->mNodes[nodeId])
        {
            pTree->mNodes[nodeId] = new(std::nothrow) SL_Octree<T, MaxDepth, Allocator>(nodeLocation, r2);

            if (!pTree->mNodes[nodeId])
            {
                break;
            }
        }

        ++currDepth;
        pTree = pTree->mNodes[nodeId];
    }

    return false;
}



/*-------------------------------------
 * Get the current data size
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline size_t SL_Octree<T, MaxDepth, Allocator>::size() const noexcept
{
    return mData.size();
}



/*-------------------------------------
 * Get the number of local partitions
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
size_t SL_Octree<T, MaxDepth, Allocator>::breadth() const noexcept
{
    size_t count = 0;

    for (const SL_Octree<T, MaxDepth, Allocator>* pNode : mNodes)
    {
        count += pNode != nullptr;
    }

    return count;
}



/*-------------------------------------
 * Get the overall depth from *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline size_t SL_Octree<T, MaxDepth, Allocator>::depth() const noexcept
{
    size_t d = 0;
    for (const SL_Octree<T, MaxDepth, Allocator>* pNode : mNodes)
    {
        if (pNode)
        {
            d = ls::math::max(d, 1+pNode->depth());
        }
    }

    return d;
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
 * Clear all data & memory
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
void SL_Octree<T, MaxDepth, Allocator>::clear() noexcept
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
 * Insert an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline bool SL_Octree<T, MaxDepth, Allocator>::insert(const ls::math::vec4& location, float radius, const T& value) noexcept
{
    return this->emplace_internal(location, radius, T{value}, 0);
}



/*-------------------------------------
 * Emplace an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline bool SL_Octree<T, MaxDepth, Allocator>::emplace(const ls::math::vec4& location, float radius, T&& value) noexcept
{
    return this->emplace_internal(location, radius, value, 0);
}



/*-------------------------------------
 * Insert an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline bool SL_Octree<T, MaxDepth, Allocator>::insert(const ls::math::vec3& location, float radius, const T& value) noexcept
{
    return this->insert(ls::math::vec4_cast(location, 0.f), radius, value);
}



/*-------------------------------------
 * Emplace an object into *this
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline bool SL_Octree<T, MaxDepth, Allocator>::emplace(const ls::math::vec3& location, float radius, T&& value) noexcept
{
    return this->emplace(ls::math::vec4_cast(location, 0.f), radius, value);
}



/*-------------------------------------
 * Find a sub-node at a location (const)
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
const SL_Octree<T, MaxDepth, Allocator>* SL_Octree<T, MaxDepth, Allocator>::find(const ls::math::vec4& location) const noexcept
{
    const int nodeId = ls::math::sign_mask(location - mOrigin);

    if (!mNodes[nodeId])
    {
        return this;
    }

    return mNodes[nodeId]->find(location);
}



/*-------------------------------------
 * Find a sub-node at a location
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
SL_Octree<T, MaxDepth, Allocator>* SL_Octree<T, MaxDepth, Allocator>::find(const ls::math::vec4& location) noexcept
{
    const int nodeId = ls::math::sign_mask(location - mOrigin);

    if (!mNodes[nodeId])
    {
        return this;
    }

    return mNodes[nodeId]->find(location);
}



/*-------------------------------------
 * Find a sub-node at a location (const)
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline const SL_Octree<T, MaxDepth, Allocator>* SL_Octree<T, MaxDepth, Allocator>::find(const ls::math::vec3& location) const noexcept
{
    return this->find(ls::math::vec4_cast(location, 0.f));
}



/*-------------------------------------
 * Find a sub-node at a location
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
inline SL_Octree<T, MaxDepth, Allocator>* SL_Octree<T, MaxDepth, Allocator>::find(const ls::math::vec3& location) noexcept
{
    return this->find(ls::math::vec4_cast(location, 0.f));
}



/*-------------------------------------
 * Internal Depth-first iteration (const)
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename ConstIterCallbackType>
void SL_Octree<T, MaxDepth, Allocator>::iterate_from_bottom_internal(ConstIterCallbackType iterCallback, size_t currDepth) const noexcept
{
    for (const SL_Octree<T, MaxDepth, Allocator>* pNode : mNodes)
    {
        if (pNode)
        {
            pNode->iterate_from_bottom_internal<ConstIterCallbackType>(iterCallback, currDepth+1);
        }
    }

    iterCallback(this, currDepth);
}



/*-------------------------------------
 * Internal Depth-first iteration
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename IterCallbackType>
void SL_Octree<T, MaxDepth, Allocator>::iterate_from_bottom_internal(IterCallbackType iterCallback, size_t currDepth) noexcept
{
    for (const SL_Octree<T, MaxDepth, Allocator>* pNode : mNodes)
    {
        if (pNode)
        {
            pNode->iterate_from_bottom_internal<IterCallbackType>(iterCallback, currDepth+1);
        }
    }

    iterCallback(this, currDepth);
}



/*-------------------------------------
 * Depth-first iteration (const)
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename ConstIterCallbackType>
inline void SL_Octree<T, MaxDepth, Allocator>::iterate_bottom_up(ConstIterCallbackType iterCallback) const noexcept
{
    this->iterate_from_bottom_internal<ConstIterCallbackType>(iterCallback, 0);
}



/*-------------------------------------
 * Depth-first iteration
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename IterCallbackType>
inline void SL_Octree<T, MaxDepth, Allocator>::iterate_bottom_up(IterCallbackType iterCallback) noexcept
{
    this->iterate_from_bottom_internal<IterCallbackType>(iterCallback, 0);
}



/*-------------------------------------
 * Internal Breadth-first iteration (const)
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename ConstIterCallbackType>
void SL_Octree<T, MaxDepth, Allocator>::iterate_from_top_internal(ConstIterCallbackType iterCallback, size_t currDepth) const noexcept
{
    if (!iterCallback(this, currDepth))
    {
        return;
    }

    for (const SL_Octree<T, MaxDepth, Allocator>* pNode : mNodes)
    {
        if (pNode)
        {
            pNode->iterate_from_top_internal<ConstIterCallbackType>(iterCallback, currDepth+1);
        }
    }
}



/*-------------------------------------
 * Internal Top-down iteration
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename IterCallbackType>
void SL_Octree<T, MaxDepth, Allocator>::iterate_from_top_internal(IterCallbackType iterCallback, size_t currDepth) noexcept
{
    if (!iterCallback(this, currDepth))
    {
        return;
    }

    for (const SL_Octree<T, MaxDepth, Allocator>* pNode : mNodes)
    {
        if (pNode)
        {
            pNode->iterate_from_top_internal<IterCallbackType>(iterCallback, currDepth+1);
        }
    }
}



/*-------------------------------------
 * Top-down iteration (const)
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename ConstIterCallbackType>
inline void SL_Octree<T, MaxDepth, Allocator>::iterate_top_down(ConstIterCallbackType iterCallback) const noexcept
{
    this->iterate_from_top_internal<ConstIterCallbackType>(iterCallback, 0);
}



/*-------------------------------------
 * Top-down iteration
-------------------------------------*/
template <typename T, size_t MaxDepth, class Allocator>
template <typename IterCallbackType>
inline void SL_Octree<T, MaxDepth, Allocator>::iterate_top_down(IterCallbackType iterCallback) noexcept
{
    this->iterate_from_top_internal<IterCallbackType>(iterCallback, 0);
}



#endif /* SL_OCTREE_HPP */
