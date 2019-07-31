
#ifndef SR_WINDOW_BUFFER_XLIB_HPP
#define SR_WINDOW_BUFFER_XLIB_HPP

#include "soft_render/SR_WindowBuffer.hpp"



// Should be defined by the build system
// OSX with XQuartz runs out of memory when attaching textures to shared
// memory segments.
#ifndef SR_ENABLE_XSHM
    #define SR_ENABLE_XSHM 0
#endif /* SR_ENABLE_XSHM */



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
 * Xlib Render Window
-----------------------------------------------------------------------------*/
class SR_WindowBufferXlib : public SR_WindowBuffer
{
  private:
    SR_RenderWindow* mWindow;

    void* mBuffer;

    #if SR_ENABLE_XSHM != 0
    void* mShmInfo;
    #endif

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

    virtual const ls::math::vec4_t<uint8_t>* buffer() const noexcept override;

    virtual ls::math::vec4_t<uint8_t>* buffer() noexcept override;
};



/*-------------------------------------
 * Get the backbuffer width
-------------------------------------*/
inline unsigned SR_WindowBufferXlib::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SR_WindowBufferXlib::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * Native Handle
-------------------------------------*/
inline const void* SR_WindowBufferXlib::native_handle() const noexcept
{
    return mBuffer;
}



/*-------------------------------------
 * Native Handle
-------------------------------------*/
inline void* SR_WindowBufferXlib::native_handle() noexcept
{
    return mBuffer;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SR_WindowBufferXlib::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SR_WindowBufferXlib::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SR_WINDOW_BUFFER_XLIB_HPP */
