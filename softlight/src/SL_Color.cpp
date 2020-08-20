
#include "softlight/SL_Color.hpp"



/*-------------------------------------
 * Get the number of bytes per pixel
-------------------------------------*/
size_t sl_bytes_per_color(SL_ColorDataType p)
{
    switch (p)
    {
        case SL_COLOR_R_8U:        return 1 * sizeof(uint8_t);
        case SL_COLOR_R_16U:       return 1 * sizeof(uint16_t);
        case SL_COLOR_R_32U:       return 1 * sizeof(uint32_t);
        case SL_COLOR_R_64U:       return 1 * sizeof(uint64_t);
        case SL_COLOR_R_FLOAT:     return 1 * sizeof(float);
        case SL_COLOR_R_DOUBLE:    return 1 * sizeof(double);

        case SL_COLOR_RG_8U:       return 2 * sizeof(uint8_t);
        case SL_COLOR_RG_16U:      return 2 * sizeof(uint16_t);
        case SL_COLOR_RG_32U:      return 2 * sizeof(uint32_t);
        case SL_COLOR_RG_64U:      return 2 * sizeof(uint64_t);
        case SL_COLOR_RG_FLOAT:    return 2 * sizeof(float);
        case SL_COLOR_RG_DOUBLE:   return 2 * sizeof(double);

        case SL_COLOR_RGB_8U:      return 3 * sizeof(uint8_t);
        case SL_COLOR_RGB_16U:     return 3 * sizeof(uint16_t);
        case SL_COLOR_RGB_32U:     return 3 * sizeof(uint32_t);
        case SL_COLOR_RGB_64U:     return 3 * sizeof(uint64_t);
        case SL_COLOR_RGB_FLOAT:   return 3 * sizeof(float);
        case SL_COLOR_RGB_DOUBLE:  return 3 * sizeof(double);

        case SL_COLOR_RGBA_8U:     return 4 * sizeof(uint8_t);
        case SL_COLOR_RGBA_16U:    return 4 * sizeof(uint16_t);
        case SL_COLOR_RGBA_32U:    return 4 * sizeof(uint32_t);
        case SL_COLOR_RGBA_64U:    return 4 * sizeof(uint64_t);
        case SL_COLOR_RGBA_FLOAT:  return 4 * sizeof(float);
        case SL_COLOR_RGBA_DOUBLE: return 4 * sizeof(double);

        default:
            break;
    }

    return 0;
}



/*-------------------------------------
 * Get the number of elements per pixel
-------------------------------------*/
unsigned sl_elements_per_color(SL_ColorDataType p)
{
    switch (p)
    {
        case SL_COLOR_R_8U:        return 1;
        case SL_COLOR_R_16U:       return 1;
        case SL_COLOR_R_32U:       return 1;
        case SL_COLOR_R_64U:       return 1;
        case SL_COLOR_R_FLOAT:     return 1;
        case SL_COLOR_R_DOUBLE:    return 1;

        case SL_COLOR_RG_8U:       return 2;
        case SL_COLOR_RG_16U:      return 2;
        case SL_COLOR_RG_32U:      return 2;
        case SL_COLOR_RG_64U:      return 2;
        case SL_COLOR_RG_FLOAT:    return 2;
        case SL_COLOR_RG_DOUBLE:   return 2;

        case SL_COLOR_RGB_8U:      return 3;
        case SL_COLOR_RGB_16U:     return 3;
        case SL_COLOR_RGB_32U:     return 3;
        case SL_COLOR_RGB_64U:     return 3;
        case SL_COLOR_RGB_FLOAT:   return 3;
        case SL_COLOR_RGB_DOUBLE:  return 3;

        case SL_COLOR_RGBA_8U:     return 4;
        case SL_COLOR_RGBA_16U:    return 4;
        case SL_COLOR_RGBA_32U:    return 4;
        case SL_COLOR_RGBA_64U:    return 4;
        case SL_COLOR_RGBA_FLOAT:  return 4;
        case SL_COLOR_RGBA_DOUBLE: return 4;

        default:
            break;
    }

    return 0;
}



/*-----------------------------------------------------------------------------
 * General Conversion for standard types (lossy).
-----------------------------------------------------------------------------*/
template <typename color_type>
SL_GeneralColor sl_match_color_for_type(SL_ColorDataType typeToMatch, const color_type& inColor) noexcept
{
    typedef typename color_type::value_type ConvertedType;

    // First, convert the input color to a double so we can work in a common
    // format
    const auto&& temp = color_cast<double, ConvertedType>(inColor);
    ls::math::vec4_t<double> temp4{0.0};

    // Import all relevant color channels
    switch (color_type::num_components())
    {
        case 4: temp4[3] = temp[3];
        case 3: temp4[2] = temp[2];
        case 2: temp4[1] = temp[1];
        case 1: temp4[0] = temp[0];
    }

    SL_GeneralColor outColor;

    // Convert to the correct output type
    switch (typeToMatch)
    {
        case SL_COLOR_R_8U:        outColor.type = SL_COLOR_R_8U; outColor.color.r8 = color_cast<uint8_t, double>(*reinterpret_cast<const SL_ColorRd*>(temp4.v)); break;
        case SL_COLOR_RG_8U:       outColor.type = SL_COLOR_RG_8U; outColor.color.rg8 = color_cast<uint8_t, double>(*reinterpret_cast<const SL_ColorRGd*>(temp4.v)); break;
        case SL_COLOR_RGB_8U:      outColor.type = SL_COLOR_RGB_8U; outColor.color.rgb8 = color_cast<uint8_t, double>(*reinterpret_cast<const SL_ColorRGBd*>(temp4.v)); break;
        case SL_COLOR_RGBA_8U:     outColor.type = SL_COLOR_RGBA_8U; outColor.color.rgba8 = color_cast<uint8_t, double>(*reinterpret_cast<const SL_ColorRGBAd*>(temp4.v)); break;

        case SL_COLOR_R_16U:       outColor.type = SL_COLOR_R_16U; outColor.color.r16 = color_cast<uint16_t, double>(*reinterpret_cast<const SL_ColorRd*>(temp4.v)); break;
        case SL_COLOR_RG_16U:      outColor.type = SL_COLOR_RG_16U; outColor.color.rg16 = color_cast<uint16_t, double>(*reinterpret_cast<const SL_ColorRGd*>(temp4.v)); break;
        case SL_COLOR_RGB_16U:     outColor.type = SL_COLOR_RGB_16U; outColor.color.rgb16 = color_cast<uint16_t, double>(*reinterpret_cast<const SL_ColorRGBd*>(temp4.v)); break;
        case SL_COLOR_RGBA_16U:    outColor.type = SL_COLOR_RGBA_16U; outColor.color.rgba16 = color_cast<uint16_t, double>(*reinterpret_cast<const SL_ColorRGBAd*>(temp4.v)); break;

        case SL_COLOR_R_32U:       outColor.type = SL_COLOR_R_32U; outColor.color.r32 = color_cast<uint32_t, double>(*reinterpret_cast<const SL_ColorRd*>(temp4.v)); break;
        case SL_COLOR_RG_32U:      outColor.type = SL_COLOR_RG_32U; outColor.color.rg32 = color_cast<uint32_t, double>(*reinterpret_cast<const SL_ColorRGd*>(temp4.v)); break;
        case SL_COLOR_RGB_32U:     outColor.type = SL_COLOR_RGB_32U; outColor.color.rgb32 = color_cast<uint32_t, double>(*reinterpret_cast<const SL_ColorRGBd*>(temp4.v)); break;
        case SL_COLOR_RGBA_32U:    outColor.type = SL_COLOR_RGBA_32U; outColor.color.rgba32 = color_cast<uint32_t, double>(*reinterpret_cast<const SL_ColorRGBAd*>(temp4.v)); break;

        case SL_COLOR_R_64U:       outColor.type = SL_COLOR_R_64U; outColor.color.r64 = color_cast<uint64_t, double>(*reinterpret_cast<const SL_ColorRd*>(temp4.v)); break;
        case SL_COLOR_RG_64U:      outColor.type = SL_COLOR_RG_64U; outColor.color.rg64 = color_cast<uint64_t, double>(*reinterpret_cast<const SL_ColorRGd*>(temp4.v)); break;
        case SL_COLOR_RGB_64U:     outColor.type = SL_COLOR_RGB_64U; outColor.color.rgb64 = color_cast<uint64_t, double>(*reinterpret_cast<const SL_ColorRGBd*>(temp4.v)); break;
        case SL_COLOR_RGBA_64U:    outColor.type = SL_COLOR_RGBA_64U; outColor.color.rgba64 = color_cast<uint64_t, double>(*reinterpret_cast<const SL_ColorRGBAd*>(temp4.v)); break;

        case SL_COLOR_R_FLOAT:     outColor.type = SL_COLOR_R_FLOAT; outColor.color.rf = color_cast<float, double>(*reinterpret_cast<const SL_ColorRd*>(temp4.v)); break;
        case SL_COLOR_RG_FLOAT:    outColor.type = SL_COLOR_RG_FLOAT; outColor.color.rgf = color_cast<float, double>(*reinterpret_cast<const SL_ColorRGd*>(temp4.v)); break;
        case SL_COLOR_RGB_FLOAT:   outColor.type = SL_COLOR_RGB_FLOAT; outColor.color.rgbf = color_cast<float, double>(*reinterpret_cast<const SL_ColorRGBd*>(temp4.v)); break;
        case SL_COLOR_RGBA_FLOAT:  outColor.type = SL_COLOR_RGBA_FLOAT; outColor.color.rgbaf = color_cast<float, double>(*reinterpret_cast<const SL_ColorRGBAd*>(temp4.v)); break;

        case SL_COLOR_R_DOUBLE:     outColor.type = SL_COLOR_R_DOUBLE; outColor.color.rd = *reinterpret_cast<const SL_ColorRd*>(temp4.v); break;
        case SL_COLOR_RG_DOUBLE:    outColor.type = SL_COLOR_RG_DOUBLE; outColor.color.rgd = *reinterpret_cast<const SL_ColorRGd*>(temp4.v); break;
        case SL_COLOR_RGB_DOUBLE:   outColor.type = SL_COLOR_RGB_DOUBLE; outColor.color.rgbd = *reinterpret_cast<const SL_ColorRGBd*>(temp4.v); break;
        case SL_COLOR_RGBA_DOUBLE:  outColor.type = SL_COLOR_RGBA_DOUBLE; outColor.color.rgbad = *reinterpret_cast<const SL_ColorRGBAd*>(temp4.v); break;

        default:
            LS_UNREACHABLE();
    }

    // Clear the output texture
    return outColor;
}

template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<uint8_t>>( SL_ColorDataType, const SL_ColorRType<uint8_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<uint16_t>>(SL_ColorDataType, const SL_ColorRType<uint16_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<uint32_t>>(SL_ColorDataType, const SL_ColorRType<uint32_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<uint64_t>>(SL_ColorDataType, const SL_ColorRType<uint64_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<float>>(   SL_ColorDataType, const SL_ColorRType<float>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<SL_ColorRType<double>>(  SL_ColorDataType, const SL_ColorRType<double>&) noexcept;

template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<uint8_t>>( SL_ColorDataType, const ls::math::vec2_t<uint8_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<uint16_t>>(SL_ColorDataType, const ls::math::vec2_t<uint16_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<uint32_t>>(SL_ColorDataType, const ls::math::vec2_t<uint32_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<uint64_t>>(SL_ColorDataType, const ls::math::vec2_t<uint64_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<float>>(   SL_ColorDataType, const ls::math::vec2_t<float>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec2_t<double>>(  SL_ColorDataType, const ls::math::vec2_t<double>&) noexcept;

template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<uint8_t>>( SL_ColorDataType, const ls::math::vec3_t<uint8_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<uint16_t>>(SL_ColorDataType, const ls::math::vec3_t<uint16_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<uint32_t>>(SL_ColorDataType, const ls::math::vec3_t<uint32_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<uint64_t>>(SL_ColorDataType, const ls::math::vec3_t<uint64_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<float>>(   SL_ColorDataType, const ls::math::vec3_t<float>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec3_t<double>>(  SL_ColorDataType, const ls::math::vec3_t<double>&) noexcept;

template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<uint8_t>>( SL_ColorDataType, const ls::math::vec4_t<uint8_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<uint16_t>>(SL_ColorDataType, const ls::math::vec4_t<uint16_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<uint32_t>>(SL_ColorDataType, const ls::math::vec4_t<uint32_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<uint64_t>>(SL_ColorDataType, const ls::math::vec4_t<uint64_t>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<float>>(   SL_ColorDataType, const ls::math::vec4_t<float>&) noexcept;
template SL_GeneralColor sl_match_color_for_type<ls::math::vec4_t<double>>(  SL_ColorDataType, const ls::math::vec4_t<double>&) noexcept;
