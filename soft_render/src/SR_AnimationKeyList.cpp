
#include "lightsky/math/Math.h"

#include "lightsky/utils/Log.h"

#include "soft_render/SR_AnimationKeyList.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Template Specializations
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * 3D vector interpolation
-------------------------------------*/
template<>
math::vec3 SR_AnimationKeyList<math::vec3>::get_interpolated_data(SR_AnimPrecision percent, const SR_AnimationFlag animFlags) const noexcept
{
    if (percent <= get_start_time())
    {
        return get_start_data();
    }

    if (percent >= get_end_time() && (animFlags & SR_AnimationFlag::SR_ANIM_FLAG_REPEAT) == 0)
    {
        return get_end_data();
    }

    size_t currFrame, nextFrame;
    SR_AnimPrecision interpAmount = calc_frame_interpolation(percent, currFrame, nextFrame);

    if ((animFlags & SR_AnimationFlag::SR_ANIM_FLAG_IMMEDIATE) != 0)
    {
        interpAmount = SR_AnimPrecision{0.0};
    }

    const math::vec3& c = mKeyData[currFrame];
    const math::vec3& n = mKeyData[nextFrame];

    return math::mix<float>(c, n, interpAmount);
}

/*-------------------------------------
 * Quaternion interpolation
-------------------------------------*/
template<>
math::quat SR_AnimationKeyList<math::quat>::get_interpolated_data(SR_AnimPrecision percent, const SR_AnimationFlag animFlags) const noexcept
{
    if (percent <= get_start_time())
    {
        return get_start_data();
    }

    if (percent >= get_end_time() && (animFlags & SR_AnimationFlag::SR_ANIM_FLAG_REPEAT) == 0)
    {
        return get_end_data();
    }

    size_t currFrame, nextFrame;
    SR_AnimPrecision interpAmount = calc_frame_interpolation(percent, currFrame, nextFrame);

    if ((animFlags & SR_AnimationFlag::SR_ANIM_FLAG_IMMEDIATE) != 0)
    {
        interpAmount = SR_AnimPrecision{0.0};
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
LS_DEFINE_CLASS_TYPE(SR_AnimationKeyList, math::vec3);

LS_DEFINE_CLASS_TYPE(SR_AnimationKeyList, math::quat);
