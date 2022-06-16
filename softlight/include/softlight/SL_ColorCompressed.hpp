
#ifndef SL_COLOR_COMPRESSED_HPP
#define SL_COLOR_COMPRESSED_HPP

#include <cstdint>
#include "softlight/SL_Color.hpp"



/*-----------------------------------------------------------------------------
 * RGB332 Types
-----------------------------------------------------------------------------*/
template <typename color_t>
struct SL_ColorRGB332Type;

/**
 * @brief RGB-332 Color Structure
 */
 template <>
struct alignas(alignof(uint8_t)) SL_ColorRGB332Type<uint8_t>
{
    typedef uint8_t value_type;
    static constexpr unsigned num_components() noexcept { return 3; }

    uint8_t r : 3;
    uint8_t g : 3;
    uint8_t b : 2;
};

static_assert(sizeof(SL_ColorRGB332Type<uint8_t>) == sizeof(uint8_t), "Compressed RGB332 is not 8 bytes.");



/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRGB332Type<uint8_t>  SL_ColorRGB332;



/*-----------------------------------------------------------------------------
 * Internal limits of color RGB332 ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <>
struct SL_ColorLimits<uint8_t, SL_ColorRGB332Type>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr SL_ColorRGB332Type<uint8_t> min() noexcept
    {
        return SL_ColorRGB332Type<uint8_t>{0, 0, 0};
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 10.0.
    */
    static constexpr SL_ColorRGB332Type<uint8_t> max() noexcept
    {
        return SL_ColorRGB332Type<uint8_t>{7, 7, 3};
    }
};



/*-----------------------------------------------------------------------------
 * RGB to RGB-332 Casting
-----------------------------------------------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB332>::value, SL_ColorRGB332>::type
rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBType<U>>::type& c)
{
    return SL_ColorRGB332Type<uint8_t>{
        (uint8_t)(c[0] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[0] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().r)),
        (uint8_t)(c[1] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[1] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().g)),
        (uint8_t)(c[2] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[2] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().b))
    };
}

template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB332>::value, SL_ColorRGB332>::type
rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBAType<U>>::type& c)
{
    return SL_ColorRGB332Type<uint8_t>{
        (uint8_t)(c[0] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[0] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().r)),
        (uint8_t)(c[1] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[1] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().g)),
        (uint8_t)(c[2] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[2] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().b))
    };
}



/*-------------------------------------
 * RGB (float) to 332
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB332>::value, SL_ColorRGB332>::type
rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBType<U>>::type& c)
{
    return SL_ColorRGB332Type<uint8_t>{
        (uint8_t)(c[0] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().r),
        (uint8_t)(c[1] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().g),
        (uint8_t)(c[2] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().b)
    };
}

template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB332>::value, SL_ColorRGB332>::type
rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBAType<U>>::type& c)
{
    return SL_ColorRGB332Type<uint8_t>{
        (uint8_t)(c[0] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().r),
        (uint8_t)(c[1] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().g),
        (uint8_t)(c[2] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().b)
    };
}



/*-----------------------------------------------------------------------------
 * RGB-332 to RGB Casting
-----------------------------------------------------------------------------*/
template <typename T, typename U>
constexpr SL_ColorRGBType<T> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGB332Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBType<T>{
        (T)(c.r * (SL_ColorLimits<T, SL_ColorRGBType>::max()[0] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().r)),
        (T)(c.g * (SL_ColorLimits<T, SL_ColorRGBType>::max()[1] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().g)),
        (T)(c.b * (SL_ColorLimits<T, SL_ColorRGBType>::max()[2] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().b))
    };
}

template <typename T, typename U>
constexpr SL_ColorRGBAType<T> rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGB332Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)(c.r * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[0] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().r)),
        (T)(c.g * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[1] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().g)),
        (T)(c.b * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[2] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().b)),
        SL_ColorLimits<T, SL_ColorRGBAType>::max()[3]
    };
}



/*-------------------------------------
 * 332 to RGB (float)
-------------------------------------*/
template <typename T, typename U>
constexpr SL_ColorRGBType<T> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGB332Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBType<T>{
        (T)c.r / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().r,
        (T)c.g / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().g,
        (T)c.b / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().b
    };
}

template <typename T, typename U>
constexpr SL_ColorRGBAType<T> rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGB332Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)c.r / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().r,
        (T)c.g / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().g,
        (T)c.b / (T)SL_ColorLimits<uint8_t, SL_ColorRGB332Type>::max().b,
        SL_ColorLimits<T, SL_ColorRGBAType>::max()[3]
    };
}



/*-----------------------------------------------------------------------------
 * RGB565 Types
-----------------------------------------------------------------------------*/
template <typename color_t>
struct SL_ColorRGB565Type;

/**
 * @brief RGB-565 Color Structure
 */
 template <>
struct alignas(alignof(uint16_t)) SL_ColorRGB565Type<uint8_t>
{
    typedef uint8_t value_type;
    static constexpr unsigned num_components() noexcept { return 3; }

    uint16_t r : 5;
    uint16_t g : 6;
    uint16_t b : 5;
};

static_assert(sizeof(SL_ColorRGB565Type<uint8_t>) == sizeof(uint16_t), "Compressed RGB565 is not 16 bytes.");



/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRGB565Type<uint8_t>  SL_ColorRGB565;



/*-----------------------------------------------------------------------------
 * Internal limits of color RGB565 ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <>
struct SL_ColorLimits<uint8_t, SL_ColorRGB565Type>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr SL_ColorRGB565Type<uint8_t> min() noexcept
    {
        return SL_ColorRGB565Type<uint8_t>{0, 0, 0};
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 10.0.
    */
    static constexpr SL_ColorRGB565Type<uint8_t> max() noexcept
    {
        return SL_ColorRGB565Type<uint8_t>{31, 63, 31};
    }
};



/*-----------------------------------------------------------------------------
 * RGB to RGB-565 Casting
-----------------------------------------------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB565>::value, SL_ColorRGB565>::type
rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBType<U>>::type& c)
{
    return SL_ColorRGB565Type<uint8_t>{
        (uint8_t)(c[0] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[0] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().r)),
        (uint8_t)(c[1] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[1] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().g)),
        (uint8_t)(c[2] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[2] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().b))
    };
}

template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB565>::value, SL_ColorRGB565>::type
rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBAType<U>>::type& c)
{
    return SL_ColorRGB565Type<uint8_t>{
        (uint8_t)(c[0] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[0] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().r)),
        (uint8_t)(c[1] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[1] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().g)),
        (uint8_t)(c[2] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[2] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().b))
    };
}



/*-------------------------------------
 * RGB (float) to 565
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB565>::value, SL_ColorRGB565>::type
rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBType<U>>::type& c)
{
    return SL_ColorRGB565Type<uint8_t>{
        (uint8_t)(c[0] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().r),
        (uint8_t)(c[1] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().g),
        (uint8_t)(c[2] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().b)
    };
}

template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB565>::value, SL_ColorRGB565>::type
rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBAType<U>>::type& c)
{
    return SL_ColorRGB565Type<uint8_t>{
        (uint8_t)(c[0] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().r),
        (uint8_t)(c[1] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().g),
        (uint8_t)(c[2] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().b)
    };
}



/*-----------------------------------------------------------------------------
 * RGB-565 to RGB Casting
-----------------------------------------------------------------------------*/
template <typename T, typename U>
constexpr SL_ColorRGBType<T> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGB565Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBType<T>{
        (T)(c.r * (SL_ColorLimits<T, SL_ColorRGBType>::max()[0] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().r)),
        (T)(c.g * (SL_ColorLimits<T, SL_ColorRGBType>::max()[1] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().g)),
        (T)(c.b * (SL_ColorLimits<T, SL_ColorRGBType>::max()[2] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().b))
    };
}

template <typename T, typename U>
constexpr SL_ColorRGBAType<T> rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGB565Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)(c.r * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[0] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().r)),
        (T)(c.g * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[1] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().g)),
        (T)(c.b * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[2] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().b)),
        SL_ColorLimits<T, SL_ColorRGBAType>::max()[3]
    };
}



/*-------------------------------------
 * 565 to RGB (float)
-------------------------------------*/
template <typename T, typename U>
constexpr SL_ColorRGBType<T> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGB565Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBType<T>{
        (T)c.r / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().r,
        (T)c.g / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().g,
        (T)c.b / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().b
    };
}

template <typename T, typename U>
constexpr SL_ColorRGBAType<T> rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGB565Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)c.r / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().r,
        (T)c.g / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().g,
        (T)c.b / (T)SL_ColorLimits<uint8_t, SL_ColorRGB565Type>::max().b,
        SL_ColorLimits<T, SL_ColorRGBAType>::max()[3]
    };
}



/*-----------------------------------------------------------------------------
 * RGB5551 Types
-----------------------------------------------------------------------------*/
template <typename color_t>
struct SL_ColorRGB5551Type;

/**
 * @brief RGB-5551 Color Structure
 */
 template <>
struct alignas(alignof(uint16_t)) SL_ColorRGB5551Type<uint8_t>
{
    typedef uint8_t value_type;
    static constexpr unsigned num_components() noexcept { return 4; }

    uint16_t r : 5;
    uint16_t g : 5;
    uint16_t b : 5;
    uint16_t a : 1;
};

static_assert(sizeof(SL_ColorRGB5551Type<uint8_t>) == sizeof(uint16_t), "Compressed RGB5551 is not 16 bytes.");



/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRGB5551Type<uint8_t>  SL_ColorRGB5551;



/*-----------------------------------------------------------------------------
 * Internal limits of color RGB5551 ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <>
struct SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorRGB5551Type<uint8_t> min() noexcept
    {
        return SL_ColorRGB5551Type<uint8_t>{0, 0, 0, 0};
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorRGB5551Type<uint8_t> max() noexcept
    {
        return SL_ColorRGB5551Type<uint8_t>{31, 31, 31, 1};
    }
};



/*-----------------------------------------------------------------------------
 * RGB to RGB-5551 Casting
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * RGB (integral) to 5551
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB5551>::value, SL_ColorRGB5551>::type
rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBType<U>>::type& c)
{
    return SL_ColorRGB5551Type<uint8_t>{
        (uint8_t)(c[0] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[0] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().r)),
        (uint8_t)(c[1] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[1] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().g)),
        (uint8_t)(c[2] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[2] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().b)),
        (uint8_t)(c[3] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[3] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().a))
    };
}

template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB5551>::value, SL_ColorRGB5551>::type
rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBAType<U>>::type& c)
{
    return SL_ColorRGB5551Type<uint8_t>{
        (uint8_t)(c[0] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[0] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().r)),
        (uint8_t)(c[1] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[1] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().g)),
        (uint8_t)(c[2] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[2] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().b)),
        (uint8_t)(c[3] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[3] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().a))
    };
}



/*-------------------------------------
 * RGB (float) to 5551
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB5551>::value, SL_ColorRGB5551>::type
rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBType<U>>::type& c)
{
    return SL_ColorRGB5551Type<uint8_t>{
        (uint8_t)(c[0] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().r),
        (uint8_t)(c[1] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().g),
        (uint8_t)(c[2] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().b),
        (uint8_t)(c[3] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().a)
    };
}

template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB5551>::value, SL_ColorRGB5551>::type
rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBAType<U>>::type& c)
{
    return SL_ColorRGB5551Type<uint8_t>{
        (uint8_t)(c[0] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().r),
        (uint8_t)(c[1] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().g),
        (uint8_t)(c[2] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().b),
        (uint8_t)(c[3] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().a)
    };
}



/*-----------------------------------------------------------------------------
 * RGB-5551 to RGB Casting
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * 5551 to RGB (integral)
-------------------------------------*/
template <typename T, typename U>
constexpr SL_ColorRGBType<T> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGB5551Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBType<T>{
        (T)(c.r * (SL_ColorLimits<T, SL_ColorRGBType>::max()[0] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().r)),
        (T)(c.g * (SL_ColorLimits<T, SL_ColorRGBType>::max()[1] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().g)),
        (T)(c.b * (SL_ColorLimits<T, SL_ColorRGBType>::max()[2] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().b))
    };
}

template <typename T, typename U>
constexpr SL_ColorRGBAType<T> rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGB5551Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)(c.r * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[0] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().r)),
        (T)(c.g * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[1] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().g)),
        (T)(c.b * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[2] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().b)),
        (T)(c.a * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[3] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().a)),
    };
}



/*-------------------------------------
 * 5551 to RGB (float)
-------------------------------------*/
template <typename T, typename U>
constexpr SL_ColorRGBType<T> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGB5551Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBType<T>{
        (T)c.r / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().r,
        (T)c.g / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().g,
        (T)c.b / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().b
    };
}
template <typename T, typename U>
constexpr SL_ColorRGBAType<T> rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGB5551Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)c.r / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().r,
        (T)c.g / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().g,
        (T)c.b / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().b,
        (T)c.a / (T)SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().a
    };
}



/*-----------------------------------------------------------------------------
 * RGB4444 Types
-----------------------------------------------------------------------------*/
template <typename color_t>
struct SL_ColorRGB4444Type;

/**
 * @brief RGB-4444 Color Structure
 */
 template <>
struct alignas(alignof(uint16_t)) SL_ColorRGB4444Type<uint8_t>
{
    typedef uint8_t value_type;
    static constexpr unsigned num_components() noexcept { return 4; }

    uint16_t r : 4;
    uint16_t g : 4;
    uint16_t b : 4;
    uint16_t a : 4;
};

static_assert(sizeof(SL_ColorRGB4444Type<uint8_t>) == sizeof(uint16_t), "Compressed RGB4444 is not 16 bytes.");



/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRGB4444Type<uint8_t>  SL_ColorRGB4444;



/*-----------------------------------------------------------------------------
 * Internal limits of color RGBA4444 ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <>
struct SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorRGB4444Type<uint8_t> min() noexcept
    {
        return SL_ColorRGB4444Type<uint8_t>{0, 0, 0, 0};
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    */
    static constexpr SL_ColorRGB4444Type<uint8_t> max() noexcept
    {
        return SL_ColorRGB4444Type<uint8_t>{15, 15, 15, 15};
    }
};



/*-----------------------------------------------------------------------------
 * RGB to RGB-4444 Casting
-----------------------------------------------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB4444>::value, SL_ColorRGB4444>::type
rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBType<U>>::type& c)
{
    return SL_ColorRGB4444Type<uint8_t>{
        (uint8_t)(c[0] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[0] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().r)),
        (uint8_t)(c[1] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[1] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().g)),
        (uint8_t)(c[2] / (SL_ColorLimits<U, SL_ColorRGBType>::max()[2] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().b)),
        SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().a
    };
}

template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB4444>::value, SL_ColorRGB4444>::type
rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBAType<U>>::type& c)
{
    return SL_ColorRGB4444Type<uint8_t>{
        (uint8_t)(c[0] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[0] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().r)),
        (uint8_t)(c[1] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[1] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().g)),
        (uint8_t)(c[2] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[2] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().b)),
        (uint8_t)(c[3] / (SL_ColorLimits<U, SL_ColorRGBAType>::max()[3] / (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().a))
    };
}



/*-------------------------------------
 * RGB (float) to 4444
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB4444>::value, SL_ColorRGB4444>::type
rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBType<U>>::type& c)
{
    return SL_ColorRGB4444Type<uint8_t>{
        (uint8_t)(c[0] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().r),
        (uint8_t)(c[1] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().g),
        (uint8_t)(c[2] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().b),
        (uint8_t)(SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().a)
    };
}

template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsSame<T, SL_ColorRGB4444>::value, SL_ColorRGB4444>::type
rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBAType<U>>::type& c)
{
    return SL_ColorRGB4444Type<uint8_t>{
        (uint8_t)(c[0] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().r),
        (uint8_t)(c[1] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().g),
        (uint8_t)(c[2] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().b),
        (uint8_t)(c[3] * (U)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().a)
    };
}



/*-----------------------------------------------------------------------------
 * RGB-4444 to RGB Casting
-----------------------------------------------------------------------------*/
template <typename T, typename U>
constexpr SL_ColorRGBType<T> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGB4444Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBType<T>{
        (T)(c.r * (SL_ColorLimits<T, SL_ColorRGBType>::max()[0] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().r)),
        (T)(c.g * (SL_ColorLimits<T, SL_ColorRGBType>::max()[1] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().g)),
        (T)(c.b * (SL_ColorLimits<T, SL_ColorRGBType>::max()[2] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().b))
    };
}

template <typename T, typename U>
constexpr SL_ColorRGBAType<T> rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGB4444Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)(c.r * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[0] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().r)),
        (T)(c.g * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[1] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().g)),
        (T)(c.b * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[2] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().b)),
        (T)(c.a * (SL_ColorLimits<T, SL_ColorRGBAType>::max()[3] / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().a))
    };
}



/*-------------------------------------
 * 4444 to RGB (float)
-------------------------------------*/
template <typename T, typename U>
constexpr SL_ColorRGBType<T> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGB4444Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBType<T>{
        (T)c.r / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().r,
        (T)c.g / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().g,
        (T)c.b / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().b
    };
}

template <typename T, typename U>
constexpr SL_ColorRGBAType<T> rgba_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGB4444Type<uint8_t>>::type& c) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)c.r / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().r,
        (T)c.g / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().g,
        (T)c.b / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().b,
        (T)c.a / (T)SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().a
    };
}



#endif /* SL_COLOR_COMPRESSED_HPP */

