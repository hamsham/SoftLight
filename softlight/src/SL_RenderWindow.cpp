
#include <string>

#include "lightsky/setup/OS.h" // OS detection

#ifdef LS_OS_WINDOWS
    #include "softlight/SL_RenderWindowWin32.hpp"
#elif defined(SL_PREFER_COCOA)
        #include "softlight/SL_RenderWindowCocoa.hpp"
#else
    #include "softlight/SL_RenderWindowXCB.hpp"
    #include "softlight/SL_RenderWindowXlib.hpp"
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
    #ifdef LS_OS_WINDOWS
        return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowWin32{}};
    #elif defined(SL_PREFER_COCOA)
        return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowCocoa{}};
    #elif defined(LS_OS_UNIX)
        #if defined(SL_PREFER_XCB)
            return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowXCB{}};
        #else
            return ls::utils::Pointer<SL_RenderWindow>{new SL_RenderWindowXlib{}};
        #endif
    #else
        #error "Window buffer backend not implemented for this platform."
    #endif
}
