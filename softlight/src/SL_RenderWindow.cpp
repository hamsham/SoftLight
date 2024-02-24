
#include <string>

#include "lightsky/setup/OS.h" // OS detection

#if defined(SL_HAVE_WIN32_BACKEND)
    #include "softlight/SL_RenderWindowWin32.hpp"
#endif

#if defined(SL_HAVE_COCOA_BACKEND)
        #include "softlight/SL_RenderWindowCocoa.hpp"
#endif

#if defined(SL_HAVE_X11_BACKEND)
    #include "softlight/SL_RenderWindowXlib.hpp"
#endif

#if defined(SL_HAVE_XCB_BACKEND)
    #include "softlight/SL_RenderWindowXCB.hpp"
#endif



/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_RenderWindow::~SL_RenderWindow() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_RenderWindow::SL_RenderWindow() noexcept :
  mCurrentState{WindowStateInfo::WINDOW_CLOSED}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_RenderWindow::SL_RenderWindow(const SL_RenderWindow& rw) noexcept :
  mCurrentState{rw.mCurrentState}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_RenderWindow::SL_RenderWindow(SL_RenderWindow&& rw) noexcept :
  mCurrentState{rw.mCurrentState}
{
  rw.mCurrentState = WindowStateInfo::WINDOW_CLOSED;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_RenderWindow& SL_RenderWindow::operator=(const SL_RenderWindow& rw) noexcept
{
  mCurrentState = rw.mCurrentState;
  return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_RenderWindow& SL_RenderWindow::operator=(SL_RenderWindow&& rw) noexcept
{
  mCurrentState = rw.mCurrentState;
  rw.mCurrentState = WindowStateInfo::WINDOW_CLOSED;
  return *this;
}



/*-------------------------------------
 * Instance Creation
-------------------------------------*/
ls::utils::Pointer<SL_RenderWindow> SL_RenderWindow::create() noexcept
{
    #if defined(SL_HAVE_WIN32_BACKEND)
        return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowWin32{}};
    #elif defined(SL_PREFER_COCOA) && defined(SL_HAVE_COCOA_BACKEND)
        return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowCocoa{}};
    #elif defined(SL_PREFER_XCB) && defined(SL_HAVE_XCB_BACKEND)
            return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowXCB{}};
    #elif defined(SL_HAVE_X11_BACKEND)
        return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowXlib{}};
    #else
        #error "Window buffer backend not implemented for this platform."
    #endif
}



/*-------------------------------------
 * Instance Creation (with user-defined hinting)
-------------------------------------*/
ls::utils::Pointer<SL_RenderWindow> SL_RenderWindow::create(SL_WindowBackend backendHint) noexcept
{
    switch (backendHint)
    {
        case SL_WindowBackend::WIN32_BACKEND:
            #if defined(SL_HAVE_WIN32_BACKEND)
                return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowWin32{}};
            #else
                break;
            #endif

        case SL_WindowBackend::COCOA_BACKEND:
            #if defined(SL_HAVE_COCOA_BACKEND)
                return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowCocoa{}};
            #else
                break;
            #endif

        case SL_WindowBackend::XCB_BACKEND:
            #if defined(SL_HAVE_XCB_BACKEND)
                return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowXCB{}};
            #else
                break;
            #endif

        case SL_WindowBackend::X11_BACKEND:
            #if defined(SL_HAVE_X11_BACKEND)
                return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowXlib{}};
            #else
                break;
            #endif

        default:
            break;
    }

    return SL_RenderWindow::create();
}
