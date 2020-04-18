
#include <cstddef> // ptrdiff_t

#include "lightsky/setup/OS.h"

// Textures on POSIX-based systems are page-aligned. On windows, they're
// aligned to the size of a vector register (__m256 or float32x4_t).
#if !defined(LS_OS_WINDOWS)
    #include <unistd.h> // posix_memalign(), sysconf(_SC_PAGESIZE)
#endif

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h" // aligned allocation

#include "soft_render/SR_ImgFile.hpp"
#include "soft_render/SR_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace
{
/*-------------------------------------
 *
-------------------------------------*/
char* _sr_allocate_texture(size_t w, size_t h, size_t d, size_t bpt) noexcept
{
    // Z-ordered textures need to be padded if they are not divisible by SR_TEXELS_PER_CHUNK
    w = w + (SR_TEXELS_PER_CHUNK - (w % SR_TEXELS_PER_CHUNK));
    h = h + (SR_TEXELS_PER_CHUNK - (h % SR_TEXELS_PER_CHUNK));
    d = d + (SR_TEXELS_PER_CHUNK - (d % SR_TEXELS_PER_CHUNK));

    // 8 pixels can be acquired at a time
    const size_t numBytes     = w * h * d * bpt;
    const size_t maxRemaining = 4 * bpt;
    const size_t texAlignment = maxRemaining - (numBytes % maxRemaining);
    size_t numAlignedBytes    = numBytes + texAlignment;

    #if defined(LS_OS_WINDOWS)
        char* const pTexels = (char*)ls::utils::aligned_malloc(numAlignedBytes);
    #else
        char* pTexels = nullptr;
        if (0 != posix_memalign((void**)&pTexels, sysconf(_SC_PAGESIZE), numAlignedBytes))
        {
            return nullptr;
        }
    #endif

    ls::utils::fast_memset(pTexels, 0, numAlignedBytes);

    return pTexels;
}


/*-------------------------------------
 *
-------------------------------------*/
char* _sr_copy_texture(size_t w, size_t h, size_t d, size_t bpt, const char* pData) noexcept
{
    if (!pData)
    {
        return nullptr;
    }

    // Z-ordered textures need to be padded if they are not divisible by SR_TEXELS_PER_CHUNK
    w = w + (SR_TEXELS_PER_CHUNK - (w % SR_TEXELS_PER_CHUNK));
    h = h + (SR_TEXELS_PER_CHUNK - (h % SR_TEXELS_PER_CHUNK));
    d = d + (SR_TEXELS_PER_CHUNK - (d % SR_TEXELS_PER_CHUNK));

    // 8 pixels can be acquired at a time
    const size_t numBytes     = w * h * d * bpt;
    const size_t maxRemaining = 4 * bpt;
    const size_t texAlignment = maxRemaining - (numBytes % maxRemaining);
    size_t numAlignedBytes    = numBytes + texAlignment;

    #if defined(LS_OS_WINDOWS)
        char* const pTexels = (char*)ls::utils::aligned_malloc(numAlignedBytes);
    #else
        char* pTexels = nullptr;
        if (0 != posix_memalign((void**)&pTexels, sysconf(_SC_PAGESIZE), numAlignedBytes))
        {
            return nullptr;
        }
    #endif

    ls::utils::fast_memcpy(pTexels, pData, numAlignedBytes);

    return pTexels;
}



} // end anonymous namespace



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture::~SR_Texture() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture::SR_Texture() noexcept :
    mWrapping{SR_TEXTURE_WRAP_DEFAULT},
    mWidth{0},
    mHeight{0},
    mDepth{0},
    mType{SR_COLOR_RGB_DEFAULT},
    mBytesPerTexel{0},
    mNumChannels{0},
    mTexels{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture::SR_Texture(const SR_Texture& r) noexcept :
    mWrapping{r.mWrapping},
    mWidth{r.mWidth},
    mHeight{r.mHeight},
    mDepth{r.mDepth},
    mType{r.mType},
    mBytesPerTexel{r.mBytesPerTexel},
    mNumChannels{r.mNumChannels},
    mTexels{_sr_copy_texture(r.mWidth, r.mHeight, r.mDepth, r.mBytesPerTexel, r.mTexels)}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture::SR_Texture(SR_Texture&& r) noexcept :
    mWrapping{r.mWrapping},
    mWidth{r.mWidth},
    mHeight{r.mHeight},
    mDepth{r.mDepth},
    mType{r.mType},
    mBytesPerTexel{r.mBytesPerTexel},
    mNumChannels{r.mNumChannels},
    mTexels{r.mTexels}
{
    r.mWrapping = SR_TEXTURE_WRAP_DEFAULT;
    r.mWidth = 0;
    r.mHeight = 0;
    r.mDepth = 0;
    r.mType = SR_COLOR_RGB_DEFAULT;
    r.mBytesPerTexel = 0;
    r.mNumChannels = 0;
    r.mTexels = nullptr;
}




/*-------------------------------------
 *
-------------------------------------*/
SR_Texture& SR_Texture::operator=(const SR_Texture& r) noexcept
{
    if (this == &r)
    {
        return *this;
    }

    terminate();

    mWrapping = r.mWrapping;
    mWidth = r.mWidth;
    mHeight = r.mHeight;
    mDepth = r.mDepth;
    mType = r.mType;
    mBytesPerTexel = r.mBytesPerTexel;
    mNumChannels = r.mNumChannels;
    mTexels = _sr_copy_texture(r.mWidth, r.mHeight, r.mDepth, r.mBytesPerTexel, r.mTexels);

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture& SR_Texture::operator=(SR_Texture&& r) noexcept
{
    if (this == &r)
    {
        return *this;
    }

    terminate();

    mWrapping = r.mWrapping;
    r.mWrapping = SR_TEXTURE_WRAP_DEFAULT;

    mWidth = r.mWidth;
    r.mWidth = 0;

    mHeight = r.mHeight;
    r.mHeight = 0;

    mDepth = r.mDepth;
    r.mDepth = 0;

    mType = r.mType;
    r.mType = SR_COLOR_RGB_DEFAULT;

    mNumChannels = r.mNumChannels;
    r.mNumChannels = 0;

    mBytesPerTexel = r.mBytesPerTexel;
    r.mBytesPerTexel = 0;

    mTexels = r.mTexels;
    r.mTexels = nullptr;

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_Texture::init(SR_ColorDataType type, uint16_t w, uint16_t h, uint16_t d) noexcept
{
    LS_DEBUG_ASSERT(w > 0);
    LS_DEBUG_ASSERT(h > 0);
    LS_DEBUG_ASSERT(d > 0);

    const size_t bpt = sr_bytes_per_color(type);
    char* pData = _sr_allocate_texture(w, h, d, bpt);

    if (!pData)
    {
        return  -1;
    }

    if (mTexels)
    {
        terminate();
    }

    mWrapping      = SR_TEXTURE_WRAP_DEFAULT;
    mWidth         = w;
    mHeight        = h;
    mDepth         = d;
    mType          = type;
    mBytesPerTexel = (uint16_t)bpt;
    mNumChannels   = (uint32_t)sr_elements_per_color(type);
    mTexels        = pData;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_Texture::init(const SR_ImgFile& imgFile, SR_TexelOrder texelOrder) noexcept
{
    if (!imgFile.data())
    {
        return -1;
    }

    if (this->data())
    {
        return -2;
    }

    const size_t* const dimens = imgFile.size();

    if (dimens[0] > std::numeric_limits<uint16_t>::max()
    || dimens[1] > std::numeric_limits<uint16_t>::max()
    || dimens[2] > std::numeric_limits<uint16_t>::max())
    {
        return -3;
    }

    int retCode = this->init(imgFile.format(), (uint16_t)dimens[0], (uint16_t)dimens[1], (uint16_t)dimens[2]);

    if (retCode == 0)
    {
        const unsigned char* pInTex = reinterpret_cast<const unsigned char*>(imgFile.data());

        if (texelOrder == SR_TexelOrder::SR_TEXELS_SWIZZLED)
        {
            const size_t bytesPerColor = mBytesPerTexel;

            for (uint16_t z = 0; z < mDepth; ++z)
            {
                for (uint16_t y = 0; y < mHeight; ++y)
                {
                    for (uint16_t x = 0; x < mWidth; ++x)
                    {
                        const ptrdiff_t index = x + mWidth * (y + mHeight * z);
                        const ptrdiff_t offset = bytesPerColor * index;
                        set_texel<SR_TexelOrder::SR_TEXELS_SWIZZLED>(x, y, z, pInTex + offset);
                    }
                }
            }
        }
        else
        {
            ls::utils::fast_memcpy(mTexels, pInTex, imgFile.num_bytes());
        }
    }

    return retCode;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Texture::terminate() noexcept
{
    mWrapping = SR_TEXTURE_WRAP_DEFAULT;
    mWidth = 0;
    mHeight = 0;
    mDepth = 0;
    mType = SR_COLOR_RGB_DEFAULT;
    mBytesPerTexel = 0;
    mNumChannels = 0;

    #if defined(LS_OS_WINDOWS)
    ls::utils::aligned_free(mTexels);
    #else
    free(mTexels);
    #endif

    mTexels = nullptr;
}
