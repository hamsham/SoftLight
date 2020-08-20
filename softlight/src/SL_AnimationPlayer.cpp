
#include "lightsky/utils/Assertions.h"

#include "lightsky/math/scalar_utils.h" // fmod

#include "softlight/SL_Animation.hpp"
#include "softlight/SL_AnimationPlayer.hpp"
#include "softlight/SL_SceneGraph.hpp"



/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_AnimationPlayer::~SL_AnimationPlayer() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_AnimationPlayer::SL_AnimationPlayer() noexcept :
    mCurrentState {SL_ANIM_STATE_STOPPED},
    mNumPlays {PLAY_AUTO},
    mCurrentPercent {0.0},
    mDilation {1.0}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_AnimationPlayer::SL_AnimationPlayer(const SL_AnimationPlayer& a) noexcept :
    mCurrentState {a.mCurrentState},
    mNumPlays {a.mNumPlays},
    mCurrentPercent {a.mCurrentPercent},
    mDilation {a.mDilation}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_AnimationPlayer::SL_AnimationPlayer(SL_AnimationPlayer&& a) noexcept :
    mCurrentState {a.mCurrentState},
    mNumPlays {a.mNumPlays},
    mCurrentPercent {a.mCurrentPercent},
    mDilation {a.mDilation}
{
    a.mCurrentState = SL_ANIM_STATE_STOPPED;
    a.mNumPlays = PLAY_AUTO;
    a.mCurrentPercent = 0.0;
    a.mDilation = 1.0;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_AnimationPlayer& SL_AnimationPlayer::operator =(const SL_AnimationPlayer& a) noexcept
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
SL_AnimationPlayer& SL_AnimationPlayer::operator =(SL_AnimationPlayer&& a) noexcept
    {
    mCurrentState = a.mCurrentState;
    a.mCurrentState = SL_ANIM_STATE_STOPPED;

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
unsigned SL_AnimationPlayer::get_num_plays() const noexcept
{
    return mNumPlays;
}



/*-------------------------------------
 * Set the current number of plays
-------------------------------------*/
void SL_AnimationPlayer::set_num_plays(const unsigned playCount) noexcept
{
    mNumPlays = playCount;
}



/*-------------------------------------
 * Get the current tick time (ticks elapsed)
-------------------------------------*/
SL_AnimPrecision SL_AnimationPlayer::get_current_ticks() const noexcept
{
    return mCurrentPercent;
}



/*-------------------------------------
 * Progress an animation
-------------------------------------*/
void SL_AnimationPlayer::tick(SL_SceneGraph& graph, size_t animationIndex, int64_t millis) noexcept
{
    if (mCurrentState != SL_ANIM_STATE_PLAYING)
    {
        return;
    }

    const SL_AlignedVector<SL_Animation>& animations = graph.mAnimations;
    const SL_Animation& anim = animations[animationIndex];

    if (mNumPlays == PLAY_AUTO)
    {
        mNumPlays = (anim.play_mode() == SL_AnimPlayMode::SL_ANIM_PLAY_REPEAT)
                    ? PLAY_REPEAT
                    : PLAY_ONCE;
    }

    if (!mNumPlays)
    {
        stop_anim();
        return;
    }

    const SL_AnimPrecision secondsDelta  = SL_AnimPrecision{0.001} * (SL_AnimPrecision)millis;
    const SL_AnimPrecision ticksDelta    = secondsDelta * anim.ticks_per_sec();
    const SL_AnimPrecision percentDelta  = (ticksDelta * mDilation) / anim.duration();
    const SL_AnimPrecision percentDone   = mCurrentPercent + percentDelta;
    const SL_AnimPrecision nextPercent   = percentDone >= SL_AnimPrecision{0.0} ? percentDone : (SL_AnimPrecision{1}+percentDone);

    anim.animate(graph, nextPercent);

    // check for a looped SL_Animation even when time is going backwards.
    if (percentDone >= SL_AnimPrecision{1}
        || (mCurrentPercent > SL_AnimPrecision{0} && percentDone < SL_AnimPrecision{0}))
    {
        if (mNumPlays != PLAY_REPEAT)
        {
            --mNumPlays;
        }
    }

    mCurrentPercent = ls::math::fmod(nextPercent, SL_AnimPrecision{1});

    if (!mNumPlays)
    {
        stop_anim();
    }
}



/*-------------------------------------
 * Progress an animation
-------------------------------------*/
void SL_AnimationPlayer::tick(SL_SceneGraph& graph, size_t animationIndex, int64_t millis, size_t baseTransformId) noexcept
{
    if (mCurrentState != SL_ANIM_STATE_PLAYING)
    {
        return;
    }

    const SL_AlignedVector<SL_Animation>& animations = graph.mAnimations;
    const SL_Animation& anim = animations[animationIndex];

    if (mNumPlays == PLAY_AUTO)
    {
        mNumPlays = (anim.play_mode() == SL_AnimPlayMode::SL_ANIM_PLAY_REPEAT)
            ? PLAY_REPEAT
            : PLAY_ONCE;
    }

    if (!mNumPlays)
    {
        stop_anim();
        return;
    }

    const SL_AnimPrecision secondsDelta  = SL_AnimPrecision{0.001} * (SL_AnimPrecision)millis;
    const SL_AnimPrecision ticksDelta    = secondsDelta * anim.ticks_per_sec();
    const SL_AnimPrecision percentDelta  = (ticksDelta * mDilation) / anim.duration();
    const SL_AnimPrecision percentDone   = mCurrentPercent + percentDelta;
    const SL_AnimPrecision nextPercent   = percentDone >= SL_AnimPrecision{0.0} ? percentDone : (SL_AnimPrecision{1}+percentDone);

    anim.animate(graph, nextPercent, baseTransformId);

    // check for a looped SL_Animation even when time is going backwards.
    if (percentDone >= SL_AnimPrecision{1}
    || (mCurrentPercent > SL_AnimPrecision{0} && percentDone < SL_AnimPrecision{0}))
    {
        if (mNumPlays != PLAY_REPEAT)
        {
            --mNumPlays;
        }
    }

    mCurrentPercent = ls::math::fmod(nextPercent, SL_AnimPrecision{1});

    if (!mNumPlays)
    {
        stop_anim();
    }
}



/*-------------------------------------
 * Progress an SL_Animation to an explicit time
-------------------------------------*/
SL_AnimPrecision SL_AnimationPlayer::tick_explicit(
    SL_SceneGraph& graph, size_t animationIndex,
    int64_t requestedMillis,
    size_t transformOffset
) const noexcept
{
    const SL_AlignedVector<SL_Animation>& animations = graph.mAnimations;
    const SL_Animation& anim = animations[animationIndex];

    const SL_AnimPrecision secondsDelta  = SL_AnimPrecision{0.001} * (SL_AnimPrecision)requestedMillis;
    const SL_AnimPrecision ticksDelta    = secondsDelta * anim.ticks_per_sec();
    const SL_AnimPrecision percentDelta  = ticksDelta / anim.duration();
    const SL_AnimPrecision percentDone   = percentDelta;
    const SL_AnimPrecision nextPercent   = percentDone >= SL_AnimPrecision{0.0} ? percentDone : (SL_AnimPrecision{1}+percentDone);

    anim.animate(graph, nextPercent, transformOffset);

    const SL_AnimPrecision percentAt = ls::math::fmod(nextPercent, SL_AnimPrecision{1});

    return percentAt * anim.duration();
}



/*-------------------------------------
 * Get the current play state
-------------------------------------*/
SL_AnimationState SL_AnimationPlayer::get_anim_state() const noexcept
{
    return mCurrentState;
}



/*-------------------------------------
 * Set the current play state
-------------------------------------*/
void SL_AnimationPlayer::set_play_state(const SL_AnimationState playState) noexcept
{
    if (mCurrentState == SL_ANIM_STATE_STOPPED && playState == SL_AnimationState::SL_ANIM_STATE_PLAYING)
    {
        mCurrentPercent = 0.0;
    }
    
    mCurrentState = playState;
}



/*-------------------------------------
 * Determine if *this is playing
-------------------------------------*/
bool SL_AnimationPlayer::is_playing() const noexcept
{
    return mCurrentState == SL_ANIM_STATE_PLAYING;
}



/*-------------------------------------
 * Check if paused
-------------------------------------*/
bool SL_AnimationPlayer::is_paused() const noexcept
{
    return mCurrentState == SL_ANIM_STATE_PAUSED;
}



/*-------------------------------------
 * Check if stopped
-------------------------------------*/
bool SL_AnimationPlayer::is_stopped() const noexcept
{
    return mCurrentState == SL_ANIM_STATE_STOPPED;
}



/*-------------------------------------
 * Force stop
-------------------------------------*/
void SL_AnimationPlayer::stop_anim() noexcept
{
    mCurrentState = SL_ANIM_STATE_STOPPED;
    mCurrentPercent = 0.0;
}



/*-------------------------------------
 * Get the time speed-up or slow-down value
-------------------------------------*/
SL_AnimPrecision SL_AnimationPlayer::get_time_dilation() const noexcept
{
    return mDilation;
}



/*-------------------------------------
 * Set the time speed-up or slow-down value
-------------------------------------*/
void SL_AnimationPlayer::set_time_dilation(const SL_AnimPrecision percentNormalTime) noexcept
{
    mDilation = percentNormalTime;
}



/*-------------------------------------
 * Reset all internal parameters
-------------------------------------*/
void SL_AnimationPlayer::reset() noexcept
{
    mCurrentState = SL_ANIM_STATE_STOPPED;
    mNumPlays = PLAY_AUTO;
    mCurrentPercent = 0.0;
    mDilation = 1.0;
}
