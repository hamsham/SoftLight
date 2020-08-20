
#include <utility> // std::move()

#include "lightsky/setup/OS.h" // OS detection

#if defined(LS_OS_WINDOWS)
    #include "softlight/SL_WindowBufferWin32.hpp"
#elif defined(SL_PREFER_COCOA)
    #include "softlight/SL_WindowBufferCocoa.hpp"
#else
    #include "softlight/SL_WindowBufferXCB.hpp"
    #include "softlight/SL_WindowBufferXlib.hpp"
#endif

#include "softlight/SL_WindowBuffer.hpp"



/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_WindowBuffer::~SL_WindowBuffer() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_WindowBuffer::SL_WindowBuffer() noexcept
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_WindowBuffer::SL_WindowBuffer(SL_WindowBuffer&& wb) noexcept :
    mTexture{std::move(wb.mTexture)}
{}



/*-------------------------------------
 * Move another window buffer into *this.
-------------------------------------*/
SL_WindowBuffer& SL_WindowBuffer::operator=(SL_WindowBuffer&& wb) noexcept
{
    mTexture = std::move(wb.mTexture);
    return *this;
}



/*-------------------------------------
 * Instance Creation
-------------------------------------*/
ls::utils::Pointer<SL_WindowBuffer> SL_WindowBuffer::create() noexcept
{
    #ifdef LS_OS_WINDOWS
        return ls::utils::Pointer<SL_WindowBuffer>{new SL_WindowBufferWin32{}};
    #elif defined(SL_PREFER_COCOA)
            return ls::utils::Pointer<SL_WindowBuffer>{new SL_WindowBufferCocoa{}};
    #elif defined(LS_OS_UNIX)
        #if defined(SL_PREFER_XCB)
            return ls::utils::Pointer<SL_WindowBuffer>{new SL_WindowBufferXCB{}};
        #else
            return ls::utils::Pointer<SL_WindowBuffer>{new SL_WindowBufferXlib{}};
        #endif
    #else
        #error "Window buffer backend not implemented for this platform."
    #endif
}
