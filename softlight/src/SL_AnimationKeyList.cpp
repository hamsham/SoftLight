
#include "lightsky/math/Math.h"

#include "lightsky/utils/Log.h"

#include "softlight/SL_AnimationKeyList.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Template Specializations
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * 3D vector interpolation
-------------------------------------*/
template<>
math::vec3 SL_AnimationKeyList<math::vec3>::interpolated_data(SL_AnimPrecision percent, const SL_AnimationFlag animFlags) const noexcept
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

    const math::vec3& c = mKeyData[currFrame];
    const math::vec3& n = mKeyData[nextFrame];

    return math::mix<float>(c, n, interpAmount);
}

/*-------------------------------------
 * Quaternion interpolation
-------------------------------------*/
template<>
math::quat SL_AnimationKeyList<math::quat>::interpolated_data(SL_AnimPrecision percent, const SL_AnimationFlag animFlags) const noexcept
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

    const math::quat& c = mKeyData[currFrame];
    const math::quat& n = mKeyData[nextFrame];

    // Regular linear interpolations don't work correctly for rotations over
    // 180 degrees.
    return math::slerp<float>(c, n, interpAmount);
}


/*-----------------------------------------------------------------------------
 * Pre-Compiled Template Specializations
-----------------------------------------------------------------------------*/
LS_DEFINE_CLASS_TYPE(SL_AnimationKeyList, math::vec3);

LS_DEFINE_CLASS_TYPE(SL_AnimationKeyList, math::quat);
