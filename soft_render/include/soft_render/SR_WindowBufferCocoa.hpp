
#ifndef SR_WINDOW_BUFFER_COCOA_HPP
#define SR_WINDOW_BUFFER_COCOA_HPP

#include "soft_render/SR_WindowBuffer.hpp"



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
class SR_WindowBufferCocoa : public SR_WindowBuffer
{
    friend class SR_RenderWindowCocoa;

  private:
    void* mImageProvider; // CGDataProviderRef

    void* mColorSpace; // CGColorSpaceRef

    //void* mImageRef; // CGImageRef

  public:
    virtual ~SR_WindowBufferCocoa() noexcept override;

    SR_WindowBufferCocoa() noexcept;

    SR_WindowBufferCocoa(const SR_WindowBufferCocoa&) = delete;

    SR_WindowBufferCocoa(SR_WindowBufferCocoa&&) noexcept;

    SR_WindowBufferCocoa& operator=(const SR_WindowBufferCocoa&) = delete;

    SR_WindowBufferCocoa& operator=(SR_WindowBufferCocoa&&) noexcept;

    int init(SR_RenderWindow& win, unsigned width, unsigned height) noexcept override;

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
inline unsigned SR_WindowBufferCocoa::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SR_WindowBufferCocoa::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline const void* SR_WindowBufferCocoa::native_handle() const noexcept
{
    //return mImageRef;
    return mImageProvider;
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline void* SR_WindowBufferCocoa::native_handle() noexcept
{
    //return mImageRef;
    return mImageProvider;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SR_WindowBufferCocoa::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SR_WindowBufferCocoa::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SR_WINDOW_BUFFER_COCOA_HPP */
