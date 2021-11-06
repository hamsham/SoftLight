
#ifndef SL_TEXTURE_HPP
#define SL_TEXTURE_HPP

#include <cstddef> // ptrdiff_t

#include "lightsky/setup/Arch.h"

#include "lightsky/math/fixed.h"
#include "lightsky/math/vec_utils.h"

#include "softlight/SL_Color.hpp" // SL_ColorDataType
#include "softlight/SL_Swizzle.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SL_ImgFile;



/*-----------------------------------------------------------------------------
 * Enumerations for texture wrapping/clamping
-----------------------------------------------------------------------------*/
enum SL_TexWrapMode : uint16_t
{
    SL_TEXTURE_WRAP_REPEAT,
    SL_TEXTURE_WRAP_CUTOFF,
    SL_TEXTURE_WRAP_CLAMP,

    SL_TEXTURE_WRAP_DEFAULT = SL_TEXTURE_WRAP_REPEAT
};



enum SL_TexChunkInfo : uint32_t
{
    SL_TEXELS_PER_CHUNK = 4,
    SL_TEXEL_SHIFTS_PER_CHUNK = sl_swizzle_ctz<uint32_t>(SL_TEXELS_PER_CHUNK) // log2(SL_TEXELS_PER_CHUNK)
};



enum SL_TexelOrder
{
    SL_TEXELS_ORDERED,
    SL_TEXELS_SWIZZLED
};



/**----------------------------------------------------------------------------
 * @brief Generic texture Class
 *
 * This class encodes textures using a Z-ordered curve.
-----------------------------------------------------------------------------*/
class alignas(sizeof(uint64_t)) SL_Texture
{
  public:
    typedef ls::math::long_medp_t fixed_type;

  private:
    uint16_t mWidth; // 2 bytes

    uint16_t mHeight; // 2 bytes

    uint16_t mDepth; // 2 bytes

    SL_ColorDataType mType; // 2 bytes

    uint16_t mBytesPerTexel; // 2 bytes

    uint16_t mNumChannels; // 2 bytes

    char* mTexels; // 4-8 bytes

  public:
    ~SL_Texture() noexcept;

    SL_Texture() noexcept;

    SL_Texture(const SL_Texture& t) noexcept;

    SL_Texture(SL_Texture&& t) noexcept;

    SL_Texture& operator=(const SL_Texture& t) noexcept;

    SL_Texture& operator=(SL_Texture&& t) noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y) const noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    uint16_t width() const noexcept;

    uint16_t height() const noexcept;

    uint16_t depth() const noexcept;

    uint16_t bpp() const noexcept;

    uint32_t channels() const noexcept;

    int init(SL_ColorDataType type, uint16_t w, uint16_t h, uint16_t d = 1) noexcept;

    int init(const SL_ImgFile& imgFile, SL_TexelOrder texelOrder = SL_TexelOrder::SL_TEXELS_ORDERED) noexcept;

    void terminate() noexcept;

    SL_ColorDataType type() const noexcept;

    const void* data() const noexcept;

    void* data() noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    void set_texel(uint16_t x, uint16_t y, uint16_t z, const void* pData) noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    void set_texels(
            uint16_t x,
            uint16_t y,
            uint16_t z,
            uint16_t w,
            uint16_t h,
            uint16_t d,
            const void* pData
    ) noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    const color_type texel(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    color_type& texel(uint16_t x, uint16_t y) noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    const color_type texel(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    color_type& texel(uint16_t x, uint16_t y, uint16_t z) noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    const color_type* texel_pointer(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    color_type* texel_pointer(uint16_t x, uint16_t y) noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    const color_type* texel_pointer(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
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

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
    const ls::math::vec4_t<color_type> texel4(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::SL_TEXELS_ORDERED>
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
inline LS_INLINE ptrdiff_t SL_Texture::map_coordinate<SL_TexelOrder::SL_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    return (ptrdiff_t)(x + (uint_fast32_t)mWidth * y);
}



template <>
inline LS_INLINE ptrdiff_t SL_Texture::map_coordinate<SL_TexelOrder::SL_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    return (ptrdiff_t)sl_swizzle_2d_index<SL_TEXELS_PER_CHUNK, SL_TEXEL_SHIFTS_PER_CHUNK>(x, y, mWidth);
}



/*-------------------------------------
 * Convert an X/Y coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
template <SL_TexelOrder order>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SL_Texture::map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    return map_coordinates<order>(x, y, 0u);
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to a Z-ordered coordinate.
-------------------------------------*/
template <>
inline LS_INLINE ptrdiff_t SL_Texture::map_coordinate<SL_TexelOrder::SL_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return (ptrdiff_t)(x + mWidth * (y + mHeight * z));
}



template <>
inline LS_INLINE ptrdiff_t SL_Texture::map_coordinate<SL_TexelOrder::SL_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return (ptrdiff_t)sl_swizzle_3d_index<SL_TEXELS_PER_CHUNK, SL_TEXEL_SHIFTS_PER_CHUNK>(x, y, z, mWidth, mHeight);
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
template <>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SL_Texture::map_coordinates<SL_TexelOrder::SL_TEXELS_ORDERED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return ls::math::vec4_t<ptrdiff_t>{0, 1, 2, 3} + x + mWidth * (y + mHeight * z);
}



template <>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SL_Texture::map_coordinates<SL_TexelOrder::SL_TEXELS_SWIZZLED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    const uint_fast32_t idsPerBlock = SL_TEXELS_PER_CHUNK * SL_TEXELS_PER_CHUNK * ((mDepth > 1) ? LS_ENUM_VAL(SL_TEXELS_PER_CHUNK) : 1);

    const uint_fast32_t x0          = x+0u;
    const uint_fast32_t x1          = x+1u;
    const uint_fast32_t x2          = x+2u;
    const uint_fast32_t x3          = x+3u;
    const uint_fast32_t tileX0      = x0 >> SL_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileX1      = x1 >> SL_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileX2      = x2 >> SL_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileX3      = x3 >> SL_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileY       = y >> SL_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileZ       = z >> SL_TEXEL_SHIFTS_PER_CHUNK;
    const uint_fast32_t tileShift   = (mWidth >> SL_TEXEL_SHIFTS_PER_CHUNK) * (tileY + ((mHeight >> SL_TEXEL_SHIFTS_PER_CHUNK) * tileZ));
    const uint_fast32_t tileId0     = tileX0 + tileShift;
    const uint_fast32_t tileId1     = tileX1 + tileShift;
    const uint_fast32_t tileId2     = tileX2 + tileShift;
    const uint_fast32_t tileId3     = tileX3 + tileShift;
    const uint_fast32_t innerX0     = x0 & (SL_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerX1     = x1 & (SL_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerX2     = x2 & (SL_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerX3     = x3 & (SL_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerY      = y & (SL_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerZ      = z & (SL_TEXELS_PER_CHUNK-1u);
    const uint_fast32_t innerShift  = (innerY << SL_TEXEL_SHIFTS_PER_CHUNK) + (SL_TEXELS_PER_CHUNK * (innerZ << SL_TEXEL_SHIFTS_PER_CHUNK));
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
inline LS_INLINE uint16_t SL_Texture::width() const noexcept
{
    return mWidth;
}



/*-------------------------------------
 * Get the texture height
-------------------------------------*/
inline LS_INLINE uint16_t SL_Texture::height() const noexcept
{
    return mHeight;
}



/*-------------------------------------
 * Get the texture depth
-------------------------------------*/
inline LS_INLINE uint16_t SL_Texture::depth() const noexcept
{
    return mDepth;
}



/*-------------------------------------
 * Get the bytes per pixel
-------------------------------------*/
inline LS_INLINE uint16_t SL_Texture::bpp() const noexcept
{
    return mBytesPerTexel;
}



/*-------------------------------------
 * Get the elements per pixel
-------------------------------------*/
inline LS_INLINE uint32_t SL_Texture::channels() const noexcept
{
    return (uint32_t)mNumChannels;
}



/*-------------------------------------
 * Get the texture mType
-------------------------------------*/
inline LS_INLINE SL_ColorDataType SL_Texture::type() const noexcept
{
    return mType;
}



/*-------------------------------------
 * Retrieve the raw texels
-------------------------------------*/
inline LS_INLINE const void* SL_Texture::data() const noexcept
{
    return mTexels;
}



/*-------------------------------------
 * Retrieve the raw texels
-------------------------------------*/
inline LS_INLINE void* SL_Texture::data() noexcept
{
    return mTexels;
}



/*-------------------------------------
 * Retrieve a raw texels
-------------------------------------*/
template <SL_TexelOrder order>
inline void SL_Texture::set_texel(uint16_t x, uint16_t y, uint16_t z, const void* pData) noexcept
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
template <SL_TexelOrder order>
void SL_Texture::set_texels(
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
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE const color_type SL_Texture::texel(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE color_type& SL_Texture::texel(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE const color_type SL_Texture::texel(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE color_type& SL_Texture::texel(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE const color_type* SL_Texture::texel_pointer(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y);
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE color_type* SL_Texture::texel_pointer(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y);
    return reinterpret_cast<color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE const color_type* SL_Texture::texel_pointer(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE color_type* SL_Texture::texel_pointer(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = map_coordinate<order>(x, y, z);
    return reinterpret_cast<color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a scanline
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type* SL_Texture::row_pointer(uintptr_t y) const noexcept
{
    return reinterpret_cast<const color_type*>(mTexels) + (uintptr_t)y * (uintptr_t)mWidth;
}



/*-------------------------------------
 * Retrieve a scanline
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type* SL_Texture::row_pointer(uintptr_t y) noexcept
{
    return reinterpret_cast<color_type*>(mTexels) + (uintptr_t)y * (uintptr_t)mWidth;
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type* SL_Texture::raw_texel_pointer(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type* SL_Texture::raw_texel_pointer(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type* SL_Texture::raw_texel_pointer(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type* SL_Texture::raw_texel_pointer(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve 4 swizzled texels (const)
-------------------------------------*/
template <>
inline LS_INLINE const ls::math::vec4_t<float> SL_Texture::texel4<float, SL_TexelOrder::SL_TEXELS_ORDERED>(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = map_coordinate<SL_TexelOrder::SL_TEXELS_ORDERED>(x, y);
    const float* pTexels = reinterpret_cast<const float*>(mTexels) + index;

    #if defined(LS_ARCH_X86)
        return ls::math::vec4_t<float>{_mm_loadu_ps(pTexels)};
    #elif defined(LS_ARM_NEON)
        return ls::math::vec4_t<float>{vld1q_f32(pTexels)};
    #else
        return *reinterpret_cast<const ls::math::vec4_t<float>*>(pTexels);
    #endif
}

#if defined(LS_ARCH_X86)
template <>
inline const ls::math::vec4_t<float> SL_Texture::texel4<float, SL_TexelOrder::SL_TEXELS_SWIZZLED>(uint16_t x, uint16_t y) const noexcept
{
    const ls::math::vec4_t<ptrdiff_t> index = map_coordinates<SL_TexelOrder::SL_TEXELS_SWIZZLED>(x, y);
    const __m128i indices = _mm_set_epi32(index[3], index[2], index[1], index[0]);
    const float* pTexels = reinterpret_cast<const float*>(mTexels);

    return ls::math::vec4_t<float>{_mm_i32gather_ps(pTexels, indices, 4)};
}
#endif



template <typename color_type, SL_TexelOrder order>
inline LS_INLINE const ls::math::vec4_t<color_type> SL_Texture::texel4(uint16_t x, uint16_t y) const noexcept
{
    if (order == SL_TexelOrder::SL_TEXELS_SWIZZLED)
    {
        const ls::math::vec4_t<ptrdiff_t>&& index = map_coordinates<SL_TexelOrder::SL_TEXELS_SWIZZLED>(x, y);
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
        const ptrdiff_t index = map_coordinate<SL_TexelOrder::SL_TEXELS_ORDERED>(x, y);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels) + index;
        return *reinterpret_cast<const ls::math::vec4_t<color_type>*>(pTexels);
    }
}



/*-------------------------------------
 * Retrieve 4 swizzled texels (const)
-------------------------------------*/
template <typename color_type, SL_TexelOrder order>
inline LS_INLINE const ls::math::vec4_t<color_type> SL_Texture::texel4(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    if (order == SL_TexelOrder::SL_TEXELS_SWIZZLED)
    {
        const ls::math::vec4_t<ptrdiff_t>&& index = map_coordinates<SL_TexelOrder::SL_TEXELS_SWIZZLED>(x, y, z);
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
        const ptrdiff_t index = map_coordinate<SL_TexelOrder::SL_TEXELS_ORDERED>(x, y, z);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels) + index;
        return *reinterpret_cast<const ls::math::vec4_t<color_type>*>(pTexels);
    }
}



/*-------------------------------------
 * Retrieve 4 raw texels (const)
-------------------------------------*/
template <typename data_t>
inline LS_INLINE ls::math::vec4_t<data_t> SL_Texture::raw_texel4(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    const data_t* pBuffer = reinterpret_cast<data_t*>(mTexels) + index;
    return ls::math::vec4_t<data_t>{pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]};
}



template <>
inline LS_INLINE ls::math::vec4_t<float> SL_Texture::raw_texel4<float>(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    const float* pBuffer = reinterpret_cast<const float*>(mTexels) + index;

    #if defined(LS_ARCH_X86)
        return ls::math::vec4_t<float>{_mm_loadu_ps(pBuffer)};
    #elif defined(LS_ARM_NEON)
        return ls::math::vec4_t<float>{vld1q_f32(pBuffer)};
    #else
        return *reinterpret_cast<const ls::math::vec4_t<float>*>(pBuffer);
    #endif
}



/*-------------------------------------
 * Retrieve 4 raw texels (const)
-------------------------------------*/
template <typename data_t>
inline LS_INLINE ls::math::vec4_t<data_t> SL_Texture::raw_texel4(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    const data_t* pBuffer = reinterpret_cast<data_t*>(mTexels) + index;
    return ls::math::vec4_t<data_t>{pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]};
}



template <>
inline LS_INLINE ls::math::vec4_t<float> SL_Texture::raw_texel4<float>(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    const float* pBuffer = reinterpret_cast<const float*>(mTexels) + index;

    #if defined(LS_ARCH_X86)
        return ls::math::vec4_t<float>{_mm_loadu_ps(pBuffer)};
    #elif defined(LS_ARM_NEON)
        return ls::math::vec4_t<float>{vld1q_f32(pBuffer)};
    #else
        return *reinterpret_cast<const ls::math::vec4_t<float>*>(pBuffer);
    #endif
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type SL_Texture::raw_texel(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type& SL_Texture::raw_texel(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type SL_Texture::raw_texel(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type& SL_Texture::raw_texel(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type SL_Texture::raw_texel(ptrdiff_t index) const noexcept
{
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type& SL_Texture::raw_texel(ptrdiff_t index) noexcept
{
    return reinterpret_cast<color_type*>(mTexels)[index];
}



#endif /* SL_TEXTURE_HPP */
