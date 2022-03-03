
#include "lightsky/setup/Compiler.h" // LS_COMPILER_MSC

#include "lightsky/math/vec_utils.h" // vector casting

#include "softlight/SL_Color.hpp"
#include "softlight/SL_ColorCompressed.hpp"

#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_PipelineState.hpp" // SL_BlendMode
#include "softlight/SL_Shader.hpp" // SL_FragmentParam

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
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // texture objects will truncate excess color components
    typedef typename color_type::value_type ConvertedType;

    // Get a reference to the source texel
    void* const outTexel = pTexture->texel_pointer<color_type>(x, y);
    const ls::math::vec4_t<ConvertedType>&& c = color_cast<ConvertedType, float>(rgba);

    // Should be optimized by the compiler
    switch (color_type::num_components())
    {
        case 4:
            *reinterpret_cast<SL_ColorRGBAType<ConvertedType>*>(outTexel) = c;
            break;

        case 3:
            *reinterpret_cast<SL_ColorRGBType<ConvertedType>*>(outTexel) = ls::math::vec3_cast<ConvertedType>(c);
            break;

        case 2:
            *reinterpret_cast<SL_ColorRGType<ConvertedType>*>(outTexel) = ls::math::vec2_cast<ConvertedType>(c);
            break;

        case 1:
            *reinterpret_cast<ConvertedType*>(outTexel) = c[0];
            break;

        default:
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Place a compressed pixel onto a texture
-------------------------------------*/
template <>
inline void assign_pixel<SL_ColorRGB565>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // Get a reference to the source texel
    SL_ColorRGB565* const outTexel = pTexture->texel_pointer<SL_ColorRGB565>(x, y);
    //*outTexel = rgb565_cast<float>(ls::math::vec3_cast(rgba));
    *outTexel = rgb565_cast<uint32_t>(ls::math::vec3_cast(color_cast<uint32_t, float>(rgba)));
}

template <>
inline void assign_pixel<SL_ColorRGB5551>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // Get a reference to the source texel
    SL_ColorRGB5551* const outTexel = pTexture->texel_pointer<SL_ColorRGB5551>(x, y);
    //*outTexel = rgb5551_cast<float>(rgba);
    *outTexel = rgb5551_cast<uint32_t>(color_cast<uint32_t, float>(rgba));
}

template <>
inline void assign_pixel<SL_ColorRGB4444>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // Get a reference to the source texel
    SL_ColorRGB4444* const outTexel = pTexture->texel_pointer<SL_ColorRGB4444>(x, y);
    //*outTexel = rgb4444_cast<float>(rgba);
    *outTexel = rgb4444_cast<uint32_t>(color_cast<uint32_t, float>(rgba));
}



/*-------------------------------------
 * Place a single pixel onto a texture
-------------------------------------*/
#if defined(LS_ARCH_X86)
template <>
inline void assign_pixel<SL_ColorRGBA8>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // Get a reference to the source texel
    int32_t* const  outTexel = pTexture->texel_pointer<int32_t>(x, y);
    SL_ColorRGBA8&& inTexel  = color_cast<uint8_t, float>(rgba);

    _mm_stream_si32(outTexel, reinterpret_cast<int32_t&>(inTexel));
    //*outTexel = reinterpret_cast<int32_t&>(inTexel);
}



template <>
inline void assign_pixel<SL_ColorRGBA16>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // Get a reference to the source texel

    // MSVConly supports __m64 on 32-bit builds
    #if defined(LS_COMPILER_MSC) && LS_ARCH_X86 >= 64
        __int64* const outTexel = pTexture->texel_pointer<__int64>(x, y);

        union
        {
            SL_ColorRGBA16 vec;
            __int64 scalar;
        } inTexel{color_cast<uint16_t, float>(rgba)};
    
        _mm_stream_si64(outTexel, inTexel.scalar);
    #else
        __m64* const outTexel = pTexture->texel_pointer<__m64>(x, y);
    
        union
        {
            SL_ColorRGBA16 vec;
            __m64 scalar;
        } inTexel{ color_cast<uint16_t, float>(rgba) };
    
        _mm_stream_pi(outTexel, inTexel.scalar);
        _mm_empty();
    #endif
}



template <>
inline void assign_pixel<SL_ColorRGBAf>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // Get a reference to the source texel
    SL_ColorRGBAf* const outTexel = pTexture->texel_pointer<SL_ColorRGBAf>(x, y);

    _mm_stream_ps(reinterpret_cast<float*>(outTexel), _mm_load_ps(&rgba));
}



#elif defined(LS_ARM_NEON)
template <>
inline void assign_pixel<SL_ColorRGBA8>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // Get a reference to the source texel
    uint32_t* const outTexel = pTexture->texel_pointer<uint32_t>(x, y);

    #if defined(LS_ARCH_AARCH64)
        const uint32x4_t color32 = vcvtq_u32_f32(vmulq_n_f32(rgba.simd, 255.f));
    #else
        const uint32x4_t color32 = vcvtq_u32_f32(vmulq_f32(rgba.simd, vdupq_n_f32(255.f)));
    #endif
    const uint16x8_t color16 = vcombine_u16(vmovn_u32(color32), vmovn_u32(color32));
    const uint8x8_t color8 = vmovn_u16(color16);
    vst1_lane_u32(outTexel, vreinterpret_u32_u8(color8), 0);
}



template <>
inline void assign_pixel<SL_ColorRGBA16>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture) noexcept
{
    // Get a reference to the source texel
    int64_t* const outTexel = pTexture->texel_pointer<int64_t>(x, y);

    #if defined(LS_ARCH_AARCH64)
        const uint32x4_t color32 = vcvtq_u32_f32(vmulq_n_f32(rgba.simd, 65536.f));
    #else
        const uint32x4_t color32 = vcvtq_u32_f32(vmulq_f32(rgba.simd, vdupq_n_f32(65536.f)));
    #endif

    vst1_u16(reinterpret_cast<uint16_t*>(outTexel), vmovn_u32(color32));
}

#endif



/*-------------------------------------
 * Place an alpha-blended pixel onto a texture
-------------------------------------*/
template <typename color_type>
inline void assign_alpha_pixel(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture,
    const SL_BlendMode blendMode) noexcept
{
    // texture objects will truncate excess color components
    typedef typename color_type::value_type ConvertedType;

    // Get a reference to the source texel
    void* const outTexel = pTexture->texel_pointer<color_type>(x, y);

    // sample the source texel
    union DestColor
    {
        SL_ColorRGBAType<float> rgba;
        SL_ColorRGBType<float> rgb;
        SL_ColorRGType<float> rg;
        SL_ColorRType<float> r;
    } s{rgba}, d{color_cast<float, typename color_type::value_type>(*reinterpret_cast<SL_ColorRGBAType<ConvertedType>*>(outTexel))};

    switch (color_type::num_components())
    {
        case 1:
            d.rgba = math::vec4{d.r.r, 0.f, 0.f, 1.f};
            break;

        case 2:
            d.rgba = math::vec4_cast<float>(d.rg, 0.f, 1.f);
            break;

        case 3:
            d.rgba = math::vec4_cast<float>(d.rgb, 1.f);
            break;

        default:
            break;
    }

    // This method of blending uses premultiplied alpha. I will need to support
    // configurable blend modes later.
    const math::vec4 srcAlpha = rgba[3];
    const math::vec4&& modulation = math::vec4{1.f - rgba[3]};

    if (blendMode == SL_BLEND_ALPHA)
    {
        const math::vec4&& dstMod = modulation * d.rgba[3];
        d.rgba[3] = dstMod[3] + srcAlpha[3];
        d.rgb = math::vec3_cast(math::fmadd(s.rgba, srcAlpha, (d.rgba * dstMod)) * math::rcp(d.rgba[3]));
    }
    else if (blendMode == SL_BLEND_PREMULTIPLED_ALPHA)
    {
        d.rgba = math::fmadd(d.rgba, modulation, s.rgba);
    }
    else if (blendMode == SL_BLEND_ADDITIVE)
    {
        d.rgba = math::fmadd(s.rgba, srcAlpha, d.rgba);
    }
    else if (blendMode == SL_BLEND_SCREEN)
    {
        d.rgba = (s.rgba*srcAlpha) + (d.rgba*modulation);
    }

    d.rgba = math::clamp(d.rgba, math::vec4{0.f, 0.f, 0.f, 0.f}, math::vec4{1.f, 1.f, 1.f, 1.f});

    switch (color_type::num_components())
    {
        case 4:
            *reinterpret_cast<SL_ColorRGBAType<ConvertedType>*>(outTexel) = color_cast<typename color_type::value_type, float>(d.rgba);
            break;

        case 3:
            *reinterpret_cast<SL_ColorRGBType<ConvertedType>*>(outTexel) = color_cast<typename color_type::value_type, float>(d.rgb);
            break;

        case 2:
            *reinterpret_cast<SL_ColorRGType<ConvertedType>*>(outTexel) = color_cast<typename color_type::value_type, float>(d.rg);
            break;

        case 1:
            *reinterpret_cast<SL_ColorRType<ConvertedType>*>(outTexel) = color_cast<typename color_type::value_type, float>(d.r);
            break;
    }
}

template <>
inline void assign_alpha_pixel<SL_ColorRGB565>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture,
    const SL_BlendMode blendMode) noexcept
{
    // Get a reference to the source texel
    SL_ColorRGB565* const outTexel = pTexture->texel_pointer<SL_ColorRGB565>(x, y);

    // sample the source texel
    SL_ColorRGBAType<float> s = rgba;
    SL_ColorRGBAType<float> d = math::vec4_cast(rgb_cast<float>(*outTexel), 1.f);

    // This method of blending uses premultiplied alpha. I will need to support
    // configurable blend modes later.
    const math::vec4 srcAlpha = rgba[3];
    const math::vec4&& modulation = math::vec4{1.f - rgba[3]};

    if (blendMode == SL_BLEND_ALPHA)
    {
        const math::vec4&& dstMod = modulation * d[3];
        d = math::fmadd(s, srcAlpha, (d * dstMod)) * math::rcp(d[3]);
        d[3] = dstMod[3] + srcAlpha[3];
    }
    else if (blendMode == SL_BLEND_PREMULTIPLED_ALPHA)
    {
        d = math::fmadd(d, modulation, s);
    }
    else if (blendMode == SL_BLEND_ADDITIVE)
    {
        d = math::fmadd(s, srcAlpha, d);
    }
    else if (blendMode == SL_BLEND_SCREEN)
    {
        d = (s*srcAlpha) + (d*modulation);
    }

    d = math::clamp(d, math::vec4{0.f, 0.f, 0.f, 0.f}, math::vec4{1.f, 1.f, 1.f, 1.f});
    *outTexel = rgb565_cast<float>(math::vec3_cast<float>(d));
}

template <>
inline void assign_alpha_pixel<SL_ColorRGB5551>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture,
    const SL_BlendMode blendMode) noexcept
{
    // Get a reference to the source texel
    SL_ColorRGB5551* const outTexel = pTexture->texel_pointer<SL_ColorRGB5551>(x, y);

    // sample the source texel
    SL_ColorRGBAType<float> s = rgba;
    SL_ColorRGBAType<float> d = rgb_cast<float>(*outTexel);

    // This method of blending uses premultiplied alpha. I will need to support
    // configurable blend modes later.
    const math::vec4 srcAlpha = rgba[3];
    const math::vec4&& modulation = math::vec4{1.f - rgba[3]};

    if (blendMode == SL_BLEND_ALPHA)
    {
        const math::vec4&& dstMod = modulation * d[3];
        d = math::fmadd(s, srcAlpha, (d * dstMod)) * math::rcp(d[3]);
        d[3] = dstMod[3] + srcAlpha[3];
    }
    else if (blendMode == SL_BLEND_PREMULTIPLED_ALPHA)
    {
        d = math::fmadd(d, modulation, s);
    }
    else if (blendMode == SL_BLEND_ADDITIVE)
    {
        d = math::fmadd(s, srcAlpha, d);
    }
    else if (blendMode == SL_BLEND_SCREEN)
    {
        d = (s*srcAlpha) + (d*modulation);
    }

    d = math::clamp(d, math::vec4{0.f, 0.f, 0.f, 0.f}, math::vec4{1.f, 1.f, 1.f, 1.f});
    *outTexel = rgb5551_cast<float>(d);
}

template <>
inline void assign_alpha_pixel<SL_ColorRGB4444>(
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba,
    SL_Texture* pTexture,
    const SL_BlendMode blendMode) noexcept
{
    // Get a reference to the source texel
    SL_ColorRGB4444* const outTexel = pTexture->texel_pointer<SL_ColorRGB4444>(x, y);

    // sample the source texel
    SL_ColorRGBAType<float> s = rgba;
    SL_ColorRGBAType<float> d = rgb_cast<float>(*outTexel);

    // This method of blending uses premultiplied alpha. I will need to support
    // configurable blend modes later.
    const math::vec4 srcAlpha = rgba[3];
    const math::vec4&& modulation = math::vec4{1.f - rgba[3]};

    if (blendMode == SL_BLEND_ALPHA)
    {
        const math::vec4&& dstMod = modulation * d[3];
        d = math::fmadd(s, srcAlpha, (d * dstMod)) * math::rcp(d[3]);
        d[3] = dstMod[3] + srcAlpha[3];
    }
    else if (blendMode == SL_BLEND_PREMULTIPLED_ALPHA)
    {
        d = math::fmadd(d, modulation, s);
    }
    else if (blendMode == SL_BLEND_ADDITIVE)
    {
        d = math::fmadd(s, srcAlpha, d);
    }
    else if (blendMode == SL_BLEND_SCREEN)
    {
        d = (s*srcAlpha) + (d*modulation);
    }

    d = math::clamp(d, math::vec4{0.f, 0.f, 0.f, 0.f}, math::vec4{1.f, 1.f, 1.f, 1.f});
    *outTexel = rgb4444_cast<float>(d);
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * FBO class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 *
-------------------------------------*/
SL_Framebuffer::~SL_Framebuffer() noexcept
{
    terminate();
    delete [] mColors;
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Framebuffer::SL_Framebuffer() noexcept :
    mNumColors{0},
    mColors{nullptr},
    mDepth{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_Framebuffer::SL_Framebuffer(const SL_Framebuffer& r) noexcept :
    SL_Framebuffer{}
{
    *this = r;
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Framebuffer::SL_Framebuffer(SL_Framebuffer&& f) noexcept :
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
SL_Framebuffer& SL_Framebuffer::operator=(const SL_Framebuffer& f) noexcept
{
    if (this == &f)
    {
        return *this;
    }

    terminate();

    if (f.mNumColors)
    {
        SL_Texture** pTextures = new SL_Texture*[f.mNumColors];

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
SL_Framebuffer& SL_Framebuffer::operator=(SL_Framebuffer&& f) noexcept
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
int SL_Framebuffer::reserve_color_buffers(uint64_t numColorBuffers) noexcept
{
    if (numColorBuffers == mNumColors)
    {
        return 0;
    }

    if (numColorBuffers > 0)
    {
        const uint64_t numNewColors = math::min(mNumColors, numColorBuffers);
        SL_Texture** pNewBuffer = new SL_Texture*[numColorBuffers];

        if (!pNewBuffer)
        {
            return -1;
        }

        for (uint64_t i = 0; i < numColorBuffers; ++i)
        {
            pNewBuffer[i] = nullptr;
        }

        // keep old textures
        if (mColors)
        {
            for (uint64_t i = 0; i < numNewColors; ++i)
            {
                pNewBuffer[i] = mColors[i];
            }

            delete [] mColors;
        }

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
int SL_Framebuffer::attach_color_buffer(uint64_t index, SL_Texture& t) noexcept
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
SL_Texture* SL_Framebuffer::detach_color_buffer(uint64_t index) noexcept
{
    SL_Texture* pTexture = nullptr;

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
void SL_Framebuffer::clear_color_buffers() noexcept
{
    for (uint64_t i = 0; i < mNumColors; ++i)
    {
        SL_Texture* const pTex = mColors[i];

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
int SL_Framebuffer::attach_depth_buffer(SL_Texture& d) noexcept
{
    if (mDepth != nullptr)
    {
        return -1;
    }

    switch (d.type())
    {
        case SL_ColorDataType::SL_COLOR_R_16U:
        case SL_ColorDataType::SL_COLOR_R_FLOAT:
        case SL_ColorDataType::SL_COLOR_R_DOUBLE:
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
SL_Texture* SL_Framebuffer::detach_depth_buffer() noexcept
{
    SL_Texture* pTexture = mDepth;
    mDepth = nullptr;
    return pTexture;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_Framebuffer::valid() const noexcept
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

    if (mDepth->type() != SL_COLOR_R_16U && mDepth->type() != SL_COLOR_R_FLOAT && mDepth->type() != SL_COLOR_R_DOUBLE)
    {
        return -12;
    }

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Framebuffer::terminate() noexcept
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
void SL_Framebuffer::put_pixel(
    uint64_t targetId,
    uint16_t x,
    uint16_t y,
    const math::vec4& rgba) noexcept
{
    SL_Texture* const      pTexture = mColors[targetId];
    const SL_ColorDataType type     = pTexture->type();

    switch (type)
    {
        case SL_COLOR_R_8U:        assign_pixel<SL_ColorR8>(x, y, rgba, pTexture); break;
        case SL_COLOR_RG_8U:       assign_pixel<SL_ColorRG8>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGB_8U:      assign_pixel<SL_ColorRGB8>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGBA_8U:     assign_pixel<SL_ColorRGBA8>(x, y, rgba, pTexture); break;

        case SL_COLOR_R_16U:       assign_pixel<SL_ColorR16>(x, y, rgba, pTexture); break;
        case SL_COLOR_RG_16U:      assign_pixel<SL_ColorRG16>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGB_16U:     assign_pixel<SL_ColorRGB16>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGBA_16U:    assign_pixel<SL_ColorRGBA16>(x, y, rgba, pTexture); break;

        case SL_COLOR_R_32U:       assign_pixel<SL_ColorR32>(x, y, rgba, pTexture); break;
        case SL_COLOR_RG_32U:      assign_pixel<SL_ColorRG32>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGB_32U:     assign_pixel<SL_ColorRGB32>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGBA_32U:    assign_pixel<SL_ColorRGBA32>(x, y, rgba, pTexture); break;

        case SL_COLOR_R_64U:       assign_pixel<SL_ColorR64>(x, y, rgba, pTexture); break;
        case SL_COLOR_RG_64U:      assign_pixel<SL_ColorRG64>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGB_64U:     assign_pixel<SL_ColorRGB64>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGBA_64U:    assign_pixel<SL_ColorRGBA64>(x, y, rgba, pTexture); break;

        case SL_COLOR_R_FLOAT:     assign_pixel<SL_ColorRf>(x, y, rgba, pTexture); break;
        case SL_COLOR_RG_FLOAT:    assign_pixel<SL_ColorRGf>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGB_FLOAT:   assign_pixel<SL_ColorRGBf>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGBA_FLOAT:  assign_pixel<SL_ColorRGBAf>(x, y, rgba, pTexture); break;

        case SL_COLOR_R_DOUBLE:    assign_pixel<SL_ColorRd>(x, y, rgba, pTexture); break;
        case SL_COLOR_RG_DOUBLE:   assign_pixel<SL_ColorRGd>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGB_DOUBLE:  assign_pixel<SL_ColorRGBd>(x, y, rgba, pTexture); break;
        case SL_COLOR_RGBA_DOUBLE: assign_pixel<SL_ColorRGBAd>(x, y, rgba, pTexture); break;

        case SL_ColorDataType::SL_COLOR_RGB_565:   assign_pixel<SL_ColorRGB565>(x, y, rgba, pTexture); break;
        case SL_ColorDataType::SL_COLOR_RGBA_5551: assign_pixel<SL_ColorRGB5551>(x, y, rgba, pTexture); break;
        case SL_ColorDataType::SL_COLOR_RGBA_4444: assign_pixel<SL_ColorRGB4444>(x, y, rgba, pTexture); break;

        default:
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Place a single pixel onto a texture
-------------------------------------*/
void SL_Framebuffer::put_pixel(SL_FboOutputMask outMask, SL_BlendMode blendMode, const SL_FragmentParam& fragParam) noexcept
{
    switch (outMask)
    {
        case SL_FBO_OUTPUT_ATTACHMENT_0_1_2_3: this->put_pixel(3, fragParam.coord.x, fragParam.coord.y, fragParam.pOutputs[3]);
        case SL_FBO_OUTPUT_ATTACHMENT_0_1_2:   this->put_pixel(2, fragParam.coord.x, fragParam.coord.y, fragParam.pOutputs[2]);
        case SL_FBO_OUTPUT_ATTACHMENT_0_1:     this->put_pixel(1, fragParam.coord.x, fragParam.coord.y, fragParam.pOutputs[1]);
        case SL_FBO_OUTPUT_ATTACHMENT_0:       this->put_pixel(0, fragParam.coord.x, fragParam.coord.y, fragParam.pOutputs[0]);
            break;

        case SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1_2_3: this->put_alpha_pixel(3, fragParam.coord.x, fragParam.coord.y, fragParam.pOutputs[3], blendMode);
        case SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1_2:   this->put_alpha_pixel(2, fragParam.coord.x, fragParam.coord.y, fragParam.pOutputs[2], blendMode);
        case SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0_1:     this->put_alpha_pixel(1, fragParam.coord.x, fragParam.coord.y, fragParam.pOutputs[1], blendMode);
        case SL_FBO_OUTPUT_ALPHA_ATTACHMENT_0:       this->put_alpha_pixel(0, fragParam.coord.x, fragParam.coord.y, fragParam.pOutputs[0], blendMode);

        default:
            break;
    }
}



/*-------------------------------------
 * Place a pixel onto a texture with alpha blending
-------------------------------------*/
void SL_Framebuffer::put_alpha_pixel(
    uint64_t targetId,
    uint16_t x,
    uint16_t y,
    const math::vec4& colors,
    const SL_BlendMode blendMode) noexcept
{
    SL_Texture* const      pTexture = mColors[targetId];
    const SL_ColorDataType type     = pTexture->type();

    switch (type)
    {
        case SL_COLOR_R_8U:        assign_alpha_pixel<SL_ColorR8>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RG_8U:       assign_alpha_pixel<SL_ColorRG8>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGB_8U:      assign_alpha_pixel<SL_ColorRGB8>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGBA_8U:     assign_alpha_pixel<SL_ColorRGBA8>(x, y, colors, pTexture, blendMode); break;

        case SL_COLOR_R_16U:       assign_alpha_pixel<SL_ColorR16>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RG_16U:      assign_alpha_pixel<SL_ColorRG16>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGB_16U:     assign_alpha_pixel<SL_ColorRGB16>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGBA_16U:    assign_alpha_pixel<SL_ColorRGBA16>(x, y, colors, pTexture, blendMode); break;

        case SL_COLOR_R_32U:       assign_alpha_pixel<SL_ColorR32>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RG_32U:      assign_alpha_pixel<SL_ColorRG32>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGB_32U:     assign_alpha_pixel<SL_ColorRGB32>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGBA_32U:    assign_alpha_pixel<SL_ColorRGBA32>(x, y, colors, pTexture, blendMode); break;

        case SL_COLOR_R_64U:       assign_alpha_pixel<SL_ColorR64>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RG_64U:      assign_alpha_pixel<SL_ColorRG64>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGB_64U:     assign_alpha_pixel<SL_ColorRGB64>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGBA_64U:    assign_alpha_pixel<SL_ColorRGBA64>(x, y, colors, pTexture, blendMode); break;

        case SL_COLOR_R_FLOAT:     assign_alpha_pixel<SL_ColorRf>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RG_FLOAT:    assign_alpha_pixel<SL_ColorRGf>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGB_FLOAT:   assign_alpha_pixel<SL_ColorRGBf>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGBA_FLOAT:  assign_alpha_pixel<SL_ColorRGBAf>(x, y, colors, pTexture, blendMode); break;

        case SL_COLOR_R_DOUBLE:    assign_alpha_pixel<SL_ColorRd>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RG_DOUBLE:   assign_alpha_pixel<SL_ColorRGd>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGB_DOUBLE:  assign_alpha_pixel<SL_ColorRGBd>(x, y, colors, pTexture, blendMode); break;
        case SL_COLOR_RGBA_DOUBLE: assign_alpha_pixel<SL_ColorRGBAd>(x, y, colors, pTexture, blendMode); break;

        case SL_ColorDataType::SL_COLOR_RGB_565:   assign_alpha_pixel<SL_ColorRGB565>(x, y, colors, pTexture, blendMode); break;
        case SL_ColorDataType::SL_COLOR_RGBA_5551: assign_alpha_pixel<SL_ColorRGB5551>(x, y, colors, pTexture, blendMode); break;
        case SL_ColorDataType::SL_COLOR_RGBA_4444: assign_alpha_pixel<SL_ColorRGB4444>(x, y, colors, pTexture, blendMode); break;

        default:
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 *
-------------------------------------*/
uint16_t SL_Framebuffer::width() const noexcept
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
uint16_t SL_Framebuffer::height() const noexcept
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
uint16_t SL_Framebuffer::depth() const noexcept
{
    if (mColors)
    {
        return mColors[0]->depth();
    }

    return 0;
}
