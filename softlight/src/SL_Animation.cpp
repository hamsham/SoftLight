
#include <utility> // std::move()

#include "lightsky/math/Math.h"

#include "lightsky/utils/Assertions.h"

#include "softlight/SL_Animation.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/SL_SceneNode.hpp"
#include "softlight/SL_Transform.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SL_Animation object.
------------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_Animation::~SL_Animation() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_Animation::SL_Animation() noexcept :
    mPlayMode {SL_AnimPlayMode::SL_ANIM_PLAY_DEFAULT},
    mAnimId {0},
    mTotalTicks {0},
    mTicksPerSec {0.0},
    mName {""},
    mChannelIds {},
    mTrackIds {},
    mTransformIds {}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_Animation::SL_Animation(const SL_Animation& a) noexcept :
    mPlayMode {a.mPlayMode},
    mAnimId {a.mAnimId},
    mTotalTicks {a.mTotalTicks},
    mTicksPerSec {a.mTicksPerSec},
    mName {a.mName},
    mChannelIds {a.mChannelIds},
    mTrackIds {a.mTrackIds},
    mTransformIds {a.mTransformIds}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_Animation::SL_Animation(SL_Animation&& a) noexcept :
    mPlayMode {a.mPlayMode},
    mAnimId {a.mAnimId},
    mTotalTicks {a.mTotalTicks},
    mTicksPerSec {a.mTicksPerSec},
    mName {std::move(a.mName)},
    mChannelIds {std::move(a.mChannelIds)},
    mTrackIds {std::move(a.mTrackIds)},
    mTransformIds {std::move(a.mTransformIds)}
{
    a.mPlayMode = SL_AnimPlayMode::SL_ANIM_PLAY_DEFAULT;
    a.mAnimId = 0;
    a.mTotalTicks = 0.0;
    a.mTicksPerSec = 0.0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_Animation& SL_Animation::operator =(const SL_Animation& a) noexcept
    {
    mPlayMode = a.mPlayMode;
    mAnimId = a.mAnimId;
    mTotalTicks = a.mTotalTicks;
    mTicksPerSec = a.mTicksPerSec;
    mName = a.mName;
    mChannelIds = a.mChannelIds;
    mTrackIds = a.mTrackIds;
    mTransformIds = a.mTransformIds;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_Animation& SL_Animation::operator =(SL_Animation&& a) noexcept
    {
    mPlayMode = a.mPlayMode;
    a.mPlayMode = SL_AnimPlayMode::SL_ANIM_PLAY_DEFAULT;

    mAnimId = a.mAnimId;
    a.mAnimId = 0;

    mTotalTicks = a.mTotalTicks;
    a.mTotalTicks = 0.0;

    mTicksPerSec = a.mTicksPerSec;
    a.mTicksPerSec = 0.0;

    mName = std::move(a.mName);
    mChannelIds = std::move(a.mChannelIds);
    mTrackIds = std::move(a.mTrackIds);
    mTransformIds = std::move(a.mTransformIds);

    return *this;
}



/*-------------------------------------
 * Retrieve the SL_Animation mode
-------------------------------------*/
SL_AnimPlayMode SL_Animation::play_mode() const noexcept
{
    return mPlayMode;
}



/*-------------------------------------
 * Set the SL_Animation mode
-------------------------------------*/
void SL_Animation::play_mode(const SL_AnimPlayMode animMode) noexcept
{
    mPlayMode = animMode;
}



/*-------------------------------------
 * Retrieve the SL_Animation's hash number
-------------------------------------*/
size_t SL_Animation::id() const noexcept
{
    return mAnimId;
}



/*-------------------------------------
 * Retrieve the SL_Animation's name
-------------------------------------*/
const std::string& SL_Animation::name() const noexcept
{
    return mName;
}



/*-------------------------------------
 * Set the SL_Animation duration (ticks per second)
-------------------------------------*/
SL_AnimPrecision SL_Animation::duration() const noexcept
{
    return mTotalTicks;
}



/*-------------------------------------
 * Retrieve the SL_Animation duration (tps).
-------------------------------------*/
void SL_Animation::duration(const SL_AnimPrecision ticks) noexcept
{
    mTotalTicks = ticks;
}



/*-------------------------------------
 * Retrieve the time interpolation (tps).
-------------------------------------*/
SL_AnimPrecision SL_Animation::ticks_per_sec() const noexcept
{
    return mTicksPerSec;
}



/*-------------------------------------
 * Retrieve the time interpolation (tps).
-------------------------------------*/
void SL_Animation::ticks_per_sec(const SL_AnimPrecision numTicks) noexcept
{
    mTicksPerSec = numTicks;
}



/*-------------------------------------
 * Retrieve the transformations affected by *this (const).
-------------------------------------*/
const std::vector<size_t>& SL_Animation::transforms() const noexcept
{
    return mTransformIds;
}



/*-------------------------------------
 * Retrieve the transformations affected by *this.
-------------------------------------*/
std::vector<size_t>& SL_Animation::transforms() noexcept
{
    return mTransformIds;
}



/*-------------------------------------
 * Retrieve the animation channels affected by *this.
-------------------------------------*/
const std::vector<size_t>& SL_Animation::tracks() const noexcept
{
    return mTrackIds;
}



/*-------------------------------------
 * Retrieve an array of animations affected by *this.
-------------------------------------*/
const std::vector<size_t>& SL_Animation::animations() const noexcept
{
    return mChannelIds;
}



/*-------------------------------------
 * Get the number of sub-animations
-------------------------------------*/
size_t SL_Animation::size() const noexcept
{
    LS_DEBUG_ASSERT(mTransformIds.size() == mChannelIds.size());
    LS_DEBUG_ASSERT(mTransformIds.size() == mTrackIds.size());
    return mTransformIds.size();
}



/*-------------------------------------
 * Add a node's animation track to *this
-------------------------------------*/
void SL_Animation::add_channel(const size_t sceneChannelId, const size_t nodeTrackId, const size_t nodeId) noexcept
{
    mChannelIds.push_back(sceneChannelId);
    mTrackIds.push_back(nodeTrackId);
    mTransformIds.push_back(nodeId);
}



/*-------------------------------------
 * Remove a node's animation from *this.
-------------------------------------*/
void SL_Animation::erase(const size_t trackId) noexcept
{
    LS_DEBUG_ASSERT(mTransformIds.size() == mChannelIds.size());
    LS_DEBUG_ASSERT(mTransformIds.size() == mTrackIds.size());
    
    LS_DEBUG_ASSERT(trackId < mTransformIds.size());
    
    mChannelIds.erase(mChannelIds.begin() + trackId);
    mTrackIds.erase(mTrackIds.begin() + trackId);
    mTransformIds.erase(mTransformIds.begin() + trackId);
}



/*-------------------------------------
 * Clear all sub-animations.
-------------------------------------*/
void SL_Animation::clear() noexcept
{
    mChannelIds.clear();
    mTrackIds.clear();
    mTransformIds.clear();
}



/*-------------------------------------
 * Reserve space for animation tracks.
-------------------------------------*/
void SL_Animation::reserve(const size_t reserveSize) noexcept
{
    mChannelIds.reserve(reserveSize);
    mTrackIds.reserve(reserveSize);
    mTransformIds.reserve(reserveSize);
}



/*-------------------------------------
 * Animate a scene graph using all tracks.
-------------------------------------*/
void SL_Animation::animate(SL_SceneGraph& graph, const SL_AnimPrecision percentDone) const noexcept
{
    LS_DEBUG_ASSERT(percentDone >= 0.0);
    LS_DEBUG_ASSERT(mTransformIds.size() == mChannelIds.size());
    LS_DEBUG_ASSERT(mTransformIds.size() == mTrackIds.size());

    // prefetch
    const SL_AlignedVector<SL_AnimationChannel>* pNodeAnims = graph.mNodeAnims.data();
    SL_Transform* const pTransforms = graph.mCurrentTransforms.data();

    for (size_t i = mTransformIds.size(); i --> 0;)
    {
        const size_t animChannelId       = mChannelIds[i];   // maps to SL_SceneGraph.mNodeAnims[node.animId]
        const size_t nodeTrackId         = mTrackIds[i];     // maps to SL_SceneGraph.mNodeAnims[node.animId][nodeTrackId]
        const size_t transformId         = mTransformIds[i]; // maps to SL_SceneGraph.mCurrentTransforms[node.nodeId]
        const SL_AnimationChannel& track = pNodeAnims[animChannelId][nodeTrackId];
        SL_Transform& nodeTransform      = pTransforms[transformId];

        LS_DEBUG_ASSERT(transformId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID);

        if (track.has_position_frame(percentDone))
        {
            const math::vec3&& pos = track.position_frame(percentDone);
            nodeTransform.position(pos);
        }

        if (track.has_scale_frame(percentDone))
        {
            const math::vec3&& scl = track.scale_frame(percentDone);
            nodeTransform.scaling(scl);
        }

        if (track.has_rotation_frame(percentDone))
        {
            math::quat&& rot = track.rotation_frame(percentDone);
            nodeTransform.orientation(rot);
        }
    }
}



/*-------------------------------------
 * Animate a scene graph using all tracks.
-------------------------------------*/
void SL_Animation::animate(SL_SceneGraph& graph, const SL_AnimPrecision percentDone, size_t baseTransformId) const noexcept
{
    LS_DEBUG_ASSERT(percentDone >= 0.0);
    LS_DEBUG_ASSERT(mTransformIds.size() == mChannelIds.size());
    LS_DEBUG_ASSERT(mTransformIds.size() == mTrackIds.size());

    // prefetch
    const SL_AlignedVector<SL_AnimationChannel>* pNodeAnims = graph.mNodeAnims.data();
    SL_Transform* const pTransforms = graph.mCurrentTransforms.data();

    size_t rootIndex = !mTransformIds.empty() ? mTransformIds[0] : 0;

    for (size_t i = mTransformIds.size(); i --> 0;)
    {
        const size_t animChannelId       = mChannelIds[i];   // maps to SL_SceneGraph.mNodeAnims[node.animId]
        const size_t nodeTrackId         = mTrackIds[i];     // maps to SL_SceneGraph.mNodeAnims[node.animId][nodeTrackId]
        const size_t transformId         = baseTransformId + (mTransformIds[i] - rootIndex); // maps to SL_SceneGraph.mCurrentTransforms[node.nodeId]
        const SL_AnimationChannel& track = pNodeAnims[animChannelId][nodeTrackId];
        SL_Transform& nodeTransform      = pTransforms[transformId];

        LS_DEBUG_ASSERT(transformId != SL_SceneNodeProp::SCENE_NODE_ROOT_ID);

        if (track.has_position_frame(percentDone))
        {
            const math::vec3&& pos = track.position_frame(percentDone);
            nodeTransform.position(pos);
        }

        if (track.has_scale_frame(percentDone))
        {
            const math::vec3&& scl = track.scale_frame(percentDone);
            nodeTransform.scaling(scl);
        }

        if (track.has_rotation_frame(percentDone))
        {
            math::quat&& rot = track.rotation_frame(percentDone);
            nodeTransform.orientation(rot);
        }
    }
}



/*-------------------------------------
 * Animate a scene graph using all tracks.
-------------------------------------*/
void SL_Animation::init(SL_SceneGraph& graph, const bool atStart) const noexcept
{
    LS_DEBUG_ASSERT(mTransformIds.size() == mChannelIds.size());
    LS_DEBUG_ASSERT(mTransformIds.size() == mTrackIds.size());
    
    // prefetch
    const SL_AlignedVector<SL_AnimationChannel>* pNodeAnims = graph.mNodeAnims.data();
    SL_Transform* pTransforms = graph.mCurrentTransforms.data();
    
    for (size_t i = mTransformIds.size(); i --> 0;)
    {
        const size_t animChannelId       = mChannelIds[i]; // SL_SceneGraph.mNodeAnims[node.animId]
        const size_t nodeTrackId         = mTrackIds[i]; // SL_SceneGraph.mNodeAnims[node.animId][nodeTrackId]
        const size_t transformId         = mTransformIds[i]; // SL_SceneGraph.currentTransforms[node.nodeId]
        const SL_AnimationChannel& track = pNodeAnims[animChannelId][nodeTrackId];
        SL_Transform& nodeTransform      = pTransforms[transformId];

        if (track.mPosFrames.valid())
        {
            nodeTransform.position(atStart ? track.mPosFrames.start_data() : track.mPosFrames.end_data());
        }

        if (track.mScaleFrames.valid())
        {
            nodeTransform.scaling(atStart ? track.mScaleFrames.start_data() : track.mScaleFrames.end_data());
        }

        if (track.mOrientFrames.valid())
        {
            nodeTransform.orientation(atStart ? track.mOrientFrames.start_data() : track.mOrientFrames.end_data());
        }
    }
}



/*-------------------------------------
 * Determine if we can use a baseTransformId
-------------------------------------*/
bool SL_Animation::have_monotonic_transforms() const noexcept
{
    size_t lastTransformId = mTransformIds.empty() ? 0 : mTransformIds[0];

    for (size_t i = 0; i < mTransformIds.size(); ++i, ++lastTransformId)
    {
        if (mTransformIds[i] != lastTransformId)
        {
            return false;
        }
    }

    return true;
}
