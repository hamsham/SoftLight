
#ifndef SR_ANIMATION_KEY_LIST_HPP
#define SR_ANIMATION_KEY_LIST_HPP

#include <utility> // std::move

#include "lightsky/setup/Macros.h" // LS_DECLARE_CLASS_TYPE

#include "lightsky/math/scalar_utils.h" // floor()

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h" // fast_memset()
#include "lightsky/utils/Pointer.h" // aligned_alloc, AlignedDeleter

#include "soft_render/SR_AnimationProperty.hpp"



/*-----------------------------------------------------------------------------
 * SR_Animation Modes
-----------------------------------------------------------------------------*/
enum SR_AnimationFlag : unsigned
{
    SR_ANIM_FLAG_NONE = 0x00, // no interpolation, should be performed.
    SR_ANIM_FLAG_IMMEDIATE = 0x01, // immediately jump from frame to frame.
    SR_ANIM_FLAG_INTERPOLATE = 0x02, // linearly interpolate between the current and next frame.
    SR_ANIM_FLAG_REPEAT = 0x04, // repeat an SR_Animation.

    SR_ANIM_FLAG_DEFAULT = SR_ANIM_FLAG_INTERPOLATE
};


/*-----------------------------------------------------------------------------
 * SR_Animation Key Frame Helper Class (for interpolating animations).
 *
 * FIXME: Animations do not play if only two keyframes are present. At least
 * 3 frames are necessary for an SR_Animation to play.
-----------------------------------------------------------------------------*/
template<typename data_t>
class SR_AnimationKeyList
{
  private:
    /**
     * @brief numPositions contains the total number of position keys.
     */
    size_t mNumFrames;

    /**
     * @brief positionTimes contains the keyframe times of a particular
     * animation's positions.
     */
    ls::utils::Pointer<SR_AnimPrecision[], ls::utils::AlignedDeleter> mKeyTimes;

    /**
     * @brief keyData contains a list of variables which can be
     * interpolated during an animation.
     */
    ls::utils::Pointer<data_t[], ls::utils::AlignedDeleter> mKeyData;

  public:
    /*
     * Destructor
     *
     * Frees all dynamic memory usage from *this.
     */
    ~SR_AnimationKeyList() noexcept;

    /**
     * Constructor
     *
     * Initializes all internal members to their default values. No dynamic
     * memory is allocated at this time.
     */
    SR_AnimationKeyList() noexcept;

    /**
     * Copy Constructor
     *
     * Copies all data from the input parameter into *this.
     *
     * @param k
     * A constant reference to another AnimationKeyList type which contains
     * keyframe data.
     */
    SR_AnimationKeyList(const SR_AnimationKeyList& k) noexcept;

    /**
     * Move Constructor
     *
     * Moves all data from the input parameter into *this. No dynamic
     * allocations are performed.
     *
     * @param k
     * An r-value reference to another AnimationKeyList type which contains
     * keyframe data.
     */
    SR_AnimationKeyList(SR_AnimationKeyList&& k) noexcept;

    /**
     * Copy Operator
     *
     * Copies all data from the input parameter into *this.
     *
     * @param k
     * A constant reference to another AnimationKeyList type which contains
     * keyframe data.
     *
     * @return A reference to *this.
     */
    SR_AnimationKeyList& operator=(const SR_AnimationKeyList& k) noexcept;

    /**
     * Move Operator
     *
     * Moves all data from the input parameter into *this. No dynamic
     * allocations are performed.
     *
     * @param k
     * An r-value reference to another AnimationKeyList type which contains
     * keyframe data.
     *
     * @return A reference to *this.
     */
    SR_AnimationKeyList& operator=(SR_AnimationKeyList&& k) noexcept;

    /**
     * Free all dynamic memory from *this and return the internal
     * members to their default values.
     */
    void clear() noexcept;

    /**
     * Retrieve the number of keyframes in *this.
     *
     * @return A size_t type, containing the current number of key frames
     * contained in *this.
     */
    size_t size() const noexcept;

    /**
     * Initialize and allocate an array of keyframes for *this to use.
     *
     * This method will clear any old keyframes which previously existed in
     * *this.
     *
     * This method is not reentrant.
     *
     * @param keyCount
     * The desired number of keyframes to allocate.
     *
     * @return TRUE if the internal array of keyframes was successfully
     * allocated, FALSE if not.
     */
    bool init(const size_t keyCount) noexcept;

    /**
     * Determine if there are keyframes in *this to use for animation.
     *
     * @return TRUE if *this object contains at least one keyframe to use
     * of FALSE if not.
     */
    bool is_valid() const noexcept;

    /**
     * Retrieve the time difference between the initial keyframe and final
     * keyframe in *this.
     *
     * @note All animations in the library use percentage values between
     * 0.0 and 1.0, inclusive, for time representation unless documented
     * otherwise.
     *
     * @return A floating-point value containing the time difference
     * between the starting and ending keyframes.
     */
    SR_AnimPrecision get_duration() const noexcept;

    /**
     * Retrieve the time of the starting keyframe in *this.
     *
     * @return A floating-point value within the range (0.0, 1.0) which
     * determines when a particular keyframe should be used to start an
     * animation.
     */
    SR_AnimPrecision get_start_time() const noexcept;

    /**
     * Set the time of the starting keyframe in *this.
     *
     * @param startOffset
     * A floating-point value within the range (0.0, 1.0) which determines
     * when a particular keyframe should be used to start an animation.
     */
    void set_start_time(const SR_AnimPrecision startOffset) noexcept;

    /**
     * Retrieve the time of the final keyframe in *this.
     *
     * @return A floating-point value within the range (0.0, 1.0) which
     * determines when a particular keyframe should be used to end an
     * animation.
     */
    SR_AnimPrecision get_end_time() const noexcept;

    /**
     * Retrieve the time of a single keyframe from *this.
     *
     * @return A floating-point value within the range (0.0, 1.0) which
     * determines when a particular keyframe should be used in an
     * animation.
     */
    SR_AnimPrecision get_frame_time(const size_t keyIndex) const noexcept;

    /**
     * Retrieve the data of a particular keyframe.
     *
     * This method will raise an assertion if the index is out of range of
     * any available keys.
     *
     * @param keyIndex
     * An array index to the desired keyframe.
     *
     * @return A constant reference to the data within a keyframe.
     */
    const data_t& get_frame_data(const size_t keyIndex) const noexcept;

    /**
     * Retrieve the data of a particular keyframe.
     *
     * This method will raise an assertion if the index is out of range of
     * any available keys.
     *
     * @param keyIndex
     * An array index to the desired keyframe.
     *
     * @return A reference to the data within a keyframe.
     */
    data_t& get_frame_data(const size_t keyIndex) noexcept;

    /**
     * Retrieve the data of the first keyframe in *this.
     *
     * This method will raise an assertion if there are no available
     * frames to retrieve data from.
     *
     * @return A reference to the initial keyframe's data.
     */
    const data_t& get_start_data() const noexcept;

    /**
     * Retrieve the data of the last keyframe in *this.
     *
     * This method will raise an assertion if there are no available
     * frames to retrieve data from.
     *
     * @return A reference to the last keyframe's data.
     */
    const data_t& get_end_data() const noexcept;

    /**
     * Assign data to a particular frame in *this.
     *
     * This method will raise an assertion if the index is out of range of
     * any available keys.
     *
     * @param keyIndex
     * An array index to the desired keyframe.
     *
     * @param frameTime
     * A time of a single frame, represented as a percentage of an entire
     * animation, in the range (0.0, 1.0).
     *
     * @param frameData
     * A constant reference to the data which will be used for a keyframe
     * at a perticular time.
     */
    void set_frame(const size_t frameIndex, const SR_AnimPrecision frameTime, const data_t& frameData) noexcept;

    /**
     * Retrieve the interpolation between two keyframes closest to the
     * percentage of an overall animation's length.
     *
     * @param percent
     * A floating-point value, representing the overall time that has
     * elapsed in an animation.
     *
     * @param animFlags
     * A set of flags which determines how the output data will be
     * interpolated.
     *
     * @return The interpolation between two animation frames at a given
     * time of an animation.
     */
    data_t get_interpolated_data(SR_AnimPrecision percent, const SR_AnimationFlag animFlags) const noexcept;

    /**
     * Calculate the percent of interpolation which is required to mix the
     * data between two animation frames.
     *
     * @param totalAnimPercent
     * The overall percent of time elapsed in an animation.
     *
     * @param outCurrFrame
     * A reference to an integer, which will contain the array index of the
     * current frame in *this which should be used for interpolation.
     *
     * @param outNextFrame
     * A reference to an integer, which will contain the array index of the
     * next frame in *this which should be used for interpolation.
     *
     * @return A percentage, which should be used to determine the amount
     * of interpolation between the frames at 'outCurrFrane' and
     * 'outNextFrame.' This return value may be greater than the standard
     * range (0.0, 1.0), In such a case, it's up to the function caller to
     * determine what to do.
     */
    SR_AnimPrecision calc_frame_interpolation(
        const SR_AnimPrecision totalAnimPercent,
        size_t& outCurrFrame,
        size_t& outNextFrame
    ) const noexcept;
};



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
SR_AnimationKeyList<data_t>::~SR_AnimationKeyList() noexcept
{
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
SR_AnimationKeyList<data_t>::SR_AnimationKeyList() noexcept :
    mNumFrames{0},
    mKeyTimes{nullptr},
    mKeyData{nullptr}
{}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
SR_AnimationKeyList<data_t>::SR_AnimationKeyList(const SR_AnimationKeyList& a) noexcept :
    mNumFrames{0},
    mKeyTimes{nullptr},
    mKeyData{nullptr}
{
    *this = a;
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
SR_AnimationKeyList<data_t>::SR_AnimationKeyList(SR_AnimationKeyList&& a) noexcept :
    mNumFrames{a.mNumFrames},
    mKeyTimes{std::move(a.mKeyTimes)},
    mKeyData{std::move(a.mKeyData)}
{
    a.mNumFrames = 0;
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
SR_AnimationKeyList<data_t>& SR_AnimationKeyList<data_t>::operator=(const SR_AnimationKeyList& k) noexcept
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
        mKeyTimes.reset(new SR_AnimPrecision[k.mNumFrames]);
        mKeyData.reset(new data_t[k.mNumFrames]);
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
-------------------------------------*/
template<typename data_t>
SR_AnimationKeyList<data_t>& SR_AnimationKeyList<data_t>::operator=(SR_AnimationKeyList&& k) noexcept
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
-------------------------------------*/
template<typename data_t>
void SR_AnimationKeyList<data_t>::clear() noexcept
{
    mNumFrames = 0;
    mKeyTimes.reset();
    mKeyData.reset();
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline size_t SR_AnimationKeyList<data_t>::size() const noexcept
{
    return mNumFrames;
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
bool SR_AnimationKeyList<data_t>::init(const size_t keyCount) noexcept
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
        mKeyTimes.reset(new SR_AnimPrecision[keyCount]);
        mKeyData.reset(new data_t[keyCount]);
    }

    if (!mKeyTimes || !mKeyData)
    {
        clear();
        return false;
    }

    mNumFrames = keyCount;

    for (size_t i = 0; i < keyCount; ++i)
    {
        mKeyTimes[i] = SR_AnimPrecision{0};
        mKeyData[i] = data_t{0};
    }

    return true;
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline bool SR_AnimationKeyList<data_t>::is_valid() const noexcept
{
    return mNumFrames > 0;
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline SR_AnimPrecision SR_AnimationKeyList<data_t>::get_duration() const noexcept
{
    return get_end_time() - get_start_time();
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline SR_AnimPrecision SR_AnimationKeyList<data_t>::get_start_time() const noexcept
{
    return mNumFrames ? mKeyTimes[0] : SR_AnimPrecision{0};
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
void SR_AnimationKeyList<data_t>::set_start_time(const SR_AnimPrecision startOffset) noexcept
{
    LS_DEBUG_ASSERT(startOffset >= SR_AnimPrecision{0.0});
    LS_DEBUG_ASSERT(startOffset < SR_AnimPrecision{1.0}); // because somewhere, someone hasn't read the documentation

    const SR_AnimPrecision currentOffset = get_start_time();
    const SR_AnimPrecision newOffset = currentOffset - startOffset;

    for (size_t i = 0; i < mNumFrames; ++i)
    {
        mKeyTimes[i] = ls::math::clamp<SR_AnimPrecision>(mKeyTimes[i] - newOffset, 0.0, 1.0);
    }
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline SR_AnimPrecision SR_AnimationKeyList<data_t>::get_end_time() const noexcept
{
    return mNumFrames ? mKeyTimes[mNumFrames - 1] : SR_AnimPrecision{0};
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline SR_AnimPrecision SR_AnimationKeyList<data_t>::get_frame_time(const size_t keyIndex) const noexcept
{
    LS_DEBUG_ASSERT(keyIndex < mNumFrames);
    return mKeyTimes[keyIndex];
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline const data_t& SR_AnimationKeyList<data_t>::get_frame_data(const size_t keyIndex) const noexcept
{
    LS_DEBUG_ASSERT(keyIndex < mNumFrames);
    return mKeyData[keyIndex];
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline data_t& SR_AnimationKeyList<data_t>::get_frame_data(const size_t keyIndex) noexcept
{
    LS_DEBUG_ASSERT(keyIndex < mNumFrames);
    return mKeyData[keyIndex];
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline const data_t& SR_AnimationKeyList<data_t>::get_start_data() const noexcept
{
    LS_DEBUG_ASSERT(mNumFrames > 0);
    return mKeyData[0];
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline const data_t& SR_AnimationKeyList<data_t>::get_end_data() const noexcept
{
    LS_DEBUG_ASSERT(mNumFrames > 0);
    return mKeyData[mNumFrames - 1];
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
inline void SR_AnimationKeyList<data_t>::set_frame(
    const size_t frameIndex,
    const SR_AnimPrecision frameTime,
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
inline SR_AnimPrecision SR_AnimationKeyList<data_t>::calc_frame_interpolation(
    const SR_AnimPrecision totalAnimPercent,
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
    while (mKeyTimes[outNextFrame] <= totalAnimPercent && outNextFrame < mNumFrames - 1)
    {
        outCurrFrame++;
        outNextFrame++;
    }

    const SR_AnimPrecision currTime = mKeyTimes[outCurrFrame];
    const SR_AnimPrecision nextTime = mKeyTimes[outNextFrame];
    const SR_AnimPrecision frameDelta = nextTime - currTime;
    const SR_AnimPrecision ret = SR_AnimPrecision{1} - ((nextTime - totalAnimPercent) / frameDelta);

    return ret;
}



/*-------------------------------------
-------------------------------------*/
template<typename data_t>
data_t SR_AnimationKeyList<data_t>::get_interpolated_data(SR_AnimPrecision, const SR_AnimationFlag) const noexcept
{
    LS_ASSERT(false);
    return data_t{};
}



template<>
ls::math::vec3_t<float> SR_AnimationKeyList<ls::math::vec3_t < float>>::get_interpolated_data(SR_AnimPrecision percent, const SR_AnimationFlag animFlags) const noexcept;

template<>
ls::math::quat_t<float> SR_AnimationKeyList<ls::math::quat_t < float>>::get_interpolated_data(SR_AnimPrecision percent, const SR_AnimationFlag animFlags) const noexcept;



/*-----------------------------------------------------------------------------
 * Pre-Compiled Template Specializations
-----------------------------------------------------------------------------*/
LS_DECLARE_CLASS_TYPE(SR_AnimationKeyListVec3, SR_AnimationKeyList, ls::math::vec3_t<float>);
LS_DECLARE_CLASS_TYPE(SR_AnimationKeyListQuat, SR_AnimationKeyList, ls::math::quat_t<float>);


#endif /* SR_ANIMATION_KEY_LIST_HPP */
