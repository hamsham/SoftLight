
#ifndef SR_WINDOW_BUFFER_XLIB_HPP
#define SR_WINDOW_BUFFER_XLIB_HPP

#include "soft_render/SR_WindowBuffer.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SR_WindowBufferXlib : public SR_WindowBuffer
{
  private:
    SR_RenderWindow* mWindow;

    void* mBuffer;

    void* mShmInfo;

  public:
    virtual ~SR_WindowBufferXlib() noexcept override;

    SR_WindowBufferXlib() noexcept;

    SR_WindowBufferXlib(const SR_WindowBufferXlib&) = delete;

    SR_WindowBufferXlib(SR_WindowBufferXlib&&) noexcept;

    SR_WindowBufferXlib& operator=(const SR_WindowBufferXlib&) = delete;

    SR_WindowBufferXlib& operator=(SR_WindowBufferXlib&&) noexcept;

    virtual int init(SR_RenderWindow& win, unsigned width, unsigned height) noexcept override;

    virtual int terminate() noexcept override;

    virtual unsigned width() const noexcept override;

    virtual unsigned height() const noexcept override;

    virtual const void* native_handle() const noexcept override;

    virtual void* native_handle() noexcept override;

    virtual const SR_ColorRGBAType<uint8_t>* buffer() const noexcept override;

    virtual SR_ColorRGBAType<uint8_t>* buffer() noexcept override;
};



#endif /* SR_WINDOW_BUFFER_XLIB_HPP */
