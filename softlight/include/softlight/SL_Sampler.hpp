
#ifndef SL_SAMPLER_HPP
#define SL_SAMPLER_HPP

#include "lightsky/setup/Types.h"

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/fixed.h"

#include "softlight/SL_Texture.hpp"



enum class SL_SamplerWrap : uint8_t
{
    REPEAT,
    BORDER,
    CLAMP,

    DEFAULT = REPEAT
};



struct SL_WrapModeClampEdge;
struct SL_WrapModeClampBorder;
struct SL_WrapModeRepeat;

// Pseudo enum class
namespace SL_WrapMode
{
    typedef SL_WrapModeClampEdge   EDGE;
    typedef SL_WrapModeClampBorder BORDER;
    typedef SL_WrapModeRepeat      REPEAT;

    template <typename data_t>
    struct SL_IsWrapModeBorder : public ls::setup::FalseType<data_t>
    {
    };

    template <>
    struct SL_IsWrapModeBorder<SL_WrapModeClampBorder> : public ls::setup::TrueType<SL_WrapModeClampBorder>
    {
    };
}



struct SL_WrapModeClampEdge
{
    constexpr SL_WrapModeClampEdge() = default;

    constexpr LS_INLINE SL_Texture::fixed_type operator()(SL_Texture::fixed_type uvw) const noexcept
    {
        return ls::math::clamp<SL_Texture::fixed_type>(uvw, SL_Texture::fixed_type{0u}, ls::math::fixed_cast<SL_Texture::fixed_type>(1u));
    }

    constexpr LS_INLINE int operator()(int uvw, int maxVal) const noexcept
    {
        return ls::math::clamp<int>(uvw, 0, maxVal);
    }

    inline LS_INLINE float operator()(float uvw) const noexcept
    {
        return ls::math::clamp(uvw, 0.f, 1.f);
    }
};



struct SL_WrapModeClampBorder
{
    constexpr SL_WrapModeClampBorder() = default;

    constexpr LS_INLINE SL_Texture::fixed_type operator()(SL_Texture::fixed_type uvw) const noexcept
    {
        return (uvw >= ls::math::fixed_cast<SL_Texture::fixed_type>(0u) && uvw < ls::math::fixed_cast<SL_Texture::fixed_type>(1u))
            ? uvw
            : ls::math::fixed_cast<SL_Texture::fixed_type>(-1u);
    }

    constexpr LS_INLINE int operator()(int uvw, int maxVal) const noexcept
    {
        return (uvw < maxVal && uvw >= 0)
            ? uvw
            : -1;
    }

    constexpr LS_INLINE float operator()(float uvw) const noexcept
    {
        return (uvw >= 0.f && uvw < 1.f) ? uvw : -1.f;
    }
};



struct SL_WrapModeRepeat
{
    constexpr SL_WrapModeRepeat() = default;

    constexpr LS_INLINE SL_Texture::fixed_type operator()(SL_Texture::fixed_type uvw) const noexcept
    {
        return ls::math::clamp<SL_Texture::fixed_type>(uvw, SL_Texture::fixed_type{0u}, ls::math::fixed_cast<SL_Texture::fixed_type>(1u));
    }

    constexpr LS_INLINE int operator()(int uvw, int maxVal) const noexcept
    {
        return ls::math::clamp<int>(uvw, 0, maxVal);
    }

    inline LS_INLINE float operator()(float uvw) const noexcept
    {
        return (uvw < 0.f ? 1.f : 0.f) + ls::math::fmod_1(uvw);
    }
};



template <typename color_type, class WrapMode, SL_TexelOrder order = SL_TexelOrder::ORDERED>
inline LS_INLINE color_type sl_sample_nearest(const SL_Texture& tex, float x, float y) noexcept
{
    if (SL_WrapMode::SL_IsWrapModeBorder<WrapMode>::value && (x < 0.f || x >= 1.f || y < 0.f || y >= 1.f))
    {
        return color_type{0};
    }

    constexpr WrapMode wrapMode;

    const float wx = wrapMode(x);
    const float wy = wrapMode(y);

    #if 1
        const uint32_t xi = (uint_fast32_t)((float)tex.width()  * wx);
        const uint32_t yi = (uint_fast32_t)((float)tex.height() * wy);
    #else
        typedef typename SL_Texture::fixed_type fixed_type;
        const fixed_type    xf = ls::math::fixed_cast<fixed_type, float>(wx);
        const fixed_type    yf = ls::math::fixed_cast<fixed_type, float>(wy);
        const uint_fast32_t xi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(tex.width()) * xf);
        const uint_fast32_t yi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(tex.height()) * yf);
    #endif

    const ptrdiff_t index = tex.map_coordinate<order>(xi, yi);
    return reinterpret_cast<const color_type*>(tex.data())[index];
}

template <typename color_type, class WrapMode, SL_TexelOrder order = SL_TexelOrder::ORDERED>
inline LS_INLINE color_type sl_sample_nearest(const SL_Texture& tex, float x, float y, float z) noexcept
{
    if (SL_WrapMode::SL_IsWrapModeBorder<WrapMode>::value && (x < 0.f || x >= 1.f || y < 0.f || y >= 1.f || z < 0.f || z >= 1.f))
    {
        return color_type{0};
    }

    constexpr WrapMode wrapMode;
    const float wx = wrapMode(x);
    const float wy = wrapMode(y);
    const float wz = wrapMode(z);

    #if 1
        const uint32_t xi = (uint_fast32_t)((float)tex.width()  * wx);
        const uint32_t yi = (uint_fast32_t)((float)tex.height() * wy);
        const uint32_t zi = (uint_fast32_t)ls::math::round((float)tex.depth() * wz);
    #else
        typedef typename SL_Texture::fixed_type fixed_type;
        const fixed_type    xf = ls::math::fixed_cast<fixed_type, float>(wx);
        const fixed_type    yf = ls::math::fixed_cast<fixed_type, float>(wy);
        const fixed_type    zf = ls::math::fixed_cast<fixed_type, float>(wz) + ls::math::fixed_cast<fixed_type, float>(0.1f);
        const uint_fast32_t xi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(tex.width()) * xf);
        const uint_fast32_t yi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(tex.height()) * yf);
        const uint_fast32_t zi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(tex.depth()) * zf);
    #endif

    const ptrdiff_t index = tex.map_coordinate<order>(xi, yi, zi);
    return reinterpret_cast<const color_type*>(tex.data())[index];
}



template <typename color_type, class WrapMode, SL_TexelOrder order = SL_TexelOrder::ORDERED>
inline LS_INLINE color_type sl_sample_bilinear(const SL_Texture& tex, float x, float y) noexcept
{
    if (SL_WrapMode::SL_IsWrapModeBorder<WrapMode>::value && (x < 0.f || x >= 1.f || y < 0.f || y >= 1.f))
    {
        return color_type{0};
    }

    constexpr WrapMode wrapMode;

    const float    xf      = wrapMode(x) * (float)tex.width();
    const float    yf      = wrapMode(y) * (float)tex.height();
    const uint16_t xi0     = (uint16_t)xf;
    const uint16_t yi0     = (uint16_t)yf;
    const uint16_t xi1     = ls::math::clamp<uint16_t>(xi0+1u, 0u, tex.width());
    const uint16_t yi1     = ls::math::clamp<uint16_t>(yi0+1u, 0u, tex.height());
    const float    dx      = xf - (float)xi0;
    const float    dy      = yf - (float)yi0;
    const float    omdx    = 1.f - dx;
    const float    omdy    = 1.f - dy;
    const auto&&   pixel0  = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi0, yi0));
    const auto&&   pixel1  = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi0, yi1));
    const auto&&   pixel2  = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi1, yi0));
    const auto&&   pixel3  = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi1, yi1));
    const auto&&   weight0 = pixel0 * omdx * omdy;
    const auto&&   weight1 = pixel1 * omdx * dy;
    const auto&&   weight2 = pixel2 * dx * omdy;
    const auto&&   weight3 = pixel3 * dx * dy;

    const auto&& ret = ls::math::sum(weight0, weight1, weight2, weight3);

    return color_cast<typename color_type::value_type, float>(ret);
}

template <typename color_type, class WrapMode, SL_TexelOrder order = SL_TexelOrder::ORDERED>
inline LS_INLINE color_type sl_sample_bilinear(const SL_Texture& tex, float x, float y, float z) noexcept
{
    if (SL_WrapMode::SL_IsWrapModeBorder<WrapMode>::value && (x < 0.f || x >= 1.f || y < 0.f || y >= 1.f || z < 0.f || z >= 1.f))
    {
        return color_type{0};
    }

    constexpr WrapMode wrapMode;

    const float    xf      = wrapMode(x) * (float)tex.width();
    const float    yf      = wrapMode(y) * (float)tex.height();
    const uint16_t zi      = (uint16_t)ls::math::round(wrapMode(z) * (float)tex.depth());
    const uint16_t xi0     = (uint16_t)xf;
    const uint16_t yi0     = (uint16_t)yf;
    const uint16_t xi1     = ls::math::clamp<uint16_t>(xi0+1u, 0u, tex.width());
    const uint16_t yi1     = ls::math::clamp<uint16_t>(yi0+1u, 0u, tex.height());
    const float    dx      = xf - (float)xi0;
    const float    dy      = yf - (float)yi0;
    const float    omdx    = 1.f - dx;
    const float    omdy    = 1.f - dy;
    const auto&&   pixel0  = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi0, yi0, zi));
    const auto&&   pixel1  = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi0, yi1, zi));
    const auto&&   pixel2  = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi1, yi0, zi));
    const auto&&   pixel3  = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi1, yi1, zi));
    const auto&&   weight0 = pixel0 * omdx * omdy;
    const auto&&   weight1 = pixel1 * omdx * dy;
    const auto&&   weight2 = pixel2 * dx * omdy;
    const auto&&   weight3 = pixel3 * dx * dy;

    const auto&& ret = ls::math::sum(weight0, weight1, weight2, weight3);

    return color_cast<typename color_type::value_type, float>(ret);
}



template <typename color_type, class WrapMode, SL_TexelOrder order = SL_TexelOrder::ORDERED>
inline LS_INLINE color_type sl_sample_trilinear(const SL_Texture& tex, float x, float y) noexcept
{
    if (SL_WrapMode::SL_IsWrapModeBorder<WrapMode>::value && (x < 0.f || x >= 1.f || y < 0.f || y >= 1.f))
    {
        return color_type{0};
    }

    constexpr WrapMode wrapMode;

    /*
       V000 (1 - x) (1 - y) (1 - z) +
       V100 x (1 - y) (1 - z) +
       V010 (1 - x) y (1 - z) +
       V001 (1 - x) (1 - y) z +
       V101 x (1 - y) z +
       V011 (1 - x) y z +
       V110 x y (1 - z) +
       V111 x y z
     */

    namespace math = ls::math;

    x = wrapMode(x) * (float)tex.width();
    const uint16_t xi = (uint16_t)x;
    const uint16_t si = (uint16_t)math::max(x-1.f, 0.f);

    y = wrapMode(y) * (float)tex.height();
    const uint16_t yi = (uint16_t)y;
    const uint16_t ti = (uint16_t)math::max(y-1.f, 0.f);

    const auto&& c000 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(si, ti));
    const auto&& c100 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi, ti));
    const auto&& c010 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(si, yi));
    const auto&& c110 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi, yi));

    // floating-point math can be used for calculating the texel weights
    const float xf = x - math::floor(x);
    const float yf = y - math::floor(y);
    const float xd = 1.f - xf;
    const float yd = 1.f - yf;

    const auto&& weight000 = c000 * xd*yd;
    const auto&& weight100 = c100 * xf*yd;
    const auto&& weight010 = c010 * xd*yf;
    const auto&& weight110 = c110 * xf*yf;

    const auto&& ret = math::sum(weight000, weight100, weight010, weight110);

    return color_cast<typename color_type::value_type, float>(ret);
}

template <typename color_type, class WrapMode, SL_TexelOrder order = SL_TexelOrder::ORDERED>
inline LS_INLINE color_type sl_sample_trilinear(const SL_Texture& tex, float x, float y, float z) noexcept
{
    if (SL_WrapMode::SL_IsWrapModeBorder<WrapMode>::value && (x < 0.f || x >= 1.f || y < 0.f || y >= 1.f || z < 0.f || z >= 1.f))
    {
        return color_type{0};
    }

    constexpr WrapMode wrapMode;

    /*
       V000 (1 - x) (1 - y) (1 - z) +
       V100 x (1 - y) (1 - z) +
       V010 (1 - x) y (1 - z) +
       V001 (1 - x) (1 - y) z +
       V101 x (1 - y) z +
       V011 (1 - x) y z +
       V110 x y (1 - z) +
       V111 x y z
     */

    namespace math = ls::math;

    x = wrapMode(x) * (float)tex.width();
    const uint16_t xi = (uint16_t)x;
    const uint16_t si = math::max(x-1.f, 0.f);

    y = wrapMode(y) * (float)tex.height();
    const uint16_t yi = (uint16_t)y;
    const uint16_t ti = math::max(y-1.f, 0.f);

    z = wrapMode(z) * (float)tex.depth();
    const uint16_t zi = (uint16_t)z;
    const uint16_t ri = math::max(z-1.f, 0.f);

    const auto&& c000 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(si, ti, ri));
    const auto&& c100 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi, ti, ri));
    const auto&& c010 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(si, yi, ri));
    const auto&& c001 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(si, ti, zi));
    const auto&& c101 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi, ti, zi));
    const auto&& c011 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(si, yi, zi));
    const auto&& c110 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi, yi, ri));
    const auto&& c111 = color_cast<float, typename color_type::value_type>(tex.texel<color_type, order>(xi, yi, zi));

    const float xf = x - math::floor(x);
    const float yf = y - math::floor(y);
    const float zf = z - math::floor(z);
    const float xd = 1.f - xf;
    const float yd = 1.f - yf;
    const float zd = 1.f - zf;

    const auto&& weight000 = c000 * xd*yd*zd;
    const auto&& weight100 = c100 * xf*yd*zd;
    const auto&& weight010 = c010 * xd*yf*zd;
    const auto&& weight001 = c001 * xd*yd*zf;
    const auto&& weight101 = c101 * xf*yd*zf;
    const auto&& weight011 = c011 * xd*yf*zf;
    const auto&& weight110 = c110 * xf*yf*zd;
    const auto&& weight111 = c111 * xf*yf*zf;

    const auto&& ret = math::sum(
        weight000,
        weight100,
        weight010,
        weight001,
        weight101,
        weight011,
        weight110,
        weight111
    );

    return color_cast<typename color_type::value_type, float>(ret);
}



#endif /* SL_SAMPLER_HPP */
