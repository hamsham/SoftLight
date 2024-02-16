/*
 * File:   SL_PackedVertex.cpp
 * Author: hammy
 * Created on February 16, 2024, at 7:57 AM
 */

#include "lightsky/utils/Assertions.h"

#include "softlight/SL_PackedVertex.hpp"


/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace
{

constexpr float _max_rgb9e5_exp() noexcept
{
    return (float)(1 << (SL_PackedVertex_9e5::RGB9E5_MAX_VALID_BIASED_EXP - SL_PackedVertex_9e5::RGB9E5_EXP_BIAS));
}

constexpr float _rgb9e5_mantissa_values() noexcept
{
    return (float)(1 << SL_PackedVertex_9e5::RGB9E5_MANTISSA_BITS);
}

constexpr float _max_rgb9e5_mantissa() noexcept
{
    return (float)(_rgb9e5_mantissa_values() - 1);
}

constexpr float _max_rgb9e5() noexcept
{
    return _max_rgb9e5_mantissa() / _rgb9e5_mantissa_values() * _max_rgb9e5_exp();
}

// FloorLog2 is not correct for the denorm and zero values, but we are
// going to do a max of this value with the minimum rgb9e5 exponent that
// will hide these problem cases.
inline int32_t _floor_log2(float x) noexcept
{
    struct alignas(alignof(uint32_t)) BitsOfIEEE754
    {
        uint32_t mantissa : 23;
        uint32_t biasedexponent : 8;
        uint32_t negative : 1;
    };

    union float754
    {
        float value;
        BitsOfIEEE754 field;
    } f{x};

    return (f.field.biasedexponent - 127);
}

} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * Shared-exponent RGB9e5
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * RGB9e5 packing
-------------------------------------*/
uint32_t SL_PackedVertex_9e5::_pack_vector(const ls::math::vec3& rgb) noexcept
{
    const float rc     = ls::math::clamp<float>(rgb[0], 0.f, _max_rgb9e5());
    const float gc     = ls::math::clamp<float>(rgb[1], 0.f, _max_rgb9e5());
    const float bc     = ls::math::clamp<float>(rgb[2], 0.f, _max_rgb9e5());
    const float maxrgb = ls::math::max<float>(rc, gc, bc);

    int32_t exp_shared
        = ls::math::max<int32_t>(-RGB9E5_EXP_BIAS-1, _floor_log2(maxrgb))
            + 1 + RGB9E5_EXP_BIAS;

    LS_ASSERT(exp_shared <= RGB9E5_MAX_VALID_BIASED_EXP);
    LS_ASSERT(exp_shared >= 0);

    // This pow function could be replaced by a table.
    float denom = std::exp2((float)(exp_shared - RGB9E5_EXP_BIAS - RGB9E5_MANTISSA_BITS));

    float rDenom = 1.f / denom;
    const int32_t maxm = (int32_t)std::floor(ls::math::fmadd(maxrgb, rDenom, 0.5f));
    if (maxm == (1 << (int32_t)RGB9E5_MANTISSA_BITS))
    {
        //denom *= 2.f;
        denom += denom;
        exp_shared += 1;

        //LS_ASSERT(exp_shared <= RGB9E5_MAX_VALID_BIASED_EXP);
        if (exp_shared > RGB9E5_MAX_VALID_BIASED_EXP)
        {
            return 0xFFFFFFFFu;
        }
    }
    else
    {
        //LS_ASSERT(maxm <= (int32_t)_max_rgb9e5_mantissa());
        if (maxm > (int32_t)_max_rgb9e5_mantissa())
        {
            return 0xFFFFFFFFu;
        }
    }

    rDenom = 1.f / denom;
    const int32_t rm = (int32_t)std::floor(ls::math::fmadd(rc, rDenom, 0.5f));
    const int32_t gm = (int32_t)std::floor(ls::math::fmadd(gc, rDenom, 0.5f));
    const int32_t bm = (int32_t)std::floor(ls::math::fmadd(bc, rDenom, 0.5f));

    //LS_ASSERT(rm <= (int32_t)_max_rgb9e5_mantissa());
    //LS_ASSERT(gm <= (int32_t)_max_rgb9e5_mantissa());
    //LS_ASSERT(bm <= (int32_t)_max_rgb9e5_mantissa());
    //LS_ASSERT(rm >= 0);
    //LS_ASSERT(gm >= 0);
    //LS_ASSERT(bm >= 0);

    SL_PackedVertex_9e5 result;
    result.mField.r = rm;
    result.mField.g = gm;
    result.mField.b = bm;
    result.mField.biasedexponent = exp_shared;

    return result.mRaw;
}
