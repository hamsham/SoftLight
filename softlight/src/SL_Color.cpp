
#include "lightsky/math/vec_utils.h"

#include "softlight/SL_Color.hpp"
#include "softlight/SL_ColorCompressed.hpp"



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

        case SL_COLOR_RGB_565:     return sizeof(uint16_t);
        case SL_COLOR_RGBA_5551:   return sizeof(uint16_t);
        case SL_COLOR_RGBA_4444:   return sizeof(uint16_t);

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

        case SL_COLOR_RGB_565:     return 3;
        case SL_COLOR_RGBA_5551:   return 3;
        case SL_COLOR_RGBA_4444:   return 3;

        default:
            break;
    }

    return 0;
}



/*-------------------------------------
 * Compressed format check
-------------------------------------*/
bool sl_is_compressed_color(SL_ColorDataType p)
{
    switch (p)
    {
        case SL_COLOR_RGB_565:
        case SL_COLOR_RGBA_5551:
        case SL_COLOR_RGBA_4444:
            return true;

        default:
            break;
    }

    return false;
}



/*-------------------------------------
 * helper function to avoid breaking reinterpret_cast from a compressed color
 * to uint16_t (i.e., don't break strict aliasing).
-------------------------------------*/
template <typename CompressedColorType>
inline LS_INLINE uint16_t _sl_rgb565_as_bits(const CompressedColorType& c) noexcept
{
    static_assert(sizeof(CompressedColorType) == sizeof(uint16_t), "Error in compressed color size.");

    union
    {
        CompressedColorType rgb;
        uint16_t bits;
    } ret{c};
    return ret.bits;
}

/*-----------------------------------------------------------------------------
 * General Conversion for standard types (lossy).
-----------------------------------------------------------------------------*/
SL_GeneralColor sl_match_color_for_type(SL_ColorDataType typeToMatch, const ls::math::vec4_t<double>& inColor) noexcept
{
    SL_GeneralColor outColor;
    outColor.type = typeToMatch;

    // Convert to the correct output type
    switch (typeToMatch)
    {
        case SL_COLOR_R_8U:        outColor.color.r8 = color_cast<uint8_t, double>(*reinterpret_cast<const SL_ColorRd*>(inColor.v)); break;
        case SL_COLOR_RG_8U:       outColor.color.rg8 = color_cast<uint8_t, double>(*reinterpret_cast<const SL_ColorRGd*>(inColor.v)); break;
        case SL_COLOR_RGB_8U:      outColor.color.rgb8 = color_cast<uint8_t, double>(*reinterpret_cast<const SL_ColorRGBd*>(inColor.v)); break;
        case SL_COLOR_RGBA_8U:     outColor.color.rgba8 = color_cast<uint8_t, double>(*reinterpret_cast<const SL_ColorRGBAd*>(inColor.v)); break;

        case SL_COLOR_R_16U:       outColor.color.r16 = color_cast<uint16_t, double>(*reinterpret_cast<const SL_ColorRd*>(inColor.v)); break;
        case SL_COLOR_RG_16U:      outColor.color.rg16 = color_cast<uint16_t, double>(*reinterpret_cast<const SL_ColorRGd*>(inColor.v)); break;
        case SL_COLOR_RGB_16U:     outColor.color.rgb16 = color_cast<uint16_t, double>(*reinterpret_cast<const SL_ColorRGBd*>(inColor.v)); break;
        case SL_COLOR_RGBA_16U:    outColor.color.rgba16 = color_cast<uint16_t, double>(*reinterpret_cast<const SL_ColorRGBAd*>(inColor.v)); break;

        case SL_COLOR_R_32U:       outColor.color.r32 = color_cast<uint32_t, double>(*reinterpret_cast<const SL_ColorRd*>(inColor.v)); break;
        case SL_COLOR_RG_32U:      outColor.color.rg32 = color_cast<uint32_t, double>(*reinterpret_cast<const SL_ColorRGd*>(inColor.v)); break;
        case SL_COLOR_RGB_32U:     outColor.color.rgb32 = color_cast<uint32_t, double>(*reinterpret_cast<const SL_ColorRGBd*>(inColor.v)); break;
        case SL_COLOR_RGBA_32U:    outColor.color.rgba32 = color_cast<uint32_t, double>(*reinterpret_cast<const SL_ColorRGBAd*>(inColor.v)); break;

        case SL_COLOR_R_64U:       outColor.color.r64 = color_cast<uint64_t, double>(*reinterpret_cast<const SL_ColorRd*>(inColor.v)); break;
        case SL_COLOR_RG_64U:      outColor.color.rg64 = color_cast<uint64_t, double>(*reinterpret_cast<const SL_ColorRGd*>(inColor.v)); break;
        case SL_COLOR_RGB_64U:     outColor.color.rgb64 = color_cast<uint64_t, double>(*reinterpret_cast<const SL_ColorRGBd*>(inColor.v)); break;
        case SL_COLOR_RGBA_64U:    outColor.color.rgba64 = color_cast<uint64_t, double>(*reinterpret_cast<const SL_ColorRGBAd*>(inColor.v)); break;

        case SL_COLOR_R_FLOAT:     outColor.color.rf = color_cast<float, double>(*reinterpret_cast<const SL_ColorRd*>(inColor.v)); break;
        case SL_COLOR_RG_FLOAT:    outColor.color.rgf = color_cast<float, double>(*reinterpret_cast<const SL_ColorRGd*>(inColor.v)); break;
        case SL_COLOR_RGB_FLOAT:   outColor.color.rgbf = color_cast<float, double>(*reinterpret_cast<const SL_ColorRGBd*>(inColor.v)); break;
        case SL_COLOR_RGBA_FLOAT:  outColor.color.rgbaf = color_cast<float, double>(*reinterpret_cast<const SL_ColorRGBAd*>(inColor.v)); break;

        case SL_COLOR_R_DOUBLE:    outColor.color.rd = *reinterpret_cast<const SL_ColorRd*>(inColor.v); break;
        case SL_COLOR_RG_DOUBLE:   outColor.color.rgd = *reinterpret_cast<const SL_ColorRGd*>(inColor.v); break;
        case SL_COLOR_RGB_DOUBLE:  outColor.color.rgbd = *reinterpret_cast<const SL_ColorRGBd*>(inColor.v); break;
        case SL_COLOR_RGBA_DOUBLE: outColor.color.rgbad = *reinterpret_cast<const SL_ColorRGBAd*>(inColor.v); break;

        case SL_COLOR_RGB_565:     outColor.color.rgb565   = _sl_rgb565_as_bits(rgb565_cast<double>(ls::math::vec3_cast<double>(inColor))); break;
        case SL_COLOR_RGBA_5551:   outColor.color.rgba5551 = _sl_rgb565_as_bits(rgb5551_cast<double>(inColor)); break;
        case SL_COLOR_RGBA_4444:   outColor.color.rgba4444 = _sl_rgb565_as_bits(rgb4444_cast<double>(inColor)); break;

        default:
            LS_UNREACHABLE();
    }

    // Clear the output texture
    return outColor;
}
