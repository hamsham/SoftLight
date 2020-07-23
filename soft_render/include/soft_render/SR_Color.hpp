
#ifndef SR_COLOR_TYPE_HPP
#define SR_COLOR_TYPE_HPP

#include "lightsky/setup/Api.h"
#include "lightsky/setup/Compiler.h"

// Microsoft imposing its will on us inferior programmers
#ifdef LS_COMPILER_MSC
    #ifdef min
        #undef min
    #endif /* min */

    #ifdef max
        #undef max
    #endif /* max */
#endif /* LS_COMPILER_MSC */

#include <cassert>
#include <cstdint> // fixed-width types
#include <limits> // c++ limits

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec2.h"
#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"



/**----------------------------------------------------------------------------
 * @brief SR_Color Information
-----------------------------------------------------------------------------*/
enum SR_ColorDataType : uint16_t
{
    SR_COLOR_R_8U,
    SR_COLOR_R_16U,
    SR_COLOR_R_32U,
    SR_COLOR_R_64U,
    SR_COLOR_R_FLOAT,
    SR_COLOR_R_DOUBLE,

    SR_COLOR_RG_8U,
    SR_COLOR_RG_16U,
    SR_COLOR_RG_32U,
    SR_COLOR_RG_64U,
    SR_COLOR_RG_FLOAT,
    SR_COLOR_RG_DOUBLE,

    SR_COLOR_RGB_8U,
    SR_COLOR_RGB_16U,
    SR_COLOR_RGB_32U,
    SR_COLOR_RGB_64U,
    SR_COLOR_RGB_FLOAT,
    SR_COLOR_RGB_DOUBLE,

    SR_COLOR_RGBA_8U,
    SR_COLOR_RGBA_16U,
    SR_COLOR_RGBA_32U,
    SR_COLOR_RGBA_64U,
    SR_COLOR_RGBA_FLOAT,
    SR_COLOR_RGBA_DOUBLE,

    SR_COLOR_RGB_DEFAULT = SR_COLOR_RGB_8U,
    SR_COLOR_INVALID
};



/*-------------------------------------
 * Number of bytes per color
-------------------------------------*/
size_t sr_bytes_per_color(SR_ColorDataType p);



/*-------------------------------------
 * Number of elements per color
-------------------------------------*/
unsigned sr_elements_per_color(SR_ColorDataType p);



/*-------------------------------------
 * Helper Structure to Convert 8-bit colors to float
-------------------------------------*/
union SR_ColorU8ToF
{
    uint32_t i;
    float f;

    static constexpr float byte_to_float(const uint8_t c) noexcept
    {
        return SR_ColorU8ToF{0x3F800000u + c * 0x00008080u + ((c+1u) >> 1u)}.f - 1.f;
    }
};



/**
 * @brief Enumeration to describe the color model being used by a SR_ColorType<>
 * object.
 */
enum class SR_ColorModelType
{
    SR_COLOR_MODEL_RGB,
    SR_COLOR_MODEL_HSV,
    SR_COLOR_MODEL_HSL
};



/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <typename color_t>
struct SR_ColorLimits
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr color_t min() noexcept
    {
        return ls::setup::IsFloat<color_t>::value ? static_cast<color_t>(0.0) : std::numeric_limits<color_t>::min();
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 10.0.
    */
    static constexpr color_t max() noexcept
    {
        return ls::setup::IsFloat<color_t>::value ? static_cast<color_t>(1.0) : std::numeric_limits<color_t>::max();
    }
};

template <>
struct SR_ColorLimits<ls::math::half>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr ls::math::half min() noexcept
    {
        return ls::math::half{0x00u, 0x00u};
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 10.0.
    */
    static constexpr ls::math::half max() noexcept
    {
        return ls::math::half{0x7Bu, 0xFFu};
    }
};



/**----------------------------------------------------------------------------
 * @brief Red-only Color Types
-----------------------------------------------------------------------------*/
template <typename T>
struct alignas(sizeof(T)) SR_ColorRType
{
    typedef T value_type;

    T r;

    static constexpr unsigned num_components() noexcept { return 1; }

    constexpr SR_ColorRType() noexcept = default;
    constexpr SR_ColorRType(const SR_ColorRType& c ) noexcept = default;
    constexpr SR_ColorRType(SR_ColorRType&& c ) noexcept = default;

    explicit constexpr SR_ColorRType(T n) noexcept : r{n} {}

    template <typename C>
    explicit constexpr SR_ColorRType(C n) noexcept : r{(T)n} {}

    inline SR_ColorRType& operator=(const SR_ColorRType& c ) noexcept = default;
    inline SR_ColorRType& operator=(SR_ColorRType&& c ) noexcept = default;
    inline SR_ColorRType& operator=(const T n) noexcept { r = n; return *this; }

    explicit constexpr operator T() const noexcept { return r; }

    template <typename C>
    explicit constexpr operator C() const noexcept { return (C)r; }

    template <typename C>
    explicit constexpr operator SR_ColorRType<C>() const noexcept { return SR_ColorRType{(C)r}; }

    template <typename index_t>
    constexpr T operator[] (index_t n) const noexcept { return (&r)[n]; }

    template <typename index_t>
    inline T& operator[] (index_t n) noexcept { return (&r)[n]; }

    constexpr SR_ColorRType operator+(const SR_ColorRType<T>& n) const noexcept { return SR_ColorRType{r + n.r}; }
    constexpr SR_ColorRType operator*(const SR_ColorRType<T>& n) const noexcept { return SR_ColorRType{r * n.r}; }

    constexpr SR_ColorRType operator+(const T n) const noexcept { return SR_ColorRType{r + n}; }
    constexpr SR_ColorRType operator*(const T n) const noexcept { return SR_ColorRType{r * n}; }
};



/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SR_ColorRType<uint8_t>  SR_ColorR8;
typedef SR_ColorRType<uint16_t> SR_ColorR16;
typedef SR_ColorRType<uint32_t> SR_ColorR32;
typedef SR_ColorRType<uint64_t> SR_ColorR64;
typedef SR_ColorRType<float>    SR_ColorRf;
typedef SR_ColorRType<double>   SR_ColorRd;

typedef SR_ColorR8 SR_ColorR;



/*-------------------------------------
 * Data type casting
-------------------------------------*/
/*-------------------------------------
 * Integer to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SR_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SR_ColorRType<U>>::type& p)
{
    return SR_ColorRType<T>
   {
       (T)(((float)std::numeric_limits<T>::max() / (float)std::numeric_limits<U>::max()) * p.r)
   };
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SR_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SR_ColorRType<U>>::type& p)
{
    return (SR_ColorRType<T>)(p.r * (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SR_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SR_ColorRType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
    ?
        SR_ColorRType<T>{0.5f * ((T)p.r * (T{1} / (float)std::numeric_limits<U>::max())) + 0.5f}
    :
        SR_ColorRType<T>{(T)p.r * (T{1} / (float)std::numeric_limits<U>::max())};
}



template <>
constexpr SR_ColorRType<float> color_cast<float, uint8_t>(const SR_ColorRType<uint8_t>& p)
{
    return SR_ColorRType<float>{(float)p.r * 0.00392156862745f};
    //return SR_ColorRType<float>{SR_ColorU8ToF::byte_to_float(p.r)};
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SR_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SR_ColorRType<U>>::type& p)
{
    return SR_ColorRType<T>{(T)p.r};
}



/*-------------------------------------
 * Data type casting (no-op)
-------------------------------------*/
template <typename T, T>
constexpr SR_ColorRType<T> color_cast(const SR_ColorRType<T>& p)
{
    return p;
}



/**----------------------------------------------------------------------------
 * @brief RG Color Type
-----------------------------------------------------------------------------*/
template <typename T>
using SR_ColorRGType = ls::math::vec2_t<T>;


/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SR_ColorRGType<uint8_t>  SR_ColorRG8;
typedef SR_ColorRGType<uint16_t> SR_ColorRG16;
typedef SR_ColorRGType<uint32_t> SR_ColorRG32;
typedef SR_ColorRGType<uint64_t> SR_ColorRG64;
typedef SR_ColorRGType<float>    SR_ColorRGf;
typedef SR_ColorRGType<double>   SR_ColorRGd;

typedef SR_ColorRG8 SR_ColorRG;



/*-------------------------------------
 * Data type casting
-------------------------------------*/
/*-------------------------------------
 * Integral to integral
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SR_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SR_ColorRGType<U>>::type& p)
{
    return (SR_ColorRGType<T>)((SR_ColorRGType<float>)p * ((float)std::numeric_limits<T>::max() / (float)std::numeric_limits<U>::max()));
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SR_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SR_ColorRGType<U>>::type& p)
{
    return (SR_ColorRGType<T>)(p * (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SR_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SR_ColorRGType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
           ? (SR_ColorRGType<T>{0.5f} * ((SR_ColorRGType<T>)p * (T{1} / (float)std::numeric_limits<U>::max())) + 0.5f)
           : ((SR_ColorRGType<T>)p * (T{1} / (float)std::numeric_limits<U>::max()));
}



template <>
inline SR_ColorRGType<float> color_cast<float, uint8_t>(const SR_ColorRGType<uint8_t>& p)
{
    return ((SR_ColorRGType<float>)p) * 0.00392156862745f;
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SR_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SR_ColorRGType<U>>::type& p)
{
    return (SR_ColorRGType<T>)p;
}



/*-------------------------------------
 * Data type casting (no-op)
-------------------------------------*/
template <typename T, T>
constexpr SR_ColorRGType<T> color_cast(const SR_ColorRGType<T>& p)
{
    return p;
}



/**----------------------------------------------------------------------------
 * @brief Generic RGB Color Structure
-----------------------------------------------------------------------------*/
template <typename T>
using SR_ColorRGBType = ls::math::vec3_t<T>;


/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SR_ColorRGBType<uint8_t>  SR_ColorRGB8;
typedef SR_ColorRGBType<uint16_t> SR_ColorRGB16;
typedef SR_ColorRGBType<uint32_t> SR_ColorRGB32;
typedef SR_ColorRGBType<uint64_t> SR_ColorRGB64;
typedef SR_ColorRGBType<float>    SR_ColorRGBf;
typedef SR_ColorRGBType<double>   SR_ColorRGBd;

typedef SR_ColorRGB8 SR_ColorRGB;



/*-------------------------------------
 * Data type casting
-------------------------------------*/
/*-------------------------------------
 * Integral to integral
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SR_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SR_ColorRGBType<U>>::type& p)
{
    return (SR_ColorRGBType<T>)((SR_ColorRGBType<float>)p * ((float)std::numeric_limits<T>::max() / (float)std::numeric_limits<U>::max()));
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SR_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SR_ColorRGBType<U>>::type& p)
{
    return (SR_ColorRGBType<T>)(p * (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SR_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SR_ColorRGBType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
           ? (SR_ColorRGBType<T>{0.5f} * ((SR_ColorRGBType<T>)p * (T{1} / (float)std::numeric_limits<U>::max())) + 0.5f)
           : ((SR_ColorRGBType<T>)p * (T{1} / (float)std::numeric_limits<U>::max()));
}



template <>
inline SR_ColorRGBType<float> color_cast<float, uint8_t>(const SR_ColorRGBType<uint8_t>& p)
{
    return ((SR_ColorRGBType<float>)p) * 0.00392156862745f;
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SR_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SR_ColorRGBType<U>>::type& p)
{
    return (SR_ColorRGBType<T>)p;
}



/*-------------------------------------
 * Data type casting (no-op)
-------------------------------------*/
template <typename T, T>
constexpr SR_ColorRGBType<T> color_cast(const SR_ColorRGBType<T>& p)
{
    return p;
}



/**----------------------------------------------------------------------------
 * @brief RGBA Color Types
-----------------------------------------------------------------------------*/
template <typename T>
using SR_ColorRGBAType = ls::math::vec4_t<T>;


/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SR_ColorRGBAType<uint8_t>  SR_ColorRGBA8;
typedef SR_ColorRGBAType<uint16_t> SR_ColorRGBA16;
typedef SR_ColorRGBAType<uint32_t> SR_ColorRGBA32;
typedef SR_ColorRGBAType<uint64_t> SR_ColorRGBA64;
typedef SR_ColorRGBAType<float>    SR_ColorRGBAf;
typedef SR_ColorRGBAType<double>   SR_ColorRGBAd;

typedef SR_ColorRGBA8 SR_ColorRGBA;



/*-------------------------------------
 * Data type casting
-------------------------------------*/
/*-------------------------------------
 * Integral to integral
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SR_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SR_ColorRGBAType<U>>::type& p)
{
    return (SR_ColorRGBAType<T>)((SR_ColorRGBAType<float>)p * ((float)std::numeric_limits<T>::max() / (float)std::numeric_limits<U>::max()));
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SR_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SR_ColorRGBAType<U>>::type& p)
{
    return (SR_ColorRGBAType<T>)(p * (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SR_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SR_ColorRGBAType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
    ? (SR_ColorRGBAType<T>{0.5f} * ((SR_ColorRGBAType<T>)p * (T{1} / std::numeric_limits<U>::max())) + 0.5f)
    : ((SR_ColorRGBAType<T>)p * (T{1} / (float)std::numeric_limits<U>::max()));
}



template <>
inline SR_ColorRGBAType<float> color_cast<float, uint8_t>(const SR_ColorRGBAType<uint8_t>& p)
{
    return ((SR_ColorRGBAType<float>)p) * 0.00392156862745f;
}



/*
template <>
inline SR_ColorRGBAType<float> color_cast<float, uint16_t>(const SR_ColorRGBAType<uint16_t>& p)
{
    return (SR_ColorRGBAType<float>)p * 1.52590218967e-05f;
}



template <>
inline SR_ColorRGBAType<float> color_cast<float, uint32_t>(const SR_ColorRGBAType<uint32_t>& p)
{
    return (SR_ColorRGBAType<float>)p * 2.32830643708e-10f;
}
*/



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SR_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SR_ColorRGBAType<U>>::type& p)
{
    return (SR_ColorRGBAType<T>)p;
}



/*-------------------------------------
 * Data type casting (no-op)
-------------------------------------*/
template <typename T, T>
constexpr SR_ColorRGBAType<T> color_cast(const SR_ColorRGBAType<T>& p)
{
    return p;
}



/*-----------------------------------------------------------------------------
 * Extended Color Models
-----------------------------------------------------------------------------*/
/**
 * @brief Generic HSV Color Structure
 */
template<typename color_t>
struct alignas(sizeof(color_t)) SR_ColorTypeHSV
{
    static_assert(ls::setup::IsFloat<color_t>::value, "HSV can only be represented by floating-point numbers.");
        
    public:
        color_t h;
        color_t s;
        color_t v;
};

typedef SR_ColorTypeHSV<float> SR_ColorTypeHSVf;
typedef SR_ColorTypeHSV<double> SR_ColorTypeHSVd;




/**
 * @brief Generic HSL Color Structure
 */
template<typename color_t>
struct alignas(sizeof(color_t)) SR_ColorTypeHSL
{
    static_assert(ls::setup::IsFloat<color_t>::value, "HSL can only be represented by floating-point numbers.");
        
    public:
        color_t h;
        color_t s;
        color_t l;
};

typedef SR_ColorTypeHSV<float> SR_ColorTypeHSLf;
typedef SR_ColorTypeHSL<double> SR_ColorTypeHSLd;



/*-----------------------------------------------------------------------------
 * Color Casting Operations
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Cast from HSV to RGB
--------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorRGBType<color_t> rgb_cast(const SR_ColorTypeHSV<other_color_t>& inC) noexcept
{
    const other_color_t c = inC.v * inC.s;
    const other_color_t x = c * (other_color_t{1.f} - ls::math::abs(ls::math::fmod(inC.h/other_color_t{60.f}, other_color_t{2.f}) - other_color_t{1.f}));
    const other_color_t m = inC.v - c;

    other_color_t tempR;
    other_color_t tempG;
    other_color_t tempB;

    if (inC.h <= other_color_t{60.f})
    {
        tempR = c;
        tempG = x;
        tempB = 0.f;
    }
    else if (inC.h <= other_color_t{120.f})
    {
        tempR = x;
        tempG = c;
        tempB = 0.f;
    }
    else if (inC.h <= other_color_t{180.f})
    {
        tempR = 0.f;
        tempG = c;
        tempB = x;
    }
    else if (inC.h <= other_color_t{240.f})
    {
        tempR = 0.f;
        tempG = x;
        tempB = c;
    }
    else if (inC.h <= other_color_t{300.f})
    {
        tempR = x;
        tempG = 0.f;
        tempB = c;
    }
    else
    {
        tempR = c;
        tempG = 0.f;
        tempB = x;
    }
    
    tempR += m;
    tempG += m;
    tempB += m;

    static const other_color_t COLOR_MAX_VAL = SR_ColorLimits<color_t>::max();
        
    return SR_ColorRGBType<color_t>{
        static_cast<color_t>(tempR * COLOR_MAX_VAL),
        static_cast<color_t>(tempG * COLOR_MAX_VAL),
        static_cast<color_t>(tempB * COLOR_MAX_VAL)
    };
}



/*--------------------------------------
 * Cast from HSL to RGB
--------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorRGBType<color_t> rgb_cast(const SR_ColorTypeHSL<other_color_t>& inC) noexcept
{
    const other_color_t c = inC.s * (other_color_t{1.f} - ls::math::abs(other_color_t{2.f} * inC.l - other_color_t{1.f}));
    const other_color_t x = c * (other_color_t{1.f} - ls::math::abs(ls::math::fmod(inC.h/other_color_t{60.f}, other_color_t{2.f}) - other_color_t{1.f}));
    const other_color_t m = inC.l - (c * other_color_t{0.5f});
    
    other_color_t tempR;
    other_color_t tempG;
    other_color_t tempB;

    if (inC.h <= other_color_t{60.f})
    {
        tempR = c;
        tempG = x;
        tempB = 0.f;
    }
    else if (inC.h <= other_color_t{120.f})
    {
        tempR = x;
        tempG = c;
        tempB = 0.f;
    }
    else if (inC.h <= other_color_t{180.f})
    {
        tempR = 0.f;
        tempG = c;
        tempB = x;
    }
    else if (inC.h <= other_color_t{240.f})
    {
        tempR = 0.f;
        tempG = x;
        tempB = c;
    }
    else if (inC.h <= other_color_t{300.f})
    {
        tempR = x;
        tempG = 0.f;
        tempB = c;
    }
    else
    {
        tempR = c;
        tempG = 0.f;
        tempB = x;
    }
    
    tempR += m;
    tempG += m;
    tempB += m;

    constexpr other_color_t COLOR_MAX_VAL = SR_ColorLimits<color_t>::max();
    
    return SR_ColorRGBType<color_t>{
        static_cast<color_t>(tempR * COLOR_MAX_VAL),
        static_cast<color_t>(tempG * COLOR_MAX_VAL),
        static_cast<color_t>(tempB * COLOR_MAX_VAL)
    };
}



/*--------------------------------------
 * Empty HSV Cast
--------------------------------------*/
template <typename color_t>
constexpr SR_ColorTypeHSV<color_t> hsv_cast(const SR_ColorTypeHSV<color_t>& c) noexcept
{
    return c;
}



/*--------------------------------------
 * Cast from one HSV type to another HSV Type
--------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorTypeHSV<color_t> hsv_cast(const SR_ColorTypeHSV<other_color_t>& c) noexcept
{
    return SR_ColorTypeHSV<color_t>{
        static_cast<color_t>(c.h),
        static_cast<color_t>(c.s),
        static_cast<color_t>(c.v)
    };
}



/*--------------------------------------
 * RGB To HSV
--------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorTypeHSV<color_t> hsv_cast(const SR_ColorRGBType<other_color_t>& c) noexcept
{
    static const color_t COLOR_EPSILON = color_t{1.0e-6f};
    
    // HSV deals with normalized numbers. Integral types won't work until
    // we're ready to return the data.
    static const color_t COLOR_MAX_VAL = SR_ColorLimits<other_color_t>::max();
    static const color_t COLOR_MIN_VAL = SR_ColorLimits<other_color_t>::min();
    color_t normR, normG, normB;

    if (ls::setup::IsFloat<other_color_t>::value)
    {
        normR = (color_t)(0.5f * (static_cast<float>(c[0]) + 1.f));
        normG = (color_t)(0.5f * (static_cast<float>(c[1]) + 1.f));
        normB = (color_t)(0.5f * (static_cast<float>(c[2]) + 1.f));
    }
    else
    {
        normR = static_cast<color_t>(c[0]) / COLOR_MAX_VAL;
        normG = static_cast<color_t>(c[1]) / COLOR_MAX_VAL;
        normB = static_cast<color_t>(c[2]) / COLOR_MAX_VAL;
    }

    // normalize the input values and calculate their deltas
    const color_t maxVal = ls::math::max(normR, ls::math::max(normG, normB));
    const color_t minVal = ls::math::min(normR, ls::math::min(normG, normB));
    const color_t delta = maxVal - minVal;
    
    // check if we are near 0 (min)
    if (ls::math::abs(maxVal) <= COLOR_MIN_VAL)
    {
        return SR_ColorTypeHSV<color_t>{
            static_cast<color_t>(-1.f),
            static_cast<color_t>(NAN),
            static_cast<color_t>(INFINITY)
        };
    }

  color_t hue = 60.f;

  if (ls::math::abs(maxVal-normR) <= COLOR_EPSILON)
  {
    hue *= ls::math::fmod(normG - normB, 6.f) / delta;
  }
  else if (ls::math::abs(maxVal-normG) <= COLOR_EPSILON)
  {
    hue *= 2.f + ((normB - normR) / delta);
  }
  else
  {
    hue *= 4.f + ((normR - normG) / delta);
  }

  // This part of the conversion requires a data type with more than 2 bytes.
  // Some values may be valid, others may be truncated/undefined.
  hue = (hue < color_t{0.f}) ? (hue+color_t{360.f}) : hue;

  // result
  return SR_ColorTypeHSV<color_t>{hue, delta / maxVal, maxVal};
}



/*--------------------------------------
 * HSL To HSV
--------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorTypeHSV<color_t> hsv_cast(const SR_ColorTypeHSL<other_color_t>& c) noexcept
{
    color_t l = color_t{2} * c.l;
    color_t s = c.s * ((l <= color_t{1}) ? l : color_t{2} - l);

    return SR_ColorTypeHSV<color_t>{
        c.h,
        (color_t{2} * s) / (l + s),
        (l + s) / color_t{2}
    };
}




/*-------------------------------------
 * RGB to HSL
-------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorTypeHSL<color_t> hsl_cast(const SR_ColorRGBType<other_color_t>& c) noexcept
{
    static const color_t COLOR_EPSILON = color_t{1.0e-6f};
    
    // HSL deals with normalized numbers. Integral types won't work until
    // we're ready to return the data.
    static const color_t COLOR_MAX_VAL = SR_ColorLimits<other_color_t>::max();
    static const color_t COLOR_MIN_VAL = SR_ColorLimits<other_color_t>::min();
    color_t normR, normG, normB;

    if (ls::setup::IsFloat<other_color_t>::value)
    {
        normR = (color_t)(0.5f * (static_cast<float>(c[0]) + 1.f));
        normG = (color_t)(0.5f * (static_cast<float>(c[1]) + 1.f));
        normB = (color_t)(0.5f * (static_cast<float>(c[2]) + 1.f));
    }
    else
    {
        normR = static_cast<color_t>(c[0]) / COLOR_MAX_VAL;
        normG = static_cast<color_t>(c[1]) / COLOR_MAX_VAL;
        normB = static_cast<color_t>(c[2]) / COLOR_MAX_VAL;
    }

    // normalize the input values and calculate their deltas
    const color_t maxVal = ls::math::max(normR, ls::math::max(normG, normB));
    const color_t minVal = ls::math::min(normR, ls::math::min(normG, normB));
    const color_t delta = maxVal - minVal;

    // check if we are near 0
    if (ls::math::abs(maxVal) <= COLOR_MIN_VAL)
    {
        return SR_ColorTypeHSL<color_t>{0.f, 0.f, 0.f};
    }

    color_t hue = 60.f;

    if (ls::math::abs(maxVal-normR) <= COLOR_EPSILON)
    {
        hue *= ls::math::fmod(normG - normB, 6.f) / delta;
    }
    else if (ls::math::abs(maxVal-normG) <= COLOR_EPSILON)
    {
        hue *= 2.f + ((normB - normR) / delta);
    }
    else
    {
        hue *= 4.f + ((normR - normG) / delta);
    }

    // This part of the conversion requires a data type with more than 2 bytes.
    // Some values may be valid, others may be truncated/undefined.
    hue = (hue < color_t{0.f}) ? (hue+color_t{360.f}) : hue;

    color_t lightness = color_t{0.5f} * (maxVal+minVal);
    color_t saturation = color_t{0.f};
    if (ls::math::abs(maxVal) > COLOR_MIN_VAL)
    {
        saturation = delta / (color_t{1.f} - ls::math::abs(color_t{2.f} * lightness - color_t{1.f}));
    }

    // result
    return SR_ColorTypeHSL<color_t>{hue, saturation, lightness};
}



/*-------------------------------------
 * HSV to HSL
-------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorTypeHSL<color_t> hsl_cast(const SR_ColorTypeHSV<other_color_t>& c) noexcept
{
    color_t s = c.s * c.v;
    color_t l = (color_t{2} - c.s) * c.v;

    return SR_ColorTypeHSL<color_t>{
        c.h,
        s / ((l <= color_t{1}) ? l : color_t{2} - l),
        l / color_t{2}
    };
}



/*-----------------------------------------------------------------------------
 * YCoCg Types
-----------------------------------------------------------------------------*/
/**
 * @brief Generic YcoCg Color Structure
 */
template<typename color_t>
struct alignas(sizeof(color_t)) SR_ColorTypeYCoCg
{
    color_t y;
    color_t co;
    color_t cg;
};

// Specializations
typedef SR_ColorTypeYCoCg<int8_t>   SR_ColorYCoCg8;
typedef SR_ColorTypeYCoCg<uint8_t>  SR_ColorYCoCg8u;
typedef SR_ColorTypeYCoCg<int16_t>  SR_ColorYCoCg16;
typedef SR_ColorTypeYCoCg<uint16_t> SR_ColorYCoCg16u;
typedef SR_ColorTypeYCoCg<int32_t>  SR_ColorYCoCg32;
typedef SR_ColorTypeYCoCg<uint32_t> SR_ColorYCoCg32u;
typedef SR_ColorTypeYCoCg<int64_t>  SR_ColorYCoCg64;
typedef SR_ColorTypeYCoCg<uint64_t> SR_ColorYCoCg64u;
typedef SR_ColorTypeYCoCg<float>    SR_ColorYCoCgf;
typedef SR_ColorTypeYCoCg<double>   SR_ColorYCoCgd;

typedef SR_ColorYCoCgf SR_ColorYCoCg;



/**
 * @brief YcoCg Color Structure with Alpha
 */
template<typename color_t>
struct alignas(sizeof(color_t)*4) SR_ColorTypeYCoCgA
{
    color_t y;
    color_t co;
    color_t cg;
    color_t a;
};

// Specializations
typedef SR_ColorTypeYCoCgA<int8_t>   SR_ColorYCoCgA8;
typedef SR_ColorTypeYCoCgA<uint8_t>  SR_ColorYCoCgA8u;
typedef SR_ColorTypeYCoCgA<int16_t>  SR_ColorYCoCgA16;
typedef SR_ColorTypeYCoCgA<uint16_t> SR_ColorYCoCgA16u;
typedef SR_ColorTypeYCoCgA<int32_t>  SR_ColorYCoCgA32;
typedef SR_ColorTypeYCoCgA<uint32_t> SR_ColorYCoCgA32u;
typedef SR_ColorTypeYCoCgA<int64_t>  SR_ColorYCoCgA64;
typedef SR_ColorTypeYCoCgA<uint64_t> SR_ColorYCoCgA64u;
typedef SR_ColorTypeYCoCgA<float>    SR_ColorYCoCgAf;
typedef SR_ColorTypeYCoCgA<double>   SR_ColorYCoCgAd;

typedef SR_ColorYCoCgAf SR_ColorYCoCgA;



/*-----------------------------------------------------------------------------
 * YCoCg & RGB Casting
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * RGB to YCoCg
-------------------------------------*/
template <typename T>
constexpr SR_ColorTypeYCoCg<T> ycocg_cast(const SR_ColorRGBType<T>& p) noexcept
{
    return SR_ColorTypeYCoCg<T>{
        (T)((p[0]/T{4}) + (p[1]/T{2}) - (p[2]/T{4})),
        (T)((p[0]/T{2})               + (p[2]/T{2})),
        (T)((p[0]/T{4}) - (p[1]/T{2}) - (p[2]/T{4}))
    };
}



/*-------------------------------------
 * YCoCg to RGB
-------------------------------------*/
template <typename T>
constexpr SR_ColorRGBType<T> rgb_cast(const SR_ColorTypeYCoCg<T>& p) noexcept
{
    return SR_ColorRGBType<T>{
        (T)(p.y + p.co + p.cg),
        (T)(p.y        - p.cg),
        (T)(p.co - p.y - p.cg),
    };
}



/*-----------------------------------------------------------------------------
 * YCoCg & RGB Casting
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * RGBA to YCoCg
-------------------------------------*/
template <typename T>
constexpr SR_ColorTypeYCoCgA<T> ycocg_cast(const SR_ColorRGBAType<T>& p) noexcept
{
    return SR_ColorTypeYCoCgA<T>{
        (T)((p[0]/T{4}) + (p[1]/T{2}) - (p[2]/T{4})),
        (T)((p[0]/T{2})               + (p[2]/T{2})),
        (T)((p[0]/T{4}) - (p[1]/T{2}) - (p[2]/T{4})),
        p[3]
    };
}

#if defined(LS_ARCH_X86)
inline SR_ColorTypeYCoCgA<float> ycocg_cast(const SR_ColorRGBAType<float>& p) noexcept
{
    const __m128 rgb = _mm_loadu_ps(&p);
    const __m128 a = _mm_set_ps(1.f,  0.25f, 0.5f,  0.25f);
    const __m128 b = _mm_set_ps(0.f, -0.5f,  0.f,   0.5f);
    const __m128 c = _mm_set_ps(0.f, -0.25f, 0.5f, -0.25f);
    const __m128 ycocg = _mm_fmadd_ps(rgb, c, _mm_fmadd_ps(rgb, b, _mm_mul_ps(rgb, a)));

    SR_ColorTypeYCoCgA<float> ret;
    _mm_store_ps(&ret.y, ycocg);
    return ret;
}

#elif defined(LS_ARCH_ARM)
inline SR_ColorTypeYCoCgA<float> ycocg_cast(const SR_ColorRGBAType<float>& p) noexcept
{
    constexpr float32x4_t a{1.f,  0.25f, 0.5f,  0.25f};
    constexpr float32x4_t b{0.f, -0.5f,  0.f,   0.5f};
    constexpr float32x4_t c{0.f, -0.25f, 0.5f, -0.25f};

    const float32x4_t ycocg = vmlaq_f32(vmlaq_f32(vmulq_f32(p.simd, a), p.simd, b), p.simd, c);

    SR_ColorTypeYCoCgA<float> ret;
    vst1q_f32(&ret.y, ycocg);
    return ret;
}

#endif



/*-------------------------------------
 * YCoCgA to RGBA
-------------------------------------*/
template <typename T>
constexpr SR_ColorRGBAType<T> rgb_cast(const SR_ColorTypeYCoCgA<T>& p) noexcept
{
    return SR_ColorRGBAType<T>{
        (T)(p.y + p.co + p.cg),
        (T)(p.y        - p.cg),
        (T)(p.co - p.y - p.cg),
        p.a
    };
}



#endif /* SR_COLOR_TYPE_HPP */

