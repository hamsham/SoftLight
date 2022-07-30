
#include <cstddef> // ptrdiff_t

#include "lightsky/setup/OS.h"

// Textures on POSIX-based systems are page-aligned. On windows, they're
// aligned to the size of a vector register (__m256 or float32x4_t).
#if !defined(LS_OS_WINDOWS)
    #include <unistd.h> // posix_memalign(), sysconf(_SC_PAGESIZE)
#endif

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h" // aligned allocation

#include "softlight/SL_ImgFile.hpp"
#include "softlight/SL_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace
{
/*-------------------------------------
 *
-------------------------------------*/
char* _sl_allocate_texture(size_t w, size_t h, size_t d, size_t bpt) noexcept
{
    // Z-ordered textures need to be padded if they are not divisible by SL_TEXELS_PER_CHUNK
    w = w + (SL_TEXELS_PER_CHUNK - (w % SL_TEXELS_PER_CHUNK));
    h = h + (SL_TEXELS_PER_CHUNK - (h % SL_TEXELS_PER_CHUNK));
    d = d + (SL_TEXELS_PER_CHUNK - (d % SL_TEXELS_PER_CHUNK));

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
char* _sl_copy_texture(size_t w, size_t h, size_t d, size_t bpt, const char* pData) noexcept
{
    if (!pData)
    {
        return nullptr;
    }

    // Z-ordered textures need to be padded if they are not divisible by SL_TEXELS_PER_CHUNK
    w = w + (SL_TEXELS_PER_CHUNK - (w % SL_TEXELS_PER_CHUNK));
    h = h + (SL_TEXELS_PER_CHUNK - (h % SL_TEXELS_PER_CHUNK));
    d = d + (SL_TEXELS_PER_CHUNK - (d % SL_TEXELS_PER_CHUNK));

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



/*-----------------------------------------------------------------------------
 * SL_TextureView
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Reset a texture view
-------------------------------------*/
void sl_reset(SL_TextureView& view) noexcept
{
    view.width = 0;
    view.height = 0;
    view.depth = 0;
    view.bytesPerTexel = 0;
    view.numChannels = 0;
    view.pTexels = nullptr;
    view.type = SL_COLOR_RGB_DEFAULT;
}



/*-------------------------------------
 * Create a Texture view from pre-supplied data
-------------------------------------*/
void sl_texture_view_from_buffer(SL_TextureView& outView, uint16_t w, uint16_t h, uint16_t d, SL_ColorDataType type, void* pTexels) noexcept
{
    outView.width         = w;
    outView.height        = h;
    outView.depth         = d;
    outView.bytesPerTexel = (uint16_t)sl_bytes_per_color(type);;
    outView.numChannels   = (uint16_t)sl_elements_per_color(type);
    outView.pTexels       = (char*)pTexels;
    outView.type          = type;
}



/*-----------------------------------------------------------------------------
 * SL_Texture
-----------------------------------------------------------------------------*/
/*-------------------------------------
 *
-------------------------------------*/
SL_Texture::~SL_Texture() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Texture::SL_Texture() noexcept :
    mView
    {
        0,
        0,
        0,
        0,
        0,
        nullptr,
        SL_COLOR_RGB_DEFAULT
    }
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_Texture::SL_Texture(const SL_Texture& r) noexcept :
    mView
    {
        r.mView.width,
        r.mView.height,
        r.mView.depth,
        r.mView.bytesPerTexel,
        r.mView.numChannels,
        _sl_copy_texture(r.mView.width, r.mView.height, r.mView.depth, r.mView.bytesPerTexel, r.mView.pTexels),
        r.mView.type
    }
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_Texture::SL_Texture(SL_Texture&& r) noexcept :
    mView
    {
        r.mView.width,
        r.mView.height,
        r.mView.depth,
        r.mView.bytesPerTexel,
        r.mView.numChannels,
        r.mView.pTexels,
        r.mView.type
    }
{
    r.mView.width = 0;
    r.mView.height = 0;
    r.mView.depth = 0;
    r.mView.bytesPerTexel = 0;
    r.mView.numChannels = 0;
    r.mView.pTexels = nullptr;
    r.mView.type = SL_COLOR_RGB_DEFAULT;
}




/*-------------------------------------
 *
-------------------------------------*/
SL_Texture& SL_Texture::operator=(const SL_Texture& r) noexcept
{
    if (this == &r)
    {
        return *this;
    }

    terminate();

    mView.width = r.mView.width;
    mView.height = r.mView.height;
    mView.depth = r.mView.depth;
    mView.bytesPerTexel = r.mView.bytesPerTexel;
    mView.numChannels = r.mView.numChannels;
    mView.pTexels = _sl_copy_texture(r.mView.width, r.mView.height, r.mView.depth, r.mView.bytesPerTexel, r.mView.pTexels);
    mView.type = r.mView.type;

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Texture& SL_Texture::operator=(SL_Texture&& r) noexcept
{
    if (this == &r)
    {
        return *this;
    }

    terminate();

    mView.width = r.mView.width;
    r.mView.width = 0;

    mView.height = r.mView.height;
    r.mView.height = 0;

    mView.depth = r.mView.depth;
    r.mView.depth = 0;

    mView.numChannels = r.mView.numChannels;
    r.mView.numChannels = 0;

    mView.bytesPerTexel = r.mView.bytesPerTexel;
    r.mView.bytesPerTexel = 0;

    mView.pTexels = r.mView.pTexels;
    r.mView.pTexels = nullptr;

    mView.type = r.mView.type;
    r.mView.type = SL_COLOR_RGB_DEFAULT;

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_Texture::init(SL_ColorDataType type, uint16_t w, uint16_t h, uint16_t d) noexcept
{
    const size_t bpt = sl_bytes_per_color(type);
    char* pData = _sl_allocate_texture(w, h, d, bpt);

    if (!pData)
    {
        return  -1;
    }

    if (mView.pTexels)
    {
        terminate();
    }

    sl_texture_view_from_buffer(mView, w, h, d, type, pData);

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_Texture::init(const SL_ImgFile& imgFile, SL_TexelOrder texelOrder) noexcept
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

        if (texelOrder == SL_TexelOrder::SWIZZLED)
        {
            const size_t bytesPerColor = mView.bytesPerTexel;

            for (uint16_t z = 0; z < mView.depth; ++z)
            {
                for (uint16_t y = 0; y < mView.height; ++y)
                {
                    for (uint16_t x = 0; x < mView.width; ++x)
                    {
                        const ptrdiff_t index = x + mView.width * (y + mView.height * z);
                        const ptrdiff_t offset = bytesPerColor * index;
                        set_texel<SL_TexelOrder::SWIZZLED>(x, y, z, pInTex + offset);
                    }
                }
            }
        }
        else
        {
            ls::utils::fast_memcpy(mView.pTexels, pInTex, imgFile.num_bytes());
        }
    }

    return retCode;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Texture::terminate() noexcept
{
    mView.width = 0;
    mView.height = 0;
    mView.depth = 0;
    mView.bytesPerTexel = 0;
    mView.numChannels = 0;

    #if defined(LS_OS_WINDOWS)
    ls::utils::aligned_free(mView.pTexels);
    #else
    free(mView.pTexels);
    #endif

    mView.pTexels = nullptr;

    mView.type = SL_COLOR_RGB_DEFAULT;
}
