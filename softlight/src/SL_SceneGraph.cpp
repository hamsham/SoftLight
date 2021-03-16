
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

template <class RandomIter>
void rotate_left(RandomIter nums, size_t size, size_t numRotations)
{
    rotate_right(nums, size, size-numRotations);
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
    mCameras(),
    mMeshes(),
    mMeshBounds(),
    mMaterials(),
    mNodes(),
    mBaseTransforms(),
    mCurrentTransforms(),
    mModelMatrices(),
    mInvBoneTransforms(),
    mBoneOffsets(),
    mNodeNames(),
    mAnimations(),
    mNodeAnims(),
    mNodeMeshes(),
    mNumNodeMeshes(),
    mContext()
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
    mCameras = s.mCameras;
    mMeshes = s.mMeshes;
    mMeshBounds = s.mMeshBounds;
    mMaterials = s.mMaterials;
    mNodes = s.mNodes;
    mBaseTransforms = s.mBaseTransforms;
    mCurrentTransforms = s.mCurrentTransforms;
    mModelMatrices = s.mModelMatrices;
    mInvBoneTransforms = s.mInvBoneTransforms;
    mBoneOffsets = s.mBoneOffsets;
    mNodeNames = s.mNodeNames;
    mAnimations = s.mAnimations;
    mNodeAnims = s.mNodeAnims;

    LS_DEBUG_ASSERT(s.mNumNodeMeshes.size() == s.mNodeMeshes.size());
    mNumNodeMeshes = s.mNumNodeMeshes;

    mNodeMeshes.reserve(s.mNodeMeshes.size());

    for (size_t i = 0; i < s.mNodeMeshes.size(); ++i)
    {
        const ls::utils::Pointer<size_t[]>& inMeshes = s.mNodeMeshes[i];
        const size_t inMeshCount = s.mNumNodeMeshes[i];

        LS_DEBUG_ASSERT(inMeshCount > 0);

        ls::utils::Pointer<size_t[]> outMeshes{new size_t[inMeshCount]};
        ls::utils::fast_copy(outMeshes.get(), inMeshes.get(), inMeshCount);

        mNodeMeshes.emplace_back(std::move(outMeshes));
    }

    mNumNodeMeshes = s.mNumNodeMeshes;
    mContext = s.mContext;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_SceneGraph& SL_SceneGraph::operator=(SL_SceneGraph&& s) noexcept
{
    mCameras = std::move(s.mCameras);
    mMeshes = std::move(s.mMeshes);
    mMeshBounds = std::move(s.mMeshBounds);
    mMaterials = std::move(s.mMaterials);
    mNodes = std::move(s.mNodes);
    mBaseTransforms = std::move(s.mBaseTransforms);
    mCurrentTransforms = std::move(s.mCurrentTransforms);
    mModelMatrices = std::move(s.mModelMatrices);
    mInvBoneTransforms = std::move(s.mInvBoneTransforms);
    mBoneOffsets = std::move(s.mBoneOffsets);
    mNodeNames = std::move(s.mNodeNames);
    mAnimations = std::move(s.mAnimations);
    mNodeAnims = std::move(s.mNodeAnims);
    mNodeMeshes = std::move(s.mNodeMeshes);
    mNumNodeMeshes = std::move(s.mNumNodeMeshes);

    mContext = std::move(s.mContext);

    return *this;
}



/*-------------------------------------
 * Terminate
-------------------------------------*/
void SL_SceneGraph::terminate() noexcept
{
    mCameras.clear();
    mMeshes.clear();
    mMeshBounds.clear();
    mMaterials.clear();
    mNodes.clear();
    mBaseTransforms.clear();
    mCurrentTransforms.clear();
    mModelMatrices.clear();
    mInvBoneTransforms.clear();
    mBoneOffsets.clear();
    mNodeNames.clear();
    mAnimations.clear();
    mNodeAnims.clear();
    mNodeMeshes.clear();
    mNumNodeMeshes.clear();

    mContext.terminate();
}



/*-------------------------------------
 * Node updating
-------------------------------------*/
void SL_SceneGraph::update_node_transform(const size_t transformId) noexcept
{
    SL_Transform& t = mCurrentTransforms[transformId];

    const size_t parentId = t.mParentId;
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

    // TODO:
    // Bone nodes should be implemented as a tree in a separate array to allow
    // animations to be re-used among nodes.
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
    SL_Transform* const pTransforms = mCurrentTransforms.data();

    for (size_t i = 0; i < numNodes; ++i)
    {
        // If a parent node is dirty, update all child nodes, iteratively
        if (pTransforms[i].is_dirty())
        {
            update_node_transform(i);

            for (size_t j = i+1; j < numNodes; ++j)
            {
                if (pTransforms[j].mParentId < i)
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
    mNodeMeshes.erase(mNodeMeshes.begin() + nodeDataId);
    mNumNodeMeshes.erase(mNumNodeMeshes.begin() + nodeDataId);
}

/*-------------------------------------
 * Bone Node Deletion
-------------------------------------*/
void SL_SceneGraph::delete_bone_node_data(const size_t nodeDataId) noexcept
{
    mInvBoneTransforms.erase(mInvBoneTransforms.begin() + nodeDataId);
    mBoneOffsets.erase(mBoneOffsets.begin() + nodeDataId);
}

/*-------------------------------------
 * Camera Deletion
-------------------------------------*/
void SL_SceneGraph::delete_camera_node_data(const size_t nodeDataId) noexcept
{
    mCameras.erase(mCameras.begin() + nodeDataId);
}

/*-------------------------------------
 * SL_Animation Deletion
-------------------------------------*/
void SL_SceneGraph::delete_node_animation_data(const size_t nodeId, const size_t animId) noexcept
{
    // Remove all animation channels associated with the current node.
    for (size_t i = mAnimations.size(); i-- > 0;)
    {

        // Animations contain transformation IDs which may need to be
        // decremented. Search for transformation indices with a value greater
        // than the current node's nodeId.
        SL_Animation& currentAnim = mAnimations[i];

        // friendly class member manipulation
        std::vector<size_t>& currentAnimIds  = currentAnim.mChannelIds;
        std::vector<size_t>& currentTransIds = currentAnim.mTransformIds;

        // Reduce the animation ID of all animations in *this.
        for (size_t j = currentAnimIds.size(); j-- > 0;)
        {
            if (currentTransIds[j] == nodeId)
            {
                currentAnim.erase(j);
                continue;
            }

            if (currentTransIds[j] > nodeId)
            {
                --currentTransIds[j];
            }

            if (animId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID && currentAnimIds[j] > animId)
            {
                --currentAnimIds[j];
            }
        }

        // Remove any defunct animations
        if (!currentAnim.size())
        {
            mAnimations.erase(mAnimations.begin() + i);
        }
    }

    // SL_Animation Channels
    if (animId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        mNodeAnims.erase(mNodeAnims.begin() + animId);
    }
}

/*-------------------------------------
 * Delete all nodes
-------------------------------------*/
void SL_SceneGraph::clear_node_data() noexcept
{
    mCameras.clear();
    mNodes.clear();
    mBaseTransforms.clear();
    mCurrentTransforms.clear();
    mModelMatrices.clear();
    mNodeNames.clear();
    mAnimations.clear();
    mNodeAnims.clear();
    mNodeMeshes.clear();
    mNumNodeMeshes.clear();
}

/*-------------------------------------
 * Node Deletion
-------------------------------------*/
size_t SL_SceneGraph::delete_node(const size_t nodeIndex) noexcept
{
    size_t numDeleted = 1;

    if (nodeIndex == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        numDeleted = mNodes.size();
        clear_node_data();
        return numDeleted;
    }

    // No mercy for client code
    LS_DEBUG_ASSERT(nodeIndex < mNodes.size());

    // Remove all child nodes and their data
    for (size_t i = mNodes.size(); i-- > nodeIndex;)
    {
        if (mCurrentTransforms[i].mParentId == nodeIndex)
        {
            numDeleted += delete_node(i);
        }
    }

    const SL_SceneNode&    n      = mNodes[nodeIndex];
    const SL_SceneNodeType typeId = n.type;
    const size_t           dataId = n.dataId;
    const size_t           animId = n.animListId;

    LS_DEBUG_ASSERT(nodeIndex == n.nodeId);

    // Delete any specific data associated with the node.
    switch (typeId)
    {
        case NODE_TYPE_CAMERA:
            delete_camera_node_data(dataId);
            break;

        case NODE_TYPE_MESH:
            delete_mesh_node_data(dataId);
            break;

        case NODE_TYPE_BONE:
            delete_bone_node_data(dataId);
            break;

        case NODE_TYPE_EMPTY:
            break;
    }

    // Delete the actual node
    mNodes.erase(mNodes.begin() + nodeIndex);
    mCurrentTransforms.erase(mCurrentTransforms.begin() + nodeIndex);
    mBaseTransforms.erase(mBaseTransforms.begin() + nodeIndex);
    mModelMatrices.erase(mModelMatrices.begin() + nodeIndex);
    mNodeNames.erase(mNodeNames.begin() + nodeIndex);

    // early exit in case there are no animations tied to the current node.
    delete_node_animation_data(nodeIndex, animId);

    // Decrement all node ID and data ID indices that are greater than those in
    // the current node. Also deal with the last bit of transformation data in
    // case a recursive deletion is in required.
    for (size_t i = mNodes.size(); i --> nodeIndex;)
    {
        SL_SceneNode&          nextNode      = mNodes[i];
        const SL_SceneNodeType nextType      = nextNode.type;
        size_t&                nextDataId    = nextNode.dataId;
        size_t&                nextAnimId    = nextNode.animListId;
        SL_Transform&          nextTransform = mCurrentTransforms[i];
        const size_t           nextParentId  = nextTransform.mParentId;

        // Placing assertion here because nodeIds must never equate to the
        // root node ID. They must always have tangible data to point at.
        LS_DEBUG_ASSERT(nextNode.nodeId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID);

        mNodes[i].nodeId = i;

        if (nextParentId > nodeIndex && nextParentId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
        {
            // decrement the next node's parent ID if necessary
            nextTransform.mParentId = nextParentId - 1;
        }

        // the node dataId member can be equal to the root node ID. This is
        // because empty nodes may not have have data to use.
        if (nextType == typeId
            && nextDataId > dataId
            && nextDataId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID
            )
        {
            --nextDataId;
        }

        // decrement the animation ID from all nodes with a value greater than
        // the current node's
        if (nextAnimId > animId && nextAnimId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
        {
            --nextAnimId;
        }
    }

    return numDeleted;
}

/*-------------------------------------
 * Node Parenting
-------------------------------------*/
bool SL_SceneGraph::reparent_node(const size_t nodeIndex, const size_t newParentId) noexcept
{
    if (newParentId == mCurrentTransforms[nodeIndex].mParentId)
    {
        // easy win
        return true;
    }

    if (nodeIndex == SL_SceneNodeProp::SCENE_NODE_ROOT_ID)
    {
        return false;
    }

    if (node_is_child(newParentId, nodeIndex))
    {
        LS_LOG_MSG("Cannot make a node ", nodeIndex, " a parent of its ancestor ", newParentId, '.');
        return false;
    }

    LS_DEBUG_ASSERT(nodeIndex < mNodes.size());

    const size_t numChildren    = num_total_children(nodeIndex);
    const size_t displacement   = 1 + numChildren;
    const size_t numNewSiblings = num_total_children(newParentId);
    const size_t newNodeIndex   = 1 + newParentId + numNewSiblings;

    // Keep track of the range of elements which need to be updated.
    const size_t effectStart = nodeIndex < newParentId ? nodeIndex : newNodeIndex;
    const size_t effectEnd   = nodeIndex < newParentId ? newNodeIndex : (nodeIndex+displacement);
    const size_t numAffected = effectEnd - effectStart;

    // Determine if we're moving "up", closer to the root, or "down" away from
    // the root
    const bool movingUp = newParentId < nodeIndex && (newParentId != SCENE_NODE_ROOT_ID);
    size_t amountToMove;

    if (movingUp)
    {
        amountToMove = nodeIndex - newNodeIndex;

        rotate_right(mNodes.data()             + effectStart, numAffected, displacement);
        rotate_right(mBaseTransforms.data()    + effectStart, numAffected, displacement);
        rotate_right(mCurrentTransforms.data() + effectStart, numAffected, displacement);
        rotate_right(mModelMatrices.data()     + effectStart, numAffected, displacement);
        rotate_right(mNodeNames.data()         + effectStart, numAffected, displacement);
    }
    else
    {
        amountToMove = newNodeIndex - nodeIndex;

        rotate_left(mNodes.data()             + effectStart, numAffected, displacement);
        rotate_left(mBaseTransforms.data()    + effectStart, numAffected, displacement);
        rotate_left(mCurrentTransforms.data() + effectStart, numAffected, displacement);
        rotate_left(mModelMatrices.data()     + effectStart, numAffected, displacement);
        rotate_left(mNodeNames.data()         + effectStart, numAffected, displacement);
    }

    for (size_t i = effectStart; i < effectEnd; ++i)
    {
        size_t& rParentId = mCurrentTransforms[i].mParentId;
        size_t  pId  = rParentId;
        size_t  nId  = mNodes[i].nodeId;

        mNodes[i].nodeId  = i;
        mCurrentTransforms[i].set_dirty();

        // Update the requested node's index
        if (nId == nodeIndex)
        {
            pId = newParentId;

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
                if (i <= amountToMove-numChildren)
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

    //LS_LOG_MSG("\tDone.");

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

    const SL_AlignedVector<SL_SceneNode>::iterator&& nodeIter = mNodes.begin() + nodeIndex;
    mNodes.insert(nodeIter+displacement, nodeIter, nodeIter+displacement);

    const SL_AlignedVector<ls::math::mat4>::iterator&& baseTransIter = mBaseTransforms.begin() + nodeIndex;
    mBaseTransforms.insert(baseTransIter+displacement, baseTransIter, baseTransIter+displacement);

    const SL_AlignedVector<SL_Transform>::iterator&& nodeTransIter = mCurrentTransforms.begin() + nodeIndex;
    mCurrentTransforms.insert(nodeTransIter+displacement, nodeTransIter, nodeTransIter+displacement);

    const SL_AlignedVector<ls::math::mat4>::iterator&& modelMatIter = mModelMatrices.begin() + nodeIndex;
    mModelMatrices.insert(modelMatIter+displacement, modelMatIter, modelMatIter+displacement);

    const SL_AlignedVector<std::string>::iterator&& nameIter = mNodeNames.begin() + nodeIndex;
    mNodeNames.insert(nameIter+displacement, nameIter, nameIter+displacement);

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
    for (size_t i = 0; i < numTotalNodes; ++i)
    {
        mNodes[i].nodeId = i;
    }

    for (size_t i = 0; i < displacement; ++i)
    {
        const size_t orig = nodeIndex + i;
        const size_t dupe = orig + displacement;

        // ensure all parent indices have been adjusted
        if (i > 0)
        {
            mCurrentTransforms[dupe].mParentId += displacement;
        }

        // we're copying nodes, not animations
        mNodes[dupe].animListId = SL_SceneNodeProp::SCENE_NODE_ROOT_ID;

        LS_LOG_MSG("Copying node parent from ", mCurrentTransforms[orig].mParentId, " to ", mCurrentTransforms[dupe].mParentId);

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
                // empty nodes only contain
                // have the compiler warn us if we missed an enum...
                break;
        }
    }

    if (numBoneNodes)
    {
        LS_LOG_MSG("Copying ", numBoneNodes, " bones.");
        const size_t lastBone = boneOffset+numBoneNodes;
        mInvBoneTransforms.insert(mInvBoneTransforms.begin()+lastBone, mInvBoneTransforms.begin()+boneOffset, mInvBoneTransforms.begin()+lastBone);
        mBoneOffsets.insert(mBoneOffsets.begin()+lastBone, mBoneOffsets.begin()+boneOffset, mBoneOffsets.begin()+lastBone);
    }

    // no node-specific data held within the cameras
    if (numCameraNodes)
    {
        LS_LOG_MSG("Copying ", numCameraNodes, " cameras.");
        const size_t lastCam = camOffset+numCameraNodes;
        mCameras.insert(mCameras.begin()+lastCam, mCameras.begin()+camOffset, mCameras.begin()+lastCam);
    }

    if (numMeshNodes)
    {
        LS_LOG_MSG("Copying ", numMeshNodes, " meshes.");
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
            if (animId > nodeIndex)
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
        const size_t pId = mCurrentTransforms[cId].mParentId;

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
        const size_t pId = mCurrentTransforms[cId].mParentId;

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

    const size_t pId = mCurrentTransforms[nodeIndex].mParentId;

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

        iter = mCurrentTransforms[iter].mParentId;
    }

    return false;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_SceneGraph::import(SL_SceneGraph& inGraph) noexcept
{
    const std::size_t baseVaoId  = mContext.vaos().size();
    const std::size_t baseMatId  = mMaterials.size();
    const std::size_t baseNodeId = mNodes.size();
    SL_Context&       inContext  = inGraph.mContext;

    SL_AlignedVector<SL_Mesh>& inMeshes = inGraph.mMeshes;

    for (SL_SceneNode& n : inGraph.mNodes)
    {
        n.nodeId += baseNodeId;

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

        n.animListId += mNodeAnims.size();
    }
    std::move(inGraph.mNodes.begin(), inGraph.mNodes.end(), std::back_inserter(mNodes));
    inGraph.mNodes.clear();

    for (SL_Mesh& inMesh : inMeshes)
    {
        inMesh.vaoId += baseVaoId;
        inMesh.materialId += (uint32_t)baseMatId;
    }

    std::move(inMeshes.begin(), inMeshes.end(), std::back_inserter(mMeshes));
    inMeshes.clear();

    std::move(inGraph.mCameras.begin(), inGraph.mCameras.end(), std::back_inserter(mCameras));
    inGraph.mCameras.clear();

    std::move(inGraph.mMeshBounds.begin(), inGraph.mMeshBounds.end(), std::back_inserter(mMeshBounds));
    inGraph.mMeshBounds.clear();

    std::move(inGraph.mMaterials.begin(), inGraph.mMaterials.end(), std::back_inserter(mMaterials));
    inGraph.mMaterials.clear();

    std::move(inGraph.mBaseTransforms.begin(), inGraph.mBaseTransforms.end(), std::back_inserter(mBaseTransforms));
    inGraph.mBaseTransforms.clear();

    for (SL_Transform& t : inGraph.mCurrentTransforms)
    {
        t.mParentId += t.mParentId == SCENE_NODE_ROOT_ID ? 0 : baseNodeId;
    }
    std::move(inGraph.mCurrentTransforms.begin(), inGraph.mCurrentTransforms.end(), std::back_inserter(mCurrentTransforms));
    inGraph.mCurrentTransforms.clear();

    std::move(inGraph.mModelMatrices.begin(), inGraph.mModelMatrices.end(), std::back_inserter(mModelMatrices));
    inGraph.mModelMatrices.clear();

    std::move(inGraph.mInvBoneTransforms.begin(), inGraph.mInvBoneTransforms.end(), std::back_inserter(mInvBoneTransforms));
    inGraph.mInvBoneTransforms.clear();

    std::move(inGraph.mBoneOffsets.begin(), inGraph.mBoneOffsets.end(), std::back_inserter(mBoneOffsets));
    inGraph.mBoneOffsets.clear();

    std::move(inGraph.mNodeNames.begin(), inGraph.mNodeNames.end(), std::back_inserter(mNodeNames));
    inGraph.mNodeNames.clear();

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

    std::move(inGraph.mNodeMeshes.begin(), inGraph.mNodeMeshes.end(), std::back_inserter(mNodeMeshes));
    inGraph.mNodeMeshes.clear();

    std::move(inGraph.mNumNodeMeshes.begin(), inGraph.mNumNodeMeshes.end(), std::back_inserter(mNumNodeMeshes));
    inGraph.mNumNodeMeshes.clear();

    mContext.import(std::move(inContext));

    return 0;
}
