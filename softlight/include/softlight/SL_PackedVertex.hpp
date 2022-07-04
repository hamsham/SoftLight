
#ifndef SL_PACKED_VERTEX_HPP
#define SL_PACKED_VERTEX_HPP

#include <cstdint>

#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"



/**------------------------------------
 * @brief SL_PackedVertex_2_10_10_10
 * This is a Vertex Packing Structure which can convert a vertex normal to the
 * OpenGL-compatible GL_UNSIGNED_INT_2_10_10_10_REV integer format. This
 * format can reduce memory from 66% (for 3D vectors) to 75% (4D vectors) per
 * vertex.
 *
 * It is recommended to only use this for vertex normals, tangents, or
 * bi-tangents as there is significant precision loss for values outside of
 * the range (-1, 1).
-------------------------------------*/
struct alignas(sizeof(int32_t)) SL_PackedVertex_2_10_10_10
{
    int32_t x: 10;
    int32_t y: 10;
    int32_t z: 10;
    int32_t w: 2;

    constexpr LS_INLINE SL_PackedVertex_2_10_10_10(const int32_t& v) noexcept :
        x{(v >>  0) & 0x03FF},
        y{(v >> 10) & 0x03FF},
        z{(v >> 20) & 0x03FF},
        w{0}
    {}

    constexpr LS_INLINE SL_PackedVertex_2_10_10_10(const ls::math::vec3& v) noexcept :
        x{(int32_t)(v[0] * 511.f)},
        y{(int32_t)(v[1] * 511.f)},
        z{(int32_t)(v[2] * 511.f)},
        w{0}
    {}

    inline LS_INLINE SL_PackedVertex_2_10_10_10(const ls::math::vec4& v) noexcept :
        x{(int32_t)(v[0] * 511.f)},
        y{(int32_t)(v[1] * 511.f)},
        z{(int32_t)(v[2] * 511.f)},
        w{0}
    {}
    
    explicit inline LS_INLINE operator int32_t() const noexcept
    {
        return *reinterpret_cast<const int32_t*>(this);
    }

    explicit constexpr LS_INLINE operator ls::math::vec3() const noexcept
    {
        return ls::math::vec3{
            (float)x * (1.f / 511.f),
            (float)y * (1.f / 511.f),
            (float)z * (1.f / 511.f)
        };
    }

    explicit inline LS_INLINE operator ls::math::vec4() const noexcept
    {
        // BEWARE: Undefined behavior ahead
        #if defined(LS_X86_AVX2)
            const __m128i elems    = _mm_castps_si128(_mm_broadcast_ss(reinterpret_cast<const float*>(this)));
            const __m128i shifted  = _mm_sllv_epi32(elems,   _mm_set_epi32(0, 2, 12, 22));
            const __m128i extended = _mm_srav_epi32(shifted, _mm_set_epi32(30, 22, 22, 22));
            return ls::math::vec4{_mm_mul_ps(_mm_cvtepi32_ps(extended), _mm_set1_ps(1.f/511.f))};

        #elif defined(LS_X86_SSSE3)
            const __m128i elems    = _mm_castps_si128(_mm_load1_ps(reinterpret_cast<const float*>(this)));
            const __m128i a        = _mm_shuffle_epi8(_mm_slli_epi32(elems, 2),  _mm_set_epi8(-1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1));
            const __m128i b        = _mm_shuffle_epi8(_mm_slli_epi32(elems, 12), _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1));
            const __m128i c        = _mm_shuffle_epi8(_mm_slli_epi32(elems, 22), _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12));
            const __m128i shifted  = _mm_or_si128(_mm_or_si128(c, b),  a);
            const __m128i extended = _mm_srai_epi32(shifted, 22);
            return ls::math::vec4{_mm_mul_ps(_mm_cvtepi32_ps(extended), _mm_set1_ps(1.f/511.f))};

        #elif defined(LS_ARM_NEON)
            const int32x4_t leftShifts{22, 12, 2, 0};
            const int32x4_t rightShifts{-22, -22, -22, -30};
            const int32x4_t elems    = vmovq_n_s32(*reinterpret_cast<const int32_t*>(this));
            const int32x4_t shifted  = vshlq_s32(elems, leftShifts);
            const int32x4_t extended = vshlq_s32(shifted, rightShifts);
            return ls::math::vec4{vmulq_f32(vcvtq_f32_s32(extended), vdupq_n_f32(1.f/511.f))};
        #else
            return ls::math::vec4{(float)x, (float)y, (float)z, 0.f} * ls::math::vec4{1.f / 511.f, 1.f / 511.f, 1.f / 511.f, 0.f};

        #endif
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
inline int32_t sl_pack_vertex_2_10_10_10(const ls::math::vec3& norm) noexcept
{
    return (int32_t)SL_PackedVertex_2_10_10_10{norm};
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
inline LS_INLINE int32_t sl_pack_vertex_2_10_10_10(const ls::math::vec4& norm) noexcept
{
    return (int32_t)SL_PackedVertex_2_10_10_10{norm};
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
constexpr LS_INLINE ls::math::vec3 sl_unpack_vertex_vec3(int32_t norm) noexcept
{
    return (ls::math::vec3)SL_PackedVertex_2_10_10_10{norm};
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
inline LS_INLINE ls::math::vec4 sl_unpack_vertex_vec4(int32_t norm) noexcept
{
    return (ls::math::vec4)SL_PackedVertex_2_10_10_10{norm};
}



#endif /* SL_PACKED_VERTEX_HPP */
