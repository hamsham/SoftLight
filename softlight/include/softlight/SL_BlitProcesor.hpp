
#ifndef SL_BLIT_PROCESSOR_HPP
#define SL_BLIT_PROCESSOR_HPP

#include <cstdint>

#include "softlight/SL_Color.hpp"
#include "softlight/SL_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Helper functions and namespaces
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Recolor to R
-------------------------------------*/
template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRType<outColor_type>)>
struct SL_Blit_R_to_R
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                   offset  = outIndex * stride;
        const SL_ColorRType<inColor_type> inColor = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);

        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRType<outColor_type>)>
struct SL_Blit_RG_to_R
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                    offset  = outIndex * stride;
        const SL_ColorRGType<inColor_type> inColor = pTexture->texel<SL_ColorRGType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);

        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRType<outColor_type>)>
struct SL_Blit_RGB_to_R
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset  = outIndex * stride;
        const SL_ColorRGBType<inColor_type> inColor = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);

        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRType<outColor_type>)>
struct SL_Blit_RGBA_to_R
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset  = outIndex * stride;
        const SL_ColorRGBAType<inColor_type> inColor = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);

        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};



/*-------------------------------------
 * Recolor to RG
-------------------------------------*/
template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGType<outColor_type>)>
struct SL_Blit_R_to_RG
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                    offset   = outIndex * stride;
        const SL_ColorRType<inColor_type>  inColorR = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGType<inColor_type> inColor  = SL_ColorRGType<inColor_type>{inColorR[0], (inColor_type)0};

        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGType<outColor_type>)>
struct SL_Blit_RG_to_RG
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                    offset  = outIndex * stride;
        const SL_ColorRGType<inColor_type> inColor = pTexture->texel<SL_ColorRGType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);

        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGType<outColor_type>)>
struct SL_Blit_RGB_to_RG
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset     = outIndex * stride;
        const SL_ColorRGBType<inColor_type> inColorRGB = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGType<inColor_type>  inColor    = ls::math::vec2_cast(inColorRGB);

        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGType<outColor_type>)>
struct SL_Blit_RGBA_to_RG
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset      = outIndex * stride;
        const SL_ColorRGBAType<inColor_type> inColorRGBA = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGType<inColor_type>   inColor     = ls::math::vec2_cast(inColorRGBA);

        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * Recolor to RGB
-------------------------------------*/
template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGBType<outColor_type>)>
struct SL_Blit_R_to_RGB
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset   = outIndex * stride;
        const SL_ColorRType<inColor_type>   inColorR = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBType<inColor_type> inColor  = SL_ColorRGBType<inColor_type>{(inColor_type)0, (inColor_type)0, inColorR[0]};

        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGBType<outColor_type>)>
struct SL_Blit_RG_to_RGB
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset    = outIndex * stride;
        const SL_ColorRGType<inColor_type>  inColorRG = pTexture->texel<SL_ColorRGType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBType<inColor_type> inColor   = ls::math::vec3_cast(inColorRG, (inColor_type)0);

        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGBType<outColor_type>)>
struct SL_Blit_RGB_to_RGB
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset  = outIndex * stride;
        const SL_ColorRGBType<inColor_type> inColor = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);

        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGBType<outColor_type>)>
struct SL_Blit_RGBA_to_RGB
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset      = outIndex * stride;
        const SL_ColorRGBAType<inColor_type> inColorRGBA = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBType<inColor_type>  inColor     = ls::math::vec3_cast(inColorRGBA);

        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * Recolor to RGBA
-------------------------------------*/
template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGBAType<outColor_type>)>
struct SL_Blit_R_to_RGBA
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset   = outIndex * stride;
        const SL_ColorRType<inColor_type>    inColorR = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<inColor_type> inColor  = SL_ColorRGBAType<inColor_type>{(inColor_type)0, (inColor_type)0, inColorR[0], (inColor_type)1};

        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGBAType<outColor_type>)>
struct SL_Blit_RG_to_RGBA
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset    = outIndex * stride;
        const SL_ColorRGType<inColor_type>   inColorRG = pTexture->texel<SL_ColorRGType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<inColor_type> inColor   = ls::math::vec4_cast((inColor_type)0, inColorRG, (inColor_type)1);

        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGBAType<outColor_type>)>
struct SL_Blit_RGB_to_RGBA
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset     = outIndex * stride;
        const SL_ColorRGBType<inColor_type>  inColorRGB = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<inColor_type> inColor    = ls::math::vec4_cast(inColorRGB, (inColor_type)1);

        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SL_ColorRGBAType<outColor_type>)>
struct SL_Blit_RGBA_to_RGBA
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset = outIndex * stride;
        const SL_ColorRGBAType<inColor_type> inColor = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);

        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};



// I spent enough time on blitting... Here, I'm only optimizing for the most
// common blit operations, from RGBAf & RGBA8 to RGBA8
template<>
struct SL_Blit_RGBA_to_RGBA<uint8_t, uint8_t, sizeof(SL_ColorRGBAType<uint8_t>)>
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t offset = outIndex * sizeof(SL_ColorRGBAType<uint8_t>);
        const int32_t   inColor = pTexture->texel<int32_t>((uint16_t)srcX, (uint16_t)srcY);

        //_mm_stream_s32(reinterpret_cast<int32_t*>(pOutBuf + offset), inColor);
        *reinterpret_cast<int32_t*>(pOutBuf + offset) = inColor;
    }
};



template<>
struct SL_Blit_RGBA_to_RGBA<float, uint8_t, sizeof(SL_ColorRGBAType<uint8_t>)>
{
    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                 offset = outIndex * sizeof(SL_ColorRGBAType<uint8_t>);
        const SL_ColorRGBAType<float>   inColor = pTexture->texel<SL_ColorRGBAType<float>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<uint8_t> in = color_cast<uint8_t, float>(inColor);

        //_mm_stream_si32(reinterpret_cast<int32_t*>(pOutBuf + offset), reinterpret_cast<const int32_t&>(in));
        *reinterpret_cast<int32_t*>(pOutBuf + offset) = reinterpret_cast<const int32_t&>(in);
    }
};



/**----------------------------------------------------------------------------
 * @brief The Blit Processor helps to perform texture blitting to the native
 * window backbuffer on another thread.
 *
 * Much of the blitting routines are templated to support conversion between
 * possible texture types and the backbuffer (which is an 8-bit RGBA buffer).
 *
 * Texture blitting uses nearest-neighbor filtering to increase or decrease the
 * resolution and fit the backbuffer. Fixed-point calculation is used to avoid
 * precision errors and increase ALU throughput. Benchmarks on x86 and ARM has
 * shown that floating-point logic performs worse in this area.
-----------------------------------------------------------------------------*/
struct SL_BlitProcessor
{
    using sl_fixed_type = ls::math::ulong_lowp_t;

    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 64-bits
    uint16_t srcX0;
    uint16_t srcY0;
    uint16_t srcX1;
    uint16_t srcY1;

    // 64-bits
    uint16_t dstX0;
    uint16_t dstY0;
    uint16_t dstX1;
    uint16_t dstY1;

    // 64-128 bits
    const SL_Texture* mTexture;
    SL_Texture* mBackBuffer;

    // 224-288 bits total, 28-36 bytes

    // Blit a single R channel
    template<typename inColor_type>
    void blit_src_r() noexcept;

    // Blit a texture with only RG color channels
    template<typename inColor_type>
    void blit_src_rg() noexcept;

    // Blit an RGB texture
    template<typename inColor_type>
    void blit_src_rgb() noexcept;

    // Blit all 4 color components
    template<typename inColor_type>
    void blit_src_rgba() noexcept;

    // Blit all 4 color components
    template<class BlitOp>
    void blit_nearest() noexcept;

    void execute() noexcept;
};




/*-------------------------------------
 * Nearest-neighbor filtering (R Channel)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitProcessor::blit_src_r() noexcept
{
    switch (mBackBuffer->type())
    {
        case SL_COLOR_R_8U:         blit_nearest<SL_Blit_R_to_R<inColor_type, uint8_t>>();     break;
        case SL_COLOR_R_16U:        blit_nearest<SL_Blit_R_to_R<inColor_type, uint16_t>>();    break;
        case SL_COLOR_R_32U:        blit_nearest<SL_Blit_R_to_R<inColor_type, uint32_t>>();    break;
        case SL_COLOR_R_64U:        blit_nearest<SL_Blit_R_to_R<inColor_type, uint64_t>>();    break;
        case SL_COLOR_R_FLOAT:      blit_nearest<SL_Blit_R_to_R<inColor_type, float>>();       break;
        case SL_COLOR_R_DOUBLE:     blit_nearest<SL_Blit_R_to_R<inColor_type, double>>();      break;

        case SL_COLOR_RG_8U:        blit_nearest<SL_Blit_R_to_RG<inColor_type, uint8_t>>();    break;
        case SL_COLOR_RG_16U:       blit_nearest<SL_Blit_R_to_RG<inColor_type, uint16_t>>();   break;
        case SL_COLOR_RG_32U:       blit_nearest<SL_Blit_R_to_RG<inColor_type, uint32_t>>();   break;
        case SL_COLOR_RG_64U:       blit_nearest<SL_Blit_R_to_RG<inColor_type, uint64_t>>();   break;
        case SL_COLOR_RG_FLOAT:     blit_nearest<SL_Blit_R_to_RG<inColor_type, float>>();      break;
        case SL_COLOR_RG_DOUBLE:    blit_nearest<SL_Blit_R_to_RG<inColor_type, double>>();     break;

        case SL_COLOR_RGB_8U:       blit_nearest<SL_Blit_R_to_RGB<inColor_type, uint8_t>>();   break;
        case SL_COLOR_RGB_16U:      blit_nearest<SL_Blit_R_to_RGB<inColor_type, uint16_t>>();  break;
        case SL_COLOR_RGB_32U:      blit_nearest<SL_Blit_R_to_RGB<inColor_type, uint32_t>>();  break;
        case SL_COLOR_RGB_64U:      blit_nearest<SL_Blit_R_to_RGB<inColor_type, uint64_t>>();  break;
        case SL_COLOR_RGB_FLOAT:    blit_nearest<SL_Blit_R_to_RGB<inColor_type, float>>();     break;
        case SL_COLOR_RGB_DOUBLE:   blit_nearest<SL_Blit_R_to_RGB<inColor_type, double>>();    break;

        case SL_COLOR_RGBA_8U:      blit_nearest<SL_Blit_R_to_RGBA<inColor_type, uint8_t>>();  break;
        case SL_COLOR_RGBA_16U:     blit_nearest<SL_Blit_R_to_RGBA<inColor_type, uint16_t>>(); break;
        case SL_COLOR_RGBA_32U:     blit_nearest<SL_Blit_R_to_RGBA<inColor_type, uint32_t>>(); break;
        case SL_COLOR_RGBA_64U:     blit_nearest<SL_Blit_R_to_RGBA<inColor_type, uint64_t>>(); break;
        case SL_COLOR_RGBA_FLOAT:   blit_nearest<SL_Blit_R_to_RGBA<inColor_type, float>>();    break;
        case SL_COLOR_RGBA_DOUBLE:  blit_nearest<SL_Blit_R_to_RGBA<inColor_type, double>>();   break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (R & G Channels)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitProcessor::blit_src_rg() noexcept
{
    switch (mBackBuffer->type())
    {
        case SL_COLOR_R_8U:         blit_nearest<SL_Blit_RG_to_R<inColor_type, uint8_t>>();     break;
        case SL_COLOR_R_16U:        blit_nearest<SL_Blit_RG_to_R<inColor_type, uint16_t>>();    break;
        case SL_COLOR_R_32U:        blit_nearest<SL_Blit_RG_to_R<inColor_type, uint32_t>>();    break;
        case SL_COLOR_R_64U:        blit_nearest<SL_Blit_RG_to_R<inColor_type, uint64_t>>();    break;
        case SL_COLOR_R_FLOAT:      blit_nearest<SL_Blit_RG_to_R<inColor_type, float>>();       break;
        case SL_COLOR_R_DOUBLE:     blit_nearest<SL_Blit_RG_to_R<inColor_type, double>>();      break;

        case SL_COLOR_RG_8U:        blit_nearest<SL_Blit_RG_to_RG<inColor_type, uint8_t>>();    break;
        case SL_COLOR_RG_16U:       blit_nearest<SL_Blit_RG_to_RG<inColor_type, uint16_t>>();   break;
        case SL_COLOR_RG_32U:       blit_nearest<SL_Blit_RG_to_RG<inColor_type, uint32_t>>();   break;
        case SL_COLOR_RG_64U:       blit_nearest<SL_Blit_RG_to_RG<inColor_type, uint64_t>>();   break;
        case SL_COLOR_RG_FLOAT:     blit_nearest<SL_Blit_RG_to_RG<inColor_type, float>>();      break;
        case SL_COLOR_RG_DOUBLE:    blit_nearest<SL_Blit_RG_to_RG<inColor_type, double>>();     break;

        case SL_COLOR_RGB_8U:       blit_nearest<SL_Blit_RG_to_RGB<inColor_type, uint8_t>>();   break;
        case SL_COLOR_RGB_16U:      blit_nearest<SL_Blit_RG_to_RGB<inColor_type, uint16_t>>();  break;
        case SL_COLOR_RGB_32U:      blit_nearest<SL_Blit_RG_to_RGB<inColor_type, uint32_t>>();  break;
        case SL_COLOR_RGB_64U:      blit_nearest<SL_Blit_RG_to_RGB<inColor_type, uint64_t>>();  break;
        case SL_COLOR_RGB_FLOAT:    blit_nearest<SL_Blit_RG_to_RGB<inColor_type, float>>();     break;
        case SL_COLOR_RGB_DOUBLE:   blit_nearest<SL_Blit_RG_to_RGB<inColor_type, double>>();    break;

        case SL_COLOR_RGBA_8U:      blit_nearest<SL_Blit_RG_to_RGBA<inColor_type, uint8_t>>();  break;
        case SL_COLOR_RGBA_16U:     blit_nearest<SL_Blit_RG_to_RGBA<inColor_type, uint16_t>>(); break;
        case SL_COLOR_RGBA_32U:     blit_nearest<SL_Blit_RG_to_RGBA<inColor_type, uint32_t>>(); break;
        case SL_COLOR_RGBA_64U:     blit_nearest<SL_Blit_RG_to_RGBA<inColor_type, uint64_t>>(); break;
        case SL_COLOR_RGBA_FLOAT:   blit_nearest<SL_Blit_RG_to_RGBA<inColor_type, float>>();    break;
        case SL_COLOR_RGBA_DOUBLE:  blit_nearest<SL_Blit_RG_to_RGBA<inColor_type, double>>();   break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGB)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitProcessor::blit_src_rgb() noexcept
{
    switch (mBackBuffer->type())
    {
        case SL_COLOR_R_8U:         blit_nearest<SL_Blit_RGB_to_R<inColor_type, uint8_t>>();     break;
        case SL_COLOR_R_16U:        blit_nearest<SL_Blit_RGB_to_R<inColor_type, uint16_t>>();    break;
        case SL_COLOR_R_32U:        blit_nearest<SL_Blit_RGB_to_R<inColor_type, uint32_t>>();    break;
        case SL_COLOR_R_64U:        blit_nearest<SL_Blit_RGB_to_R<inColor_type, uint64_t>>();    break;
        case SL_COLOR_R_FLOAT:      blit_nearest<SL_Blit_RGB_to_R<inColor_type, float>>();       break;
        case SL_COLOR_R_DOUBLE:     blit_nearest<SL_Blit_RGB_to_R<inColor_type, double>>();      break;

        case SL_COLOR_RG_8U:        blit_nearest<SL_Blit_RGB_to_RG<inColor_type, uint8_t>>();    break;
        case SL_COLOR_RG_16U:       blit_nearest<SL_Blit_RGB_to_RG<inColor_type, uint16_t>>();   break;
        case SL_COLOR_RG_32U:       blit_nearest<SL_Blit_RGB_to_RG<inColor_type, uint32_t>>();   break;
        case SL_COLOR_RG_64U:       blit_nearest<SL_Blit_RGB_to_RG<inColor_type, uint64_t>>();   break;
        case SL_COLOR_RG_FLOAT:     blit_nearest<SL_Blit_RGB_to_RG<inColor_type, float>>();      break;
        case SL_COLOR_RG_DOUBLE:    blit_nearest<SL_Blit_RGB_to_RG<inColor_type, double>>();     break;

        case SL_COLOR_RGB_8U:       blit_nearest<SL_Blit_RGB_to_RGB<inColor_type, uint8_t>>();   break;
        case SL_COLOR_RGB_16U:      blit_nearest<SL_Blit_RGB_to_RGB<inColor_type, uint16_t>>();  break;
        case SL_COLOR_RGB_32U:      blit_nearest<SL_Blit_RGB_to_RGB<inColor_type, uint32_t>>();  break;
        case SL_COLOR_RGB_64U:      blit_nearest<SL_Blit_RGB_to_RGB<inColor_type, uint64_t>>();  break;
        case SL_COLOR_RGB_FLOAT:    blit_nearest<SL_Blit_RGB_to_RGB<inColor_type, float>>();     break;
        case SL_COLOR_RGB_DOUBLE:   blit_nearest<SL_Blit_RGB_to_RGB<inColor_type, double>>();    break;

        case SL_COLOR_RGBA_8U:      blit_nearest<SL_Blit_RGB_to_RGBA<inColor_type, uint8_t>>();  break;
        case SL_COLOR_RGBA_16U:     blit_nearest<SL_Blit_RGB_to_RGBA<inColor_type, uint16_t>>(); break;
        case SL_COLOR_RGBA_32U:     blit_nearest<SL_Blit_RGB_to_RGBA<inColor_type, uint32_t>>(); break;
        case SL_COLOR_RGBA_64U:     blit_nearest<SL_Blit_RGB_to_RGBA<inColor_type, uint64_t>>(); break;
        case SL_COLOR_RGBA_FLOAT:   blit_nearest<SL_Blit_RGB_to_RGBA<inColor_type, float>>();    break;
        case SL_COLOR_RGBA_DOUBLE:  blit_nearest<SL_Blit_RGB_to_RGBA<inColor_type, double>>();   break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitProcessor::blit_src_rgba() noexcept
{
    switch (mBackBuffer->type())
    {
        case SL_COLOR_R_8U:         blit_nearest<SL_Blit_RGBA_to_R<inColor_type, uint8_t>>();     break;
        case SL_COLOR_R_16U:        blit_nearest<SL_Blit_RGBA_to_R<inColor_type, uint16_t>>();    break;
        case SL_COLOR_R_32U:        blit_nearest<SL_Blit_RGBA_to_R<inColor_type, uint32_t>>();    break;
        case SL_COLOR_R_64U:        blit_nearest<SL_Blit_RGBA_to_R<inColor_type, uint64_t>>();    break;
        case SL_COLOR_R_FLOAT:      blit_nearest<SL_Blit_RGBA_to_R<inColor_type, float>>();       break;
        case SL_COLOR_R_DOUBLE:     blit_nearest<SL_Blit_RGBA_to_R<inColor_type, double>>();      break;

        case SL_COLOR_RG_8U:        blit_nearest<SL_Blit_RGBA_to_RG<inColor_type, uint8_t>>();    break;
        case SL_COLOR_RG_16U:       blit_nearest<SL_Blit_RGBA_to_RG<inColor_type, uint16_t>>();   break;
        case SL_COLOR_RG_32U:       blit_nearest<SL_Blit_RGBA_to_RG<inColor_type, uint32_t>>();   break;
        case SL_COLOR_RG_64U:       blit_nearest<SL_Blit_RGBA_to_RG<inColor_type, uint64_t>>();   break;
        case SL_COLOR_RG_FLOAT:     blit_nearest<SL_Blit_RGBA_to_RG<inColor_type, float>>();      break;
        case SL_COLOR_RG_DOUBLE:    blit_nearest<SL_Blit_RGBA_to_RG<inColor_type, double>>();     break;

        case SL_COLOR_RGB_8U:       blit_nearest<SL_Blit_RGBA_to_RGB<inColor_type, uint8_t>>();   break;
        case SL_COLOR_RGB_16U:      blit_nearest<SL_Blit_RGBA_to_RGB<inColor_type, uint16_t>>();  break;
        case SL_COLOR_RGB_32U:      blit_nearest<SL_Blit_RGBA_to_RGB<inColor_type, uint32_t>>();  break;
        case SL_COLOR_RGB_64U:      blit_nearest<SL_Blit_RGBA_to_RGB<inColor_type, uint64_t>>();  break;
        case SL_COLOR_RGB_FLOAT:    blit_nearest<SL_Blit_RGBA_to_RGB<inColor_type, float>>();     break;
        case SL_COLOR_RGB_DOUBLE:   blit_nearest<SL_Blit_RGBA_to_RGB<inColor_type, double>>();    break;

        case SL_COLOR_RGBA_8U:      blit_nearest<SL_Blit_RGBA_to_RGBA<inColor_type, uint8_t>>();  break;
        case SL_COLOR_RGBA_16U:     blit_nearest<SL_Blit_RGBA_to_RGBA<inColor_type, uint16_t>>(); break;
        case SL_COLOR_RGBA_32U:     blit_nearest<SL_Blit_RGBA_to_RGBA<inColor_type, uint32_t>>(); break;
        case SL_COLOR_RGBA_64U:     blit_nearest<SL_Blit_RGBA_to_RGBA<inColor_type, uint64_t>>(); break;
        case SL_COLOR_RGBA_FLOAT:   blit_nearest<SL_Blit_RGBA_to_RGBA<inColor_type, float>>();    break;
        case SL_COLOR_RGBA_DOUBLE:  blit_nearest<SL_Blit_RGBA_to_RGBA<inColor_type, double>>();   break;
    }
}



// MSVC crashes when generating all blit permutations.
#if !defined(LS_COMPILER_MSC)
    extern template void SL_BlitProcessor::blit_src_r<uint8_t>();
    extern template void SL_BlitProcessor::blit_src_r<uint16_t>();
    extern template void SL_BlitProcessor::blit_src_r<uint32_t>();
    extern template void SL_BlitProcessor::blit_src_r<uint64_t>();
    extern template void SL_BlitProcessor::blit_src_r<float>();
    extern template void SL_BlitProcessor::blit_src_r<double>();
    extern template void SL_BlitProcessor::blit_src_rg<uint8_t>();
    extern template void SL_BlitProcessor::blit_src_rg<uint16_t>();
    extern template void SL_BlitProcessor::blit_src_rg<uint32_t>();
    extern template void SL_BlitProcessor::blit_src_rg<uint64_t>();
    extern template void SL_BlitProcessor::blit_src_rg<float>();
    extern template void SL_BlitProcessor::blit_src_rg<double>();
    extern template void SL_BlitProcessor::blit_src_rgb<uint8_t>();
    extern template void SL_BlitProcessor::blit_src_rgb<uint16_t>();
    extern template void SL_BlitProcessor::blit_src_rgb<uint32_t>();
    extern template void SL_BlitProcessor::blit_src_rgb<uint64_t>();
    extern template void SL_BlitProcessor::blit_src_rgb<float>();
    extern template void SL_BlitProcessor::blit_src_rgb<double>();
    extern template void SL_BlitProcessor::blit_src_rgba<uint8_t>();
    extern template void SL_BlitProcessor::blit_src_rgba<uint16_t>();
    extern template void SL_BlitProcessor::blit_src_rgba<uint32_t>();
    extern template void SL_BlitProcessor::blit_src_rgba<uint64_t>();
    extern template void SL_BlitProcessor::blit_src_rgba<float>();
    extern template void SL_BlitProcessor::blit_src_rgba<double>();
#endif



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<class BlipOp>
void SL_BlitProcessor::blit_nearest() noexcept
{
    constexpr BlipOp blitOp;
    unsigned char* const pOutBuf = reinterpret_cast<unsigned char* const>(mBackBuffer->data());

    const uint_fast32_t inW  = (uint_fast32_t)srcX1 - (uint_fast32_t)srcX0;
    const uint_fast32_t inH  = (uint_fast32_t)srcY1 - (uint_fast32_t)srcY0;
    const uint_fast32_t outW = (uint_fast32_t)dstX1 - (uint_fast32_t)dstX0;

    const uint_fast32_t totalOutW = mBackBuffer->width();
    const uint_fast32_t totalOutH = mBackBuffer->height();

    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast32_t x0        = ls::math::max<uint_fast32_t>(0u, dstX0);
    const uint_fast32_t x1        = ls::math::min<uint_fast32_t>(totalOutW, x0 + outW);
    const uint_fast32_t y0        = dstY0+mThreadId;
    const uint_fast32_t y1        = dstY1;

    const sl_fixed_type finW      = ls::math::fixed_cast<sl_fixed_type>(inW);
    const sl_fixed_type finH      = ls::math::fixed_cast<sl_fixed_type>(inH);
    const sl_fixed_type foutW     = finW / ls::math::fixed_cast<sl_fixed_type>(totalOutW);
    const sl_fixed_type foutH     = finH / ls::math::fixed_cast<sl_fixed_type>(totalOutH);

    for (uint_fast32_t y = y0; y < y1; y += mNumThreads)
    {
        const sl_fixed_type yf   = ls::math::fixed_cast<sl_fixed_type>(y) * foutH;
        const uint_fast32_t srcY = srcY1 - (srcY0 + ls::math::integer_cast<uint_fast32_t>(yf)) - 1;

        for (uint_fast32_t x = x0; x < x1; ++x)
        {
            const sl_fixed_type  xf       = ls::math::fixed_cast<sl_fixed_type>(x) * foutW;
            const uint_fast32_t  srcX     = srcX0 + ls::math::integer_cast<uint_fast32_t>(xf);
            const uint_fast32_t  outIndex = x + totalOutW * y;

            blitOp(mTexture, srcX, srcY, pOutBuf, outIndex);
        }
    }
}



#endif /* SL_BLIT_PROCESSOR_HPP */
