
#ifndef SR_PACKED_VERTEX_HPP
#define SR_PACKED_VERTEX_HPP

#include <cstdint>

#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"



/**------------------------------------
 * @brief SR_PackedVertex_2_10_10_10
 * This is a Vertex Packing Structure which can convert a vertex normal to the
 * OpenGL-compatible GL_UNSIGNED_INT_2_10_10_10_REV integer format. This
 * format can reduce memory from 66% (for 3D vectors) to 75% (4D vectors) per
 * vertex.
 *
 * It is recommended to only use this for vertex normals, tangents, or
 * bi-tangents as there is significant precision loss for values outside of
 * the range (-1, 1).
-------------------------------------*/
struct alignas(sizeof(int32_t)) SR_PackedVertex_2_10_10_10
{
    int32_t x: 10;
    int32_t y: 10;
    int32_t z: 10;
    int32_t w: 2;

    constexpr SR_PackedVertex_2_10_10_10(const int32_t& v) noexcept :
        x{(v >>  0) & 0x03FF},
        y{(v >> 10) & 0x03FF},
        z{(v >> 20) & 0x03FF},
        w{0}
    {}

    constexpr SR_PackedVertex_2_10_10_10(const ls::math::vec3& v) noexcept :
        x{(int32_t)(v[0] * 511.f)},
        y{(int32_t)(v[1] * 511.f)},
        z{(int32_t)(v[2] * 511.f)},
        w{0}
    {}

    inline SR_PackedVertex_2_10_10_10(const ls::math::vec4& v) noexcept :
        x{(int32_t)(v[0] * 511.f)},
        y{(int32_t)(v[1] * 511.f)},
        z{(int32_t)(v[2] * 511.f)},
        w{0}
    {}
    
    explicit inline operator int32_t() const noexcept
    {
        return *reinterpret_cast<const int32_t*>(this);
    }

    explicit constexpr operator ls::math::vec3() const noexcept
    {
        return ls::math::vec3{
            (float)x * (1.f / 511.f),
            (float)y * (1.f / 511.f),
            (float)z * (1.f / 511.f)
        };
    }

    explicit inline operator ls::math::vec4() const noexcept
    {
        return ls::math::vec4{
            (float)x * (1.f / 511.f),
            (float)y * (1.f / 511.f),
            (float)z * (1.f / 511.f),
            0.f
        };
    }
};



/**------------------------------------
 * @brief Convert a 3-dimensional vertex normal to a packed vertex normal,
 * following the GL_UNSIGNED_INT_2_10_10_10_REV format or similar.
 *
 * @param norm
 * A constant reference to a normalized vector within the range of [-1, 1],
 * inclusive,
 *
 * @return A signed 32-bit integer containing a vertex normal with data in the
 * range of [-2^10, 2^10].
-------------------------------------*/
inline int32_t sr_pack_vertex_2_10_10_10(const ls::math::vec3& norm) noexcept
{
    return (int32_t)SR_PackedVertex_2_10_10_10{norm};
}




/**------------------------------------
 * @brief Convert a 4-dimensional vertex normal to a packed vertex normal,
 * following the GL_UNSIGNED_INT_2_10_10_10_REV format or similar.
 *
 * @param norm
 * A constant reference to a normalized vector within the range of [-1, 1],
 * inclusive,
 *
 * @return A signed 32-bit integer containing a vertex normal with data in the
 * range of [-2^10, 2^10].
-------------------------------------*/
inline int32_t sr_pack_vertex_2_10_10_10(const ls::math::vec4& norm) noexcept
{
    return (int32_t)SR_PackedVertex_2_10_10_10{norm};
}



/**------------------------------------
 * @brief Convert a packed vertex normal type into a 3D vector.
 *
 * @param norm
 * A 32-bit integer containing a vertex packed into the 2x10x10x10 integer
 * format,
 *
 * @return A 3D vector containing the unpacked vertex.
-------------------------------------*/
inline ls::math::vec3 sr_unpack_vertex_vec3(int32_t norm) noexcept
{
    return (ls::math::vec3)SR_PackedVertex_2_10_10_10{norm};
}




/**------------------------------------
 * @brief Convert a packed vertex normal type into a 4D vector.
 *
 * @param norm
 * A 32-bit integer containing a vertex packed into the 2x10x10x10 integer
 * format,
 *
 * @return A 4D vector containing the unpacked vertex.
-------------------------------------*/
inline ls::math::vec4 sr_unpack_vertex_vec4(int32_t norm) noexcept
{
    return (ls::math::vec4)SR_PackedVertex_2_10_10_10{norm};
}



#endif /* SR_PACKED_VERTEX_HPP */
