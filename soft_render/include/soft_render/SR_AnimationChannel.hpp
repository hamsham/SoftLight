
#ifndef SR_ANIMATION_REEL_HPP
#define SR_ANIMATION_REEL_HPP

#include "lightsky/utils/Pointer.h"

#include "lightsky/math/vec3.h"
#include "lightsky/math/quat.h"

#include "soft_render/SR_AnimationProperty.hpp"
#include "soft_render/SR_AnimationKeyList.hpp"



/*-----------------------------------------------------------------------------
 * SR_Animation Keys (interpolations of animations).
-----------------------------------------------------------------------------*/
struct SR_AnimationChannel
{
    /**
     * @brief animationMode Is a bit-flag which is used during
     * "getFrame(...)". This helps to identify if different frames of an
     * Animation should be interpolated/non-interpolated, repeated/non-
     * repeated.
     */
    SR_AnimationFlag mAnimMode;

    /**
     * @brief positionFrames contains the self-relative positioning of a
     * node.
     */
    SR_AnimationKeyListVec3 mPosFrames;

    /**
     * @brief scaleFrames contains the self-relative scaling of a node.
     */
    SR_AnimationKeyListVec3 mScaleFrames;

    /**
     * @brief rotationFrames contains the self-relative orientation of a
     * node.
     */
    SR_AnimationKeyListQuat mOrientFrames;

    /**
     * @brief Destructor
     *
     * Frees all dynamic memory from *this.
     */
    ~SR_AnimationChannel() noexcept;

    /**
     * @brief Constructor
     *
     * Initializes all internal pointers to NULL.
     */
    SR_AnimationChannel() noexcept;

    /**
     * @brief Copy Constructor
     *
     * Copies all data from the input parameter into *this.
     *
     * @param a
     * A constant reference to another animationReel object.
     */
    SR_AnimationChannel(const SR_AnimationChannel& a) noexcept;

    /**
     * @brief Move Constructor
     *
     * Moves all data from the input parameter into *this without
     * performing any copies.
     *
     * @param a
     * An r-value reference to another animationReel object.
     */
    SR_AnimationChannel(SR_AnimationChannel&& a) noexcept;

    /**
     * @brief Copy Operator
     *
     * Copies all data from the input parameter into *this. All previously
     * stored data will be deleted.
     *
     * @param a
     * A constant reference to another animationReel object.
     *
     * @return A reference to *this.
     */
    SR_AnimationChannel& operator=(const SR_AnimationChannel& a) noexcept;

    /**
     * @brief Move Operator
     *
     * Moves all data from the input parameter into *this without
     * performing any copies. All previously stored data will be deleted.
     *
     * @param a
     * An r-value reference to another animationReel object.
     *
     * @return A reference to *this.
     */
    SR_AnimationChannel& operator=(SR_AnimationChannel&& ak) noexcept;

    /**
     * @brief Get the bitmasked Animation flags used by *this during
     * interpolation.
     *
     * @return A bitmask of the interpolation modes that are used by *this.
     */
    SR_AnimationFlag flags() const noexcept;

    /**
     * @brief Set the number of frames in *this to animate.
     *
     * Caling this method will delete all current data.
     *
     * @param positionKeys
     * The number of frames to allocate for position interpolation.
     *
     * @param scalingKeys
     * The number of frames to allocate for position interpolation.
     *
     * @param rotationKeys
     * The number of frames to allocate for position interpolation.
     *
     * @return TRUE if the number of requested frames were successfully
     * allocated, FALSE if not.
     */
    bool size(
        const size_t positionKeys,
        const size_t scalingKeys,
        const size_t rotationKeys
    ) noexcept;

    /**
     * @brief Clear all frames from *this and reset all internal members.
     */
    void clear() noexcept;

    /**
     * Run a simple check to determine if there are position frames in *this
     * which can be used for scene node animations, given a percent of the
     * animation which has already played.
     * 
     * @param animPercent
     * The percentage of the entire animation which has been played.
     * 
     * @return TRUE if there are positions in *this which can be used for
     * animations, FALSE if not.
     */
    bool has_position_frame(const SR_AnimPrecision animPercent) const noexcept;

    /**
     * Run a simple check to determine if there are scaling frames in *this
     * which can be used for scene node animations, given a percent of the
     * animation which has already played.
     * 
     * @param animPercent
     * The percentage of the entire animation which has been played.
     * 
     * @return TRUE if there are scaling in *this which can be used for
     * animations, FALSE if not.
     */
    bool has_scale_frame(const SR_AnimPrecision animPercent) const noexcept;

    /**
     * Run a simple check to determine if there are rotation frames in *this
     * which can be used for scene node animations, given a percent of the
     * animation which has already played.
     * 
     * @param animPercent
     * The percentage of the entire animation which has been played.
     * 
     * @return TRUE if there are rotations in *this which can be used for
     * animations, FALSE if not.
     */
    bool has_rotation_frame(const SR_AnimPrecision animPercent) const noexcept;

    /**
     * @brief Set the position of a frame.
     *
     * @param frameIndex
     * The array-offset to an Animation frame which is to be updated. This
     * index must be less than the value returned by "getNumFrames()".
     * 
     * @param percent
     * A percentage from 0 - 1, inclusive, which determines at what point
     * within the total duration of an animation that this keyframe should be
     * set. An assertion will be raised if the percent is greater than one from
     * the previous keyframe.
     *
     * @param pos
     * A constant reference to a 3D vector, containing the node-relative
     * position which will be stored in *this at a particular frame.
     */
    void position_frame(
        const unsigned frameIndex,
        const SR_AnimPrecision percent,
        const ls::math::vec3_t<float>& pos
    ) noexcept;

    /**
     * @brief Retrieve the position that a node should be during a
     * particular frame.
     *
     * @param percent
     * The percent of the position reel, clamped between 0 and 1, which
     * has been completed.
     *
     * @return A 3D vector, containing the node-relative position which
     * will be stored in *this at a particular frame.
     */
    ls::math::vec3_t<float> position_frame(const SR_AnimPrecision percent) const noexcept;

    /**
     * @brief Set the scale of a frame.
     *
     * @param frameIndex
     * The array-offset to an Animation frame which is to be updated. This
     * index must be less than the value returned by "getNumFrames()".
     * 
     * @param percent
     * A percentage from 0 - 1, inclusive, which determines at what point
     * within the total duration of an animation that this keyframe should be
     * set. An assertion will be raised if the percent is greater than one from
     * the previous keyframe.
     *
     * @param scale
     * A constant reference to a 3D vector, containing the node-relative
     * scaling which will be stored in *this at a particular frame.
     */
    void scale_frame(
        const unsigned frameIndex,
        const SR_AnimPrecision percent,
        const ls::math::vec3_t<float>& scale
    ) noexcept;

    /**
     * @brief Retrieve the scaling that a node should contain during a
     * particular frame.
     *
     * @param percent
     * The percent of the scaling reel, clamped between 0 and 1, which
     * has been completed.
     *
     * @return A 3D vector containing the node-relative scaling which will
     * be stored in *this at a particular frame.
     */
    ls::math::vec3_t<float> scale_frame(const SR_AnimPrecision percent) const noexcept;

    /**
     * @brief Set the rotation of a frame.
     *
     * @param frameIndex
     * The array-offset to an Animation frame which is to be updated. This
     * index must be less than the value returned by "getNumFrames()".
     * 
     * @param percent
     * A percentage from 0 - 1, inclusive, which determines at what point
     * within the total duration of an animation that this keyframe should be
     * set. An assertion will be raised if the percent is greater than one from
     * the previous keyframe.
     *
     * @param rotation
     * A constant reference to a quaternion, containing the node-relative
     * rotation which will be stored in *this at a particular frame.
     */
    void rotation_frame(
        const unsigned frameIndex,
        const SR_AnimPrecision percent,
        const ls::math::quat_t<float>& rotation
    ) noexcept;

    /**
     * @brief Retrieve the rotation that should be applied to a scene node
     * during a particular frame.
     *
     * @param percent
     * The percent of the rotation reel, clamped between 0 and 1, which
     * has been completed.
     *
     * @return A quaternion containing the node-relative rotation which
     * will be stored in *this at a particular frame.
     */
    ls::math::quat_t<float> rotation_frame(const SR_AnimPrecision percent) const noexcept;

    /**
     * @brief Retrieve the position, scale, and rotation of a node at a
     * percentage of its total frame index.
     *
     * This method is intended for use by the "Animation" object, which
     * contains the total time that an Animation reel operates at.
     *
     * This method returns immediately if the current Animation flags
     * are equal to "ANIM_NONE."
     *
     * @param outPosition
     * A reference to a 3D vector, containing the position of a node at a
     * given percentage of its Animation reel.
     *
     * @param outscale
     * A reference to a 3D vector, containing the scaling of a node at a
     * given percentage of its Animation reel.
     *
     * @param outRotation
     * A reference to a quaternion, containing the rotation of a node at a
     * given percentage of its Animation reel.
     *
     * @param percentFinished
     * The percent of the Animation reel, clamped between 0 and 1, which
     * has been completed.
     *
     * @return TRUE if an interpolation was performed, FALSE if not.
     */
    bool frame(
        ls::math::vec3_t<float>& outPosition,
        ls::math::vec3_t<float>& outscale,
        ls::math::quat_t<float>& outRotation,
        const SR_AnimPrecision percentFinished // currentTime / timePerSecond
    ) const noexcept;

    /**
     * Retrieve the time of the first keyframe in *this.
     * 
     * @return A floating-point number, containing the time at which *this
     * animation channel starts running.
     */
    SR_AnimPrecision start_time() const noexcept;

    /**
     * Set the start time for all keyframes in *this. All internal keyframes
     * will use this time as an offset before playing in an animation player.
     * 
     * @param startOffset
     * A floating-point number, containing the time at which *this animation
     * channel starts running.
     */
    void start_time(const SR_AnimPrecision startOffset) noexcept;

    /**
     * Retrieve the time of the last keyframe in *this.
     * 
     * @return A floating-point number, containing the time at which *this
     * animation channel finishes running.
     */
    SR_AnimPrecision end_time() const noexcept;

    /**
     * Retrieve the total running time of *this animation channel.
     * 
     * @return A floating-point number, containing the difference between the
     * last keyframe time and the initial keyframe time.
     */
    SR_AnimPrecision duration() const noexcept;
};



/*-------------------------------------
 * Rotation availability check
-------------------------------------*/
inline bool SR_AnimationChannel::has_position_frame(const SR_AnimPrecision animPercent) const noexcept
{
    //return mPosFrames.valid();
    return animPercent >= mPosFrames.start_time() && animPercent <= mPosFrames.end_time();
}



/*-------------------------------------
 * Rotation availability check
-------------------------------------*/
inline bool SR_AnimationChannel::has_scale_frame(const SR_AnimPrecision animPercent) const noexcept
{
    //return mScaleFrames.valid();
    return animPercent >= mScaleFrames.start_time() && animPercent <= mScaleFrames.end_time();
}



/*-------------------------------------
 * Rotation availability check
-------------------------------------*/
inline bool SR_AnimationChannel::has_rotation_frame(const SR_AnimPrecision animPercent) const noexcept
{
    //return mOrientFrames.valid();
    return animPercent >= mOrientFrames.start_time() && animPercent <= mOrientFrames.end_time();
}



/*-------------------------------------
 * Set a single position key
-------------------------------------*/
inline void SR_AnimationChannel::position_frame(
    const unsigned frameIndex,
    const SR_AnimPrecision percent,
    const ls::math::vec3_t<float>& pos
) noexcept
{
    LS_DEBUG_ASSERT(percent >= -1.0 && percent <= 1.0);
    mPosFrames.frame(frameIndex, percent, pos);
}



/*-------------------------------------
 * Set a single scale key
-------------------------------------*/
inline void SR_AnimationChannel::scale_frame(
    const unsigned frameIndex,
    const SR_AnimPrecision percent,
    const ls::math::vec3_t<float>& scale
) noexcept
{
    LS_DEBUG_ASSERT(percent >= -1.0 && percent <= 1.0);
    mScaleFrames.frame(frameIndex, percent, scale);
}



/*-------------------------------------
 * Set a single rotation key
-------------------------------------*/
inline void SR_AnimationChannel::rotation_frame(
    const unsigned frameIndex,
    const SR_AnimPrecision percent,
    const ls::math::quat_t<float>& rotation
) noexcept
{
    LS_DEBUG_ASSERT(percent >= -1.0 && percent <= 1.0);
    mOrientFrames.frame(frameIndex, percent, rotation);
}



/*-------------------------------------
 * Get a single position key
-------------------------------------*/
inline ls::math::vec3 SR_AnimationChannel::position_frame(const SR_AnimPrecision percent) const noexcept
{
    return mPosFrames.interpolated_data(percent, mAnimMode);
}



/*-------------------------------------
 * Get a single scale key
-------------------------------------*/
inline ls::math::vec3 SR_AnimationChannel::scale_frame(const SR_AnimPrecision percent) const noexcept
{
    return mScaleFrames.interpolated_data(percent, mAnimMode);
}



/*-------------------------------------
 * Get a single rotaion key
-------------------------------------*/
inline ls::math::quat SR_AnimationChannel::rotation_frame(const SR_AnimPrecision percent) const noexcept
{
    return mOrientFrames.interpolated_data(percent, mAnimMode);
}



/*-------------------------------------
 * SR_Animation Key Interpolator
-------------------------------------*/
inline bool SR_AnimationChannel::frame(
    ls::math::vec3& outPosition,
    ls::math::vec3& outscale,
    ls::math::quat& outRotation,
    const SR_AnimPrecision percentFinished
) const noexcept
{
    outPosition = position_frame(percentFinished);
    outscale = scale_frame(percentFinished);
    outRotation = rotation_frame(percentFinished);
    return true;
}



/*-------------------------------------
 * Retrieve the total track running time
-------------------------------------*/
inline SR_AnimPrecision SR_AnimationChannel::duration() const noexcept
{
    return end_time() - start_time();
}



#endif /* SR_ANIMATION_REEL_HPP */
