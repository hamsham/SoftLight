
#include <algorithm> // std::rotate()
#include <iterator> // std::iterator_traits

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Log.h"

#include "softlight/SL_Animation.hpp"
#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_SceneNode.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous utility functions
-----------------------------------------------------------------------------*/
namespace
{

template <class RandomIter>
void rotate_right(RandomIter nums, size_t size, size_t numRotations)
{
    numRotations = numRotations % size;
    if (!numRotations)
    {
        return;
    }

    const size_t numSwaps = size*numRotations+numRotations;
    typename std::iterator_traits<RandomIter>::value_type prev = nums[size - numRotations];

    for (size_t i = 0; i < numSwaps; ++i)
    {
        const size_t j = (i + numRotations) % size;
        typename std::iterator_traits<RandomIter>::value_type temp = nums[j];

        nums[j] = prev;
        prev = temp;
    }

    *nums = prev;
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_SceneGraph Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_SceneGraph::~SL_SceneGraph() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_SceneGraph::SL_SceneGraph() noexcept :
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
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_SceneGraph::SL_SceneGraph(const SL_SceneGraph& s) noexcept
{
    *this = s;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_SceneGraph::SL_SceneGraph(SL_SceneGraph&& s) noexcept
{
    *this = std::move(s);
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_SceneGraph& SL_SceneGraph::operator=(const SL_SceneGraph& s) noexcept
{
    mContext = s.mContext;
    mNodeParentIds = s.mNodeParentIds;
    mNodes = s.mNodes;
    mNodeNames = s.mNodeNames;
    mBaseTransforms = s.mBaseTransforms;
    mCurrentTransforms = s.mCurrentTransforms;
    mModelMatrices = s.mModelMatrices;
    mNumNodeMeshes = s.mNumNodeMeshes;

    LS_DEBUG_ASSERT(s.mNumNodeMeshes.size() == s.mNodeMeshes.size());
    mNumNodeMeshes = s.mNumNodeMeshes;

    mNodeMeshes.reserve(s.mNodeMeshes.size());

    for (size_t i = 0; i < s.mNodeMeshes.size(); ++i)
    {
        const ls::utils::Pointer<size_t[]>& inMeshes = s.mNodeMeshes[i];
        const size_t inMeshCount = s.mNumNodeMeshes[i];

        LS_DEBUG_ASSERT(inMeshCount > 0);
        mNodeMeshes.emplace_back(new size_t[inMeshCount]);

        ls::utils::Pointer<size_t[]>& outMeshes = mNodeMeshes.back();
        ls::utils::fast_copy(outMeshes.get(), inMeshes.get(), inMeshCount);
    }

    mMeshes = s.mMeshes;
    mMaterials = s.mMaterials;
    mMeshBounds = s.mMeshBounds;
    mInvBoneTransforms = s.mInvBoneTransforms;
    mBoneOffsets = s.mBoneOffsets;
    mCameras = s.mCameras;
    mAnimations = s.mAnimations;
    mNodeAnims = s.mNodeAnims;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_SceneGraph& SL_SceneGraph::operator=(SL_SceneGraph&& s) noexcept
{
    mContext = std::move(s.mContext);
    mNodeParentIds = std::move(s.mNodeParentIds);
    mNodes = std::move(s.mNodes);
    mNodeNames = std::move(s.mNodeNames);
    mBaseTransforms = std::move(s.mBaseTransforms);
    mCurrentTransforms = std::move(s.mCurrentTransforms);
    mModelMatrices = std::move(s.mModelMatrices);
    mNumNodeMeshes = std::move(s.mNumNodeMeshes);
    mNodeMeshes = std::move(s.mNodeMeshes);
    mMeshes = std::move(s.mMeshes);
    mMaterials = std::move(s.mMaterials);
    mMeshBounds = std::move(s.mMeshBounds);
    mInvBoneTransforms = std::move(s.mInvBoneTransforms);
    mBoneOffsets = std::move(s.mBoneOffsets);
    mCameras = std::move(s.mCameras);
    mAnimations = std::move(s.mAnimations);
    mNodeAnims = std::move(s.mNodeAnims);

    return *this;
}



/*-------------------------------------
 * Terminate
-------------------------------------*/
void SL_SceneGraph::terminate() noexcept
{
    mContext.terminate();
    mNodeParentIds.clear();
    mNodes.clear();
    mNodeNames.clear();
    mBaseTransforms.clear();
    mCurrentTransforms.clear();
    mModelMatrices.clear();
    mNumNodeMeshes.clear();
    mNodeMeshes.clear();
    mMeshes.clear();
    mMaterials.clear();
    mMeshBounds.clear();
    mInvBoneTransforms.clear();
    mBoneOffsets.clear();
    mCameras.clear();
    mAnimations.clear();
    mNodeAnims.clear();
}



/*-------------------------------------
 * Node updating
-------------------------------------*/
void SL_SceneGraph::update_node_transform(const size_t transformId) noexcept
{
    SL_Transform& t = mCurrentTransforms[transformId];

    const size_t parentId = mNodeParentIds[transformId];
    const bool doesParentExist = parentId != SCENE_NODE_ROOT_ID;

    if (doesParentExist)
    {
        SL_Transform& pt = mCurrentTransforms[parentId];
        t.apply_pre_transform(pt.transform());
    }
    else
    {
        t.apply_transform();
    }

    if (mNodes[transformId].type == NODE_TYPE_BONE)
    {
        // bones should be part of a skeleton, not tied to meshes or cameras
        LS_DEBUG_ASSERT(mNodes[parentId].type == NODE_TYPE_BONE || mNodes[parentId].type == NODE_TYPE_EMPTY);

        const size_t boneId = mNodes[transformId].dataId;
        mModelMatrices[transformId] = mInvBoneTransforms[boneId] * t.transform() * mBoneOffsets[boneId];
    }
    else
    {
        mModelMatrices[transformId] = t.transform();
    }
}



/*-------------------------------------
 * Scene Updating
-------------------------------------*/
void SL_SceneGraph::update() noexcept
{
    // Transformation indices have a 1:1 relationship with node indices.
    // Child nodes always have a parent ID which is less-than their node ID.
    // If the child node is less than the parent node's ID, we have ourselves
    // a good-old-fashioned bug.
    const size_t numNodes = mCurrentTransforms.size();
    std::size_t* const pParentIds = mNodeParentIds.data();
    SL_Transform* const pTransforms = mCurrentTransforms.data();

    for (size_t i = 0; i < numNodes; ++i)
    {
        // If a parent node is dirty, update all child nodes, iteratively
        if (pTransforms[i].is_dirty())
        {
            update_node_transform(i);

            for (size_t j = i+1; j < numNodes; ++j)
            {
                if (pParentIds[j] < i)
                {
                    i = j-1;
                    break;
                }
                else
                {
                    // force an update to the child node.
                    update_node_transform(j);
                }
            }
        }
    }

    for (SL_Camera& cam : mCameras)
    {
        if (cam.is_dirty())
        {
            cam.update();
        }
    }
}



/*-------------------------------------
 * Mesh Node Deletion
-------------------------------------*/
void SL_SceneGraph::delete_mesh_node_data(const size_t nodeDataId) noexcept
{
    LS_DEBUG_ASSERT(mNumNodeMeshes.size() == mNodeMeshes.size());

    if (mNumNodeMeshes.size() > 1)
    {
        size_t lastDataIndex = mNumNodeMeshes.size()-1;
        mNumNodeMeshes[nodeDataId] = mNumNodeMeshes.back();
        mNodeMeshes[nodeDataId] = std::move(mNodeMeshes.back());

        for (SL_SceneNode& node : mNodes)
        {
            if (node.type == SL_SceneNodeType::NODE_TYPE_MESH && node.dataId == lastDataIndex)
            {
                node.dataId = nodeDataId;
                break;
            }
        }
    }

    mNumNodeMeshes.pop_back();
    mNodeMeshes.pop_back();
}



/*-------------------------------------
 * Bone Node Deletion
-------------------------------------*/
void SL_SceneGraph::delete_bone_node_data(const size_t nodeDataId) noexcept
{
    LS_DEBUG_ASSERT(mInvBoneTransforms.size() == mBoneOffsets.size());

    if (mInvBoneTransforms.size() > 1)
    {
        size_t lastDataIndex = mInvBoneTransforms.size()-1;
        mInvBoneTransforms[nodeDataId] = mInvBoneTransforms.back();
        mBoneOffsets[nodeDataId] = mBoneOffsets.back();

        for (SL_SceneNode& node : mNodes)
        {
            if (node.type == SL_SceneNodeType::NODE_TYPE_BONE && node.dataId == lastDataIndex)
            {
                node.dataId = nodeDataId;
                break;
            }
        }
    }

    mInvBoneTransforms.pop_back();
    mBoneOffsets.pop_back();
}



/*-------------------------------------
 * Camera Deletion
-------------------------------------*/
void SL_SceneGraph::delete_camera_node_data(const size_t nodeDataId) noexcept
{
    if (mCameras.size() > 1)
    {
        size_t lastDataIndex = mCameras.size()-1;
        mCameras[nodeDataId] = mCameras.back();

        for (SL_SceneNode& node : mNodes)
        {
            if (node.type == SL_SceneNodeType::NODE_TYPE_CAMERA && node.dataId == lastDataIndex)
            {
                node.dataId = nodeDataId;
                break;
            }
        }
    }

    mCameras.pop_back();
}



/*-------------------------------------
 * SL_Animation Deletion
-------------------------------------*/
void SL_SceneGraph::delete_node_animation_data(const size_t nodeId, bool includeChildren) noexcept
{
    // Remove all animation channels associated with the current node.
    const size_t numChildren = includeChildren ? num_total_children(nodeId) : 0;
    const size_t firstNode = nodeId;
    const size_t lastNode = nodeId + numChildren;
    const size_t totalNodes = 1 + numChildren;

    for (size_t i = mAnimations.size(); i--;)
    {

        // Animations contain transformation IDs which may need to be
        // decremented. Search for transformation indices with a value greater
        // than the current node's nodeId.
        SL_Animation& currentAnim = mAnimations[i];

        // friendly class member manipulation
        std::vector<size_t>& currentTransIds = currentAnim.mTransformIds;

        // Reduce the animation ID of all animations in *this.
        for (size_t j = currentTransIds.size(); j--;)
        {
            if (currentTransIds[j] >= firstNode && currentTransIds[j] <= lastNode)
            {
                currentAnim.erase(j);
            }
            else if (currentTransIds[j] > lastNode)
            {
                currentTransIds[j] -= totalNodes;
            }
        }

        // Remove any defunct animations
        if (!currentAnim.size())
        {
            mAnimations.erase(mAnimations.begin() + i);
        }
    }
}



/*-------------------------------------
 * Delete all nodes
-------------------------------------*/
void SL_SceneGraph::clear_node_data() noexcept
{
    mNodeParentIds.clear();
    mNodes.clear();
    mNodeNames.clear();
    mBaseTransforms.clear();
    mCurrentTransforms.clear();
    mModelMatrices.clear();

    mNumNodeMeshes.clear();
    mNodeMeshes.clear();

    mInvBoneTransforms.clear();
    mBoneOffsets.clear();

    mCameras.clear();

    mAnimations.clear();
    mNodeAnims.clear();
}



/*-------------------------------------
 * Node Deletion
-------------------------------------*/
size_t SL_SceneGraph::delete_node(const size_t nodeIndex) noexcept
{
    size_t numDeleted;

    if (nodeIndex == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        numDeleted = mNodes.size();
        clear_node_data();
        return numDeleted;
    }

    // No mercy for client code
    LS_DEBUG_ASSERT(nodeIndex < mNodes.size());

    size_t numChildren = num_total_children(nodeIndex);
    size_t lastNode = nodeIndex+numChildren;
    numDeleted = 1 + numChildren;

    // find all child nodes and gather their types
    // FIXME: O(n^2)
    for (size_t i = nodeIndex; i < mNodes.size(); ++i)
    {
        if (i <= nodeIndex+numChildren)
        {
            switch (mNodes[i].type)
            {
                case SL_SceneNodeType::NODE_TYPE_CAMERA:
                    delete_camera_node_data(i);
                    break;

                case SL_SceneNodeType::NODE_TYPE_MESH:
                    delete_mesh_node_data(i);
                    break;

                case SL_SceneNodeType::NODE_TYPE_BONE:
                    delete_bone_node_data(i);
                    break;

                case SL_SceneNodeType::NODE_TYPE_EMPTY:
                    break;
            }
        }
    }

    // Delete the actual node
    mNodeParentIds.erase(mNodeParentIds.begin()+nodeIndex,         mNodeParentIds.begin()+nodeIndex+numChildren+1);
    mNodes.erase(mNodes.begin()+nodeIndex,                         mNodes.begin()+nodeIndex+numChildren+1);
    mNodeNames.erase(mNodeNames.begin()+nodeIndex,                 mNodeNames.begin()+nodeIndex+numChildren+1);
    mBaseTransforms.erase(mBaseTransforms.begin()+nodeIndex,       mBaseTransforms.begin()+nodeIndex+numChildren+1);
    mCurrentTransforms.erase(mCurrentTransforms.begin()+nodeIndex, mCurrentTransforms.begin()+nodeIndex+numChildren+1);
    mModelMatrices.erase(mModelMatrices.begin()+nodeIndex,         mModelMatrices.begin()+nodeIndex+numChildren+1);

    // early exit in case there are no animations tied to the current node.
    delete_node_animation_data(nodeIndex, true);

    // Decrement all node ID and data ID indices that are greater than those in
    // the current node. Also deal with the last bit of transformation data in
    // case a recursive deletion is in required.
    for (size_t i = mNodes.size(); i --> nodeIndex;)
    {
        if (mNodeParentIds[i] > lastNode && mNodeParentIds[i] != SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
        {
            // decrement the next node's parent ID if necessary
            mNodeParentIds[i] -=  numDeleted;
        }
    }

    return numDeleted;
}



/*-------------------------------------
 * Node Parenting
-------------------------------------*/
bool SL_SceneGraph::reparent_node(const size_t nodeIndex, const size_t newParentId) noexcept
{
    // data validation & early exits
    if (nodeIndex == SL_SceneNodeProp::SCENE_NODE_ROOT_ID || nodeIndex >= mNodeParentIds.size() || mNodeParentIds[nodeIndex] == newParentId)
    {
        return false;
    }

    if (newParentId >= mNodeParentIds.size() && newParentId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        return false;
    }

    // Cannot make a node a parent of its ancestor. It's possible, but then
    // what would the new ancestor of the node then be?
    if (newParentId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        if (node_is_child(newParentId, nodeIndex))
        {
            return false;
        }
    }

    const size_t numChildren    = num_total_children(nodeIndex);
    const size_t displacement   = 1 + numChildren;
    const size_t numNewSiblings = num_total_children(newParentId);
    const size_t newNodeIndex   = 1 + newParentId + numNewSiblings;

    // Keep track of the range of elements which need to be updated.
    const size_t effectStart = nodeIndex < newParentId ? nodeIndex : newNodeIndex;
    size_t       effectEnd   = nodeIndex < newParentId ? newNodeIndex : (nodeIndex+displacement);
    const size_t numAffected = effectEnd - effectStart;

    // Determine if we're moving "up", closer to the root, or "down" away from
    // the root
    const bool movingUp = newParentId < nodeIndex && (newParentId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID);
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
        size_t& rParentId = mNodeParentIds[i];
        size_t  pId  = rParentId;
        size_t  nId  = i;

        mCurrentTransforms[i].set_dirty();

        // Update the requested node's index
        if (nId == nodeIndex)
        {
            if (newParentId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
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
            if (pId == SL_SceneNodeProp::SCENE_NODE_ROOT_ID || pId < effectStart)
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
        while (effectEnd < mNodeParentIds.size())
        {
            size_t i = effectEnd++;

            if (mNodeParentIds[i] < nodeIndex)
            {
                mNodeParentIds[i] += displacement;
            }
            else if (mNodeParentIds[i] <= newParentId || mNodeParentIds[i] == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
            {
                break;
            }
        }
    }

    const size_t numRotations = movingUp ? displacement : (numAffected-displacement);
    rotate_right(mNodeParentIds.data()     + effectStart, numAffected, numRotations);
    rotate_right(mNodes.data()             + effectStart, numAffected, numRotations);
    rotate_right(mNodeNames.data()         + effectStart, numAffected, numRotations);
    rotate_right(mBaseTransforms.data()    + effectStart, numAffected, numRotations);
    rotate_right(mCurrentTransforms.data() + effectStart, numAffected, numRotations);
    rotate_right(mModelMatrices.data()     + effectStart, numAffected, numRotations);

    // Animations need love too
    for (SL_Animation& anim : mAnimations)
    {
        for (size_t& transformId : anim.mTransformIds)
        {
            if (movingUp)
            {
                if (transformId < nodeIndex)
                {
                    transformId += displacement;
                }
                else
                {
                    transformId -= amountToMove;
                }
            }
            else
            {
                if (transformId >= nodeIndex+numChildren)
                {
                    transformId -= displacement;
                }
                else
                {
                    transformId += amountToMove-displacement;
                }
            }
        }
    }

    LS_DEBUG_ASSERT(newNodeIndex <= mNodes.size());

    return true;
}



/*-------------------------------------
 * Node Duplication
-------------------------------------*/
bool SL_SceneGraph::copy_node(const size_t nodeIndex) noexcept
{
    if (nodeIndex == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        return false;
    }

    LS_DEBUG_ASSERT(nodeIndex < mNodes.size());

    const size_t numChildren    = num_total_children(nodeIndex);
    const size_t displacement   = 1 + numChildren;

    const SL_AlignedVector<std::size_t>::iterator&& parentIter = mNodeParentIds.begin() + nodeIndex;
    mNodeParentIds.insert(parentIter+displacement, parentIter, parentIter+displacement);

    const SL_AlignedVector<SL_SceneNode>::iterator&& nodeIter = mNodes.begin() + nodeIndex;
    mNodes.insert(nodeIter+displacement, nodeIter, nodeIter+displacement);

    const SL_AlignedVector<std::string>::iterator&& nameIter = mNodeNames.begin() + nodeIndex;
    mNodeNames.insert(nameIter+displacement, nameIter, nameIter+displacement);

    const SL_AlignedVector<ls::math::mat4>::iterator&& baseTransIter = mBaseTransforms.begin() + nodeIndex;
    mBaseTransforms.insert(baseTransIter+displacement, baseTransIter, baseTransIter+displacement);

    const SL_AlignedVector<SL_Transform>::iterator&& nodeTransIter = mCurrentTransforms.begin() + nodeIndex;
    mCurrentTransforms.insert(nodeTransIter+displacement, nodeTransIter, nodeTransIter+displacement);

    const SL_AlignedVector<ls::math::mat4>::iterator&& modelMatIter = mModelMatrices.begin() + nodeIndex;
    mModelMatrices.insert(modelMatIter+displacement, modelMatIter, modelMatIter+displacement);

    // easy part is done. now move onto the node data...
    // how many data elements of each node type are we moving
    size_t numCameraNodes = 0;
    size_t numMeshNodes = 0;
    size_t numBoneNodes = 0;

    // these are to determine where the first node-data element exists
    size_t camOffset = ~((size_t)0);
    size_t meshOffset = ~((size_t)0);
    size_t boneOffset = ~((size_t)0);

    // node IDs must match their index within the node array
    const size_t numTotalNodes = mNodes.size();

    for (size_t i = 0; i < displacement; ++i)
    {
        const size_t orig = nodeIndex + i;
        const size_t dupe = orig + displacement;

        // ensure all parent indices have been adjusted
        if (i > 0)
        {
            mNodeParentIds[dupe] += displacement;
        }

        LS_LOG_MSG("Copying node parent from ", mNodeParentIds[orig], " to ", mNodeParentIds[dupe]);

        // Enumerate how much node data is getting moved too
        switch (mNodes[orig].type)
        {
            case NODE_TYPE_BONE:
                ++numBoneNodes;
                boneOffset = boneOffset != (~(size_t)0) ? boneOffset : mNodes[orig].dataId;
                break;

            case NODE_TYPE_CAMERA:
                ++numCameraNodes;
                camOffset = camOffset != (~(size_t)0) ? camOffset : mNodes[orig].dataId;
                break;

            case NODE_TYPE_MESH:
                ++numMeshNodes;
                meshOffset = meshOffset != (~(size_t)0) ? meshOffset : mNodes[orig].dataId;
                break;

            case NODE_TYPE_EMPTY:
                // empty nodes have already been adjusted.
                break;

            // For the default case, let the compiler warn us if we missed an enum...
        }
    }

    if (numBoneNodes)
    {
        //LS_LOG_MSG("Copying ", numBoneNodes, " bones.");
        const size_t lastBone = boneOffset+numBoneNodes;
        mInvBoneTransforms.insert(mInvBoneTransforms.begin()+lastBone, mInvBoneTransforms.begin()+boneOffset, mInvBoneTransforms.begin()+lastBone);
        mBoneOffsets.insert(mBoneOffsets.begin()+lastBone, mBoneOffsets.begin()+boneOffset, mBoneOffsets.begin()+lastBone);
    }

    // no node-specific data held within the cameras
    if (numCameraNodes)
    {
        //LS_LOG_MSG("Copying ", numCameraNodes, " cameras.");
        const size_t lastCam = camOffset+numCameraNodes;
        mCameras.insert(mCameras.begin()+lastCam, mCameras.begin()+camOffset, mCameras.begin()+lastCam);
    }

    if (numMeshNodes)
    {
        //LS_LOG_MSG("Copying ", numMeshNodes, " meshes.");
        const size_t lastMesh = meshOffset+numMeshNodes;
        mNodeMeshes.reserve(mNodeMeshes.size()+numMeshNodes);
        mNumNodeMeshes.insert(mNumNodeMeshes.begin()+lastMesh, mNumNodeMeshes.begin()+meshOffset, mNumNodeMeshes.begin()+lastMesh);

        for (size_t i = 0; i < numMeshNodes; ++i)
        {
            size_t orig = meshOffset + i;
            size_t dupe = lastMesh + i;

            mNodeMeshes.emplace(mNodeMeshes.begin()+lastMesh+i, ls::utils::Pointer<size_t[]>{new size_t[mNumNodeMeshes[orig]]});

            for (size_t j = 0; j < mNumNodeMeshes[orig]; ++j)
            {
                mNodeMeshes[dupe][j] = mNodeMeshes[orig][j];
                //LS_LOG_MSG("Copied mesh index ", orig, '-', j, " to ", dupe, '-', j);
            }
        }
    }

    // Ensure all proper node data is now adjusted
    for (size_t i = nodeIndex + displacement; i < numTotalNodes; ++i)
    {
        switch (mNodes[i].type)
        {
            case NODE_TYPE_BONE:
                mNodes[i].dataId += numBoneNodes;
                break;

            case NODE_TYPE_CAMERA:
                mNodes[i].dataId += numCameraNodes;
                break;

            case NODE_TYPE_MESH:
                mNodes[i].dataId += numMeshNodes;
                break;

            case NODE_TYPE_EMPTY:
                // empty nodes only contain
                // have the compiler warn us if we missed an enum...
                break;
        }
    }

    // handle animation transform IDs
    /*
    for (SL_Animation& anim : mAnimations)
    {
        for (size_t& animId : anim.mTransformIds)
        {
            if (animId > nodeIndex+numChildren)
            {
                animId += displacement;
            }
        }
    }
    */

    return true;
}



/*-------------------------------------
 * Node Searching
-------------------------------------*/
size_t SL_SceneGraph::find_node_id(const std::string& nameQuery) const noexcept
{
    size_t nodeId = SL_SceneNodeProp::SCENE_NODE_ROOT_ID;

    for (size_t i = mNodeNames.size(); i-- > 0;)
    {
        if (mNodeNames[i] == nameQuery)
        {
            return i;
        }
    }

    return nodeId;
}



/*-------------------------------------
 * Node Child Counting (total)
-------------------------------------*/
size_t SL_SceneGraph::num_total_children(const size_t nodeIndex) const noexcept
{
    if (nodeIndex == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        return mNodes.size();
    }

    size_t numChildren = 0;

    for (size_t cId = nodeIndex + 1; cId < mCurrentTransforms.size(); ++cId)
    {
        const size_t pId = mNodeParentIds[cId];

        if (pId < nodeIndex || pId == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
        {
            break;
        }

        //LS_LOG_MSG("Node ", nodeIndex, " has a child at ", cId);

        numChildren++;
    }

    return numChildren;
}



/*-------------------------------------
 * Node Child Counting (immediate)
-------------------------------------*/
size_t SL_SceneGraph::num_immediate_children(const size_t nodeIndex) const noexcept
{
    if (nodeIndex == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        return mNodes.size();
    }

    const size_t startIndex = nodeIndex + 1;
    const size_t stopIndex = nodeIndex;
    size_t numChildren = 0;

    for (size_t cId = startIndex; cId < mCurrentTransforms.size(); ++cId)
    {
        const size_t pId = mNodeParentIds[cId];

        if (pId == SL_SceneNodeProp::SCENE_NODE_ROOT_ID || pId < stopIndex)
        {
            break;
        }

        //LS_LOG_MSG("Node ", nodeIndex, " has a child at ", cId);
        if (pId == nodeIndex)
        {
            numChildren++;
        }
    }

    return numChildren;
}



/*-------------------------------------
 * Check if a node is a child of another node
-------------------------------------*/
bool SL_SceneGraph::node_is_child(const size_t nodeIndex, const size_t parentId) const noexcept
{
    // parent IDs are always less than their child IDs.
    if (nodeIndex == SL_SceneNodeProp::SCENE_NODE_ROOT_ID || parentId >= nodeIndex)
    {
        return false;
    }

    // all nodes are children of the root node
    if (parentId == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        return true;
    }

    const size_t pId = mNodeParentIds[nodeIndex];

    // check for ancestry
    if (pId < parentId)
    {
        return false;
    }

    // check for immediate parenting
    if (pId == parentId)
    {
        return true;
    }

    for (size_t iter = pId; iter != SL_SceneNodeProp::SCENE_NODE_ROOT_ID;)
    {
        if (iter == parentId)
        {
            return true;
        }

        iter = mNodeParentIds[iter];
    }

    return false;
}



/*-------------------------------------
 *
-------------------------------------*/
size_t SL_SceneGraph::import(SL_SceneGraph& inGraph) noexcept
{
    const std::size_t baseVaoId  = mContext.vaos().size();
    const std::size_t baseMatId  = mMaterials.size();
    const std::size_t baseNodeId = mNodes.size();
    SL_Context&       inContext  = inGraph.mContext;

    for (size_t& parentId : inGraph.mNodeParentIds)
    {
        parentId += parentId == SCENE_NODE_ROOT_ID ? 0 : baseNodeId;
    }
    std::move(inGraph.mNodeParentIds.begin(), inGraph.mNodeParentIds.end(), std::back_inserter(mNodeParentIds));
    inGraph.mNodeParentIds.clear();

    for (SL_SceneNode& n : inGraph.mNodes)
    {
        if (n.type == NODE_TYPE_CAMERA)
        {
            n.dataId += mCameras.size();
        }
        else if (n.type == NODE_TYPE_MESH)
        {
            const size_t numNodeMeshes = inGraph.mNumNodeMeshes[n.dataId];
            const ls::utils::Pointer<size_t[]>& meshIds = inGraph.mNodeMeshes[n.dataId];

            for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
            {
                meshIds[meshId] += mMeshes.size();
            }

            n.dataId += mNumNodeMeshes.size();
        }
    }

    std::move(inGraph.mNodes.begin(), inGraph.mNodes.end(), std::back_inserter(mNodes));
    inGraph.mNodes.clear();

    std::move(inGraph.mNodeNames.begin(), inGraph.mNodeNames.end(), std::back_inserter(mNodeNames));
    inGraph.mNodeNames.clear();

    std::move(inGraph.mBaseTransforms.begin(), inGraph.mBaseTransforms.end(), std::back_inserter(mBaseTransforms));
    inGraph.mBaseTransforms.clear();

    std::move(inGraph.mCurrentTransforms.begin(), inGraph.mCurrentTransforms.end(), std::back_inserter(mCurrentTransforms));
    inGraph.mCurrentTransforms.clear();

    std::move(inGraph.mModelMatrices.begin(), inGraph.mModelMatrices.end(), std::back_inserter(mModelMatrices));
    inGraph.mModelMatrices.clear();

    for (SL_Mesh& inMesh : inGraph.mMeshes)
    {
        inMesh.vaoId += baseVaoId;
        inMesh.materialId += (uint32_t)baseMatId;
    }

    std::move(inGraph.mMeshes.begin(), inGraph.mMeshes.end(), std::back_inserter(mMeshes));
    inGraph.mMeshes.clear();

    std::move(inGraph.mMaterials.begin(), inGraph.mMaterials.end(), std::back_inserter(mMaterials));
    inGraph.mMaterials.clear();

    std::move(inGraph.mMeshBounds.begin(), inGraph.mMeshBounds.end(), std::back_inserter(mMeshBounds));
    inGraph.mMeshBounds.clear();

    std::move(inGraph.mNumNodeMeshes.begin(), inGraph.mNumNodeMeshes.end(), std::back_inserter(mNumNodeMeshes));
    inGraph.mNumNodeMeshes.clear();

    std::move(inGraph.mNodeMeshes.begin(), inGraph.mNodeMeshes.end(), std::back_inserter(mNodeMeshes));
    inGraph.mNodeMeshes.clear();

    std::move(inGraph.mInvBoneTransforms.begin(), inGraph.mInvBoneTransforms.end(), std::back_inserter(mInvBoneTransforms));
    inGraph.mInvBoneTransforms.clear();

    std::move(inGraph.mBoneOffsets.begin(), inGraph.mBoneOffsets.end(), std::back_inserter(mBoneOffsets));
    inGraph.mBoneOffsets.clear();

    std::move(inGraph.mCameras.begin(), inGraph.mCameras.end(), std::back_inserter(mCameras));
    inGraph.mCameras.clear();

    for (SL_Animation& a : inGraph.mAnimations)
    {
        for (size_t& channelId : a.mChannelIds)
        {
            channelId += mNodeAnims.size();
        }

        for (size_t& t : a.mTransformIds)
        {
            t += baseNodeId;
        }
    }

    std::move(inGraph.mAnimations.begin(), inGraph.mAnimations.end(), std::back_inserter(mAnimations));
    inGraph.mAnimations.clear();

    std::move(inGraph.mNodeAnims.begin(), inGraph.mNodeAnims.end(), std::back_inserter(mNodeAnims));
    inGraph.mNodeAnims.clear();

    mContext.import(std::move(inContext));

    return baseNodeId;
}



/*-------------------------------------
 * insert a mesh node
-------------------------------------*/
size_t SL_SceneGraph::insert_empty_node(
    size_t parentId,
    const char* name,
    const SL_Transform& transform) noexcept
{
    LS_ASSERT(parentId == SCENE_NODE_ROOT_ID || parentId < mNodes.size());
    LS_ASSERT(name != nullptr && name[0] != '\0');

    SL_SceneNode node;
    node.dataId = ~(size_t)0;
    node.type = SL_SceneNodeType::NODE_TYPE_EMPTY;

    mNodeParentIds.push_back(parentId);
    mNodes.push_back(node);
    mNodeNames.emplace_back(name);
    mBaseTransforms.push_back(transform.transform());
    mCurrentTransforms.push_back(transform);
    mModelMatrices.push_back(transform.transform());

    if (parentId != SCENE_NODE_ROOT_ID && mNodes.size() > 1)
    {
        reparent_node(mNodes.size()-1, parentId);
    }

    return mNodes.size()-1;
}



/*-------------------------------------
 * insert the mesh data for mesh nodes
-------------------------------------*/
size_t SL_SceneGraph::insert_mesh(const SL_Mesh& m, const SL_BoundingBox& meshBounds) noexcept
{
    LS_DEBUG_ASSERT(mMeshes.size() == mMeshBounds.size());
    //LS_DEBUG_ASSERT(m.materialId <= mMeshBounds.size() || m.materialId == ~(uint32_t)0);

    mMeshes.push_back(m);
    mMeshBounds.push_back(meshBounds);

    return mMeshes.size()-1;
}



/*-------------------------------------
 * Insert a Mesh node which references previous data
-------------------------------------*/
size_t SL_SceneGraph::insert_mesh_node(
    size_t parentId,
    const char* name,
    size_t numSubMeshes,
    const size_t* subMeshIds,
    const SL_Transform& transform) noexcept
{
    LS_ASSERT(numSubMeshes != 0);

    size_t nodeId = insert_empty_node(parentId, name, transform);
    SL_SceneNode& node = mNodes[nodeId];
    node.type = SL_SceneNodeType::NODE_TYPE_MESH;
    node.dataId = mNodeMeshes.size();

    mNodeMeshes.emplace_back(new(std::nothrow) size_t[numSubMeshes]);
    LS_ASSERT(mNodeMeshes.back().get() != nullptr);

    size_t* const pSubMeshIds = mNodeMeshes.back().get();
    for (size_t i = 0; i < numSubMeshes; ++i)
    {
        pSubMeshIds[i] = subMeshIds[i];
    }

    mNumNodeMeshes.push_back(numSubMeshes);

    return mNodes.size()-1;
}



/*-------------------------------------
 * Insert a bone node
-------------------------------------*/
size_t SL_SceneGraph::insert_bone_node(
    size_t parentId,
    const char* name,
    const ls::math::mat4& inverseTransform,
    const ls::math::mat4& boneOffset,
    const SL_Transform& transform) noexcept
{
    size_t nodeId = insert_empty_node(parentId, name, transform);
    SL_SceneNode& node = mNodes[nodeId];
    node.type = SL_SceneNodeType::NODE_TYPE_BONE;
    node.dataId = mInvBoneTransforms.size();

    mInvBoneTransforms.push_back(inverseTransform);
    mBoneOffsets.push_back(boneOffset);

    return mNodes.size()-1;
}



/*-------------------------------------
 * Insert a camera node
-------------------------------------*/
size_t SL_SceneGraph::insert_camera_node(
    size_t parentId,
    const char* name,
    const SL_Camera& cam,
    const SL_Transform& transform) noexcept
{
    size_t nodeId = insert_empty_node(parentId, name, transform);
    SL_SceneNode& node = mNodes[nodeId];
    node.type = SL_SceneNodeType::NODE_TYPE_CAMERA;
    node.dataId = mCameras.size();

    mCameras.push_back(cam);

    return mNodes.size()-1;
}
