
#include "lightsky/utils/Assertions.h"

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec_utils.h" // vector casting

#include "softlight/SL_BlitProcesor.hpp"
#include "softlight/SL_Color.hpp"
#include "softlight/SL_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions and namespaces
-----------------------------------------------------------------------------*/
namespace math = ls::math;

/*-------------------------------------
 * Recolor to R
-------------------------------------*/
template<typename inColor_type, typename outColor_type>
struct SL_Blit_R_to_R
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRType<inColor_type> inColor = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RG_to_R
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGType<inColor_type> inColor = pTexture->texel<SL_ColorRGType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RGB_to_R
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBType<inColor_type> inColor = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RGBA_to_R
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBAType<inColor_type> inColor = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor)[0];
    }
};



/*-------------------------------------
 * Recolor to RG
-------------------------------------*/
template<typename inColor_type, typename outColor_type>
struct SL_Blit_R_to_RG
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRType<inColor_type>  inColorR = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGType<inColor_type> inColor  = SL_ColorRGType<inColor_type>{inColorR[0], SL_ColorLimits<inColor_type, SL_ColorRType>::min().r};

        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RG_to_RG
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGType<inColor_type> inColor = pTexture->texel<SL_ColorRGType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RGB_to_RG
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBType<inColor_type> inColorRGB = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGType<inColor_type>  inColor    = ls::math::vec2_cast(inColorRGB);

        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RGBA_to_RG
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBAType<inColor_type> inColorRGBA = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGType<inColor_type>   inColor     = ls::math::vec2_cast(inColorRGBA);

        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * Recolor to RGB
-------------------------------------*/
template<typename inColor_type, typename outColor_type>
struct SL_Blit_R_to_RGB
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRType<inColor_type>   inColorR = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBType<inColor_type> inColor  = SL_ColorRGBType<inColor_type>{SL_ColorLimits<inColor_type, SL_ColorRType>::min().r, SL_ColorLimits<inColor_type, SL_ColorRType>::min().r, inColorR[0]};

        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RG_to_RGB
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGType<inColor_type>  inColorRG = pTexture->texel<SL_ColorRGType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBType<inColor_type> inColor   = ls::math::vec3_cast(inColorRG, SL_ColorLimits<inColor_type, SL_ColorRType>::min().r);

        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RGB_to_RGB
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBType<inColor_type> inColor = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RGBA_to_RGB
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBAType<inColor_type> inColorRGBA = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBType<inColor_type>  inColor     = ls::math::vec3_cast(inColorRGBA);

        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * Recolor to RGBA
-------------------------------------*/
template<typename inColor_type, typename outColor_type>
struct SL_Blit_R_to_RGBA
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBAType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRType<inColor_type>    inColorR = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<inColor_type> inColor  = SL_ColorRGBAType<inColor_type>{SL_ColorLimits<inColor_type, SL_ColorRType>::min().r, SL_ColorLimits<inColor_type, SL_ColorRType>::min().r, inColorR[0], SL_ColorLimits<inColor_type, SL_ColorRGBAType>::max()[3]};

        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RG_to_RGBA
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBAType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGType<inColor_type>   inColorRG = pTexture->texel<SL_ColorRGType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<inColor_type> inColor   = ls::math::vec4_cast(SL_ColorLimits<inColor_type, SL_ColorRType>::min().r, inColorRG, SL_ColorLimits<inColor_type, SL_ColorRGBAType>::max()[3]);

        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

template<typename inColor_type, typename outColor_type>
struct SL_Blit_RGB_to_RGBA
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBAType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBType<inColor_type>  inColorRGB = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<inColor_type> inColor    = ls::math::vec4_cast(inColorRGB, SL_ColorLimits<inColor_type, SL_ColorRGBAType>::max()[3]);

        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};


template<typename inColor_type, typename outColor_type>
struct SL_Blit_RGBA_to_RGBA
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBAType<outColor_type>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBAType<inColor_type> inColor = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + outIndex) = color_cast<outColor_type, inColor_type>(inColor);
    }
};

// I spent enough time on blitting... Here, I'm only optimizing for the most
// common blit operations, from RGBAf & RGBA8 to RGBA8
template<>
struct SL_Blit_RGBA_to_RGBA<uint8_t, uint8_t>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBAType<uint8_t>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const int32_t inColor = pTexture->texel<int32_t>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<int32_t*>(pOutBuf + outIndex) = inColor;
    }
};



template<>
struct SL_Blit_RGBA_to_RGBA<float, uint8_t>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGBAType<uint8_t>)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBAType<float>   inColor = pTexture->texel<SL_ColorRGBAType<float>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<uint8_t> in = color_cast<uint8_t, float>(inColor);
        *reinterpret_cast<int32_t*>(pOutBuf + outIndex) = reinterpret_cast<const int32_t&>(in);
    }
};



/*-----------------------------------------------------------------------------
 * SL_BlitProcessor functions and namespaces
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Nearest-neighbor filtering (R Channel)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitProcessor::blit_src_r() noexcept
{
    switch (mDstTex->type())
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

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (R & G Channels)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitProcessor::blit_src_rg() noexcept
{
    switch (mDstTex->type())
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

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGB)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitProcessor::blit_src_rgb() noexcept
{
    switch (mDstTex->type())
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

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitProcessor::blit_src_rgba() noexcept
{
    switch (mDstTex->type())
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

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<class BlipOp>
void SL_BlitProcessor::blit_nearest() noexcept
{
    constexpr BlipOp blitOp;
    unsigned char* const pOutBuf = reinterpret_cast<unsigned char* const>(mDstTex->data());

    const uint_fast32_t inW  = (uint_fast32_t)srcX1 - (uint_fast32_t)srcX0;
    const uint_fast32_t inH  = (uint_fast32_t)srcY1 - (uint_fast32_t)srcY0;
    const uint_fast32_t outW = (uint_fast32_t)dstX1 - (uint_fast32_t)dstX0;

    const uint_fast32_t totalOutW = mDstTex->width();
    const uint_fast32_t totalOutH = mDstTex->height();

    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast32_t x0    = ls::math::max<uint_fast32_t>(0u, dstX0);
    const uint_fast32_t x1    = ls::math::min<uint_fast32_t>(totalOutW, x0 + outW);
    const uint_fast32_t y0    = dstY0+mThreadId;
    const uint_fast32_t y1    = dstY1;
    const uint_fast32_t finW  = (inW << NUM_FIXED_BITS);
    const uint_fast32_t finH  = (inH << NUM_FIXED_BITS);
    const uint_fast32_t foutW = (finW / totalOutW) + 1u; // account for rounding errors
    const uint_fast32_t foutH = (finH / totalOutH) + 1u;

    uint_fast32_t y = y0;

    while (LS_LIKELY(y < y1))
    {
        const uint_fast32_t yf       = (y * foutH) >> NUM_FIXED_BITS;
        const uint_fast32_t srcY     = srcY1 - (srcY0 + yf) - 1u;
        uint_fast32_t       outIndex = (x0 + totalOutW * y) * BlipOp::stride;

        uint_fast32_t x = x0;

        while (LS_LIKELY(x < x1))
        {
            const uint_fast32_t xf   = x * foutW;
            const uint_fast32_t srcX = xf >> NUM_FIXED_BITS;

            blitOp(mSrcTex, srcX, srcY, pOutBuf, outIndex);
            ++x;
            outIndex += BlipOp::stride;
        }

        y += mNumThreads;
    }
}



/*-------------------------------------
 * Run the texture blitter
-------------------------------------*/
void SL_BlitProcessor::execute() noexcept
{
    LS_ASSERT(!sl_is_compressed_color(mSrcTex->type()) && !sl_is_compressed_color(mDstTex->type()));

    switch (mSrcTex->type())
    {
        case SL_COLOR_R_8U:       blit_src_r<uint8_t>();     break;
        case SL_COLOR_R_16U:      blit_src_r<uint16_t>();    break;
        case SL_COLOR_R_32U:      blit_src_r<uint32_t>();    break;
        case SL_COLOR_R_64U:      blit_src_r<uint64_t>();    break;
        case SL_COLOR_R_FLOAT:    blit_src_r<float>();       break;
        case SL_COLOR_R_DOUBLE:   blit_src_r<double>();      break;

        case SL_COLOR_RG_8U:      blit_src_rg<uint8_t>();    break;
        case SL_COLOR_RG_16U:     blit_src_rg<uint16_t>();   break;
        case SL_COLOR_RG_32U:     blit_src_rg<uint32_t>();   break;
        case SL_COLOR_RG_64U:     blit_src_rg<uint64_t>();   break;
        case SL_COLOR_RG_FLOAT:   blit_src_rg<float>();      break;
        case SL_COLOR_RG_DOUBLE:  blit_src_rg<double>();     break;

        case SL_COLOR_RGB_8U:     blit_src_rgb<uint8_t>();   break;
        case SL_COLOR_RGB_16U:    blit_src_rgb<uint16_t>();  break;
        case SL_COLOR_RGB_32U:    blit_src_rgb<uint32_t>();  break;
        case SL_COLOR_RGB_64U:    blit_src_rgb<uint64_t>();  break;
        case SL_COLOR_RGB_FLOAT:  blit_src_rgb<float>();     break;
        case SL_COLOR_RGB_DOUBLE: blit_src_rgb<double>();    break;

        case SL_COLOR_RGBA_8U:     blit_src_rgba<uint8_t>();  break;
        case SL_COLOR_RGBA_16U:    blit_src_rgba<uint16_t>(); break;
        case SL_COLOR_RGBA_32U:    blit_src_rgba<uint32_t>(); break;
        case SL_COLOR_RGBA_64U:    blit_src_rgba<uint64_t>(); break;
        case SL_COLOR_RGBA_FLOAT:  blit_src_rgba<float>();    break;
        case SL_COLOR_RGBA_DOUBLE: blit_src_rgba<double>();   break;

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}
