
#ifndef SL_SWAPCHAIN_XCB_HPP
#define SL_SWAPCHAIN_XCB_HPP

#include "softlight/SL_Swapchain.hpp"



// Should be defined by the build system
// OSX with XQuartz runs out of memory when attaching textures to shared
// memory segments.
#ifndef SL_ENABLE_XSHM
    #define SL_ENABLE_XSHM 0
#endif /* SL_ENABLE_XSHM */



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
class SL_SwapchainXCB : public SL_Swapchain
{
  friend class SL_RenderWindowXCB;

  private:
    SL_RenderWindow* mWindow;

    #if SL_ENABLE_XSHM != 0
    void* mShmInfo;
    #endif

  public:
    virtual ~SL_SwapchainXCB() noexcept override;

    SL_SwapchainXCB() noexcept;

    SL_SwapchainXCB(const SL_SwapchainXCB&) = delete;

    SL_SwapchainXCB(SL_SwapchainXCB&&) noexcept;

    SL_SwapchainXCB& operator=(const SL_SwapchainXCB&) = delete;

    SL_SwapchainXCB& operator=(SL_SwapchainXCB&&) noexcept;

    virtual int init(SL_RenderWindow& win, unsigned width, unsigned height) noexcept override;

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
inline unsigned SL_SwapchainXCB::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SL_SwapchainXCB::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * Native Handle
-------------------------------------*/
inline const void* SL_SwapchainXCB::native_handle() const noexcept
{
    return &mTexture;
}



/*-------------------------------------
 * Native Handle
-------------------------------------*/
inline void* SL_SwapchainXCB::native_handle() noexcept
{
    return &mTexture;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SL_SwapchainXCB::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SL_SwapchainXCB::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SL_SWAPCHAIN_XCB_HPP */
