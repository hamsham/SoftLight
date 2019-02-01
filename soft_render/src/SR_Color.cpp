
#include "soft_render/SR_Color.hpp"



/*-------------------------------------
 * Get the number of bytes per pixel
-------------------------------------*/
size_t sr_bytes_per_color(SR_ColorDataType p)
{
    switch (p)
    {
        case SR_COLOR_R_8U:        return 1 * sizeof(uint8_t);
        case SR_COLOR_R_16U:       return 1 * sizeof(uint16_t);
        case SR_COLOR_R_32U:       return 1 * sizeof(uint32_t);
        case SR_COLOR_R_64U:       return 1 * sizeof(uint64_t);
        case SR_COLOR_R_FLOAT:     return 1 * sizeof(float);
        case SR_COLOR_R_DOUBLE:    return 1 * sizeof(double);

        case SR_COLOR_RG_8U:       return 2 * sizeof(uint8_t);
        case SR_COLOR_RG_16U:      return 2 * sizeof(uint16_t);
        case SR_COLOR_RG_32U:      return 2 * sizeof(uint32_t);
        case SR_COLOR_RG_64U:      return 2 * sizeof(uint64_t);
        case SR_COLOR_RG_FLOAT:    return 2 * sizeof(float);
        case SR_COLOR_RG_DOUBLE:   return 2 * sizeof(double);

        case SR_COLOR_RGB_8U:      return 3 * sizeof(uint8_t);
        case SR_COLOR_RGB_16U:     return 3 * sizeof(uint16_t);
        case SR_COLOR_RGB_32U:     return 3 * sizeof(uint32_t);
        case SR_COLOR_RGB_64U:     return 3 * sizeof(uint64_t);
        case SR_COLOR_RGB_FLOAT:   return 3 * sizeof(float);
        case SR_COLOR_RGB_DOUBLE:  return 3 * sizeof(double);

        case SR_COLOR_RGBA_8U:     return 4 * sizeof(uint8_t);
        case SR_COLOR_RGBA_16U:    return 4 * sizeof(uint16_t);
        case SR_COLOR_RGBA_32U:    return 4 * sizeof(uint32_t);
        case SR_COLOR_RGBA_64U:    return 4 * sizeof(uint64_t);
        case SR_COLOR_RGBA_FLOAT:  return 4 * sizeof(float);
        case SR_COLOR_RGBA_DOUBLE: return 4 * sizeof(double);

        default:
            break;
    }

    return 0;
}



/*-------------------------------------
 * Get the number of elements per pixel
-------------------------------------*/
unsigned sr_elements_per_color(SR_ColorDataType p)
{
    switch (p)
    {
        case SR_COLOR_R_8U:        return 1;
        case SR_COLOR_R_16U:       return 1;
        case SR_COLOR_R_32U:       return 1;
        case SR_COLOR_R_64U:       return 1;
        case SR_COLOR_R_FLOAT:     return 1;
        case SR_COLOR_R_DOUBLE:    return 1;

        case SR_COLOR_RG_8U:       return 2;
        case SR_COLOR_RG_16U:      return 2;
        case SR_COLOR_RG_32U:      return 2;
        case SR_COLOR_RG_64U:      return 2;
        case SR_COLOR_RG_FLOAT:    return 2;
        case SR_COLOR_RG_DOUBLE:   return 2;

        case SR_COLOR_RGB_8U:      return 3;
        case SR_COLOR_RGB_16U:     return 3;
        case SR_COLOR_RGB_32U:     return 3;
        case SR_COLOR_RGB_64U:     return 3;
        case SR_COLOR_RGB_FLOAT:   return 3;
        case SR_COLOR_RGB_DOUBLE:  return 3;

        case SR_COLOR_RGBA_8U:     return 4;
        case SR_COLOR_RGBA_16U:    return 4;
        case SR_COLOR_RGBA_32U:    return 4;
        case SR_COLOR_RGBA_64U:    return 4;
        case SR_COLOR_RGBA_FLOAT:  return 4;
        case SR_COLOR_RGBA_DOUBLE: return 4;

        default:
            break;
    }

    return 0;
}
