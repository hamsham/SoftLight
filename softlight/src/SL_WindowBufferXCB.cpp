
#include <cstdint> // fixed-width types
#include <cstdlib>
#include <utility> // std::move()

#include <unistd.h>
#include <string.h> // strerror()

extern "C"
{
    #if SL_ENABLE_XSHM != 0
        #include <xcb/xcb.h>
        #include <xcb/shm.h>
        #include <xcb/xcb_image.h>

        #include <sys/ipc.h> // IPC_CREAT
        #include <sys/shm.h> // shmget
        #include <sys/stat.h> // SL_IRWXU
    #endif /* SL_ENABLE_XSHM */
}

#include "lightsky/utils/Log.h"

#include "softlight/SL_Color.hpp"
#include "softlight/SL_RenderWindowXCB.hpp"
#include "softlight/SL_WindowBufferXCB.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
#if SL_ENABLE_XSHM != 0
/*-------------------------------------
 *
-------------------------------------*/
SL_WindowBufferXCB::~SL_WindowBufferXCB() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SL_WindowBufferXCB::SL_WindowBufferXCB() noexcept :
    SL_WindowBuffer{},
    mWindow{nullptr},
    mShmInfo{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_WindowBufferXCB::SL_WindowBufferXCB(SL_WindowBufferXCB&& wb) noexcept :
    SL_WindowBuffer{std::move(wb)},
    mWindow{wb.mWindow},
    mShmInfo{wb.mShmInfo}
{
    wb.mWindow = nullptr;
    wb.mShmInfo = nullptr;
}




/*-------------------------------------
 *
-------------------------------------*/
SL_WindowBufferXCB& SL_WindowBufferXCB::operator=(SL_WindowBufferXCB&& wb) noexcept
{
    if (this != &wb)
    {
        SL_WindowBuffer::operator=(std::move(wb));

        mWindow = wb.mWindow;
        wb.mWindow = nullptr;

        mShmInfo = wb.mShmInfo;
        wb.mShmInfo = nullptr;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_WindowBufferXCB::init(SL_RenderWindow& win, unsigned width, unsigned height) noexcept
{
    if (mTexture.data())
    {
        return -1;
    }

    SL_RenderWindowXCB* pWin = dynamic_cast<SL_RenderWindowXCB*>(&win);

    if (pWin == nullptr)
    {
        return -2;
    }

    if (!pWin->valid())
    {
        LS_LOG_ERR("Attempting to create a window buffer for an invalid window.");
        return -3;
    }

    //Shm test
    xcb_shm_segment_info_t*        pInfo = nullptr;
    xcb_connection_t*              pConnection = reinterpret_cast<xcb_connection_t*>(pWin->native_handle());

    if (!pConnection)
    {
        return -4;
    }

    pInfo =  new(std::nothrow) xcb_shm_segment_info_t;
    if (!pInfo)
    {
        LS_LOG_ERR("Unable to allocate memory to setup XCB shared memory data.");
        return -5;
    }

    if (mTexture.init(SL_COLOR_RGBA_8U, width, height, 1) != 0)
    {
        LS_LOG_ERR("Unable to initialize a texture for a XCB SHM window.");
        delete pInfo;
        return -7;
    }

    // Some POSIX systems require that the user, group, and "other" can all
    // read to and write to the shared memory segment.
    constexpr int permissions =
        0
        | S_IREAD
        | S_IWRITE
        | S_IRGRP
        | S_IWGRP
        | S_IROTH
        | S_IWOTH
        | 0;

    // Textures on POSIX-based systems are page-aligned to ensure we can use
    // the X11-shared memory extension.
    // Hopefully this won't fail...
    pInfo->shmid = shmget(IPC_PRIVATE, width*height*sizeof(SL_ColorRGBA8), IPC_CREAT|permissions);

    if (!pInfo->shmid)
    {
        LS_LOG_ERR("Unable to allocate a shared memory segment: (", errno, ") ", strerror(errno));
        delete pInfo;
        mTexture.terminate();
        return -8;
    }

    pInfo->shmaddr = (unsigned char*)shmat(pInfo->shmid, (char*)mTexture.data(), SHM_REMAP);
    pInfo->shmseg = xcb_generate_id(pConnection);
    xcb_shm_attach(pConnection, pInfo->shmseg, pInfo->shmid, 0);
    //shmctl(info.shmid, IPC_RMID, 0);

    mWindow = &win;
    mShmInfo = pInfo;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_WindowBufferXCB::terminate() noexcept
{
    if (mTexture.data())
    {
        mTexture.terminate();

        delete (xcb_shm_segment_info_t*)mShmInfo;
        mShmInfo = nullptr;

        mWindow = nullptr;
    }

    return 0;
}




/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
#else
/*-------------------------------------
 *
-------------------------------------*/
SL_WindowBufferXCB::~SL_WindowBufferXCB() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SL_WindowBufferXCB::SL_WindowBufferXCB() noexcept :
    SL_WindowBuffer{},
    mWindow{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_WindowBufferXCB::SL_WindowBufferXCB(SL_WindowBufferXCB&& wb) noexcept :
    SL_WindowBuffer{std::move(wb)},
    mWindow{wb.mWindow}
{
    wb.mWindow = nullptr;
}




/*-------------------------------------
 *
-------------------------------------*/
SL_WindowBufferXCB& SL_WindowBufferXCB::operator=(SL_WindowBufferXCB&& wb) noexcept
{
    if (this != &wb)
    {
        SL_WindowBuffer::operator=(std::move(wb));

        mWindow = wb.mWindow;
        wb.mWindow = nullptr;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_WindowBufferXCB::init(SL_RenderWindow& win, unsigned width, unsigned height) noexcept
{
    if (mTexture.data())
    {
        return -1;
    }

    SL_RenderWindowXCB* pWin = dynamic_cast<SL_RenderWindowXCB*>(&win);

    if (pWin == nullptr)
    {
        return -2;
    }

    if (!pWin->valid())
    {
        return -3;
    }

    if (mTexture.init(SL_COLOR_RGBA_8U, width, height, 1) != 0)
    {
        return -4;
    }

    mWindow = &win;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_WindowBufferXCB::terminate() noexcept
{
    if (mTexture.data())
    {
        mTexture.terminate();

        mWindow = nullptr;
    }

    return 0;
}



#endif /* SL_ENABLE_XSHM */
