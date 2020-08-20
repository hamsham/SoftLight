
#ifndef SL_GEOMETRY_HPP
#define SL_GEOMETRY_HPP

#include "lightsky/setup/Macros.h"

#include "lightsky/math/vec_utils.h"

#include "softlight/SL_Setup.hpp"
#include "softlight/SL_Color.hpp"



/*--------------------------------------
 * Vertex Dimension Specifiers
--------------------------------------*/
enum SL_Dimension : unsigned
{
    VERTEX_DIMENSION_1 = 0,
    VERTEX_DIMENSION_2,
    VERTEX_DIMENSION_3,
    VERTEX_DIMENSION_4
};



/*--------------------------------------
 * Vertex Data Types
--------------------------------------*/
enum SL_DataType
{
    VERTEX_DATA_BYTE,
    VERTEX_DATA_SHORT,
    VERTEX_DATA_INT,
    VERTEX_DATA_LONG,
    VERTEX_DATA_FLOAT,
    VERTEX_DATA_DOUBLE,
    VERTEX_DATA_INVALID
};



/*--------------------------------------
 * Vertex Data Types
--------------------------------------*/
enum SL_BoneInfo : unsigned
{
    SL_BONE_MAX_WEIGHTS = 4
};



struct SL_BoneData
{
    union
    {
        ls::math::vec4_t<uint32_t> ids32;
        ls::math::vec4_t<uint16_t> ids16;
    };

    union
    {
        ls::math::vec4_t<float> weights32;
        ls::math::vec4_t<ls::math::half> weights16;
    };
};



/*-------------------------------------
 * Common vertex types, Natively supported by the renderer.
-------------------------------------*/
enum SL_CommonVertType : uint32_t
{
    POSITION_VERTEX           = 0x00000001,
    TEXTURE_VERTEX            = 0x00000002,
    PACKED_TEXTURE_VERTEX     = 0x00000004,
    COLOR_VERTEX              = 0x00000008,
    NORMAL_VERTEX             = 0x00000010,
    TANGENT_VERTEX            = 0x00000020,
    BITANGENT_VERTEX          = 0x00000040,
    PACKED_NORMAL_VERTEX      = 0x00000080,
    PACKED_TANGENT_VERTEX     = 0x00000100,
    PACKED_BITANGENT_VERTEX   = 0x00000200,
    MODEL_MAT_VERTEX          = 0x00000400,
    BONE_ID_VERTEX            = 0x00000800,
    PACKED_BONE_ID_VERTEX     = 0x00001000,
    BONE_WEIGHT_VERTEX        = 0x00002000,
    PACKED_BONE_WEIGHT_VERTEX = 0x00004000,
    INDEX_VERTEX              = 0x00008000,
    BBOX_TRR_VERTEX           = 0x00010000,
    BBOX_BFL_VERTEX           = 0x00020000,


    /**
     * @brief A standard vertex is the most commonly supported collection of
     * individual base types within the rendering framework.
     */
    STANDARD_VERTEX = (
        0
        | POSITION_VERTEX
        | TEXTURE_VERTEX
        | NORMAL_VERTEX
        | 0),

    /**
     * @brief A bone vertex allows for a single vertex type to contain enough
     * information to perform skeletal animation/skinning.
     */
    BONE_VERTEX = (
        0
        | BONE_ID_VERTEX
        | BONE_WEIGHT_VERTEX
        | 0),

    /**
     * @brief An Occlusion vertex only provides information about positions to
     * GLSL. Occlusion vertices are intended to have a certain number of
     * position components which are instanced and translated in GLSL by a
     * model matrix.
     */
    OCCLUSION_VERTEX = (
        0
        | POSITION_VERTEX
        | BBOX_TRR_VERTEX
        | BBOX_BFL_VERTEX
        | 0),
};

constexpr SL_CommonVertType SL_COMMON_VERTEX_FLAGS[] = {
    SL_CommonVertType::POSITION_VERTEX,

    SL_CommonVertType::TEXTURE_VERTEX,
    SL_CommonVertType::PACKED_TEXTURE_VERTEX,

    SL_CommonVertType::COLOR_VERTEX,

    SL_CommonVertType::NORMAL_VERTEX,
    SL_CommonVertType::TANGENT_VERTEX,
    SL_CommonVertType::BITANGENT_VERTEX,

    SL_CommonVertType::PACKED_NORMAL_VERTEX,
    SL_CommonVertType::PACKED_TANGENT_VERTEX,
    SL_CommonVertType::PACKED_BITANGENT_VERTEX,

    SL_CommonVertType::MODEL_MAT_VERTEX,

    SL_CommonVertType::BONE_ID_VERTEX,
    SL_CommonVertType::PACKED_BONE_ID_VERTEX,
    SL_CommonVertType::BONE_WEIGHT_VERTEX,
    SL_CommonVertType::PACKED_BONE_WEIGHT_VERTEX,

    SL_CommonVertType::INDEX_VERTEX,

    SL_CommonVertType::BBOX_TRR_VERTEX,
    SL_CommonVertType::BBOX_BFL_VERTEX
};

constexpr unsigned SL_NUM_COMMON_VERTEX_FLAGS = LS_ARRAY_SIZE(SL_COMMON_VERTEX_FLAGS);



/*-----------------------------------------------------------------------------
 * Vertex helper function
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Determine the number of pixels required to store a scalar mType
-------------------------------------*/
uint32_t sl_bytes_per_type(SL_DataType t);



/*-------------------------------------
 * Determine the number of pixels required to store a vertex object
-------------------------------------*/
uint32_t sl_bytes_per_vertex(SL_DataType t, SL_Dimension d);



/**------------------------------------
 * @brief Retrieve the offset to a particular attribute within a vertex who's
 * layout is described with the SL_CommonVertType enumeration. This function is
 * essentially an 'offsetof()' replacement for flexible vertex formats.
 *
 * @param vertFlags
 * A bitmask of SL_CommonVertType flags, representing all of the vertex elements
 * within a vertex buffer.
 *
 * @param mask
 * A single value from the SL_CommonVertType enumeration which identifies the
 * particular vertex attribute which should be queried for a byte-offset.
 *
 * @return A pointer representing the number of bytes that pad a vertex until
 * a particular attribute is reached.
-------------------------------------*/
unsigned sl_vertex_attrib_offset(const SL_CommonVertType vertFlags, const SL_CommonVertType mask);



/*-------------------------------------
 * Count the number of active vertex attributes
-------------------------------------*/
unsigned sl_count_vertex_attribs(const SL_CommonVertType vertAttribs);



/*-------------------------------------
 * Get the Nth active vertex attributes in an attribute bitmask.
-------------------------------------*/
SL_CommonVertType sl_get_vertex_attrib(const SL_CommonVertType vertAttribs, unsigned index);



/*-------------------------------------
 * Determine the number of dimensions used by a vertex type
-------------------------------------*/
SL_Dimension sl_dimens_of_vertex(const SL_CommonVertType vertType);



/*-------------------------------------
 * Determine the basic data type for a common vertex
-------------------------------------*/
SL_DataType sl_type_of_vertex(const SL_CommonVertType vertType);



/**------------------------------------
 * @brief Determine the number of bytes required to store one or more vertices
 * within a flexible-vertex-format.
 *
 * @param vertexTypes
 * A bitmask, containing the types of vertices which will be used in the output
 * byte-size calculation.
 *
 * @return The size, in bytes, of a flexible-vertex-format.
-------------------------------------*/
inline unsigned sl_vertex_byte_size(const SL_CommonVertType vertexTypes)
{
    return sl_vertex_attrib_offset(vertexTypes, (SL_CommonVertType)0);
}



inline unsigned sl_vertex_stride(const SL_CommonVertType vertexTypes)
{
    return sl_vertex_byte_size(vertexTypes);
}




/*-------------------------------------
 * @brief the common vertex names array helps to keep track of all vertex names
 * and make iteration over them easier in client code.
-------------------------------------*/
const char* const* sl_common_vertex_names() noexcept;

constexpr unsigned SL_NUM_COMMON_VERTEX_NAMES = SL_NUM_COMMON_VERTEX_FLAGS;



/*-----------------------------------------------------------------------------
 * Indexed vertex helper function
-----------------------------------------------------------------------------*/
/**------------------------------------
 * @brief Determine the minimum required storage format needed to render a
 * number of vertices within OpenGL.
 *
 * @param numVertices
 * An unsigned integral type, containing the number of vertices which are to be
 * rendered using indexed draw calls.
 *
 * @return An SL_IndexDataType, containing either INDEX_TYPE_USHORT or
 * INDEX_TYPE_UINT, based on the number of vertices passed into the function.
-------------------------------------*/
SL_DataType sl_required_index_type(const unsigned numVertices);



/**------------------------------------
 * @brief Determine the number of bytes required to store one or more indices
 * within a flexible-index-format.
 *
 * @param indexType
 * An enumeration containing the types of indices which will be used in the
 * output byte-size calculation.
 *
 * @return The size, in bytes, of a flexible-index-format.
-------------------------------------*/
unsigned sl_index_byte_size(const SL_DataType indexType);



/*------------------------------------------------------------------------------
 * Basic drawing algorithms
------------------------------------------------------------------------------*/
/*-------------------------------------
 * Apply SL_Colors to an Image
-------------------------------------*/
static inline void sl_draw_pixel(SL_ColorRGB8* const p, const coord_shrt_t w, coord_shrt_t x, coord_shrt_t y, const SL_ColorRGB8& color)
{
    p[w * y + x] = color;
}



/*-------------------------------------
 * Line Drawing: Bresenham Base Case
-------------------------------------*/
void sl_draw_line_bresenham(SL_ColorRGB8* const pImg, coord_shrt_t w, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SL_ColorRGB8& color);



/*-------------------------------------
 * Line Drawing: EFLA (Variant 5)
-------------------------------------*/
void sl_draw_line_efla5(SL_ColorRGB8* pImg, coord_shrt_t width, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SL_ColorRGB8& color);



/*-------------------------------------
 * Line Drawing: Bresenham's (Fixed-Point)
-------------------------------------*/
void sl_draw_line_fixed(SL_ColorRGB8* const pImg, coord_shrt_t w, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SL_ColorRGB8& color);



/*-----------------------------------------------------------------------------
 * Triangle Feature Algorithms
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * 2D Barycentric Coordinates
-------------------------------------*/
template <typename data_t>
inline ls::math::vec3_t<data_t> sl_barycentric(
    const ls::math::vec2_t<data_t>& p,
    const ls::math::vec2_t<data_t>& a,
    const ls::math::vec2_t<data_t>& b,
    const ls::math::vec2_t<data_t>& c
) noexcept
{
    const ls::math::vec4_t<data_t>   u = {c[0]-a[0], b[0]-a[0], a[0]-p[0], (data_t)0};
    const ls::math::vec4_t<data_t>   v = {c[1]-a[1], b[1]-a[1], a[1]-p[1], (data_t)0};
    const ls::math::vec4_t<data_t>&& s = ls::math::cross<data_t>(u, v);

    const data_t si = ls::math::rcp<data_t>(s[2]);
    const data_t s2 = s[0] * si;
    const data_t s1 = s[1] * si;
    const data_t s0 = (data_t)1 - (s1+s2);

    return ls::math::vec3_t<data_t>{s0, s1, s2};
}



template <typename data_t>
inline ls::math::vec3_t<data_t> sl_barycentric(
    const ls::math::vec3_t<data_t>& p,
    const ls::math::vec3_t<data_t>& a,
    const ls::math::vec3_t<data_t>& b,
    const ls::math::vec3_t<data_t>& c
) noexcept
{
    const ls::math::vec4_t<data_t>   u = {c[0]-a[0], b[0]-a[0], a[0]-p[0], (data_t)0};
    const ls::math::vec4_t<data_t>   v = {c[1]-a[1], b[1]-a[1], a[1]-p[1], (data_t)0};
    const ls::math::vec4_t<data_t>&& s = ls::math::cross<data_t>(u, v);

    const data_t si = ls::math::rcp<data_t>(s[2]);
    const data_t s2 = s[0] * si;
    const data_t s1 = s[1] * si;
    const data_t s0 = (data_t)1 - (s1+s2);

    return ls::math::vec3_t<data_t>{s0, s1, s2};
}



template <typename data_t>
inline ls::math::vec4_t<data_t> sl_barycentric(
    const ls::math::vec4_t<data_t>& p,
    const ls::math::vec4_t<data_t>& a,
    const ls::math::vec4_t<data_t>& b,
    const ls::math::vec4_t<data_t>& c
) noexcept
{
    const ls::math::vec4_t<data_t>   u = {c[0]-a[0], b[0]-a[0], a[0]-p[0], 0.f};
    const ls::math::vec4_t<data_t>   v = {c[1]-a[1], b[1]-a[1], a[1]-p[1], 0.f};
    const ls::math::vec4_t<data_t>&& s = ls::math::cross<data_t>(u, v);

    const data_t si = ls::math::rcp<data_t>(s[2]);
    const data_t s2 = s[0] * si;
    const data_t s1 = s[1] * si;
    const data_t s0 = (data_t)1 - (s1+s2);

    return ls::math::vec4_t<data_t>{s0, s1, s2, (data_t)0};
}



/*-----------------------------------------------------------------------------
 * Vertex Information Algorithms
-----------------------------------------------------------------------------*/
/**------------------------------------
 * Helper function to calculate a vertex normal from 3 vertices.
 *
 * @param v0
 * @param v1
 * @param v2
 *
 * @return A 3-dimensional vector that represents a vertex normal.
-------------------------------------*/
ls::math::vec3 sl_calc_normal(
    const ls::math::vec3& v0,
    const ls::math::vec3& v1,
    const ls::math::vec3& v2
);

/**------------------------------------
 * Calculate the tangents for a set of triangles (placed in a vertex array).
 *
 * @param vertCount
 * The number of vertices in the input arrays.
 *
 * @param positions
 * A pointer to an array (of size 'vertCount') of 3D vectors, representing the
 * positions of a set of triangles.
 *
 * @param uvs
 * A pointer to an array (of size 'vertCount') of 3D vectors, representing the
 * uvs of a set of triangles.
 *
 * @param tangents
 * A pointer to an array (of size 'vertCount') of 3D vectors, representing the
 * tangents of a set of triangles.
 *
 * @param bitangents
 * A pointer to an array (of size 'vertCount') of 3D vectors, representing the
 * bitangents of a set of triangles.
 *
-------------------------------------*/
void sl_calc_tangents(
    unsigned vertCount,
    const ls::math::vec3* const positions,
    const ls::math::vec2* const uvs,
    ls::math::vec3* tangents,
    ls::math::vec3* bitangents
);

/**------------------------------------
 * @brief Calculate the tangents for a set of triangles (placed in a vertex
 * array).
 *
 * @param pos0
 * The first vertex of a triangle in model-space.
 *
 * @param pos1
 * The second vertex of a triangle in model-space.
 *
 * @param pos2
 * The third vertex of a triangle in model-space.
 *
 * @param uv0
 * The third UV coordinate of a triangle in texture-space.
 *
 * @param uv1
 * The third UV coordinate of a triangle in texture-space.
 *
 * @param uv2
 * The third UV coordinate of a triangle in texture-space.
 *
 * @return A 3D vector, representing the tangent coordinate of a textured
 * triangle.
-------------------------------------*/
ls::math::vec3 sl_calc_tangent(
    const ls::math::vec3& pos0, const ls::math::vec3& pos1, const ls::math::vec3& pos2,
    const ls::math::vec2& uv0, const ls::math::vec2& uv1, const ls::math::vec2& uv2
);

/**------------------------------------
 * @brief Calculate the tangents for a set of triangles (placed in a vertex
 * array).
 *
 * @param pos0
 * The first vertex of a triangle in model-space.
 *
 * @param pos1
 * The second vertex of a triangle in model-space.
 *
 * @param pos2
 * The third vertex of a triangle in model-space.
 *
 * @param uv0
 * The third UV coordinate of a triangle in texture-space.
 *
 * @param uv1
 * The third UV coordinate of a triangle in texture-space.
 *
 * @param uv2
 * The third UV coordinate of a triangle in texture-space.
 *
 * @return A 3D vector, representing the bi-tangent coordinate of a textured
 * triangle.
-------------------------------------*/
ls::math::vec3 sl_calc_bitangent(
    const ls::math::vec3& pos0, const ls::math::vec3& pos1, const ls::math::vec3& pos2,
    const ls::math::vec2& uv0, const ls::math::vec2& uv1, const ls::math::vec2& uv2
);



#endif /* SL_GEOMETRY_HPP */
