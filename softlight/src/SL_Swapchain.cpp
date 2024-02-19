
#include <utility> // std::move()

#if defined(SL_HAVE_WIN32_BACKEND)
    #include "softlight/SL_SwapchainWin32.hpp"
#endif

#if defined(SL_HAVE_COCOA_BACKEND)
    #include "softlight/SL_SwapchainCocoa.hpp"
#endif

#if defined(SL_HAVE_X11_BACKEND)
    #include "softlight/SL_SwapchainXlib.hpp"
#endif

#if defined(SL_HAVE_XCB_BACKEND)
    #include "softlight/SL_SwapchainXCB.hpp"
#endif

#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Swapchain.hpp"



/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_Swapchain::~SL_Swapchain() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_Swapchain::SL_Swapchain() noexcept
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_Swapchain::SL_Swapchain(SL_Swapchain&& wb) noexcept :
    mTexture{std::move(wb.mTexture)}
{}



/*-------------------------------------
 * Move another window buffer into *this.
-------------------------------------*/
SL_Swapchain& SL_Swapchain::operator=(SL_Swapchain&& wb) noexcept
{
    mTexture = std::move(wb.mTexture);
    return *this;
}



/*-------------------------------------
 * Instance Creation
-------------------------------------*/
ls::utils::Pointer<SL_Swapchain> SL_Swapchain::create(SL_WindowBackend backend) noexcept
{
    switch (backend)
    {
        #if defined(SL_HAVE_WIN32_BACKEND)
            case SL_WindowBackend::WIN32_BACKEND: return ls::utils::Pointer<SL_Swapchain>{new SL_SwapchainWin32{}};
        #endif

        #if defined(SL_HAVE_COCOA_BACKEND)
            case SL_WindowBackend::COCOA_BACKEND: return ls::utils::Pointer<SL_Swapchain>{new SL_SwapchainCocoa{}};
        #endif

        #if defined(SL_HAVE_XCB_BACKEND)
            case SL_WindowBackend::XCB_BACKEND: return ls::utils::Pointer<SL_Swapchain>{new SL_SwapchainXCB{}};
        #endif

        #if defined(SL_HAVE_XCB_BACKEND)
            case SL_WindowBackend::X11_BACKEND: return ls::utils::Pointer<SL_Swapchain>{new SL_SwapchainXlib{}};
        #endif

        default:
            break;
    }

    return ls::utils::Pointer<SL_Swapchain>{nullptr};
}
