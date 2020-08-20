
#ifndef SL_WINDOW_BUFFER_WIN32_HPP
#define SL_WINDOW_BUFFER_WIN32_HPP

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
 * Win32 Window Backbuffer
-----------------------------------------------------------------------------*/
class SL_WindowBufferWin32 : public SL_WindowBuffer
{
    friend class SL_RenderWindowWin32;

  private:
    void* mBitmapInfo; // PBITMAPINFO

  public:
    virtual ~SL_WindowBufferWin32() noexcept override;

    SL_WindowBufferWin32() noexcept;

    SL_WindowBufferWin32(const SL_WindowBufferWin32&) = delete;

    SL_WindowBufferWin32(SL_WindowBufferWin32&&) noexcept;

    SL_WindowBufferWin32& operator=(const SL_WindowBufferWin32&) = delete;

    SL_WindowBufferWin32& operator=(SL_WindowBufferWin32&&) noexcept;

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
inline unsigned SL_WindowBufferWin32::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SL_WindowBufferWin32::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline const void* SL_WindowBufferWin32::native_handle() const noexcept
{
    return mBitmapInfo;
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline void* SL_WindowBufferWin32::native_handle() noexcept
{
    return mBitmapInfo;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SL_WindowBufferWin32::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SL_WindowBufferWin32::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SL_WINDOW_BUFFER_WIN32_HPP */
