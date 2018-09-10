
#include <cstddef> // ptrdiff_t
#include <utility> // std::move

#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_Texture.hpp"

namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace
{


/*-------------------------------------
 * Place a single pixel onto a texture
-------------------------------------*/
template <typename color_type>
inline void assign_pixel(
    uint16_t x,
    uint16_t y,
    uint16_t z,
    const SR_ColorRGBAf& rgba,
    SR_Texture* pTexture) noexcept
{
    // texture objects will truncate excess color components
    typedef typename color_type::value_type ConvertedType;
    union
    {
        SR_ColorRGBAType<typename color_type::value_type> rgba;
        SR_ColorRGBType<typename  color_type::value_type> rgb;
        SR_ColorRGType<typename   color_type::value_type> rg;
        SR_ColorRType<typename    color_type::value_type> r;
    } c{color_cast<typename color_type::value_type, float>(rgba)};

    color_type& outTexel = pTexture->texel<color_type>(x, y, z);

    // Should be optimized by the compiler
    switch (color_type::num_components())
    {
        case 4: *reinterpret_cast<SR_ColorRGBAType<ConvertedType>*>((void*)&outTexel) = c.rgba; break;
        case 3: *reinterpret_cast<SR_ColorRGBType<ConvertedType>*>((void*)&outTexel)  = c.rgb; break;
        case 2: *reinterpret_cast<SR_ColorRGType<ConvertedType>*>((void*)&outTexel)   = c.rg; break;
        case 1: *reinterpret_cast<SR_ColorRType<ConvertedType>*>((void*)&outTexel)    = c.r; break;
    }
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * FBO class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 *
-------------------------------------*/
SR_Framebuffer::~SR_Framebuffer() noexcept
{
    terminate();
    delete [] mColors;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Framebuffer::SR_Framebuffer() noexcept :
    mNumColors{0},
    mColors{nullptr},
    mDepth{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_Framebuffer::SR_Framebuffer(const SR_Framebuffer& r) noexcept :
    SR_Framebuffer{}
{
    *this = r;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Framebuffer::SR_Framebuffer(SR_Framebuffer&& f) noexcept :
    mNumColors{f.mNumColors},
    mColors{f.mColors},
    mDepth{f.mDepth}
{
    f.mNumColors = 0;
    f.mColors = nullptr;
    f.mDepth = nullptr;

}



/*-------------------------------------
 *
-------------------------------------*/
SR_Framebuffer& SR_Framebuffer::operator=(const SR_Framebuffer& f) noexcept
{
    if (this == &f)
    {
        return *this;
    }

    terminate();

    if (f.mNumColors)
    {
        SR_Texture** pTextures = new SR_Texture*[f.mNumColors];

        if (!*pTextures)
        {
            return *this;
        }

        for (uint64_t i = 0; i < f.mNumColors; ++i)
        {
            pTextures[i] = f.mColors[i];
        }

        mNumColors = f.mNumColors;
        mColors = pTextures;
        mDepth = f.mDepth;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Framebuffer& SR_Framebuffer::operator=(SR_Framebuffer&& f) noexcept
{
    if (this == &f)
    {
        return *this;
    }

    terminate();

    mNumColors = f.mNumColors;
    f.mNumColors = 0;

    mColors = f.mColors;
    f.mColors = nullptr;

    mDepth = f.mDepth;
    f.mDepth = nullptr;

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_Framebuffer::reserve_color_buffers(uint64_t numColorBuffers) noexcept
{
    if (numColorBuffers == mNumColors)
    {
        return 0;
    }

    if (numColorBuffers > 0)
    {
        const uint64_t numNewColors = math::min(mNumColors, numColorBuffers);
        SR_Texture** pNewBuffer = new SR_Texture*[numColorBuffers];

        if (!pNewBuffer)
        {
            return -1;
        }

        for (uint64_t i = 0; i < numColorBuffers; ++i)
        {
            pNewBuffer[i] = nullptr;
        }

        // keep old textures
        for (uint64_t i = 0; i < numNewColors; ++i)
        {
            pNewBuffer[i] = mColors[i];
        }

        delete [] mColors;
        mColors = pNewBuffer;
    }
    else
    {
        delete [] mColors;
        mColors = nullptr;
    }

    mNumColors = numColorBuffers;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_Framebuffer::attach_color_buffer(uint64_t index, SR_Texture& t) noexcept
{
    if (index >= mNumColors)
    {
        return -1;
    }

    if (mColors == nullptr)
    {
        return -2;
    }

    if (mColors[index] != nullptr)
    {
        return -3;
    }

    mColors[index] = &t;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture* SR_Framebuffer::detach_color_buffer(uint64_t index) noexcept
{
    SR_Texture* pTexture = nullptr;

    if (index < mNumColors)
    {
        pTexture = mColors[index];
        mColors[index] = nullptr;
    }

    return pTexture;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Framebuffer::clear_color_buffers() noexcept
{
    for (uint64_t i = 0; i < mNumColors; ++i)
    {
        SR_Texture* const pTex = mColors[i];

        if (pTex->data())
        {
            const uint64_t numBytes = pTex->bpp() * pTex->width() * pTex->height() * pTex->depth();
            ls::utils::fast_memset(pTex->data(), 0, numBytes);
        }
    }
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_Framebuffer::attach_depth_buffer(SR_Texture& d) noexcept
{
    if (mDepth != nullptr)
    {
        return -1;
    }

    switch (d.type())
    {
        case SR_ColorDataType::SR_COLOR_R_FLOAT:
        case SR_ColorDataType::SR_COLOR_R_DOUBLE:
            break;

        default:
            return -2;
    }

    mDepth = &d;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture* SR_Framebuffer::detach_depth_buffer() noexcept
{
    SR_Texture* pTexture = mDepth;
    mDepth = nullptr;
    return pTexture;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_Framebuffer::valid() const noexcept
{
    if (!mNumColors)
    {
        return -1;
    }

    uint16_t width = mColors[0]->width();
    uint16_t height = mColors[0]->height();
    uint16_t depth = mColors[0]->depth();

    for (uint64_t i = 0; i < mNumColors; ++i)
    {
        if (mColors[i] == nullptr)
        {
            return -2;
        }

        if (mColors[i]->data() == nullptr)
        {
            return -3;
        }

        if (mColors[i]->width() != width)
        {
            return -4;
        }

        if (mColors[i]->height() != height)
        {
            return -5;
        }

        if (mColors[i]->depth() != depth)
        {
            return -6;
        }
    }

    if (!mDepth)
    {
        return -7;
    }

    if (mDepth->data() == nullptr)
    {
        return -8;
    }

    if (mDepth->width() != width)
    {
        return -9;
    }

    if (mDepth->height() != height)
    {
        return -10;
    }

    if (mDepth->depth() > 1)
    {
        return -11;
    }

    if (mDepth->type() != SR_COLOR_R_FLOAT && mDepth->type() != SR_COLOR_R_DOUBLE)
    {
        return -12;
    }

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Framebuffer::terminate() noexcept
{

    for (uint64_t i = 0; i < mNumColors; ++i)
    {
        mColors[i] = nullptr;
    }

    mNumColors = 0;

    mDepth = nullptr;
}



/*-------------------------------------
 * Place a single pixel onto a texture
-------------------------------------*/
bool SR_Framebuffer::put_pixel(
    uint64_t targetId,
    uint16_t x,
    uint16_t y,
    uint16_t z,
    const SR_ColorRGBAf& rgba) noexcept
{
    SR_Texture* const pTexture = mColors[targetId];
    const SR_ColorDataType type = pTexture->type();

    switch (type)
    {
        case SR_COLOR_R_8U:        assign_pixel<SR_ColorR8>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RG_8U:       assign_pixel<SR_ColorRG8>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGB_8U:      assign_pixel<SR_ColorRGB8>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGBA_8U:     assign_pixel<SR_ColorRGBA8>(x, y, z, rgba, pTexture); break;

        case SR_COLOR_R_16U:       assign_pixel<SR_ColorR16>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RG_16U:      assign_pixel<SR_ColorRG16>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGB_16U:     assign_pixel<SR_ColorRGB16>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGBA_16U:    assign_pixel<SR_ColorRGBA16>(x, y, z, rgba, pTexture); break;

        case SR_COLOR_R_32U:       assign_pixel<SR_ColorR32>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RG_32U:      assign_pixel<SR_ColorRG32>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGB_32U:     assign_pixel<SR_ColorRGB32>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGBA_32U:    assign_pixel<SR_ColorRGBA32>(x, y, z, rgba, pTexture); break;

        case SR_COLOR_R_64U:       assign_pixel<SR_ColorR64>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RG_64U:      assign_pixel<SR_ColorRG64>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGB_64U:     assign_pixel<SR_ColorRGB64>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGBA_64U:    assign_pixel<SR_ColorRGBA64>(x, y, z, rgba, pTexture); break;

        case SR_COLOR_R_FLOAT:     assign_pixel<SR_ColorRf>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RG_FLOAT:    assign_pixel<SR_ColorRGf>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGB_FLOAT:   assign_pixel<SR_ColorRGBf>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGBA_FLOAT:  assign_pixel<SR_ColorRGBAf>(x, y, z, rgba, pTexture); break;

        case SR_COLOR_R_DOUBLE:    assign_pixel<SR_ColorRd>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RG_DOUBLE:   assign_pixel<SR_ColorRGd>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGB_DOUBLE:  assign_pixel<SR_ColorRGBd>(x, y, z, rgba, pTexture); break;
        case SR_COLOR_RGBA_DOUBLE: assign_pixel<SR_ColorRGBAd>(x, y, z, rgba, pTexture); break;

        default:
            break;
    }

    return true;
}



/*-------------------------------------
 *
-------------------------------------*/
uint16_t SR_Framebuffer::width() const noexcept
{
    if (mColors)
    {
        return mColors[0]->width();
    }

    if (mDepth)
    {
        return mDepth->width();
    }

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
uint16_t SR_Framebuffer::height() const noexcept
{
    if (mColors)
    {
        return mColors[0]->height();
    }

    if (mDepth)
    {
        return mDepth->height();
    }

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
uint16_t SR_Framebuffer::depth() const noexcept
{
    if (mColors)
    {
        return mColors[0]->depth();
    }

    return 0;
}
