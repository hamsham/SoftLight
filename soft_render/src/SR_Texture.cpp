
#include <cstddef> // ptrdiff_t

#include "lightsky/utils/Assertions.h"
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
char* _sr_allocate_texture(uint16_t w, uint16_t h, uint16_t d, size_t bpt)
{
    // 8 pixels can be acquired at a time
    const uint16_t wAlignment = 8 - (w%8);
    const uint16_t hAlignment = 8 - (h%8);
    const size_t   numBytes   = (w + wAlignment) * (h + hAlignment) * d * bpt;
    char* const    pTexels    = (char*)ls::utils::aligned_malloc(numBytes);

    ls::utils::fast_memset(pTexels, 0, numBytes);

    return pTexels;
}


/*-------------------------------------
 *
-------------------------------------*/
char* _sr_copy_texture(size_t w, size_t h, size_t d, size_t bpt, const char* pData)
{
    if (!pData)
    {
        return nullptr;
    }

    // 8 pixels can be acquired at a time
    const uint16_t wAlignment = 8 - (w%8);
    const uint16_t hAlignment = 8 - (h%8);
    const size_t   numBytes   = (w + wAlignment) * (h + hAlignment) * d * bpt;
    char* const    pTexels    = (char*)ls::utils::aligned_malloc(numBytes);

    ls::utils::fast_memcpy(pTexels, pData, numBytes);

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
    mWidth{0},
    mHeight{0},
    mDepth{0},
    mWrapping{SR_TEXTURE_WRAP_DEFAULT},
    mWidthf{0.f},
    mHeightf{0.f},
    mDepthf{0.f},
    mTexels{nullptr},
    mType{SR_COLOR_RGB_DEFAULT},
    mBytesPerTexel{0}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture::SR_Texture(const SR_Texture& r) noexcept :
    mWidth{r.mWidth},
    mHeight{r.mHeight},
    mDepth{r.mDepth},
    mWrapping{r.mWrapping},
    mWidthf{r.mWidthf},
    mHeightf{r.mHeightf},
    mDepthf{r.mDepthf},
    mTexels{_sr_copy_texture(r.mWidth, r.mHeight, r.mDepth, r.mBytesPerTexel, r.mTexels)},
    mType{r.mType},
    mBytesPerTexel{r.mBytesPerTexel}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture::SR_Texture(SR_Texture&& r) noexcept :
    mWidth{r.mWidth},
    mHeight{r.mHeight},
    mDepth{r.mDepth},
    mWrapping{r.mWrapping},
    mWidthf{r.mWidthf},
    mHeightf{r.mHeightf},
    mDepthf{r.mDepthf},
    mTexels{r.mTexels},
    mType{r.mType},
    mBytesPerTexel{r.mBytesPerTexel}
{
    r.mWidth = 0;
    r.mHeight = 0;
    r.mDepth = 0;
    r.mWrapping = SR_TEXTURE_WRAP_DEFAULT;
    r.mWidthf = 0.f;
    r.mHeightf = 0.f;
    r.mDepthf = 0.f;
    r.mTexels = nullptr;
    r.mType = SR_COLOR_RGB_DEFAULT;
    r.mBytesPerTexel = 0;
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

    mWidth = r.mWidth;
    mHeight = r.mHeight;
    mDepth = r.mDepth;
    mWrapping = r.mWrapping;
    mTexels = _sr_copy_texture(r.mWidth, r.mHeight, r.mDepth, r.mBytesPerTexel, r.mTexels);
    mType = r.mType;
    mBytesPerTexel = r.mBytesPerTexel;

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

    mWidth = r.mWidth;
    r.mWidth = 0;

    mHeight = r.mHeight;
    r.mHeight = 0;

    mDepth = r.mDepth;
    r.mDepth = 0;

    mWrapping = r.mWrapping;
    r.mWrapping = SR_TEXTURE_WRAP_DEFAULT;

    mWidth = r.mWidth;
    r.mWidthf = 0.f;

    mHeightf = r.mHeightf;
    r.mHeightf = 0.f;

    mDepthf = r.mDepthf;
    r.mDepthf = 0.f;

    mTexels = r.mTexels;
    r.mTexels = nullptr;

    mType = r.mType;
    r.mType = SR_COLOR_RGB_DEFAULT;

    mBytesPerTexel = r.mBytesPerTexel;
    r.mBytesPerTexel = 0;

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

    // Z-ordered textures need to be padded if they are not divisible by SR_TEXELS_PER_CHUNK
    #ifdef SR_TEXTURE_Z_ORDERING
        const uint16_t w0 = w + (SR_TEXELS_PER_CHUNK - (w % SR_TEXELS_PER_CHUNK));
        const uint16_t h0 = h + (SR_TEXELS_PER_CHUNK - (h % SR_TEXELS_PER_CHUNK));
    #else
        const uint16_t w0 = w;
        const uint16_t h0 = h;
    #endif

    const size_t bpt = sr_bytes_per_color(type);
    char* pData = _sr_allocate_texture(w0, h0, d, bpt);

    if (!pData)
    {
        return  -1;
    }

    if (mTexels)
    {
        terminate();
    }

    mWidth         = w;
    mHeight        = h;
    mDepth         = d;
    mWrapping      = SR_TEXTURE_WRAP_DEFAULT;
    mWidthf        = (float)w;
    mHeightf       = (float)h;
    mDepthf        = (float)d;
    mTexels        = pData;
    mType          = type;
    mBytesPerTexel = (uint32_t)bpt;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_Texture::init(const SR_ImgFile& imgFile) noexcept
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

        #ifdef SR_TEXTURE_Z_ORDERING
        const size_t bytesPerColor = mBytesPerTexel;

        for (uint16_t z = 0; z < mDepth; ++z)
        {
            for (uint16_t y = 0; y < mHeight; ++y)
            {
                for (uint16_t x = 0; x < mWidth; ++x)
                {
                    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
                    const ptrdiff_t offset = bytesPerColor * index;
                    set_texel(x, y, z, pInTex + offset);
                }
            }
        }
        #else
        ls::utils::fast_memcpy(mTexels, pInTex, imgFile.num_bytes());
        #endif
    }

    return retCode;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Texture::terminate() noexcept
{
    mWidth = 0;
    mHeight = 0;
    mDepth = 0;
    mWrapping = SR_TEXTURE_WRAP_DEFAULT;
    mWidthf = 0.f;
    mHeightf = 0.f;
    mDepthf = 0.f;

    ls::utils::aligned_free(mTexels);
    mTexels = nullptr;

    mType = SR_COLOR_RGB_DEFAULT;
    mBytesPerTexel = 0;
}



/*-------------------------------------
 * Retrieve a raw texels
-------------------------------------*/
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

                set_texel(x, y, z, pSrc+offset);
            }
        }
    }
}
