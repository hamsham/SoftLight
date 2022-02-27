
#ifndef SL_COLOR_TYPE_HPP
#define SL_COLOR_TYPE_HPP

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

#include <cstdint> // fixed-width types
#include <limits> // c++ limits

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec2.h"
#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"



/**----------------------------------------------------------------------------
 * @brief SL_Color Information
-----------------------------------------------------------------------------*/
enum SL_ColorDataType : uint8_t
{
    SL_COLOR_R_8U,
    SL_COLOR_R_16U,
    SL_COLOR_R_32U,
    SL_COLOR_R_64U,
    SL_COLOR_R_FLOAT,
    SL_COLOR_R_DOUBLE,

    SL_COLOR_RG_8U,
    SL_COLOR_RG_16U,
    SL_COLOR_RG_32U,
    SL_COLOR_RG_64U,
    SL_COLOR_RG_FLOAT,
    SL_COLOR_RG_DOUBLE,

    SL_COLOR_RGB_8U,
    SL_COLOR_RGB_16U,
    SL_COLOR_RGB_32U,
    SL_COLOR_RGB_64U,
    SL_COLOR_RGB_FLOAT,
    SL_COLOR_RGB_DOUBLE,

    SL_COLOR_RGBA_8U,
    SL_COLOR_RGBA_16U,
    SL_COLOR_RGBA_32U,
    SL_COLOR_RGBA_64U,
    SL_COLOR_RGBA_FLOAT,
    SL_COLOR_RGBA_DOUBLE,

    SL_COLOR_RGB_DEFAULT = SL_COLOR_RGB_8U,
    SL_COLOR_INVALID
};



/*-------------------------------------
 * Number of bytes per color
-------------------------------------*/
size_t sl_bytes_per_color(SL_ColorDataType p);



/*-------------------------------------
 * Number of elements per color
-------------------------------------*/
unsigned sl_elements_per_color(SL_ColorDataType p);



/*-------------------------------------
 * Helper Structure to Convert 8-bit colors to float
-------------------------------------*/
union SL_ColorU8ToF
{
    uint32_t i;
    float f;

    static constexpr float byte_to_float(const uint8_t c) noexcept
    {
        return SL_ColorU8ToF{0x3F800000u + c * 0x00008080u + ((c+1u) >> 1u)}.f - 1.f;
    }
};



/**
 * @brief Enumeration to describe the color model being used by a SL_ColorType<>
 * object.
 */
enum class SL_ColorModelType
{
    SL_COLOR_MODEL_RGB,
    SL_COLOR_MODEL_HSV,
    SL_COLOR_MODEL_HSL
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
struct SL_ColorLimits
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
struct SL_ColorLimits<ls::math::half>
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
struct alignas(sizeof(T)) SL_ColorRType
{
    typedef T value_type;

    T r;

    static constexpr unsigned num_components() noexcept { return 1; }

    constexpr SL_ColorRType() noexcept = default;
    constexpr SL_ColorRType(const SL_ColorRType& c ) noexcept = default;
    constexpr SL_ColorRType(SL_ColorRType&& c ) noexcept = default;

    explicit constexpr SL_ColorRType(T n) noexcept : r{n} {}

    template <typename C>
    explicit constexpr SL_ColorRType(C n) noexcept : r{(T)n} {}

    inline SL_ColorRType& operator=(const SL_ColorRType& c ) noexcept = default;
    inline SL_ColorRType& operator=(SL_ColorRType&& c ) noexcept = default;
    inline SL_ColorRType& operator=(const T n) noexcept { r = n; return *this; }

    explicit constexpr operator T() const noexcept { return r; }

    template <typename C>
    explicit constexpr operator C() const noexcept { return (C)r; }

    template <typename C>
    explicit constexpr operator SL_ColorRType<C>() const noexcept { return SL_ColorRType{(C)r}; }

    template <typename index_t>
    constexpr T operator[] (index_t n) const noexcept { return (&r)[n]; }

    template <typename index_t>
    inline T& operator[] (index_t n) noexcept { return (&r)[n]; }

    constexpr SL_ColorRType operator+(const SL_ColorRType<T>& n) const noexcept { return SL_ColorRType{r + n.r}; }
    constexpr SL_ColorRType operator*(const SL_ColorRType<T>& n) const noexcept { return SL_ColorRType{r * n.r}; }

    constexpr SL_ColorRType operator+(const T n) const noexcept { return SL_ColorRType{r + n}; }
    constexpr SL_ColorRType operator*(const T n) const noexcept { return SL_ColorRType{r * n}; }
};



/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRType<uint8_t>  SL_ColorR8;
typedef SL_ColorRType<uint16_t> SL_ColorR16;
typedef SL_ColorRType<uint32_t> SL_ColorR32;
typedef SL_ColorRType<uint64_t> SL_ColorR64;
typedef SL_ColorRType<float>    SL_ColorRf;
typedef SL_ColorRType<double>   SL_ColorRd;

typedef SL_ColorR8 SL_ColorR;



/*-------------------------------------
 * Data type casting
-------------------------------------*/
/*-------------------------------------
 * Integer to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRType<U>>::type& p)
{
    return SL_ColorRType<T>
   {
        ((uint64_t)std::numeric_limits<T>::max() > (uint64_t)std::numeric_limits<U>::max())
            ? ((T)p.r * (std::numeric_limits<T>::max() / (T)std::numeric_limits<U>::max()))
            : (T)(p.r / (std::numeric_limits<U>::max() / (U)std::numeric_limits<T>::max()))
   };
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRType<U>>::type& p)
{
    return (SL_ColorRType<T>)(p.r * (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
    ?
        SL_ColorRType<T>{T{0.5} * ((T)p.r * (T{1} / (T)std::numeric_limits<U>::max())) + T{0.5}}
    :
        SL_ColorRType<T>{(T)p.r * (T{1} / (T)std::numeric_limits<U>::max())};
}



template <>
constexpr SL_ColorRType<float> color_cast<float, uint8_t>(const SL_ColorRType<uint8_t>& p)
{
    return SL_ColorRType<float>{(float)p.r * 0.00392156862745f};
    //return SL_ColorRType<float>{SL_ColorU8ToF::byte_to_float(p.r)};
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRType<U>>::type& p)
{
    return SL_ColorRType<T>{(T)p.r};
}



/*-------------------------------------
 * Data type casting (no-op)
-------------------------------------*/
template <typename T, T>
constexpr SL_ColorRType<T> color_cast(const SL_ColorRType<T>& p)
{
    return p;
}



/**----------------------------------------------------------------------------
 * @brief RG Color Type
-----------------------------------------------------------------------------*/
template <typename T>
using SL_ColorRGType = ls::math::vec2_t<T>;


/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRGType<uint8_t>  SL_ColorRG8;
typedef SL_ColorRGType<uint16_t> SL_ColorRG16;
typedef SL_ColorRGType<uint32_t> SL_ColorRG32;
typedef SL_ColorRGType<uint64_t> SL_ColorRG64;
typedef SL_ColorRGType<float>    SL_ColorRGf;
typedef SL_ColorRGType<double>   SL_ColorRGd;

typedef SL_ColorRG8 SL_ColorRG;



/*-------------------------------------
 * Data type casting
-------------------------------------*/
/*-------------------------------------
 * Integral to integral
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGType<U>>::type& p)
{
    return ((uint64_t)std::numeric_limits<T>::max() > (uint64_t)std::numeric_limits<U>::max())
        ? ((SL_ColorRGType<T>)p * std::numeric_limits<T>::max())
        : (SL_ColorRGType<T>)(p / (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGType<U>>::type& p)
{
    return (SL_ColorRGType<T>)(p * (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
           ? (SL_ColorRGType<T>{0.5f} * ((SL_ColorRGType<T>)p * (T{1} / (float)std::numeric_limits<U>::max())) + SL_ColorRGType<T>{0.5f})
           : ((SL_ColorRGType<T>)p * (T{1} / (float)std::numeric_limits<U>::max()));
}



template <>
inline SL_ColorRGType<float> color_cast<float, uint8_t>(const SL_ColorRGType<uint8_t>& p)
{
    return ((SL_ColorRGType<float>)p) * 0.00392156862745f;
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGType<U>>::type& p)
{
    return (SL_ColorRGType<T>)p;
}



/*-------------------------------------
 * Data type casting (no-op)
-------------------------------------*/
template <typename T, T>
constexpr SL_ColorRGType<T> color_cast(const SL_ColorRGType<T>& p)
{
    return p;
}



/**----------------------------------------------------------------------------
 * @brief Generic RGB Color Structure
-----------------------------------------------------------------------------*/
template <typename T>
using SL_ColorRGBType = ls::math::vec3_t<T>;


/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRGBType<uint8_t>  SL_ColorRGB8;
typedef SL_ColorRGBType<uint16_t> SL_ColorRGB16;
typedef SL_ColorRGBType<uint32_t> SL_ColorRGB32;
typedef SL_ColorRGBType<uint64_t> SL_ColorRGB64;
typedef SL_ColorRGBType<float>    SL_ColorRGBf;
typedef SL_ColorRGBType<double>   SL_ColorRGBd;

typedef SL_ColorRGB8 SL_ColorRGB;



/*-------------------------------------
 * Data type casting
-------------------------------------*/
/*-------------------------------------
 * Integral to integral
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBType<U>>::type& p)
{
    return ((uint64_t)std::numeric_limits<T>::max() > (uint64_t)std::numeric_limits<U>::max())
        ? ((SL_ColorRGBType<T>)p * (std::numeric_limits<T>::max() / (T)std::numeric_limits<U>::max()))
        : (SL_ColorRGBType<T>)(p / (std::numeric_limits<U>::max() / (U)std::numeric_limits<T>::max()));
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBType<U>>::type& p)
{
    return (SL_ColorRGBType<T>)(p * (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
           ? (SL_ColorRGBType<T>{0.5f} * ((SL_ColorRGBType<T>)p * (T{1} / (T)std::numeric_limits<U>::max())) + SL_ColorRGBType<T>{0.5f})
           : ((SL_ColorRGBType<T>)p * (T{1} / (T)std::numeric_limits<U>::max()));
}



template <>
inline SL_ColorRGBType<float> color_cast<float, uint8_t>(const SL_ColorRGBType<uint8_t>& p)
{
    return ((SL_ColorRGBType<float>)p) * 0.00392156862745f;
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBType<U>>::type& p)
{
    return (SL_ColorRGBType<T>)p;
}



/*-------------------------------------
 * Data type casting (no-op)
-------------------------------------*/
template <typename T, T>
constexpr SL_ColorRGBType<T> color_cast(const SL_ColorRGBType<T>& p)
{
    return p;
}



/**----------------------------------------------------------------------------
 * @brief RGBA Color Types
-----------------------------------------------------------------------------*/
template <typename T>
using SL_ColorRGBAType = ls::math::vec4_t<T>;


/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRGBAType<uint8_t>  SL_ColorRGBA8;
typedef SL_ColorRGBAType<uint16_t> SL_ColorRGBA16;
typedef SL_ColorRGBAType<uint32_t> SL_ColorRGBA32;
typedef SL_ColorRGBAType<uint64_t> SL_ColorRGBA64;
typedef SL_ColorRGBAType<float>    SL_ColorRGBAf;
typedef SL_ColorRGBAType<double>   SL_ColorRGBAd;

typedef SL_ColorRGBA8 SL_ColorRGBA;



/*-------------------------------------
 * Data type casting
-------------------------------------*/
/*-------------------------------------
 * Integral to integral
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBAType<U>>::type& p)
{
    return ((uint64_t)std::numeric_limits<T>::max() > (uint64_t)std::numeric_limits<U>::max())
        ? ((SL_ColorRGBAType<T>)p * (std::numeric_limits<T>::max() / (T)std::numeric_limits<U>::max()))
        : (SL_ColorRGBAType<T>)(p / (std::numeric_limits<U>::max() / (U)std::numeric_limits<T>::max()));
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBAType<U>>::type& p)
{
    return (SL_ColorRGBAType<T>)(p * (U)std::numeric_limits<T>::max());
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value, SL_ColorRGBAType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
    ? (SL_ColorRGBAType<T>{0.5f} * ((SL_ColorRGBAType<T>)p * (T{1} / (T)std::numeric_limits<U>::max())) + SL_ColorRGBAType<T>{0.5f})
    : ((SL_ColorRGBAType<T>)p * (T{1} / (T)std::numeric_limits<U>::max()));
}



template <>
inline SL_ColorRGBAType<float> color_cast<float, uint8_t>(const SL_ColorRGBAType<uint8_t>& p)
{
    return ((SL_ColorRGBAType<float>)p) * 0.00392156862745f;
}



/*-------------------------------------
 * Float to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value, SL_ColorRGBAType<U>>::type& p)
{
    return (SL_ColorRGBAType<T>)p;
}



/*-------------------------------------
 * Data type casting (no-op)
-------------------------------------*/
template <typename T, T>
constexpr SL_ColorRGBAType<T> color_cast(const SL_ColorRGBAType<T>& p)
{
    return p;
}



/*-----------------------------------------------------------------------------
 * General Conversion for standard types (lossy).
-----------------------------------------------------------------------------*/
struct SL_GeneralColor
{
    SL_ColorDataType type;

    union
    {
        SL_ColorRType<uint8_t> r8;
        SL_ColorRType<uint16_t> r16;
        SL_ColorRType<uint32_t> r32;
        SL_ColorRType<uint64_t> r64;
        SL_ColorRType<float> rf;
        SL_ColorRType<double> rd;

        ls::math::vec2_t<uint8_t> rg8;
        ls::math::vec2_t<uint16_t> rg16;
        ls::math::vec2_t<uint32_t> rg32;
        ls::math::vec2_t<uint64_t> rg64;
        ls::math::vec2_t<float> rgf;
        ls::math::vec2_t<double> rgd;

        ls::math::vec3_t<uint8_t> rgb8;
        ls::math::vec3_t<uint16_t> rgb16;
        ls::math::vec3_t<uint32_t> rgb32;
        ls::math::vec3_t<uint64_t> rgb64;
        ls::math::vec3_t<float> rgbf;
        ls::math::vec3_t<double> rgbd;

        ls::math::vec4_t<uint8_t> rgba8;
        ls::math::vec4_t<uint16_t> rgba16;
        ls::math::vec4_t<uint32_t> rgba32;
        ls::math::vec4_t<uint64_t> rgba64;
        ls::math::vec4_t<float> rgbaf;
        ls::math::vec4_t<double> rgbad;
    } color;
};

template <typename color_type>
SL_GeneralColor sl_match_color_for_type(SL_ColorDataType typeToMatch, const color_type& inColor) noexcept;

extern template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<uint8_t>>( SL_ColorDataType, const SL_ColorRType<uint8_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<uint16_t>>(SL_ColorDataType, const SL_ColorRType<uint16_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<uint32_t>>(SL_ColorDataType, const SL_ColorRType<uint32_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<uint64_t>>(SL_ColorDataType, const SL_ColorRType<uint64_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<float>>(   SL_ColorDataType, const SL_ColorRType<float>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<double>>(  SL_ColorDataType, const SL_ColorRType<double>&) noexcept;

extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<uint8_t>>( SL_ColorDataType, const ls::math::vec2_t<uint8_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<uint16_t>>(SL_ColorDataType, const ls::math::vec2_t<uint16_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<uint32_t>>(SL_ColorDataType, const ls::math::vec2_t<uint32_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<uint64_t>>(SL_ColorDataType, const ls::math::vec2_t<uint64_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<float>>(   SL_ColorDataType, const ls::math::vec2_t<float>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<double>>(  SL_ColorDataType, const ls::math::vec2_t<double>&) noexcept;

extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<uint8_t>>( SL_ColorDataType, const ls::math::vec3_t<uint8_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<uint16_t>>(SL_ColorDataType, const ls::math::vec3_t<uint16_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<uint32_t>>(SL_ColorDataType, const ls::math::vec3_t<uint32_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<uint64_t>>(SL_ColorDataType, const ls::math::vec3_t<uint64_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<float>>(   SL_ColorDataType, const ls::math::vec3_t<float>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<double>>(  SL_ColorDataType, const ls::math::vec3_t<double>&) noexcept;

extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<uint8_t>>( SL_ColorDataType, const ls::math::vec4_t<uint8_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<uint16_t>>(SL_ColorDataType, const ls::math::vec4_t<uint16_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<uint32_t>>(SL_ColorDataType, const ls::math::vec4_t<uint32_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<uint64_t>>(SL_ColorDataType, const ls::math::vec4_t<uint64_t>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<float>>(   SL_ColorDataType, const ls::math::vec4_t<float>&) noexcept;
extern template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<double>>(  SL_ColorDataType, const ls::math::vec4_t<double>&) noexcept;



#endif /* SL_COLOR_TYPE_HPP */

