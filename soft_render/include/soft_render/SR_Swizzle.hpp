
#ifndef SR_SWIZZLE_HPP
#define SR_SWIZZLE_HPP

#include "lightsky/setup/Api.h"
#include "lightsky/math/scalar_utils.h"



template <typename data_t>
constexpr data_t LS_INLINE sr_swizzle_ctz(data_t n) noexcept
{
    return (n >> 1) ? (1 + sr_swizzle_ctz<data_t>(n >> 1)) : 0;
}



template <uint_fast32_t texels_per_chunk, uint_fast32_t shifts_per_chunk = sr_swizzle_ctz(texels_per_chunk)>
inline uint_fast32_t LS_INLINE sr_swizzle_2d_index(uint_fast32_t x, uint_fast32_t y, uint_fast32_t imgWidth) noexcept
{
    static_assert(ls::math::is_pow2<uint_fast32_t>(texels_per_chunk), "Texels-per-chunk must be a power of two.");
    static_assert(texels_per_chunk == (1 << shifts_per_chunk), "texels_per_chunk != 2^shifts_per_chunk.");

    constexpr uint_fast32_t idsPerBlock = texels_per_chunk*texels_per_chunk;
    const uint_fast32_t     tileX       = x >> shifts_per_chunk;
    const uint_fast32_t     tileY       = y >> shifts_per_chunk;
    const uint_fast32_t     tileId      = (tileX + (imgWidth >> shifts_per_chunk) * tileY);

    // We're only getting the remainder of a power of 2. Use bit operations
    // instead of a modulo.
    const uint_fast32_t     innerX      = x & (texels_per_chunk-1u);
    const uint_fast32_t     innerY      = y & (texels_per_chunk-1u);
    const uint_fast32_t     innerId     = innerX + (innerY << shifts_per_chunk);

    return innerId + tileId * idsPerBlock;
}



template <uint_fast32_t texels_per_chunk, uint_fast32_t shifts_per_chunk = sr_swizzle_ctz(texels_per_chunk)>
inline uint_fast32_t LS_INLINE sr_swizzle_3d_index(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z, uint_fast32_t imgWidth, uint_fast32_t imgHeight) noexcept
{
    static_assert(ls::math::is_pow2<uint_fast32_t>(texels_per_chunk), "Texels-per-chunk must be a power of two.");
    static_assert(texels_per_chunk == (1 << shifts_per_chunk), "texels_per_chunk != 2^shifts_per_chunk.");

    const uint_fast32_t idsPerBlock = texels_per_chunk*texels_per_chunk*texels_per_chunk;

    const uint_fast32_t tileX       = x >> shifts_per_chunk;
    const uint_fast32_t tileY       = y >> shifts_per_chunk;
    const uint_fast32_t tileZ       = z >> shifts_per_chunk;
    const uint_fast32_t tileId      = tileX + ((imgWidth >> shifts_per_chunk) * (tileY + ((imgHeight >> shifts_per_chunk) * tileZ)));

    const uint_fast32_t innerX      = x & (texels_per_chunk-1u);
    const uint_fast32_t innerY      = y & (texels_per_chunk-1u);
    const uint_fast32_t innerZ      = z & (texels_per_chunk-1u);
    const uint_fast32_t innerId     = innerX + ((innerY << shifts_per_chunk) + (texels_per_chunk * (innerZ << shifts_per_chunk)));

    return innerId + tileId * idsPerBlock;
}



#endif /* SR_SWIZZLE_HPP */
