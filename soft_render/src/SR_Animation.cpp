
#include <functional>
#include <utility>

#include "lightsky/math/Math.h"

#include "lightsky/utils/Assertions.h"

#include "soft_render/SR_Animation.hpp"
#include "soft_render/SR_SceneGraph.hpp"
#include "soft_render/SR_SceneNode.hpp"
#include "soft_render/SR_Transform.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SR_Animation object.
------------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_Animation::~SR_Animation() noexcept {
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_Animation::SR_Animation() noexcept :
    mPlayMode {SR_AnimPlayMode::SR_ANIM_PLAY_DEFAULT},
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
SR_Animation::SR_Animation(const SR_Animation& a) noexcept :
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
SR_Animation::SR_Animation(SR_Animation&& a) noexcept :
    mPlayMode {a.mPlayMode},
    mAnimId {a.mAnimId},
    mTotalTicks {a.mTotalTicks},
    mTicksPerSec {a.mTicksPerSec},
    mName {std::move(a.mName)},
    mChannelIds {std::move(a.mChannelIds)},
    mTrackIds {std::move(a.mTrackIds)},
    mTransformIds {std::move(a.mTransformIds)}
{
    a.mPlayMode = SR_AnimPlayMode::SR_ANIM_PLAY_DEFAULT;
    a.mAnimId = 0;
    a.mTotalTicks = 0.0;
    a.mTicksPerSec = 0.0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SR_Animation& SR_Animation::operator =(const SR_Animation& a) noexcept {
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
SR_Animation& SR_Animation::operator =(SR_Animation&& a) noexcept {
    mPlayMode = a.mPlayMode;
    a.mPlayMode = SR_AnimPlayMode::SR_ANIM_PLAY_DEFAULT;

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
 * Retrieve the SR_Animation mode
-------------------------------------*/
SR_AnimPlayMode SR_Animation::play_mode() const noexcept {
    return mPlayMode;
}



/*-------------------------------------
 * Set the SR_Animation mode
-------------------------------------*/
void SR_Animation::play_mode(const SR_AnimPlayMode animMode) noexcept {
    mPlayMode = animMode;
}



/*-------------------------------------
 * Retrieve the SR_Animation's hash number
-------------------------------------*/
size_t SR_Animation::id() const noexcept {
    return mAnimId;
}



/*-------------------------------------
 * Retrieve the SR_Animation's name
-------------------------------------*/
const std::string& SR_Animation::name() const noexcept {
    return mName;
}



/*-------------------------------------
 * Set the SR_Animation duration (ticks per second)
-------------------------------------*/
SR_AnimPrecision SR_Animation::duration() const noexcept {
    return mTotalTicks;
}



/*-------------------------------------
 * Retrieve the SR_Animation duration (tps).
-------------------------------------*/
void SR_Animation::duration(const SR_AnimPrecision ticks) noexcept {
    mTotalTicks = ticks;
}



/*-------------------------------------
 * Retrieve the time interpolation (tps).
-------------------------------------*/
SR_AnimPrecision SR_Animation::ticks_per_sec() const noexcept {
    return mTicksPerSec;
}



/*-------------------------------------
 * Retrieve the time interpolation (tps).
-------------------------------------*/
void SR_Animation::ticks_per_sec(const SR_AnimPrecision numTicks) noexcept {
    mTicksPerSec = numTicks;
}



/*-------------------------------------
 * Retrieve the transformations affected by *this.
-------------------------------------*/
const std::vector<size_t>& SR_Animation::get_transforms() const noexcept {
    return mTransformIds;
}



/*-------------------------------------
 * Retrieve the animation channels affected by *this.
-------------------------------------*/
const std::vector<size_t>& SR_Animation::tracks() const noexcept {
    return mTrackIds;
}



/*-------------------------------------
 * Retrieve an array of animations affected by *this.
-------------------------------------*/
const std::vector<size_t>& SR_Animation::animations() const noexcept {
    return mChannelIds;
}



/*-------------------------------------
 * Get the number of sub-animations
-------------------------------------*/
size_t SR_Animation::size() const noexcept {
    LS_DEBUG_ASSERT(mTransformIds.size() == mChannelIds.size());
    LS_DEBUG_ASSERT(mTransformIds.size() == mTrackIds.size());
    return mTransformIds.size();
}



/*-------------------------------------
 * Add a node's animation track to *this
-------------------------------------*/
void SR_Animation::add_channel(const SR_SceneNode& node, const size_t nodeTrackId) noexcept {
    mChannelIds.push_back(node.animListId);
    mTrackIds.push_back(nodeTrackId);
    mTransformIds.push_back(node.nodeId);
}



/*-------------------------------------
 * Remove a node's animation from *this.
-------------------------------------*/
void SR_Animation::erase(const size_t trackId) noexcept {
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
void SR_Animation::clear() noexcept {
    mChannelIds.clear();
    mTrackIds.clear();
    mTransformIds.clear();
}



/*-------------------------------------
 * Reserve space for animation tracks.
-------------------------------------*/
void SR_Animation::reserve(const size_t reserveSize) noexcept {
    mChannelIds.reserve(reserveSize);
    mTrackIds.reserve(reserveSize);
    mTransformIds.reserve(reserveSize);
}



/*-------------------------------------
 * Animate a scene graph using all tracks.
-------------------------------------*/
void SR_Animation::animate(SR_SceneGraph& graph, const SR_AnimPrecision percentDone) const noexcept {
    LS_DEBUG_ASSERT(percentDone >= 0.0);
    LS_DEBUG_ASSERT(mTransformIds.size() == mChannelIds.size());
    LS_DEBUG_ASSERT(mTransformIds.size() == mTrackIds.size());
    
    // prefetch
    const std::vector<std::vector<SR_AnimationChannel>>& nodeAnims = graph.mNodeAnims;
    SR_Transform* const pTransforms = graph.mCurrentTransforms.data();
    
    for (size_t i = mTransformIds.size(); i --> 0;) {
        const size_t animChannelId       = mChannelIds[i]; // SR_SceneGraph.mNodeAnims[node.animId]
        const size_t nodeTrackId         = mTrackIds[i]; // SR_SceneGraph.mNodeAnims[node.animId][nodeTrackId]
        const size_t transformId         = mTransformIds[i]; // SR_SceneGraph.currentTransforms[node.nodeId]
        const SR_AnimationChannel& track = nodeAnims[animChannelId][nodeTrackId];
        SR_Transform& nodeTransform      = pTransforms[transformId];
        
        LS_DEBUG_ASSERT(transformId != SR_SceneNodeProp::SCENE_NODE_ROOT_ID);

        if (track.has_position_frame(percentDone)) {
            const math::vec3&& pos = track.get_position_frame(percentDone);
            nodeTransform.set_position(pos);
        }

        if (track.has_scale_frame(percentDone)) {
            const math::vec3&& scl = track.get_scale_frame(percentDone);
            nodeTransform.set_scale(scl);
        }

        if (track.has_rotation_frame(percentDone)) {
            math::quat&& rot = track.get_rotation_frame(percentDone);
            nodeTransform.set_orientation(rot);
        }
    }
}



/*-------------------------------------
 * Animate a scene graph using all tracks.
-------------------------------------*/
void SR_Animation::init(SR_SceneGraph& graph, const bool atStart) const noexcept {
    LS_DEBUG_ASSERT(mTransformIds.size() == mChannelIds.size());
    LS_DEBUG_ASSERT(mTransformIds.size() == mTrackIds.size());
    
    // prefetch
    const std::vector<std::vector<SR_AnimationChannel>>& nodeAnims = graph.mNodeAnims;
    SR_Transform* pTransforms = graph.mCurrentTransforms.data();
    
    for (size_t i = mTransformIds.size(); i --> 0;) {
        const size_t animChannelId       = mChannelIds[i]; // SR_SceneGraph.mNodeAnims[node.animId]
        const size_t nodeTrackId         = mTrackIds[i]; // SR_SceneGraph.mNodeAnims[node.animId][nodeTrackId]
        const size_t transformId         = mTransformIds[i]; // SR_SceneGraph.currentTransforms[node.nodeId]
        const SR_AnimationChannel& track = nodeAnims[animChannelId][nodeTrackId];
        SR_Transform& nodeTransform      = pTransforms[transformId];

        if (track.mPosFrames.is_valid()) {
            nodeTransform.set_position(atStart ? track.mPosFrames.get_start_data() : track.mPosFrames.get_end_data());
        }

        if (track.mScaleFrames.is_valid()) {
            nodeTransform.set_scale(atStart ? track.mScaleFrames.get_start_data() : track.mScaleFrames.get_end_data());
        }

        if (track.mOrientFrames.is_valid()) {
            nodeTransform.set_orientation(atStart ? track.mOrientFrames.get_start_data() : track.mOrientFrames.get_end_data());
        }
    }
}
