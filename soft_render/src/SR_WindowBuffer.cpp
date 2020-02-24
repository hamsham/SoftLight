
#include <utility> // std::move()

#include "lightsky/setup/OS.h" // OS detection

#if defined(LS_OS_WINDOWS)
    #include "soft_render/SR_WindowBufferWin32.hpp"
#elif defined(SR_PREFER_COCOA)
    #include "soft_render/SR_WindowBufferCocoa.hpp"
#else
    #include "soft_render/SR_WindowBufferXCB.hpp"
    #include "soft_render/SR_WindowBufferXlib.hpp"
#endif

#include "soft_render/SR_WindowBuffer.hpp"



/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_WindowBuffer::~SR_WindowBuffer() noexcept
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_WindowBuffer::SR_WindowBuffer() noexcept
{
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_WindowBuffer::SR_WindowBuffer(SR_WindowBuffer&& wb) noexcept :
    mTexture{std::move(wb.mTexture)}
{}



/*-------------------------------------
 * Move another window buffer into *this.
-------------------------------------*/
SR_WindowBuffer& SR_WindowBuffer::operator=(SR_WindowBuffer&& wb) noexcept
{
    mTexture = std::move(wb.mTexture);
    return *this;
}



/*-------------------------------------
 * Instance Creation
-------------------------------------*/
ls::utils::Pointer<SR_WindowBuffer> SR_WindowBuffer::create() noexcept
{
    #ifdef LS_OS_WINDOWS
        return ls::utils::Pointer<SR_WindowBuffer>{new SR_WindowBufferWin32{}};
    #elif defined(SR_PREFER_COCOA)
            return ls::utils::Pointer<SR_WindowBuffer>{new SR_WindowBufferCocoa{}};
    #elif defined(LS_OS_UNIX)
        #if defined(SR_PREFER_XCB)
            return ls::utils::Pointer<SR_WindowBuffer>{new SR_WindowBufferXCB{}};
        #else
            return ls::utils::Pointer<SR_WindowBuffer>{new SR_WindowBufferXlib{}};
        #endif
    #else
        #error "Window buffer backend not implemented for this platform."
    #endif
}
