
#ifndef SR_WINDOW_BUFFER_XCB_HPP
#define SR_WINDOW_BUFFER_XCB_HPP

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
class SR_WindowBufferXCB : public SR_WindowBuffer
{
  friend class SR_RenderWindowXCB;

  private:
    SR_RenderWindow* mWindow;

    #if SR_ENABLE_XSHM != 0
    void* mShmInfo;
    #endif

  public:
    virtual ~SR_WindowBufferXCB() noexcept override;

    SR_WindowBufferXCB() noexcept;

    SR_WindowBufferXCB(const SR_WindowBufferXCB&) = delete;

    SR_WindowBufferXCB(SR_WindowBufferXCB&&) noexcept;

    SR_WindowBufferXCB& operator=(const SR_WindowBufferXCB&) = delete;

    SR_WindowBufferXCB& operator=(SR_WindowBufferXCB&&) noexcept;

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
inline unsigned SR_WindowBufferXCB::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SR_WindowBufferXCB::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * Native Handle
-------------------------------------*/
inline const void* SR_WindowBufferXCB::native_handle() const noexcept
{
    return &mTexture;
}



/*-------------------------------------
 * Native Handle
-------------------------------------*/
inline void* SR_WindowBufferXCB::native_handle() noexcept
{
    return &mTexture;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SR_WindowBufferXCB::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SR_WindowBufferXCB::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SR_WINDOW_BUFFER_XCB_HPP */
