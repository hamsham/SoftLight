
#include <string>

#include "lightsky/setup/OS.h" // OS detection

#ifdef LS_OS_WINDOWS
    #include "soft_render/SR_RenderWindowWin32.hpp"
#elif defined(SR_PREFER_COCOA)
        #include "soft_render/SR_RenderWindowCocoa.hpp"
#else
    #include "soft_render/SR_RenderWindowXCB.hpp"
    #include "soft_render/SR_RenderWindowXlib.hpp"
#endif



/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_RenderWindow::~SR_RenderWindow() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_RenderWindow::SR_RenderWindow() noexcept :
  mCurrentState{WindowStateInfo::WINDOW_CLOSED}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SR_RenderWindow::SR_RenderWindow(const SR_RenderWindow& rw) noexcept :
  mCurrentState{rw.mCurrentState}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_RenderWindow::SR_RenderWindow(SR_RenderWindow&& rw) noexcept :
  mCurrentState{rw.mCurrentState}
{
  rw.mCurrentState = WindowStateInfo::WINDOW_CLOSED;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SR_RenderWindow& SR_RenderWindow::operator=(const SR_RenderWindow& rw) noexcept
{
  mCurrentState = rw.mCurrentState;
  return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SR_RenderWindow& SR_RenderWindow::operator=(SR_RenderWindow&& rw) noexcept
{
  mCurrentState = rw.mCurrentState;
  rw.mCurrentState = WindowStateInfo::WINDOW_CLOSED;
  return *this;
}



/*-------------------------------------
 * Instance Creation
-------------------------------------*/
ls::utils::Pointer<SR_RenderWindow> SR_RenderWindow::create() noexcept
{
    #ifdef LS_OS_WINDOWS
        return ls::utils::Pointer<SR_RenderWindow>{new SR_RenderWindowWin32{}};
    #elif defined(SR_PREFER_COCOA)
        return ls::utils::Pointer<SR_RenderWindow>{new SR_RenderWindowCocoa{}};
    #elif defined(LS_OS_UNIX)
        #if defined(SR_PREFER_XCB)
            return ls::utils::Pointer<SR_RenderWindow>{new SR_RenderWindowXCB{}};
        #else
            return ls::utils::Pointer<SR_RenderWindow>{new SR_RenderWindowXlib{}};
        #endif
    #else
        #error "Window buffer backend not implemented for this platform."
    #endif
}
