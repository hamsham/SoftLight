// This test was extrapolated directly from the source code provided by the
// OpenGL extension for EXT_texture_shared_exponent:
// https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_shared_exponent.txt

#include <iostream>

#include "lightsky/utils/Assertions.h"

#include "lightsky/math/scalar_utils.h"

#include "softlight/SL_PackedVertex.hpp"
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

    static constexpr float MAX_RGB9E5_EXP         = (float)(1 << (RGB9E5_MAX_VALID_BIASED_EXP - RGB9E5_EXP_BIAS));
    static constexpr float RGB9E5_MANTISSA_VALUES = (float)(1 << RGB9E5_MANTISSA_BITS);
    static constexpr float MAX_RGB9E5_MANTISSA    = (float)(RGB9E5_MANTISSA_VALUES - 1);
    static constexpr float MAX_RGB9E5             = MAX_RGB9E5_MANTISSA / RGB9E5_MANTISSA_VALUES * MAX_RGB9E5_EXP;
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



union sl_rgb9e5f
{
    unsigned int raw;
    BitsOfRGB9E5 field;
};



inline float rgb9e5_clamp(float x) noexcept
{
    return math::clamp(x, 0.f, SL_RGB9e5Properties::MAX_RGB9E5);
}



// FloorLog2 is not correct for the denorm and zero values, but we are going to
// do a max of this value with the minimum rgb9e5 exponent that will hide these
// problem cases.
inline int rgb9e5_floor_log2(float x) noexcept
{
    float754 f;
    f.value = x;

    return (f.field.biasedexponent - 127);
}


sl_rgb9e5f float3_to_rgb9e5(const SL_ColorRGBf& rgb) noexcept
{
    const float rc = rgb9e5_clamp(rgb[0]);
    const float gc = rgb9e5_clamp(rgb[1]);
    const float bc = rgb9e5_clamp(rgb[2]);

    const float maxrgb = math::max<float>(rc, gc, bc);
    int exp_shared = math::max<int>(-SL_RGB9e5Properties::RGB9E5_EXP_BIAS-1, (int)rgb9e5_floor_log2(maxrgb));
    exp_shared = exp_shared + 1 + SL_RGB9e5Properties::RGB9E5_EXP_BIAS;

    LS_ASSERT(exp_shared <= SL_RGB9e5Properties::RGB9E5_MAX_VALID_BIASED_EXP);
    LS_ASSERT(exp_shared >= 0);

    // This pow function could be replaced by a table.
    float denom = std::exp2((float)(exp_shared - SL_RGB9e5Properties::RGB9E5_EXP_BIAS - SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS));

    float rDenom = 1.f / denom;
    const int maxm = (int)std::floor(math::fmadd(maxrgb, rDenom, 0.5f));
    if (maxm == (1 << (int)SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS))
    {
        //denom *= 2.f;
        denom += denom;
        exp_shared += 1;
        LS_ASSERT(exp_shared <= SL_RGB9e5Properties::RGB9E5_MAX_VALID_BIASED_EXP);
    }
    else
    {
        LS_ASSERT(maxm <= (int)SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA);
    }

    rDenom = 1.f / denom;
    const int rm = (int)std::floor(math::fmadd(rc, rDenom, 0.5f));
    const int gm = (int)std::floor(math::fmadd(gc, rDenom, 0.5f));
    const int bm = (int)std::floor(math::fmadd(bc, rDenom, 0.5f));

    LS_ASSERT(rm <= (int)SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA);
    LS_ASSERT(gm <= (int)SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA);
    LS_ASSERT(bm <= (int)SL_RGB9e5Properties::MAX_RGB9E5_MANTISSA);
    LS_ASSERT(rm >= 0);
    LS_ASSERT(gm >= 0);
    LS_ASSERT(bm >= 0);

    //std::cout << rm << ", " << gm << ", " << bm << ", " << exp_shared << std::endl;

    sl_rgb9e5f retval;
    retval.field.r = rm;
    retval.field.g = gm;
    retval.field.b = bm;
    retval.field.biasedexponent = exp_shared;

    return retval;
}



inline SL_ColorRGBf rgb9e5_to_float3(sl_rgb9e5f v) noexcept
{
    const float exponent = (float)(v.field.biasedexponent - SL_RGB9e5Properties::RGB9E5_EXP_BIAS - SL_RGB9e5Properties::RGB9E5_MANTISSA_BITS);
    const float scale = std::exp2(exponent);

    return SL_ColorRGBf
    {
        (float)v.field.r * scale,
        (float)v.field.g * scale,
        (float)v.field.b * scale
    };
}



/*-----------------------------------------------------------------------------
 * plane from 3 points
-----------------------------------------------------------------------------*/
typedef math::fixed_t<int8_t, 7> PlaneFpType;

struct OctNormFp
{
    typedef math::fixed_t<int32_t, 12> FpType;
    FpType x;
    FpType y;
};


float calc_psnr(float maxVal, float inX, float inY, float inZ, float testX, float testY, float testZ)
{
    float x = inX - testX;
    float y = inY - testY;
    float z = inZ - testZ;

    x *= x;
    y *= y;
    z *= z;

    float mse = (x + y + z) / 3.f;
    if (!mse)
    {
        return 100.f;
    }

    float inv = std::sqrt(mse);
    float logTen = std::log10(maxVal / inv);
    return 20.f * logTen;
}


float calc_psnr(
    float maxVal,
    const math::vec3& inX, const math::vec3& inY, const math::vec3& inZ,
    const math::vec3& testX, const math::vec3& testY, const math::vec3& testZ)
{
    math::vec3&& x = inX - testX;
    math::vec3&& y = inY - testY;
    math::vec3&& z = inZ - testZ;

    x *= x;
    y *= y;
    z *= z;

    float mse = (math::sum(x) + math::sum(y) + math::sum(z)) / 3.f;
    if (!mse)
    {
        return 100.f;
    }

    float inv = std::sqrt(mse);
    float logTen = std::log10(maxVal / inv);
    return 20.f * logTen;
}


struct LinearIndexScheme
{
    static inline uint32_t encode_index3d(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        constexpr uint32_t w = 255;
        constexpr uint32_t h = 255;
        return x + w * (y + (h * z));
    }

    static inline void decode_index3d(uint32_t index, uint32_t& x, uint32_t& y, uint32_t& z) noexcept
    {
        constexpr uint32_t w = 255;
        constexpr uint32_t h = 255;
        constexpr uint32_t d = 255;

        x = index % w;
        y = (index / w) % d;
        z = index / (w * h);
    }
};



struct Uint24IndexScheme
{
    static inline uint32_t encode_index3d(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        uint32_t ret = (x & 0xFF) << 0u;
        ret |= (y & 0xFF) << 8u;
        ret |= (z & 0xFF) << 16u;
        return ret;
    }

    static inline void decode_index3d(uint32_t index, uint32_t& x, uint32_t& y, uint32_t& z) noexcept
    {
        x = (index >> 0) & 0xFFu;
        y = (index >> 8) & 0xFFu;
        z = (index >> 16) & 0xFFu;
    }
};



// forcibly encode the decimal digits as the lower 24 float bits to reduce
// precision loss.
struct MortonIndexScheme
{
    static inline uint32_t encode_index3d(uint32_t x, uint32_t y, uint32_t z) noexcept
    {
        const auto spread = [](uint32_t w) noexcept -> uint32_t
        {
            uint64_t m = (uint64_t)w & 0x00000000001fffffull;
            m = (m | m << 32ull) & 0x001f00000000ffffull;
            m = (m | m << 16ull) & 0x001f0000ff0000ffull;
            m = (m | m <<  8ull) & 0x010f00f00f00f00full;
            m = (m | m <<  4ull) & 0x10c30c30c30c30c3ull;
            m = (m | m <<  2ull) & 0x1249249249249249ull;
            return (uint32_t)m;
        };

        return (spread(x)) | (spread(y) << 1) | (spread(z) << 2);
    }

    static inline void decode_index3d(uint32_t index, uint32_t& x, uint32_t& y, uint32_t& z) noexcept
    {
        const auto compact = [](uint32_t w) noexcept -> uint32_t
        {
            uint64_t m = (uint64_t)w & 0x1249249249249249ull;
            m = (m ^ (m >> 2ull))  & 0x30c30c30c30c30c3ull;
            m = (m ^ (m >> 4ull))  & 0xf00f00f00f00f00full;
            m = (m ^ (m >> 8ull))  & 0x00ff0000ff0000ffull;
            m = (m ^ (m >> 16ull)) & 0x00ff00000000ffffull;
            m = (m ^ (m >> 32ull)) & 0x00000000001fffffull;
            return (uint32_t)m;
        };

        x = compact(index);
        y = compact(index >> 1ull);
        z = compact(index >> 2ull);
    }

};



template <typename IndexScheme>
inline float xyz_to_index(const SL_ColorRGBf& xyz) noexcept
{
    constexpr unsigned w = 255;
    constexpr unsigned h = 255;
    constexpr unsigned d = 255;

    constexpr float wf = 255.f;
    constexpr float hf = 255.f;
    constexpr float df = 255.f;

    const unsigned a = math::clamp<unsigned>((unsigned)(xyz[0] * wf), 0, w);
    const unsigned b = math::clamp<unsigned>((unsigned)(xyz[1] * hf), 0, h);
    const unsigned c = math::clamp<unsigned>((unsigned)(xyz[2] * df), 0, d);

    union
    {
        uint32_t i;
        float f;
    } ret;

    ret.i = IndexScheme::encode_index3d(a, b, c);

    // encode the bits needed for normalization between (0.0, 1.0)
    ret.i |= 0x3F000000u;

    //std::cout << "INDEX IN: " << ret.i << std::endl;

    return ret.f;
}



template <typename IndexScheme>
inline SL_ColorRGBf index_to_xyz(float index) noexcept
{
    constexpr float w = 255.f;
    constexpr float h = 255.f;
    constexpr float d = 255.f;

    union
    {
        float f;
        uint32_t i;
    } vals{index};

    //std::cout << "INDEX OUT: " << vals.i << std::endl;

    // decode the decimal bits of the float
    uint32_t x, y, z;
    vals.i &= 0x00FFFFFF;
    IndexScheme::decode_index3d(vals.i, x, y, z);

    SL_ColorRGBf ret = SL_ColorRGBf{(float)x, (float)y, (float)z} / math::vec3{w, h, d};
    return math::clamp(ret, SL_ColorRGBf{0.f}, SL_ColorRGBf{1.f});
}



inline math::vec2 octahedral_norm_encode(const math::vec3& n) noexcept
{
    math::vec3&& m = n / (math::abs(n[0]) + math::abs(n[1]) + math::abs(n[2]));
    m[0] += m[2] >= 0.f ? 0.f : 1.f;
    m[1] += m[2] >= 0.f ? 0.f : 1.f;
    return math::vec2_cast(m);// * num_t{0.5} + num_t{0.5};
}



inline math::vec3 octahedral_norm_decode(const math::vec2& n) noexcept
{
    math::vec2 f = n;// * num_t{2.0} - num_t{1.0};
    math::vec3&& m = {f[0], f[1], 1.f - math::abs(f[0]) - math::abs(f[1])};
    float t = math::saturate(-m[2]);
    m[0] -= std::copysign(t, m[0]);
    m[1] -= std::copysign(t, m[1]);
    return math::normalize(m);
}



inline math::vec2 octahedral_norm_encode2(const math::vec3& n) noexcept
{
    math::vec3&& m = n / (math::abs(n[0]) + math::abs(n[1]) + math::abs(n[2]));
    return math::vec2{m[0] + m[1], m[0] - m[1]};
}



inline math::vec3 octahedral_norm_decode2(const math::vec2& n) noexcept
{
    const math::vec2 f{n[0] + n[1], n[0] - n[1]};
    return math::normalize(math::vec3{f[0], f[1], 2.f - math::abs(f[0]) - math::abs(f[1])});
}



inline math::vec2 hemimax_norm_encode(const math::vec3& n) noexcept
{
    const float d = math::max(math::abs(n[0]), math::abs(n[1]));
    const float sz = math::abs(n[2]) + d;   //could drop the abs(z)
    return math::vec2_cast(n) / sz;
}



inline math::vec3 hemimax_norm_decode(const math::vec2& n) noexcept
{
    const float z = 1.f - math::max(math::abs(n[0]), math::abs(n[1]));
    return math::normalize(math::vec3_cast(n, z));
}



template <typename IndexScheme>
inline SL_Plane colors_to_plane(const SL_ColorRGBf& a, const SL_ColorRGBf& b, const SL_ColorRGBf& c) noexcept
{
    const float index0 = xyz_to_index<IndexScheme>(a);
    const float index1 = xyz_to_index<IndexScheme>(b);
    const float index2 = xyz_to_index<IndexScheme>(c);

    const math::vec3 x{index0, 0.f, 0.f};
    const math::vec3 y{0.f, index1, 0.f};
    const math::vec3 z{0.f, 0.f, index2};

    return sl_plane_from_points(x, y, z);
}



template <typename IndexScheme>
inline void plane_to_colors(const SL_Plane& p, SL_ColorRGBf& a, SL_ColorRGBf& b, SL_ColorRGBf& c) noexcept
{
    SL_ColorRGBf x;
    SL_ColorRGBf y;
    SL_ColorRGBf z;

    sl_plane_intersect_line(p, math::vec3{0.f, 0.f, 0.f}, math::vec3{1.f, 0.f, 0.f}, x);
    sl_plane_intersect_line(p, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f}, y);
    sl_plane_intersect_line(p, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 0.f, 1.f}, z);

    a = index_to_xyz<IndexScheme>(x[0]);
    b = index_to_xyz<IndexScheme>(y[1]);
    c = index_to_xyz<IndexScheme>(z[2]);
}



struct Rgb9e5Compression
{
    union Rgb9e5Vec4
    {
        math::vec4 f;
        sl_rgb9e5f i[4];
    };

    static inline math::vec4 encode(const SL_ColorRGB8& a0, const SL_ColorRGB8& b0, const SL_ColorRGB8& c0) noexcept
    {
        SL_ColorRGBf a = color_cast<float, uint8_t>(a0);
        SL_ColorRGBf b = color_cast<float, uint8_t>(b0);
        SL_ColorRGBf c = color_cast<float, uint8_t>(c0);

        Rgb9e5Vec4 ret;
        ret.i[0] = float3_to_rgb9e5(a);
        ret.i[1] = float3_to_rgb9e5(b);
        ret.i[2] = float3_to_rgb9e5(c);
        ret.i[3].raw = 0;

        std::cout << "IN: Af: " << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
        std::cout << "IN: Bf: " << b[0] << ", " << b[1] << ", " << b[2] << std::endl;
        std::cout << "IN: Cf: " << c[0] << ", " << c[1] << ", " << c[2] << std::endl;
        std::cout << "IN: P: " << ret.i[0].raw << ", " << ret.i[1].raw << ", " << ret.i[2].raw << ", " << ret.i[3].raw << std::endl;

        return ret.f;
    }

    static inline void decode(const math::vec4& plane, SL_ColorRGB8& a0, SL_ColorRGB8& b0, SL_ColorRGB8& c0) noexcept
    {
        Rgb9e5Vec4 ret;
        ret.f = plane;

        const SL_ColorRGBf&& a = rgb9e5_to_float3(ret.i[0]);
        const SL_ColorRGBf&& b = rgb9e5_to_float3(ret.i[1]);
        const SL_ColorRGBf&& c = rgb9e5_to_float3(ret.i[2]);

        std::cout << "OUT: Af: " << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
        std::cout << "OUT: Bf: " << b[0] << ", " << b[1] << ", " << b[2] << std::endl;
        std::cout << "OUT: Cf: " << c[0] << ", " << c[1] << ", " << c[2] << std::endl;
        std::cout << "OUT: P: " << ret.i[0].raw << ", " << ret.i[1].raw << ", " << ret.i[2].raw << ", " << ret.i[3].raw << std::endl;

        a0 = color_cast<uint8_t, float>(a);
        b0 = color_cast<uint8_t, float>(b);
        c0 = color_cast<uint8_t, float>(c);
    }
};



// OpenVDB quantization
int16_t float_compress16(float f) noexcept
{
    //return (int16_t)std::round(f * (float)(1 << 16) + 0.5f);
    return (int16_t)std::round((f * 0.5f + 0.5f) * (float)(1 << 16));
}

float float_decompress16(int16_t i) noexcept
{
    //return (float)i / (float)(1 << 16);
    return ((float)i / (float)(1 << 16)) * 2.f - 1.f;
}



template <typename IndexScheme>
struct PlaneCompression
{
    static inline math::vec4 encode(const SL_ColorRGB8& a0, const SL_ColorRGB8& b0, const SL_ColorRGB8& c0) noexcept
    {
        SL_ColorRGBf a = color_cast<float, uint8_t>(a0);
        SL_ColorRGBf b = color_cast<float, uint8_t>(b0);
        SL_ColorRGBf c = color_cast<float, uint8_t>(c0);
        const SL_Plane&& plane = colors_to_plane<IndexScheme>(a, b, c);

        std::cout << "IN: Af: " << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
        std::cout << "IN: Bf: " << b[0] << ", " << b[1] << ", " << b[2] << std::endl;
        std::cout << "IN: Cf: " << c[0] << ", " << c[1] << ", " << c[2] << std::endl;
        std::cout << "IN: P:  " << plane[0] << ", " << plane[1] << ", " << plane[2] << ", " << plane[3] << std::endl;

        return plane;
    }

    static inline void decode(const math::vec4& plane, SL_ColorRGB8& a0, SL_ColorRGB8& b0, SL_ColorRGB8& c0) noexcept
    {
        SL_ColorRGBf a, b, c;

        const math::vec4& p = plane;

        //const math::vec4&& p = (math::vec4)((math::vec4h)plane);

        //const rgb9e5&& rgb9 = float3_to_rgb9e5(math::vec3_cast(plane));
        //const math::vec4&& p = math::vec4_cast(rgb9e5_to_float3(rgb9), plane[3]);

        //SL_PackedVertex_2_10_10_10&& rgb10 = SL_PackedVertex_2_10_10_10{math::vec3_cast(plane)};
        //const math::vec4&& p = math::vec4_cast((math::vec3)rgb10, plane[3]);

        /*
        math::vec4_t<int16_t> pi{
            float_compress16(plane[0]),
            float_compress16(plane[1]),
            float_compress16(plane[2]),
            float_compress16(plane[3])
        };

        math::vec4 p{
            float_decompress16(pi[0]),
            float_decompress16(pi[1]),
            float_decompress16(pi[2]),
            float_decompress16(pi[3])
        };
        */

        plane_to_colors<IndexScheme>(p, a, b, c);

        std::cout << "OUT: Af: " << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
        std::cout << "OUT: Bf: " << b[0] << ", " << b[1] << ", " << b[2] << std::endl;
        std::cout << "OUT: Cf: " << c[0] << ", " << c[1] << ", " << c[2] << std::endl;
        std::cout << "OUT: P:  " << p[0] << ", " << p[1] << ", " << p[2] << ", " << p[3] << std::endl;

        a0 = color_cast<uint8_t, float>(a);
        b0 = color_cast<uint8_t, float>(b);
        c0 = color_cast<uint8_t, float>(c);
    }
};

typedef PlaneCompression<LinearIndexScheme> PlaneCompressionLinear;
typedef PlaneCompression<Uint24IndexScheme> PlaneCompressionUint24;
typedef PlaneCompression<MortonIndexScheme> PlaneCompressionMorton;



template <typename Compression>
void test_compression(const char* testName) noexcept
{
    std::cout << "----------------------------------------" << std::endl;
    std::cout << testName << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    const SL_ColorRGB8 a0{13, 77, 92};
    const SL_ColorRGB8 b0{42, 255, 0};
    const SL_ColorRGB8 c0{168, 39, 254};

    std::cout << "Ai: " << (unsigned)a0[0] << ", " << (unsigned)a0[1] << ", " << (unsigned)a0[2] << std::endl;
    std::cout << "Bi: " << (unsigned)b0[0] << ", " << (unsigned)b0[1] << ", " << (unsigned)b0[2] << std::endl;
    std::cout << "Ci: " << (unsigned)c0[0] << ", " << (unsigned)c0[1] << ", " << (unsigned)c0[2] << std::endl;

    const math::vec4&& plane = Compression::encode(a0, b0, c0);
    SL_ColorRGB8 a1, b1, c1;
    Compression::decode(plane, a1, b1, c1);

    std::cout << "Ai: " << (unsigned)a1[0] << ", " << (unsigned)a1[1] << ", " << (unsigned)a1[2] << std::endl;
    std::cout << "Bi: " << (unsigned)b1[0] << ", " << (unsigned)b1[1] << ", " << (unsigned)b1[2] << std::endl;
    std::cout << "Ci: " << (unsigned)c1[0] << ", " << (unsigned)c1[1] << ", " << (unsigned)c1[2] << std::endl;
    std::cout << "PSNR: " << calc_psnr(255.f, (math::vec3)a0, (math::vec3)b0, (math::vec3)c0, (math::vec3)a1, (math::vec3)b1, (math::vec3)c1) << std::endl;

    std::cout << std::endl;
}



template <typename IndexScheme>
void test_index_scheme(const char* testName) noexcept
{
    std::cout << "----------------------------------------" << std::endl;
    std::cout << testName << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    {
        const math::vec3 rgbf = SL_ColorRGBf{0.165053f, 0.301649f, 0.756863f};
        std::cout << "Unencoded (RGBf): " << rgbf[0] << ", " << rgbf[1] << ", " << rgbf[2] << std::endl;

        const float i = xyz_to_index<IndexScheme>(rgbf);
        std::cout << "Encoded (RGBf): " << i << std::endl;

        const SL_ColorRGBf d = index_to_xyz<IndexScheme>(i);
        std::cout << "Decoded (RGBf): " << d[0] << ", " << d[1] << ", " << d[2] << std::endl;
        std::cout << "PSNR: " << calc_psnr(1.f, rgbf[0], rgbf[1], rgbf[2], d[0], d[1], d[2]) << std::endl;
        std::cout << std::endl;
    }

    {
        constexpr uint32_t x = 5;
        constexpr uint32_t y = 42;
        constexpr uint32_t z = 255;
        std::cout << "Unencoded Index (RGB8): " << x << ", " << y << ", " << z << std::endl;

        uint32_t index = IndexScheme::encode_index3d(x, y, z);
        std::cout << "Encoded Index (RGB8): " << index << std::endl;

        uint32_t x1;
        uint32_t y1;
        uint32_t z1;
        IndexScheme::decode_index3d(index, x1, y1, z1);
        std::cout << "Decoded Index (RGB8): " << x << ", " << y << ", " << z << std::endl;
        std::cout << "PSNR: " << calc_psnr(255.f, (float)x, (float)y, (float)z, (float)x1, (float)y1, (float)z1) << std::endl;
    }

    std::cout << std::endl;
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

    sl_rgb9e5f rgb9 = float3_to_rgb9e5(rgbf);
    std::cout << "RGB8:   " << (unsigned)rgb8[0] << ", " << (unsigned)rgb8[1] << ", " << (unsigned)rgb8[2] << std::endl;
    std::cout << "RGBf:   " << rgbf[0] << ", " << rgbf[1] << ", " << rgbf[2] << std::endl;
    std::cout << "RGB9e5: " << rgb9.field.r << ", " << rgb9.field.g << ", " << rgb9.field.b << std::endl;

    rgbf = rgb9e5_to_float3(rgb9);
    rgb8 = color_cast<uint8_t, float>(rgbf);

    std::cout << "RGB8:   " << (unsigned)rgb8[0] << ", " << (unsigned)rgb8[1] << ", " << (unsigned)rgb8[2] << std::endl;
    std::cout << "RGBf:   " << rgbf[0] << ", " << rgbf[1] << ", " << rgbf[2] << std::endl;
    std::cout << std::endl;

    test_index_scheme<LinearIndexScheme>("Linear Indexing");
    test_index_scheme<Uint24IndexScheme>("Uint24 Indexing");
    test_index_scheme<MortonIndexScheme>("Morton Indexing");

    test_compression<Rgb9e5Compression>("Rgb9e5 Compression");
    test_compression<PlaneCompressionLinear>("Plane Compression (linear encoding)");
    test_compression<PlaneCompressionUint24>("Plane Compression (uint24 encoding)");
    test_compression<PlaneCompressionMorton>("Plane Compression (morton encoding)");

    {
        math::vec3&& planeNorm = math::normalize(rgbf);
        std::cout << "Plane Norm: " << planeNorm[0] << ", " << planeNorm[1] << ", " << planeNorm[2] << std::endl;
        math::vec2 octahedral = octahedral_norm_encode(planeNorm);
        OctNormFp fp = {math::fixed_cast<OctNormFp::FpType, float>(octahedral[0]), math::fixed_cast<OctNormFp::FpType, float>(octahedral[1])};
        octahedral[0] = math::float_cast<float>(fp.x);
        octahedral[1] = math::float_cast<float>(fp.y);
        planeNorm = octahedral_norm_decode(octahedral);
        std::cout << "Oct-Decoded: " << planeNorm[0] << ", " << planeNorm[1] << ", " << planeNorm[2] << std::endl;
        std::cout << std::endl;
    }

    {
        math::vec3&& planeNorm = math::normalize(rgbf);
        std::cout << "Plane Norm: " << planeNorm[0] << ", " << planeNorm[1] << ", " << planeNorm[2] << std::endl;
        math::vec2 octahedral = octahedral_norm_encode2(planeNorm);
        OctNormFp fp = {math::fixed_cast<OctNormFp::FpType, float>(octahedral[0]), math::fixed_cast<OctNormFp::FpType, float>(octahedral[1])};
        octahedral[0] = math::float_cast<float>(fp.x);
        octahedral[1] = math::float_cast<float>(fp.y);
        planeNorm = octahedral_norm_decode2(octahedral);
        std::cout << "Oct-Decoded: " << planeNorm[0] << ", " << planeNorm[1] << ", " << planeNorm[2] << std::endl;
        std::cout << std::endl;
    }

    {
        math::vec3&& planeNorm = math::normalize(rgbf);
        std::cout << "Plane Norm: " << planeNorm[0] << ", " << planeNorm[1] << ", " << planeNorm[2] << std::endl;
        math::vec2 octahedral = hemimax_norm_encode(planeNorm);
        OctNormFp fp = {math::fixed_cast<OctNormFp::FpType, float>(octahedral[0]), math::fixed_cast<OctNormFp::FpType, float>(octahedral[1])};
        octahedral[0] = math::float_cast<float>(fp.x);
        octahedral[1] = math::float_cast<float>(fp.y);
        planeNorm = hemimax_norm_decode(octahedral);
        std::cout << "Oct-Decoded: " << planeNorm[0] << ", " << planeNorm[1] << ", " << planeNorm[2] << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

