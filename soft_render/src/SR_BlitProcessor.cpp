
#include "lightsky/math/scalar_utils.h"
#include "lightsky/math/fixed.h"

#include "soft_render/SR_BlitProcesor.hpp"
#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_WindowBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions and namespaces
-----------------------------------------------------------------------------*/
namespace math = ls::math;
using sr_fixed_type = math::ulong_lowp_t;



/*-----------------------------------------------------------------------------
 * SR_BlitProcessor Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Nearest-neighbor filtering
-------------------------------------*/
void SR_BlitProcessor::blit_nearest(
    SR_ColorRGBA8* const pOutBuf,
    const uint_fast16_t  inW,
    const uint_fast16_t  inH,
    const uint_fast16_t  outW,
    const uint_fast16_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast16_t dstH  = outH / mNumThreads;
    const uint_fast16_t dstY0 = mThreadId * dstH;
    const uint_fast16_t dstY1 = dstY0 + dstH;
    const sr_fixed_type finW  = math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH  = math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW = finW / math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH = finH / math::fixed_cast<sr_fixed_type>(outH);

    const uint_fast16_t numPixels = (outW*outH) - 1;

    for (uint_fast16_t y = dstY0; y < dstY1; ++y)
    {
        const sr_fixed_type yf = math::fixed_cast<sr_fixed_type>(y);

        for (uint_fast16_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf   = math::fixed_cast<sr_fixed_type>(x);
            uint_fast16_t       srcX = (uint_fast16_t)(xf * foutW);
            uint_fast16_t       srcY = (uint_fast16_t)(yf * foutH);

            srcX = ls::math::min<uint_fast16_t>(srcX, inW - 1);
            srcY = ls::math::min<uint_fast16_t>(srcY, inH - 1);

            const uint_fast16_t outIndex = x + outW * y;
            const SR_ColorRGB8  inColor  = mTexture->texel<SR_ColorRGB8>(srcX, srcY);
            SR_ColorRGBA8&      outColor = pOutBuf[numPixels - outIndex];

            outColor = SR_ColorRGBA8{inColor.r, inColor.g, inColor.b, 255};
        }
    }
}



/*-------------------------------------
 * Run the texture blitter
-------------------------------------*/
void SR_BlitProcessor::execute() noexcept
{
    // calculate the bounds of the tile which a certain thread will be
    // responsible for
    const uint_fast16_t bufferW = mBackBuffer->width();
    const uint_fast16_t bufferH = mBackBuffer->height();
    const uint_fast16_t texW    = mTexture->width();
    const uint_fast16_t texH    = mTexture->height();
    SR_ColorRGBA8*      pDest   = mBackBuffer->buffer();

    blit_nearest(pDest, texW, texH, bufferW, bufferH);
}
