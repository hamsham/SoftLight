
#ifndef SL_SWAPCHAIN_WIN32_HPP
#define SL_SWAPCHAIN_WIN32_HPP

#include "softlight/SL_Swapchain.hpp"



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
class SL_SwapchainWin32 : public SL_Swapchain
{
    friend class SL_RenderWindowWin32;

  private:
    void* mBitmapInfo; // PBITMAPINFO

  public:
    virtual ~SL_SwapchainWin32() noexcept override;

    SL_SwapchainWin32() noexcept;

    SL_SwapchainWin32(const SL_SwapchainWin32&) = delete;

    SL_SwapchainWin32(SL_SwapchainWin32&&) noexcept;

    SL_SwapchainWin32& operator=(const SL_SwapchainWin32&) = delete;

    SL_SwapchainWin32& operator=(SL_SwapchainWin32&&) noexcept;

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
inline unsigned SL_SwapchainWin32::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SL_SwapchainWin32::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline const void* SL_SwapchainWin32::native_handle() const noexcept
{
    return mBitmapInfo;
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline void* SL_SwapchainWin32::native_handle() noexcept
{
    return mBitmapInfo;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SL_SwapchainWin32::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SL_SwapchainWin32::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SL_SWAPCHAIN_WIN32_HPP */
