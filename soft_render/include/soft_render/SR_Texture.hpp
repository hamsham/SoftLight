
#ifndef SR_TEXTURE_HPP
#define SR_TEXTURE_HPP

#include <cstddef> // ptrdiff_t

#include "lightsky/setup/Arch.h"
#include "lightsky/setup/Api.h"

#include "lightsky/utils/Copy.h"

#include "lightsky/math/scalar_utils.h"

#include "soft_render/SR_Color.hpp" // SR_ColorDataType
#include "soft_render/SR_Geometry.hpp"

// x86 will grab 4 pixels at a time, swizzle on non-vectorized implementations.
#ifndef SR_TEXTURE_Z_ORDERING
    //#define SR_TEXTURE_Z_ORDERING
#endif



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



enum SR_TexChunkInfo : uint_fast32_t
{
    SR_TEXELS_PER_CHUNK = 4,
    SR_TEXEL_SHIFTS_PER_CHUNK = 2 // 2^SR_TEXEL_SHIFTS_PER_CHUNK = SR_TEXELS_PER_CHUNK
};



/**----------------------------------------------------------------------------
 * @brief Generic texture Class
 *
 * This class encodes textures using a Z-ordered curve.
-----------------------------------------------------------------------------*/
class SR_Texture
{
  private:
    uint16_t mWidth;

    uint16_t mHeight;

    uint16_t mDepth;

    SR_TexWrapMode mWrapping;

    float mWidthf;

    float mHeightf;

    float mDepthf;

    char* mTexels;

    SR_ColorDataType mType;

    uint32_t mBytesPerTexel;

  public:
    ~SR_Texture() noexcept;

    SR_Texture() noexcept;

    SR_Texture(const SR_Texture& t) noexcept;

    SR_Texture(SR_Texture&& t) noexcept;

    SR_Texture& operator=(const SR_Texture& t) noexcept;

    SR_Texture& operator=(SR_Texture&& t) noexcept;

    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y) const noexcept;

    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept;

    ptrdiff_t map_coordinate(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    ls::math::vec4_t<ptrdiff_t> map_coordinates(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept;

    uint16_t width() const noexcept;

    uint16_t height() const noexcept;

    uint16_t depth() const noexcept;

    uint32_t bpp() const noexcept;

    SR_TexWrapMode wrap_mode() const noexcept;

    void set_wrap_mode(const SR_TexWrapMode wrapMode) noexcept;

    int init(SR_ColorDataType type, uint16_t w, uint16_t h, uint16_t d = 1) noexcept;

    int init(const SR_ImgFile& imgFile) noexcept;

    void terminate() noexcept;

    SR_ColorDataType type() const noexcept;

    const void* data() const noexcept;

    void* data() noexcept;

    void set_texel(uint16_t x, uint16_t y, uint16_t z, const void* pData) noexcept;

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

    int wrap_coordinate(int uvw, int maxVal) const noexcept;

    template <typename color_type>
    const color_type texel(uint16_t x, uint16_t y, uint16_t z) const noexcept;

    template <typename color_type>
    color_type& texel(uint16_t x, uint16_t y, uint16_t z) noexcept;

    template <typename color_type>
    const color_type texel(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type>
    color_type& texel(uint16_t x, uint16_t y) noexcept;

    template <typename color_type>
    const ls::math::vec4_t<color_type> texel4(uint16_t x, uint16_t y) const noexcept;

    template <typename color_type>
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

    template <typename color_type>
    color_type nearest(float x, float y) const noexcept;

    template <typename color_type>
    color_type nearest(float x, float y, float z) const noexcept;

    template <typename color_type>
    color_type bilinear(float x, float y) const noexcept;

    template <typename color_type>
    color_type bilinear(float x, float y, float z) const noexcept;
};



/*-------------------------------------
 * Convert an X/Y coordinate to a Z-ordered coordinate.
-------------------------------------*/
inline ptrdiff_t SR_Texture::map_coordinate(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    #ifdef SR_TEXTURE_Z_ORDERING
        constexpr uint_fast32_t idsPerBlock = SR_TEXELS_PER_CHUNK*SR_TEXELS_PER_CHUNK;
        const uint_fast32_t     tileX       = x >> SR_TEXEL_SHIFTS_PER_CHUNK;
        const uint_fast32_t     tileY       = y >> SR_TEXEL_SHIFTS_PER_CHUNK;
        const uint_fast32_t     tileId      = (tileX + (mWidth >> SR_TEXEL_SHIFTS_PER_CHUNK) * tileY);

        // We're only getting the remainder of a power of 2. Use bit operations
        // instead of a modulo.
        const uint_fast32_t     innerX      = x & (SR_TEXELS_PER_CHUNK-1u);
        const uint_fast32_t     innerY      = y & (SR_TEXELS_PER_CHUNK-1u);
        const uint_fast32_t     innerId     = innerX + (innerY << SR_TEXEL_SHIFTS_PER_CHUNK);

        return innerId + tileId * idsPerBlock;
    #else
        return x + mWidth * y;
    #endif
}



/*-------------------------------------
 * Convert an X/Y coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
inline ls::math::vec4_t<ptrdiff_t> SR_Texture::map_coordinates(uint_fast32_t x, uint_fast32_t y) const noexcept
{
    #ifdef SR_TEXTURE_Z_ORDERING
        constexpr uint_fast32_t idsPerBlock = SR_TEXELS_PER_CHUNK*SR_TEXELS_PER_CHUNK;
        const uint_fast32_t     x0          = x;
        const uint_fast32_t     x1          = x+1;
        const uint_fast32_t     x2          = x+2;
        const uint_fast32_t     x3          = x+3;
        const uint_fast32_t     tileX0      = x0 >> SR_TEXEL_SHIFTS_PER_CHUNK;
        const uint_fast32_t     tileX1      = x1 >> SR_TEXEL_SHIFTS_PER_CHUNK;
        const uint_fast32_t     tileX2      = x2 >> SR_TEXEL_SHIFTS_PER_CHUNK;
        const uint_fast32_t     tileX3      = x3 >> SR_TEXEL_SHIFTS_PER_CHUNK;
        const uint_fast32_t     tileY       = y >> SR_TEXEL_SHIFTS_PER_CHUNK;
        const uint_fast32_t     xOffset     = (mWidth >> SR_TEXEL_SHIFTS_PER_CHUNK) * tileY;
        const uint_fast32_t     tileId0     = tileX0 + xOffset;
        const uint_fast32_t     tileId1     = tileX1 + xOffset;
        const uint_fast32_t     tileId2     = tileX2 + xOffset;
        const uint_fast32_t     tileId3     = tileX3 + xOffset;

        // We're only getting the remainder of a power of 2. Use bit operations
        // instead of a modulo.
        const uint_fast32_t     innerX0     = x0 & (SR_TEXELS_PER_CHUNK-1u);
        const uint_fast32_t     innerX1     = x1 & (SR_TEXELS_PER_CHUNK-1u);
        const uint_fast32_t     innerX2     = x2 & (SR_TEXELS_PER_CHUNK-1u);
        const uint_fast32_t     innerX3     = x3 & (SR_TEXELS_PER_CHUNK-1u);
        const uint_fast32_t     innerY      = y & (SR_TEXELS_PER_CHUNK-1u);
        const uint_fast32_t     innerId0    = innerX0 + (innerY << SR_TEXEL_SHIFTS_PER_CHUNK);
        const uint_fast32_t     innerId1    = innerX1 + (innerY << SR_TEXEL_SHIFTS_PER_CHUNK);
        const uint_fast32_t     innerId2    = innerX2 + (innerY << SR_TEXEL_SHIFTS_PER_CHUNK);
        const uint_fast32_t     innerId3    = innerX3 + (innerY << SR_TEXEL_SHIFTS_PER_CHUNK);

        return ls::math::vec4_t<ptrdiff_t>{
            (ptrdiff_t)(innerId0 + tileId0 * idsPerBlock),
            (ptrdiff_t)(innerId1 + tileId1 * idsPerBlock),
            (ptrdiff_t)(innerId2 + tileId2 * idsPerBlock),
            (ptrdiff_t)(innerId3 + tileId3 * idsPerBlock)
        };
    #else
        return ls::math::vec4_t<ptrdiff_t>{0, 1, 2, 3} + (x + mWidth * y);
    #endif
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to a Z-ordered coordinate.
-------------------------------------*/
inline ptrdiff_t SR_Texture::map_coordinate(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    #ifdef SR_TEXTURE_Z_ORDERING
        return map_coordinate(x, y) + (z * mWidth * mHeight);
    #else
        return x + mWidth * (y + mHeight * z);
    #endif
}



/*-------------------------------------
 * Convert an X/Y/Z coordinate to 4 Z-ordered coordinates.
-------------------------------------*/
inline ls::math::vec4_t<ptrdiff_t> SR_Texture::map_coordinates(uint_fast32_t x, uint_fast32_t y, uint_fast32_t z) const noexcept
{
    #ifdef SR_TEXTURE_Z_ORDERING
        return map_coordinates(x, y) + (ptrdiff_t)(z * mWidth * mHeight);
    #else
        return ls::math::vec4_t<ptrdiff_t>{0, 1, 2, 3} + x + mWidth * (y + mHeight * z);
    #endif
}



/*-------------------------------------
 * Get the texture width
-------------------------------------*/
inline uint16_t SR_Texture::width() const noexcept
{
    return mWidth;
}



/*-------------------------------------
 * Get the texture height
-------------------------------------*/
inline uint16_t SR_Texture::height() const noexcept
{
    return mHeight;
}



/*-------------------------------------
 * Get the texture depth
-------------------------------------*/
inline uint16_t SR_Texture::depth() const noexcept
{
    return mDepth;
}



/*-------------------------------------
 * Get the bytes per pixel
-------------------------------------*/
inline uint32_t SR_Texture::bpp() const noexcept
{
    return mBytesPerTexel;
}



/*-------------------------------------
 * Get the current wrapping mode
-------------------------------------*/
inline SR_TexWrapMode SR_Texture::wrap_mode() const noexcept
{
    return mWrapping;
}



/*-------------------------------------
 * Set the current wrapping mode
-------------------------------------*/
inline void SR_Texture::set_wrap_mode(const SR_TexWrapMode wrapMode) noexcept
{
    mWrapping = wrapMode;
}



/*-------------------------------------
 * Get the texture mType
-------------------------------------*/
inline SR_ColorDataType SR_Texture::type() const noexcept
{
    return mType;
}



/*-------------------------------------
 * Retrieve the raw texels
-------------------------------------*/
inline const void* SR_Texture::data() const noexcept
{
    return mTexels;
}



/*-------------------------------------
 * Retrieve the raw texels
-------------------------------------*/
inline void* SR_Texture::data() noexcept
{
    return mTexels;
}



/*-------------------------------------
 * Retrieve a raw texels
-------------------------------------*/
inline void SR_Texture::set_texel(uint16_t x, uint16_t y, uint16_t z, const void* pData) noexcept
{
    const ptrdiff_t index = map_coordinate(x, y, z);
    const size_t bytesPerColor = mBytesPerTexel;
    const ptrdiff_t offset = bytesPerColor * index;

    ls::utils::fast_memcpy(mTexels+offset, pData, bytesPerColor);
}



/*-------------------------------------
 * Keep all UV values within the (0.f, 1.f) range.
-------------------------------------*/
inline float SR_Texture::wrap_coordinate(float uvw) const noexcept
{
    return (mWrapping == SR_TEXTURE_WRAP_REPEAT)
           ? ((uvw < 0.f ? 1.f : 0.f) + ls::math::fmod_1(uvw))
           : ls::math::clamp(uvw, 0.f, 1.f);
}



/*-------------------------------------
 * Keep all UV values within the range (0, maxVal).
-------------------------------------*/
inline int SR_Texture::wrap_coordinate(int uvw, int maxVal) const noexcept
{
    return (mWrapping == SR_TEXTURE_WRAP_REPEAT)
           //? ((-(uvw < 0) & maxVal) + (uvw % maxVal))
           ? ((uvw % maxVal) + (-(uvw < 0) & maxVal))
           : ls::math::clamp<int>(uvw, 0, maxVal);
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type>
inline const color_type SR_Texture::texel(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = map_coordinate(x, y, z);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type>
inline color_type& SR_Texture::texel(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = map_coordinate(x, y, z);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel (const)
-------------------------------------*/
template <typename color_type>
inline const color_type SR_Texture::texel(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = map_coordinate(x, y);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a swizzled texel
-------------------------------------*/
template <typename color_type>
inline color_type& SR_Texture::texel(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = map_coordinate(x, y);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve 4 swizzled texels (const)
-------------------------------------*/
#ifndef SR_TEXTURE_Z_ORDERING
template <>
inline const ls::math::vec4_t<float> SR_Texture::texel4<float>(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = map_coordinate(x, y);
    const float* pTexels = reinterpret_cast<const float*>(mTexels) + index;

    #ifdef LS_ARCH_X86
        return ls::math::vec4_t<float>{_mm_loadu_ps(pTexels)};
    #elif defined(LS_ARCH_ARM)
        return ls::math::vec4_t<float>{vld1q_f32(pTexels)};
    #else
        return ls::math::vec4_t<color_type>{
            pTexels[0],
            pTexels[1],
            pTexels[2],
            pTexels[3]
        };
    #endif
}
#elif defined(LS_ARCH_X86)
template <>
inline const ls::math::vec4_t<float> SR_Texture::texel4<float>(uint16_t x, uint16_t y) const noexcept
{
    const ls::math::vec4_t<ptrdiff_t> index = map_coordinates(x, y);
    const __m128i indices = _mm_set_epi32(index[3], index[2], index[1], index[0]);
    const float* pTexels = reinterpret_cast<const float*>(mTexels);

    return ls::math::vec4_t<float>{_mm_i32gather_ps(pTexels, indices, 4)};
}
#endif



template <typename color_type>
inline const ls::math::vec4_t<color_type> SR_Texture::texel4(uint16_t x, uint16_t y) const noexcept
{
    #ifdef SR_TEXTURE_Z_ORDERING
        const ls::math::vec4_t<ptrdiff_t>&& index = map_coordinates(x, y);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels);
        return ls::math::vec4_t<color_type>{
            pTexels[index[0]],
            pTexels[index[1]],
            pTexels[index[2]],
            pTexels[index[3]]
        };
    #else
        const ptrdiff_t index = map_coordinate(x, y);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels) + index;
        return ls::math::vec4_t<color_type>{
            pTexels[0],
            pTexels[1],
            pTexels[2],
            pTexels[3]
        };
    #endif
}



/*-------------------------------------
 * Retrieve 4 swizzled texels (const)
-------------------------------------*/
template <typename color_type>
inline const ls::math::vec4_t<color_type> SR_Texture::texel4(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    #ifdef SR_TEXTURE_Z_ORDERING
        const ls::math::vec4_t<ptrdiff_t>&& index = map_coordinates(x, y, z);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels);
        return ls::math::vec4_t<color_type>{
            pTexels[index[0]],
            pTexels[index[1]],
            pTexels[index[2]],
            pTexels[index[3]]
        };
    #else
        const ptrdiff_t index = map_coordinate(x, y, z);
        const color_type* pTexels = reinterpret_cast<const color_type*>(mTexels);
        return ls::math::vec4_t<color_type>{
            pTexels[index+0],
            pTexels[index+1],
            pTexels[index+2],
            pTexels[index+3]
        };
    #endif
}



/*-------------------------------------
 * Retrieve 4 raw texels (const)
-------------------------------------*/
template <typename data_t>
inline ls::math::vec4_t<data_t> SR_Texture::raw_texel4(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    const data_t* pBuffer = reinterpret_cast<data_t*>(mTexels) + index;
    return ls::math::vec4_t<data_t>{pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]};
}



template <>
inline ls::math::vec4_t<float> SR_Texture::raw_texel4<float>(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    const float* pBuffer = reinterpret_cast<const float*>(mTexels) + index;

    #ifdef LS_ARCH_X86
        return ls::math::vec4_t<float>{_mm_loadu_ps(pBuffer)};
    #elif defined(LS_ARCH_ARM)
        return ls::math::vec4_t<float>{vld1q_f32(pBuffer)};
    #else
        return ls::math::vec4_t<float>{pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]};
    #endif
}



/*-------------------------------------
 * Retrieve 4 raw texels (const)
-------------------------------------*/
template <typename data_t>
inline ls::math::vec4_t<data_t> SR_Texture::raw_texel4(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    const data_t* pBuffer = reinterpret_cast<data_t*>(mTexels) + index;
    return ls::math::vec4_t<data_t>{pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]};
}



template <>
inline ls::math::vec4_t<float> SR_Texture::raw_texel4<float>(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    const float* pBuffer = reinterpret_cast<const float*>(mTexels) + index;

    #ifdef LS_ARCH_X86
        return ls::math::vec4_t<float>{_mm_loadu_ps(pBuffer)};
    #elif defined(LS_ARCH_ARM)
        return ls::math::vec4_t<float>{vld1q_f32(pBuffer)};
    #else
        return ls::math::vec4_t<float>{pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]};
    #endif
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline const color_type SR_Texture::raw_texel(uint16_t x, uint16_t y, uint16_t z) const noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline color_type& SR_Texture::raw_texel(uint16_t x, uint16_t y, uint16_t z) noexcept
{
    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline const color_type SR_Texture::raw_texel(uint16_t x, uint16_t y) const noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline color_type& SR_Texture::raw_texel(uint16_t x, uint16_t y) noexcept
{
    const ptrdiff_t index = x + mWidth * y;
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel (const)
-------------------------------------*/
template <typename color_type>
inline const color_type SR_Texture::raw_texel(ptrdiff_t index) const noexcept
{
    return reinterpret_cast<const color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Retrieve a raw texel
-------------------------------------*/
template <typename color_type>
inline color_type& SR_Texture::raw_texel(ptrdiff_t index) noexcept
{
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Nearest-neighbor lookup
-------------------------------------*/
template <typename color_type>
inline color_type SR_Texture::nearest(float x, float y) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y) < 0.f || ls::math::max(x, y) >= 1.f)) return color_type{0};

    const uint_fast32_t xi = (uint_fast32_t)(mWidthf * wrap_coordinate(x));
    const uint_fast32_t yi = (uint_fast32_t)(mHeightf * wrap_coordinate(y));
    const ptrdiff_t index = map_coordinate(xi, yi);

    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Nearest-neighbor lookup
-------------------------------------*/
template <typename color_type>
inline color_type SR_Texture::nearest(float x, float y, float z) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y, z) < 0.f || ls::math::max(x, y, z) >= 1.f)) return color_type{0};

    const uint_fast32_t xi = (uint_fast32_t)(mWidthf  * wrap_coordinate(x));
    const uint_fast32_t yi = (uint_fast32_t)(mHeightf * wrap_coordinate(y));
    const uint_fast32_t zi = (uint_fast32_t)(mDepthf  * wrap_coordinate(z));

    const ptrdiff_t index = map_coordinate(xi, yi, zi);
    return reinterpret_cast<color_type*>(mTexels)[index];
}



/*-------------------------------------
 * Bilinear Texture Lookup
-------------------------------------*/
template <typename color_type>
color_type SR_Texture::bilinear(float x, float y) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y) < 0.f || ls::math::max(x, y) >= 1.f)) return color_type{0};

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
    const color_type pixel0  = this->texel<color_type>(xi0, yi0);
    const color_type pixel1  = this->texel<color_type>(xi0, yi1);
    const color_type pixel2  = this->texel<color_type>(xi1, yi0);
    const color_type pixel3  = this->texel<color_type>(xi1, yi1);
    const float      weight0 = omdx * omdy;
    const float      weight1 = omdx * dy;
    const float      weight2 = dx * omdy;
    const float      weight3 = dx * dy;
    color_type       ret;

    switch (color_type::num_components())
    {
        case 4: ret[3] = (weight0*pixel0[3]) + (weight1*pixel1[3]) + (weight2*pixel2[3]) + (weight3*pixel3[3]);
        case 3: ret[2] = (weight0*pixel0[2]) + (weight1*pixel1[2]) + (weight2*pixel2[2]) + (weight3*pixel3[2]);
        case 2: ret[1] = (weight0*pixel0[1]) + (weight1*pixel1[1]) + (weight2*pixel2[1]) + (weight3*pixel3[1]);
        case 1: ret[0] = (weight0*pixel0[0]) + (weight1*pixel1[0]) + (weight2*pixel2[0]) + (weight3*pixel3[0]);
    }

    return ret;
}



/*-------------------------------------
 * Bilinear Texture Lookup
-------------------------------------*/
template <typename color_type>
color_type SR_Texture::bilinear(float x, float y, float z) const noexcept
{
    if (mWrapping == SR_TEXTURE_WRAP_CUTOFF && (ls::math::min(x, y, z) < 0.f || ls::math::max(x, y, z) >= 1.f)) return color_type{0};

    const float      xf      = wrap_coordinate(x) * mWidthf;
    const float      yf      = wrap_coordinate(y) * mHeightf;
    const uint16_t   zi      = (uint16_t)ls::math::round(wrap_coordinate(z) * mDepthf);
    const uint16_t   xi0     = (uint16_t)xf;
    const uint16_t   yi0     = (uint16_t)yf;
    const uint16_t   xi1     = ls::math::clamp<uint16_t>(xi0+1u, 0u, mWidth);
    const uint16_t   yi1     = ls::math::clamp<uint16_t>(yi0+1u, 0u, mHeight);
    const float      dx      = xf - (float)xi0;
    const float      dy      = yf - (float)yi0;
    const float      omdx    = 1.f - dx;
    const float      omdy    = 1.f - dy;
    const color_type pixel0  = this->texel<color_type>(xi0, yi0, zi);
    const color_type pixel1  = this->texel<color_type>(xi0, yi1, zi);
    const color_type pixel2  = this->texel<color_type>(xi1, yi0, zi);
    const color_type pixel3  = this->texel<color_type>(xi1, yi1, zi);
    const float      weight0 = omdx * omdy;
    const float      weight1 = omdx * dy;
    const float      weight2 = dx * omdy;
    const float      weight3 = dx * dy;
    color_type       ret;

    switch (color_type::num_components())
    {
        case 4: ret[3] = (weight0*pixel0[3]) + (weight1*pixel1[3]) + (weight2*pixel2[3]) + (weight3*pixel3[3]);
        case 3: ret[2] = (weight0*pixel0[2]) + (weight1*pixel1[2]) + (weight2*pixel2[2]) + (weight3*pixel3[2]);
        case 2: ret[1] = (weight0*pixel0[1]) + (weight1*pixel1[1]) + (weight2*pixel2[1]) + (weight3*pixel3[1]);
        case 1: ret[0] = (weight0*pixel0[0]) + (weight1*pixel1[0]) + (weight2*pixel2[0]) + (weight3*pixel3[0]);
    }

    return ret;
}


#endif /* SR_TEXTURE_HPP */
