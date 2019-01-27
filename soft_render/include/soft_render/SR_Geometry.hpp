
#ifndef SR_GEOMETRY_HPP
#define SR_GEOMETRY_HPP

#include "lightsky/setup/Macros.h"

#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_Setup.hpp"
#include "soft_render/SR_Color.hpp"



/*--------------------------------------
 * Vertex Dimension Specifiers
--------------------------------------*/
enum SR_Dimension : unsigned
{
    VERTEX_DIMENSION_1 = 0,
    VERTEX_DIMENSION_2,
    VERTEX_DIMENSION_3,
    VERTEX_DIMENSION_4
};



/*--------------------------------------
 * Vertex Data Types
--------------------------------------*/
enum SR_DataType
{
    VERTEX_DATA_BYTE,
    VERTEX_DATA_SHORT,
    VERTEX_DATA_INT,
    VERTEX_DATA_LONG,
    VERTEX_DATA_FLOAT,
    VERTEX_DATA_DOUBLE,
    VERTEX_DATA_INVALID
};



/*-------------------------------------
 * Common vertex types, Natively supported by the renderer.
-------------------------------------*/
enum SR_CommonVertType : uint32_t
{
    POSITION_VERTEX     = 0x10000000,
    TEXTURE_VERTEX      = 0x20000000,
    COLOR_VERTEX        = 0x40000000,

    NORMAL_VERTEX       = 0x80000000,
    TANGENT_VERTEX      = 0x01000000,
    BITANGENT_VERTEX    = 0x02000000,

    MODEL_MAT_VERTEX    = 0x04000000,

    BONE_ID_VERTEX      = 0x08000000,
    BONE_WEIGHT_VERTEX  = 0x00100000,

    AMBIENT_VERTEX      = 0x00200000,
    DIFFUSE_VERTEX      = 0x00400000,
    SPECULAR_VERTEX     = 0x00800000,
    ROUGHNESS_VERTEX    = 0x00010000,
    METALLIC_VERTEX     = 0x00020000,

    INDEX_VERTEX        = 0x00040000,
    BBOX_TRR_VERTEX     = 0x00080000,
    BBOX_BFL_VERTEX     = 0x00001000,


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

constexpr SR_CommonVertType SR_COMMON_VERTEX_FLAGS[] = {
    SR_CommonVertType::POSITION_VERTEX,
    SR_CommonVertType::TEXTURE_VERTEX,
    SR_CommonVertType::COLOR_VERTEX,

    SR_CommonVertType::NORMAL_VERTEX,
    SR_CommonVertType::TANGENT_VERTEX,
    SR_CommonVertType::BITANGENT_VERTEX,

    SR_CommonVertType::MODEL_MAT_VERTEX,

    SR_CommonVertType::BONE_ID_VERTEX,
    SR_CommonVertType::BONE_WEIGHT_VERTEX,

    SR_CommonVertType::AMBIENT_VERTEX,
    SR_CommonVertType::DIFFUSE_VERTEX,
    SR_CommonVertType::SPECULAR_VERTEX,
    SR_CommonVertType::ROUGHNESS_VERTEX,
    SR_CommonVertType::METALLIC_VERTEX,

    SR_CommonVertType::INDEX_VERTEX,

    SR_CommonVertType::BBOX_TRR_VERTEX,
    SR_CommonVertType::BBOX_BFL_VERTEX
};

constexpr unsigned SR_NUM_COMMON_VERTEX_FLAGS = LS_ARRAY_SIZE(SR_COMMON_VERTEX_FLAGS);



/*-----------------------------------------------------------------------------
 * Vertex helper function
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Determine the number of pixels required to store a scalar mType
-------------------------------------*/
uint32_t sr_bytes_per_type(SR_DataType t);



/*-------------------------------------
 * Determine the number of pixels required to store a vertex object
-------------------------------------*/
uint32_t sr_bytes_per_vertex(SR_DataType t, SR_Dimension d);



/**------------------------------------
 * @brief Retrieve the offset to a particular attribute within a vertex who's
 * layout is described with the SR_CommonVertType enumeration. This function is
 * essentially an 'offsetof()' replacement for flexible vertex formats.
 *
 * @param vertFlags
 * A bitmask of SR_CommonVertType flags, representing all of the vertex elements
 * within a vertex buffer.
 *
 * @param mask
 * A single value from the SR_CommonVertType enumeration which identifies the
 * particular vertex attribute which should be queried for a byte-offset.
 *
 * @return A pointer representing the number of bytes that pad a vertex until
 * a particular attribute is reached.
-------------------------------------*/
unsigned sr_vertex_attrib_offset(const SR_CommonVertType vertFlags, const SR_CommonVertType mask);



/*-------------------------------------
 * Determine the number of dimensions used by a vertex type
-------------------------------------*/
SR_Dimension sr_dimens_of_vertex(const SR_CommonVertType vertType);



/*-------------------------------------
 * Determine the basic data type for a common vertex
-------------------------------------*/
SR_DataType sr_type_of_vertex(const SR_CommonVertType vertType);



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
inline unsigned sr_vertex_byte_size(const SR_CommonVertType vertexTypes)
{
    return sr_vertex_attrib_offset(vertexTypes, (SR_CommonVertType)0);
}



inline unsigned sr_vertex_stride(const SR_CommonVertType vertexTypes)
{
    return sr_vertex_byte_size(vertexTypes);
}




/*-------------------------------------
 * @brief the common vertex names array helps to keep track of all vertex names
 * and make iteration over them easier in client code.
-------------------------------------*/
const char* const* sr_common_vertex_names() noexcept;

constexpr unsigned SR_NUM_COMMON_VERTEX_NAMES = SR_NUM_COMMON_VERTEX_FLAGS;



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
 * @return An SR_IndexDataType, containing either INDEX_TYPE_USHORT or
 * INDEX_TYPE_UINT, based on the number of vertices passed into the function.
-------------------------------------*/
SR_DataType sr_required_index_type(const unsigned numVertices);



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
unsigned sr_index_byte_size(const SR_DataType indexType);



/*------------------------------------------------------------------------------
 * Basic drawing algorithms
------------------------------------------------------------------------------*/
/*-------------------------------------
 * Apply SR_Colors to an Image
-------------------------------------*/
static inline void sr_draw_pixel(SR_ColorRGB8* const p, const coord_shrt_t w, coord_shrt_t x, coord_shrt_t y, const SR_ColorRGB8& color)
{
    p[w * y + x] = color;
}



/*-------------------------------------
 * Line Drawing: Bresenham Base Case
-------------------------------------*/
void sr_draw_line_bresenham(SR_ColorRGB8* const pImg, coord_shrt_t w, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SR_ColorRGB8& color);



/*-------------------------------------
 * Line Drawing: EFLA (Variant 5)
-------------------------------------*/
void sr_draw_line_efla5(SR_ColorRGB8* pImg, coord_shrt_t width, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SR_ColorRGB8& color);



/*-------------------------------------
 * Line Drawing: Bresenham's (Fixed-Point)
-------------------------------------*/
void sr_draw_line_fixed(SR_ColorRGB8* const pImg, coord_shrt_t w, coord_shrt_t x1, coord_shrt_t y1, coord_shrt_t x2, coord_shrt_t y2, const SR_ColorRGB8& color);



/*-----------------------------------------------------------------------------
 * Triangle Feature Algorithms
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * 2D Barycentric Coordinates
-------------------------------------*/
template <typename data_t>
inline ls::math::vec3_t<data_t> sr_barycentric(
    const ls::math::vec2_t<data_t>& p,
    const ls::math::vec2_t<data_t>& a,
    const ls::math::vec2_t<data_t>& b,
    const ls::math::vec2_t<data_t>& c
) noexcept
{
    const ls::math::vec3_t<data_t>   u = {c[0]-a[0], b[0]-a[0], a[0]-p[0]};
    const ls::math::vec3_t<data_t>   v = {c[1]-a[1], b[1]-a[1], a[1]-p[1]};
    const ls::math::vec3_t<data_t>&& s = ls::math::cross<data_t>(u, v);

    const data_t si = ls::math::rcp<data_t>(s[2]);
    const data_t s2 = s[0] * si;
    const data_t s1 = s[1] * si;
    const data_t s0 = (data_t)1 - (s1+s2);

    return ls::math::vec3_t<data_t>{s0, s1, s2};
}



template <typename data_t>
inline ls::math::vec3_t<data_t> sr_barycentric(
    const ls::math::vec3_t<data_t>& p,
    const ls::math::vec3_t<data_t>& a,
    const ls::math::vec3_t<data_t>& b,
    const ls::math::vec3_t<data_t>& c
) noexcept
{
    const ls::math::vec3_t<data_t>   u = {c[0]-a[0], b[0]-a[0], a[0]-p[0]};
    const ls::math::vec3_t<data_t>   v = {c[1]-a[1], b[1]-a[1], a[1]-p[1]};
    const ls::math::vec3_t<data_t>&& s = ls::math::cross<data_t>(u, v);

    const data_t si = ls::math::rcp<data_t>(s[2]);
    const data_t s2 = s[0] * si;
    const data_t s1 = s[1] * si;
    const data_t s0 = (data_t)1 - (s1+s2);

    return ls::math::vec3_t<data_t>{s0, s1, s2};
}



template <typename data_t>
inline ls::math::vec4_t<data_t> sr_barycentric(
    const ls::math::vec4_t<data_t>& p,
    const ls::math::vec4_t<data_t>& a,
    const ls::math::vec4_t<data_t>& b,
    const ls::math::vec4_t<data_t>& c
) noexcept
{
    const ls::math::vec3_t<data_t>   u = {c[0]-a[0], b[0]-a[0], a[0]-p[0]};
    const ls::math::vec3_t<data_t>   v = {c[1]-a[1], b[1]-a[1], a[1]-p[1]};
    const ls::math::vec3_t<data_t>&& s = ls::math::cross<data_t>(u, v);

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
ls::math::vec3 sr_calc_normal(
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
void sr_calc_tangents(
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
ls::math::vec3 sr_calc_tangent(
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
ls::math::vec3 sr_calc_bitangent(
    const ls::math::vec3& pos0, const ls::math::vec3& pos1, const ls::math::vec3& pos2,
    const ls::math::vec2& uv0, const ls::math::vec2& uv1, const ls::math::vec2& uv2
);



#endif /* SR_GEOMETRY_HPP */

