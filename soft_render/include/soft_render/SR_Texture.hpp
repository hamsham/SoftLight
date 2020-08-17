
#ifndef SR_TEXTURE_HPP
#define SR_TEXTURE_HPP

#include <cstddef> // ptrdiff_t

#include "lightsky/setup/Arch.h"
#include "lightsky/setup/Api.h"

#include "lightsky/math/fixed.h"
#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_Color.hpp" // SR_ColorDataType



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SR_ImgFile;



/*-----------------------------------------------------------------------------
 * Enumerations for texture wrapping/clamping
-----------------------------------------------------------------------------*/
enum SR_TexWrapMode : uint16_t
{
    SR_TEXTURE_WRAP_REPEAT,
    SR_TEXTURE_WRAP_CUTOFF,
    SR_TEXTURE_WRAP_CLAMP,

    SR_TEXTURE_WRAP_DEFAULT = SR_TEXTURE_WRAP_REPEAT
};



enum SR_TexChunkInfo : uint32_t
{
    SR_TEXELS_PER_CHUNK = 4,
    SR_TEXEL_SHIFTS_PER_CHUNK = 2 // 2^SR_TEXEL_SHIFTS_PER_CHUNK = SR_TEXELS_PER_CHUNK
};



enum SR_TexelOrder
{
    SR_TEXELS_ORDERED,
    SR_TEXELS_SWIZZLED
};



/**----------------------------------------------------------------------------
 * @brief Generic texture Class
 *
 * This class encodes textures using a Z-ordered curve.
-----------------------------------------------------------------------------*/
class alignas(sizeof(uint64_t)) SR_Texture
{
  public:
    typedef ls::math::long_medp_t fixed_type;

  private:
    uint16_t mWidth; // 2 bytes

    uint16_t mHeight; // 2 bytes

    uint16_t mDepth; // 2 bytes

    SR_ColorDataType mType; // 2 bytes

    uint16_t mBytesPerTexel; // 2 bytes

    uint32_t mNumChannels; // 4 bytes

    char* mTexels; // 4-8 bytes

  public:
    ~SR_Texture() noexcept;

    SR_Texture() noexcept;

    SR_Texture(const SR_Texture& t) noexcept;

    SR_Texture(SR_Texture&& t) noexcept;

    SR_Texture& operator=(const SR_Texture& t) noexcept;

    SR_Texture& operator=(SR_Texture&& t) noexcept;

    template <SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y) const noexcept;

    template <SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept;

    template <SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    template <SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    uint16_t width() const noexcept;

    uint16_t height() const noexcept;

    uint16_t depth() const noexcept;

    uint16_t bpp() const noexcept;

    uint32_t channels() const noexcept;

    int init(SR_ColorDataType type, uint16_t w, uint16_t h, uint16_t d = 1) noexcept;

    int init(const SR_ImgFile& imgFile, SR_TexelOrder texelOrder = SR_TexelOrder::SR_TEXELS_ORDERED) noexcept;

    void terminate() noexcept;

    SR_ColorDataType type() const noexcept;

    const void* data() const noexcept;

    void* data() noexcept;

    template <SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    void set_texel(uint16_t x, uint16_t y, uint16_t z, const void* pData) noexcept;

    template <SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    void set_texels(
            uint16_t x,
            uint16_t y,
            uint16_t z,
            uint16_t w,
            uint16_t h,
            uint16_t d,
            const void* pData
    ) noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    const color_type texel(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type& texel(uint16_t x, uint16_t y) noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    const color_type texel(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type& texel(uint16_t x, uint16_t y, uint16_t z) noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    const color_type* texel_pointer(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type* texel_pointer(uint16_t x, uint16_t y) noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    const color_type* texel_pointer(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type* texel_pointer(uint16_t x, uint16_t y, uint16_t z) noexcept;

    template <typename color_type>
    const color_type* row_pointer(uintptr_t y) const noexcept;

    template <typename color_type>
    color_type* row_pointer(uintptr_t y) noexcept;

    template <typename color_type>
    const color_type* raw_texel_pointer(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type>
    color_type* raw_texel_pointer(uint16_t x, uint16_t y) noexcept;

    template <typename color_type>
    const color_type* raw_texel_pointer(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type>
    color_type* raw_texel_pointer(uint16_t x, uint16_t y, uint16_t z) noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    const ls::math::vec4_t<color_type> texel4(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    const ls::math::vec4_t<color_type> texel4(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename data_t>
    ls::math::vec4_t<data_t> raw_texel4(uint16_t x, uint16_t y) const noexcept;

    template <typename data_t>
    ls::math::vec4_t<data_t> raw_texel4(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type>
    const color_type raw_texel(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type>
    color_type& raw_texel(uint16_t x, uint16_t y, uint16_t z) noexcept;

    template <typename color_type>
    const color_type raw_texel(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type>
    color_type& raw_texel(uint16_t x, uint16_t y) noexcept;

    template <typename color_type>
    const color_type raw_texel(ptrdiff_t index) const noexcept;

    template <typename color_type>
    color_type& raw_texel(ptrdiff_t index) noexcept;
};



/*-------------------------------------
 * Convert an X/Y coordinate to a Z-ordered coordinate.
-------------------------------------*/
template <>
inline LS_INLINE ptrdiff_t SR_Texture::map_coordinate<SR_TexelOrder::SR_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    return (ptrdiff_t)(x + (uint_fast32_t)mWidth * y);
}



template <>
inline LS_INLINE ptrdiff_t SR_Texture::map_coordinate<SR_TexelOrder::SR_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    constexpr uint_fast32_t idsPerBlock = SR_TEXELS_PER_CHUNK*SR_TEXELS_PER_CHUNK;
    const uint_fast32_t     tileX       = x >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t     tileY       = y >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t     tileId      = (tileX + (mWidth >> SR_TEXEL_SHIFTS_PER_CHUNK) * tileY);

    // We're only getting the remainder of a power of 2. Use bit operations
    // instead of a modulo.
    const uint_fast32_t     innerX      = x & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t     innerY      = y & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t     innerId     = innerX + (innerY << SR_TEXEL_SHIFTS_PER_CHUNK);

    return (ptrdiff_t)(innerId + tileId * idsPerBlock);
}



/*-------------------------------------
 * Convert an X/Y coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
template <SR_TexelOrder order>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SR_Texture::map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    return map_coordinates<order>(x, y, 0u);
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to a Z-ordered coordinate.
-------------------------------------*/
template <>
inline LS_INLINE ptrdiff_t SR_Texture::map_coordinate<SR_TexelOrder::SR_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return (ptrdiff_t)(x + mWidth * (y + mHeight * z));
}



template <>
inline LS_INLINE ptrdiff_t SR_Texture::map_coordinate<SR_TexelOrder::SR_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    const uint_fast32_t idsPerBlock = SR_TEXELS_PER_CHUNK * SR_TEXELS_PER_CHUNK * ((mDepth > 1) ? LS_ENUM_VAL(SR_TEXELS_PER_CHUNK) : 1);

    const uint_fast32_t tileX       = x >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileY       = y >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileZ       = z >> SR_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileId      = tileX + ((mWidth >> SR_TEXEL_SHIFTS_PER_CHUNK) * (tileY + ((mHeight >> SR_TEXEL_SHIFTS_PER_CHUNK) * tileZ)));

    const uint_fast32_t innerX      = x & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerY      = y & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerZ      = z & (SR_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerId     = innerX + ((innerY << SR_TEXEL_SHIFTS_PER_CHUNK) + (SR_TEXELS_PER_CHUNK * (innerZ << SR_TEXEL_SHIFTS_PER_CHUNK)));

    return (ptrdiff_t)(innerId + tileId * idsPerBlock);
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
template <>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SR_Texture::map_coordinates<SR_TexelOrder::SR_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return ls::math::vec4_t<ptrdiff_t>{0, 1, 2, 3} + x + mWidth * (y + mHeight * z);
}



template <>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SR_Texture::map_coordinates<SR_TexelOrder::SR_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    const uint_fast32_t idsPerBlock = SR_TEXELS_PER_CHUNK * SR_TEXELS_PER_CHUNK * ((mDepth > 1) ? LS_ENUM_VAL(SR_TEXELS_PER_CHUNK) : 1);

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
    const uint_fast32_t tileShift   = (mWidth >> SR_TEXEL_SHIFTS_PER_CHUNK) * (tileY + ((mHeight >> SR_TEXEL_SHIFTS_PER_CHUNK) * tileZ));
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
 * Get the texture width
-------------------------------------*/
inline LS_INLINE uint16_t SR_Texture::width() const noexcept
{
    return mWidth;
}



/*-------------------------------------
 * Get the texture height
-------------------------------------*/
inline LS_INLINE uint16_t SR_Texture::height() const noexcept
{
    return mHeight;
}



/*-------------------------------------
 * Get the texture depth
-------------------------------------*/
inline LS_INLINE uint16_t SR_Texture::depth() const noexcept
{
    return mDepth;
}



/*-------------------------------------
 * Get the bytes per pixel
-------------------------------------*/
inline LS_INLINE uint16_t SR_Texture::bpp() const noexcept
{
    return mBytesPerTexel;
}



/*-------------------------------------
 * Get the elements per pixel
-------------------------------------*/
inline LS_INLINE uint32_t SR_Texture::channels() const noexcept
{
    return mNumChannels;
}



/*-------------------------------------
 * Get the texture mType
-------------------------------------*/
inline LS_INLINE SR_ColorDataType SR_Texture::type() const noexcept
{
    return mType;
}



/*-------------------------------------
 * Retrieve the raw texels
-------------------------------------*/
inline LS_INLINE const void* SR_Texture::data() const noexcept
{
    return mTexels;
}



/*-------------------------------------
 * Retrieve the raw texels
-------------------------------------*/
inline LS_INLINE void* SR_Texture::data() noexcept
{
    return mTexels;
}



/*-------------------------------------
 * Retrieve a raw texels
-------------------------------------*/
template <SR_TexelOrder order>
inline void SR_Texture::set_texel(uint16_t x, uint16_t y, uint16_t z, const void* pData) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    size_t bpp = mBytesPerTexel;
    const ptrdiff_t offset = bpp * index;

    char* pOut = mTexels + offset;
    const char* pIn = reinterpret_cast<const char*>(pData);

    while (bpp--)
    {
        *pOut++ = *pIn++;
    }
}



/*-------------------------------------
 * Retrieve a raw texels
-------------------------------------*/
template <SR_TexelOrder order>
void SR_Texture::set_texels(
    uint16_t x,
    uint16_t y,
    uint16_t z,
    uint16_t w,
    uint16_t h,
    uint16_t d,
    const void* pData) noexcept
{
    const char* pSrc = reinterpret_cast<const char*>(pData);
    const size_t bytesPerColor = mBytesPerTexel;

    for (uint16_t k = 0, z0 = z; k < d; ++k, ++z0)
    {
        for (uint16_t j = 0, y0 = y; j < h; ++j, ++y0)
        {
            for (uint16_t i = 0, x0 = x; i < w; ++i, ++x0)
            {
                const ptrdiff_t index = i + w * (j + (h * k));
                const ptrdiff_t offset = (index * bytesPerColor);

                set_texel<order>(x0, y0, z0, pSrc+offset);
            }
        }
    }
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE const color_type SR_Texture::texel(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE color_type& SR_Texture::texel(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE const color_type SR_Texture::texel(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE color_type& SR_Texture::texel(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE const color_type* SR_Texture::texel_pointer(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y);
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE color_type* SR_Texture::texel_pointer(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y);
    return reinterpret_cast<color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE const color_type* SR_Texture::texel_pointer(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE color_type* SR_Texture::texel_pointer(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    return reinterpret_cast<color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a scanline
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type* SR_Texture::row_pointer(uintptr_t y) const noexcept
{
    return reinterpret_cast<const color_type*>(mTexels) + (uintptr_t)y * (uintptr_t)mWidth;
}



/*-------------------------------------
 * Retrieve a scanline
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type* SR_Texture::row_pointer(uintptr_t y) noexcept
{
    return reinterpret_cast<color_type*>(mTexels) + (uintptr_t)y * (uintptr_t)mWidth;
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type* SR_Texture::raw_texel_pointer(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type* SR_Texture::raw_texel_pointer(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type* SR_Texture::raw_texel_pointer(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type* SR_Texture::raw_texel_pointer(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve 4 swizzled texels (const)
-------------------------------------*/
template <>
inline LS_INLINE const ls::math::vec4_t<float> SR_Texture::texel4<float, SR_TexelOrder::SR_TEXELS_ORDERED>(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = map_coordinate<SR_TexelOrder::SR_TEXELS_ORDERED>(x, y);
    const float* pTexels = reinterpret_cast<const float*>(mTexels) + index;

    #if defined(LS_ARCH_X86)
        return ls::math::vec4_t<float>{_mm_loadu_ps(pTexels)};
    #elif defined(LS_ARCH_ARM)
        return ls::math::vec4_t<float>{vld1q_f32(pTexels)};
    #else
        return *reinterpret_cast<const ls::math::vec4_t<float>*>(pTexels);
    #endif
}

#if defined(LS_ARCH_X86)
template <>
inline const ls::math::vec4_t<float> SR_Texture::texel4<float, SR_TexelOrder::SR_TEXELS_SWIZZLED>(uint16_t x, uint16_t y) const noexcept
{
    const ls::math::vec4_t<ptrdiff_t> index = map_coordinates<SR_TexelOrder::SR_TEXELS_SWIZZLED>(x, y);
    const __m128i indices = _mm_set_epi32(index[3], index[2], index[1], index[0]);
    const float* pTexels = reinterpret_cast<const float*>(mTexels);

    return ls::math::vec4_t<float>{_mm_i32gather_ps(pTexels, indices, 4)};
}
#endif



template <typename color_type, SR_TexelOrder order>
inline LS_INLINE const ls::math::vec4_t<color_type> SR_Texture::texel4(uint16_t x, uint16_t y) const noexcept
{
    if (order == SR_TexelOrder::SR_TEXELS_SWIZZLED)
    {
        const ls::math::vec4_t<ptrdiff_t>&& index = map_coordinates<SR_TexelOrder::SR_TEXELS_SWIZZLED>(x, y);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels);
        return ls::math::vec4_t<color_type>{
            pTexels[index[0]],
            pTexels[index[1]],
            pTexels[index[2]],
            pTexels[index[3]]
        };
    }
    else
    {
        const ptrdiff_t index = map_coordinate<SR_TexelOrder::SR_TEXELS_ORDERED>(x, y);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels) + index;
        return *reinterpret_cast<const ls::math::vec4_t<color_type>*>(pTexels);
    }
}



/*-------------------------------------
 * Retrieve 4 swizzled texels (const)
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE const ls::math::vec4_t<color_type> SR_Texture::texel4(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    if (order == SR_TexelOrder::SR_TEXELS_SWIZZLED)
    {
        const ls::math::vec4_t<ptrdiff_t>&& index = map_coordinates<SR_TexelOrder::SR_TEXELS_SWIZZLED>(x, y, z);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels);
        return ls::math::vec4_t<color_type>{
            pTexels[index[0]],
            pTexels[index[1]],
            pTexels[index[2]],
            pTexels[index[3]]
        };
    }
    else
    {
        const ptrdiff_t index = map_coordinate<SR_TexelOrder::SR_TEXELS_ORDERED>(x, y, z);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels) + index;
        return *reinterpret_cast<const ls::math::vec4_t<color_type>*>(pTexels);
    }
}



/*-------------------------------------
 * Retrieve 4 raw texels (const)
-------------------------------------*/
template <typename data_t>
inline LS_INLINE ls::math::vec4_t<data_t> SR_Texture::raw_texel4(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    const data_t* pBuffer = reinterpret_cast<data_t*>(mTexels) + index;
    return ls::math::vec4_t<data_t>{pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]};
}



template <>
inline LS_INLINE ls::math::vec4_t<float> SR_Texture::raw_texel4<float>(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    const float* pBuffer = reinterpret_cast<const float*>(mTexels) + index;

    #if defined(LS_ARCH_X86)
        return ls::math::vec4_t<float>{_mm_loadu_ps(pBuffer)};
    #elif defined(LS_ARCH_ARM)
        return ls::math::vec4_t<float>{vld1q_f32(pBuffer)};
    #else
        return *reinterpret_cast<const ls::math::vec4_t<float>*>(pBuffer);
    #endif
}



/*-------------------------------------
 * Retrieve 4 raw texels (const)
-------------------------------------*/
template <typename data_t>
inline LS_INLINE ls::math::vec4_t<data_t> SR_Texture::raw_texel4(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    const data_t* pBuffer = reinterpret_cast<data_t*>(mTexels) + index;
    return ls::math::vec4_t<data_t>{pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]};
}



template <>
inline LS_INLINE ls::math::vec4_t<float> SR_Texture::raw_texel4<float>(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    const float* pBuffer = reinterpret_cast<const float*>(mTexels) + index;

    #if defined(LS_ARCH_X86)
        return ls::math::vec4_t<float>{_mm_loadu_ps(pBuffer)};
    #elif defined(LS_ARCH_ARM)
        return ls::math::vec4_t<float>{vld1q_f32(pBuffer)};
    #else
        return *reinterpret_cast<const ls::math::vec4_t<float>*>(pBuffer);
    #endif
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type SR_Texture::raw_texel(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type& SR_Texture::raw_texel(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type SR_Texture::raw_texel(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type& SR_Texture::raw_texel(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type SR_Texture::raw_texel(ptrdiff_t index) const noexcept
{
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type& SR_Texture::raw_texel(ptrdiff_t index) noexcept
{
    return reinterpret_cast<color_type*>(mTexels)[index];
}



#endif /* SR_TEXTURE_HPP */
