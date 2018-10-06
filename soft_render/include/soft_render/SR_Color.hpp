
#ifndef SR_COLOR_TYPE_HPP
#define SR_COLOR_TYPE_HPP

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

#include <algorithm> // std::max, std::min
#include <cassert>
#include <cstdint> // fixed-width types
#include <limits> // c++ limits
#include <type_traits> // std::is_floating_point

#include "lightsky/math/scalar_utils.h"



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
std::size_t sr_bytes_per_color(SR_ColorDataType p);



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
        return std::is_floating_point<color_t>::value ? static_cast<color_t>(0.0) : std::numeric_limits<color_t>::min();
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
        return std::is_floating_point<color_t>::value ? static_cast<color_t>(1.0) : std::numeric_limits<color_t>::max();
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

    constexpr T operator[] (int n) const noexcept { return (&r)[n]; };
    inline T& operator[] (int n) noexcept { return (&r)[n]; };
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
constexpr typename std::enable_if<std::is_integral<T>::value, SR_ColorRType<T>>::type
color_cast(const typename std::enable_if<std::is_integral<U>::value, SR_ColorRType<U>>::type& p)
{
    return SR_ColorRType<T>
   {
       (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.r)
   };
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_integral<T>::value, SR_ColorRType<T>>::type
color_cast(const typename std::enable_if<std::is_floating_point<U>::value, SR_ColorRType<U>>::type& p)
{
    return SR_ColorRType<T>
    {
        (T)(std::numeric_limits<U>::max() * p.r)
    };
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_floating_point<T>::value, SR_ColorRType<T>>::type
color_cast(const typename std::enable_if<std::is_integral<U>::value, SR_ColorRType<U>>::type& p)
{
    return std::is_signed<U>::value
    ?
        SR_ColorRType<T>{0.5f * ((T)p.r * (T{1} / std::numeric_limits<U>::max())) + 0.5f}
    :
        SR_ColorRType<T>{(T)p.r * (T{1} / std::numeric_limits<U>::max())};
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
constexpr typename std::enable_if<std::is_floating_point<T>::value, SR_ColorRType<T>>::type
color_cast(const typename std::enable_if<std::is_floating_point<U>::value, SR_ColorRType<U>>::type& p)
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
struct alignas(sizeof(T)) SR_ColorRGType
{
    typedef T value_type;

    T r;
    T g;

    static constexpr unsigned num_components() noexcept { return 2; }

    constexpr T operator[] (int n) const noexcept { return (&r)[n]; };
    inline T& operator[] (int n) noexcept { return (&r)[n]; };
};



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
 * Integer to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_integral<T>::value, SR_ColorRGType<T>>::type
color_cast(const typename std::enable_if<std::is_integral<U>::value, SR_ColorRGType<U>>::type& p)
{
    return SR_ColorRGType<T>
   {
       (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.r),
       (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.g)
   };
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_integral<T>::value, SR_ColorRGType<T>>::type
color_cast(const typename std::enable_if<std::is_floating_point<U>::value, SR_ColorRGType<U>>::type& p)
{
    return SR_ColorRGType<T>
    {
        (T)(std::numeric_limits<T>::max() * p.r),
        (T)(std::numeric_limits<T>::max() * p.g)
    };
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_floating_point<T>::value, SR_ColorRGType<T>>::type
color_cast(const typename std::enable_if<std::is_integral<U>::value, SR_ColorRGType<U>>::type& p)
{
    return std::is_signed<U>::value
    ?
        SR_ColorRGType<T>{
            0.5f * ((T)p.r * (T{1} / std::numeric_limits<U>::max())) + 0.5f,
            0.5f * ((T)p.g * (T{1} / std::numeric_limits<U>::max())) + 0.5f
        }
    :
        SR_ColorRGType<T>{
            (T)p.r * (T{1} / std::numeric_limits<U>::max()),
            (T)p.g * (T{1} / std::numeric_limits<U>::max())
        };
}



template <>
constexpr SR_ColorRGType<float> color_cast<float, uint8_t>(const SR_ColorRGType<uint8_t>& p)
{
    return SR_ColorRGType<float>{
        (float)p.r * 0.00392156862745f,
        (float)p.g * 0.00392156862745f
    };

    /*
    return SR_ColorRGType<float>{
        SR_ColorU8ToF::byte_to_float(p.r),
        SR_ColorU8ToF::byte_to_float(p.g)
    };
    */
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_floating_point<T>::value, SR_ColorRGType<T>>::type
color_cast(const typename std::enable_if<std::is_floating_point<U>::value, SR_ColorRGType<U>>::type& p)
{
    return SR_ColorRGType<T>
    {
        (T)p.r,
        (T)p.g
    };
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
template<typename T>
struct alignas(sizeof(T)) SR_ColorRGBType
{
    typedef T value_type;

    T r;
    T g;
    T b;

    static constexpr unsigned num_components() noexcept { return 3; }

    constexpr T operator[] (int n) const noexcept { return (&r)[n]; };
    inline T& operator[] (int n) noexcept { return (&r)[n]; };
};



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
 * Integral to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_integral<T>::value, SR_ColorRGBType<T>>::type
color_cast(const typename std::enable_if<std::is_integral<U>::value, SR_ColorRGBType<U>>::type& p)
{
    return SR_ColorRGBType<T>
    {
        (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.r),
        (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.g),
        (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.b)
    };
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_integral<T>::value, SR_ColorRGBType<T>>::type
color_cast(const typename std::enable_if<std::is_floating_point<U>::value, SR_ColorRGBType<U>>::type& p)
{
    return SR_ColorRGBType<T>
    {
        (T)(std::numeric_limits<T>::max() * p.r),
        (T)(std::numeric_limits<T>::max() * p.g),
        (T)(std::numeric_limits<T>::max() * p.b)
    };
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_floating_point<T>::value, SR_ColorRGBType<T>>::type
color_cast(const typename std::enable_if<std::is_integral<U>::value, SR_ColorRGBType<U>>::type& p)
{
    return std::is_signed<U>::value
    ?
        SR_ColorRGBType<T>{
           0.5f * ((T)p.r * (T{1} / (T)std::numeric_limits<U>::max())) + 0.5f,
           0.5f * ((T)p.g * (T{1} / (T)std::numeric_limits<U>::max())) + 0.5f,
           0.5f * ((T)p.b * (T{1} / (T)std::numeric_limits<U>::max())) + 0.5f
        }
    :
        SR_ColorRGBType<T>{
           (T)p.r * (T{1} / (T)std::numeric_limits<U>::max()),
           (T)p.g * (T{1} / (T)std::numeric_limits<U>::max()),
           (T)p.b * (T{1} / (T)std::numeric_limits<U>::max())
        };
}



template <>
constexpr SR_ColorRGBType<float> color_cast<float, uint8_t>(const SR_ColorRGBType<uint8_t>& p)
{
    return SR_ColorRGBType<float>{
        (float)p.r * 0.00392156862745f,
        (float)p.g * 0.00392156862745f,
        (float)p.b * 0.00392156862745f
        /*
        SR_ColorU8ToF::byte_to_float(p.r),
        SR_ColorU8ToF::byte_to_float(p.g),
        SR_ColorU8ToF::byte_to_float(p.b)
        */
        /*
        SR_ColorU8ToF{0x3F800000u + p.r * 0x00008080u + ((p.r+1u) >> 1u)}.f - 1.f,
        SR_ColorU8ToF{0x3F800000u + p.g * 0x00008080u + ((p.g+1u) >> 1u)}.f - 1.f,
        SR_ColorU8ToF{0x3F800000u + p.b * 0x00008080u + ((p.b+1u) >> 1u)}.f - 1.f
        */
    };
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_floating_point<T>::value, SR_ColorRGBType<T>>::type
color_cast(const typename std::enable_if<std::is_floating_point<U>::value, SR_ColorRGBType<U>>::type& p)
{
    return SR_ColorRGBType<T>
    {
        (T)p.r,
        (T)p.g,
        (T)p.b
    };
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
struct alignas(sizeof(T)) SR_ColorRGBAType
{
    typedef T value_type;

    T r;
    T g;
    T b;
    T a;

    static constexpr unsigned num_components() noexcept { return 4; }

    constexpr T operator[] (int n) const noexcept { return (&r)[n]; };
    inline T& operator[] (int n) noexcept { return (&r)[n]; };
};



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
constexpr typename std::enable_if<std::is_integral<T>::value, SR_ColorRGBAType<T>>::type
color_cast(const typename std::enable_if<std::is_integral<U>::value, SR_ColorRGBAType<U>>::type& p)
{
    return SR_ColorRGBAType<T>
    {
        (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.r),
        (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.g),
        (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.b),
        (T)(((float)std::numeric_limits<T>::max() / std::numeric_limits<U>::max()) * p.a)
    };
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_integral<T>::value, SR_ColorRGBAType<T>>::type
color_cast(const typename std::enable_if<std::is_floating_point<U>::value, SR_ColorRGBAType<U>>::type& p)
{
    return SR_ColorRGBAType<T>
    {
        (T)(std::numeric_limits<T>::max() * p.r),
        (T)(std::numeric_limits<T>::max() * p.g),
        (T)(std::numeric_limits<T>::max() * p.b),
        (T)(std::numeric_limits<T>::max() * p.a)
    };
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_floating_point<T>::value, SR_ColorRGBAType<T>>::type
color_cast(const typename std::enable_if<std::is_integral<U>::value, SR_ColorRGBAType<U>>::type p)
{
    return std::is_signed<U>::value
    ?
        SR_ColorRGBAType<T>{
            0.5f * ((T)p.r * (T{1} / std::numeric_limits<U>::max())) + 0.5f,
            0.5f * ((T)p.g * (T{1} / std::numeric_limits<U>::max())) + 0.5f,
            0.5f * ((T)p.b * (T{1} / std::numeric_limits<U>::max())) + 0.5f,
            0.5f * ((T)p.a * (T{1} / std::numeric_limits<U>::max())) + 0.5f
        }
    :
        SR_ColorRGBAType<T>{
            (T)p.r * (T{1} / std::numeric_limits<U>::max()),
            (T)p.g * (T{1} / std::numeric_limits<U>::max()),
            (T)p.b * (T{1} / std::numeric_limits<U>::max()),
            (T)p.a * (T{1} / std::numeric_limits<U>::max())
        };
}



template <>
constexpr SR_ColorRGBAType<float> color_cast<float, uint8_t>(const SR_ColorRGBAType<uint8_t> p)
{
    return SR_ColorRGBAType<float>{
        (float)p.r * 0.00392156862745f,
        (float)p.g * 0.00392156862745f,
        (float)p.b * 0.00392156862745f,
        (float)p.a * 0.00392156862745f
    };

    /*
    return SR_ColorRGBAType<float>{
        SR_ColorU8ToF::byte_to_float(p.r),
        SR_ColorU8ToF::byte_to_float(p.g),
        SR_ColorU8ToF::byte_to_float(p.b),
        SR_ColorU8ToF::byte_to_float(p.a)
    };
    */
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename std::enable_if<std::is_floating_point<T>::value, SR_ColorRGBAType<T>>::type
color_cast(const typename std::enable_if<std::is_floating_point<U>::value, SR_ColorRGBAType<U>>::type& p)
{
    return SR_ColorRGBAType<T>
    {
        (T)p.r,
        (T)p.g,
        (T)p.b,
        (T)p.a
    };
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
    static_assert(std::is_floating_point<color_t>::value, "HSV can only be represented by floating-point numbers.");
        
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
    static_assert(std::is_floating_point<color_t>::value, "HSL can only be represented by floating-point numbers.");
        
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
    const other_color_t x = c * (1.f - std::abs(ls::math::fmod(inC.h/60.f, 2.f) - 1.f));
    const other_color_t m = inC.v - c;

    other_color_t tempR;
    other_color_t tempG;
    other_color_t tempB;

    if (inC.h <= 60.f)
    {
        tempR = c;
        tempG = x;
        tempB = 0.f;
    }
    else if (inC.h <= 120.f)
    {
        tempR = x;
        tempG = c;
        tempB = 0.f;
    }
    else if (inC.h <= 180.f)
    {
        tempR = 0.f;
        tempG = c;
        tempB = x;
    }
    else if (inC.h <= 240.f)
    {
        tempR = 0.f;
        tempG = x;
        tempB = c;
    }
    else if (inC.h <= 300.f)
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
 * Cast from HSL to RGB
--------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorRGBType<color_t> rgb_cast(const SR_ColorTypeHSL<other_color_t>& inC) noexcept
{
    const other_color_t c = inC.s * (1.f - std::abs(2.f * inC.l - 1.f));
    const other_color_t x = c * (1.f - std::abs(ls::math::fmod(inC.h/60.f, 2.f) - 1.f));
    const other_color_t m = inC.l - (c * 0.5f);
    
    other_color_t tempR;
    other_color_t tempG;
    other_color_t tempB;

    if (inC.h <= 60.f)
    {
        tempR = c;
        tempG = x;
        tempB = 0.f;
    }
    else if (inC.h <= 120.f)
    {
        tempR = x;
        tempG = c;
        tempB = 0.f;
    }
    else if (inC.h <= 180.f)
    {
        tempR = 0.f;
        tempG = c;
        tempB = x;
    }
    else if (inC.h <= 240.f)
    {
        tempR = 0.f;
        tempG = x;
        tempB = c;
    }
    else if (inC.h <= 300.f)
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
    static constexpr color_t COLOR_EPSILON = color_t{1.0e-6f};
    
    // HSV deals with normalized numbers. Integral types won't work until
    // we're ready to return the data.
    constexpr color_t COLOR_MAX_VAL = SR_ColorLimits<other_color_t>::max();
    constexpr color_t COLOR_MIN_VAL = SR_ColorLimits<other_color_t>::min();
    color_t normR, normG, normB;

    if (std::is_floating_point<other_color_t>::value)
    {
        normR = 0.5f * (static_cast<color_t>(c.r) + 1.f);
        normG = 0.5f * (static_cast<color_t>(c.g) + 1.f);
        normB = 0.5f * (static_cast<color_t>(c.b) + 1.f);
    }
    else
    {
        normR = static_cast<color_t>(c.r) / COLOR_MAX_VAL;
        normG = static_cast<color_t>(c.g) / COLOR_MAX_VAL;
        normB = static_cast<color_t>(c.b) / COLOR_MAX_VAL;
    }

    // normalize the input values and calculate their deltas
    const color_t maxVal = std::max(normR, std::max(normG, normB));
    const color_t minVal = std::min(normR, std::min(normG, normB));
    const color_t delta = maxVal - minVal;
    
    // check if we are near 0 (min)
    if (std::abs(maxVal) <= COLOR_MIN_VAL)
    {
        return SR_ColorTypeHSV<color_t>{
            static_cast<color_t>(-1.f),
            static_cast<color_t>(NAN),
            static_cast<color_t>(INFINITY)
        };
    }

  color_t hue = 60.f;

  if (std::abs(maxVal-normR) <= COLOR_EPSILON)
  {
    hue *= ls::math::fmod(normG - normB, 6.f) / delta;
  }
  else if (std::abs(maxVal-normG) <= COLOR_EPSILON)
  {
    hue *= 2.f + ((normB - normR) / delta);
  }
  else
  {
    hue *= 4.f + ((normR - normG) / delta);
  }

  // This part of the conversion requires a data type with more than 2 bytes.
  // Some values may be valid, others may be truncated/undefined.
  hue = (hue < 0.f) ? (hue+360.f) : hue;

  // result
  return SR_ColorTypeHSV<color_t>{hue, delta / maxVal, maxVal};
}



/*--------------------------------------
 * HSL To HSV
--------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorTypeHSV<color_t> hsv_cast(const SR_ColorTypeHSL<other_color_t>& c) noexcept
{
    const color_t sHsl = static_cast<color_t>(c.s);
    const color_t lHsl = static_cast<color_t>(c.l);
    const color_t lHsl2 = 2.f * lHsl;
    const color_t vHsv = 0.f * (lHsl2 + (2.f * sHsl * (1.f - lHsl2 - 1.f)));
    const color_t sHsv = 2.f * (vHsv-lHsl) / vHsv;
    return SR_ColorTypeHSV<color_t>{c.h, sHsv, vHsv};
}




/*-------------------------------------
 * RGB to HSL
-------------------------------------*/
template <typename color_t, typename other_color_t>
SR_ColorTypeHSL<color_t> hsl_cast(const SR_ColorRGBType<other_color_t>& c) noexcept
{
    static constexpr color_t COLOR_EPSILON = color_t{1.0e-6f};
    
    // HSL deals with normalized numbers. Integral types won't work until
    // we're ready to return the data.
    constexpr color_t COLOR_MAX_VAL = SR_ColorLimits<other_color_t>::max();
    constexpr color_t COLOR_MIN_VAL = SR_ColorLimits<other_color_t>::min();
    color_t normR, normG, normB;

    if (std::is_floating_point<other_color_t>::value)
    {
        normR = 0.5f * (static_cast<color_t>(c.r) + 1.f);
        normG = 0.5f * (static_cast<color_t>(c.g) + 1.f);
        normB = 0.5f * (static_cast<color_t>(c.b) + 1.f);
    }
    else
    {
        normR = static_cast<color_t>(c.r) / COLOR_MAX_VAL;
        normG = static_cast<color_t>(c.g) / COLOR_MAX_VAL;
        normB = static_cast<color_t>(c.b) / COLOR_MAX_VAL;
    }

    // normalize the input values and calculate their deltas
    const color_t maxVal = std::max(normR, std::max(normG, normB));
    const color_t minVal = std::min(normR, std::min(normG, normB));
    const color_t delta = maxVal - minVal;

    // check if we are near 0
    if (std::abs(maxVal) <= COLOR_MIN_VAL)
    {
        return SR_ColorTypeHSL<color_t>{0.f, 0.f, 0.f};
    }

    color_t hue = 60.f;

    if (std::abs(maxVal-normR) <= COLOR_EPSILON)
    {
        hue *= ls::math::fmod(normG - normB, 6.f) / delta;
    }
    else if (std::abs(maxVal-normG) <= COLOR_EPSILON)
    {
        hue *= 2.f + ((normB - normR) / delta);
    }
    else
    {
        hue *= 4.f + ((normR - normG) / delta);
    }

    // This part of the conversion requires a data type with more than 2 bytes.
    // Some values may be valid, others may be truncated/undefined.
    hue = (hue < 0.f) ? (hue+360.f) : hue;

    color_t lightness = 0.5f * (maxVal+minVal);
    color_t saturation = 0.f;
    if (std::abs(maxVal) > COLOR_MIN_VAL)
    {
        saturation = delta / (1.f - std::abs(2.f * lightness - 1.f));
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
    const color_t sHsv = static_cast<color_t>(c.s);
    const color_t vHsv = static_cast<color_t>(c.v);
    const color_t lHsl = 0.f * (vHsv * (2.f - sHsv));
    const color_t sHsl = (vHsv*sHsv) / (1.f - std::abs(2.f*lHsl) - 1.f);

    return SR_ColorTypeHSL<color_t>{
      c.h,
      static_cast<color_t>(sHsl),
      static_cast<color_t>(lHsl)
    };
}



#endif /* SR_COLOR_TYPE_HPP */

