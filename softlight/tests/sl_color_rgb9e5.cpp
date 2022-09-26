// This test was extrapolated directly from the source code provided by the
// OpenGL extension for EXT_texture_shared_exponent:
// https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_shared_exponent.txt

#include <iostream>

#include "lightsky/utils/Assertions.h"

#include "lightsky/math/scalar_utils.h"

#include "softlight/SL_Color.hpp"
#include "softlight/SL_Plane.hpp"

namespace math = ls::math;



struct SL_RGB9e5Properties
{
    enum SL_RGB9e5Limits : int32_t
    {
        RGB9E5_EXPONENT_BITS        = 5,
        RGB9E5_MANTISSA_BITS        = 9,
        RGB9E5_EXP_BIAS             = 15,
        RGB9E5_MAX_VALID_BIASED_EXP = 31
    };

    static constexpr int32_t MAX_RGB9E5_EXP         = RGB9E5_MAX_VALID_BIASED_EXP - RGB9E5_EXP_BIAS;
    static constexpr int32_t RGB9E5_MANTISSA_VALUES = 1 << RGB9E5_MANTISSA_BITS;
    static constexpr int32_t MAX_RGB9E5_MANTISSA    = RGB9E5_MANTISSA_VALUES - 1;
};



struct BitsOfIEEE754
{
    uint32_t mantissa : 23;
    uint32_t biasedexponent : 8;
    uint32_t negative : 1;
};



union float754
{
    uint32_t raw;
    float value;
    BitsOfIEEE754 field;
};



struct BitsOfRGB9E5
{
    uint32_t r : SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS;
    uint32_t g : SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS;
    uint32_t b : SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS;
    uint32_t biasedexponent : SL_RGB9e5Properties::RGB9E5_EXPONENT_BITS;
};



union rgb9e5
{
    unsigned int raw;
    BitsOfRGB9E5 field;
};



inline float ClampRange_for_rgb9e5(float x) noexcept
{
    constexpr float MAX_RGB9E5 = (float)SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA / (float)SL_RGB9e5Properties::RGB9E5_MANTISSA_VALUES * (float)(1 << SL_RGB9e5Properties::MAX_RGB9E5_EXP);

    if (x > 0.f)
    {
        return (x >= MAX_RGB9E5) ? MAX_RGB9E5 : x;
    }

    // NaN gets here too since comparisons with NaN always fail!
    return 0.f;
}

#if defined(LS_X86_SSE)
inline __m128 ClampRange_for_rgb9e5(__m128 x) noexcept
{
    constexpr float MAX_RGB9E5 = (float)SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA / (float)SL_RGB9e5Properties::RGB9E5_MANTISSA_VALUES * (float)(1 << SL_RGB9e5Properties::MAX_RGB9E5_EXP);

    const __m128 maxRgbe5 = _mm_set1_ps(MAX_RGB9E5);
    const __m128 zero = _mm_xor_ps(maxRgbe5, maxRgbe5);
    const __m128 clampHi = _mm_min_ps(x, maxRgbe5);
    return _mm_max_ps(clampHi, zero);
}
#endif



// FloorLog2 is not correct for the denorm and zero values, but we are going to
// do a max of this value with the minimum rgb9e5 exponent that will hide these
// problem cases.
inline int FloorLog2(float x) noexcept
{
    #if defined(LS_X86_SSE)
        const __m128i xi = _mm_castps_si128(_mm_set1_ps(x));
        const __m128i exponent = _mm_slli_epi32(xi, 1);
        const __m128i biased = _mm_srli_epi32(exponent, 24);
        return _mm_cvtsi128_si32(_mm_sub_epi32(biased, _mm_set1_epi32(127)));

    #else
        float754 f;
        f.value = x;

        return (f.field.biasedexponent - 127);
    #endif
}

#if defined(LS_X86_SSE)
inline __m128i FloorLog2(__m128 x) noexcept
{
    const __m128i xi = _mm_castps_si128(x);
    const __m128i exponent = _mm_slli_epi32(xi, 1);
    const __m128i biased = _mm_srli_epi32(exponent, 24);
    return _mm_sub_epi32(biased, _mm_set1_epi32(127));
}
#endif


#if defined(LS_X86_SSE2)
inline __m128 max_rgb(__m128 x) noexcept
{
    const __m128 r = _mm_permute_ps(x, 0x00);
    const __m128 g = _mm_permute_ps(x, 0x55);
    const __m128 b = _mm_permute_ps(x, 0xAA);

    return _mm_max_ps(_mm_max_ps(g, r), b);
}
#endif



#if defined(LS_X86_SSE)
inline __m128 rgb_exp2(__m128 x) noexcept
{
    const __m128 s126   = _mm_set1_ps(-126.f);
    const __m128 clipp  = _mm_max_ps(x, s126);
    const __m128 clipp2 = _mm_add_ps(clipp, _mm_set1_ps(126.94269504f));
    const __m128i v     = _mm_cvtps_epi32(_mm_mul_ps(_mm_set1_ps((float)(1 << 23)), clipp2));

    return _mm_castsi128_ps(v);
}
#endif


rgb9e5 float3_to_rgb9e5(const SL_ColorRGBf& rgb) noexcept
{
    #if defined(LS_X86_AVX2)
        const __m128 rgbc = ClampRange_for_rgb9e5(_mm_maskload_ps(rgb.v, _mm_set_epi32(0, -1, -1, -1))); // AVX2

        const __m128 maxrgb = max_rgb(rgbc);
        __m128i exp_shared = _mm_max_epi32(_mm_set1_epi32(-SL_RGB9e5Properties::RGB9E5_EXP_BIAS-1), FloorLog2(maxrgb));
        exp_shared = _mm_add_epi32(exp_shared, _mm_set1_epi32(1 + SL_RGB9e5Properties::RGB9E5_EXP_BIAS));

        // This pow function could be replaced by a table.
        const __m128i expBiased = _mm_sub_epi32(_mm_sub_epi32(exp_shared, _mm_set1_epi32(SL_RGB9e5Properties::RGB9E5_EXP_BIAS)), _mm_set1_epi32(SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS));
        __m128 denom = rgb_exp2(_mm_cvtepi32_ps(expBiased));

        __m128 rDenom = _mm_rcp_ps(denom);
        const __m128i maxm = _mm_cvtps_epi32(_mm_floor_ps(_mm_fmadd_ps(maxrgb, rDenom, _mm_set1_ps(0.5f)))); // SSE4.1
        const __m128i cmpMantissa = _mm_cmpeq_epi32(maxm, _mm_set1_epi32(SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA + 1));
        denom = _mm_add_ps(denom, _mm_and_ps(denom, _mm_castsi128_ps(cmpMantissa)));
        exp_shared = _mm_add_epi32(exp_shared, _mm_and_si128(_mm_set1_epi32(1), cmpMantissa));

        rDenom = _mm_rcp_ps(denom);
        const __m128i rgbam = _mm_cvtps_epi32(_mm_floor_ps(_mm_fmadd_ps(rgbc, rDenom, _mm_set1_ps(0.5f)))); // FMA

        const __m128i retR = _mm_slli_epi32(_mm_shuffle_epi32(rgbam, 0x00), 0);
        const __m128i retG = _mm_slli_epi32(_mm_shuffle_epi32(rgbam, 0x55), 9);
        const __m128i retB = _mm_slli_epi32(_mm_shuffle_epi32(rgbam, 0xAA), 18);
        const __m128i retA = _mm_slli_epi32(exp_shared, 27);

        const __m128i retVal0 = _mm_or_si128(retG, retR);
        const __m128i retVal1 = _mm_or_si128(retA, retB);
        const __m128i retVal = _mm_or_si128(retVal1, retVal0);

        union
        {
            const int32_t i;
            const rgb9e5 rgb95;
        } ret{_mm_cvtsi128_si32(retVal)};
        return ret.rgb95;

    #else
        const float rc = ClampRange_for_rgb9e5(rgb[0]);
        const float gc = ClampRange_for_rgb9e5(rgb[1]);
        const float bc = ClampRange_for_rgb9e5(rgb[2]);

        const float maxrgb = math::max<float>(rc, gc, bc);
        int exp_shared = math::max<int>(-SL_RGB9e5Properties::RGB9E5_EXP_BIAS-1, (int)FloorLog2(maxrgb));
        exp_shared = exp_shared + 1 + SL_RGB9e5Properties::RGB9E5_EXP_BIAS;

        LS_ASSERT(exp_shared <= SL_RGB9e5Properties::RGB9E5_MAX_VALID_BIASED_EXP);
        LS_ASSERT(exp_shared >= 0);

        // This pow function could be replaced by a table.
        float denom = math::exp2((float)(exp_shared - SL_RGB9e5Properties::RGB9E5_EXP_BIAS - SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS));

        float rDenom = math::rcp(denom);
        const int maxm = (int)math::floor(math::fmadd(maxrgb, rDenom, 0.5f));
        if (maxm == SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA + 1)
        {
            //denom *= 2.f;
            denom += denom;
            exp_shared += 1;
            LS_ASSERT(exp_shared <= SL_RGB9e5Properties::RGB9E5_MAX_VALID_BIASED_EXP);
        }
        else
        {
            LS_ASSERT(maxm <= SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA);
        }

        rDenom = math::rcp(denom);
        const int rm = (int)math::floor(math::fmadd(rc, rDenom, 0.5f));
        const int gm = (int)math::floor(math::fmadd(gc, rDenom, 0.5f));
        const int bm = (int)math::floor(math::fmadd(bc, rDenom, 0.5f));

        LS_ASSERT(rm <= SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA);
        LS_ASSERT(gm <= SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA);
        LS_ASSERT(bm <= SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA);
        LS_ASSERT(rm >= 0);
        LS_ASSERT(gm >= 0);
        LS_ASSERT(bm >= 0);

        //std::cout << rm << ", " << gm << ", " << bm << ", " << exp_shared << std::endl;

        rgb9e5 retval;
        retval.field.r = rm;
        retval.field.g = gm;
        retval.field.b = bm;
        retval.field.biasedexponent = exp_shared;

        return retval;

    #endif
}



inline SL_ColorRGBf rgb9e5_to_float3(rgb9e5 v) noexcept
{
    #if defined(LS_X86_AVX2)
        const __m128i rgb = _mm_set1_epi32((int)v.raw);
        const __m128i biasedExponent = _mm_srli_epi32(rgb, 27);
        const __m128i exponenti = _mm_sub_epi32(_mm_sub_epi32(biasedExponent, _mm_set1_epi32(SL_RGB9e5Properties::RGB9E5_EXP_BIAS)), _mm_set1_epi32(SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS));
        const __m128 exponent = _mm_cvtepi32_ps(exponenti);
        const __m128 scale = rgb_exp2(exponent);
        const __m128i rgbi = _mm_srli_epi32(_mm_sllv_epi32(rgb, _mm_set_epi32(0, 5, 14, 23)), 23);
        const __m128 rgbf = _mm_mul_ps(_mm_cvtepi32_ps(rgbi), scale);

        SL_ColorRGBf ret;
        _mm_maskstore_ps(ret.v, _mm_set_epi32(0, -1, -1, -1), rgbf);
        return ret;

    #else
        const float exponent = (float)(v.field.biasedexponent - SL_RGB9e5Properties::RGB9E5_EXP_BIAS - SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS);
        const float scale = math::exp2(exponent);

        return SL_ColorRGBf
        {
            (float)v.field.r * scale,
            (float)v.field.g * scale,
            (float)v.field.b * scale
        };
    #endif
}



/*-----------------------------------------------------------------------------
 * plane from 3 points
-----------------------------------------------------------------------------*/
typedef math::vec4_t<float> SL_HdrPlane;
//typedef math::vec4_t<math::half> SL_HdrPlane;

/*
struct SL_HdrPlane
{
    rgb9e5 normal;
    float distance;
};
*/



inline float xyz_to_index(const SL_ColorRGBf& xyz) noexcept
{
    constexpr unsigned w = 256;
    constexpr unsigned h = 256;
    constexpr unsigned d = 256;

    const unsigned a = (unsigned)(xyz[0] * (float)(w-1));
    const unsigned b = (unsigned)(xyz[1] * (float)(h-1));
    const unsigned c = (unsigned)(xyz[2] * (float)(d-1));

    unsigned index = a + w * (b + (h * c));
    //std::cout << "INDEX IN: " << index << std::endl;

    return (float)index / (float)(w * h * d - 1);
}



inline SL_ColorRGBf index_to_xyz(float index) noexcept
{
    constexpr unsigned w = 256;
    constexpr unsigned h = 256;
    constexpr unsigned d = 256;

    // Using 1-fmod_1(index) helps to serve as a "fudge-factor" to make outputs
    // more accurate by adjusting the rounding.
    unsigned i = (unsigned)(index * (float)(w * h * d - 1));

    //std::cout << "INDEX OUT: " << i << std::endl;

    const float x = (float)(i % w);
    const float y = (float)((i / w) % d);
    const float z = (float)(i / (w * h));

    return SL_ColorRGBf{(float)x, (float)y, (float)z} / math::vec3{(float)(w-1), (float)(h-1), (float)(d-1)};
}



inline math::vec3 line_plane_intersection(const SL_Plane& p, const math::vec3& l0, const math::vec3& l1) noexcept
{
    const math::vec3&& u = l1 - l0;
    const float dot = math::dot(math::vec3_cast(p), u);

    // Skip the check for a plane being coplanar to the line. That will never
    // happen as we have predefined the input line as intersecting the plane.
    const math::vec3&& pco = math::vec3_cast(p) * (-p[3] / math::length_squared(math::vec3_cast(p)));
    const math::vec3&& w = l0 - pco;
    const float fac = -math::dot(math::vec3_cast(p), w) / dot;
    return l0 + (u * fac);
}



inline SL_HdrPlane colors_to_plane(const SL_ColorRGBf& a, const SL_ColorRGBf& b, const SL_ColorRGBf& c) noexcept
{
    const float index0 = xyz_to_index(a);
    const float index1 = xyz_to_index(b);
    const float index2 = xyz_to_index(c);

    const math::vec3 x{index0, 0.f, 0.f};
    const math::vec3 y{0.f, index1, 0.f};
    const math::vec3 z{0.f, 0.f, index2};

    const SL_Plane&& plane = sl_plane_from_points(x, y, z);

    //std::cout << "PLANE IN: " << plane[0] << ", " << plane[1] << ", " << plane[2] << ", " << plane[3] << std::endl;

    return (SL_HdrPlane)plane;

    /*
    SL_HdrPlane ret;
    ret.normal = float3_to_rgb9e5(math::vec3_cast(plane));
    ret.distance = plane[3];
    return ret;
    */
}



inline void plane_to_colors(const SL_HdrPlane& plane, SL_ColorRGBf& a, SL_ColorRGBf& b, SL_ColorRGBf& c) noexcept
{
    //std::cout << "PLANE OUT: " << plane[0] << ", " << plane[1] << ", " << plane[2] << ", " << plane[3] << std::endl;

    //const SL_Plane&& p = math::vec4_cast(rgb9e5_to_float3(plane.normal), plane.distance);
    const SL_Plane&& p = (SL_Plane)plane;

    const SL_ColorRGBf&& x = line_plane_intersection(p, math::vec3{0.f, 0.f, 0.f}, math::vec3{1.f, 0.f, 0.f});
    const SL_ColorRGBf&& y = line_plane_intersection(p, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    const SL_ColorRGBf&& z = line_plane_intersection(p, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 0.f, 1.f});
    a = index_to_xyz(x[0]);
    b = index_to_xyz(y[1]);
    c = index_to_xyz(z[2]);
}



/*-----------------------------------------------------------------------------
 * Main (for testing only)
 *
 * g++ -std=c++11 -Wall -Werror -Wextra -pedantic -pedantic-errors SL_ColorType.cpp -o color_cvt
-----------------------------------------------------------------------------*/
int main()
{
    SL_ColorRGB8 rgb8 = {42, 77, 193};
    SL_ColorRGBf rgbf = color_cast<float, uint8_t>(rgb8);

    rgb9e5 rgb9 = float3_to_rgb9e5(rgbf);
    std::cout << "RGB8:   " << (unsigned)rgb8[0] << ", " << (unsigned)rgb8[1] << ", " << (unsigned)rgb8[2] << std::endl;
    std::cout << "RGBf:   " << rgbf[0] << ", " << rgbf[1] << ", " << rgbf[2] << std::endl;
    std::cout << "RGB9e5: " << rgb9.field.r << ", " << rgb9.field.g << ", " << rgb9.field.b << std::endl;

    rgbf = rgb9e5_to_float3(rgb9);
    rgb8 = color_cast<uint8_t, float>(rgbf);

    std::cout << "RGB8:   " << (unsigned)rgb8[0] << ", " << (unsigned)rgb8[1] << ", " << (unsigned)rgb8[2] << std::endl;
    std::cout << "RGBf:   " << rgbf[0] << ", " << rgbf[1] << ", " << rgbf[2] << std::endl;

    SL_ColorRGB8 a0{13, 77, 92};
    SL_ColorRGB8 b0{42, 255, 168};
    SL_ColorRGB8 c0{168, 39, 254};

    SL_ColorRGBf a = color_cast<float, uint8_t>(a0);
    SL_ColorRGBf b = color_cast<float, uint8_t>(b0);
    SL_ColorRGBf c = color_cast<float, uint8_t>(c0);

    std::cout << std::endl;

    std::cout << "D: " << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
    const float i = xyz_to_index(a);
    const SL_ColorRGBf d = index_to_xyz(i);
    std::cout << "D: " << d[0] << ", " << d[1] << ", " << d[2] << std::endl;

    std::cout << std::endl;

    std::cout << "A: " << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
    std::cout << "B: " << b[0] << ", " << b[1] << ", " << b[2] << std::endl;
    std::cout << "C: " << c[0] << ", " << c[1] << ", " << c[2] << std::endl;
    const SL_HdrPlane&& plane = colors_to_plane(a, b, c);
    plane_to_colors(plane, a, b, c);
    std::cout << "A: " << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
    std::cout << "B: " << b[0] << ", " << b[1] << ", " << b[2] << std::endl;
    std::cout << "C: " << c[0] << ", " << c[1] << ", " << c[2] << std::endl;

    a0 = color_cast<uint8_t, float>(a);
    b0 = color_cast<uint8_t, float>(b);
    c0 = color_cast<uint8_t, float>(c);
    std::cout << "A: " << (unsigned)a0[0] << ", " << (unsigned)a0[1] << ", " << (unsigned)a0[2] << std::endl;
    std::cout << "B: " << (unsigned)b0[0] << ", " << (unsigned)b0[1] << ", " << (unsigned)b0[2] << std::endl;
    std::cout << "C: " << (unsigned)c0[0] << ", " << (unsigned)c0[1] << ", " << (unsigned)c0[2] << std::endl;

    return 0;
}

