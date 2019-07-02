
#ifndef SR_WINDOW_BUFFER_WIN32_HPP
#define SR_WINDOW_BUFFER_WIN32_HPP

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
 * Win32 Window Backbuffer
-----------------------------------------------------------------------------*/
class SR_WindowBufferWin32 : public SR_WindowBuffer
{
    friend class SR_RenderWindowWin32;

  private:
    void* mBitmapInfo; // PBITMAPINFO

  public:
    virtual ~SR_WindowBufferWin32() noexcept override;

    SR_WindowBufferWin32() noexcept;

    SR_WindowBufferWin32(const SR_WindowBufferWin32&) = delete;

    SR_WindowBufferWin32(SR_WindowBufferWin32&&) noexcept;

    SR_WindowBufferWin32& operator=(const SR_WindowBufferWin32&) = delete;

    SR_WindowBufferWin32& operator=(SR_WindowBufferWin32&&) noexcept;

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
inline unsigned SR_WindowBufferWin32::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SR_WindowBufferWin32::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline const void* SR_WindowBufferWin32::native_handle() const noexcept
{
    return mBitmapInfo;
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline void* SR_WindowBufferWin32::native_handle() noexcept
{
    return mBitmapInfo;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SR_WindowBufferWin32::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SR_WindowBufferWin32::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SR_WINDOW_BUFFER_WIN32_HPP */
