
#include <string>

#include "lightsky/setup/OS.h" // OS detection

#ifdef LS_OS_WINDOWS
    #include "soft_render/SR_RenderWindowWin32.hpp"
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
    #elif defined(LS_OS_OSX)
        return ls::utils::Pointer<SR_RenderWindow>{new SR_RenderWindowXCB{}};
    #elif defined(LS_OS_LINUX)
        return ls::utils::Pointer<SR_RenderWindow>{new SR_RenderWindowXlib{}};
    #else
        return ls::utils::Pointer<SR_RenderWindow>{nullptr};
    #endif
}
