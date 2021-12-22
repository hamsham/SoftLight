
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

    uint8_t mBytesPerTexel; // 1 byte

    uint8_t mNumChannels; // 1 byte

    alignas(alignof(uint64_t)) char* mTexels; // 4-8 bytes

    SL_ColorDataType mType; // 1 byte

  public:
    ~SL_Texture() noexcept;

    SL_Texture() noexcept;

    SL_Texture(const SL_Texture& t) noexcept;

    SL_Texture(SL_Texture&& t) noexcept;

    SL_Texture& operator=(const SL_Texture& t) noexcept;

    SL_Texture& operator=(SL_Texture&& t) noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::ORDERED>
    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y) const noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::ORDERED>
    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::ORDERED>
    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::ORDERED>
    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    uint16_t width() const noexcept;

    uint16_t height() const noexcept;

    uint16_t depth() const noexcept;

    uint16_t bpp() const noexcept;

    uint32_t channels() const noexcept;

    int init(SL_ColorDataType type, uint16_t w, uint16_t h, uint16_t d = 1) noexcept;

    int init(const SL_ImgFile& imgFile, SL_TexelOrder texelOrder = SL_TexelOrder::ORDERED) noexcept;

    void terminate() noexcept;

    SL_ColorDataType type() const noexcept;

    const void* data() const noexcept;

    void* data() noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::ORDERED>
    void set_texel(uint16_t x, uint16_t y, uint16_t z, const void* pData) noexcept;

    template <SL_TexelOrder order = SL_TexelOrder::ORDERED>
    void set_texels(
            uint16_t x,
            uint16_t y,
            uint16_t z,
            uint16_t w,
            uint16_t h,
            uint16_t d,
            const void* pData
    ) noexcept;

    template <typename color_type>
    const color_type texel(ptrdiff_t index) const noexcept;

    template <typename color_type>
    color_type& texel(ptrdiff_t index) noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::ORDERED>
    const color_type texel(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::ORDERED>
    color_type& texel(uint16_t x, uint16_t y) noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::ORDERED>
    const color_type texel(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::ORDERED>
    color_type& texel(uint16_t x, uint16_t y, uint16_t z) noexcept;

    template <typename color_type>
    const color_type* texel_pointer(ptrdiff_t index) const noexcept;

    template <typename color_type>
    color_type* texel_pointer(ptrdiff_t index) noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::ORDERED>
    const color_type* texel_pointer(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::ORDERED>
    color_type* texel_pointer(uint16_t x, uint16_t y) noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::ORDERED>
    const color_type* texel_pointer(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type, SL_TexelOrder order = SL_TexelOrder::ORDERED>
    color_type* texel_pointer(uint16_t x, uint16_t y, uint16_t z) noexcept;
};



/*-------------------------------------
 * Convert an X/Y coordinate to a Z-ordered coordinate.
-------------------------------------*/
template <>
inline LS_INLINE ptrdiff_t SL_Texture::map_coordinate<SL_TexelOrder::ORDERED>(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    return (ptrdiff_t)(x + (uint_fast32_t)mWidth * y);
}



template <>
inline LS_INLINE ptrdiff_t SL_Texture::map_coordinate<SL_TexelOrder::SWIZZLED>(uint_fast32_t x, uint_fast32_t y) const noexcept
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
inline LS_INLINE ptrdiff_t SL_Texture::map_coordinate<SL_TexelOrder::ORDERED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return (ptrdiff_t)(x + mWidth * (y + mHeight * z));
}



template <>
inline LS_INLINE ptrdiff_t SL_Texture::map_coordinate<SL_TexelOrder::SWIZZLED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return (ptrdiff_t)sl_swizzle_3d_index<SL_TEXELS_PER_CHUNK, SL_TEXEL_SHIFTS_PER_CHUNK>(x, y, z, mWidth, mHeight);
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
template <>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SL_Texture::map_coordinates<SL_TexelOrder::ORDERED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    return ls::math::vec4_t<ptrdiff_t>{0, 1, 2, 3} + x + mWidth * (y + mHeight * z);
}



template <>
inline LS_INLINE ls::math::vec4_t<ptrdiff_t> SL_Texture::map_coordinates<SL_TexelOrder::SWIZZLED>(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
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
    ptrdiff_t index;
    if (1 >= z)
    {
        index = map_coordinate<order>(x, y);
    }
    else
    {
        index = map_coordinate<order>(x, y, z);
    }

    const ptrdiff_t bpp    = mBytesPerTexel;
    const ptrdiff_t offset = bpp * index;
    char* const     pOut   = mTexels + offset;
    const char*     pIn    = reinterpret_cast<const char*>(pData);

    for (ptrdiff_t i = 0; i < bpp; ++i)
    {
        pOut[i] = pIn[i];
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
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type SL_Texture::texel(ptrdiff_t index) const noexcept
{
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type& SL_Texture::texel(ptrdiff_t index) noexcept
{
    return reinterpret_cast<color_type*>(mTexels)[index];
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
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline LS_INLINE const color_type* SL_Texture::texel_pointer(ptrdiff_t index) const noexcept
{
    return reinterpret_cast<const color_type*>(mTexels) + index;
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline LS_INLINE color_type* SL_Texture::texel_pointer(ptrdiff_t index) noexcept
{
    return reinterpret_cast<color_type*>(mTexels) + index;
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



#endif /* SL_TEXTURE_HPP */
