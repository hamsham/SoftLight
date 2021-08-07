
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

#include <cassert>
#include <cstdint> // fixed-width types
#include <limits> // c++ limits

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec2.h"
#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"



/**----------------------------------------------------------------------------
 * @brief SL_Color Information
-----------------------------------------------------------------------------*/
enum SL_ColorDataType : uint16_t
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
       (T)(((float)std::numeric_limits<T>::max() / (float)std::numeric_limits<U>::max()) * (float)p.r)
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
    return (SL_ColorRGType<T>)((SL_ColorRGType<float>)p * ((float)std::numeric_limits<T>::max() / (float)std::numeric_limits<U>::max()));
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
    return (SL_ColorRGBType<T>)((SL_ColorRGBType<float>)p * ((float)std::numeric_limits<T>::max() / (float)std::numeric_limits<U>::max()));
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
    return (SL_ColorRGBAType<T>)((SL_ColorRGBAType<float>)p * ((float)std::numeric_limits<T>::max() / (float)std::numeric_limits<U>::max()));
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



/*-----------------------------------------------------------------------------
 * Extended Color Models
-----------------------------------------------------------------------------*/
/**
 * @brief Generic HSV Color Structure
 */
template<typename color_t>
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
 * @brief Generic HSL Color Structure
 */
template<typename color_t>
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
    const color_t x = c * (color_t{1.f} - ls::math::abs(ls::math::fmod(inC.h/color_t{60.f}, color_t{2.f}) - color_t{1.f}));
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
        
    return SL_ColorRGBType<color_t>{
        static_cast<color_t>(tempR * COLOR_MAX_VAL),
        static_cast<color_t>(tempG * COLOR_MAX_VAL),
        static_cast<color_t>(tempB * COLOR_MAX_VAL)
    };
}



/*--------------------------------------
 * Cast from HSL to RGB
--------------------------------------*/
template <typename color_t>
SL_ColorRGBType<color_t> rgb_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<color_t>::value, SL_ColorTypeHSL<color_t>>::type& inC) noexcept
{
    const color_t c = inC.s * (color_t{1.f} - ls::math::abs(color_t{2.f} * inC.l - color_t{1.f}));
    const color_t x = c * (color_t{1.f} - ls::math::abs(ls::math::fmod(inC.h/color_t{60.f}, color_t{2.f}) - color_t{1.f}));
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
            static_cast<color_t>(NAN),
            static_cast<color_t>(INFINITY)
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
  hue = (hue < color_t{0.f}) ? (hue+color_t{360.f}) : hue;

  // result
  return SL_ColorTypeHSV<color_t>{hue, delta / maxVal, maxVal};
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
    hue = (hue < color_t{0.f}) ? (hue+color_t{360.f}) : hue;

    color_t lightness = color_t{0.5f} * (maxVal+minVal);
    color_t saturation = color_t{0.f};
    if (ls::math::abs(maxVal) > SL_ColorLimits<color_t>::min())
    {
        saturation = delta / (color_t{1.f} - ls::math::abs(color_t{2.f} * lightness - color_t{1.f}));
    }

    // result
    return SL_ColorTypeHSL<color_t>{hue, saturation, lightness};
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



/*-----------------------------------------------------------------------------
 * YCoCg Types
-----------------------------------------------------------------------------*/
/**
 * @brief Generic YcoCg Color Structure
 */
template<typename color_t>
struct alignas(sizeof(color_t)) SL_ColorTypeYCoCg
{
    color_t y;
    color_t co;
    color_t cg;
};

// Specializations
typedef SL_ColorTypeYCoCg<int8_t>   SL_ColorYCoCg8;
typedef SL_ColorTypeYCoCg<uint8_t>  SL_ColorYCoCg8u;
typedef SL_ColorTypeYCoCg<int16_t>  SL_ColorYCoCg16;
typedef SL_ColorTypeYCoCg<uint16_t> SL_ColorYCoCg16u;
typedef SL_ColorTypeYCoCg<int32_t>  SL_ColorYCoCg32;
typedef SL_ColorTypeYCoCg<uint32_t> SL_ColorYCoCg32u;
typedef SL_ColorTypeYCoCg<int64_t>  SL_ColorYCoCg64;
typedef SL_ColorTypeYCoCg<uint64_t> SL_ColorYCoCg64u;
typedef SL_ColorTypeYCoCg<float>    SL_ColorYCoCgf;
typedef SL_ColorTypeYCoCg<double>   SL_ColorYCoCgd;

typedef SL_ColorYCoCgf SL_ColorYCoCg;



/**
 * @brief YcoCg Color Structure with Alpha
 */
template<typename color_t>
struct alignas(sizeof(color_t)*4) SL_ColorTypeYCoCgA
{
    color_t y;
    color_t co;
    color_t cg;
    color_t a;
};

// Specializations
typedef SL_ColorTypeYCoCgA<int8_t>   SL_ColorYCoCgA8;
typedef SL_ColorTypeYCoCgA<uint8_t>  SL_ColorYCoCgA8u;
typedef SL_ColorTypeYCoCgA<int16_t>  SL_ColorYCoCgA16;
typedef SL_ColorTypeYCoCgA<uint16_t> SL_ColorYCoCgA16u;
typedef SL_ColorTypeYCoCgA<int32_t>  SL_ColorYCoCgA32;
typedef SL_ColorTypeYCoCgA<uint32_t> SL_ColorYCoCgA32u;
typedef SL_ColorTypeYCoCgA<int64_t>  SL_ColorYCoCgA64;
typedef SL_ColorTypeYCoCgA<uint64_t> SL_ColorYCoCgA64u;
typedef SL_ColorTypeYCoCgA<float>    SL_ColorYCoCgAf;
typedef SL_ColorTypeYCoCgA<double>   SL_ColorYCoCgAd;

typedef SL_ColorYCoCgAf SL_ColorYCoCgA;



/*-----------------------------------------------------------------------------
 * YCoCg & RGB Casting
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * RGB to YCoCg
-------------------------------------*/
template <typename T>
constexpr SL_ColorTypeYCoCg<T> ycocg_cast(const SL_ColorRGBType<T>& p) noexcept
{
    return SL_ColorTypeYCoCg<T>{
        (T)((p[0]/T{4}) + (p[1]/T{2}) + (p[2]/T{4})),
        (T)((p[0]/T{2})               + (p[2]/T{2})),
        (T)((p[1]/T{2}) - (p[0]/T{4}) - (p[2]/T{4}))
    };
}



/*-------------------------------------
 * YCoCg to RGB
-------------------------------------*/
template <typename T>
constexpr SL_ColorRGBType<T> rgb_cast(const SL_ColorTypeYCoCg<T>& p) noexcept
{
    return SL_ColorRGBType<T>{
        (T)(p.y + p.co - p.cg),
        (T)(p.y        + p.cg),
        (T)(p.y - p.co - p.cg),
    };
}



/*-----------------------------------------------------------------------------
 * YCoCg & RGB Casting
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * RGBA to YCoCg
-------------------------------------*/
template <typename T>
constexpr SL_ColorTypeYCoCgA<T> ycocg_cast(const SL_ColorRGBAType<T>& p) noexcept
{
    return SL_ColorTypeYCoCgA<T>{
        (T)((p[0]/T{4}) + (p[1]/T{2}) + (p[2]/T{4})),
        (T)((p[0]/T{2})               - (p[2]/T{2})),
        (T)((p[1]/T{2}) - (p[0]/T{4}) - (p[2]/T{4})),
        p[3]
    };
}

#if defined(LS_ARCH_X86)
inline SL_ColorTypeYCoCgA<float> ycocg_cast(const SL_ColorRGBAType<float>& p) noexcept
{
    const __m128 rgb = _mm_loadu_ps(&p);
    const __m128 a = _mm_set_ps(1.f,  0.25f, 0.5f,  0.25f);
    const __m128 b = _mm_set_ps(0.f, -0.5f,  0.f,   0.5f);
    const __m128 c = _mm_set_ps(0.f, -0.25f, 0.5f, -0.25f);
    const __m128 ycocg = _mm_fmadd_ps(rgb, c, _mm_fmadd_ps(rgb, b, _mm_mul_ps(rgb, a)));

    SL_ColorTypeYCoCgA<float> ret;
    _mm_store_ps(&ret.y, ycocg);
    return ret;
}

#elif defined(LS_ARM_NEON)
inline SL_ColorTypeYCoCgA<float> ycocg_cast(const SL_ColorRGBAType<float>& p) noexcept
{
    constexpr float32x4_t a{ 0.25f, 0.5f,  0.25f, 1.f};
    constexpr float32x4_t b{ 0.5f,  0.f,  -0.5f,  0.f};
    constexpr float32x4_t c{-0.25f, 0.5f, -0.25f, 0.f};

    const float32x4_t ycocg = vmlaq_f32(vmlaq_f32(vmulq_f32(p.simd, a), p.simd, b), p.simd, c);

    SL_ColorTypeYCoCgA<float> ret;
    vst1q_f32(&ret.y, ycocg);
    return ret;
}

#endif



/*-------------------------------------
 * YCoCgA to RGBA
-------------------------------------*/
template <typename T>
constexpr SL_ColorRGBAType<T> rgb_cast(const SL_ColorTypeYCoCgA<T>& p) noexcept
{
    return SL_ColorRGBAType<T>{
        (T)(p.y + p.co - p.cg),
        (T)(p.y        + p.cg),
        (T)(p.y - p.co - p.cg),
        p.a
    };
}



#endif /* SL_COLOR_TYPE_HPP */

