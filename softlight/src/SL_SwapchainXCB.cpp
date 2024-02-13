
#include <utility> // std::move()
#include <cstring> // strerror()

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

#include "softlight/SL_RenderWindowXCB.hpp"
#include "softlight/SL_SwapchainXCB.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
#if SL_ENABLE_XSHM != 0
/*-------------------------------------
 *
-------------------------------------*/
SL_SwapchainXCB::~SL_SwapchainXCB() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SL_SwapchainXCB::SL_SwapchainXCB() noexcept :
    SL_Swapchain{},
    mWindow{nullptr},
    mShmInfo{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_SwapchainXCB::SL_SwapchainXCB(SL_SwapchainXCB&& wb) noexcept :
    SL_Swapchain{std::move(wb)},
    mWindow{wb.mWindow},
    mShmInfo{wb.mShmInfo}
{
    wb.mWindow = nullptr;
    wb.mShmInfo = nullptr;
}




/*-------------------------------------
 *
-------------------------------------*/
SL_SwapchainXCB& SL_SwapchainXCB::operator=(SL_SwapchainXCB&& wb) noexcept
{
    if (this != &wb)
    {
        SL_Swapchain::operator=(std::move(wb));

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
int SL_SwapchainXCB::init(SL_RenderWindow& win, unsigned width, unsigned height) noexcept
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

    xcb_connection_t* const pConnection = reinterpret_cast<xcb_connection_t*>(pWin->native_handle());
    if (!pConnection)
    {
        return -4;
    }

    xcb_shm_segment_info_t* const pInfo = new(std::nothrow) xcb_shm_segment_info_t;
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
    constexpr int permissions = 0
        | S_IREAD
        | S_IWRITE
        | S_IRGRP
        | S_IWGRP
        | S_IROTH
        | S_IWOTH
        ;

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

    pInfo->shmaddr = (unsigned char*)shmat((int)pInfo->shmid, (char*)mTexture.data(), SHM_REMAP);
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
int SL_SwapchainXCB::terminate() noexcept
{
    if (mTexture.data())
    {
        mTexture.terminate();

        xcb_shm_segment_info_t* const pSegment = static_cast<xcb_shm_segment_info_t*>(mShmInfo);
        xcb_connection_t* const pConnection = reinterpret_cast<xcb_connection_t*>(mWindow->native_handle());

        xcb_shm_detach(pConnection, pSegment->shmseg);
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
SL_SwapchainXCB::~SL_SwapchainXCB() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SL_SwapchainXCB::SL_SwapchainXCB() noexcept :
    SL_Swapchain{},
    mWindow{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_SwapchainXCB::SL_SwapchainXCB(SL_SwapchainXCB&& wb) noexcept :
    SL_Swapchain{std::move(wb)},
    mWindow{wb.mWindow}
{
    wb.mWindow = nullptr;
}




/*-------------------------------------
 *
-------------------------------------*/
SL_SwapchainXCB& SL_SwapchainXCB::operator=(SL_SwapchainXCB&& wb) noexcept
{
    if (this != &wb)
    {
        SL_Swapchain::operator=(std::move(wb));

        mWindow = wb.mWindow;
        wb.mWindow = nullptr;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SL_SwapchainXCB::init(SL_RenderWindow& win, unsigned width, unsigned height) noexcept
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
int SL_SwapchainXCB::terminate() noexcept
{
    if (mTexture.data())
    {
        mTexture.terminate();

        mWindow = nullptr;
    }

    return 0;
}



#endif /* SL_ENABLE_XSHM */
