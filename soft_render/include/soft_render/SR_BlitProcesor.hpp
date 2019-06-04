
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

    // 64-128 bits
    const SR_Texture* mTexture;
    SR_WindowBuffer* mBackBuffer;

    // 160 bits total, 20 bytes

    // Blit a single R channel
    template <typename color_type>
    void blit_nearest_r(
        unsigned char* const pOutBuf,
        const uint_fast32_t  inW,
        const uint_fast32_t  inH,
        const uint_fast32_t  outW,
        const uint_fast32_t  outH
    ) noexcept;

    // Blit a texture with only RG color channels
    template <typename color_type>
    void blit_nearest_rg(
        unsigned char* const pOutBuf,
        const uint_fast32_t  inW,
        const uint_fast32_t  inH,
        const uint_fast32_t  outW,
        const uint_fast32_t  outH
    ) noexcept;

    // Blit an RGB texture
    template <typename color_type>
    void blit_nearest_rgb(
        unsigned char* const pOutBuf,
        const uint_fast32_t  inW,
        const uint_fast32_t  inH,
        const uint_fast32_t  outW,
        const uint_fast32_t  outH
    ) noexcept;

    // Blit all 4 color components
    template <typename color_type>
    void blit_nearest_rgba(
        SR_ColorRGBA8* const pOutBuf,
        const uint_fast32_t  inW,
        const uint_fast32_t  inH,
        const uint_fast32_t  outW,
        const uint_fast32_t  outH
    ) noexcept;

    void execute() noexcept;
};




/*-------------------------------------
 * Nearest-neighbor filtering (R Channel)
-------------------------------------*/
template <typename color_type>
void SR_BlitProcessor::blit_nearest_r(
    unsigned char* const pOutBuf,
    const uint_fast32_t  inW,
    const uint_fast32_t  inH,
    const uint_fast32_t  outW,
    const uint_fast32_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    constexpr ptrdiff_t stride    = sizeof(SR_ColorRGBA8);
    const uint_fast32_t dstH      = outH / mNumThreads;
    const uint_fast32_t dstY0     = mThreadId * dstH;
    const uint_fast32_t dstY1     = dstY0 + dstH;
    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(outH);
    const uint_fast32_t numPixels = (outW*outH) - 1;

    for (uint_fast32_t y = dstY0; y < dstY1; ++y)
    {
        const uint_fast32_t yOffset = y * outW;
        const sr_fixed_type yf = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        uint_fast32_t srcY = ls::math::integer_cast<uint_fast32_t>(yf);
        srcY = ls::math::min<uint_fast32_t>(srcY, inH - 1);

        for (uint_fast32_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf = ls::math::fixed_cast<sr_fixed_type>(x) * foutW;
            uint_fast32_t srcX = ls::math::integer_cast<uint_fast32_t>(xf);
            srcX = ls::math::min<uint_fast32_t>(srcX, inW - 1);

            // Copy 4 color components at once
            const color_type                   inR      = mTexture->texel<color_type>(srcX,   srcY);
            const SR_ColorRGBAType<color_type> inColor  {color_type{0}, color_type{0}, inR, color_type{1}}; // BGRA
            const uint_fast32_t                outIndex = x + yOffset;

            *reinterpret_cast<SR_ColorRGBA8*>(pOutBuf + (numPixels - outIndex) * stride) = color_cast<uint8_t, color_type>(inColor);
        }
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (R & G Channels)
-------------------------------------*/
template <typename color_type>
void SR_BlitProcessor::blit_nearest_rg(
    unsigned char* const pOutBuf,
    const uint_fast32_t  inW,
    const uint_fast32_t  inH,
    const uint_fast32_t  outW,
    const uint_fast32_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    constexpr ptrdiff_t stride    = sizeof(SR_ColorRGBA8);
    const uint_fast32_t dstH      = outH / mNumThreads;
    const uint_fast32_t dstY0     = mThreadId * dstH;
    const uint_fast32_t dstY1     = dstY0 + dstH;
    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(outH);
    const uint_fast32_t numPixels = (outW*outH) - 1;

    for (uint_fast32_t y = dstY0; y < dstY1; ++y)
    {
        const uint_fast32_t yOffset = y * outW;
        const sr_fixed_type yf = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        uint_fast32_t srcY = ls::math::integer_cast<uint_fast32_t>(yf);
        srcY = ls::math::min<uint_fast32_t>(srcY, inH - 1);

        for (uint_fast32_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf = ls::math::fixed_cast<sr_fixed_type>(x) * foutW;
            uint_fast32_t srcX = ls::math::integer_cast<uint_fast32_t>(xf);
            srcX = ls::math::min<uint_fast32_t>(srcX, inW - 1);

            // Copy 4 color components at once
            const SR_ColorRGType<color_type>   inRG     = mTexture->texel<SR_ColorRGType<color_type>>(srcX,   srcY);
            const SR_ColorRGBAType<color_type> inColor  {color_type{0}, inRG[1], inRG[0], color_type{1}}; // BGRA
            const uint_fast32_t                outIndex = x + yOffset;

            *reinterpret_cast<SR_ColorRGBA8*>(pOutBuf + (numPixels - outIndex) * stride) = color_cast<uint8_t, color_type>(inColor);
        }
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGB)
-------------------------------------*/
template <typename color_type>
void SR_BlitProcessor::blit_nearest_rgb(
    unsigned char* const pOutBuf,
    const uint_fast32_t  inW,
    const uint_fast32_t  inH,
    const uint_fast32_t  outW,
    const uint_fast32_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    constexpr ptrdiff_t stride    = sizeof(SR_ColorRGBA8);
    const uint_fast32_t dstH      = outH / mNumThreads;
    const uint_fast32_t dstY0     = mThreadId * dstH;
    const uint_fast32_t dstY1     = dstY0 + dstH;
    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(outH);
    const uint_fast32_t numPixels = (outW*outH) - 1;

    for (uint_fast32_t y = dstY0; y < dstY1; ++y)
    {
        const uint_fast32_t yOffset = y * outW;
        const sr_fixed_type yf = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        uint_fast32_t srcY = ls::math::integer_cast<uint_fast32_t>(yf);
        srcY = ls::math::min<uint_fast32_t>(srcY, inH - 1);

        for (uint_fast32_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf = ls::math::fixed_cast<sr_fixed_type>(x) * foutW;
            uint_fast32_t srcX = ls::math::integer_cast<uint_fast32_t>(xf);
            srcX = ls::math::min<uint_fast32_t>(srcX, inW - 1);

            // Copy 4 color components
            const SR_ColorRGBType<color_type> inColor  = mTexture->texel<SR_ColorRGBType<color_type>>(srcX, srcY);
            const uint_fast32_t               outIndex = x + yOffset;

            *reinterpret_cast<SR_ColorRGB8*>(pOutBuf + (numPixels - outIndex) * stride) = color_cast<uint8_t, color_type>(inColor);
        }
    }
}



/*-------------------------------------
 * Nearest-neighbor filtering (RGBA)
-------------------------------------*/
template <typename color_type>
void SR_BlitProcessor::blit_nearest_rgba(
    SR_ColorRGBA8* const pOutBuf,
    const uint_fast32_t  inW,
    const uint_fast32_t  inH,
    const uint_fast32_t  outW,
    const uint_fast32_t  outH) noexcept
{
    // Only tile data along the y-axis of the render buffer. This will help to
    // make use of the CPU prefetcher when iterating pixels along the x-axis
    const uint_fast32_t dstH      = outH / mNumThreads;
    const uint_fast32_t dstY0     = mThreadId * dstH;
    const uint_fast32_t dstY1     = dstY0 + dstH;
    const sr_fixed_type finW      = ls::math::fixed_cast<sr_fixed_type>(inW);
    const sr_fixed_type finH      = ls::math::fixed_cast<sr_fixed_type>(inH);
    const sr_fixed_type foutW     = finW / ls::math::fixed_cast<sr_fixed_type>(outW);
    const sr_fixed_type foutH     = finH / ls::math::fixed_cast<sr_fixed_type>(outH);
    const uint_fast32_t numPixels = (outW*outH) - 1;

    for (uint_fast32_t y = dstY0; y < dstY1; ++y)
    {
        const uint_fast32_t yOffset = y * outW;
        const sr_fixed_type yf = ls::math::fixed_cast<sr_fixed_type>(y) * foutH;
        uint_fast32_t srcY = ls::math::integer_cast<uint_fast32_t>(yf);
        srcY = ls::math::min<uint_fast32_t>(srcY, inH - 1);

        for (uint_fast32_t x = 0; x < outW; ++x)
        {
            const sr_fixed_type xf = ls::math::fixed_cast<sr_fixed_type>(x) * foutW;
            uint_fast32_t srcX = ls::math::integer_cast<uint_fast32_t>(xf);
            srcX = ls::math::min<uint_fast32_t>(srcX, inW - 1);

            const SR_ColorRGBAType<color_type> inColor  = mTexture->texel<SR_ColorRGBAType<color_type>>(srcX, srcY);
            const uint_fast32_t                outIndex = x + yOffset;

            pOutBuf[numPixels - outIndex] = color_cast<uint8_t, color_type>(inColor);
        }
    }
}



/*-------------------------------------
 * External function declarations to keep compile times short
-------------------------------------*/
#ifndef LS_COMPILER_MSC
    extern template void SR_BlitProcessor::blit_nearest_r<uint8_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_r<uint16_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_r<uint32_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_r<uint64_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_r<float>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_r<double>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);

    extern template void SR_BlitProcessor::blit_nearest_rg<uint8_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rg<uint16_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rg<uint32_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rg<uint64_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rg<float>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rg<double>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);

    extern template void SR_BlitProcessor::blit_nearest_rgb<uint8_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgb<uint16_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgb<uint32_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgb<uint64_t>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgb<float>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgb<double>(unsigned char* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);

    extern template void SR_BlitProcessor::blit_nearest_rgba<uint8_t>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgba<uint16_t>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgba<uint32_t>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgba<uint64_t>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgba<float>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
    extern template void SR_BlitProcessor::blit_nearest_rgba<double>(SR_ColorRGBA8* const, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t, const uint_fast32_t);
#endif /* LS_COMPILER_MSC */



#endif /* SR_BLIT_PROCESOR_HPP */
