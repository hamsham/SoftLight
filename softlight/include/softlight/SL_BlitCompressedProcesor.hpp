
#ifndef SL_BLIT_COMPRESSED_PROCESSOR_HPP
#define SL_BLIT_COMPRESSED_PROCESSOR_HPP

#include <cstdint>



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
struct SL_TextureView;



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
struct SL_BlitCompressedProcessor
{
    enum : uint_fast32_t
    {
        NUM_FIXED_BITS = 16u
    };

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
    const SL_TextureView* mSrcTex;
    SL_TextureView* mDstTex;

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

    // Blit compressed color components
    template<typename inColor_type>
    void blit_src_compressed() noexcept;

    // Blit all 4 color components
    template<class BlitOp>
    void blit_nearest() noexcept;

    void execute() noexcept;
};



#endif /* SL_BLIT_COMPRESSED_PROCESSOR_HPP */
