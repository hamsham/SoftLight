
#ifndef SL_PACKED_VERTEX_HPP
#define SL_PACKED_VERTEX_HPP

#include <cstdint>

#include "lightsky/math/vec_utils.h"



/**------------------------------------
 * @brief SL_PackedVertex_10_10_10_2I
 * This is a Vertex Packing Structure which can convert a vertex normal to the
 * OpenGL-compatible GL_UNSIGNED_INT_2_10_10_10_REV integer format. This
 * format can reduce memory from 66% (for 3D vectors) to 75% (4D vectors) per
 * vertex.
 *
 * It is recommended to only use this for vertex normals, tangents, or
 * bi-tangents as there is significant precision loss for values outside of
 * the range (-1, 1).
-------------------------------------*/
struct alignas(sizeof(int32_t)) SL_PackedVertex_10_10_10_2I
{
    int32_t x: 10;
    int32_t y: 10;
    int32_t z: 10;
    int32_t w: 2;

    explicit constexpr LS_INLINE SL_PackedVertex_10_10_10_2I(const int32_t& v) noexcept :
        x{(v >>  0) & 0x03FF},
        y{(v >> 10) & 0x03FF},
        z{(v >> 20) & 0x03FF},
        w{0}
    {}

    explicit constexpr LS_INLINE SL_PackedVertex_10_10_10_2I(const uint32_t& v) noexcept :
        x{(int32_t)((v >>  0u) & 0x03FFu)},
        y{(int32_t)((v >> 10u) & 0x03FFu)},
        z{(int32_t)((v >> 20u) & 0x03FFu)},
        w{0}
    {}

    constexpr LS_INLINE SL_PackedVertex_10_10_10_2I(const ls::math::vec3& v) noexcept :
        x{(int32_t)(v[0] * 511.f)},
        y{(int32_t)(v[1] * 511.f)},
        z{(int32_t)(v[2] * 511.f)},
        w{0}
    {}

    inline LS_INLINE SL_PackedVertex_10_10_10_2I(const ls::math::vec4& v) noexcept :
        x{(int32_t)(v[0] * 511.f)},
        y{(int32_t)(v[1] * 511.f)},
        z{(int32_t)(v[2] * 511.f)},
        w{0}
    {}

    explicit inline LS_INLINE operator int32_t() const noexcept
    {
        return *reinterpret_cast<const int32_t*>(this);
    }

    explicit inline LS_INLINE operator uint32_t() const noexcept
    {
        return *reinterpret_cast<const uint32_t*>(this);
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

static_assert(sizeof(SL_PackedVertex_10_10_10_2I) == sizeof(int32_t), "Unable to store a SL_PackedVertex_10_10_10_2I type within an int32.");



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
inline int32_t sl_pack_vec3_10_10_10_2I(const ls::math::vec3& norm) noexcept
{
    return (int32_t)SL_PackedVertex_10_10_10_2I{norm};
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
inline LS_INLINE int32_t sl_pack_vec4_10_10_10_2I(const ls::math::vec4& norm) noexcept
{
    #if defined(LS_X86_AVX2)
        const __m128i i = _mm_cvtps_epi32(_mm_mul_ps(norm.simd, _mm_set1_ps(511.f)));
        const __m128i l = _mm_sllv_epi32(i, _mm_set_epi32(32, 20, 10, 0));
        const __m128i r = _mm_and_si128(l, _mm_set_epi32(0, 0x3FF00000, 0x000FFC00, 0x000003FF));
        const __m128i i0 = _mm_or_si128(r, _mm_shuffle_epi32(r, 0xB1));
        const __m128i i1 = _mm_or_si128(i0, _mm_unpackhi_epi32(i0, i0));
        return _mm_cvtsi128_si32(i1);

    #else
        return (int32_t)SL_PackedVertex_10_10_10_2I{norm};
    #endif
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
constexpr LS_INLINE ls::math::vec3 sl_unpack_vec3_10_10_10_2I(int32_t norm) noexcept
{
    return (ls::math::vec3)SL_PackedVertex_10_10_10_2I{norm};
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
inline LS_INLINE ls::math::vec4 sl_unpack_vec4_10_10_10_2I(int32_t norm) noexcept
{
    return (ls::math::vec4)SL_PackedVertex_10_10_10_2I{norm};
}



/**------------------------------------
 * @brief SL_PackedVertex_10_10_10_2U
 * This is a Vertex Packing Structure which can convert a vertex normal to the
 * OpenGL-compatible GL_UNSIGNED_INT_2_10_10_10_REV integer format. This
 * format can reduce memory from 66% (for 3D vectors) to 75% (4D vectors) per
 * vertex.
 *
 * It is recommended to only use this for vertex normals, tangents, or
 * bi-tangents as there is significant precision loss for values outside of
 * the range (-1, 1).
-------------------------------------*/
struct alignas(sizeof(int32_t)) SL_PackedVertex_10_10_10_2U
{
    uint32_t x: 10;
    uint32_t y: 10;
    uint32_t z: 10;
    uint32_t w: 2;

    explicit constexpr LS_INLINE SL_PackedVertex_10_10_10_2U(const int32_t& v) noexcept :
        x{((uint32_t)v >>  0u) & 0x03FFu},
        y{((uint32_t)v >> 10u) & 0x03FFu},
        z{((uint32_t)v >> 20u) & 0x03FFu},
        w{0}
    {}

    explicit constexpr LS_INLINE SL_PackedVertex_10_10_10_2U(const uint32_t& v) noexcept :
        x{(v >>  0u) & 0x03FFu},
        y{(v >> 10u) & 0x03FFu},
        z{(v >> 20u) & 0x03FFu},
        w{0u}
    {}

    constexpr LS_INLINE SL_PackedVertex_10_10_10_2U(const ls::math::vec3& v) noexcept :
        x{(uint32_t)(v[0] * 1023.f)},
        y{(uint32_t)(v[1] * 1023.f)},
        z{(uint32_t)(v[2] * 1023.f)},
        w{0u}
    {}

    inline LS_INLINE SL_PackedVertex_10_10_10_2U(const ls::math::vec4& v) noexcept :
        x{(uint32_t)(v[0] * 1023.f)},
        y{(uint32_t)(v[1] * 1023.f)},
        z{(uint32_t)(v[2] * 1023.f)},
        w{0u}
    {}

    explicit inline LS_INLINE operator int32_t() const noexcept
    {
        return *reinterpret_cast<const int32_t*>(this);
    }

    explicit inline LS_INLINE operator uint32_t() const noexcept
    {
        return *reinterpret_cast<const uint32_t*>(this);
    }

    explicit constexpr LS_INLINE operator ls::math::vec3() const noexcept
    {
        return ls::math::vec3{
            (float)x * (1.f / 1023.f),
            (float)y * (1.f / 1023.f),
            (float)z * (1.f / 1023.f)
        };
    }

    explicit inline LS_INLINE operator ls::math::vec4() const noexcept
    {
        // BEWARE: Undefined behavior ahead
        #if defined(LS_X86_AVX2)
            const __m128i elems    = _mm_castps_si128(_mm_broadcast_ss(reinterpret_cast<const float*>(this)));
            const __m128i extended = _mm_srlv_epi32(elems, _mm_set_epi32(32, 20, 10, 0));
            return ls::math::vec4{_mm_mul_ps(_mm_cvtepi32_ps(extended), _mm_set1_ps(1.f/1023.f))};

        #elif defined(LS_X86_SSSE3)
            const __m128i elems    = _mm_castps_si128(_mm_load1_ps(reinterpret_cast<const float*>(this)));
            const __m128i a        = _mm_shuffle_epi8(_mm_slli_epi32(elems, 2),  _mm_set_epi8(-1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1));
            const __m128i b        = _mm_shuffle_epi8(_mm_slli_epi32(elems, 12), _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1));
            const __m128i c        = _mm_shuffle_epi8(_mm_slli_epi32(elems, 22), _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12));
            const __m128i shifted  = _mm_or_si128(_mm_or_si128(c, b),  a);
            const __m128i extended = _mm_srli_epi32(shifted, 22);
            return ls::math::vec4{_mm_mul_ps(_mm_cvtepi32_ps(extended), _mm_set1_ps(1.f/1024.f))};

        #elif defined(LS_ARM_NEON)
            const uint32x4_t rightShifts{-0, -10, -20, -32};
            const uint32x4_t elems    = vmovq_n_u32(*reinterpret_cast<const uint32_t*>(this));
            const uint32x4_t extended = vshlq_u32(elems, rightShifts);
            return ls::math::vec4{vmulq_f32(vcvtq_f32_u32(extended), vdupq_n_f32(1.f/1023.f))};
        #else
            return ls::math::vec4{(float)x, (float)y, (float)z, 0.f} * ls::math::vec4{1.f / 1023.f, 1.f / 1023.f, 1.f / 1023.f, 0.f};

        #endif
    }
};

static_assert(sizeof(SL_PackedVertex_10_10_10_2U) == sizeof(uint32_t), "Unable to store a SL_PackedVertex_10_10_10_2U type within a uint32.");



/**------------------------------------
 * @brief Convert a 3-dimensional vertex normal to a packed vertex normal,
 * following the GL_UNSIGNED_INT_2_10_10_10_REV format or similar.
 *
 * @param norm
 * A constant reference to a normalized vector within the range of [0, 1],
 * inclusive,
 *
 * @return A signed 32-bit integer containing a vertex normal with data in the
 * range of [-2^10, 2^10].
-------------------------------------*/
inline int32_t sl_pack_vec3_10_10_10_2U(const ls::math::vec3& norm) noexcept
{
    return (int32_t)SL_PackedVertex_10_10_10_2U{norm};
}




/**------------------------------------
 * @brief Convert a 4-dimensional vertex normal to a packed vertex normal,
 * following the GL_UNSIGNED_INT_2_10_10_10_REV format or similar.
 *
 * @param norm
 * A constant reference to a normalized vector within the range of [0, 1],
 * inclusive,
 *
 * @return A signed 32-bit integer containing a vertex normal with data in the
 * range of [-2^10, 2^10].
-------------------------------------*/
inline LS_INLINE int32_t sl_pack_vec4_10_10_10_2U(const ls::math::vec4& norm) noexcept
{
    #if defined(LS_X86_AVX2)
        const __m128i i = _mm_cvtps_epi32(_mm_mul_ps(norm.simd, _mm_set1_ps(511.f)));
        const __m128i l = _mm_sllv_epi32(i, _mm_set_epi32(32, 20, 10, 0));
        const __m128i r = _mm_and_si128(l, _mm_set_epi32(0, 0x3FF00000, 0x000FFC00, 0x000003FF));
        const __m128i i0 = _mm_or_si128(r, _mm_shuffle_epi32(r, 0xB1));
        const __m128i i1 = _mm_or_si128(i0, _mm_unpackhi_epi32(i0, i0));
        return _mm_cvtsi128_si32(i1);

    #else
        return (int32_t)SL_PackedVertex_10_10_10_2U{norm};
    #endif
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
constexpr LS_INLINE ls::math::vec3 sl_unpack_vec3_10_10_10_2U(int32_t norm) noexcept
{
    return (ls::math::vec3)SL_PackedVertex_10_10_10_2U{norm};
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
inline LS_INLINE ls::math::vec4 sl_unpack_vec4_10_10_10_2U(int32_t norm) noexcept
{
    return (ls::math::vec4)SL_PackedVertex_10_10_10_2U{norm};
}





/**------------------------------------
 * @brief SL_PackedVertex_9e5
 * This is a Vertex Packing Structure which can convert a 3d or 4d vector into
 * the GL-compatible GL_UNSIGNED_INT_5_9_9_9_REV integer format. This format
 * can reduce HDR texture memory by  66%.
 *
 * It is recommended to only use this format for decoding 3-component HDR
 * texture data as encoding into the shared-exponent format is not suitable for
 * real-time applications.
-------------------------------------*/
union SL_PackedVertex_9e5
{
    enum SL_RGB9e5Limits : int32_t
    {
        RGB9E5_EXPONENT_BITS        = 5,
        RGB9E5_MANTISSA_BITS        = 9,
        RGB9E5_EXP_BIAS             = 15,
        RGB9E5_MAX_VALID_BIASED_EXP = 31
    };


    uint32_t mRaw;
    struct
    {
        uint32_t r : RGB9E5_MANTISSA_BITS;
        uint32_t g : RGB9E5_MANTISSA_BITS;
        uint32_t b : RGB9E5_MANTISSA_BITS;
        uint32_t biasedexponent : RGB9E5_EXPONENT_BITS;
    } mField;

  private:
    static uint32_t _pack_vector(const ls::math::vec3& rgb) noexcept;

  public:
    constexpr SL_PackedVertex_9e5() noexcept :
        mRaw{0}
    {}

    explicit constexpr LS_INLINE SL_PackedVertex_9e5(const int32_t& v) noexcept :
        mRaw{static_cast<const uint32_t&>(v)}
    {}

    explicit constexpr LS_INLINE SL_PackedVertex_9e5(const uint32_t& v) noexcept :
        mRaw{v}
    {}

    inline SL_PackedVertex_9e5(const ls::math::vec3& rgb) noexcept :
        mRaw{_pack_vector(rgb)}
    {}

    inline SL_PackedVertex_9e5(const ls::math::vec4& rgba) noexcept :
        mRaw{_pack_vector(ls::math::vec3_cast(rgba))}
    {}

    explicit inline LS_INLINE operator int32_t() const noexcept
    {
        return reinterpret_cast<const int32_t&>(this->mRaw);
    }

    explicit inline LS_INLINE operator uint32_t() const noexcept
    {
        return this->mRaw;
    }

    explicit inline operator ls::math::vec3() const noexcept
    {
        const float exponent = (float)(this->mField.biasedexponent - RGB9E5_EXP_BIAS - RGB9E5_MANTISSA_BITS);
        const float scale = std::exp2(exponent);

        return ls::math::vec3
        {
            (float)this->mField.r * scale,
            (float)this->mField.g * scale,
            (float)this->mField.b * scale
        };
    }

    explicit inline operator ls::math::vec4() const noexcept
    {
        const float exponent = (float)(this->mField.biasedexponent - RGB9E5_EXP_BIAS - RGB9E5_MANTISSA_BITS);
        const float scale = std::exp2(exponent);

        return ls::math::vec4
        {
            (float)this->mField.r * scale,
            (float)this->mField.g * scale,
            (float)this->mField.b * scale,
            0.f
        };
    }
};

static_assert(sizeof(SL_PackedVertex_9e5) == sizeof(uint32_t), "Unable to store a SL_PackedVertex_9e5 type within a uint32.");



/**------------------------------------
 * @brief Convert a 3-dimensional vertex normal to a packed 3D vertex/RGB color
 * which follows the GL_UNSIGNED_INT_5_9_9_9_REV format.
 *
 * @param rgb
 * A constant reference to a 3D vector or RGB color.
 *
 * @return A signed 32-bit integer containing a 3D vertex with 9-bits per
 * element and 5 bits for a shared exponent.
-------------------------------------*/
inline int32_t sl_pack_vec3_9e5(const ls::math::vec3& rgb) noexcept
{
    return (int32_t)SL_PackedVertex_9e5{rgb};
}




/**------------------------------------
 * @brief Convert a 4-dimensional vertex normal to a packed 3D vertex/RGB color
 * which follows the GL_UNSIGNED_INT_5_9_9_9_REV format.
 *
 * @param rgb
 * A constant reference to a 3D vector or RGB color.
 *
 * @return A signed 32-bit integer containing a 3D vertex with 9-bits per
 * element and 5 bits for a shared exponent.
-------------------------------------*/
inline LS_INLINE int32_t sl_pack_vec4_9e5(const ls::math::vec4& rgb) noexcept
{
    return (int32_t)SL_PackedVertex_9e5{rgb};
}



/**------------------------------------
 * @brief Convert a packed RGB9e5 type into a 3D vector.
 *
 * @param rgb
 * A 32-bit integer containing a vertex packed into the RGB9e5 integer format.
 *
 * @return A 3D vector containing the unpacked vertex.
-------------------------------------*/
inline LS_INLINE ls::math::vec3 sl_unpack_vec3_9e5(int32_t rgb) noexcept
{
    return (ls::math::vec3)SL_PackedVertex_9e5{rgb};
}




/**------------------------------------
 * @brief Convert a packed RGB9e5 type into a 4D vector.
 *
 * @param rgb
 * A 32-bit integer containing a vertex packed into the RGB9e5 integer format.
 *
 * @return A 4D vector containing the unpacked vertex.
-------------------------------------*/
inline LS_INLINE ls::math::vec4 sl_unpack_vec4_9e5(int32_t rgb) noexcept
{
    return (ls::math::vec4)SL_PackedVertex_9e5{rgb};
}



#endif /* SL_PACKED_VERTEX_HPP */
