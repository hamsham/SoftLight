
#ifndef SL_COLOR_HSX_HPP
#define SL_COLOR_HSX_HPP

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

    color_t h; // should be between 0-360
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

    color_t h; // should be between 0-360
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

    color_t h; // should be between 0-360
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

    color_t h; // should be between 0-360
    color_t s; // 0-1
    color_t l; // 0-1
    color_t a; // 0-1
};

typedef SL_ColorTypeHSLA<ls::math::half> SL_ColorTypeHSLAh;
typedef SL_ColorTypeHSLA<float> SL_ColorTypeHSLAf;
typedef SL_ColorTypeHSLA<double> SL_ColorTypeHSLAd;



/*-----------------------------------------------------------------------------
 * Color Casting Operations
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Cast from HSV to RGB
--------------------------------------*/
template <typename color_t>
SL_ColorRGBType<color_t> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorTypeHSV<color_t>>::type& inC) noexcept
{
    const color_t c = inC.v * inC.s;
    const color_t x = c * (color_t{1.f} - ls::math::abs(ls::math::fmod(inC.h / color_t{60.f}, color_t{2.f}) - color_t{1.f}));
    const color_t m = inC.v - c;

    color_t tempR;
    color_t tempG;
    color_t tempB;

    if (inC.h <= color_t{60.f})
    {
        tempR = c;
        tempG = x;
        tempB = color_t{0.f};
    }
    else if (inC.h <= color_t{120.f})
    {
        tempR = x;
        tempG = c;
        tempB = color_t{0.f};
    }
    else if (inC.h <= color_t{180.f})
    {
        tempR = color_t{0.f};
        tempG = c;
        tempB = x;
    }
    else if (inC.h <= color_t{240.f})
    {
        tempR = color_t{0.f};
        tempG = x;
        tempB = c;
    }
    else if (inC.h <= color_t{300.f})
    {
        tempR = x;
        tempG = color_t{0.f};
        tempB = c;
    }
    else
    {
        tempR = c;
        tempG = color_t{0.f};
        tempB = x;
    }

    tempR += m;
    tempG += m;
    tempB += m;

    constexpr color_t COLOR_MAX_VAL = SL_ColorLimits<color_t>::max();

    return SL_ColorRGBType <color_t> {
        static_cast<color_t>(tempR * COLOR_MAX_VAL),
        static_cast<color_t>(tempG * COLOR_MAX_VAL),
        static_cast<color_t>(tempB * COLOR_MAX_VAL)
    };
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
SL_ColorRGBType<color_t> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorTypeHSL<color_t>>::type& inC) noexcept
{
    const color_t c = inC.s * (color_t{1.f} - ls::math::abs(color_t{2.f} * inC.l - color_t{1.f}));
    const color_t x = c * (color_t{1.f} - ls::math::abs(ls::math::fmod(inC.h / color_t{60.f}, color_t{2.f}) - color_t{1.f}));
    const color_t m = inC.l - (c * color_t{0.5f});

    color_t tempR;
    color_t tempG;
    color_t tempB;

    if (inC.h <= color_t{60.f})
    {
        tempR = c;
        tempG = x;
        tempB = color_t{0.f};
    }
    else if (inC.h <= color_t{120.f})
    {
        tempR = x;
        tempG = c;
        tempB = color_t{0.f};
    }
    else if (inC.h <= color_t{180.f})
    {
        tempR = 0.f;
        tempG = c;
        tempB = x;
    }
    else if (inC.h <= color_t{240.f})
    {
        tempR = color_t{0.f};
        tempG = x;
        tempB = c;
    }
    else if (inC.h <= color_t{300.f})
    {
        tempR = x;
        tempG = color_t{0.f};
        tempB = c;
    }
    else
    {
        tempR = c;
        tempG = color_t{0.f};
        tempB = x;
    }

    tempR += m;
    tempG += m;
    tempB += m;

    constexpr color_t COLOR_MAX_VAL = SL_ColorLimits<color_t>::max();

    return SL_ColorRGBType<color_t>{
        static_cast<color_t>(tempR * COLOR_MAX_VAL),
        static_cast<color_t>(tempG * COLOR_MAX_VAL),
        static_cast<color_t>(tempB * COLOR_MAX_VAL)
    };
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
SL_ColorTypeHSV<color_t> hsv_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorRGBType<color_t>>::type& c) noexcept
{
    const color_t COLOR_EPSILON = color_t{1.0e-6f};
    const color_t normR = (color_t)(0.5f * (static_cast<float>(c[0]) + 1.f));
    const color_t normG = (color_t)(0.5f * (static_cast<float>(c[1]) + 1.f));
    const color_t normB = (color_t)(0.5f * (static_cast<float>(c[2]) + 1.f));

    // normalize the input values and calculate their deltas
    const color_t maxVal = ls::math::max(normR, ls::math::max(normG, normB));
    const color_t minVal = ls::math::min(normR, ls::math::min(normG, normB));
    const color_t delta = maxVal - minVal;

    // check if we are near 0 (min)
    if (ls::math::abs(maxVal) <= SL_ColorLimits<color_t>::min())
    {
        return SL_ColorTypeHSV<color_t>{
            static_cast<color_t>(-1.f),
            static_cast<color_t>(NAN), static_cast<color_t> (INFINITY)
        };
    }

    color_t hue = color_t{60.f};

    if (ls::math::abs(maxVal-normR) <= COLOR_EPSILON)
    {
        hue *= ls::math::fmod(normG - normB, color_t{6.f}) / delta;
    }
    else if (ls::math::abs(maxVal-normG) <= COLOR_EPSILON)
    {
        hue *= color_t{2.f} + ((normB - normR) / delta);
    }
    else
    {
        hue *= color_t{4.f} + ((normR - normG) / delta);
    }

    // This part of the conversion requires a data type with more than 2 bytes.
    // Some values may be valid, others may be truncated/undefined.
    if (hue < color_t{360.f})
    {
        hue = color_t{360.f} - ls::math::fmod(ls::math::abs(hue), color_t{360.f});
    }
    else
    {
        hue = ls::math::fmod(hue, color_t{360.f});
    }

    // result
    return SL_ColorTypeHSV<color_t>{hue, delta / maxVal, maxVal};
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
SL_ColorTypeHSL<color_t> hsl_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorRGBType<color_t>>::type& c) noexcept
{
    const color_t COLOR_EPSILON = color_t{1.0e-6f};
    const color_t normR = (color_t)(0.5f * (static_cast<float>(c[0]) + 1.f));
    const color_t normG = (color_t)(0.5f * (static_cast<float>(c[1]) + 1.f));
    const color_t normB = (color_t)(0.5f * (static_cast<float>(c[2]) + 1.f));

    // normalize the input values and calculate their deltas
    const color_t maxVal = ls::math::max(normR, ls::math::max(normG, normB));
    const color_t minVal = ls::math::min(normR, ls::math::min(normG, normB));
    const color_t delta = maxVal - minVal;

    // check if we are near 0
    if (ls::math::abs(maxVal) <= SL_ColorLimits<color_t>::min())
    {
        return SL_ColorTypeHSL<color_t>{0.f, 0.f, 0.f};
    }

    color_t hue = color_t{60.f};

    if (ls::math::abs(maxVal - normR) <= COLOR_EPSILON)
    {
        hue *= ls::math::fmod(normG - normB, color_t{6.f}) / delta;
    }
    else if (ls::math::abs(maxVal - normG) <= COLOR_EPSILON)
    {
        hue *= color_t{2.f} + ((normB - normR) / delta);
    }
    else
    {
        hue *= color_t{4.f} + ((normR - normG) / delta);
    }

    // This part of the conversion requires a data type with more than 2 bytes.
    // Some values may be valid, others may be truncated/undefined.
    hue = (hue < color_t{0.f}) ? (hue + color_t{360.f}) : hue;
    color_t lightness = color_t{0.5f} * (maxVal + minVal);
    color_t saturation = color_t{0.f};

    if (ls::math::abs(maxVal) > SL_ColorLimits<color_t>::min())
    {
        saturation = delta / (color_t{1.f} - ls::math::abs(color_t{2.f} * lightness - color_t{1.f}));
    }

    // result
    return SL_ColorTypeHSL<color_t>{hue, saturation, lightness};
}



/*--------------------------------------
 * Cast from RGBA to HSLA
--------------------------------------*/
template <typename color_t>
inline SL_ColorTypeHSLA<color_t> hsl_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorRGBAType<color_t>>::type& c) noexcept
{
    const SL_ColorTypeHSL<color_t>&& hsl = hsl_cast<color_t>(ls::math::vec3_cast<color_t>(c));
    return SL_ColorTypeHSLA<color_t>{hsl.h, hsl.s, hsl.v, c[3]};
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

