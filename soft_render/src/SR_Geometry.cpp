
#include <algorithm> // std::swap

#include "soft_render/SR_Geometry.hpp"

namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Common Vertex Names
-----------------------------------------------------------------------------*/
namespace
{
/**
 * @brief Common name for a vertex attribute containing positional vertices.
 */
constexpr char VERT_ATTRIB_NAME_POSITION[] = "posAttrib";

/**
 * @brief Common name for a vertex attribute containing UV coordinates.
 */
constexpr char VERT_ATTRIB_NAME_TEXTURE[] = "uvAttrib";

/**
 * @brief Common name for a vertex attribute containing floating-point color
 * information.
 */
constexpr char VERT_ATTRIB_NAME_COLOR[] = "colorAttrib";

/**
 * @brief Common name for a vertex attribute containing vertex normals.
 */
constexpr char VERT_ATTRIB_NAME_NORMAL[] = "normAttrib";

/**
 * @brief Common name for a vertex attribute containing vertex tangents.
 */
constexpr char VERT_ATTRIB_NAME_TANGENT[] = "tangAttrib";

/**
 * @brief Common name for a vertex attribute containing vertex bi-tangents.
 */
constexpr char VERT_ATTRIB_NAME_BITANGENT[] = "btngAttrib";

/**
 * @brief Common name for a vertex attribute containing model matrices.
 */
constexpr char VERT_ATTRIB_NAME_MODEL_MATRIX[] = "modelMatAttrib";

/**
 * @brief Common name for a vertex attribute containing skeletal bone IDs.
 */
constexpr char VERT_ATTRIB_NAME_BONE_ID[] = "boneWeightAttrib";

/**
 * @brief Common name for a vertex attribute containing skeletal bone weights.
 */
constexpr char VERT_ATTRIB_NAME_BONE_WEIGHT[] = "boneIdAttrib";

/**
 * @brief Common name for an ambient lighting vertex attribute.
 */
constexpr char VERT_ATTRIB_NAME_AMBIENT[] = "ambientAttrib";

/**
 * @brief Common name for a diffuse lighting vertex attribute.
 */
constexpr char VERT_ATTRIB_NAME_DIFFUSE[] = "diffuseAttrib";

/**
 * @brief Common name for a specular vertex component.
 */
constexpr char VERT_ATTRIB_NAME_SPECULAR[] = "specularAttrib";

/**
 * @brief Common name for a roughness vertex component.
 */
constexpr char VERT_ATTRIB_NAME_ROUGHNESS[] = "roughAttrib";

/**
 * @brief Common name for a metallic vertex component.
 */
constexpr char VERT_ATTRIB_NAME_METALLIC[] = "metalAttrib";

/**
 * @brief Common name for an instance index vertex component.
 */
constexpr char VERT_ATTRIB_NAME_INDEX[] = "indexAttrib";

/**
 * @brief Common name for a bounding box vertex component (Top-Rear-Right).
 */
constexpr char VERT_ATTRIB_NAME_BBOX_TRR[] = "bboxTRRAttrib";

/**
 * @brief Common name for a bounding box vertex component (Bottom-front-left).
 */
constexpr char VERT_ATTRIB_NAME_BBOX_BFL[] = "bboxBFLAttrib";



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * Vertex helper function
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Bytes per data type
-------------------------------------*/
uint32_t sr_bytes_per_type(SR_DataType t)
{
    switch (t)
    {
        case VERTEX_DATA_BYTE:
            return sizeof(int8_t);

        case VERTEX_DATA_SHORT:
            return sizeof(int16_t);

        case VERTEX_DATA_INT:
            return sizeof(int32_t);

        case VERTEX_DATA_LONG:
            return sizeof(int64_t);

        case VERTEX_DATA_FLOAT:
            return sizeof(float);

        case VERTEX_DATA_DOUBLE:
            return sizeof(double);

        default:
            break;
    }

    return 0;
}



/*-------------------------------------
 * Bytes per vertex
-------------------------------------*/
uint32_t sr_bytes_per_vertex(SR_DataType t, SR_Dimension d)
{
    const uint32_t numDimens = 1 + (uint32_t)d;

    switch (t)
    {
        case VERTEX_DATA_BYTE:
            return numDimens * sizeof(int8_t);

        case VERTEX_DATA_SHORT:
            return numDimens * sizeof(int16_t);

        case VERTEX_DATA_INT:
            return numDimens * sizeof(int32_t);

        case VERTEX_DATA_LONG:
            return numDimens * sizeof(int64_t);

        case VERTEX_DATA_FLOAT:
            return numDimens * sizeof(float);

        case VERTEX_DATA_DOUBLE:
            return numDimens * sizeof(double);

        default:
            break;
    }

    return 0;
}



/*-------------------------------------
 * Get the number of bytes which pad a vertex until a particular attribute can be found.
-------------------------------------*/
unsigned sr_vertex_attrib_offset(const SR_CommonVertType vertexTypes, const SR_CommonVertType mask)
{
    unsigned numBytes = 0;

    for (unsigned i = 0; i < SR_NUM_COMMON_VERTEX_FLAGS; ++i)
    {
        const SR_CommonVertType currentType = SR_COMMON_VERTEX_FLAGS[i];
        if (0 != (vertexTypes & currentType))
        {
            if (0 != (currentType & mask))
            {
                break;
            }

            switch (SR_COMMON_VERTEX_FLAGS[i])
            {
                case POSITION_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case TEXTURE_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_2);
                    break;

                case COLOR_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_4);
                    break;

                case NORMAL_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case TANGENT_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case BITANGENT_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case MODEL_MAT_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_4) * 4;
                    break;

                case BONE_ID_VERTEX: // Maximum of 4 bone IDs per vertex
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_INT, VERTEX_DIMENSION_4);
                    break;

                case BONE_WEIGHT_VERTEX: // Maximum of 4 bone weights per vertex
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_4);
                    break;

                case AMBIENT_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_BYTE, VERTEX_DIMENSION_1);
                    break;

                case DIFFUSE_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_BYTE, VERTEX_DIMENSION_1);
                    break;

                case SPECULAR_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_BYTE, VERTEX_DIMENSION_1);
                    break;

                case ROUGHNESS_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_BYTE, VERTEX_DIMENSION_1);
                    break;

                case METALLIC_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_BYTE, VERTEX_DIMENSION_1);
                    break;

                case INDEX_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_INT, VERTEX_DIMENSION_1);
                    break;

                case BBOX_TRR_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case BBOX_BFL_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case STANDARD_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_2);
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case BONE_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_BYTE, VERTEX_DIMENSION_1);
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_1);
                    break;

                case OCCLUSION_VERTEX:
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    numBytes += sr_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;
            }
        }
    }

    return numBytes;
}



/*-------------------------------------
 * Determine the number of get_size used by a vertex type
-------------------------------------*/
SR_Dimension sr_dimens_of_vertex(const SR_CommonVertType vertType)
{
    switch (vertType)
    {
        case POSITION_VERTEX:
            return VERTEX_DIMENSION_3;

        case TEXTURE_VERTEX:
            return  VERTEX_DIMENSION_2;

        case COLOR_VERTEX:
            return VERTEX_DIMENSION_4;

        case NORMAL_VERTEX:
            return VERTEX_DIMENSION_3;

        case TANGENT_VERTEX:
            return VERTEX_DIMENSION_3;

        case BITANGENT_VERTEX:
            return VERTEX_DIMENSION_3;

        case MODEL_MAT_VERTEX:
            return VERTEX_DIMENSION_4;

        case BONE_ID_VERTEX:
            return VERTEX_DIMENSION_1;

        case BONE_WEIGHT_VERTEX:
            return VERTEX_DIMENSION_1;

        case AMBIENT_VERTEX:
            return VERTEX_DIMENSION_1;

        case DIFFUSE_VERTEX:
            return VERTEX_DIMENSION_1;

        case SPECULAR_VERTEX:
            return VERTEX_DIMENSION_1;

        case ROUGHNESS_VERTEX:
            return VERTEX_DIMENSION_1;

        case METALLIC_VERTEX:
            return VERTEX_DIMENSION_1;

        case INDEX_VERTEX:
            return VERTEX_DIMENSION_1;

        case BBOX_TRR_VERTEX:
            return VERTEX_DIMENSION_3;

        case BBOX_BFL_VERTEX:
            return VERTEX_DIMENSION_3;

        default:
            break;
    }

    return SR_Dimension::VERTEX_DIMENSION_1;
}



/*-------------------------------------
 * Determine the basic data type for a common vertex
-------------------------------------*/
SR_DataType sr_type_of_vertex(const SR_CommonVertType vertType)
{
    switch (vertType)
    {
        case POSITION_VERTEX:
            return VERTEX_DATA_FLOAT;

        case TEXTURE_VERTEX:
            return VERTEX_DATA_FLOAT;

        case COLOR_VERTEX:
            return VERTEX_DATA_FLOAT;

        case NORMAL_VERTEX:
            return VERTEX_DATA_FLOAT;

        case TANGENT_VERTEX:
            return VERTEX_DATA_FLOAT;

        case BITANGENT_VERTEX:
            return VERTEX_DATA_FLOAT;

        case MODEL_MAT_VERTEX:
            return VERTEX_DATA_FLOAT;

        case BONE_ID_VERTEX:
            return VERTEX_DATA_BYTE;

        case BONE_WEIGHT_VERTEX:
            return VERTEX_DATA_FLOAT;

        case AMBIENT_VERTEX:
            return VERTEX_DATA_BYTE;

        case DIFFUSE_VERTEX:
            return VERTEX_DATA_BYTE;

        case SPECULAR_VERTEX:
            return VERTEX_DATA_BYTE;

        case ROUGHNESS_VERTEX:
            return VERTEX_DATA_BYTE;
            break;

        case METALLIC_VERTEX:
            return VERTEX_DATA_BYTE;

        case INDEX_VERTEX:
            return VERTEX_DATA_INT;

        case BBOX_TRR_VERTEX:
            return VERTEX_DATA_FLOAT;

        case BBOX_BFL_VERTEX:
            return VERTEX_DATA_FLOAT;

        default:
            break;
    }

    return VERTEX_DATA_INVALID;
}



/*-------------------------------------
 * Get the common name for a vertex attribute
-------------------------------------*/
const char* const* sr_common_vertex_names() noexcept
{
    static const char* const names[] = {
        VERT_ATTRIB_NAME_POSITION,
        VERT_ATTRIB_NAME_TEXTURE,
        VERT_ATTRIB_NAME_COLOR,
        VERT_ATTRIB_NAME_NORMAL,
        VERT_ATTRIB_NAME_TANGENT,
        VERT_ATTRIB_NAME_BITANGENT,
        VERT_ATTRIB_NAME_MODEL_MATRIX,
        VERT_ATTRIB_NAME_BONE_ID,
        VERT_ATTRIB_NAME_BONE_WEIGHT,
        VERT_ATTRIB_NAME_AMBIENT,
        VERT_ATTRIB_NAME_DIFFUSE,
        VERT_ATTRIB_NAME_SPECULAR,
        VERT_ATTRIB_NAME_ROUGHNESS,
        VERT_ATTRIB_NAME_METALLIC,
        VERT_ATTRIB_NAME_INDEX,
        VERT_ATTRIB_NAME_BBOX_TRR,
        VERT_ATTRIB_NAME_BBOX_BFL
    };

    // Because I still don't trust myself...
    static_assert(
        LS_ARRAY_SIZE(names) == SR_NUM_COMMON_VERTEX_FLAGS,
        "Unable to match the commonly used vertex names to their attributes in Vertex.h."
    );

    return names;
}




/*-----------------------------------------------------------------------------
 * Indexed vertex helper function
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Get the minimum required index format required to perform indexed rendering.
-------------------------------------*/
SR_DataType sr_required_index_type(const unsigned numVertices)
{
    return (numVertices <= std::numeric_limits<unsigned char>::max())
           ? SR_DataType::VERTEX_DATA_BYTE
           : (numVertices <= std::numeric_limits<unsigned short>::max())
             ? SR_DataType::VERTEX_DATA_SHORT
             : SR_DataType::VERTEX_DATA_INT;
}



/*-------------------------------------
 * Get the number of bytes required to store a set of indices.
-------------------------------------*/
unsigned sr_index_byte_size(const SR_DataType indexType)
{
    switch (indexType)
    {
        case SR_DataType::VERTEX_DATA_BYTE:
            return sizeof (unsigned char);

        case SR_DataType::VERTEX_DATA_SHORT:
            return sizeof (unsigned short);

        case SR_DataType::VERTEX_DATA_INT:
            return sizeof (unsigned int);

        default:
            break;
    }

    return 0;
}



/*------------------------------------------------------------------------------
 * Drawing algorithms
------------------------------------------------------------------------------*/
/*-------------------------------------
 * Line Drawing: Bresenham Base Case
-------------------------------------*/
void sr_draw_line_bresenham(SR_ColorRGB8* const pImg, coord_shrt_t w, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SR_ColorRGB8& color)
{
    const bool steep = math::abs(x1 - x2) < math::abs(y1 - y2);
    if (steep)
    {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    
    if (x1 > x2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    
    const coord_shrt_t dx = x2 - x1;
    const coord_shrt_t dy = y2 - y1;
    const coord_shrt_t dErr = math::abs(dy) * 2;
    const coord_shrt_t yErr = (y2 > y1) ? 1 : -1;
    coord_shrt_t currentErr = 0;
    coord_shrt_t y = y1;

    // This if-statement has been pulled out of the main loop in order to avoid
    // branching on intel CPUs. It makes no difference on ARM.
    if (steep)
    {
        for (coord_shrt_t x = x1; x <= x2; ++x)
        {
            sr_draw_pixel(pImg, w, y, x, color);
            currentErr += dErr;

            if (currentErr > dx)
            {
                y += yErr;
                currentErr -= 2 * dx;
            }
        }
    }
    else
    {
        for (coord_shrt_t x = x1; x <= x2; ++x)
        {
            sr_draw_pixel(pImg, w, x, y, color);
            currentErr += dErr;

            if (currentErr > dx)
            {
                y += yErr;
                currentErr -= 2 * dx;
            }
        }
    }
}



/*-------------------------------------
 * Line Drawing: EFLA (Variant 5)
-------------------------------------*/
void sr_draw_line_efla5(SR_ColorRGB8* pImg, coord_shrt_t width, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SR_ColorRGB8& color)
{
    coord_long_t shortLen = y2 - y1;
    coord_long_t longLen  = x2 - x1;
    const bool yLonger    = math::abs(shortLen) > math::abs(longLen);
    
    if (yLonger)
    {
        std::swap(shortLen, longLen);
    }
    
    const coord_long_t decInc = (longLen == 0) ? 0 : (((coord_long_t)shortLen << FIXED_BITS) / longLen);

    if (yLonger)
    {
        const coord_long_t fixedX = (coord_long_t)x1 << FIXED_BITS;
        const coord_long_t longLenY = longLen + y1;
            
        if (longLen > 0)
        {
            for (coord_long_t j = MASK_BITS+fixedX; y1 <= longLenY; ++y1)
            {
                sr_draw_pixel(pImg, width, j >> FIXED_BITS, y1, color);
                j += decInc;
            }
            return;
        }
        
        for (coord_long_t j = MASK_BITS+fixedX; y1 >= longLenY; --y1)
        {
            sr_draw_pixel(pImg, width, j >> FIXED_BITS, y1, color);
            j -= decInc;
        }
        return; 
    }

    const coord_long_t fixedY = (coord_long_t)y1 << FIXED_BITS;
    const coord_long_t longLenX = longLen + x1;
    
    if (longLen > 0)
    {
        for (coord_long_t j = MASK_BITS+fixedY; x1 <= longLenX; ++x1)
        {
            sr_draw_pixel(pImg, width, x1, j >> FIXED_BITS, color);
            j += decInc;
        }
        return;
    }
    
    for (coord_long_t j = MASK_BITS+fixedY; x1 >= longLenX; --x1)
    {
        sr_draw_pixel(pImg, width, x1, j >> FIXED_BITS, color);
        j -= decInc;
    }
}



/*-------------------------------------
 * Line Drawing: Bresenham's (Fixed-Point)
-------------------------------------*/
void sr_draw_line_fixed(SR_ColorRGB8* const pImg, coord_shrt_t w, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SR_ColorRGB8& color)
{
    union
    {
        coord_long_t i;
        struct
        {
            coord_shrt_t lo;
            coord_shrt_t hi;
        } fx;
    } f, g;

    // allow lines to be more vertical than horizontal
    if (y1 >= y2 && x1 >= x2)
    {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }    
    
    const coord_long_t dx = x2 - x1;
    const coord_long_t dy = y2 - y1;
    constexpr coord_long_t COORD_SHORT_MAX = std::numeric_limits<coord_shrt_t>::max();
    
    if (dx >= dy)
    {
        const coord_long_t m  = dx ? ((coord_long_t)dy << FIXED_BITS) / dx : 0;
        
        f.i = (coord_long_t)y1 << FIXED_BITS;
        
        for (coord_shrt_t x = x1; x <= x2; ++x, f.i += m)
        {
            g.i = f.i + COORD_SHORT_MAX;
            sr_draw_pixel(pImg, w, x, g.fx.hi, color);
        }
    }
    else
    {
        const coord_long_t m  = dy ? ((coord_long_t)dx << FIXED_BITS) / dy : 0;
        
        f.i = (coord_long_t)x1 << FIXED_BITS;
        
        for (coord_shrt_t y = y1; y <= y2; ++y, f.i += m)
        {
            g.i = f.i + COORD_SHORT_MAX;
            sr_draw_pixel(pImg, w, g.fx.hi, y, color);
        }
    }
}



/*-----------------------------------------------------------------------------
 * Vertex Information Algorithms
-----------------------------------------------------------------------------*/
/*-------------------------------------
    Calculate the normal vector of a triangle
-------------------------------------*/
math::vec3 sr_calc_normal(
    const math::vec3& v0,
    const math::vec3& v1,
    const math::vec3& v2
) {
    const math::vec3&& a = v1 - v0;
    const math::vec3&& b = v2 - v0;

    return math::normalize(math::vec3{
        (a.v[1] * b.v[2]) - (a.v[2] * b.v[1]),
        (a.v[2] * b.v[0]) - (a.v[0] * b.v[2]),
        (a.v[0] * b.v[1]) - (a.v[1] * b.v[0])
    });
}



/*-------------------------------------
 * Calculate the tangents for a set of triangles (placed in a vertex array).
-------------------------------------*/
void sr_calc_tangents(
    unsigned vertCount,
    const math::vec3* const positions,
    const math::vec2* const uvs,
    math::vec3* tangents,
    math::vec3* bitangents
) {
    for (unsigned i = 0; i < vertCount; ++i) {
        const math::vec3&& deltaPos1 = positions[i + 1] - positions[i];
        const math::vec3&& deltaPos2 = positions[i + 2] - positions[i];

        const math::vec2&& deltaUv1 = uvs[i + 1] - uvs[i];
        const math::vec2&& deltaUv2 = uvs[i + 2] - uvs[i];

        const float r = math::rcp((deltaUv1[0] * deltaUv2[1]) - (deltaUv1[1] * deltaUv2[0]));

        tangents[i] = tangents[i + 1] = tangents[i + 2] =
            math::vec3{(deltaPos1 * deltaUv2[1]) - (deltaPos2 * deltaUv1[1])} * r;

        bitangents[i] = bitangents[i + 1] = bitangents[i + 2] =
            math::vec3{(deltaPos2 * deltaUv1[0]) - (deltaPos1 * deltaUv2[0])} * r;
    }
}



/*-------------------------------------
 * Calculate the tangent of a textured triangle in model-space.
-------------------------------------*/
math::vec3 sr_calc_tangent(
    const math::vec3& pos0,
    const math::vec3& pos1,
    const math::vec3& pos2,
    const math::vec2& uv0,
    const math::vec2& uv1,
    const math::vec2& uv2
) {
    const math::vec3&& deltaPos1 = pos1 - pos0;
    const math::vec3&& deltaPos2 = pos2 - pos0;

    const math::vec2&& deltaUv1 = uv1 - uv0;
    const math::vec2&& deltaUv2 = uv2 - uv0;

    const float r = math::rcp((deltaUv1[0] * deltaUv2[1]) - (deltaUv1[1] * deltaUv2[0]));

    return math::vec3{(deltaPos1 * deltaUv2[1]) - (deltaPos2 * deltaUv1[1])} * r;
}



/*-------------------------------------
 * Calculate the bi-tangent of a textured triangle in model-space.
-------------------------------------*/
math::vec3 sr_calc_bitangent(
    const math::vec3& pos0,
    const math::vec3& pos1,
    const math::vec3& pos2,
    const math::vec2& uv0,
    const math::vec2& uv1,
    const math::vec2& uv2
) {
    const math::vec3&& deltaPos1 = pos1 - pos0;
    const math::vec3&& deltaPos2 = pos2 - pos0;

    const math::vec2&& deltaUv1 = uv1 - uv0;
    const math::vec2&& deltaUv2 = uv2 - uv0;

    const float r = math::rcp((deltaUv1[0] * deltaUv2[1]) - (deltaUv1[1] * deltaUv2[0]));

    return math::vec3{(deltaPos2 * deltaUv1[0]) - (deltaPos1 * deltaUv2[0])} * r;
}
