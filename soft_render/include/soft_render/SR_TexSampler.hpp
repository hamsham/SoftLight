
#ifndef SR_TEX_SAMPLER_HPP
#define SR_TEX_SAMPLER_HPP

#include "soft_render/SR_Texture.hpp"



/*
enum SR_TexSwizzleParam : uint32_t
{
    SR_TEXELS_PER_CHUNK = 4,
    SR_TEXEL_SHIFTS_PER_CHUNK = 2 // 2^SR_TEXEL_SHIFTS_PER_CHUNK = SR_TEXELS_PER_CHUNK
};
*/



class SR_TexSampler
{
  private:
    SR_TexWrapMode mWrapping; // 2 bytes

    uint16_t mWidthi; // 2 bytes

    uint16_t mHeighti; // 2 bytes

    uint16_t mDepthi; // 2 bytes

    SR_Texture::fixed_type mWidthf; // 8 bytes

    SR_Texture::fixed_type mHeightf; // 8 bytes

    SR_Texture::fixed_type mDepthf; // 8 bytes

    uint16_t mBytesPerTexel; // 2 bytes

    SR_ColorDataType mType; // 2 bytes

    uint32_t mNumChannels; // 4 bytes

    const void* mTexData; // 4-8 bytes

    float wrap_coordinate(float uvw) const noexcept;

    inline SR_Texture::fixed_type wrap_coordinate(SR_Texture::fixed_type uvw) const noexcept;

    int wrap_coordinate(int uvw, int maxVal) const noexcept;

    template <SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y) const noexcept;

    template <SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept;

    template <SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    template <SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

  public:
    ~SR_TexSampler() noexcept;

    SR_TexSampler() noexcept;

    SR_TexSampler(const SR_Texture&) noexcept;

    SR_TexSampler(const SR_TexSampler&) noexcept;

    SR_TexSampler(SR_TexSampler&&) noexcept;

    SR_TexSampler& operator=(const SR_TexSampler&) noexcept;

    SR_TexSampler& operator=(SR_TexSampler&&) noexcept;

    void init(const SR_Texture&) noexcept;

    uint16_t bpp() const noexcept;

    uint32_t channels() const noexcept;

    SR_TexWrapMode wrap_mode() const noexcept;

    template <typename color_type, SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type nearest(float x, float y) const noexcept;

    template <typename color_type, SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type nearest(float x, float y, float z) const noexcept;

    /*
    template <typename color_type, SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type bilinear(float x, float y) const noexcept;

    template <typename color_type, SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type bilinear(float x, float y, float z) const noexcept;

    template <typename color_type, SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type trilinear(float x, float y) const noexcept;

    template <typename color_type, SR_TexelOrder mode = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type trilinear(float x, float y, float z) const noexcept;
    */
};



/*-------------------------------------
 * Keep all UV values within the (0.f, 1.f) range.
-------------------------------------*/
inline LS_INLINE float SR_TexSampler::wrap_coordinate(float uvw) const noexcept
{
    return (mWrapping == SR_TEXTURE_WRAP_REPEAT)
           ? ((uvw < 0.f ? 1.f : 0.f) + ls::math::fmod_1(uvw))
           : ls::math::clamp(uvw, 0.f, 1.f);
}



/*-------------------------------------
 * Keep all UV values within the (0.f, 1.f) range.
-------------------------------------*/
inline LS_INLINE SR_Texture::fixed_type SR_TexSampler::wrap_coordinate(SR_Texture::fixed_type uvw) const noexcept
{
    return (mWrapping == SR_TEXTURE_WRAP_REPEAT)
           ? ((uvw < SR_Texture::fixed_type{0u}
               ? ls::math::fixed_cast<SR_Texture::fixed_type>(1u)
               : SR_Texture::fixed_type{0u}) + ls::math::fmod_1(uvw))
           : ls::math::clamp<SR_Texture::fixed_type>(uvw, SR_Texture::fixed_type{0u}, ls::math::fixed_cast<SR_Texture::fixed_type>(1u));
}



/*-------------------------------------
 * Keep all UV values within the range (0, maxVal).
-------------------------------------*/
inline LS_INLINE int SR_TexSampler::wrap_coordinate(int uvw, int maxVal) const noexcept
{
    return (mWrapping == SR_TEXTURE_WRAP_REPEAT)
           //? ((-(uvw < 0) & maxVal) + (uvw % maxVal))
           ? ((uvw % maxVal) + (-(uvw < 0) & maxVal))
           : ls::math::clamp<int>(uvw, 0, maxVal);
}



/*-------------------------------------
 * Convert an X/Y coordinate to a Z-ordered coordinate.
-------------------------------------*/
template <>
inline LS_INLINE ptrdiff_t SR_TexSampler::map_coordinate<SR_TexelOrder::SR_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    return x + (uint_fast32_t)mWidthi * y;
}



template <>
inline LS_INLINE ptrdiff_t SR_TexSampler::map_coordinate<SR_TexelOrder::SR_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    constexpr uint_fast32_t idsPerBlock = SR_TEXELS_PER_CHUNK*SR_TEXELS_PER_CHUNK;
    const uint_fast32_t     tileX       = x >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t     tileY       = y >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t     tileId      = (tileX + ((uint_fast32_t)mWidthi >> SR_TEXEL_SHIFTS_PER_CHUNK) * tileY);

    // We're only getting the remainder of a power of 2. Use bit operations
    // instead of a modulo.
    const uint_fast32_t     innerX      = x & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t     innerY      = y & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t     innerId     = innerX + (innerY << SR_TEXEL_SHIFTS_PER_CHUNK);

    return innerId + tileId * idsPerBlock;
}



/*-------------------------------------
 * Convert an X/Y coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
template <SR_TexelOrder order>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SR_TexSampler::map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    return map_coordinates<order>(x, y, 0u);
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to a Z-ordered coordinate.
-------------------------------------*/
template <>
inline LS_INLINE ptrdiff_t SR_TexSampler::map_coordinate<SR_TexelOrder::SR_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return x + (uint_fast32_t)mWidthi * (y + (uint_fast32_t)mHeighti * z);
}



template <>
inline LS_INLINE ptrdiff_t SR_TexSampler::map_coordinate<SR_TexelOrder::SR_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    const uint_fast32_t idsPerBlock = SR_TEXELS_PER_CHUNK * SR_TEXELS_PER_CHUNK * (((uint_fast32_t)mDepthi > 1) ? LS_ENUM_VAL(SR_TEXELS_PER_CHUNK) : 1);

    const uint_fast32_t tileX       = x >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileY       = y >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileZ       = z >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileId      = tileX + (((uint_fast32_t)mWidthi >> SR_TEXEL_SHIFTS_PER_CHUNK) * (tileY + (((uint_fast32_t)mHeighti >> SR_TEXEL_SHIFTS_PER_CHUNK) * tileZ)));

    const uint_fast32_t innerX      = x & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerY      = y & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerZ      = z & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerId     = innerX + ((innerY << SR_TEXEL_SHIFTS_PER_CHUNK) + (SR_TEXELS_PER_CHUNK * (innerZ << SR_TEXEL_SHIFTS_PER_CHUNK)));

    return innerId + tileId * idsPerBlock;
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
template <>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SR_TexSampler::map_coordinates<SR_TexelOrder::SR_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return ls::math::vec4_t<ptrdiff_t>{0, 1, 2, 3} + x + (uint_fast32_t)mWidthi * (y + (uint_fast32_t)mHeighti * z);
}



template <>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SR_TexSampler::map_coordinates<SR_TexelOrder::SR_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    const uint_fast32_t idsPerBlock = SR_TEXELS_PER_CHUNK * SR_TEXELS_PER_CHUNK * (((uint_fast32_t)mDepthi > 1) ? LS_ENUM_VAL(SR_TEXELS_PER_CHUNK) : 1);

    const uint_fast32_t x0          = x+0u;
    const uint_fast32_t x1          = x+1u;
    const uint_fast32_t x2          = x+2u;
    const uint_fast32_t x3          = x+3u;
    const uint_fast32_t tileX0      = x0 >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileX1      = x1 >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileX2      = x2 >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileX3      = x3 >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileY       = y >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileZ       = z >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileShift   = ((uint_fast32_t)mWidthi >> SR_TEXEL_SHIFTS_PER_CHUNK) * (tileY + (((uint_fast32_t)mHeighti >> SR_TEXEL_SHIFTS_PER_CHUNK) * tileZ));
    const uint_fast32_t tileId0     = tileX0 + tileShift;
    const uint_fast32_t tileId1     = tileX1 + tileShift;
    const uint_fast32_t tileId2     = tileX2 + tileShift;
    const uint_fast32_t tileId3     = tileX3 + tileShift;
    const uint_fast32_t innerX0     = x0 & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerX1     = x1 & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerX2     = x2 & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerX3     = x3 & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerY      = y & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerZ      = z & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerShift  = (innerY << SR_TEXEL_SHIFTS_PER_CHUNK) + (SR_TEXELS_PER_CHUNK * (innerZ << SR_TEXEL_SHIFTS_PER_CHUNK));
    const uint_fast32_t innerId0    =  innerX0 + innerShift;
    const uint_fast32_t innerId1    =  innerX1 + innerShift;
    const uint_fast32_t innerId2    =  innerX2 + innerShift;
    const uint_fast32_t innerId3    =  innerX3 + innerShift;

    const ls::math::vec4_t<ptrdiff_t> innerIds{(ptrdiff_t)innerId0, (ptrdiff_t)innerId1, (ptrdiff_t)innerId2, (ptrdiff_t)innerId3};
    const ls::math::vec4_t<ptrdiff_t> tileIds{(ptrdiff_t)tileId0, (ptrdiff_t)tileId1, (ptrdiff_t)tileId2, (ptrdiff_t)tileId3};

    return innerIds + tileIds * idsPerBlock;
}



/*-------------------------------------
 * Get the bytes per pixel
-------------------------------------*/
inline LS_INLINE uint16_t SR_TexSampler::bpp() const noexcept
{
    return mBytesPerTexel;
}



/*-------------------------------------
 * Get the elements per pixel
-------------------------------------*/
inline LS_INLINE uint32_t SR_TexSampler::channels() const noexcept
{
    return mNumChannels;
}



/*-------------------------------------
 * Get the current wrapping mode
-------------------------------------*/
inline LS_INLINE SR_TexWrapMode SR_TexSampler::wrap_mode() const noexcept
{
    return mWrapping;
}



/*-------------------------------------
 * Nearest-neighbor lookup
-------------------------------------*/
template <typename color_type, SR_TexelOrder mode>
inline LS_INLINE color_type SR_TexSampler::nearest(float x, float y) const noexcept
{
    if (mWrapping != SR_TEXTURE_WRAP_CUTOFF || (ls::math::min(x, y, 0.f, 0.f) >= 0.f && ls::math::max(x, y, 1.f, 1.f) <= 1.f))
    {
        const SR_Texture::fixed_type xf = ls::math::fixed_cast<SR_Texture::fixed_type, float>(x);
        const SR_Texture::fixed_type yf = ls::math::fixed_cast<SR_Texture::fixed_type, float>(y);

        const uint_fast32_t xi = ls::math::integer_cast<uint_fast32_t>(mWidthf * wrap_coordinate(xf));
        const uint_fast32_t yi = ls::math::integer_cast<uint_fast32_t>(mHeightf * wrap_coordinate(yf));

        const ptrdiff_t index = map_coordinate<mode>(xi, yi);

        return reinterpret_cast<const color_type*>(mTexData)[index];
    }

    return color_type{0};
}



/*-------------------------------------
 * Nearest-neighbor lookup
-------------------------------------*/
template <typename color_type, SR_TexelOrder mode>
inline LS_INLINE color_type SR_TexSampler::nearest(float x, float y, float z) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y, z) < 0.f || ls::math::max(x, y, z) >= 1.f))
    {
        return color_type{0};
    }

    const SR_Texture::fixed_type xf = ls::math::fixed_cast<SR_Texture::fixed_type, float>(x);
    const SR_Texture::fixed_type yf = ls::math::fixed_cast<SR_Texture::fixed_type, float>(y);
    const SR_Texture::fixed_type zf = ls::math::fixed_cast<SR_Texture::fixed_type, float>(z);

    const uint_fast32_t xi = ls::math::integer_cast<uint_fast32_t>(mWidthf * wrap_coordinate(xf));
    const uint_fast32_t yi = ls::math::integer_cast<uint_fast32_t>(mHeightf * wrap_coordinate(yf));
    const uint_fast32_t zi = ls::math::integer_cast<uint_fast32_t>(mDepthf * wrap_coordinate(zf));

    const ptrdiff_t index = map_coordinate<mode>(xi, yi, zi);
    return reinterpret_cast<const color_type*>(mTexData)[index];
}



#endif /* SR_TEX_SAMPLER_HPP */
