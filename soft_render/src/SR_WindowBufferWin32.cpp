
#include <windows.h>
#include <wingdi.h>

#include <iostream>

#include <cstdint> // fixed-width types
#include <cstdlib> // malloc(), free()
#include <limits> // std::numeric_limits
#include <memory> // std::nothrow
#include <utility> // std::move()

#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_RenderWindowWin32.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_WindowBufferWin32.hpp"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferWin32::~SR_WindowBufferWin32() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferWin32::SR_WindowBufferWin32() noexcept :
    SR_WindowBuffer{},
    mBitmapInfo{nullptr},
    mBuffer{nullptr},
    mWidth{0},
    mHeight{0}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferWin32::SR_WindowBufferWin32(SR_WindowBufferWin32&& wb) noexcept :
    SR_WindowBuffer{std::move(wb)},
    mBitmapInfo{wb.mBitmapInfo},
    mBuffer{wb.mBuffer},
    mWidth{wb.mWidth},
    mHeight{wb.mHeight}
{
    wb.mBitmapInfo = nullptr;
    wb.mBuffer = nullptr;
    wb.mWidth = 0;
    wb.mHeight = 0;
}




/*-------------------------------------
 *
-------------------------------------*/
SR_WindowBufferWin32& SR_WindowBufferWin32::operator=(SR_WindowBufferWin32&& wb) noexcept
{
    if (this != &wb)
    {
        SR_WindowBuffer::operator=(std::move(wb));

        mBitmapInfo = wb.mBitmapInfo;
        wb.mBitmapInfo = nullptr;

        mBuffer = wb.mBuffer;
        wb.mBuffer = nullptr;

        mWidth = wb.mWidth;
        wb.mWidth = 0;

        mHeight = wb.mHeight;
        wb.mHeight = 0;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_WindowBufferWin32::init(SR_RenderWindow& win, unsigned width, unsigned height) noexcept
{
    if (mBuffer)
    {
        return -1;
    }

    SR_RenderWindowWin32* pWin = dynamic_cast<SR_RenderWindowWin32*>(&win);
    if (!pWin)
    {
        return -2;
    }

    PBITMAPINFO pInfo = new BITMAPINFO;
    ls::utils::fast_memset(pInfo, 0, sizeof(BITMAPINFO));

    // invert the image's Y-axis to maintain consistency with Xlib
    pInfo->bmiHeader.biSize          = sizeof(BITMAPINFO);
    pInfo->bmiHeader.biWidth         = width;
    pInfo->bmiHeader.biHeight        = -height;
    pInfo->bmiHeader.biPlanes        = 1;
    pInfo->bmiHeader.biBitCount      = 32; // bpp
    pInfo->bmiHeader.biCompression   = BI_RGB;
    pInfo->bmiHeader.biSizeImage     = width * height * sizeof(SR_ColorRGBA8);
    pInfo->bmiHeader.biXPelsPerMeter = 0;
    pInfo->bmiHeader.biYPelsPerMeter = 0;
    pInfo->bmiHeader.biClrUsed       = 0;
    pInfo->bmiHeader.biClrImportant  = 0;

    SR_ColorRGBA8* pBitmap = new SR_ColorRGBA8[width * height];

    mBitmapInfo = pInfo;
    mBuffer     = pBitmap;
    mWidth      = width;
    mHeight     = height;

    return 0;
}



/*-------------------------------------
 *
-------------------------------------*/
int SR_WindowBufferWin32::terminate() noexcept
{
    if (mBuffer)
    {
        delete reinterpret_cast<PBITMAPINFO>(mBitmapInfo);
        mBitmapInfo = nullptr;

        delete [] mBuffer;
        mBuffer = nullptr;

        mWidth = 0;
        mHeight = 0;
    }

    return 0;
}
