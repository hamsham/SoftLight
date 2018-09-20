
#include <cstddef> // ptrdiff_t

#include "lightsky/utils/Assertions.h"

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
char* _sr_allocate_texture(uint16_t w, uint16_t h, uint16_t d, unsigned bpt)
{
    // 4 pixels can be acquired at a time
    const uint16_t alignment = 4 - (w%4);

    const size_t numBytes = (w + alignment) * h * d * bpt;
    char* pData = new char[numBytes];

    ls::utils::fast_memset(pData, 0, numBytes);

    return pData;
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

    ptrdiff_t numBytes = w * h * d * bpt;

    char* pTexture = new char[numBytes];
    ls::utils::fast_memcpy(pTexture, pData, numBytes);

    return pTexture;
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
    mTexels{r.mTexels},
    mType{r.mType},
    mBytesPerTexel{r.mBytesPerTexel}
{
    r.mWidth = 0;
    r.mHeight = 0;
    r.mDepth = 0;
    r.mWrapping = SR_TEXTURE_WRAP_DEFAULT;
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

    const int* const dimens = imgFile.size();

    if (dimens[0] > std::numeric_limits<uint16_t>::max()
    || dimens[1] > std::numeric_limits<uint16_t>::max()
    || dimens[2] > std::numeric_limits<uint16_t>::max())
    {
        return -3;
    }

    int retCode = this->init(imgFile.format(), dimens[0], dimens[1], dimens[2]);

    if (retCode == 0)
    {
        const unsigned char* pInTex = reinterpret_cast<const unsigned char*>(imgFile.data());

        #ifdef SR_TEXTURE_Z_ORDERING
        for (uint16_t z = 0; z < mDepth; ++z)
        {
            for (uint16_t y = 0; y < mHeight; ++y)
            {
                for (uint16_t x = 0; x < mWidth; ++x)
                {
                    const ptrdiff_t index = x + mWidth * (y + mHeight * z);
                    const size_t bytesPerColor = mBytesPerTexel;
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

    delete [] mTexels;
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

    for (uint16_t k = 0, z1 = z+d; k < d; ++k, ++z1)
    {
        for (uint16_t j = 0, y1 = y+h; j < h; ++j, ++y1)
        {
            for (uint16_t i = 0, x1 = x+w; i < w; ++i, ++x1)
            {
                const ptrdiff_t index = i + h * (j + w * k);
                const ptrdiff_t offset = (index * bytesPerColor);

                set_texel(x1, y1, z1, pSrc+offset);
            }
        }
    }
}
