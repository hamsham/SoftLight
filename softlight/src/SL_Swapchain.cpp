
#include <utility> // std::move()

#include "lightsky/setup/OS.h" // OS detection

#if defined(LS_OS_WINDOWS)
    #include "softlight/SL_SwapchainWin32.hpp"
#elif defined(SL_PREFER_COCOA)
    #include "softlight/SL_SwapchainCocoa.hpp"
#else
    #include "softlight/SL_SwapchainXCB.hpp"
    #include "softlight/SL_SwapchainXlib.hpp"
#endif

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
ls::utils::Pointer<SL_Swapchain> SL_Swapchain::create() noexcept
{
    #ifdef LS_OS_WINDOWS
        return ls::utils::Pointer<SL_Swapchain>{new SL_SwapchainWin32{}};
    #elif defined(SL_PREFER_COCOA)
            return ls::utils::Pointer<SL_Swapchain>{new SL_SwapchainCocoa{}};
    #elif defined(LS_OS_UNIX)
        #if defined(SL_PREFER_XCB)
            return ls::utils::Pointer<SL_Swapchain>{new SL_SwapchainXCB{}};
        #else
            return ls::utils::Pointer<SL_Swapchain>{new SL_SwapchainXlib{}};
        #endif
    #else
        #error "Window buffer backend not implemented for this platform."
    #endif
}
