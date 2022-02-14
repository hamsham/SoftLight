
#ifndef SL_SPATIALHIERARCHY_HPP
#define SL_SPATIALHIERARCHY_HPP

#include <algorithm>
#include <iterator> // std::back_inserter()
#include <utility> // std::move(), std::forward()
#include <vector>



/**
 * @brief Spatial Hierarchy objects can be used to instantiate basic tree
 * structures such as a scene graph, for example.
 */
template <typename T, typename ValueAllocator = std::allocator<T>, typename IndexAllocator = std::allocator<std::size_t>>
class SL_SpatialHierarchy
{
  public:
    enum : std::size_t
    {
        ROOT_NODE_INDEX = ~(std::size_t)0
    };

    typedef T value_type;
    typedef ValueAllocator value_allocator_type;
    typedef IndexAllocator index_allocator_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t different_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef typename std::allocator_traits<ValueAllocator>::pointer pointer;
    typedef typename std::allocator_traits<ValueAllocator>::const_pointer const_pointer;
    typedef typename std::vector<T, ValueAllocator>::iterator iterator;
    typedef typename std::vector<T, ValueAllocator>::const_iterator const_iterator;
    typedef typename std::vector<T, ValueAllocator>::reverse_iterator reverse_iterator;
    typedef typename std::vector<T, ValueAllocator>::const_reverse_iterator const_reverse_iterator;

  private: // member objects
    // This vector contains the IDs of all parent nodes.
    std::vector<size_type, IndexAllocator> mParents;

    std::vector<T, ValueAllocator> mNodes;

    template <class RandomIter>
    static void _rotate_right(RandomIter iter, size_t size, size_t numRotations) noexcept;

    size_type _rearrange_for_insert(size_type parentIndex, size_type numNewNodes);

  public:
    ~SL_SpatialHierarchy() = default;

    SL_SpatialHierarchy();

    SL_SpatialHierarchy(const SL_SpatialHierarchy& s);

    SL_SpatialHierarchy(SL_SpatialHierarchy&& s) noexcept;

    SL_SpatialHierarchy& operator=(const SL_SpatialHierarchy& s);

    SL_SpatialHierarchy& operator=(SL_SpatialHierarchy&& s) noexcept;

    index_allocator_type get_index_allocator() const { return mParents.get_allocator(); }
    value_allocator_type get_value_allocator() const { return mNodes.get_allocator(); }

    const_iterator cbegin() noexcept { return mNodes.cbegin(); }
    const_iterator cend() noexcept { return mNodes.cend(); }

    iterator begin() noexcept { return mNodes.begin(); }
    const_iterator begin() const noexcept { return mNodes.begin(); }
    iterator end() noexcept { return mNodes.end(); }
    const_iterator end() const noexcept { return mNodes.end(); }

    const_reverse_iterator crbegin() noexcept { return mNodes.crbegin(); }
    const_reverse_iterator crend() noexcept { return mNodes.crend(); }

    reverse_iterator rbegin() noexcept { return mNodes.rbegin(); }
    reverse_iterator rend() noexcept { return mNodes.rend(); }

    const_reference operator[](size_type index) const { return mNodes[index]; }
    reference operator[](size_type index) { return mNodes[index]; }

    const_pointer data() const { return mNodes.data(); }
    pointer data() { return mNodes.data(); }

    void clear();

    size_type insert(size_type parentIndex, T&& node);

    template <typename... Args>
    size_type emplace(size_type parentIndex, Args&&... args);

    size_type erase(size_type nodeIndex);

    size_type parent(size_type nodeIndex) const;

    bool reparent(size_type nodeIndex, size_type newParentId);

    bool duplicate(size_type nodeIndex);

    size_type find(const T&) const noexcept;

    size_type total_children(size_type nodeIndex) const noexcept;

    size_type immediate_children(size_type nodeIndex) const noexcept;

    bool is_descendant(size_type nodeIndex, size_type parentId) const noexcept;

    size_type size() const noexcept;

    size_type capacity() const noexcept;

    bool empty() const noexcept;

    void reserve(size_type numElements);
};



/*
template <typename T, typename ValueAllocator, typename IndexAllocator>
SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::~SL_SpatialHierarchy()
{
}
*/



template <typename T, typename ValueAllocator, typename IndexAllocator>
SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::SL_SpatialHierarchy() :
    mParents{},
    mNodes{}
{}



template <typename T, typename ValueAllocator, typename IndexAllocator>
SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::SL_SpatialHierarchy(const SL_SpatialHierarchy& s) :
    mParents{s.mParents},
    mNodes{s.mNodes}
{}



template <typename T, typename ValueAllocator, typename IndexAllocator>
SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::SL_SpatialHierarchy(SL_SpatialHierarchy&& s) noexcept :
    mParents{std::move(s.mParents)},
    mNodes{std::move(s.mNodes)}
{}



template <typename T, typename ValueAllocator, typename IndexAllocator>
SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>& SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::operator=(const SL_SpatialHierarchy& s)
{
    if (this != &s)
    {
        mParents = s.mParents;
        mNodes = s.mNodes;
    }

    return *this;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>& SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::operator=(SL_SpatialHierarchy&& s) noexcept
{
    if (this != &s)
    {
        mParents = std::move(s.mParents);
        mNodes = std::move(s.mNodes);
    }

    return *this;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
void SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::clear()
{
    mParents.clear();
    mNodes.clear();
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
template <class RandomIter>
void SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::_rotate_right(RandomIter iter, size_t size, size_t numRotations) noexcept
{
    numRotations = numRotations % size;
    if (!numRotations)
    {
        return;
    }

    const size_t numSwaps = size*numRotations+numRotations;
    typename std::iterator_traits<RandomIter>::value_type prev = iter[size - numRotations];

    for (size_t i = 0; i < numSwaps; ++i)
    {
        const size_t j = (i + numRotations) % size;
        typename std::iterator_traits<RandomIter>::value_type temp = iter[j];

        iter[j] = prev;
        prev = temp;
    }

    *iter = prev;
}


template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::_rearrange_for_insert(size_type parentIndex, size_type numNewNodes)
{
    if (parentIndex >= mParents.size() && parentIndex != ROOT_NODE_INDEX)
    {
        return ROOT_NODE_INDEX;
    }

    size_type oldSize = mParents.size();
    mParents.resize(mParents.size()+numNewNodes);
    mNodes.resize(mNodes.size()+numNewNodes);

    // scan for the first sibling node of a parent, so we can insert new nodes
    // towards the end of the node array. This should save on the total number
    // of nodes moved to another location
    size_type insertIndex = parentIndex != ROOT_NODE_INDEX ? (parentIndex+total_children(parentIndex)+1) : oldSize;
    size_type endIter = mNodes.size()-1;
    size_type beginIter = insertIndex;

    for (size_type i = endIter; i > beginIter; --i)
    {
        size_type newParentId = mParents[i-numNewNodes];

        if (newParentId > parentIndex && newParentId != ROOT_NODE_INDEX)
        {
            newParentId += numNewNodes;
        }

         mParents[i] = newParentId;
         mNodes[i] = std::move(mNodes[i-numNewNodes]);
    }

    return insertIndex;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::insert(size_type parentIndex, T&& node)
{
    if (parentIndex == ROOT_NODE_INDEX)
    {
        mParents.push_back(parentIndex);
        mNodes.push_back(std::forward<T>(node));
        return mNodes.size()-1;
    }

    size_type insertIndex = _rearrange_for_insert(parentIndex, 1);
    if (insertIndex == ROOT_NODE_INDEX)
    {
        return ROOT_NODE_INDEX;
    }

    mParents[insertIndex] = parentIndex;
    mNodes[insertIndex] = std::forward<T>(node);

    return insertIndex;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
template <typename... Args>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::emplace(size_type parentIndex, Args&&... args)
{
    if (parentIndex == ROOT_NODE_INDEX)
    {
        mParents.push_back(parentIndex);
        mNodes.emplace_back(std::forward<Args>(args)...);
        return mNodes.size()-1;
    }

    size_type insertIndex = _rearrange_for_insert(parentIndex, 1);
    if (insertIndex == ROOT_NODE_INDEX)
    {
        return ROOT_NODE_INDEX;
    }

    mParents[insertIndex] = parentIndex;
    mNodes[insertIndex] = T{std::forward<Args>(args)...};

    return insertIndex;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::erase(size_type nodeIndex)
{
    if (nodeIndex == ROOT_NODE_INDEX)
    {
        size_type numDeleted = mParents.size();
        clear();
        return numDeleted;
    }

    size_type numChildren = total_children(nodeIndex);
    size_type lastNodeDeleted = nodeIndex+numChildren;
    size_type totalNodesDeleted = numChildren+1;

    if (lastNodeDeleted == mParents.size()-1)
    {
        mParents.resize(nodeIndex);
        mNodes.resize(nodeIndex);
        return totalNodesDeleted;
    }

    for (size_type i = nodeIndex, j = lastNodeDeleted+1; j < mParents.size(); ++i, ++j)
    {
        if (mParents[j] > nodeIndex && mParents[j] != ROOT_NODE_INDEX)
        {
            mParents[i] = mParents[j] - totalNodesDeleted;
        }
        else
        {
            mParents[i] = mParents[j];
        }

        mNodes[i] = mNodes[j];
    }

    mParents.resize(mParents.size()-totalNodesDeleted);
    mNodes.resize(mNodes.size()-totalNodesDeleted);

    return totalNodesDeleted;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::parent(size_type nodeIndex) const
{
    return mParents[nodeIndex];
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
bool SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::reparent(size_type nodeIndex, size_type newParentId)
{
    // data validation & early exits
    if (nodeIndex == ROOT_NODE_INDEX || nodeIndex >= mParents.size() || mParents[nodeIndex] == newParentId)
    {
        return false;
    }

    if (newParentId >= mParents.size() && newParentId != ROOT_NODE_INDEX)
    {
        return false;
    }

    // Cannot make a node a parent of its ancestor. It's possible, but then
    // what would the new ancestor of the node then be?
    if (newParentId != ROOT_NODE_INDEX)
    {
        if (is_descendant(newParentId, nodeIndex))
        {
            return false;
        }
    }

    const size_t numChildren    = total_children(nodeIndex);
    const size_t displacement   = 1 + numChildren;
    const size_t numNewSiblings = total_children(newParentId);
    const size_t newNodeIndex   = 1 + newParentId + numNewSiblings;

    // Keep track of the range of elements which need to be updated.
    const size_t effectStart = nodeIndex < newParentId ? nodeIndex : newNodeIndex;
    const size_t effectEnd   = nodeIndex < newParentId ? newNodeIndex : (nodeIndex+displacement);
    const size_t numAffected = effectEnd - effectStart;

    // Determine if we're moving "up", closer to the root, or "down" away from
    // the root
    const bool movingUp = newParentId < nodeIndex && (newParentId != ROOT_NODE_INDEX);
    size_t amountToMove;

    if (movingUp)
    {
        amountToMove = nodeIndex - newNodeIndex;
    }
    else
    {
        amountToMove = newNodeIndex - nodeIndex;
    }

    for (size_t i = effectStart; i < effectEnd; ++i)
    {
        size_t& rParentId = mParents[i];
        size_t  pId  = rParentId;
        size_t  nId  = i;

        // Update the requested node's index
        if (nId == nodeIndex)
        {
            if (newParentId != ROOT_NODE_INDEX)
            {
                pId = newParentId - (nodeIndex < newParentId ? displacement : 0);
            }
            else
            {
                pId = newParentId;
            }
        }
        else
        {
            // Determine if there's a node which even needs its parent ID updated.
            if (pId == ROOT_NODE_INDEX || pId < effectStart)
            {
                continue;
            }

            if (movingUp)
            {
                if (nId < nodeIndex)
                {
                    pId += displacement;
                }
                else
                {
                    pId -= amountToMove;
                }
            }
            else
            {
                if (i > nodeIndex+numChildren)
                {
                    pId -= displacement;
                }
                else
                {
                    pId += amountToMove-displacement;
                }
            }
        }

        rParentId = pId;
    }

    if (nodeIndex > newParentId)
    {
        for (size_t i = effectEnd; i < mParents.size(); ++i)
        {
            if (mParents[i] < nodeIndex)
            {
                mParents[i] += displacement;
            }
            else if (mParents[i] <= newParentId || mParents[i] == ROOT_NODE_INDEX)
            {
                break;
            }
        }
    }

    const size_t numRotations = movingUp ? displacement : (numAffected-displacement);
    _rotate_right(mParents.data() + effectStart, numAffected, numRotations);
    _rotate_right(mNodes.data()   + effectStart, numAffected, numRotations);

    return true;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
bool SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::duplicate(size_type nodeIndex)
{
    if (nodeIndex == ROOT_NODE_INDEX || nodeIndex >= mParents.size())
    {
        return false;
    }

    size_type totalChildren  = total_children(nodeIndex);
    size_type totalNodes     = 1 + totalChildren;
    size_type insertedOffset = mNodes.size();
    //size_type offsetDifference = mNodes.size() - nodeIndex;

    // To keep things simple, insert the new node at the end of the arrays,
    // parented to the root, then reparent
    typename std::vector<size_type, IndexAllocator>::iterator&& parentBegin = mParents.begin() + nodeIndex;
    std::copy(parentBegin, parentBegin+totalNodes, std::back_inserter(mParents));

    iterator&& nodeBegin = mNodes.begin() + nodeIndex;
    std::copy(nodeBegin, nodeBegin+totalNodes, std::back_inserter(mNodes));

    mParents[insertedOffset] = ROOT_NODE_INDEX;
    for (size_type i = insertedOffset+1; i < mNodes.size(); ++i)
    {
        mParents[i] = (mParents[i]-nodeIndex) + insertedOffset;
    }

    return reparent(insertedOffset, mParents[nodeIndex]);
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::find(const T& node) const noexcept
{
    for (size_t iter = 0; iter < mNodes.size(); ++iter)
    {
        if (mNodes[iter] == node)
        {
            return iter;
        }
    }

    return ROOT_NODE_INDEX;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::total_children(size_type nodeIndex) const noexcept
{
    if (nodeIndex == ROOT_NODE_INDEX)
    {
        return mParents.size();
    }

    size_type numChildren = 0;
    for (size_type i = nodeIndex+1; i < mParents.size(); ++i, ++numChildren)
    {
        if (mParents[i] < nodeIndex || mParents[i] == ROOT_NODE_INDEX)
        {
            break;
        }
    }

    return numChildren;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::immediate_children(size_type nodeIndex) const noexcept
{
    if (nodeIndex == ROOT_NODE_INDEX)
    {
        return mParents.size();
    }

    size_type numChildren = 0;
    for (size_type i = nodeIndex+1; i < mParents.size(); ++i)
    {
        if (mParents[i] < nodeIndex || mParents[i] == ROOT_NODE_INDEX)
        {
            break;
        }
        else if (mParents[i] == nodeIndex)
        {
            ++numChildren;
        }
    }

    return numChildren;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
bool SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::is_descendant(size_type nodeIndex, size_type parentId) const noexcept
{
    if (parentId == ROOT_NODE_INDEX)
    {
        return true;
    }

    if (nodeIndex == ROOT_NODE_INDEX || nodeIndex == parentId)
    {
        return false;
    }

    for (size_type iter = nodeIndex; iter > parentId && iter != ROOT_NODE_INDEX; iter = mParents[iter])
    {
        if (iter == parentId)
        {
            return true;
        }
    }

    return false;
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size() const noexcept
{
    return mParents.size();
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
typename SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::size_type SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::capacity() const noexcept
{
    return mParents.capacity();
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
bool SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::empty() const noexcept
{
    return mParents.empty();
}



template <typename T, typename ValueAllocator, typename IndexAllocator>
void SL_SpatialHierarchy<T, ValueAllocator, IndexAllocator>::reserve(size_type numElements)
{
    mParents.resize(numElements);
    mNodes.reserve(numElements);
}



#endif /* SL_SPATIALHIERARCHY_HPP */
