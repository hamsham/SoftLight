
#ifndef SL_SWAPCHAIN_COCOA_HPP
#define SL_SWAPCHAIN_COCOA_HPP

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
 * Cocoa Window Backbuffer
-----------------------------------------------------------------------------*/
class SL_SwapchainCocoa : public SL_Swapchain
{
    friend class SL_RenderWindowCocoa;

  private:
    void* mImageProvider; // CGDataProviderRef

    void* mColorSpace; // CGColorSpaceRef

    //void* mImageRef; // CGImageRef

  public:
    virtual ~SL_SwapchainCocoa() noexcept override;

    SL_SwapchainCocoa() noexcept;

    SL_SwapchainCocoa(const SL_SwapchainCocoa&) = delete;

    SL_SwapchainCocoa(SL_SwapchainCocoa&&) noexcept;

    SL_SwapchainCocoa& operator=(const SL_SwapchainCocoa&) = delete;

    SL_SwapchainCocoa& operator=(SL_SwapchainCocoa&&) noexcept;

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
inline unsigned SL_SwapchainCocoa::width() const noexcept
{
    return mTexture.width();
}



/*-------------------------------------
 * Get the backbuffer height
-------------------------------------*/
inline unsigned SL_SwapchainCocoa::height() const noexcept
{
    return mTexture.height();
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline const void* SL_SwapchainCocoa::native_handle() const noexcept
{
    //return mImageRef;
    return mImageProvider;
}



/*-------------------------------------
 * HDC
-------------------------------------*/
inline void* SL_SwapchainCocoa::native_handle() noexcept
{
    //return mImageRef;
    return mImageProvider;
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline const ls::math::vec4_t<uint8_t>* SL_SwapchainCocoa::buffer() const noexcept
{
    return reinterpret_cast<const ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



/*-------------------------------------
 * Retrieve the raw data within the backbuffer
-------------------------------------*/
inline ls::math::vec4_t<uint8_t>* SL_SwapchainCocoa::buffer() noexcept
{
    return reinterpret_cast<ls::math::vec4_t<uint8_t>*>(mTexture.data());
}



#endif /* SL_SWAPCHAIN_COCOA_HPP */
