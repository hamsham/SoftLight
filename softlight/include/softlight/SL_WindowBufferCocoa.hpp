
#ifndef SL_WINDOW_BUFFER_COCOA_HPP
#define SL_WINDOW_BUFFER_COCOA_HPP

#include "softlight/SL_WindowBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace math
{
template <typename color_type>
union vec4_t;
}
}



/*-----------------------------------------------------------------------------
 * Cocoa Window Backbuffer
-----------------------------------------------------------------------------*/
class SL_WindowBufferCocoa : public SL_WindowBuffer
{
    friend class SL_RenderWindowCocoa;

  private:
    void* mImageProvider; // CGDataProviderRef

    void* mColorSpace; // CGColorSpaceRef

    //void* mImageRef; // CGImageRef

  public:
    virtual ~SL_WindowBufferCocoa() noexcept override;

    SL_WindowBufferCocoa() noexcept;

    SL_WindowBufferCocoa(const SL_WindowBufferCocoa&) = delete;

    SL_WindowBufferCocoa(SL_WindowBufferCocoa&&) noexcept;

    SL_WindowBufferCocoa& operator=(const SL_WindowBufferCocoa&) = delete;

    SL_WindowBufferCocoa& operator=(SL_WindowBufferCocoa&&) noexcept;

    int init(SL_RenderWindow& win, unsigned width, unsigned height) noexcept override;

    int terminate() noexcept override;

    unsigned width() const noexcept override;

    unsigned height() const noexcept override;

    const void* native_handle() const noexcept override;

    void* native_handle() noexcept override;

    const ls::math::vec4_t<uint8_t>* buffer() const noexcept override;

    ls::math::vec4_t<uint8_t>* buffer() noexcept override;
};



/*-------------------------------------
 * Get the backbuffer width
-------------------------------------*/
inline unsigned SL_WindowBufferCocoa::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SL_WindowBufferCocoa::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline const void* SL_WindowBufferCocoa::native_handle() const noexcept
{
    //return mImageRef;
    return mImageProvider;
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline void* SL_WindowBufferCocoa::native_handle() noexcept
{
    //return mImageRef;
    return mImageProvider;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SL_WindowBufferCocoa::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SL_WindowBufferCocoa::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SL_WINDOW_BUFFER_COCOA_HPP */
