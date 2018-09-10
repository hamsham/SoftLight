
#include <utility>

#include "lightsky/math/Math.h"

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Log.h"

#include "soft_render/SR_AnimationChannel.hpp"
#include "soft_render/SR_SceneGraph.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SR_Animation Key Structure
------------------------------------------------------------------------------*/
/*-------------------------------------
 * SR_Animation Key Destructor
-------------------------------------*/
SR_AnimationChannel::~SR_AnimationChannel() noexcept {
    clear();
}



/*-------------------------------------
 * SR_Animation Key Constructor
-------------------------------------*/
SR_AnimationChannel::SR_AnimationChannel() noexcept :
    mAnimMode{SR_AnimationFlag::SR_ANIM_FLAG_DEFAULT},
    mPosFrames{},
    mScaleFrames{},
    mOrientFrames{}
{}



/*-------------------------------------
 * SR_Animation Key Copy Constructor
-------------------------------------*/
SR_AnimationChannel::SR_AnimationChannel(const SR_AnimationChannel& ac) noexcept :
    mAnimMode{ac.mAnimMode},
    mPosFrames{ac.mPosFrames},
    mScaleFrames{ac.mScaleFrames},
    mOrientFrames{ac.mOrientFrames}
{}



/*-------------------------------------
 * SR_Animation Key Move Constructor
-------------------------------------*/
SR_AnimationChannel::SR_AnimationChannel(SR_AnimationChannel&& ac) noexcept :
    mAnimMode{ac.mAnimMode},
    mPosFrames{std::move(ac.mPosFrames)},
    mScaleFrames{std::move(ac.mScaleFrames)},
    mOrientFrames{std::move(ac.mOrientFrames)}
{
    ac.mAnimMode = SR_AnimationFlag::SR_ANIM_FLAG_DEFAULT;
}



/*-------------------------------------
 * SR_Animation Key Copy Operator
-------------------------------------*/
SR_AnimationChannel& SR_AnimationChannel::operator =(const SR_AnimationChannel& ac) noexcept {
    if (this == &ac) {
        return *this;
    }
    
    mAnimMode = ac.mAnimMode;
    mPosFrames = ac.mPosFrames;
    mScaleFrames = ac.mScaleFrames;
    mOrientFrames = ac.mOrientFrames;
    
    return *this;
}



/*-------------------------------------
 * SR_Animation Key Destructor
-------------------------------------*/
SR_AnimationChannel& SR_AnimationChannel::operator =(SR_AnimationChannel&& ac) noexcept {
    if (this == &ac) {
        return *this;
    }
    
    mAnimMode = ac.mAnimMode;
    ac.mAnimMode = SR_AnimationFlag::SR_ANIM_FLAG_DEFAULT;
    
    mPosFrames = std::move(ac.mPosFrames);
    mScaleFrames = std::move(ac.mScaleFrames);
    mOrientFrames = std::move(ac.mOrientFrames);

    return *this;
}



/*-------------------------------------
 * Set the number of keys
-------------------------------------*/
SR_AnimationFlag SR_AnimationChannel::get_anim_flags() const noexcept
{
    return mAnimMode;
}



/*-------------------------------------
 * Set the number of keys
-------------------------------------*/
bool SR_AnimationChannel::set_num_frames(
    const unsigned posCount,
    const unsigned sclCount,
    const unsigned rotCount
) noexcept {
    if (!mPosFrames.init(posCount)
    || !mScaleFrames.init(sclCount)
    || !mOrientFrames.init(rotCount))
    {
        clear();
        return false;
    }
    
    return true;
}



/*-------------------------------------
 * Clear all SR_Animation keys
-------------------------------------*/
void SR_AnimationChannel::clear() noexcept {
    mAnimMode = SR_AnimationFlag::SR_ANIM_FLAG_DEFAULT;
    mPosFrames.clear();
    mScaleFrames.clear();
    mOrientFrames.clear();
}



/*-------------------------------------
 * Retrieve the starting time of the current animation track.
-------------------------------------*/
SR_AnimPrecision SR_AnimationChannel::get_start_time() const noexcept {
    return math::min(
        mPosFrames.get_start_time(),
        mScaleFrames.get_start_time(),
        mOrientFrames.get_start_time()
    );
}



/*-------------------------------------
 * Assign a start time for the current animation track.
-------------------------------------*/
void SR_AnimationChannel::set_start_time(const SR_AnimPrecision startOffset) noexcept {
    const SR_AnimPrecision posOffset = mPosFrames.get_start_time() - get_start_time();
    mPosFrames.set_start_time(startOffset+posOffset);
    
    const SR_AnimPrecision sclOffset = mScaleFrames.get_start_time() - get_start_time();
    mScaleFrames.set_start_time(startOffset+sclOffset);
    
    const SR_AnimPrecision rotOffset = mOrientFrames.get_start_time() - get_start_time();
    mOrientFrames.set_start_time(startOffset+rotOffset);
}



/*-------------------------------------
 * Retrieve the ending time of the current animation track.
-------------------------------------*/
SR_AnimPrecision SR_AnimationChannel::get_end_time() const noexcept {
    return math::max(
        mPosFrames.get_end_time(),
        mScaleFrames.get_end_time(),
        mOrientFrames.get_end_time()
    );
}
