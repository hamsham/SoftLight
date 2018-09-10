
#include "lightsky/setup/OS.h" // OS detection

#ifdef LS_OS_WINDOWS
    #include "soft_render/SR_WindowBufferWin32.hpp"
#else
    #include "soft_render/SR_WindowBufferXlib.hpp"
#endif

#include "soft_render/SR_WindowBuffer.hpp"




/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBuffer::~SR_WindowBuffer() noexcept
{
}



/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBuffer::SR_WindowBuffer() noexcept
{
}



/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBuffer::SR_WindowBuffer(SR_WindowBuffer&&) noexcept
{
}




/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBuffer& SR_WindowBuffer::operator=(SR_WindowBuffer&&) noexcept
{
    return *this;
}



/*-------------------------------------
 * Instance Creation
-------------------------------------*/
ls::utils::Pointer<SR_WindowBuffer> SR_WindowBuffer::create() noexcept
{
    #ifdef LS_OS_WINDOWS
        return ls::utils::Pointer<SR_WindowBuffer>{new SR_WindowBufferWin32{}};
    #else
        return ls::utils::Pointer<SR_WindowBuffer>{new SR_WindowBufferXlib{}};
    #endif
}
