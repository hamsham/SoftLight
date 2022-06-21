
#include <algorithm> // std::swap

#include "lightsky/math/bits.h"

#include "softlight/SL_Geometry.hpp"

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
 * @brief Common name for a vertex attribute containing half-float UV
 * coordinates.
 */
constexpr char VERT_ATTRIB_NAME_PACKED_TEXTURE[] = "uvAttribP";

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
 * @brief Common name for a vertex attribute containing packed vertex normals.
 */
constexpr char VERT_ATTRIB_NAME_PACKED_NORMAL[] = "normAttribP";

/**
 * @brief Common name for a vertex attribute containing packed vertex tangents.
 */
constexpr char VERT_ATTRIB_NAME_PACKED_TANGENT[] = "tangAttribP";

/**
 * @brief Common name for a vertex attribute containing packed vertex bi-tangents.
 */
constexpr char VERT_ATTRIB_NAME_PACKED_BITANGENT[] = "btngAttribP";

/**
 * @brief Common name for a vertex attribute containing model matrices.
 */
constexpr char VERT_ATTRIB_NAME_MODEL_MATRIX[] = "modelMatAttrib";

/**
 * @brief Common name for a vertex attribute containing skeletal bone IDs.
 */
constexpr char VERT_ATTRIB_NAME_BONE_ID[] = "boneIdAttrib";

/**
 * @brief Common name for a vertex attribute containing skeletal bone IDs.
 */
constexpr char VERT_ATTRIB_NAME_PACKED_BONE_ID[] = "boneIdAttribP";

/**
 * @brief Common name for a vertex attribute containing skeletal bone weights.
 */
constexpr char VERT_ATTRIB_NAME_BONE_WEIGHT[] = "boneWeightAttrib";

/**
 * @brief Common name for a vertex attribute containing skeletal bone weights.
 */
constexpr char VERT_ATTRIB_NAME_PACKED_BONE_WEIGHT[] = "boneWeightAttribP";

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
uint32_t sl_bytes_per_type(SL_DataType t)
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
uint32_t sl_bytes_per_vertex(SL_DataType t, SL_Dimension d)
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
unsigned sl_vertex_attrib_offset(const SL_CommonVertType vertexTypes, const SL_CommonVertType mask)
{
    unsigned numBytes = 0;

    for (unsigned i = 0; i < SL_NUM_COMMON_VERTEX_FLAGS; ++i)
    {
        const SL_CommonVertType currentType = SL_COMMON_VERTEX_FLAGS[i];
        if (0 != (vertexTypes & currentType))
        {
            if (0 != (currentType & mask))
            {
                break;
            }

            switch (SL_COMMON_VERTEX_FLAGS[i])
            {
                case POSITION_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case TEXTURE_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_2);
                    break;

                case PACKED_TEXTURE_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_SHORT, VERTEX_DIMENSION_2);
                    break;

                case COLOR_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_4);
                    break;

                case NORMAL_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case TANGENT_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case BITANGENT_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case PACKED_NORMAL_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_INT, VERTEX_DIMENSION_1);
                    break;

                case PACKED_TANGENT_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_INT, VERTEX_DIMENSION_1);
                    break;

                case PACKED_BITANGENT_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_INT, VERTEX_DIMENSION_1);
                    break;

                case MODEL_MAT_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_4) * 4;
                    break;

                case BONE_ID_VERTEX: // Maximum of 4 32-bit bone IDs per vertex
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_INT, VERTEX_DIMENSION_4);
                    break;

                case PACKED_BONE_ID_VERTEX: // Maximum of 4 16-bit bone IDs per vertex
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_SHORT, VERTEX_DIMENSION_4);
                    break;

                case BONE_WEIGHT_VERTEX: // Maximum of 4 bone weights per vertex
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_4);
                    break;

                case PACKED_BONE_WEIGHT_VERTEX: // Maximum of 4 16-bit float weights per vertex
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_SHORT, VERTEX_DIMENSION_4);
                    break;

                case INDEX_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_INT, VERTEX_DIMENSION_1);
                    break;

                case BBOX_TRR_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case BBOX_BFL_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case STANDARD_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_2);
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;

                case BONE_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_INT, VERTEX_DIMENSION_4);
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_4);
                    break;

                case OCCLUSION_VERTEX:
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    numBytes += sl_bytes_per_vertex(VERTEX_DATA_FLOAT, VERTEX_DIMENSION_3);
                    break;
            }
        }
    }

    return numBytes;
}



/*-------------------------------------
 * Count the number of active vertex attributes
-------------------------------------*/
unsigned sl_count_vertex_attribs(const SL_CommonVertType vertAttribs)
{
    return math::popcnt_u32((unsigned)vertAttribs);
}



/*-------------------------------------
 * Get the Nth active vertex attributes in an attribute bitmask.
-------------------------------------*/
SL_CommonVertType sl_get_vertex_attrib(const SL_CommonVertType vertAttribs, unsigned index)
{
    unsigned numAttribs = sl_count_vertex_attribs(vertAttribs);
    for (unsigned i = 0, j = 0; j < numAttribs; ++i)
    {
        unsigned mask = (unsigned)vertAttribs & (0x00000001u << i);
        if (mask)
        {
            if (j == index)
            {
                return (SL_CommonVertType)mask;
            }

            ++j;
        }
    }

    return (SL_CommonVertType)0;
}



/*-------------------------------------
 * Determine the number of get_size used by a vertex type
-------------------------------------*/
SL_Dimension sl_dimens_of_vertex(const SL_CommonVertType vertType)
{
    switch (vertType)
    {
        case POSITION_VERTEX:
            return VERTEX_DIMENSION_3;

        case TEXTURE_VERTEX:
            return  VERTEX_DIMENSION_2;

        case PACKED_TEXTURE_VERTEX:
            return  VERTEX_DIMENSION_2;

        case COLOR_VERTEX:
            return VERTEX_DIMENSION_4;

        case NORMAL_VERTEX:
            return VERTEX_DIMENSION_3;

        case TANGENT_VERTEX:
            return VERTEX_DIMENSION_3;

        case BITANGENT_VERTEX:
            return VERTEX_DIMENSION_3;

        case PACKED_NORMAL_VERTEX:
            return VERTEX_DIMENSION_1;

        case PACKED_TANGENT_VERTEX:
            return VERTEX_DIMENSION_1;

        case PACKED_BITANGENT_VERTEX:
            return VERTEX_DIMENSION_1;

        case MODEL_MAT_VERTEX:
            return VERTEX_DIMENSION_4;

        case BONE_ID_VERTEX:
            return VERTEX_DIMENSION_4;

        case PACKED_BONE_ID_VERTEX:
            return VERTEX_DIMENSION_4;

        case BONE_WEIGHT_VERTEX:
            return VERTEX_DIMENSION_4;

        case PACKED_BONE_WEIGHT_VERTEX:
            return VERTEX_DIMENSION_4;

        case INDEX_VERTEX:
            return VERTEX_DIMENSION_1;

        case BBOX_TRR_VERTEX:
            return VERTEX_DIMENSION_3;

        case BBOX_BFL_VERTEX:
            return VERTEX_DIMENSION_3;

        default:
            break;
    }

    return SL_Dimension::VERTEX_DIMENSION_1;
}



/*-------------------------------------
 * Determine the basic data type for a common vertex
-------------------------------------*/
SL_DataType sl_type_of_vertex(const SL_CommonVertType vertType)
{
    switch (vertType)
    {
        case POSITION_VERTEX:
            return VERTEX_DATA_FLOAT;

        case TEXTURE_VERTEX:
            return VERTEX_DATA_FLOAT;

        case PACKED_TEXTURE_VERTEX:
            return VERTEX_DATA_SHORT;

        case COLOR_VERTEX:
            return VERTEX_DATA_FLOAT;

        case NORMAL_VERTEX:
            return VERTEX_DATA_FLOAT;

        case TANGENT_VERTEX:
            return VERTEX_DATA_FLOAT;

        case BITANGENT_VERTEX:
            return VERTEX_DATA_FLOAT;

        case PACKED_NORMAL_VERTEX:
            return VERTEX_DATA_INT;

        case PACKED_TANGENT_VERTEX:
            return VERTEX_DATA_INT;

        case PACKED_BITANGENT_VERTEX:
            return VERTEX_DATA_INT;

        case MODEL_MAT_VERTEX:
            return VERTEX_DATA_FLOAT;

        case BONE_ID_VERTEX:
            return VERTEX_DATA_INT;

        case PACKED_BONE_ID_VERTEX:
            return VERTEX_DATA_SHORT;

        case BONE_WEIGHT_VERTEX:
            return VERTEX_DATA_FLOAT;

        case PACKED_BONE_WEIGHT_VERTEX:
            return VERTEX_DATA_SHORT;

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
const char* const* sl_common_vertex_names() noexcept
{
    static const char* const names[] = {
        VERT_ATTRIB_NAME_POSITION,
        VERT_ATTRIB_NAME_TEXTURE,
        VERT_ATTRIB_NAME_PACKED_TEXTURE,
        VERT_ATTRIB_NAME_COLOR,
        VERT_ATTRIB_NAME_NORMAL,
        VERT_ATTRIB_NAME_TANGENT,
        VERT_ATTRIB_NAME_BITANGENT,
        VERT_ATTRIB_NAME_PACKED_NORMAL,
        VERT_ATTRIB_NAME_PACKED_TANGENT,
        VERT_ATTRIB_NAME_PACKED_BITANGENT,
        VERT_ATTRIB_NAME_MODEL_MATRIX,
        VERT_ATTRIB_NAME_BONE_ID,
        VERT_ATTRIB_NAME_PACKED_BONE_ID,
        VERT_ATTRIB_NAME_BONE_WEIGHT,
        VERT_ATTRIB_NAME_PACKED_BONE_WEIGHT,
        VERT_ATTRIB_NAME_INDEX,
        VERT_ATTRIB_NAME_BBOX_TRR,
        VERT_ATTRIB_NAME_BBOX_BFL
    };

    // Because I still don't trust myself...
    static_assert(
        LS_ARRAY_SIZE(names) == SL_NUM_COMMON_VERTEX_FLAGS,
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
SL_DataType sl_required_index_type(const unsigned numVertices)
{
    return (numVertices <= std::numeric_limits<unsigned char>::max())
           ? SL_DataType::VERTEX_DATA_BYTE
           : (numVertices <= std::numeric_limits<unsigned short>::max())
             ? SL_DataType::VERTEX_DATA_SHORT
             : SL_DataType::VERTEX_DATA_INT;
}



/*-------------------------------------
 * Get the number of bytes required to store a set of indices.
-------------------------------------*/
unsigned sl_index_byte_size(const SL_DataType indexType)
{
    switch (indexType)
    {
        case SL_DataType::VERTEX_DATA_BYTE:
            return sizeof (unsigned char);

        case SL_DataType::VERTEX_DATA_SHORT:
            return sizeof (unsigned short);

        case SL_DataType::VERTEX_DATA_INT:
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
void sl_draw_colored_line_bresenham(SL_ColorRGB8* const pImg, coord_shrt_t w, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SL_ColorRGB8& color) noexcept
{
    sl_draw_line_bresenham(x1, y1, x2, y2, [&](coord_shrt_t x, coord_shrt_t y)->void
    {
        sl_draw_pixel(pImg, w, x, y, color);
    });
}



/*-------------------------------------
 * Line Drawing: EFLA (Variant 5)
-------------------------------------*/
void sl_draw_colored_line_efla5(SL_ColorRGB8* pImg, coord_shrt_t width, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SL_ColorRGB8& color) noexcept
{
    sl_draw_line_efla5(x1, y1, x2, y2, [&](coord_shrt_t x, coord_shrt_t y)->void
    {
        sl_draw_pixel(pImg, width, x, y, color);
    });
}



/*-------------------------------------
 * Line Drawing: Bresenham's (Fixed-Point)
-------------------------------------*/
void sl_draw_colored_line_fixed(SL_ColorRGB8* const pImg, coord_shrt_t width, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SL_ColorRGB8& color) noexcept
{
    sl_draw_line_fixed(x1, y1, x2, y2, [&](coord_shrt_t x, coord_shrt_t y)->void
    {
        sl_draw_pixel(pImg, width, x, y, color);
    });
}



/*-----------------------------------------------------------------------------
 * Vertex Information Algorithms
-----------------------------------------------------------------------------*/
/*-------------------------------------
    Calculate the normal vector of a triangle
-------------------------------------*/
math::vec3 sl_calc_normal(const math::vec3& v0, const math::vec3& v1, const math::vec3& v2) noexcept
{
    const math::vec3&& a = v1 - v0;
    const math::vec3&& b = v2 - v0;

    return math::normalize(math::cross(a, b));
}



/*-------------------------------------
    Calculate the normal vector of a triangle
-------------------------------------*/
math::vec4 sl_calc_normal(const math::vec4& v0, const math::vec4& v1, const math::vec4& v2) noexcept
{
    const math::vec4&& a = v1 - v0;
    const math::vec4&& b = v2 - v0;

    return math::normalize(math::cross(a, b));
}



/*-------------------------------------
 * Calculate the tangents for a set of triangles (placed in a vertex array).
-------------------------------------*/
void sl_calc_tangents(
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
math::vec3 sl_calc_tangent(
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
math::vec3 sl_calc_bitangent(
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
