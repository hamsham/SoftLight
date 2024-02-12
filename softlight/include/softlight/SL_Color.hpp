
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
    SL_COLOR_R_HALF,
    SL_COLOR_R_FLOAT,
    SL_COLOR_R_DOUBLE,

    SL_COLOR_RG_8U,
    SL_COLOR_RG_16U,
    SL_COLOR_RG_32U,
    SL_COLOR_RG_64U,
    SL_COLOR_RG_HALF,
    SL_COLOR_RG_FLOAT,
    SL_COLOR_RG_DOUBLE,

    SL_COLOR_RGB_8U,
    SL_COLOR_RGB_16U,
    SL_COLOR_RGB_32U,
    SL_COLOR_RGB_64U,
    SL_COLOR_RGB_HALF,
    SL_COLOR_RGB_FLOAT,
    SL_COLOR_RGB_DOUBLE,

    SL_COLOR_RGBA_8U,
    SL_COLOR_RGBA_16U,
    SL_COLOR_RGBA_32U,
    SL_COLOR_RGBA_64U,
    SL_COLOR_RGBA_HALF,
    SL_COLOR_RGBA_FLOAT,
    SL_COLOR_RGBA_DOUBLE,

    // These compressed formats require the inclusion of "SL_ColorCompressed.hpp"
    SL_COLOR_RGB_332,
    SL_COLOR_RGB_565,
    SL_COLOR_RGBA_5551,
    SL_COLOR_RGBA_4444,
    SL_COLOR_RGBA_1010102,

    SL_COLOR_RGB_DEFAULT = SL_COLOR_RGB_8U
};



/*-------------------------------------
 * Number of bytes per color
-------------------------------------*/
size_t sl_bytes_per_color(SL_ColorDataType p) noexcept;



/*-------------------------------------
 * Number of elements per color
-------------------------------------*/
unsigned sl_elements_per_color(SL_ColorDataType p) noexcept;



/*-------------------------------------
 * Compressed format check
-------------------------------------*/
constexpr bool sl_is_compressed_color(SL_ColorDataType p) noexcept
{
    return
        (p == SL_COLOR_RGB_332) ||
        (p == SL_COLOR_RGB_565) ||
        (p == SL_COLOR_RGBA_5551) ||
        (p == SL_COLOR_RGBA_4444) ||
        (p == SL_COLOR_RGBA_1010102);
}



/*-----------------------------------------------------------------------------
 * Internal limits of color type ranges
-----------------------------------------------------------------------------*/
/**
 * @brief Template specialization which allows for internal color calculations
 * to determine the maximum and minimum possible number ranges for certain
 * data types and their color representations.
 *
 * @tparam color_t
 * A basic C/C++ data type such as unsigned char, int, double, etc...
 */
template <typename color_t, template <typename> class SLColorType>
struct SL_ColorLimits
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr SLColorType<color_t> min() noexcept
    {
        return SLColorType<color_t>{
            ls::setup::IsFloat<color_t>::value ? static_cast<color_t>(0.0) : std::numeric_limits<color_t>::min()
        };
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 1.0.
    */
    static constexpr SLColorType<color_t> max() noexcept
    {
        return SLColorType<color_t>{
            ls::setup::IsFloat<color_t>::value ? static_cast<color_t>(1.0) : std::numeric_limits<color_t>::max()
        };
    }
};



template <template <typename> class SLColorType>
struct SL_ColorLimits<ls::math::half, SLColorType>
{
    /**
    * @brief Determine the minimum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::min(). Floating-point types will return 0.0.
    */
    static constexpr SLColorType<ls::math::half> min() noexcept
    {
        return SLColorType<ls::math::half>{ls::math::half{0x00u, 0x00u}};
    }

    /**
    * @brief Determine the maximum possible value for a color object's internal
    * data types.
    *
    * @return For integral types, the return value is equivalent to
    * std::numeric_limits<color_t>::max(). Floating-point types will return 1.0.
    */
    static constexpr SLColorType<ls::math::half> max() noexcept
    {
        return SLColorType<ls::math::half>{ls::math::half{0x3Cu, 0x00u}};
    }
};



/**----------------------------------------------------------------------------
 * @brief Red-only Color Types
-----------------------------------------------------------------------------*/
template <typename T>
struct alignas(sizeof(T)) SL_ColorRType
{
    typedef T value_type;
    static constexpr unsigned num_components() noexcept { return 1; }

    T r;

    constexpr SL_ColorRType() noexcept = default;
    constexpr SL_ColorRType(const SL_ColorRType& c) noexcept = default;
    constexpr SL_ColorRType(SL_ColorRType&& c) noexcept = default;

    explicit constexpr SL_ColorRType(T n) noexcept : r{n} {}

    template <typename C>
    explicit constexpr SL_ColorRType(C n) noexcept : r{(T)n} {}

    inline SL_ColorRType& operator=(const SL_ColorRType& c) noexcept = default;
    inline SL_ColorRType& operator=(SL_ColorRType&& c) noexcept = default;
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
    constexpr SL_ColorRType operator-(const SL_ColorRType<T>& n) const noexcept { return SL_ColorRType{r - n.r}; }
    constexpr SL_ColorRType operator*(const SL_ColorRType<T>& n) const noexcept { return SL_ColorRType{r * n.r}; }
    constexpr SL_ColorRType operator/(const SL_ColorRType<T>& n) const noexcept { return SL_ColorRType{r / n.r}; }

    constexpr SL_ColorRType operator+(const T n) const noexcept { return SL_ColorRType{r + n}; }
    constexpr SL_ColorRType operator-(const T n) const noexcept { return SL_ColorRType{r - n}; }
    constexpr SL_ColorRType operator*(const T n) const noexcept { return SL_ColorRType{r * n}; }
    constexpr SL_ColorRType operator/(const T n) const noexcept { return SL_ColorRType{r / n}; }
};



/*-------------------------------------
 * Typedef Specializations
-------------------------------------*/
typedef SL_ColorRType<uint8_t>  SL_ColorR8;
typedef SL_ColorRType<uint16_t> SL_ColorR16;
typedef SL_ColorRType<uint32_t> SL_ColorR32;
typedef SL_ColorRType<uint64_t> SL_ColorR64;
typedef SL_ColorRType<ls::math::half> SL_ColorRh;
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
        ((uint64_t)SL_ColorLimits<T, SL_ColorRType>::max().r > (uint64_t)SL_ColorLimits<U, SL_ColorRType>::max().r)
            ? ((T)p.r * (SL_ColorLimits<T, SL_ColorRType>::max().r / (T)SL_ColorLimits<U, SL_ColorRType>::max().r))
            : (T)(p.r / (SL_ColorLimits<U, SL_ColorRType>::max().r / (U)SL_ColorLimits<T, SL_ColorRType>::max().r))
   };
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value && !ls::setup::IsSame<U, ls::math::half>::value, SL_ColorRType<U>>::type& p)
{
    return (SL_ColorRType<T>)(p.r * (U)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value && ls::setup::IsSame<U, ls::math::half>::value, SL_ColorRType<U>>::type& p)
{
    return (SL_ColorRType<T>)((float)p.r * (float)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value && !ls::setup::IsSame<T, ls::math::half>::value, SL_ColorRType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
        ? SL_ColorRType<T>{T{0.5} * ((T)p.r * (T{1.0} / (T)SL_ColorLimits<U, SL_ColorRType>::max().r)) + T{0.5}}
        : SL_ColorRType<T>{(T)p.r * (T{1.0} / (T)SL_ColorLimits<U, SL_ColorRType>::max().r)};
}



template <typename T, typename U>
constexpr typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value && ls::setup::IsSame<T, ls::math::half>::value, SL_ColorRType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
        ? SL_ColorRType<T>{0.5f * ((float)p.r * (1.f / (float)SL_ColorLimits<U, SL_ColorRType>::max().r)) + 0.5f}
        : SL_ColorRType<T>{(float)p.r * (1.f / (float)SL_ColorLimits<U, SL_ColorRType>::max().r)};
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
typedef SL_ColorRGType<ls::math::half> SL_ColorRGh;
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
    return ((uint64_t)SL_ColorLimits<T, SL_ColorRType>::max().r > (uint64_t)SL_ColorLimits<U, SL_ColorRType>::max().r)
        ? ((SL_ColorRGType<T>)p * SL_ColorLimits<T, SL_ColorRType>::max().r)
        : (SL_ColorRGType<T>)(p / (U)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value && !ls::setup::IsSame<U, ls::math::half>::value, SL_ColorRGType<U>>::type& p)
{
    return (SL_ColorRGType<T>)(p * (U)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value && ls::setup::IsSame<U, ls::math::half>::value, SL_ColorRGType<U>>::type& p)
{
    return (SL_ColorRGType<T>)((SL_ColorRGType<float>)p * (float)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value && !ls::setup::IsSame<T, ls::math::half>::value, SL_ColorRGType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
           ? (SL_ColorRGType<T>{0.5} * ((SL_ColorRGType<T>)p * (T{1.0} / (T)SL_ColorLimits<U, SL_ColorRType>::max().r)) + SL_ColorRGType<T>{0.5})
           : ((SL_ColorRGType<T>)p * (T{1.0} / (T)SL_ColorLimits<U, SL_ColorRType>::max().r));
}



template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value && ls::setup::IsSame<T, ls::math::half>::value, SL_ColorRGType<U>>::type& p)
{
    return (SL_ColorRGType<ls::math::half>)(ls::setup::IsSigned<U>::value
        ? SL_ColorRGType<float>{0.5f} * ((SL_ColorRGType<float>)p * (1.f / (float)SL_ColorLimits<U, SL_ColorRType>::max().r)) + SL_ColorRGType<float>{0.5f}
        : (SL_ColorRGType<float>)p * (1.f / (float)SL_ColorLimits<U, SL_ColorRType>::max().r)
    );
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
typedef SL_ColorRGBType<ls::math::half> SL_ColorRGBh;
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
    return ((uint64_t)SL_ColorLimits<T, SL_ColorRType>::max().r > (uint64_t)SL_ColorLimits<U, SL_ColorRType>::max().r)
        ? ((SL_ColorRGBType<T>)p * (SL_ColorLimits<T, SL_ColorRType>::max().r / (T)SL_ColorLimits<U, SL_ColorRType>::max().r))
        : (SL_ColorRGBType<T>)(p / (SL_ColorLimits<U, SL_ColorRType>::max().r / (U)SL_ColorLimits<T, SL_ColorRType>::max().r));
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value && !ls::setup::IsSame<U, ls::math::half>::value, SL_ColorRGBType<U>>::type& p)
{
    return (SL_ColorRGBType<T>)(p * (U)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value && ls::setup::IsSame<U, ls::math::half>::value, SL_ColorRGBType<U>>::type& p)
{
    return (SL_ColorRGBType<T>)((SL_ColorRGBType<float>)p * (float)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value && !ls::setup::IsSame<T, ls::math::half>::value, SL_ColorRGBType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
           ? (SL_ColorRGBType<T>{0.5} * ((SL_ColorRGBType<T>)p * (T{1.0} / (T)SL_ColorLimits<U, SL_ColorRType>::max().r)) + SL_ColorRGBType<T>{0.5})
           : ((SL_ColorRGBType<T>)p * (T{1.0} / (T)SL_ColorLimits<U, SL_ColorRType>::max().r));
}



template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGBType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value && ls::setup::IsSame<T, ls::math::half>::value, SL_ColorRGBType<U>>::type& p)
{
    return (SL_ColorRGBType<ls::math::half>)(ls::setup::IsSigned<U>::value
        ? SL_ColorRGBType<float>{0.5f} * ((SL_ColorRGBType<float>)p * (1.f / (float)SL_ColorLimits<U, SL_ColorRType>::max().r)) + SL_ColorRGBType<float>{0.5f}
        : (SL_ColorRGBType<float>)p * (1.f / (float)SL_ColorLimits<U, SL_ColorRType>::max().r)
    );
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
typedef SL_ColorRGBAType<ls::math::half>    SL_ColorRGBAh;
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
    return ((uint64_t)SL_ColorLimits<T, SL_ColorRType>::max().r > (uint64_t)SL_ColorLimits<U, SL_ColorRType>::max().r)
        ? ((SL_ColorRGBAType<T>)p * (SL_ColorLimits<T, SL_ColorRType>::max().r / (T)SL_ColorLimits<U, SL_ColorRType>::max().r))
        : (SL_ColorRGBAType<T>)(p / (SL_ColorLimits<U, SL_ColorRType>::max().r / (U)SL_ColorLimits<T, SL_ColorRType>::max().r));
}



/*-------------------------------------
 * Float to integer
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value && !ls::setup::IsSame<U, ls::math::half>::value, SL_ColorRGBAType<U>>::type& p)
{
    return (SL_ColorRGBAType<T>)(p * (U)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsIntegral<T>::value, SL_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsFloat<U>::value && ls::setup::IsSame<U, ls::math::half>::value, SL_ColorRGBAType<U>>::type& p)
{
    return (SL_ColorRGBAType<T>)((SL_ColorRGBAType<float>)p * (float)SL_ColorLimits<T, SL_ColorRType>::max().r);
}



/*-------------------------------------
 * Integer to float
-------------------------------------*/
template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value && !ls::setup::IsSame<T, ls::math::half>::value, SL_ColorRGBAType<U>>::type& p)
{
    return ls::setup::IsSigned<U>::value
           ? (SL_ColorRGBAType<T>{0.5} * ((SL_ColorRGBAType<T>)p * (T{1.0} / (T)SL_ColorLimits<U, SL_ColorRType>::max().r)) + SL_ColorRGBAType<T>{0.5})
           : ((SL_ColorRGBAType<T>)p * (T{1.0} / (T)SL_ColorLimits<U, SL_ColorRType>::max().r));
}



template <typename T, typename U>
inline typename ls::setup::EnableIf<ls::setup::IsFloat<T>::value, SL_ColorRGBAType<T>>::type
color_cast(const typename ls::setup::EnableIf<ls::setup::IsIntegral<U>::value && ls::setup::IsSame<T, ls::math::half>::value, SL_ColorRGBAType<U>>::type& p)
{
    return (SL_ColorRGBAType<ls::math::half>)(ls::setup::IsSigned<U>::value
        ? SL_ColorRGBAType<float>{0.5f} * ((SL_ColorRGBAType<float>)p * (1.f / (float)SL_ColorLimits<U, SL_ColorRType>::max().r)) + SL_ColorRGBAType<float>{0.5f}
        : (SL_ColorRGBAType<float>)p * (1.f / (float)SL_ColorLimits<U, SL_ColorRType>::max().r)
    );
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
        SL_ColorRType<ls::math::half> rh;
        SL_ColorRType<float> rf;
        SL_ColorRType<double> rd;

        ls::math::vec2_t<uint8_t> rg8;
        ls::math::vec2_t<uint16_t> rg16;
        ls::math::vec2_t<uint32_t> rg32;
        ls::math::vec2_t<uint64_t> rg64;
        ls::math::vec2_t<ls::math::half> rgh;
        ls::math::vec2_t<float> rgf;
        ls::math::vec2_t<double> rgd;

        ls::math::vec3_t<uint8_t> rgb8;
        ls::math::vec3_t<uint16_t> rgb16;
        ls::math::vec3_t<uint32_t> rgb32;
        ls::math::vec3_t<uint64_t> rgb64;
        ls::math::vec3_t<ls::math::half> rgbh;
        ls::math::vec3_t<float> rgbf;
        ls::math::vec3_t<double> rgbd;

        ls::math::vec4_t<uint8_t> rgba8;
        ls::math::vec4_t<uint16_t> rgba16;
        ls::math::vec4_t<uint32_t> rgba32;
        ls::math::vec4_t<uint64_t> rgba64;
        ls::math::vec4_t<ls::math::half> rgbah;
        ls::math::vec4_t<float> rgbaf;
        ls::math::vec4_t<double> rgbad;

        uint8_t rgb332;
        uint16_t rgb565;
        uint16_t rgba5551;
        uint16_t rgba4444;
        uint32_t rgba1010102;
    } color;
};

SL_GeneralColor sl_match_color_for_type(SL_ColorDataType typeToMatch, const ls::math::vec4_t<double>& inColor) noexcept;



#endif /* SL_COLOR_TYPE_HPP */

