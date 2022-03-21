
#include "lightsky/utils/Assertions.h"

#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/vec_utils.h" // vector casting

#include "softlight/SL_BlitCompressedProcesor.hpp"
#include "softlight/SL_ColorCompressed.hpp"
#include "softlight/SL_Texture.hpp"



/*-----------------------------------------------------------------------------
 * Helper functions and namespaces
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Compressed to R/G/B/A
-------------------------------------*/
template<typename compressed_type, typename outColor_type>
struct SL_Blit_Compressed_to_R;

template<typename compressed_type, typename outColor_type>
struct SL_Blit_Compressed_to_RG;

template<typename compressed_type, typename outColor_type>
struct SL_Blit_Compressed_to_RGB;

template<typename compressed_type, typename outColor_type>
struct SL_Blit_Compressed_to_RGBA;



/*-------------------------------------
 * R/G/B/A to Compressed
-------------------------------------*/
template<typename compressed_type, typename outColor_type>
struct SL_Blit_R_to_Compressed;

template<typename compressed_type, typename outColor_type>
struct SL_Blit_RG_to_Compressed;

template<typename compressed_type, typename outColor_type>
struct SL_Blit_RGB_to_Compressed;

template<typename compressed_type, typename outColor_type>
struct SL_Blit_RGBA_to_Compressed;



/*-------------------------------------
 * Compressed to Compressed
-------------------------------------*/
template<typename inCompressed_type, typename outCompressed_type>
struct SL_Blit_Compressed_to_Compressed;



/*-------------------------------------
 * Compressed to R
-------------------------------------*/
template<typename outColor_type>
struct SL_Blit_Compressed_to_R<SL_ColorRGB565, outColor_type>
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
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + outIndex) = rgb_cast<outColor_type>(inColor)[0];
    }
};



template<typename outColor_type>
struct SL_Blit_Compressed_to_R<SL_ColorRGB5551, outColor_type>
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
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + outIndex) = rgb_cast<outColor_type>(inColor)[0];
    }
};



template<typename outColor_type>
struct SL_Blit_Compressed_to_R<SL_ColorRGB4444, outColor_type>
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
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + outIndex) = rgb_cast<outColor_type>(inColor)[0];
    }
};



/*-------------------------------------
 * Compressed to RG
-------------------------------------*/
template<typename outColor_type>
struct SL_Blit_Compressed_to_RG<SL_ColorRGB565, outColor_type>
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
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + outIndex) = ls::math::vec2_cast(rgb_cast<outColor_type>(inColor));
    }
};



template<typename outColor_type>
struct SL_Blit_Compressed_to_RG<SL_ColorRGB5551, outColor_type>
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
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + outIndex) = ls::math::vec2_cast(rgb_cast<outColor_type>(inColor));
    }
};



template<typename outColor_type>
struct SL_Blit_Compressed_to_RG<SL_ColorRGB4444, outColor_type>
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
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + outIndex) = ls::math::vec2_cast(rgb_cast<outColor_type>(inColor));
    }
};



/*-------------------------------------
 * Compressed to RGB
-------------------------------------*/
template<typename outColor_type>
struct SL_Blit_Compressed_to_RGB<SL_ColorRGB565, outColor_type>
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
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + outIndex) = rgb_cast<outColor_type>(inColor);
    }
};



template<typename outColor_type>
struct SL_Blit_Compressed_to_RGB<SL_ColorRGB5551, outColor_type>
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
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + outIndex) = ls::math::vec3_cast(rgb_cast<outColor_type>(inColor));
    }
};



template<typename outColor_type>
struct SL_Blit_Compressed_to_RGB<SL_ColorRGB4444, outColor_type>
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
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + outIndex) = ls::math::vec3_cast(rgb_cast<outColor_type>(inColor));
    }
};



/*-------------------------------------
 * Compressed to RGBA
-------------------------------------*/
template<typename outColor_type>
struct SL_Blit_Compressed_to_RGBA<SL_ColorRGB565, outColor_type>
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
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + outIndex) = ls::math::vec4_cast(rgb_cast<outColor_type>(inColor), SL_ColorLimits<outColor_type, SL_ColorRGBAType>::max()[3]);
    }
};



template<typename outColor_type>
struct SL_Blit_Compressed_to_RGBA<SL_ColorRGB5551, outColor_type>
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
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + outIndex) = rgb_cast<outColor_type>(inColor);
    }
};



template<typename outColor_type>
struct SL_Blit_Compressed_to_RGBA<SL_ColorRGB4444, outColor_type>
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
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + outIndex) = rgb_cast<outColor_type>(inColor);
    }
};



/*-------------------------------------
 * R to Compressed
-------------------------------------*/
template<typename inColor_type>
struct SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB565>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB565)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRType<inColor_type>  inColorR = pTexture->texel<SL_ColorRType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBType<inColor_type> inColor   = ls::math::vec3_t<inColor_type>(inColorR.r, SL_ColorLimits<inColor_type, SL_ColorRType>::min().r, SL_ColorLimits<inColor_type, SL_ColorRType>::min().r);

        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb565_cast<inColor_type>(inColor);
    }
};



template<typename inColor_type>
struct SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB5551>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB5551)
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

        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgb5551_cast<inColor_type>(inColor);
    }
};



template<typename inColor_type>
struct SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB4444>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB4444)
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

        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgb4444_cast<inColor_type>(inColor);
    }
};




/*-------------------------------------
 * RG to Compressed
-------------------------------------*/
template<typename inColor_type>
struct SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB565>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB565)
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

        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb565_cast<inColor_type>(inColor);
    }
};



template<typename inColor_type>
struct SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB5551>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB5551)
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

        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgb5551_cast<inColor_type>(inColor);
    }
};



template<typename inColor_type>
struct SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB4444>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB4444)
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

        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgb4444_cast<inColor_type>(inColor);
    }
};



/*-------------------------------------
 * RGB to Compressed
-------------------------------------*/
template<typename inColor_type>
struct SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB565>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB565)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBType<inColor_type> inColor = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb565_cast<inColor_type>(inColor);
    }
};



template<typename inColor_type>
struct SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB5551>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB5551)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBType<inColor_type>   inColorRGB = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<inColor_type> inColor   = ls::math::vec4_cast(inColorRGB, SL_ColorLimits<inColor_type, SL_ColorRGBAType>::max()[3]);

        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgb5551_cast<inColor_type>(inColor);
    }
};



template<typename inColor_type>
struct SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB4444>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB4444)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBType<inColor_type>   inColorRGB = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        const SL_ColorRGBAType<inColor_type> inColor   = ls::math::vec4_cast(inColorRGB, SL_ColorLimits<inColor_type, SL_ColorRGBAType>::max()[3]);

        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgb4444_cast<inColor_type>(inColor);
    }
};



/*-------------------------------------
 * RGBA to Compressed
-------------------------------------*/
template<typename inColor_type>
struct SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB565>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB565)
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

        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb565_cast<inColor_type>(inColor);
    }
};



template<typename inColor_type>
struct SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB5551>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB5551)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBAType<inColor_type> inColor = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgb5551_cast<inColor_type>(inColor);
    }
};



template<typename inColor_type>
struct SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB4444>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB4444)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBAType<inColor_type> inColor = pTexture->texel<SL_ColorRGBAType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgb4444_cast<inColor_type>(inColor);
    }
};



/*-------------------------------------
 * Compressed to Compressed
-------------------------------------*/
template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB565, SL_ColorRGB565>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB565)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = inColor;
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB5551, SL_ColorRGB565>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB565)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec3_t<uint8_t> outColor = ls::math::vec3_cast<uint8_t>(rgb_cast<uint8_t>(inColor));
        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb565_cast<uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB4444, SL_ColorRGB565>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB565)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec3_t<uint8_t> outColor = ls::math::vec3_cast<uint8_t>(rgb_cast<uint8_t>(inColor));
        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb565_cast<uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB565, SL_ColorRGB5551>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB5551)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint8_t> outRGBA = ls::math::vec4_cast<uint8_t>(rgb_cast<uint8_t>(inColor), SL_ColorLimits<uint8_t, SL_ColorRGB5551Type>::max().a);
        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgb5551_cast<uint8_t>(outRGBA);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB5551, SL_ColorRGB5551>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB5551)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = inColor;
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB4444, SL_ColorRGB5551>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB5551)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint8_t> outColor = rgb_cast<uint8_t>(inColor);
        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgb5551_cast<uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB565, SL_ColorRGB4444>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB4444)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint8_t> outRGBA = ls::math::vec4_cast<uint8_t>(rgb_cast<uint8_t>(inColor), SL_ColorLimits<uint8_t, SL_ColorRGB4444Type>::max().a);
        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgb4444_cast<uint8_t>(outRGBA);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB5551, SL_ColorRGB4444>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB4444)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint8_t> outColor = rgb_cast<uint8_t>(inColor);
        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgb4444_cast<uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB4444, SL_ColorRGB4444>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB4444)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = inColor;
    }
};




/*-----------------------------------------------------------------------------
 * SL_BlitProcessorCompressed functions and namespaces
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Nearest-neighbor filtering (R Channel)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitCompressedProcessor::blit_src_r() noexcept
{
    switch (mDstTex->type())
    {
        case SL_COLOR_RGB_565:     blit_nearest<SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB565>>();  break;
        case SL_COLOR_RGBA_5551:   blit_nearest<SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB5551>>(); break;
        case SL_COLOR_RGBA_4444:   blit_nearest<SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB4444>>(); break;

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (R & G Channels)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitCompressedProcessor::blit_src_rg() noexcept
{
    switch (mDstTex->type())
    {
        case SL_COLOR_RGB_565:     blit_nearest<SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB565>>();  break;
        case SL_COLOR_RGBA_5551:   blit_nearest<SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB5551>>(); break;
        case SL_COLOR_RGBA_4444:   blit_nearest<SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB4444>>(); break;

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGB)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitCompressedProcessor::blit_src_rgb() noexcept
{
    switch (mDstTex->type())
    {
        case SL_COLOR_RGB_565:     blit_nearest<SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB565>>();  break;
        case SL_COLOR_RGBA_5551:   blit_nearest<SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB5551>>(); break;
        case SL_COLOR_RGBA_4444:   blit_nearest<SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB4444>>(); break;

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitCompressedProcessor::blit_src_rgba() noexcept
{
    switch (mDstTex->type())
    {
        case SL_COLOR_RGB_565:     blit_nearest<SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB565>>();  break;
        case SL_COLOR_RGBA_5551:   blit_nearest<SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB5551>>(); break;
        case SL_COLOR_RGBA_4444:   blit_nearest<SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB4444>>(); break;

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<typename inColor_type>
void SL_BlitCompressedProcessor::blit_src_compressed() noexcept
{
    switch (mDstTex->type())
    {
        case SL_COLOR_R_8U:         blit_nearest<SL_Blit_Compressed_to_R<inColor_type, uint8_t>>();     break;
        case SL_COLOR_R_16U:        blit_nearest<SL_Blit_Compressed_to_R<inColor_type, uint16_t>>();    break;
        case SL_COLOR_R_32U:        blit_nearest<SL_Blit_Compressed_to_R<inColor_type, uint32_t>>();    break;
        case SL_COLOR_R_64U:        blit_nearest<SL_Blit_Compressed_to_R<inColor_type, uint64_t>>();    break;
        case SL_COLOR_R_FLOAT:      blit_nearest<SL_Blit_Compressed_to_R<inColor_type, float>>();       break;
        case SL_COLOR_R_DOUBLE:     blit_nearest<SL_Blit_Compressed_to_R<inColor_type, double>>();      break;

        case SL_COLOR_RG_8U:        blit_nearest<SL_Blit_Compressed_to_RG<inColor_type, uint8_t>>();    break;
        case SL_COLOR_RG_16U:       blit_nearest<SL_Blit_Compressed_to_RG<inColor_type, uint16_t>>();   break;
        case SL_COLOR_RG_32U:       blit_nearest<SL_Blit_Compressed_to_RG<inColor_type, uint32_t>>();   break;
        case SL_COLOR_RG_64U:       blit_nearest<SL_Blit_Compressed_to_RG<inColor_type, uint64_t>>();   break;
        case SL_COLOR_RG_FLOAT:     blit_nearest<SL_Blit_Compressed_to_RG<inColor_type, float>>();      break;
        case SL_COLOR_RG_DOUBLE:    blit_nearest<SL_Blit_Compressed_to_RG<inColor_type, double>>();     break;

        case SL_COLOR_RGB_8U:       blit_nearest<SL_Blit_Compressed_to_RGB<inColor_type, uint8_t>>();   break;
        case SL_COLOR_RGB_16U:      blit_nearest<SL_Blit_Compressed_to_RGB<inColor_type, uint16_t>>();  break;
        case SL_COLOR_RGB_32U:      blit_nearest<SL_Blit_Compressed_to_RGB<inColor_type, uint32_t>>();  break;
        case SL_COLOR_RGB_64U:      blit_nearest<SL_Blit_Compressed_to_RGB<inColor_type, uint64_t>>();  break;
        case SL_COLOR_RGB_FLOAT:    blit_nearest<SL_Blit_Compressed_to_RGB<inColor_type, float>>();     break;
        case SL_COLOR_RGB_DOUBLE:   blit_nearest<SL_Blit_Compressed_to_RGB<inColor_type, double>>();    break;

        case SL_COLOR_RGBA_8U:      blit_nearest<SL_Blit_Compressed_to_RGBA<inColor_type, uint8_t>>();  break;
        case SL_COLOR_RGBA_16U:     blit_nearest<SL_Blit_Compressed_to_RGBA<inColor_type, uint16_t>>(); break;
        case SL_COLOR_RGBA_32U:     blit_nearest<SL_Blit_Compressed_to_RGBA<inColor_type, uint32_t>>(); break;
        case SL_COLOR_RGBA_64U:     blit_nearest<SL_Blit_Compressed_to_RGBA<inColor_type, uint64_t>>(); break;
        case SL_COLOR_RGBA_FLOAT:   blit_nearest<SL_Blit_Compressed_to_RGBA<inColor_type, float>>();    break;
        case SL_COLOR_RGBA_DOUBLE:  blit_nearest<SL_Blit_Compressed_to_RGBA<inColor_type, double>>();   break;

        case SL_COLOR_RGB_565:     blit_nearest<SL_Blit_Compressed_to_Compressed<inColor_type, SL_ColorRGB565>>();  break;
        case SL_COLOR_RGBA_5551:   blit_nearest<SL_Blit_Compressed_to_Compressed<inColor_type, SL_ColorRGB5551>>(); break;
        case SL_COLOR_RGBA_4444:   blit_nearest<SL_Blit_Compressed_to_Compressed<inColor_type, SL_ColorRGB4444>>(); break;

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template<class BlitOp>
void SL_BlitCompressedProcessor::blit_nearest() noexcept
{
    constexpr BlitOp blitOp;
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
        uint_fast32_t       outIndex = (x0 + totalOutW * y) * BlitOp::stride;

        uint_fast32_t x = x0;

        while (LS_LIKELY(x < x1))
        {
            const uint_fast32_t xf   = x * foutW;
            const uint_fast32_t srcX = xf >> NUM_FIXED_BITS;

            blitOp(mSrcTex, srcX, srcY, pOutBuf, outIndex);
            ++x;
            outIndex += BlitOp::stride;
        }

        y += mNumThreads;
    }
}



/*-------------------------------------
 * Run the texture blitter
-------------------------------------*/
void SL_BlitCompressedProcessor::execute() noexcept
{
    LS_ASSERT(sl_is_compressed_color(mSrcTex->type()) || sl_is_compressed_color(mDstTex->type()));

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

        case SL_COLOR_RGB_565:     blit_src_compressed<SL_ColorRGB565>();  break;
        case SL_COLOR_RGBA_5551:   blit_src_compressed<SL_ColorRGB5551>(); break;
        case SL_COLOR_RGBA_4444:   blit_src_compressed<SL_ColorRGB4444>(); break;

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}
