
#include "lightsky/math/vec_utils.h"
#include "lightsky/math/quat_utils.h"

#include "lightsky/utils/Assertions.h"

#include "softlight/SL_AnimationKeyList.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace
{



struct _SL_AnimInterpolator
{
    template <typename data_t>
    inline LS_INLINE math::vec3_t<data_t> operator()(const math::vec3_t<data_t>& a, const math::vec3_t<data_t>& b, const data_t& t) const noexcept
    {
        return math::mix(a, b, t);
    }

    template <typename data_t>
    inline LS_INLINE math::vec4_t<data_t> operator()(const math::vec4_t<data_t>& a, const math::vec4_t<data_t>& b, const data_t& t) const noexcept
    {
        return math::mix(a, b, t);
    }

    // Regular linear interpolations don't work correctly for rotations over
    // 180 degrees.
    template <typename data_t>
    inline LS_INLINE math::quat_t<data_t> operator()(const math::quat_t<data_t>& a, const math::quat_t<data_t>& b, const data_t& t) const noexcept
    {
        return math::slerp(a, b, t);
    }
};



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_Animation Key Frame Class.
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
template<typename data_t>
SL_AnimationKeyList<data_t>::~SL_AnimationKeyList() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
template<typename data_t>
SL_AnimationKeyList<data_t>::SL_AnimationKeyList() noexcept :
    mNumFrames{0},
    mKeyTimes{nullptr},
    mKeyData{nullptr}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
template<typename data_t>
SL_AnimationKeyList<data_t>::SL_AnimationKeyList(const SL_AnimationKeyList& a) noexcept :
    mNumFrames{0},
    mKeyTimes{nullptr},
    mKeyData{nullptr}
{
    *this = a;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
template<typename data_t>
SL_AnimationKeyList<data_t>::SL_AnimationKeyList(SL_AnimationKeyList&& a) noexcept :
    mNumFrames{a.mNumFrames},
    mKeyTimes{std::move(a.mKeyTimes)},
    mKeyData{std::move(a.mKeyData)}
{
    a.mNumFrames = 0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
template<typename data_t>
SL_AnimationKeyList<data_t>& SL_AnimationKeyList<data_t>::operator=(const SL_AnimationKeyList& k) noexcept
{
    if (this == &k)
    {
        return *this;
    }

    if (!k.mNumFrames)
    {
        if (mNumFrames)
        {
            clear();
        }
        return *this;
    }

    if (k.mNumFrames != mNumFrames)
    {
        mKeyTimes.reset((SL_AnimPrecision*)ls::utils::aligned_malloc(sizeof(SL_AnimPrecision) * k.mNumFrames));
        mKeyData.reset((data_t*)ls::utils::aligned_malloc(sizeof(data_t) * k.mNumFrames));
    }

    if (!mKeyTimes || !mKeyData)
    {
        clear();
        return *this;
    }

    mNumFrames = k.mNumFrames;

    for (size_t i = 0; i < mNumFrames; ++i)
    {
        mKeyTimes[i] = k.mKeyTimes[i];
        mKeyData[i] = k.mKeyData[i];
    }

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
template<typename data_t>
SL_AnimationKeyList<data_t>& SL_AnimationKeyList<data_t>::operator=(SL_AnimationKeyList&& k) noexcept
{
    if (this == &k)
    {
        return *this;
    }

    mNumFrames = k.mNumFrames;
    k.mNumFrames = 0;

    mKeyTimes = std::move(k.mKeyTimes);
    mKeyData = std::move(k.mKeyData);

    return *this;
}



/*-------------------------------------
 * Reset all data in *this
-------------------------------------*/
template<typename data_t>
void SL_AnimationKeyList<data_t>::clear() noexcept
{
    mNumFrames = 0;
    mKeyTimes.reset();
    mKeyData.reset();
}



/*-------------------------------------
 * Get the number of keyframes in *this
-------------------------------------*/
template<typename data_t>
size_t SL_AnimationKeyList<data_t>::size() const noexcept
{
    return mNumFrames;
}



/*-------------------------------------
 * Initialize (allocate) data for keyframes
-------------------------------------*/
template<typename data_t>
bool SL_AnimationKeyList<data_t>::init(const size_t keyCount) noexcept
{
    if (!keyCount)
    {
        if (mNumFrames)
        {
            clear();
        }
        return true;
    }

    if (keyCount != mNumFrames)
    {
        mKeyTimes.reset((SL_AnimPrecision*)ls::utils::aligned_malloc(sizeof(SL_AnimPrecision) * keyCount));
        mKeyData.reset((data_t*)ls::utils::aligned_malloc(sizeof(data_t) * keyCount));
    }

    if (!mKeyTimes || !mKeyData)
    {
        clear();
        return false;
    }

    mNumFrames = keyCount;

    for (size_t i = 0; i < keyCount; ++i)
    {
        mKeyTimes[i] = SL_AnimPrecision{0};
        mKeyData[i] = data_t{0};
    }

    return true;
}



/*-------------------------------------
 * Determine if any keyframes exist
-------------------------------------*/
template<typename data_t>
bool SL_AnimationKeyList<data_t>::valid() const noexcept
{
    return mNumFrames > 0;
}



/*-------------------------------------
 * Get the total duration of all keyframes
-------------------------------------*/
template<typename data_t>
SL_AnimPrecision SL_AnimationKeyList<data_t>::duration() const noexcept
{
    return end_time() - start_time();
}



/*-------------------------------------
 * Time of the initial keyframe
-------------------------------------*/
template<typename data_t>
SL_AnimPrecision SL_AnimationKeyList<data_t>::start_time() const noexcept
{
    return mNumFrames ? mKeyTimes[0] : SL_AnimPrecision{0};
}



/*-------------------------------------
 * Time of a starting keyframe, given an offset
-------------------------------------*/
template<typename data_t>
void SL_AnimationKeyList<data_t>::start_time(const SL_AnimPrecision startOffset) noexcept
{
    LS_DEBUG_ASSERT(startOffset >= SL_AnimPrecision{0.0});
    LS_DEBUG_ASSERT(startOffset < SL_AnimPrecision{1.0}); // because somewhere, someone hasn't read the documentation

    const SL_AnimPrecision currentOffset = start_time();
    const SL_AnimPrecision newOffset = currentOffset - startOffset;

    for (size_t i = 0; i < mNumFrames; ++i)
    {
        mKeyTimes[i] = math::clamp<SL_AnimPrecision>(mKeyTimes[i] - newOffset, 0.0, 1.0);
    }
}



/*-------------------------------------
 * Time of the last keyframe
-------------------------------------*/
template<typename data_t>
SL_AnimPrecision SL_AnimationKeyList<data_t>::end_time() const noexcept
{
    return mNumFrames ? mKeyTimes[mNumFrames - 1] : SL_AnimPrecision{0};
}



/*-------------------------------------
 * Time of the keyframe at an index
-------------------------------------*/
template<typename data_t>
SL_AnimPrecision SL_AnimationKeyList<data_t>::frame_time(const size_t keyIndex) const noexcept
{
    LS_DEBUG_ASSERT(keyIndex < mNumFrames);
    return mKeyTimes[keyIndex];
}



/*-------------------------------------
 * Get all keyframes (const)
-------------------------------------*/
template<typename data_t>
const data_t& SL_AnimationKeyList<data_t>::frame_data(const size_t keyIndex) const noexcept
{
    LS_DEBUG_ASSERT(keyIndex < mNumFrames);
    return mKeyData[keyIndex];
}



/*-------------------------------------
 * Get all keyframes
-------------------------------------*/
template<typename data_t>
data_t& SL_AnimationKeyList<data_t>::frame_data(const size_t keyIndex) noexcept
{
    LS_DEBUG_ASSERT(keyIndex < mNumFrames);
    return mKeyData[keyIndex];
}



/*-------------------------------------
 * Retrieve the initial keyframe
-------------------------------------*/
template<typename data_t>
const data_t& SL_AnimationKeyList<data_t>::start_data() const noexcept
{
    LS_DEBUG_ASSERT(mNumFrames > 0);
    return mKeyData[0];
}



/*-------------------------------------
 * Retrieve the last keyframe
-------------------------------------*/
template<typename data_t>
const data_t& SL_AnimationKeyList<data_t>::end_data() const noexcept
{
    LS_DEBUG_ASSERT(mNumFrames > 0);
    return mKeyData[mNumFrames - 1];
}



/*-------------------------------------
 * Assign the keyframe at a specific time
-------------------------------------*/
template<typename data_t>
void SL_AnimationKeyList<data_t>::frame(
    const size_t frameIndex,
    const SL_AnimPrecision frameTime,
    const data_t& frameData
) noexcept
{
    LS_DEBUG_ASSERT(mNumFrames > 0);
    mKeyTimes[frameIndex] = frameTime;
    mKeyData[frameIndex] = frameData;
}



/*-------------------------------------
 * Frame difference interpolator
-------------------------------------*/
template<typename data_t>
SL_AnimPrecision SL_AnimationKeyList<data_t>::calc_frame_interpolation(
    const SL_AnimPrecision totalAnimPercent,
    size_t& outCurrFrame,
    size_t& outNextFrame
) const noexcept
{
    LS_DEBUG_ASSERT(mNumFrames > 0);

    outCurrFrame = 0;
    outNextFrame = 1;

    // If there's one thing I hate more in hot code paths than branches, it's
    // loops. One day I'll find out how to get rid of this. The more key frames
    // that are in an animation channel, the longer this will take to run.
    while (mKeyTimes[outNextFrame] < totalAnimPercent)
    {
        outNextFrame++;
    }

    const size_t prevFrame = outNextFrame-1;
    outCurrFrame = (outNextFrame != 0) ? prevFrame : outNextFrame;

    const SL_AnimPrecision currTime = mKeyTimes[outCurrFrame];
    const SL_AnimPrecision nextTime = mKeyTimes[outNextFrame];
    const SL_AnimPrecision frameDelta = nextTime - currTime;
    const SL_AnimPrecision ret = SL_AnimPrecision{1} - ((nextTime - totalAnimPercent) / frameDelta);

    return ret;
}



/*-------------------------------------
 * Interpolate a set of keyframe
-------------------------------------*/
template<typename data_t>
data_t SL_AnimationKeyList<data_t>::interpolated_data(SL_AnimPrecision percent, const SL_AnimationFlag animFlags) const noexcept
{
    if (percent <= start_time())
    {
        return start_data();
    }

    if (percent >= end_time() && (animFlags & SL_AnimationFlag::SL_ANIM_FLAG_REPEAT) == 0)
    {
        return end_data();
    }

    size_t currFrame, nextFrame;
    SL_AnimPrecision interpAmount = calc_frame_interpolation(percent, currFrame, nextFrame);

    if ((animFlags & SL_AnimationFlag::SL_ANIM_FLAG_IMMEDIATE) != 0)
    {
        interpAmount = SL_AnimPrecision{0.0};
    }

    const data_t& c = mKeyData[currFrame];
    const data_t& n = mKeyData[nextFrame];

    constexpr _SL_AnimInterpolator interpolator;
    return interpolator(c, n, interpAmount);
}



/*-----------------------------------------------------------------------------
 * Pre-Compiled Template Specializations
-----------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(SL_AnimationKeyList, math::vec3);
LS_DEFINE_CLASS_TYPE(SL_AnimationKeyList, math::vec4);
LS_DEFINE_CLASS_TYPE(SL_AnimationKeyList, math::quat);
