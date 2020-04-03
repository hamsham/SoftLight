
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
class SR_Texture
{
  public:
    typedef ls::math::long_medp_t fixed_type;

  private:
    SR_TexWrapMode mWrapping; // 2 bytes

    uint16_t mWidth; // 2 bytes

    uint16_t mHeight; // 2 bytes

    uint16_t mDepth; // 2 bytes

    float mWidthf; // 4 bytes

    float mHeightf; // 4 bytes

    float mDepthf; // 4 bytes

    SR_ColorDataType mType; // 2 bytes

    uint16_t mBytesPerTexel; // 2 bytes

    uint32_t mNumChannels;

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

    SR_TexWrapMode wrap_mode() const noexcept;

    void set_wrap_mode(const SR_TexWrapMode wrapMode) noexcept;

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

    float wrap_coordinate(float uvw) const noexcept;

    inline fixed_type wrap_coordinate(fixed_type uvw) const noexcept;

    int wrap_coordinate(int uvw, int maxVal) const noexcept;

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

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type nearest(float x, float y) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type nearest(float x, float y, float z) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type bilinear(float x, float y) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type bilinear(float x, float y, float z) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type trilinear(float x, float y) const noexcept;

    template <typename color_type, SR_TexelOrder order = SR_TexelOrder::SR_TEXELS_ORDERED>
    color_type trilinear(float x, float y, float z) const noexcept;
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
 * Get the current wrapping mode
-------------------------------------*/
inline LS_INLINE SR_TexWrapMode SR_Texture::wrap_mode() const noexcept
{
    return mWrapping;
}



/*-------------------------------------
 * Set the current wrapping mode
-------------------------------------*/
inline LS_INLINE void SR_Texture::set_wrap_mode(const SR_TexWrapMode wrapMode) noexcept
{
    mWrapping = wrapMode;
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

    while (bpp --> 0)
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

    for (uint16_t k = 0; z < d; ++k, ++z)
    {
        for (uint16_t j = 0; y < h; ++j, ++y)
        {
            for (uint16_t i = 0; x < w; ++i, ++x)
            {
                const ptrdiff_t index = i + w * (j + (h * k));
                const ptrdiff_t offset = (index * bytesPerColor);

                set_texel<order>(x, y, z, pSrc+offset);
            }
        }
    }
}



/*-------------------------------------
 * Keep all UV values within the (0.f, 1.f) range.
-------------------------------------*/
inline LS_INLINE float SR_Texture::wrap_coordinate(float uvw) const noexcept
{
    return (mWrapping == SR_TEXTURE_WRAP_REPEAT)
           ? ((uvw < 0.f ? 1.f : 0.f) + ls::math::fmod_1(uvw))
           : ls::math::clamp(uvw, 0.f, 1.f);
}



/*-------------------------------------
 * Keep all UV values within the (0.f, 1.f) range.
-------------------------------------*/
inline LS_INLINE SR_Texture::fixed_type SR_Texture::wrap_coordinate(SR_Texture::fixed_type uvw) const noexcept
{
    return (mWrapping == SR_TEXTURE_WRAP_REPEAT)
           ? ((uvw < fixed_type{0u}
               ? ls::math::fixed_cast<fixed_type>(1u)
               : fixed_type{0u}) + ls::math::fmod_1(uvw))
           : ls::math::clamp<fixed_type>(uvw, fixed_type{0u}, ls::math::fixed_cast<fixed_type>(1u));
}



/*-------------------------------------
 * Keep all UV values within the range (0, maxVal).
-------------------------------------*/
inline LS_INLINE int SR_Texture::wrap_coordinate(int uvw, int maxVal) const noexcept
{
    return (mWrapping == SR_TEXTURE_WRAP_REPEAT)
           //? ((-(uvw < 0) & maxVal) + (uvw % maxVal))
           ? ((uvw % maxVal) + (-(uvw < 0) & maxVal))
           : ls::math::clamp<int>(uvw, 0, maxVal);
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



/*-------------------------------------
 * Nearest-neighbor lookup
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE color_type SR_Texture::nearest(float x, float y) const noexcept
{
    color_type ret;

    if (mWrapping != SR_TEXTURE_WRAP_CUTOFF || (ls::math::min(x, y, 0.f, 0.f) >= 0.f && ls::math::max(x, y, 1.f, 1.f) <= 1.f))
    {
        #if 0
            const uint_fast32_t xi = (uint_fast32_t)(mWidthf  * wrap_coordinate(x));
            const uint_fast32_t yi = (uint_fast32_t)(mHeightf * wrap_coordinate(y));
        #else
            const fixed_type    xf = ls::math::fixed_cast<fixed_type, float>(x);
            const fixed_type    yf = ls::math::fixed_cast<fixed_type, float>(y);
            const uint_fast32_t xi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(mWidth) * wrap_coordinate(xf));
            const uint_fast32_t yi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(mHeight) * wrap_coordinate(yf));
        #endif

        const ptrdiff_t index = map_coordinate<order>(xi, yi);

        ret = reinterpret_cast<color_type*>(mTexels)[index];
    }
    else
    {
        ret = color_type{0};
    }

    return ret;
}



/*-------------------------------------
 * Nearest-neighbor lookup
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline LS_INLINE color_type SR_Texture::nearest(float x, float y, float z) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y, z) < 0.f || ls::math::max(x, y, z) >= 1.f))
    {
        return color_type{0};
    }

    #if 0
        const uint32_t xi = (uint_fast32_t)(mWidthf  * wrap_coordinate(x));
        const uint32_t yi = (uint_fast32_t)(mHeightf * wrap_coordinate(y));
        const uint32_t zi = (uint_fast32_t)(mDepthf  * wrap_coordinate(z));
    #else
        const fixed_type    xf = ls::math::fixed_cast<fixed_type, float>(x);
        const fixed_type    yf = ls::math::fixed_cast<fixed_type, float>(y);
        const fixed_type    zf = ls::math::fixed_cast<fixed_type, float>(z);
        const uint_fast32_t xi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(mWidth) * wrap_coordinate(xf));
        const uint_fast32_t yi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(mHeight) * wrap_coordinate(yf));
        const uint_fast32_t zi = ls::math::integer_cast<uint_fast32_t>(ls::math::fixed_cast<fixed_type, uint16_t>(mDepth) * wrap_coordinate(zf));
    #endif

    const ptrdiff_t index = map_coordinate<order>(xi, yi, zi);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Bilinear Texture Lookup
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline color_type SR_Texture::bilinear(float x, float y) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y) < 0.f || ls::math::max(x, y) >= 1.f))
    {
        return color_type{0};
    }

    const float      xf      = wrap_coordinate(x) * mWidthf;
    const float      yf      = wrap_coordinate(y) * mHeightf;
    const uint16_t   xi0     = (uint16_t)xf;
    const uint16_t   yi0     = (uint16_t)yf;
    const uint16_t   xi1     = ls::math::clamp<uint16_t>(xi0+1u, 0u, mWidth);
    const uint16_t   yi1     = ls::math::clamp<uint16_t>(yi0+1u, 0u, mHeight);
    const float      dx      = xf - (float)xi0;
    const float      dy      = yf - (float)yi0;
    const float      omdx    = 1.f - dx;
    const float      omdy    = 1.f - dy;
    const auto&&   pixel0  = color_cast<float, typename color_type::value_type>(texel<color_type, order>(xi0, yi0));
    const auto&&   pixel1  = color_cast<float, typename color_type::value_type>(texel<color_type, order>(xi0, yi1));
    const auto&&   pixel2  = color_cast<float, typename color_type::value_type>(texel<color_type, order>(xi1, yi0));
    const auto&&   pixel3  = color_cast<float, typename color_type::value_type>(texel<color_type, order>(xi1, yi1));
    const auto&&   weight0 = pixel0 * omdx * omdy;
    const auto&&   weight1 = pixel1 * omdx * dy;
    const auto&&   weight2 = pixel2 * dx * omdy;
    const auto&&   weight3 = pixel3 * dx * dy;

    const auto&& ret = ls::math::sum(weight0, weight1, weight2, weight3);

    return color_cast<typename color_type::value_type, float>(ret);
}



/*-------------------------------------
 * Bilinear Texture Lookup
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline color_type SR_Texture::bilinear(float x, float y, float z) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y, z) < 0.f || ls::math::max(x, y, z) >= 1.f))
    {
        return color_type{0};
    }

    const float    xf      = wrap_coordinate(x) * mWidthf;
    const float    yf      = wrap_coordinate(y) * mHeightf;
    const uint16_t zi      = (uint16_t)ls::math::round(wrap_coordinate(z) * mDepthf);
    const uint16_t xi0     = (uint16_t)xf;
    const uint16_t yi0     = (uint16_t)yf;
    const uint16_t xi1     = ls::math::clamp<uint16_t>(xi0+1u, 0u, mWidth);
    const uint16_t yi1     = ls::math::clamp<uint16_t>(yi0+1u, 0u, mHeight);
    const float    dx      = xf - (float)xi0;
    const float    dy      = yf - (float)yi0;
    const float    omdx    = 1.f - dx;
    const float    omdy    = 1.f - dy;
    const auto&&   pixel0  = color_cast<float, typename color_type::value_type>(texel<color_type, order>(xi0, yi0, zi));
    const auto&&   pixel1  = color_cast<float, typename color_type::value_type>(texel<color_type, order>(xi0, yi1, zi));
    const auto&&   pixel2  = color_cast<float, typename color_type::value_type>(texel<color_type, order>(xi1, yi0, zi));
    const auto&&   pixel3  = color_cast<float, typename color_type::value_type>(texel<color_type, order>(xi1, yi1, zi));
    const auto&&   weight0 = pixel0 * omdx * omdy;
    const auto&&   weight1 = pixel1 * omdx * dy;
    const auto&&   weight2 = pixel2 * dx * omdy;
    const auto&&   weight3 = pixel3 * dx * dy;

    const auto&& ret = ls::math::sum(weight0, weight1, weight2, weight3);

    return color_cast<typename color_type::value_type, float>(ret);
}



/*-------------------------------------
 * Trilinear Texture Lookup
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline color_type SR_Texture::trilinear(float x, float y) const noexcept
{
    return trilinear<color_type, order>(x, y, 0.f);
}



/*-------------------------------------
 * Trilinear Texture Lookup
-------------------------------------*/
template <typename color_type, SR_TexelOrder order>
inline color_type SR_Texture::trilinear(float x, float y, float z) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y, z) < 0.f || ls::math::max(x, y, z) >= 1.f))
    {
        return color_type{0};
    }

    /*
       V000 (1 - x) (1 - y) (1 - z) +
       V100 x (1 - y) (1 - z) +
       V010 (1 - x) y (1 - z) +
       V001 (1 - x) (1 - y) z +
       V101 x (1 - y) z +
       V011 (1 - x) y z +
       V110 x y (1 - z) +
       V111 x y z
     */

    namespace math = ls::math;

    // use "-1" to avoid out-of-bounds errors at texture edges.
    x = wrap_coordinate(x) * (mWidthf-1.f);
    y = wrap_coordinate(y) * (mHeightf-1.f);
    z = wrap_coordinate(z) * (mDepthf-1.f);

    // only use fixed-point calculation for determining texel indices.
    const fixed_type x0 = ls::math::fixed_cast<fixed_type, float>(x);
    const fixed_type y0 = ls::math::fixed_cast<fixed_type, float>(y);
    const fixed_type z0 = ls::math::fixed_cast<fixed_type, float>(z);
    const uint16_t   xi = ls::math::integer_cast<uint16_t, fixed_type>(x0);
    const uint16_t   yi = ls::math::integer_cast<uint16_t, fixed_type>(y0);
    const uint16_t   zi = ls::math::integer_cast<uint16_t, fixed_type>(z0);

    constexpr fixed_type one = math::fixed_cast<fixed_type, int>(1);
    constexpr fixed_type zero = math::fixed_cast<fixed_type, int>(0);
    const uint16_t si = math::integer_cast<uint16_t, fixed_type>(math::max(x0-one, zero));
    const uint16_t ti = math::integer_cast<uint16_t, fixed_type>(math::max(y0-one, zero));
    const uint16_t ri = math::integer_cast<uint16_t, fixed_type>(math::max(z0-one, zero));

    const math::vec3_t<uint16_t> uv000 = {si, ti, ri};
    const math::vec3_t<uint16_t> uv100 = {xi, ti, ri};
    const math::vec3_t<uint16_t> uv010 = {si, yi, ri};
    const math::vec3_t<uint16_t> uv001 = {si, ti, zi};
    const math::vec3_t<uint16_t> uv101 = {xi, ti, zi};
    const math::vec3_t<uint16_t> uv011 = {si, yi, zi};
    const math::vec3_t<uint16_t> uv110 = {xi, yi, ri};
    const math::vec3_t<uint16_t> uv111 = {xi, yi, zi};

    const auto&& c000 = color_cast<float, typename color_type::value_type>(texel<color_type, order>(uv000[0], uv000[1], uv000[2]));
    const auto&& c100 = color_cast<float, typename color_type::value_type>(texel<color_type, order>(uv100[0], uv100[1], uv100[2]));
    const auto&& c010 = color_cast<float, typename color_type::value_type>(texel<color_type, order>(uv010[0], uv010[1], uv010[2]));
    const auto&& c001 = color_cast<float, typename color_type::value_type>(texel<color_type, order>(uv001[0], uv001[1], uv001[2]));
    const auto&& c101 = color_cast<float, typename color_type::value_type>(texel<color_type, order>(uv101[0], uv101[1], uv101[2]));
    const auto&& c011 = color_cast<float, typename color_type::value_type>(texel<color_type, order>(uv011[0], uv011[1], uv011[2]));
    const auto&& c110 = color_cast<float, typename color_type::value_type>(texel<color_type, order>(uv110[0], uv110[1], uv110[2]));
    const auto&& c111 = color_cast<float, typename color_type::value_type>(texel<color_type, order>(uv111[0], uv111[1], uv111[2]));

    // floating-point math can be used for calculating the texel weights
    const float xf = x - math::floor(x);
    const float xd = 1.f - xf;
    const float yf = y - math::floor(y);
    const float yd = 1.f - yf;
    const float zf = z - math::floor(z);
    const float zd = 1.f - zf;

    const auto&& weight000 = c000 * xd*yd*zd;
    const auto&& weight100 = c100 * xf*yd*zd;
    const auto&& weight010 = c010 * xd*yf*zd;
    const auto&& weight001 = c001 * xd*yd*zf;
    const auto&& weight101 = c101 * xf*yd*zf;
    const auto&& weight011 = c011 * xd*yf*zf;
    const auto&& weight110 = c110 * xf*yf*zd;
    const auto&& weight111 = c111 * xf*yf*zf;

    const auto&& ret = math::sum(
        weight000,
        weight100,
        weight010,
        weight001,
        weight101,
        weight011,
        weight110,
        weight111
    );

    return color_cast<typename color_type::value_type, float>(ret);
}



#endif /* SR_TEXTURE_HPP */
