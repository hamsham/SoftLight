
#ifndef SR_BLIT_PROCESOR_HPP
#define SR_BLIT_PROCESOR_HPP

#include <cstdint>

#include <iostream>

// MSVC can't handle the external templates used in this source file
#include "lightsky/setup/Compiler.h" // LS_COMPILER_MSC

#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_WindowBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SR_Texture;
class SR_WindowBuffer;



/*-----------------------------------------------------------------------------
 * Helper functions and namespaces
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Recolor to R
-------------------------------------*/
template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRType<outColor_type>)>
struct SR_Blit_R_to_R
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                   offset  = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRType<inColor_type> inColor = pTexture->texel<SR_ColorRType<inColor_type>>(srcX, srcY);

        *reinterpret_cast<SR_ColorRType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRType<outColor_type>)>
struct SR_Blit_RG_to_R
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                    offset  = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGType<inColor_type> inColor = pTexture->texel<SR_ColorRGType<inColor_type>>(srcX, srcY);

        *reinterpret_cast<SR_ColorRType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRType<outColor_type>)>
struct SR_Blit_RGB_to_R
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset  = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGBType<inColor_type> inColor = pTexture->texel<SR_ColorRGBType<inColor_type>>(srcX, srcY);

        *reinterpret_cast<SR_ColorRType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRType<outColor_type>)>
struct SR_Blit_RGBA_to_R
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset  = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGBAType<inColor_type> inColor = pTexture->texel<SR_ColorRGBAType<inColor_type>>(srcX, srcY);

        *reinterpret_cast<SR_ColorRType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};



/*-------------------------------------
 * Recolor to RG
-------------------------------------*/
template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGType<outColor_type>)>
struct SR_Blit_R_to_RG
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                    offset   = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRType<inColor_type>  inColorR = pTexture->texel<SR_ColorRType<inColor_type>>(srcX, srcY);
        const SR_ColorRGType<inColor_type> inColor  = SR_ColorRGType<inColor_type>{inColorR[0], (inColor_type)0};

        *reinterpret_cast<SR_ColorRGType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGType<outColor_type>)>
struct SR_Blit_RG_to_RG
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                    offset  = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGType<inColor_type> inColor = pTexture->texel<SR_ColorRGType<inColor_type>>(srcX, srcY);

        *reinterpret_cast<SR_ColorRGType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGType<outColor_type>)>
struct SR_Blit_RGB_to_RG
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset     = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGBType<inColor_type> inColorRGB = pTexture->texel<SR_ColorRGBType<inColor_type>>(srcX, srcY);
        const SR_ColorRGType<inColor_type>  inColor    = ls::math::vec2_cast(inColorRGB);

        *reinterpret_cast<SR_ColorRGType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGType<outColor_type>)>
struct SR_Blit_RGBA_to_RG
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset      = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGBAType<inColor_type> inColorRGBA = pTexture->texel<SR_ColorRGBAType<inColor_type>>(srcX, srcY);
        const SR_ColorRGType<inColor_type>   inColor     = ls::math::vec2_cast(inColorRGBA);

        *reinterpret_cast<SR_ColorRGType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * Recolor to RGB
-------------------------------------*/
template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGBType<outColor_type>)>
struct SR_Blit_R_to_RGB
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset   = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRType<inColor_type>   inColorR = pTexture->texel<SR_ColorRType<inColor_type>>(srcX, srcY);
        const SR_ColorRGBType<inColor_type> inColor  = SR_ColorRGBType<inColor_type>{(inColor_type)0, (inColor_type)0, inColorR[0]};

        *reinterpret_cast<SR_ColorRGBType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGBType<outColor_type>)>
struct SR_Blit_RG_to_RGB
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset    = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGType<inColor_type>  inColorRG = pTexture->texel<SR_ColorRGType<inColor_type>>(srcX, srcY);
        const SR_ColorRGBType<inColor_type> inColor   = ls::math::vec3_cast(inColorRG, (inColor_type)0);

        *reinterpret_cast<SR_ColorRGBType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGBType<outColor_type>)>
struct SR_Blit_RGB_to_RGB
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                     offset  = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGBType<inColor_type> inColor = pTexture->texel<SR_ColorRGBType<inColor_type>>(srcX, srcY);

        *reinterpret_cast<SR_ColorRGBType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGBType<outColor_type>)>
struct SR_Blit_RGBA_to_RGB
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset      = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGBAType<inColor_type> inColorRGBA = pTexture->texel<SR_ColorRGBAType<inColor_type>>(srcX, srcY);
        const SR_ColorRGBType<inColor_type>  inColor     = ls::math::vec3_cast(inColorRGBA);

        *reinterpret_cast<SR_ColorRGBType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * Recolor to RGBA
-------------------------------------*/
template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGBAType<outColor_type>)>
struct SR_Blit_R_to_RGBA
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset   = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRType<inColor_type>    inColorR = pTexture->texel<SR_ColorRType<inColor_type>>(srcX, srcY);
        const SR_ColorRGBAType<inColor_type> inColor  = SR_ColorRGBAType<inColor_type>{(inColor_type)0, (inColor_type)0, inColorR[0], (inColor_type)1};

        *reinterpret_cast<SR_ColorRGBAType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGBAType<outColor_type>)>
struct SR_Blit_RG_to_RGBA
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset    = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGType<inColor_type>   inColorRG = pTexture->texel<SR_ColorRGType<inColor_type>>(srcX, srcY);
        const SR_ColorRGBAType<inColor_type> inColor   = ls::math::vec4_cast((inColor_type)0, inColorRG, (inColor_type)1);

        *reinterpret_cast<SR_ColorRGBAType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGBAType<outColor_type>)>
struct SR_Blit_RGB_to_RGBA
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset     = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGBType<inColor_type>  inColorRGB = pTexture->texel<SR_ColorRGBType<inColor_type>>(srcX, srcY);
        const SR_ColorRGBAType<inColor_type> inColor    = ls::math::vec4_cast(inColorRGB, (inColor_type)1);

        *reinterpret_cast<SR_ColorRGBAType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type, ptrdiff_t stride = sizeof(SR_ColorRGBAType<outColor_type>)>
struct SR_Blit_RGBA_to_RGBA
{
    inline LS_INLINE void operator()(
        const SR_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t numTotalOutPixels,
        uint_fast32_t outIndex) const noexcept
    {
        const ptrdiff_t                      offset = (numTotalOutPixels - outIndex) * stride;
        const SR_ColorRGBAType<inColor_type> inColor = pTexture->texel<SR_ColorRGBAType<inColor_type>>(srcX, srcY);

        *reinterpret_cast<SR_ColorRGBAType<outColor_type>*>(pOutBuf + offset) = color_cast<outColor_type, inColor_type>(inColor);
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
struct SR_BlitProcessor
{
    using sr_fixed_type = ls::math::ulong_lowp_t;

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
    const SR_Texture* mTexture;
    SR_Texture* mBackBuffer;

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
void SR_BlitProcessor::blit_src_r() noexcept
{
    switch (mBackBuffer->type())
    {
        case SR_COLOR_R_8U:         blit_nearest<SR_Blit_R_to_R<inColor_type, uint8_t>>();     break;
        case SR_COLOR_R_16U:        blit_nearest<SR_Blit_R_to_R<inColor_type, uint16_t>>();    break;
        case SR_COLOR_R_32U:        blit_nearest<SR_Blit_R_to_R<inColor_type, uint32_t>>();    break;
        case SR_COLOR_R_64U:        blit_nearest<SR_Blit_R_to_R<inColor_type, uint64_t>>();    break;
        case SR_COLOR_R_FLOAT:      blit_nearest<SR_Blit_R_to_R<inColor_type, float>>();       break;
        case SR_COLOR_R_DOUBLE:     blit_nearest<SR_Blit_R_to_R<inColor_type, double>>();      break;

        case SR_COLOR_RG_8U:        blit_nearest<SR_Blit_R_to_RG<inColor_type, uint8_t>>();    break;
        case SR_COLOR_RG_16U:       blit_nearest<SR_Blit_R_to_RG<inColor_type, uint16_t>>();   break;
        case SR_COLOR_RG_32U:       blit_nearest<SR_Blit_R_to_RG<inColor_type, uint32_t>>();   break;
        case SR_COLOR_RG_64U:       blit_nearest<SR_Blit_R_to_RG<inColor_type, uint64_t>>();   break;
        case SR_COLOR_RG_FLOAT:     blit_nearest<SR_Blit_R_to_RG<inColor_type, float>>();      break;
        case SR_COLOR_RG_DOUBLE:    blit_nearest<SR_Blit_R_to_RG<inColor_type, double>>();     break;

        case SR_COLOR_RGB_8U:       blit_nearest<SR_Blit_R_to_RGB<inColor_type, uint8_t>>();   break;
        case SR_COLOR_RGB_16U:      blit_nearest<SR_Blit_R_to_RGB<inColor_type, uint16_t>>();  break;
        case SR_COLOR_RGB_32U:      blit_nearest<SR_Blit_R_to_RGB<inColor_type, uint32_t>>();  break;
        case SR_COLOR_RGB_64U:      blit_nearest<SR_Blit_R_to_RGB<inColor_type, uint64_t>>();  break;
        case SR_COLOR_RGB_FLOAT:    blit_nearest<SR_Blit_R_to_RGB<inColor_type, float>>();     break;
        case SR_COLOR_RGB_DOUBLE:   blit_nearest<SR_Blit_R_to_RGB<inColor_type, double>>();    break;

        case SR_COLOR_RGBA_8U:      blit_nearest<SR_Blit_R_to_RGBA<inColor_type, uint8_t>>();  break;
        case SR_COLOR_RGBA_16U:     blit_nearest<SR_Blit_R_to_RGBA<inColor_type, uint16_t>>(); break;
        case SR_COLOR_RGBA_32U:     blit_nearest<SR_Blit_R_to_RGBA<inColor_type, uint32_t>>(); break;
        case SR_COLOR_RGBA_64U:     blit_nearest<SR_Blit_R_to_RGBA<inColor_type, uint64_t>>(); break;
        case SR_COLOR_RGBA_FLOAT:   blit_nearest<SR_Blit_R_to_RGBA<inColor_type, float>>();    break;
        case SR_COLOR_RGBA_DOUBLE:  blit_nearest<SR_Blit_R_to_RGBA<inColor_type, double>>();   break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (R & G Channels)
-------------------------------------*/
template<typename inColor_type>
void SR_BlitProcessor::blit_src_rg() noexcept
{
    switch (mBackBuffer->type())
    {
        case SR_COLOR_R_8U:         blit_nearest<SR_Blit_RG_to_R<inColor_type, uint8_t>>();     break;
        case SR_COLOR_R_16U:        blit_nearest<SR_Blit_RG_to_R<inColor_type, uint16_t>>();    break;
        case SR_COLOR_R_32U:        blit_nearest<SR_Blit_RG_to_R<inColor_type, uint32_t>>();    break;
        case SR_COLOR_R_64U:        blit_nearest<SR_Blit_RG_to_R<inColor_type, uint64_t>>();    break;
        case SR_COLOR_R_FLOAT:      blit_nearest<SR_Blit_RG_to_R<inColor_type, float>>();       break;
        case SR_COLOR_R_DOUBLE:     blit_nearest<SR_Blit_RG_to_R<inColor_type, double>>();      break;

        case SR_COLOR_RG_8U:        blit_nearest<SR_Blit_RG_to_RG<inColor_type, uint8_t>>();    break;
        case SR_COLOR_RG_16U:       blit_nearest<SR_Blit_RG_to_RG<inColor_type, uint16_t>>();   break;
        case SR_COLOR_RG_32U:       blit_nearest<SR_Blit_RG_to_RG<inColor_type, uint32_t>>();   break;
        case SR_COLOR_RG_64U:       blit_nearest<SR_Blit_RG_to_RG<inColor_type, uint64_t>>();   break;
        case SR_COLOR_RG_FLOAT:     blit_nearest<SR_Blit_RG_to_RG<inColor_type, float>>();      break;
        case SR_COLOR_RG_DOUBLE:    blit_nearest<SR_Blit_RG_to_RG<inColor_type, double>>();     break;

        case SR_COLOR_RGB_8U:       blit_nearest<SR_Blit_RG_to_RGB<inColor_type, uint8_t>>();   break;
        case SR_COLOR_RGB_16U:      blit_nearest<SR_Blit_RG_to_RGB<inColor_type, uint16_t>>();  break;
        case SR_COLOR_RGB_32U:      blit_nearest<SR_Blit_RG_to_RGB<inColor_type, uint32_t>>();  break;
        case SR_COLOR_RGB_64U:      blit_nearest<SR_Blit_RG_to_RGB<inColor_type, uint64_t>>();  break;
        case SR_COLOR_RGB_FLOAT:    blit_nearest<SR_Blit_RG_to_RGB<inColor_type, float>>();     break;
        case SR_COLOR_RGB_DOUBLE:   blit_nearest<SR_Blit_RG_to_RGB<inColor_type, double>>();    break;

        case SR_COLOR_RGBA_8U:      blit_nearest<SR_Blit_RG_to_RGBA<inColor_type, uint8_t>>();  break;
        case SR_COLOR_RGBA_16U:     blit_nearest<SR_Blit_RG_to_RGBA<inColor_type, uint16_t>>(); break;
        case SR_COLOR_RGBA_32U:     blit_nearest<SR_Blit_RG_to_RGBA<inColor_type, uint32_t>>(); break;
        case SR_COLOR_RGBA_64U:     blit_nearest<SR_Blit_RG_to_RGBA<inColor_type, uint64_t>>(); break;
        case SR_COLOR_RGBA_FLOAT:   blit_nearest<SR_Blit_RG_to_RGBA<inColor_type, float>>();    break;
        case SR_COLOR_RGBA_DOUBLE:  blit_nearest<SR_Blit_RG_to_RGBA<inColor_type, double>>();   break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGB)
-------------------------------------*/
template<typename inColor_type>
void SR_BlitProcessor::blit_src_rgb() noexcept
{
    switch (mBackBuffer->type())
    {
        case SR_COLOR_R_8U:         blit_nearest<SR_Blit_RGB_to_R<inColor_type, uint8_t>>();     break;
        case SR_COLOR_R_16U:        blit_nearest<SR_Blit_RGB_to_R<inColor_type, uint16_t>>();    break;
        case SR_COLOR_R_32U:        blit_nearest<SR_Blit_RGB_to_R<inColor_type, uint32_t>>();    break;
        case SR_COLOR_R_64U:        blit_nearest<SR_Blit_RGB_to_R<inColor_type, uint64_t>>();    break;
        case SR_COLOR_R_FLOAT:      blit_nearest<SR_Blit_RGB_to_R<inColor_type, float>>();       break;
        case SR_COLOR_R_DOUBLE:     blit_nearest<SR_Blit_RGB_to_R<inColor_type, double>>();      break;

        case SR_COLOR_RG_8U:        blit_nearest<SR_Blit_RGB_to_RG<inColor_type, uint8_t>>();    break;
        case SR_COLOR_RG_16U:       blit_nearest<SR_Blit_RGB_to_RG<inColor_type, uint16_t>>();   break;
        case SR_COLOR_RG_32U:       blit_nearest<SR_Blit_RGB_to_RG<inColor_type, uint32_t>>();   break;
        case SR_COLOR_RG_64U:       blit_nearest<SR_Blit_RGB_to_RG<inColor_type, uint64_t>>();   break;
        case SR_COLOR_RG_FLOAT:     blit_nearest<SR_Blit_RGB_to_RG<inColor_type, float>>();      break;
        case SR_COLOR_RG_DOUBLE:    blit_nearest<SR_Blit_RGB_to_RG<inColor_type, double>>();     break;

        case SR_COLOR_RGB_8U:       blit_nearest<SR_Blit_RGB_to_RGB<inColor_type, uint8_t>>();   break;
        case SR_COLOR_RGB_16U:      blit_nearest<SR_Blit_RGB_to_RGB<inColor_type, uint16_t>>();  break;
        case SR_COLOR_RGB_32U:      blit_nearest<SR_Blit_RGB_to_RGB<inColor_type, uint32_t>>();  break;
        case SR_COLOR_RGB_64U:      blit_nearest<SR_Blit_RGB_to_RGB<inColor_type, uint64_t>>();  break;
        case SR_COLOR_RGB_FLOAT:    blit_nearest<SR_Blit_RGB_to_RGB<inColor_type, float>>();     break;
        case SR_COLOR_RGB_DOUBLE:   blit_nearest<SR_Blit_RGB_to_RGB<inColor_type, double>>();    break;

        case SR_COLOR_RGBA_8U:      blit_nearest<SR_Blit_RGB_to_RGBA<inColor_type, uint8_t>>();  break;
        case SR_COLOR_RGBA_16U:     blit_nearest<SR_Blit_RGB_to_RGBA<inColor_type, uint16_t>>(); break;
        case SR_COLOR_RGBA_32U:     blit_nearest<SR_Blit_RGB_to_RGBA<inColor_type, uint32_t>>(); break;
        case SR_COLOR_RGBA_64U:     blit_nearest<SR_Blit_RGB_to_RGBA<inColor_type, uint64_t>>(); break;
        case SR_COLOR_RGBA_FLOAT:   blit_nearest<SR_Blit_RGB_to_RGBA<inColor_type, float>>();    break;
        case SR_COLOR_RGBA_DOUBLE:  blit_nearest<SR_Blit_RGB_to_RGBA<inColor_type, double>>();   break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<typename inColor_type>
void SR_BlitProcessor::blit_src_rgba() noexcept
{
    switch (mBackBuffer->type())
    {
        case SR_COLOR_R_8U:         blit_nearest<SR_Blit_RGBA_to_R<inColor_type, uint8_t>>();     break;
        case SR_COLOR_R_16U:        blit_nearest<SR_Blit_RGBA_to_R<inColor_type, uint16_t>>();    break;
        case SR_COLOR_R_32U:        blit_nearest<SR_Blit_RGBA_to_R<inColor_type, uint32_t>>();    break;
        case SR_COLOR_R_64U:        blit_nearest<SR_Blit_RGBA_to_R<inColor_type, uint64_t>>();    break;
        case SR_COLOR_R_FLOAT:      blit_nearest<SR_Blit_RGBA_to_R<inColor_type, float>>();       break;
        case SR_COLOR_R_DOUBLE:     blit_nearest<SR_Blit_RGBA_to_R<inColor_type, double>>();      break;

        case SR_COLOR_RG_8U:        blit_nearest<SR_Blit_RGBA_to_RG<inColor_type, uint8_t>>();    break;
        case SR_COLOR_RG_16U:       blit_nearest<SR_Blit_RGBA_to_RG<inColor_type, uint16_t>>();   break;
        case SR_COLOR_RG_32U:       blit_nearest<SR_Blit_RGBA_to_RG<inColor_type, uint32_t>>();   break;
        case SR_COLOR_RG_64U:       blit_nearest<SR_Blit_RGBA_to_RG<inColor_type, uint64_t>>();   break;
        case SR_COLOR_RG_FLOAT:     blit_nearest<SR_Blit_RGBA_to_RG<inColor_type, float>>();      break;
        case SR_COLOR_RG_DOUBLE:    blit_nearest<SR_Blit_RGBA_to_RG<inColor_type, double>>();     break;

        case SR_COLOR_RGB_8U:       blit_nearest<SR_Blit_RGBA_to_RGB<inColor_type, uint8_t>>();   break;
        case SR_COLOR_RGB_16U:      blit_nearest<SR_Blit_RGBA_to_RGB<inColor_type, uint16_t>>();  break;
        case SR_COLOR_RGB_32U:      blit_nearest<SR_Blit_RGBA_to_RGB<inColor_type, uint32_t>>();  break;
        case SR_COLOR_RGB_64U:      blit_nearest<SR_Blit_RGBA_to_RGB<inColor_type, uint64_t>>();  break;
        case SR_COLOR_RGB_FLOAT:    blit_nearest<SR_Blit_RGBA_to_RGB<inColor_type, float>>();     break;
        case SR_COLOR_RGB_DOUBLE:   blit_nearest<SR_Blit_RGBA_to_RGB<inColor_type, double>>();    break;

        case SR_COLOR_RGBA_8U:      blit_nearest<SR_Blit_RGBA_to_RGBA<inColor_type, uint8_t>>();  break;
        case SR_COLOR_RGBA_16U:     blit_nearest<SR_Blit_RGBA_to_RGBA<inColor_type, uint16_t>>(); break;
        case SR_COLOR_RGBA_32U:     blit_nearest<SR_Blit_RGBA_to_RGBA<inColor_type, uint32_t>>(); break;
        case SR_COLOR_RGBA_64U:     blit_nearest<SR_Blit_RGBA_to_RGBA<inColor_type, uint64_t>>(); break;
        case SR_COLOR_RGBA_FLOAT:   blit_nearest<SR_Blit_RGBA_to_RGBA<inColor_type, float>>();    break;
        case SR_COLOR_RGBA_DOUBLE:  blit_nearest<SR_Blit_RGBA_to_RGBA<inColor_type, double>>();   break;
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<class BlipOp>
void SR_BlitProcessor::blit_nearest() noexcept
{
    const BlipOp blitOp;
    unsigned char* const pOutBuf = reinterpret_cast<unsigned char* const>(mBackBuffer->data());

    const uint_fast32_t inW  = (uint_fast32_t)srcX1 - (uint_fast32_t)srcX0;
    const uint_fast32_t inH  = (uint_fast32_t)srcY1 - (uint_fast32_t)srcY0;
    const uint_fast32_t outW = (uint_fast32_t)dstX1 - (uint_fast32_t)dstX0;
    const uint_fast32_t outH = (uint_fast32_t)dstY1 - (uint_fast32_t)dstY0;

    const uint_fast32_t totalOutW = mBackBuffer->width();
    const uint_fast32_t totalOutH = mBackBuffer->height();

    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast32_t x0        = ls::math::max(0u, dstX0);
    const uint_fast32_t x1        = ls::math::min(totalOutW, x0 + outW);
    const uint_fast32_t dstH      = outH / mNumThreads;
    const uint_fast32_t y0        = ls::math::max(0u, dstY0 + (mThreadId * dstH));
    const uint_fast32_t y1        = ls::math::min(totalOutH, y0 + dstH);

    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(totalOutW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(totalOutH);

    const uint_fast32_t numPixels = (totalOutW*totalOutH) - 1;

    for (uint_fast32_t y = y0; y < y1; ++y)
    {
        const sr_fixed_type yf   = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        const uint_fast32_t srcY = srcY0 + ls::math::integer_cast<uint_fast32_t>(yf);

        #if 0
        for (uint_fast32_t x = x0; x < x1; ++x)
        {
            const sr_fixed_type  xf       = ls::math::fixed_cast<sr_fixed_type>(x) * foutW;
            const uint_fast32_t  srcX     = srcX0 + ls::math::integer_cast<uint_fast32_t>(xf);
            const uint_fast32_t  outIndex = x + totalOutW * y;

            blitOp(mTexture, srcX, srcY, pOutBuf, numPixels, outIndex);
        }
        #else
        uint_fast32_t x;
        for (x = x0; x+4 < x1; x += 4)
        {
            sr_fixed_type xf[4] = {
                ls::math::fixed_cast<sr_fixed_type>(x+0),
                ls::math::fixed_cast<sr_fixed_type>(x+1),
                ls::math::fixed_cast<sr_fixed_type>(x+2),
                ls::math::fixed_cast<sr_fixed_type>(x+3)
            };

            xf[0] *= foutW;
            xf[1] *= foutW;
            xf[2] *= foutW;
            xf[3] *= foutW;

            const uint_fast32_t srcX[4] = {
                srcX0 + ls::math::integer_cast<uint_fast32_t>(xf[0]),
                srcX0 + ls::math::integer_cast<uint_fast32_t>(xf[1]),
                srcX0 + ls::math::integer_cast<uint_fast32_t>(xf[2]),
                srcX0 + ls::math::integer_cast<uint_fast32_t>(xf[3])
            };

            const uint_fast32_t outIndex[4] = {
                (x+0) + totalOutW * y,
                (x+1) + totalOutW * y,
                (x+2) + totalOutW * y,
                (x+3) + totalOutW * y
            };

            blitOp(mTexture, srcX[0], srcY, pOutBuf, numPixels, outIndex[0]);
            blitOp(mTexture, srcX[1], srcY, pOutBuf, numPixels, outIndex[1]);
            blitOp(mTexture, srcX[2], srcY, pOutBuf, numPixels, outIndex[2]);
            blitOp(mTexture, srcX[3], srcY, pOutBuf, numPixels, outIndex[3]);
        }

        for (; x < x1; ++x)
        {
            const sr_fixed_type  xf       = ls::math::fixed_cast<sr_fixed_type>(x) * foutW;
            const uint_fast32_t  srcX     = srcX0 + ls::math::integer_cast<uint_fast32_t>(xf);
            const uint_fast32_t  outIndex = x + totalOutW * y;

            blitOp(mTexture, srcX, srcY, pOutBuf, numPixels, outIndex);
        }
        #endif
    }
}



#endif /* SR_BLIT_PROCESOR_HPP */
