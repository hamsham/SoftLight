
#ifndef SR_ANIMATION_HPP
#define SR_ANIMATION_HPP

#include <vector>
#include <string>
#include <utility> // std::forward

#include "lightsky/utils/Hash.h"

#include "soft_render/SR_AnimationProperty.hpp"
#include "soft_render/SR_AnimationChannel.hpp"



/*-----------------------------------------------------------------------------
 * Forward declaration of scene types
-----------------------------------------------------------------------------*/
struct SR_SceneNode;
class SR_SceneGraph;


/**------------------------------------
 * Animation playback
 *
 * This enumeration allows for an Animation object to determine the number of
 * times an Animation should play. It is used by animationPlayer objects to
 * determine if an Animation plays once or multiple times.
-------------------------------------*/
enum SR_AnimPlayMode
{
    SR_ANIM_PLAY_ONCE,
    SR_ANIM_PLAY_REPEAT,
    SR_ANIM_PLAY_DEFAULT = SR_AnimPlayMode::SR_ANIM_PLAY_ONCE
};



/**----------------------------------------------------------------------------
 * @brief The Animation object is used to animate nodes in a scene graph.
 *
 * This class keeps track of a single Animation, made up of 'tracks' or
 * keyframes, that are used to animate one or more meshes.
------------------------------------------------------------------------------*/
class SR_Animation
{

    // Allow scene graphs to delete animation tracks based on the scene node
    friend class SR_SceneGraph;

  private:
    /**
     * @brief playMode is by Animation players to determine if an Animation
     * loops one or more times.
     */
    SR_AnimPlayMode mPlayMode;

    /**
     * @brief animationId contains a hash value, from 'animName', which is
     * used to provide an instance of this class with a unique identifier.
     */
    ls::utils::hash_t mAnimId;

    /**
     * @brief totalTicks contains the number of ticks, or duration, of an
     * Animation.
     */
    SR_AnimPrecision mTotalTicks;

    /**
     * @brief ticksPerSec determines how many ticks an Animation needs per
     * second to play.
     */
    SR_AnimPrecision mTicksPerSec;

    /**
     * @brief animName is used alongside 'animationId' to provide this
     * class with a unique, human-readable, identifier.
     */
    std::string mName;

    /**
     * @brief mChannelIds contains ID of the std::vector<AnimationChannel>
     * which will be used to identify a SceneNode's animation channel to use.
     */
    std::vector<size_t> mChannelIds;

    /**
     * @brief mTrackIds are used after the mChannelIds to determine the exact
     * AnimationChannel in a list of animation channels to use for an
     * animation.
     */
    std::vector<size_t> mTrackIds;

    /**
     * @brief transformIds contains the indices of all node transformations
     * that will contain the resulting transformation after an animation.
     */
    std::vector<size_t> mTransformIds;

  public: // public member functions
    /**
     * @brief Destructor
     *
     * Cleans up all memory and resources used.
     */
    ~SR_Animation() noexcept;

    /**
     * @brief Constructor
     *
     * Initializes all members to their default values.
     */
    SR_Animation() noexcept;

    /**
     * @brief Copy Constructor
     *
     * Copies all data from the input parameter into *this.
     *
     * @param a
     * A constant reference to another Animation object.
     */
    SR_Animation(const SR_Animation& a) noexcept;

    /**
     * @brief Move Constructor
     *
     * Moves all data from the input parameter into *this without
     * performing any reallocations. The input parameter is restored to
     * its default state after this call.
     *
     * @param a
     * An r-value reference to a temporary Animation object.
     */
    SR_Animation(SR_Animation&& a) noexcept;

    /**
     * @brief Copy Operator
     *
     * Copies all data from the input parameter into *this.
     *
     * @param a
     * A constant reference to another Animation object.
     *
     * @return A reference to *this.
     */
    SR_Animation& operator=(const SR_Animation& a) noexcept;

    /**
     * @brief Move Operator
     *
     * Moves all data from the input parameter into *this without
     * performing any reallocations. The input parameter is restored to
     * its default state after this call.
     *
     * @param a
     * An r-value reference to a temporary Animation object.
     *
     * @return A reference to *this.
     */
    SR_Animation& operator=(SR_Animation&& a) noexcept;

    /**
     * @brief Determine the current play mode used by *this.
     *
     * @return An enumeration which can be used to determine if *this has
     * a looping or non-looping Animation.
     */
    SR_AnimPlayMode play_mode() const noexcept;

    /**
     * @brief Set the current play mode used by *this.
     *
     * @param animMode
     * An enumeration which can be used to determine if *this will contain
     * a looping or non-looping Animation.
     */
    void play_mode(const SR_AnimPlayMode animMode) noexcept;

    /**
     * @brief Retrieve the unique, hashed, identifier that can be used to
     * reference *this.
     *
     * @return An unsigned integer, containing the hashed value of *this
     * Animation's name.
     */
    size_t id() const noexcept;

    /**
     * @brief Retrieve the name of *this Animation.
     *
     * @return A constant reference to a constant string, containing the
     * name of *this.
     */
    const std::string& name() const noexcept;

    /**
     * @brief Set *this Animation's name.
     *
     * Caling this function will reset *this Animation's unique integer ID
     * to the hash of the input name.
     *
     * @param name
     * A constant reference to a string, containing the new name of *this.
     */
    template<typename std_string_type>
    void name(std_string_type&& name) noexcept;

    /**
     * @brief Get the duration, in ticks, of *this Animation.
     *
     * Ticks have no units and are merely used to transition from one pont
     * in time to another. They are similar to 'seconds' in the same way
     * that 'radians' are similar to 'degrees'.
     *
     * @return A SR_AnimPrecision-precision float, containing the number of ticks
     * that an Animation plays.
     */
    SR_AnimPrecision duration() const noexcept;

    /**
     * @brief Set the duration, in ticks, of *this Animation.
     *
     * @param ticks
     * A SR_AnimPrecision-precision float, containing the number of ticks that an
     * Animation will play.
     */
    void duration(const SR_AnimPrecision ticks) noexcept;

    /**
     * @brief Get playback speed, in ticks per second, that *this object
     * will run at.
     *
     * @return A SR_AnimPrecision-precision float, containing the speed of *this
     * Animation in ticks/sec.
     */
    SR_AnimPrecision ticks_per_sec() const noexcept;

    /**
     * @brief Set playback speed, in ticks per second, that *this object
     * will run at.
     *
     * @param numTicks
     * A SR_AnimPrecision-precision float, containing the speed of *this Animation in
     * ticks/sec.
     */
    void ticks_per_sec(const SR_AnimPrecision numTicks) noexcept;

    /**
     * @brief Retrieve the list of indices which are used to reference scene
     * nodes transformations in a scene graph
     * (through the SceneGraph::currentTransform member).
     *
     * @return A reference to a constant vector of indices which reference
     * the "currentTransform" objects in a SceneGraph.
     */
    const std::vector<size_t>& transforms() const noexcept;

    /**
     * @brief Retrieve the list of indices which are used to reference scene
     * nodes transformations in a scene graph
     * (through the SceneGraph::currentTransform member).
     *
     * @return A reference to a constant vector of indices which reference
     * the "currentTransform" objects in a SceneGraph.
     */
    std::vector<size_t>& transforms() noexcept;

    /**
     * @brief Retrieve the list of indices which will be used to reference a
     * node-specific animation channel from a scene graph.
     *
     * Each sub-list of keyframes contained within the return value can
     * reference its own sceneNode. This means that only one animationReel
     * can reference a single sceneNode, but *this can animate multiple
     * sceneNodes by using multiple animationReels.
     *
     * @return A reference to a constant vector of indices which will
     * reference single AnimationChannel objects in a SceneGraph's array of
     * per-node animation channels
     * (SceneGraph::nodeAnims[animTrackId][nodeTrackId]).
     */
    const std::vector<size_t>& tracks() const noexcept;

    /**
     * @brief Retrieve the list of indices which will be used to reference
     * lists of node animation channels from a scene graph.
     * 
     * Multiple scene nodes can reference the same array of AnimationChannels
     * in a scene graph.
     *
     * @return A reference to a constant vector of indices which will
     * reference an array of AnimationChannel objects in a SceneGraph.
     * (SceneGraph::nodeAnims[animTrackId]).
     */
    const std::vector<size_t>& animations() const noexcept;

    /**
     * @brief Get the number of Animation channels that will be animated by
     * *this.
     * 
     * @return The total number of node channels which *this Animation object
     * runs during any given frame.
     */
    size_t size() const noexcept;

    /**
     * @brief Add an Animation channel to *this.
     * 
     * @param node
     * A constant reference to the scene node which will be animated.
     *
     * @param nodeTrackId
     * An unsigned integer, containing the index of the AnimationChannel in
     * the input node's std::vector<AnimationChannel> to use for animation.
     */
    void add_channel(const SR_SceneNode& node, const size_t nodeTrackId) noexcept;

    /**
     * Remove a single Animation channel from *this.
     *
     * @param trackId
     * The index of the Animation channel to remove.
     */
    void erase(const size_t trackId) noexcept;

    /**
     * Remove all Animation keyframes and channels inside of *this.
     */
    void clear() noexcept;

    /**
     * @brief Reserve a number of animation channels to help avoid the chances
     * of a reallocation when adding single animations.
     */
    void reserve(const size_t reserveSize) noexcept;

    /**
     * @brief Animate nodes in a sceneGraph.
     *
     * This function will permanently update the model matrix contained
     * within the animated sceneNodes until otherwise specified.
     *
     * @param graph
     * A reference to a sceneGraph object who's internal nodes will be
     * transformed according to the keyframes in *this.
     *
     * @param percentDone
     * The percent of the animation which has been played in total. An
     * assertion will be raised if this value is less than 0.0.
     */
    void animate(SR_SceneGraph& graph, const SR_AnimPrecision percentDone) const noexcept;

    /**
     * @brief Animate nodes in a sceneGraph.
     *
     * This function will permanently update the model matrix contained
     * within the animated sceneNodes until otherwise specified.
     *
     * This version of "animate()" should only be called if the animation being
     * used contains sequential scene nodes (i.e. animating a skeleton).
     *
     * @param graph
     * A reference to a sceneGraph object who's internal nodes will be
     * transformed according to the keyframes in *this.
     *
     * @param percentDone
     * The percent of the animation which has been played in total. An
     * assertion will be raised if this value is less than 0.0.
     *
     * @param baseTransformId
     * An index of a root transformation in the scene graph. If the offset is
     * nonzero, the number of subsequent transformations in the scene graph
     * must match the number of transform IDs contained within *this.
     */
    void animate(SR_SceneGraph& graph, const SR_AnimPrecision percentDone, size_t baseTransformId) const noexcept;

    /**
     * Initialize the animation transformations for all nodes in a scene graph.
     * 
     * @param graph
     * A reference to a sceneGraph object who's internal nodes will be
     * transformed according to the initial keyframes in *this.
     * 
     * @param atStart
     * Determines if the animation should be initialized with data from the
     * first set of keyframes or the last.
     */
    void init(SR_SceneGraph& graph, const bool atStart = true) const noexcept;

    /**
     * @brief Determine if the transformations referenced by *this map to a
     * contiguous set of transforms in a scene graph.
     *
     * @return TRUE if *this animation references a contiguous set of scene
     * node transformations, FALSE if not.
     */
    bool have_monotonic_transforms() const noexcept;
};



/*-------------------------------------
 * Set the SR_Animation name
-------------------------------------*/
template<typename std_string_type>
void SR_Animation::name(std_string_type&& name) noexcept
{
    mAnimId = ls::utils::string_hash(name.c_str());
    mName = std::forward<std_string_type>(name);
}



#endif /* SR_ANIMATION_HPP */
