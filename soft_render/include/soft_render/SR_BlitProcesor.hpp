
#ifndef SR_BLIT_PROCESOR_HPP
#define SR_BLIT_PROCESOR_HPP

#include <cstdint>

#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_WindowBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
class SR_Texture;
class SR_WindowBuffer;


/*-----------------------------------------------------------------------------
 * Encapsulation of texture blitting on another thread.
-----------------------------------------------------------------------------*/
struct SR_BlitProcessor
{
    using sr_fixed_type = ls::math::ulong_lowp_t;

    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 64-128 bits
    const SR_Texture* mTexture;
    SR_WindowBuffer* mBackBuffer;

    // 160 bits total, 20 bytes

    template <typename color_type>
    void blit_nearest_r(
        SR_ColorRGBA8* const pOutBuf,
        const uint_fast16_t  inW,
        const uint_fast16_t  inH,
        const uint_fast16_t  outW,
        const uint_fast16_t  outH
    ) noexcept;

    template <typename color_type>
    void blit_nearest_rg(
        SR_ColorRGBA8* const pOutBuf,
        const uint_fast16_t  inW,
        const uint_fast16_t  inH,
        const uint_fast16_t  outW,
        const uint_fast16_t  outH
    ) noexcept;

    template <typename color_type>
    void blit_nearest_rgb(
        SR_ColorRGBA8* const pOutBuf,
        const uint_fast16_t  inW,
        const uint_fast16_t  inH,
        const uint_fast16_t  outW,
        const uint_fast16_t  outH
    ) noexcept;

    template <typename color_type>
    void blit_nearest_rgba(
        SR_ColorRGBA8* const pOutBuf,
        const uint_fast16_t  inW,
        const uint_fast16_t  inH,
        const uint_fast16_t  outW,
        const uint_fast16_t  outH
    ) noexcept;

    void execute() noexcept;
};



/*-------------------------------------
 * External function declarations to keep compile times short
-------------------------------------*/
extern template void SR_BlitProcessor::blit_nearest_r<uint8_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_r<uint16_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_r<uint32_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_r<uint64_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_r<float>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_r<double>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);

extern template void SR_BlitProcessor::blit_nearest_rg<uint8_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rg<uint16_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rg<uint32_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rg<uint64_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rg<float>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rg<double>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);

extern template void SR_BlitProcessor::blit_nearest_rgb<uint8_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgb<uint16_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgb<uint32_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgb<uint64_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgb<float>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgb<double>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);

extern template void SR_BlitProcessor::blit_nearest_rgba<uint8_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgba<uint16_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgba<uint32_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgba<uint64_t>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgba<float>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);
extern template void SR_BlitProcessor::blit_nearest_rgba<double>(SR_ColorRGBA8* const, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t, const uint_fast16_t);



/*-------------------------------------
 * Nearest-neighbor filtering (R Channel)
-------------------------------------*/
template <typename color_type>
void SR_BlitProcessor::blit_nearest_r(
    SR_ColorRGBA8* const pOutBuf,
    const uint_fast16_t  inW,
    const uint_fast16_t  inH,
    const uint_fast16_t  outW,
    const uint_fast16_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast16_t dstH      = outH / mNumThreads;
    const uint_fast16_t dstY0     = mThreadId * dstH;
    const uint_fast16_t dstY1     = dstY0 + dstH;
    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(outH);
    const uint_fast16_t numPixels = (outW*outH) - 1;

    for (uint_fast16_t y = dstY0; y < dstY1; ++y)
    {
        const uint_fast16_t yOffset = y * outW;
        const sr_fixed_type yf = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        uint_fast16_t srcY = (uint_fast16_t)ls::math::fixed_cast<sr_fixed_type::base_type, sr_fixed_type::fraction_digits>(yf);
        srcY = ls::math::min<uint_fast16_t>(srcY, inH - 1);

        for (uint_fast16_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf = ls::math::fixed_cast<sr_fixed_type>(x);
            uint_fast16_t srcX = (uint_fast16_t)ls::math::fixed_cast<sr_fixed_type::base_type, sr_fixed_type::fraction_digits>(xf * foutW);
            srcX = ls::math::min<uint_fast16_t>(srcX, inW - 1);

            // Copy 4 color components at once
            const color_type                   inR      = mTexture->texel<color_type>(srcX,   srcY);
            const SR_ColorRGBAType<color_type> inColor  {color_type{0}, color_type{0}, inR, color_type{1}}; // BGRA
            const uint_fast16_t                outIndex = x + yOffset;
            SR_ColorRGBA8&                     outColor = reinterpret_cast<SR_ColorRGBA8*>(pOutBuf)[numPixels - outIndex];

            outColor = color_cast<uint8_t, color_type>(inColor);
        }
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (R & G Channels)
-------------------------------------*/
template <typename color_type>
void SR_BlitProcessor::blit_nearest_rg(
    SR_ColorRGBA8* const pOutBuf,
    const uint_fast16_t  inW,
    const uint_fast16_t  inH,
    const uint_fast16_t  outW,
    const uint_fast16_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast16_t dstH      = outH / mNumThreads;
    const uint_fast16_t dstY0     = mThreadId * dstH;
    const uint_fast16_t dstY1     = dstY0 + dstH;
    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(outH);
    const uint_fast16_t numPixels = (outW*outH) - 1;

    for (uint_fast16_t y = dstY0; y < dstY1; ++y)
    {
        const uint_fast16_t yOffset = y * outW;
        const sr_fixed_type yf = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        uint_fast16_t srcY = (uint_fast16_t)ls::math::fixed_cast<sr_fixed_type::base_type, sr_fixed_type::fraction_digits>(yf);
        srcY = ls::math::min<uint_fast16_t>(srcY, inH - 1);

        for (uint_fast16_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf = ls::math::fixed_cast<sr_fixed_type>(x);
            uint_fast16_t srcX = (uint_fast16_t)ls::math::fixed_cast<sr_fixed_type::base_type, sr_fixed_type::fraction_digits>(xf * foutW);
            srcX = ls::math::min<uint_fast16_t>(srcX, inW - 1);

            // Copy 4 color components at once
            const SR_ColorRGType<color_type>   inRG     = mTexture->texel<SR_ColorRGType<color_type>>(srcX,   srcY);
            const SR_ColorRGBAType<color_type> inColor  {color_type{0}, inRG[1], inRG[0], color_type{1}}; // BGRA
            const uint_fast16_t                outIndex = x + yOffset;

            pOutBuf[numPixels - outIndex] = color_cast<uint8_t, color_type>(inColor);
        }
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGB)
-------------------------------------*/
template <typename color_type>
void SR_BlitProcessor::blit_nearest_rgb(
    SR_ColorRGBA8* const pOutBuf,
    const uint_fast16_t  inW,
    const uint_fast16_t  inH,
    const uint_fast16_t  outW,
    const uint_fast16_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast16_t dstH      = outH / mNumThreads;
    const uint_fast16_t dstY0     = mThreadId * dstH;
    const uint_fast16_t dstY1     = dstY0 + dstH;
    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(outH);
    const uint_fast16_t numPixels = (outW*outH) - 1;

    for (uint_fast16_t y = dstY0; y < dstY1; ++y)
    {
        const uint_fast16_t yOffset = y * outW;
        const sr_fixed_type yf = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        uint_fast16_t srcY = (uint_fast16_t)ls::math::fixed_cast<sr_fixed_type::base_type, sr_fixed_type::fraction_digits>(yf);
        srcY = ls::math::min<uint_fast16_t>(srcY, inH - 1);

        for (uint_fast16_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf = ls::math::fixed_cast<sr_fixed_type>(x);
            uint_fast16_t srcX = (uint_fast16_t)ls::math::fixed_cast<sr_fixed_type::base_type, sr_fixed_type::fraction_digits>(xf * foutW);
            srcX = ls::math::min<uint_fast16_t>(srcX, inW - 1);

            // Copy 4 color components
            const SR_ColorRGBType<color_type>  inRGB   = mTexture->texel<SR_ColorRGBType<color_type>>(srcX, srcY);
            const SR_ColorRGBAType<color_type> inColor {inRGB[0], inRGB[1], inRGB[2], color_type{1}};
            const uint_fast16_t                outIndex = x + yOffset;

            pOutBuf[numPixels - outIndex] = color_cast<uint8_t, color_type>(inColor);
        }
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template <typename color_type>
void SR_BlitProcessor::blit_nearest_rgba(
    SR_ColorRGBA8* const pOutBuf,
    const uint_fast16_t  inW,
    const uint_fast16_t  inH,
    const uint_fast16_t  outW,
    const uint_fast16_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast16_t dstH      = outH / mNumThreads;
    const uint_fast16_t dstY0     = mThreadId * dstH;
    const uint_fast16_t dstY1     = dstY0 + dstH;
    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(outH);
    const uint_fast16_t numPixels = (outW*outH) - 1;

    for (uint_fast16_t y = dstY0; y < dstY1; ++y)
    {
        const uint_fast16_t yOffset = y * outW;
        const sr_fixed_type yf = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        uint_fast16_t srcY = (uint_fast16_t)ls::math::fixed_cast<sr_fixed_type::base_type, sr_fixed_type::fraction_digits>(yf);
        srcY = ls::math::min<uint_fast16_t>(srcY, inH - 1);

        for (uint_fast16_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf = ls::math::fixed_cast<sr_fixed_type>(x);
            uint_fast16_t srcX = (uint_fast16_t)ls::math::fixed_cast<sr_fixed_type::base_type, sr_fixed_type::fraction_digits>(xf * foutW);
            srcX = ls::math::min<uint_fast16_t>(srcX, inW - 1);

            const SR_ColorRGBAType<color_type> inColor  = mTexture->texel<SR_ColorRGBAType<color_type>>(srcX, srcY);
            const uint_fast16_t                outIndex = x + yOffset;

            pOutBuf[numPixels - outIndex] = color_cast<uint8_t, color_type>(inColor);
        }
    }
}



#endif /* SR_BLIT_PROCESOR_HPP */
