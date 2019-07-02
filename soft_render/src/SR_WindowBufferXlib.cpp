
#include <cstdint> // fixed-width types
#include <cstdlib>
#include <utility> // std::move()
#include <cstdio> // perror

#include <unistd.h>

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

    if (mTexture.init(SR_COLOR_RGBA_8U, width, height, 1) != 0)
    {
        return -4;
    }

    char*            pTexData = reinterpret_cast<char*>(mTexture.data());
    XShmSegmentInfo* pShm     = new XShmSegmentInfo;
    XImage*          pImg     = XShmCreateImage(pWin->mDisplay, pVisual, 24, ZPixmap, pTexData, pShm, width, height);

    // Leaving this in case I need to go back to a default X11 implementation.
    //XImage* pImg = XCreateImage(pWin->mDisplay, pVisual, 24, ZPixmap, 0, pData, width, height, 32, 0);

    if (!pImg)
    {
        //free(pData);
        mTexture.terminate();
        return -5;
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

    // Textures on POSIX-based systems are page-aligned to ensure we can use
    // the X11-shared memory extension.
    // Hopefully this won't fail...
    pShm->shmid    = shmget(IPC_PRIVATE, width*height*sizeof(SR_ColorRGBA8), IPC_CREAT|permissions);

    #ifdef LS_OS_LINUX
    pShm->shmaddr  = pImg->data = (char*)shmat(pShm->shmid, pTexData, SHM_REMAP);
    #else
    pShm->shmaddr  = pImg->data = (char*)shmat(pShm->shmid, pTexData, SHM_RND);
    #endif

    pShm->readOnly = False;

    if ((long long)(pImg->data) == -1ll || XShmAttach(pWin->mDisplay, pShm) == False)
    {
        //free(pData);
        mTexture.terminate();
        XDestroyImage(pImg);
        return -6;
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
