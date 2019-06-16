
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
SR_AnimationFlag SR_AnimationChannel::flags() const noexcept
{
    return mAnimMode;
}



/*-------------------------------------
 * Set the number of keys
-------------------------------------*/
bool SR_AnimationChannel::size(
    const size_t posCount,
    const size_t sclCount,
    const size_t rotCount
) noexcept
{
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
void SR_AnimationChannel::clear() noexcept
{
    mAnimMode = SR_AnimationFlag::SR_ANIM_FLAG_DEFAULT;
    mPosFrames.clear();
    mScaleFrames.clear();
    mOrientFrames.clear();
}



/*-------------------------------------
 * Retrieve the starting time of the current animation track.
-------------------------------------*/
SR_AnimPrecision SR_AnimationChannel::start_time() const noexcept
{
    return math::min(
        mPosFrames.start_time(),
        mScaleFrames.start_time(),
        mOrientFrames.start_time()
    );
}



/*-------------------------------------
 * Assign a start time for the current animation track.
-------------------------------------*/
void SR_AnimationChannel::start_time(const SR_AnimPrecision startOffset) noexcept
{
    const SR_AnimPrecision posOffset = mPosFrames.start_time() - start_time();
    mPosFrames.start_time(startOffset + posOffset);
    
    const SR_AnimPrecision sclOffset = mScaleFrames.start_time() - start_time();
    mScaleFrames.start_time(startOffset + sclOffset);
    
    const SR_AnimPrecision rotOffset = mOrientFrames.start_time() - start_time();
    mOrientFrames.start_time(startOffset + rotOffset);
}



/*-------------------------------------
 * Retrieve the ending time of the current animation track.
-------------------------------------*/
SR_AnimPrecision SR_AnimationChannel::end_time() const noexcept
{
    return math::max(
        mPosFrames.end_time(),
        mScaleFrames.end_time(),
        mOrientFrames.end_time()
    );
}
