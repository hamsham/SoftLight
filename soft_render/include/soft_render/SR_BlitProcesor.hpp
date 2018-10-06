
#ifndef SR_BLIT_PROCESOR_HPP
#define SR_BLIT_PROCESOR_HPP

#include <cstdint>



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
template<typename color_t>
struct SR_ColorRGBType;

template<typename color_t>
struct SR_ColorRGBAType;

class SR_Texture;
class SR_WindowBuffer;


/*-----------------------------------------------------------------------------
 * Encapsulation of texture blitting on another thread.
-----------------------------------------------------------------------------*/
struct SR_BlitProcessor
{
    // 32 bits
    uint16_t mThreadId;
    uint16_t mNumThreads;

    // 64-128 bits
    const SR_Texture* mTexture;
    SR_WindowBuffer* mBackBuffer;

    // 160 bits total, 20 bytes

    void blit_nearest(
        unsigned char* const pOutBuf,
        const uint_fast16_t  inW,
        const uint_fast16_t  inH,
        const uint_fast16_t  outW,
        const uint_fast16_t  outH
    ) noexcept;

    void execute() noexcept;
};


#endif /* SR_BLIT_PROCESOR_HPP */
