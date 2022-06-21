
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
 * Compressed to Compressed
-------------------------------------*/
template<typename inCompressed_type, typename outCompressed_type>
struct SL_Blit_Compressed_to_Compressed;



/*-------------------------------------
 * Compressed to R
-------------------------------------*/
template<typename compressed_type, typename outColor_type>
struct SL_Blit_Compressed_to_R
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
        const compressed_type& inColor = pTexture->texel<compressed_type>((uint16_t)srcX, (uint16_t)srcY);
        reinterpret_cast<SL_ColorRType<outColor_type>*>(pOutBuf + outIndex)->r = rgb_cast<outColor_type, compressed_type>(inColor)[0];
    }
};



/*-------------------------------------
 * Compressed to RG
-------------------------------------*/
template<typename compressed_type, typename outColor_type>
struct SL_Blit_Compressed_to_RG
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
        const compressed_type& inColor = pTexture->texel<compressed_type>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGType<outColor_type>*>(pOutBuf + outIndex) = ls::math::vec2_cast(rgba_cast<outColor_type, compressed_type>(inColor));
    }
};



/*-------------------------------------
 * Compressed to RGB
-------------------------------------*/
template<typename compressed_type, typename outColor_type>
struct SL_Blit_Compressed_to_RGB
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
        const compressed_type& inColor = pTexture->texel<compressed_type>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBType<outColor_type>*>(pOutBuf + outIndex) = rgb_cast<outColor_type, compressed_type>(inColor);
    }
};



/*-------------------------------------
 * Compressed to RGBA
-------------------------------------*/
template<typename compressed_type, typename outColor_type>
struct SL_Blit_Compressed_to_RGBA
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
        const compressed_type& inColor = pTexture->texel<compressed_type>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<SL_ColorRGBAType<outColor_type>*>(pOutBuf + outIndex) = rgba_cast<outColor_type, compressed_type>(inColor);
    }
};



/*-------------------------------------
 * R to Compressed
-------------------------------------*/
template<typename inColor_type, typename compressed_type>
struct SL_Blit_R_to_Compressed
{
    enum : uint_fast32_t
    {
        stride = sizeof(compressed_type)
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

        *reinterpret_cast<compressed_type*>(pOutBuf + outIndex) = rgb_cast<compressed_type, inColor_type>(inColor);
    }
};




/*-------------------------------------
 * RG to Compressed
-------------------------------------*/
template<typename inColor_type, typename compressed_type>
struct SL_Blit_RG_to_Compressed
{
    enum : uint_fast32_t
    {
        stride = sizeof(compressed_type)
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

        *reinterpret_cast<compressed_type*>(pOutBuf + outIndex) = rgb_cast<compressed_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * RGB to Compressed
-------------------------------------*/
template<typename inColor_type, typename compressed_type>
struct SL_Blit_RGB_to_Compressed
{
    enum : uint_fast32_t
    {
        stride = sizeof(compressed_type)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGBType<inColor_type> inColor = pTexture->texel<SL_ColorRGBType<inColor_type>>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<compressed_type*>(pOutBuf + outIndex) = rgb_cast<compressed_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * RGBA to Compressed
-------------------------------------*/
template<typename inColor_type, typename compressed_type>
struct SL_Blit_RGBA_to_Compressed
{
    enum : uint_fast32_t
    {
        stride = sizeof(compressed_type)
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

        *reinterpret_cast<compressed_type*>(pOutBuf + outIndex) = rgb_cast<compressed_type, inColor_type>(inColor);
    }
};



/*-------------------------------------
 * Compressed to Compressed
-------------------------------------*/
template<typename inCompressed_type>
struct SL_Blit_Compressed_to_Compressed<inCompressed_type, inCompressed_type>
{
    enum : uint_fast32_t
    {
        stride = sizeof(inCompressed_type)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const inCompressed_type& inColor = pTexture->texel<inCompressed_type>((uint16_t)srcX, (uint16_t)srcY);
        *reinterpret_cast<inCompressed_type*>(pOutBuf + outIndex) = inColor;
    }
};



/*-------------------------------------
 * Convert to 332
-------------------------------------*/
template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB565, SL_ColorRGB332>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB332)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec3_t<uint8_t> outColor = rgb_cast<uint8_t, SL_ColorRGB565>(inColor);
        *reinterpret_cast<SL_ColorRGB332*>(pOutBuf + outIndex) = rgb_cast<SL_ColorRGB332, uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB5551, SL_ColorRGB332>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB332)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec3_t<uint8_t> outColor = rgb_cast<uint8_t, SL_ColorRGB5551>(inColor);
        *reinterpret_cast<SL_ColorRGB332*>(pOutBuf + outIndex) = rgb_cast<SL_ColorRGB332, uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB4444, SL_ColorRGB332>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB332)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec3_t<uint8_t> outColor = rgb_cast<uint8_t, SL_ColorRGB4444>(inColor);
        *reinterpret_cast<SL_ColorRGB332*>(pOutBuf + outIndex) = rgb_cast<SL_ColorRGB332, uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB1010102, SL_ColorRGB332>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB332)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB1010102 inColor = pTexture->texel<SL_ColorRGB1010102>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec3_t<uint8_t> outColor = rgb_cast<uint8_t, SL_ColorRGB1010102>(inColor);
        *reinterpret_cast<SL_ColorRGB332*>(pOutBuf + outIndex) = rgb_cast<SL_ColorRGB332, uint8_t>(outColor);
    }
};



/*-------------------------------------
 * Convert to 565
-------------------------------------*/
template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB332, SL_ColorRGB565>
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
        const SL_ColorRGB332 inColor = pTexture->texel<SL_ColorRGB332>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec3_t<uint8_t> outColor = rgb_cast<uint8_t, SL_ColorRGB332>(inColor);
        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb_cast<SL_ColorRGB565, uint8_t>(outColor);
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
        const ls::math::vec3_t<uint8_t> outColor = rgb_cast<uint8_t, SL_ColorRGB5551>(inColor);
        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb_cast<SL_ColorRGB565, uint8_t>(outColor);
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
        const ls::math::vec3_t<uint8_t> outColor = rgb_cast<uint8_t, SL_ColorRGB4444>(inColor);
        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb_cast<SL_ColorRGB565, uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB1010102, SL_ColorRGB565>
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
        const SL_ColorRGB1010102 inColor = pTexture->texel<SL_ColorRGB1010102>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec3_t<uint8_t> outColor = rgb_cast<uint8_t, SL_ColorRGB1010102>(inColor);
        *reinterpret_cast<SL_ColorRGB565*>(pOutBuf + outIndex) = rgb_cast<SL_ColorRGB565, uint8_t>(outColor);
    }
};



/*-------------------------------------
 * Convert to 5551
-------------------------------------*/
template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB332, SL_ColorRGB5551>
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
        const SL_ColorRGB332 inColor = pTexture->texel<SL_ColorRGB332>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint8_t> outRGBA = rgba_cast<uint8_t, SL_ColorRGB332>(inColor);
        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB5551, uint8_t>(outRGBA);
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
        const ls::math::vec4_t<uint8_t> outRGBA = rgba_cast<uint8_t, SL_ColorRGB565>(inColor);
        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB5551, uint8_t>(outRGBA);
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
        const ls::math::vec4_t<uint8_t> outColor = rgba_cast<uint8_t, SL_ColorRGB4444>(inColor);
        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB5551, uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB1010102, SL_ColorRGB5551>
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
        const SL_ColorRGB1010102 inColor = pTexture->texel<SL_ColorRGB1010102>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint8_t> outColor = rgba_cast<uint8_t, SL_ColorRGB1010102>(inColor);
        *reinterpret_cast<SL_ColorRGB5551*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB5551, uint8_t>(outColor);
    }
};



/*-------------------------------------
 * Convert to 4444
-------------------------------------*/
template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB332, SL_ColorRGB4444>
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
        const SL_ColorRGB332 inColor = pTexture->texel<SL_ColorRGB332>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint8_t> outRGBA = rgba_cast<uint8_t, SL_ColorRGB332>(inColor);
        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB4444, uint8_t>(outRGBA);
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
        const ls::math::vec4_t<uint8_t> outRGBA = rgba_cast<uint8_t, SL_ColorRGB565>(inColor);
        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB4444, uint8_t>(outRGBA);
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
        const ls::math::vec4_t<uint8_t> outColor = rgba_cast<uint8_t, SL_ColorRGB5551>(inColor);
        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB4444, uint8_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB1010102, SL_ColorRGB4444>
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
        const SL_ColorRGB1010102 inColor = pTexture->texel<SL_ColorRGB1010102>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint8_t> outColor = rgba_cast<uint8_t, SL_ColorRGB1010102>(inColor);
        *reinterpret_cast<SL_ColorRGB4444*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB4444, uint8_t>(outColor);
    }
};



/*-------------------------------------
 * Convert to 1010102
-------------------------------------*/
template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB332, SL_ColorRGB1010102>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB1010102)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB332 inColor = pTexture->texel<SL_ColorRGB332>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint16_t> outRGBA = rgba_cast<uint16_t, SL_ColorRGB332>(inColor);
        *reinterpret_cast<SL_ColorRGB1010102*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB1010102, uint16_t>(outRGBA);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB565, SL_ColorRGB1010102>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB1010102)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB565 inColor = pTexture->texel<SL_ColorRGB565>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint16_t> outRGBA = rgba_cast<uint16_t, SL_ColorRGB565>(inColor);
        *reinterpret_cast<SL_ColorRGB1010102*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB1010102, uint16_t>(outRGBA);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB5551, SL_ColorRGB1010102>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB1010102)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB5551 inColor = pTexture->texel<SL_ColorRGB5551>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint16_t> outColor = rgba_cast<uint16_t, SL_ColorRGB5551>(inColor);
        *reinterpret_cast<SL_ColorRGB1010102*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB1010102, uint16_t>(outColor);
    }
};



template<>
struct SL_Blit_Compressed_to_Compressed<SL_ColorRGB4444, SL_ColorRGB1010102>
{
    enum : uint_fast32_t
    {
        stride = sizeof(SL_ColorRGB1010102)
    };

    inline LS_INLINE void operator()(
        const SL_Texture* pTexture,
        const uint_fast32_t srcX,
        const uint_fast32_t srcY,
        unsigned char* const pOutBuf,
        uint_fast32_t outIndex) const noexcept
    {
        const SL_ColorRGB4444 inColor = pTexture->texel<SL_ColorRGB4444>((uint16_t)srcX, (uint16_t)srcY);
        const ls::math::vec4_t<uint16_t> outColor = rgba_cast<uint16_t, SL_ColorRGB4444>(inColor);
        *reinterpret_cast<SL_ColorRGB1010102*>(pOutBuf + outIndex) = rgba_cast<SL_ColorRGB1010102, uint16_t>(outColor);
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
        case SL_COLOR_RGB_332:      blit_nearest<SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB332>>();     break;
        case SL_COLOR_RGB_565:      blit_nearest<SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB565>>();     break;
        case SL_COLOR_RGBA_5551:    blit_nearest<SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB5551>>();    break;
        case SL_COLOR_RGBA_4444:    blit_nearest<SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB4444>>();    break;
        case SL_COLOR_RGBA_1010102: blit_nearest<SL_Blit_R_to_Compressed<inColor_type, SL_ColorRGB1010102>>(); break;

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
        case SL_COLOR_RGB_332:      blit_nearest<SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB332>>();     break;
        case SL_COLOR_RGB_565:      blit_nearest<SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB565>>();     break;
        case SL_COLOR_RGBA_5551:    blit_nearest<SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB5551>>();    break;
        case SL_COLOR_RGBA_4444:    blit_nearest<SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB4444>>();    break;
        case SL_COLOR_RGBA_1010102: blit_nearest<SL_Blit_RG_to_Compressed<inColor_type, SL_ColorRGB1010102>>(); break;

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
        case SL_COLOR_RGB_332:      blit_nearest<SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB332>>();     break;
        case SL_COLOR_RGB_565:      blit_nearest<SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB565>>();     break;
        case SL_COLOR_RGBA_5551:    blit_nearest<SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB5551>>();    break;
        case SL_COLOR_RGBA_4444:    blit_nearest<SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB4444>>();    break;
        case SL_COLOR_RGBA_1010102: blit_nearest<SL_Blit_RGB_to_Compressed<inColor_type, SL_ColorRGB1010102>>(); break;

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
        case SL_COLOR_RGB_332:      blit_nearest<SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB332>>();     break;
        case SL_COLOR_RGB_565:      blit_nearest<SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB565>>();     break;
        case SL_COLOR_RGBA_5551:    blit_nearest<SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB5551>>();    break;
        case SL_COLOR_RGBA_4444:    blit_nearest<SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB4444>>();    break;
        case SL_COLOR_RGBA_1010102: blit_nearest<SL_Blit_RGBA_to_Compressed<inColor_type, SL_ColorRGB1010102>>(); break;

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

        case SL_COLOR_RGB_332:      blit_nearest<SL_Blit_Compressed_to_Compressed<inColor_type, SL_ColorRGB332>>();     break;
        case SL_COLOR_RGB_565:      blit_nearest<SL_Blit_Compressed_to_Compressed<inColor_type, SL_ColorRGB565>>();     break;
        case SL_COLOR_RGBA_5551:    blit_nearest<SL_Blit_Compressed_to_Compressed<inColor_type, SL_ColorRGB5551>>();    break;
        case SL_COLOR_RGBA_4444:    blit_nearest<SL_Blit_Compressed_to_Compressed<inColor_type, SL_ColorRGB4444>>();    break;
        case SL_COLOR_RGBA_1010102: blit_nearest<SL_Blit_Compressed_to_Compressed<inColor_type, SL_ColorRGB1010102>>(); break;

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

        case SL_COLOR_RGB_332:      blit_src_compressed<SL_ColorRGB332>();     break;
        case SL_COLOR_RGB_565:      blit_src_compressed<SL_ColorRGB565>();     break;
        case SL_COLOR_RGBA_5551:    blit_src_compressed<SL_ColorRGB5551>();    break;
        case SL_COLOR_RGBA_4444:    blit_src_compressed<SL_ColorRGB4444>();    break;
        case SL_COLOR_RGBA_1010102: blit_src_compressed<SL_ColorRGB1010102>(); break;

        default:
            LS_ASSERT(false);
            LS_UNREACHABLE();
    }
}
