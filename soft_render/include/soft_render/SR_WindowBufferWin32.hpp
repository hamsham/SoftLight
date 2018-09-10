
#ifndef SR_WINDOW_BUFFER_WIN32_HPP
#define SR_WINDOW_BUFFER_WIN32_HPP

#include "soft_render/SR_WindowBuffer.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SR_WindowBufferWin32 : public SR_WindowBuffer
{
    friend class SR_RenderWindowWin32;

  private:
    void* mBitmapInfo; // PBITMAPINFO

    SR_ColorRGBAType<uint8_t>* mBuffer; // SR_ColorRGBA8[]

    unsigned mWidth;

    unsigned mHeight;

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

    const SR_ColorRGBAType<uint8_t>* buffer() const noexcept override;

    SR_ColorRGBAType<uint8_t>* buffer() noexcept override;
};



/*-------------------------------------
 *
-------------------------------------*/
inline unsigned SR_WindowBufferWin32::width() const noexcept
{
    return mWidth;
}



/*-------------------------------------
 *
-------------------------------------*/
inline unsigned SR_WindowBufferWin32::height() const noexcept
{
    return mHeight;
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
 *
-------------------------------------*/
inline const SR_ColorRGBAType<uint8_t>* SR_WindowBufferWin32::buffer() const noexcept
{
    return mBuffer;
}



/*-------------------------------------
 *
-------------------------------------*/
inline SR_ColorRGBAType<uint8_t>* SR_WindowBufferWin32::buffer() noexcept
{
    return mBuffer;
}



#endif /* SR_WINDOW_BUFFER_WIN32_HPP */
