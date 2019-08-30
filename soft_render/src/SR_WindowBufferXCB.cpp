
#include <cstdint> // fixed-width types
#include <cstdlib>
#include <utility> // std::move()

#include <unistd.h>
#include <string.h> // strerror()

#include "lightsky/utils/Log.h"

#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_RenderWindowXCB.hpp"
#include "soft_render/SR_WindowBufferXCB.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferXCB::~SR_WindowBufferXCB() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferXCB::SR_WindowBufferXCB() noexcept :
    SR_WindowBuffer{},
    mWindow{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferXCB::SR_WindowBufferXCB(SR_WindowBufferXCB&& wb) noexcept :
    SR_WindowBuffer{std::move(wb)},
    mWindow{wb.mWindow}
{
    wb.mWindow = nullptr;
}




/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferXCB& SR_WindowBufferXCB::operator=(SR_WindowBufferXCB&& wb) noexcept
{
    if (this != &wb)
    {
        SR_WindowBuffer::operator=(std::move(wb));

        mWindow = wb.mWindow;
        wb.mWindow = nullptr;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_WindowBufferXCB::init(SR_RenderWindow& win, unsigned width, unsigned height) noexcept
{
    if (mTexture.data())
    {
        return -1;
    }

    SR_RenderWindowXCB* pWin = dynamic_cast<SR_RenderWindowXCB*>(&win);

    if (pWin == nullptr)
    {
        return -2;
    }

    if (!pWin->valid())
    {
        return -3;
    }

    if (mTexture.init(SR_COLOR_RGBA_8U, width, height, 1) != 0)
    {
        return -4;
    }

    mWindow = &win;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_WindowBufferXCB::terminate() noexcept
{
    if (mTexture.data())
    {
        mTexture.terminate();

        mWindow = nullptr;
    }

    return 0;
}
