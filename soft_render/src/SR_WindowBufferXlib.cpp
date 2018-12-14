
#include <cstdint> // fixed-width types
#include <cstdlib>
#include <utility> // std::move()

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <sys/ipc.h> // IPC_CREAT
#include <sys/shm.h> // shmget
#include <sys/stat.h> // SR_IRWXU

#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_RenderWindowXlib.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_WindowBufferXlib.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferXlib::~SR_WindowBufferXlib() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferXlib::SR_WindowBufferXlib() noexcept :
    SR_WindowBuffer{},
    mWindow{nullptr},
    mBuffer{nullptr},
    mShmInfo{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferXlib::SR_WindowBufferXlib(SR_WindowBufferXlib&& wb) noexcept :
    SR_WindowBuffer{std::move(wb)},
    mWindow{wb.mWindow},
    mBuffer{wb.mBuffer},
    mShmInfo{wb.mShmInfo}
{
    wb.mWindow = nullptr;
    wb.mBuffer = nullptr;
    wb.mShmInfo = nullptr;
}




/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferXlib& SR_WindowBufferXlib::operator=(SR_WindowBufferXlib&& wb) noexcept
{
    if (this != &wb)
    {
        SR_WindowBuffer::operator=(std::move(wb));

        mWindow = wb.mWindow;
        wb.mWindow = nullptr;

        mBuffer = wb.mBuffer;
        wb.mBuffer = nullptr;

        mShmInfo = wb.mShmInfo;
        wb.mShmInfo = nullptr;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_WindowBufferXlib::init(SR_RenderWindow& win, unsigned width, unsigned height) noexcept
{
    if (mBuffer)
    {
        return 0;
    }

    SR_RenderWindowXlib* pWin = dynamic_cast<SR_RenderWindowXlib*>(&win);

    if (pWin == nullptr)
    {
        return -1;
    }

    if (!pWin->valid())
    {
        return -2;
    }


    Visual* pVisual = DefaultVisual(pWin->mDisplay, DefaultScreen(pWin->mDisplay));
    if (!pVisual)
    {
        return -3;
    }

    /*
    char* pData = (char*)malloc(width*height*sizeof(uint8_t)*4);
    if (!pData)
    {
        return -4;
    }
    */

    //XImage* pImg = XCreateImage(pWin->mDisplay, pVisual, 24, ZPixmap, 0, pData, width, height, 32, 0);

    XShmSegmentInfo* pShm = new XShmSegmentInfo;
    XImage* pImg = XShmCreateImage(pWin->mDisplay, pVisual, 24, ZPixmap, nullptr, pShm, width, height);

    if (!pImg)
    {
        //free(pData);
        return -4;
    }

    // Set the shared memory permissions to "rw-rw----". This should prevent
    // unauthorized users from reading the backbuffer. I don't always care
    // about security, but this one seems too obvious.
    constexpr int permissions =
        0
        | S_IREAD
        | S_IWRITE
        | S_IRGRP
        | S_IWGRP
        | 0;

    pShm->shmid = shmget(IPC_PRIVATE, width*height*sizeof(SR_ColorRGBA8), IPC_CREAT|permissions);
    pShm->shmaddr = pImg->data = (char*)shmat(pShm->shmid, nullptr, 0);
    pShm->readOnly = False;

    if (shmctl(pShm->shmid, IPC_RMID, nullptr) < 0 || XShmAttach(pWin->mDisplay, pShm) == False)
    {
        //free(pData);
        XDestroyImage(pImg);
        return -5;
    }

    mWindow = &win;
    mBuffer = pImg;
    mShmInfo = pShm;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_WindowBufferXlib::terminate() noexcept
{
    if (mBuffer)
    {
        SR_RenderWindowXlib* pWin = dynamic_cast<SR_RenderWindowXlib*>(mWindow);

        XDestroyImage(reinterpret_cast<XImage*>(mBuffer));
        XShmDetach(pWin->mDisplay, reinterpret_cast<XShmSegmentInfo*>(mShmInfo));

        mWindow = nullptr;
        mBuffer = nullptr;

        delete reinterpret_cast<XShmSegmentInfo*>(mShmInfo);
        mShmInfo = nullptr;
    }

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
unsigned SR_WindowBufferXlib::width() const noexcept
{
    if (mBuffer != nullptr)
    {
        return (unsigned)(reinterpret_cast<XImage*>(mBuffer)->width);
    }

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
unsigned SR_WindowBufferXlib::height() const noexcept
{
    if (mBuffer != nullptr)
    {
        return (unsigned)(reinterpret_cast<XImage*>(mBuffer)->height);
    }

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
const void* SR_WindowBufferXlib::native_handle() const noexcept
{
    return mBuffer;
}



/*-------------------------------------
 *
-------------------------------------*/
void* SR_WindowBufferXlib::native_handle() noexcept
{
    return mBuffer;
}



/*-------------------------------------
 *
-------------------------------------*/
const SR_ColorRGBA8* SR_WindowBufferXlib::buffer() const noexcept
{
    const XImage* const  pImg = reinterpret_cast<XImage*>(mBuffer);
    return pImg ? reinterpret_cast<const SR_ColorRGBA8*>(pImg->data) : nullptr;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_ColorRGBA8* SR_WindowBufferXlib::buffer() noexcept
{
    XImage* const  pImg = reinterpret_cast<XImage*>(mBuffer);
    return pImg ? reinterpret_cast<SR_ColorRGBA8*>(pImg->data) : nullptr;
}
