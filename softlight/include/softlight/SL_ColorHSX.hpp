
#ifndef SL_COLOR_HSX_HPP
#define SL_COLOR_HSX_HPP

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec_utils.h"

#include "softlight/SL_Color.hpp"



/*-----------------------------------------------------------------------------
 * Extended Color Models
-----------------------------------------------------------------------------*/
/**
 * @brief Generic HSV Color Structure
 */
template <typename color_t>
struct alignas(sizeof(color_t)) SL_ColorTypeHSV
{
    static_assert(ls::setup::IsFloat<color_t>::value, "HSV can only be represented by floating-point numbers.");

    typedef color_t value_type;
    static constexpr unsigned num_components() noexcept { return 3; }

    color_t h; // 0-1
    color_t s; // 0-1
    color_t v; // 0-1
};

typedef SL_ColorTypeHSV<ls::math::half> SL_ColorTypeHSVh;
typedef SL_ColorTypeHSV<float> SL_ColorTypeHSVf;
typedef SL_ColorTypeHSV<double> SL_ColorTypeHSVd;



/**
 * @brief Generic HSVA Color Structure
 */
template <typename color_t>
struct alignas(sizeof(color_t)) SL_ColorTypeHSVA
{
    static_assert(ls::setup::IsFloat<color_t>::value, "HSV can only be represented by floating-point numbers.");

    typedef color_t value_type;
    static constexpr unsigned num_components() noexcept { return 4; }

    color_t h; // 0-1
    color_t s; // 0-1
    color_t v; // 0-1
    color_t a; // 0-1
};

typedef SL_ColorTypeHSVA<ls::math::half> SL_ColorTypeHSVAh;
typedef SL_ColorTypeHSVA<float> SL_ColorTypeHSVAf;
typedef SL_ColorTypeHSVA<double> SL_ColorTypeHSVAd;



/**
 * @brief Generic HSL Color Structure
 */
template <typename color_t>
struct alignas(sizeof(color_t)) SL_ColorTypeHSL
{
    static_assert(ls::setup::IsFloat<color_t>::value, "HSL can only be represented by floating-point numbers.");

    typedef color_t value_type;
    static constexpr unsigned num_components() noexcept { return 3; }

    color_t h; // 0-1
    color_t s; // 0-1
    color_t l; // 0-1
};

typedef SL_ColorTypeHSL<ls::math::half> SL_ColorTypeHSLh;
typedef SL_ColorTypeHSL<float> SL_ColorTypeHSLf;
typedef SL_ColorTypeHSL<double> SL_ColorTypeHSLd;



/**
 * @brief Generic HSLA Color Structure
 */
template <typename color_t>
struct alignas(sizeof(color_t)) SL_ColorTypeHSLA
{
    static_assert(ls::setup::IsFloat<color_t>::value, "HSL can only be represented by floating-point numbers.");

    typedef color_t value_type;
    static constexpr unsigned num_components() noexcept { return 4; }

    color_t h; // 0-1
    color_t s; // 0-1
    color_t l; // 0-1
    color_t a; // 0-1
};

typedef SL_ColorTypeHSLA<ls::math::half> SL_ColorTypeHSLAh;
typedef SL_ColorTypeHSLA<float> SL_ColorTypeHSLAf;
typedef SL_ColorTypeHSLA<double> SL_ColorTypeHSLAd;



/*-----------------------------------------------------------------------------
 * Internal limits of color HSV ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <typename color_t>
struct SL_ColorLimits<color_t, SL_ColorTypeHSV>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorTypeHSV<color_t> min() noexcept
    {
        return SL_ColorTypeHSV<color_t>{
            (color_t)0.0, (color_t)0.0, (color_t)0.0
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorTypeHSV<color_t> max() noexcept
    {
        return SL_ColorTypeHSV<color_t>{
            (color_t)1.0, (color_t)1.0, (color_t)1.0
        };
    }
};



template <>
struct SL_ColorLimits<ls::math::half, SL_ColorTypeHSV>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorTypeHSV<ls::math::half> min() noexcept
    {
        return SL_ColorTypeHSV<ls::math::half>{
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u}
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorTypeHSV<ls::math::half> max() noexcept
    {
        return SL_ColorTypeHSV<ls::math::half>{
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u}
        };
    }
};



/*-----------------------------------------------------------------------------
 * Internal limits of color HSVA ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <typename color_t>
struct SL_ColorLimits<color_t, SL_ColorTypeHSVA>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorTypeHSVA<color_t> min() noexcept
    {
        return SL_ColorTypeHSVA<color_t>{
            (color_t)0.0,
            (color_t)0.0,
            (color_t)0.0,
            (color_t)0.0
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorTypeHSVA<color_t> max() noexcept
    {
        return SL_ColorTypeHSVA<color_t>{
            (color_t)1.0,
            (color_t)1.0,
            (color_t)1.0,
            (color_t)1.0
        };
    }
};



template <>
struct SL_ColorLimits<ls::math::half, SL_ColorTypeHSVA>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorTypeHSVA<ls::math::half> min() noexcept
    {
        return SL_ColorTypeHSVA<ls::math::half>{
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u}
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorTypeHSVA<ls::math::half> max() noexcept
    {
        return SL_ColorTypeHSVA<ls::math::half>{
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u}
        };
    }
};



/*-----------------------------------------------------------------------------
 * Internal limits of color HSV ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <typename color_t>
struct SL_ColorLimits<color_t, SL_ColorTypeHSL>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr SL_ColorTypeHSL<color_t> min() noexcept
    {
        return SL_ColorTypeHSL<color_t>{
            (color_t)0.0, (color_t)0.0, (color_t)0.0
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 10.0.
    */
    static constexpr SL_ColorTypeHSL<color_t> max() noexcept
    {
        return SL_ColorTypeHSL<color_t>{
            (color_t)1.0, (color_t)1.0, (color_t)1.0
        };
    }
};



template <>
struct SL_ColorLimits<ls::math::half, SL_ColorTypeHSL>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr SL_ColorTypeHSL<ls::math::half> min() noexcept
    {
        return SL_ColorTypeHSL<ls::math::half>{
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u}
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 10.0.
    */
    static constexpr SL_ColorTypeHSL<ls::math::half> max() noexcept
    {
        return SL_ColorTypeHSL<ls::math::half>{
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u}
        };
    }
};



/*-----------------------------------------------------------------------------
 * Internal limits of color HSVA ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <typename color_t>
struct SL_ColorLimits<color_t, SL_ColorTypeHSLA>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr SL_ColorTypeHSLA<color_t> min() noexcept
    {
        return SL_ColorTypeHSLA<color_t>{
            (color_t)0.0,
            (color_t)0.0,
            (color_t)0.0,
            (color_t)0.0
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 10.0.
    */
    static constexpr SL_ColorTypeHSLA<color_t> max() noexcept
    {
        return SL_ColorTypeHSLA<color_t>{
            (color_t)1.0,
            (color_t)1.0,
            (color_t)1.0,
            (color_t)1.0
        };
    }
};



template <>
struct SL_ColorLimits<ls::math::half, SL_ColorTypeHSLA>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr SL_ColorTypeHSLA<ls::math::half> min() noexcept
    {
        return SL_ColorTypeHSLA<ls::math::half>{
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u},
            ls::math::half{0x00u, 0x00u}
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 10.0.
    */
    static constexpr SL_ColorTypeHSLA<ls::math::half> max() noexcept
    {
        return SL_ColorTypeHSLA<ls::math::half>{
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u},
            ls::math::half{0x3Cu, 0x00u}
        };
    }
};



/*-----------------------------------------------------------------------------
 * Color Casting Operations
-----------------------------------------------------------------------------*/
// Common function for extraction of RGB from hue
// Adapted from https://www.chilliant.com/rgb2hsv.html
template <typename color_t>
inline LS_INLINE ls::math::vec4_t<color_t> sl_hue_to_rgb(const color_t h) noexcept
{
    namespace math = ls::math;
    math::vec4_t<color_t> rgb{
        math::abs(math::fmsub(h, color_t{6}, color_t{3})) - color_t{1.0},
        color_t{2.0} - math::abs(math::fmsub(h, color_t{6.0}, color_t{2.0})),
        color_t{2.0} - math::abs(math::fmsub(h, color_t{6.0}, color_t{4.0})),
        color_t{0.0}
    };

    return math::saturate(rgb);
}



// Conversion from RGB to hue/chroma/value
template <typename color_t>
inline LS_INLINE ls::math::vec3_t<color_t> sl_rgb_to_hcv(const ls::math::vec3_t<color_t>& rgb)
{
    namespace math = ls::math;

    // Based on work by Sam Hocevar and Emil Persson
    const math::vec4_t<color_t>&& p = (rgb[1] < rgb[2])
        ? math::vec4_t<color_t>{rgb[2], rgb[1], color_t{-1.0}, color_t{ 2.0} / color_t{3.0}}
        : math::vec4_t<color_t>{rgb[1], rgb[2], color_t{ 0.0}, color_t{-1.0} / color_t{3.0}};

    const math::vec4_t<color_t>&& q = (rgb[0] < p[0])
        ? math::vec4_t<color_t>{p[0],   p[1], p[3], rgb[0]}
        : math::vec4_t<color_t>{rgb[0], p[1], p[2], p[0]  };

    const color_t c = q[0] - math::min(q[3], q[1]);
    const color_t h = math::abs((q[3] - q[1]) / (color_t{6.0} * c) + q[2]);

    return math::vec3_t<color_t>(h, c, q[0]);
}



/*--------------------------------------
 * Cast from HSV to RGB
--------------------------------------*/
template <typename color_t>
inline SL_ColorRGBType<color_t> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorTypeHSV<color_t>>::type& hsv) noexcept
{
    namespace math = ls::math;

    const math::vec4_t<color_t>&& rgb = sl_hue_to_rgb(math::fmod_1(hsv.h));
    const math::vec4_t<color_t>&& result = ((rgb - color_t{1.0}) * hsv.s + color_t{1.0}) * hsv.v;
    return math::vec3_cast(result);
}



/*--------------------------------------
 * Cast from HSVA to RGBA
--------------------------------------*/
template <typename color_t>
inline SL_ColorRGBAType<color_t> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorTypeHSVA<color_t>>::type& inC) noexcept
{
    SL_ColorTypeHSV<color_t> hsv{inC.h, inC.s, inC.l};
    const SL_ColorRGBType<color_t>&& outRGB = rgb_cast<color_t>(hsv);
    return ls::math::vec4_cast<color_t>(outRGB, inC.a);
}



/*--------------------------------------
 * Cast from HSL to RGB
--------------------------------------*/
template <typename color_t>
inline SL_ColorRGBType<color_t> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorTypeHSL<color_t>>::type& hsl) noexcept
{
    namespace math = ls::math;

    const math::vec4_t<color_t>&& rgb = sl_hue_to_rgb(math::fmod_1(hsl.h));
    const color_t c = (color_t{1.0} - math::abs(color_t{2.0} * hsl.l - color_t{1.0})) * hsl.s;
    return math::vec3_cast((rgb - color_t{0.5}) * c + hsl.l);
}



/*--------------------------------------
 * Cast from HSLA to RGBA
--------------------------------------*/
template <typename color_t>
inline SL_ColorRGBAType<color_t> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorTypeHSLA<color_t>>::type& inC) noexcept
{
    SL_ColorTypeHSL<color_t> hsl{inC.h, inC.s, inC.l};
    const SL_ColorRGBType<color_t>&& outRGB = rgb_cast<color_t>(hsl);
    return ls::math::vec4_cast<color_t>(outRGB, inC.a);
}



/*--------------------------------------
 * RGB To HSV
--------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSV<color_t> hsv_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorRGBType<color_t>>::type& rgb) noexcept
{
    namespace math = ls::math;

    math::vec3_t<color_t>&& hcv = sl_rgb_to_hcv(rgb);
    const color_t s = hcv[1] / hcv[2];
    return SL_ColorTypeHSV<color_t>{hcv[0], s, hcv[2]};
}



/*--------------------------------------
 * Cast from RGBA to HSVA
--------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSVA<color_t> hsv_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorRGBAType<color_t>>::type& c) noexcept
{
    const SL_ColorTypeHSV<color_t>&& hsv = hsv_cast<color_t>(ls::math::vec3_cast<color_t>(c));
    return SL_ColorTypeHSVA<color_t>{hsv.h, hsv.s, hsv.v, c[3]};
}



/*--------------------------------------
 * HSL To HSV
--------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSV<color_t> hsv_cast(const SL_ColorTypeHSL<color_t>& c) noexcept
{
    color_t l = color_t{2} * c.l;
    color_t s = c.s * ((l <= color_t{1}) ? l : color_t{2} - l);

    return SL_ColorTypeHSV<color_t>{
        c.h,
        (color_t{2} * s) / (l + s),
        (l + s) / color_t{2}
    };
}



/*--------------------------------------
 * HSLA To HSVA
--------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSVA<color_t> hsv_cast(const SL_ColorTypeHSLA<color_t>& c) noexcept
{
    color_t l = color_t{2} * c.l;
    color_t s = c.s * ((l <= color_t{1}) ? l : color_t{2} - l);

    return SL_ColorTypeHSVA<color_t>{
        c.h,
        (color_t{2} * s) / (l + s),
        (l + s) / color_t{2},
        c.a
    };
}



/*-------------------------------------
 * RGB to HSL
-------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSL<color_t> hsl_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorRGBType<color_t>>::type& rgb) noexcept
{
    namespace math = ls::math;

    math::vec3_t<color_t>&& hcv = sl_rgb_to_hcv(rgb);
    const color_t l = hcv[2] - hcv[1] * color_t{0.5};
    const color_t s = hcv[1] / (color_t{1.0} - math::abs(math::fmsub(l, color_t{2.0}, color_t{1.0})));
    return SL_ColorTypeHSL<color_t>{hcv[0], s, l};
}



/*--------------------------------------
 * Cast from RGBA to HSLA
--------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSLA<color_t> hsl_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorRGBAType<color_t>>::type& c) noexcept
{
    const SL_ColorTypeHSL<color_t>&& hsl = hsl_cast<color_t>(ls::math::vec3_cast<color_t>(c));
    return SL_ColorTypeHSLA<color_t>{hsl.h, hsl.s, hsl.l, c[3]};
}



/*-------------------------------------
 * HSV to HSL
-------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSL<color_t> hsl_cast(const SL_ColorTypeHSV<color_t>& c) noexcept
{
    color_t s = c.s * c.v;
    color_t l = (color_t{2} - c.s) * c.v;

    return SL_ColorTypeHSL<color_t>{
        c.h,
        s / ((l <= color_t{1}) ? l : color_t{2} - l),
        l / color_t{2}
    };
}



/*-------------------------------------
 * HSVA to HSLA
-------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSLA<color_t> hsl_cast(const SL_ColorTypeHSVA<color_t>& c) noexcept
{
    color_t s = c.s * c.v;
    color_t l = (color_t{2} - c.s) * c.v;

    return SL_ColorTypeHSLA<color_t>{
        c.h,
        s / ((l <= color_t{1}) ? l : color_t{2} - l),
        l / color_t{2},
        c.a
    };
}



#endif /* SL_COLOR_HSX_HPP */

