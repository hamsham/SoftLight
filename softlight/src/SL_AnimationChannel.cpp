
#include <utility>

#include "lightsky/utils/Copy.h"

#include "softlight/SL_AnimationChannel.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * SL_Animation Key Structure
------------------------------------------------------------------------------*/
/*-------------------------------------
 * SL_Animation Key Destructor
-------------------------------------*/
SL_AnimationChannel::~SL_AnimationChannel() noexcept {
    clear();
}



/*-------------------------------------
 * SL_Animation Key Constructor
-------------------------------------*/
SL_AnimationChannel::SL_AnimationChannel() noexcept :
    mAnimMode{SL_AnimationFlag::SL_ANIM_FLAG_DEFAULT},
    mPosFrames{},
    mScaleFrames{},
    mOrientFrames{}
{}



/*-------------------------------------
 * SL_Animation Key Copy Constructor
-------------------------------------*/
SL_AnimationChannel::SL_AnimationChannel(const SL_AnimationChannel& ac) noexcept :
    mAnimMode{ac.mAnimMode},
    mPosFrames{ac.mPosFrames},
    mScaleFrames{ac.mScaleFrames},
    mOrientFrames{ac.mOrientFrames}
{}



/*-------------------------------------
 * SL_Animation Key Move Constructor
-------------------------------------*/
SL_AnimationChannel::SL_AnimationChannel(SL_AnimationChannel&& ac) noexcept :
    mAnimMode{ac.mAnimMode},
    mPosFrames{std::move(ac.mPosFrames)},
    mScaleFrames{std::move(ac.mScaleFrames)},
    mOrientFrames{std::move(ac.mOrientFrames)}
{
    ac.mAnimMode = SL_AnimationFlag::SL_ANIM_FLAG_DEFAULT;
}



/*-------------------------------------
 * SL_Animation Key Copy Operator
-------------------------------------*/
SL_AnimationChannel& SL_AnimationChannel::operator =(const SL_AnimationChannel& ac) noexcept {
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
 * SL_Animation Key Destructor
-------------------------------------*/
SL_AnimationChannel& SL_AnimationChannel::operator =(SL_AnimationChannel&& ac) noexcept {
    if (this == &ac) {
        return *this;
    }
    
    mAnimMode = ac.mAnimMode;
    ac.mAnimMode = SL_AnimationFlag::SL_ANIM_FLAG_DEFAULT;
    
    mPosFrames = std::move(ac.mPosFrames);
    mScaleFrames = std::move(ac.mScaleFrames);
    mOrientFrames = std::move(ac.mOrientFrames);

    return *this;
}



/*-------------------------------------
 * Set the number of keys
-------------------------------------*/
SL_AnimationFlag SL_AnimationChannel::flags() const noexcept
{
    return mAnimMode;
}



/*-------------------------------------
 * Set the number of keys
-------------------------------------*/
bool SL_AnimationChannel::size(
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
 * Clear all SL_Animation keys
-------------------------------------*/
void SL_AnimationChannel::clear() noexcept
{
    mAnimMode = SL_AnimationFlag::SL_ANIM_FLAG_DEFAULT;
    mPosFrames.clear();
    mScaleFrames.clear();
    mOrientFrames.clear();
}



/*-------------------------------------
 * Retrieve the starting time of the current animation track.
-------------------------------------*/
SL_AnimPrecision SL_AnimationChannel::start_time() const noexcept
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
void SL_AnimationChannel::start_time(const SL_AnimPrecision startOffset) noexcept
{
    const SL_AnimPrecision posOffset = mPosFrames.start_time() - start_time();
    mPosFrames.start_time(startOffset + posOffset);
    
    const SL_AnimPrecision sclOffset = mScaleFrames.start_time() - start_time();
    mScaleFrames.start_time(startOffset + sclOffset);
    
    const SL_AnimPrecision rotOffset = mOrientFrames.start_time() - start_time();
    mOrientFrames.start_time(startOffset + rotOffset);
}



/*-------------------------------------
 * Retrieve the ending time of the current animation track.
-------------------------------------*/
SL_AnimPrecision SL_AnimationChannel::end_time() const noexcept
{
    return math::max(
        mPosFrames.end_time(),
        mScaleFrames.end_time(),
        mOrientFrames.end_time()
    );
}
