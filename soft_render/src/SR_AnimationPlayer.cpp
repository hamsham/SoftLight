
#include "lightsky/utils/Assertions.h"

#include "lightsky/math/scalar_utils.h" // fmod

#include "soft_render/SR_Animation.hpp"
#include "soft_render/SR_AnimationPlayer.hpp"
#include "soft_render/SR_SceneGraph.hpp"



/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_AnimationPlayer::~SR_AnimationPlayer() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_AnimationPlayer::SR_AnimationPlayer() noexcept :
    mCurrentState {SR_ANIM_STATE_STOPPED},
    mNumPlays {PLAY_AUTO},
    mCurrentPercent {0.0},
    mDilation {1.0}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SR_AnimationPlayer::SR_AnimationPlayer(const SR_AnimationPlayer& a) noexcept :
    mCurrentState {a.mCurrentState},
    mNumPlays {a.mNumPlays},
    mCurrentPercent {a.mCurrentPercent},
    mDilation {a.mDilation}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_AnimationPlayer::SR_AnimationPlayer(SR_AnimationPlayer&& a) noexcept :
    mCurrentState {a.mCurrentState},
    mNumPlays {a.mNumPlays},
    mCurrentPercent {a.mCurrentPercent},
    mDilation {a.mDilation}
{
    a.mCurrentState = SR_ANIM_STATE_STOPPED;
    a.mNumPlays = PLAY_AUTO;
    a.mCurrentPercent = 0.0;
    a.mDilation = 1.0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SR_AnimationPlayer& SR_AnimationPlayer::operator =(const SR_AnimationPlayer& a) noexcept
    {
    mCurrentState = a.mCurrentState;
    mNumPlays = a.mNumPlays;
    mCurrentPercent = a.mCurrentPercent;
    mDilation = a.mDilation;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SR_AnimationPlayer& SR_AnimationPlayer::operator =(SR_AnimationPlayer&& a) noexcept
    {
    mCurrentState = a.mCurrentState;
    a.mCurrentState = SR_ANIM_STATE_STOPPED;

    mNumPlays = a.mNumPlays;
    a.mNumPlays = PLAY_AUTO;

    mCurrentPercent = a.mCurrentPercent;
    a.mCurrentPercent = 0.0;

    mDilation = a.mDilation;
    a.mDilation = 1.0;

    return *this;
}



/*-------------------------------------
 * Get the current number of plays
-------------------------------------*/
unsigned SR_AnimationPlayer::get_num_plays() const noexcept
{
    return mNumPlays;
}



/*-------------------------------------
 * Set the current number of plays
-------------------------------------*/
void SR_AnimationPlayer::set_num_plays(const unsigned playCount) noexcept
{
    mNumPlays = playCount;
}



/*-------------------------------------
 * Get the current tick time (ticks elapsed)
-------------------------------------*/
SR_AnimPrecision SR_AnimationPlayer::get_current_ticks() const noexcept
{
    return mCurrentPercent;
}



/*-------------------------------------
 * Progress an animation
-------------------------------------*/
void SR_AnimationPlayer::tick(SR_SceneGraph& graph, size_t animationIndex, int64_t millis) noexcept
{
    if (mCurrentState != SR_ANIM_STATE_PLAYING)
    {
        return;
    }

    const std::vector<SR_Animation>& animations = graph.mAnimations;
    const SR_Animation& anim = animations[animationIndex];

    if (mNumPlays == PLAY_AUTO)
    {
        mNumPlays = (anim.play_mode() == SR_AnimPlayMode::SR_ANIM_PLAY_REPEAT)
                    ? PLAY_REPEAT
                    : PLAY_ONCE;
    }

    if (!mNumPlays)
    {
        stop_anim();
        return;
    }

    const SR_AnimPrecision secondsDelta  = SR_AnimPrecision{0.001} * (SR_AnimPrecision)millis;
    const SR_AnimPrecision ticksDelta    = secondsDelta * anim.ticks_per_sec();
    const SR_AnimPrecision percentDelta  = (ticksDelta * mDilation) / anim.duration();
    const SR_AnimPrecision percentDone   = mCurrentPercent + percentDelta;
    const SR_AnimPrecision nextPercent   = percentDone >= SR_AnimPrecision{0.0} ? percentDone : (SR_AnimPrecision{1}+percentDone);

    anim.animate(graph, nextPercent);

    // check for a looped SR_Animation even when time is going backwards.
    if (percentDone >= SR_AnimPrecision{1}
        || (mCurrentPercent > SR_AnimPrecision{0} && percentDone < SR_AnimPrecision{0}))
    {
        if (mNumPlays != PLAY_REPEAT)
        {
            --mNumPlays;
        }
    }

    mCurrentPercent = ls::math::fmod(nextPercent, SR_AnimPrecision{1});

    if (!mNumPlays)
    {
        stop_anim();
    }
}



/*-------------------------------------
 * Progress an animation
-------------------------------------*/
void SR_AnimationPlayer::tick(SR_SceneGraph& graph, size_t animationIndex, int64_t millis, size_t baseTransformId) noexcept
{
    if (mCurrentState != SR_ANIM_STATE_PLAYING)
    {
        return;
    }

    const std::vector<SR_Animation>& animations = graph.mAnimations;
    const SR_Animation& anim = animations[animationIndex];

    if (mNumPlays == PLAY_AUTO)
    {
        mNumPlays = (anim.play_mode() == SR_AnimPlayMode::SR_ANIM_PLAY_REPEAT)
            ? PLAY_REPEAT
            : PLAY_ONCE;
    }

    if (!mNumPlays)
    {
        stop_anim();
        return;
    }

    const SR_AnimPrecision secondsDelta  = SR_AnimPrecision{0.001} * (SR_AnimPrecision)millis;
    const SR_AnimPrecision ticksDelta    = secondsDelta * anim.ticks_per_sec();
    const SR_AnimPrecision percentDelta  = (ticksDelta * mDilation) / anim.duration();
    const SR_AnimPrecision percentDone   = mCurrentPercent + percentDelta;
    const SR_AnimPrecision nextPercent   = percentDone >= SR_AnimPrecision{0.0} ? percentDone : (SR_AnimPrecision{1}+percentDone);

    anim.animate(graph, nextPercent, baseTransformId);

    // check for a looped SR_Animation even when time is going backwards.
    if (percentDone >= SR_AnimPrecision{1}
    || (mCurrentPercent > SR_AnimPrecision{0} && percentDone < SR_AnimPrecision{0}))
    {
        if (mNumPlays != PLAY_REPEAT)
        {
            --mNumPlays;
        }
    }

    mCurrentPercent = ls::math::fmod(nextPercent, SR_AnimPrecision{1});

    if (!mNumPlays)
    {
        stop_anim();
    }
}



/*-------------------------------------
 * Progress an SR_Animation to an explicit time
-------------------------------------*/
SR_AnimPrecision SR_AnimationPlayer::tick_explicit(
    SR_SceneGraph& graph, size_t animationIndex,
    int64_t requestedMillis,
    size_t transformOffset
) const noexcept
{
    const std::vector<SR_Animation>& animations = graph.mAnimations;
    const SR_Animation& anim = animations[animationIndex];

    const SR_AnimPrecision secondsDelta  = SR_AnimPrecision{0.001} * (SR_AnimPrecision)requestedMillis;
    const SR_AnimPrecision ticksDelta    = secondsDelta * anim.ticks_per_sec();
    const SR_AnimPrecision percentDelta  = ticksDelta / anim.duration();
    const SR_AnimPrecision percentDone   = percentDelta;
    const SR_AnimPrecision nextPercent   = percentDone >= SR_AnimPrecision{0.0} ? percentDone : (SR_AnimPrecision{1}+percentDone);

    anim.animate(graph, nextPercent, transformOffset);

    const SR_AnimPrecision percentAt = ls::math::fmod(nextPercent, SR_AnimPrecision{1});

    return percentAt * anim.duration();
}



/*-------------------------------------
 * Get the current play state
-------------------------------------*/
SR_AnimationState SR_AnimationPlayer::get_anim_state() const noexcept
{
    return mCurrentState;
}



/*-------------------------------------
 * Set the current play state
-------------------------------------*/
void SR_AnimationPlayer::set_play_state(const SR_AnimationState playState) noexcept
{
    if (mCurrentState == SR_ANIM_STATE_STOPPED && playState == SR_AnimationState::SR_ANIM_STATE_PLAYING)
    {
        mCurrentPercent = 0.0;
    }
    
    mCurrentState = playState;
}



/*-------------------------------------
 * Determine if *this is playing
-------------------------------------*/
bool SR_AnimationPlayer::is_playing() const noexcept
{
    return mCurrentState == SR_ANIM_STATE_PLAYING;
}



/*-------------------------------------
 * Check if paused
-------------------------------------*/
bool SR_AnimationPlayer::is_paused() const noexcept
{
    return mCurrentState == SR_ANIM_STATE_PAUSED;
}



/*-------------------------------------
 * Check if stopped
-------------------------------------*/
bool SR_AnimationPlayer::is_stopped() const noexcept
{
    return mCurrentState == SR_ANIM_STATE_STOPPED;
}



/*-------------------------------------
 * Force stop
-------------------------------------*/
void SR_AnimationPlayer::stop_anim() noexcept
{
    mCurrentState = SR_ANIM_STATE_STOPPED;
    mCurrentPercent = 0.0;
}



/*-------------------------------------
 * Get the time speed-up or slow-down value
-------------------------------------*/
SR_AnimPrecision SR_AnimationPlayer::get_time_dilation() const noexcept
{
    return mDilation;
}



/*-------------------------------------
 * Set the time speed-up or slow-down value
-------------------------------------*/
void SR_AnimationPlayer::set_time_dilation(const SR_AnimPrecision percentNormalTime) noexcept
{
    mDilation = percentNormalTime;
}



/*-------------------------------------
 * Reset all internal parameters
-------------------------------------*/
void SR_AnimationPlayer::reset() noexcept
{
    mCurrentState = SR_ANIM_STATE_STOPPED;
    mNumPlays = PLAY_AUTO;
    mCurrentPercent = 0.0;
    mDilation = 1.0;
}
