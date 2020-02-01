
#include <windows.h>
#include <wingdi.h>

#include <iostream>

#include <cstdint> // fixed-width types
#include <cstdlib> // malloc(), free()
#include <limits> // std::numeric_limits
#include <memory> // std::nothrow
#include <utility> // std::move()

#include "lightsky/utils/Pointer.h" // aligned_alloc
#include "lightsky/utils/Copy.h" // fast_memset

#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_RenderWindowWin32.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_WindowBufferWin32.hpp"



/*-----------------------------------------------------------------------------
 * Windows backbuffer implementation.
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_WindowBufferWin32::~SR_WindowBufferWin32() noexcept
{
    terminate();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_WindowBufferWin32::SR_WindowBufferWin32() noexcept :
    SR_WindowBuffer{},
    mBitmapInfo{nullptr}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_WindowBufferWin32::SR_WindowBufferWin32(SR_WindowBufferWin32&& wb) noexcept :
    SR_WindowBuffer{std::move(wb)}, // This handles the internal texture
    mBitmapInfo{wb.mBitmapInfo}
{
    wb.mBitmapInfo = nullptr;
}




/*-------------------------------------
 * Move Operator
-------------------------------------*/
SR_WindowBufferWin32& SR_WindowBufferWin32::operator=(SR_WindowBufferWin32&& wb) noexcept
{
    if (this != &wb)
    {
        // This handles the internal texture
        SR_WindowBuffer::operator=(std::move(wb));

        mBitmapInfo = wb.mBitmapInfo;
        wb.mBitmapInfo = nullptr;
    }

    return *this;
}



/*-------------------------------------
 * Initialize the backbuffer
-------------------------------------*/
int SR_WindowBufferWin32::init(SR_RenderWindow& win, unsigned width, unsigned height) noexcept
{
    if (mTexture.data() != nullptr)
    {
        return -1;
    }

    SR_RenderWindowWin32* pWin = dynamic_cast<SR_RenderWindowWin32*>(&win);
    if (!pWin)
    {
        return -2;
    }

    PBITMAPINFO pInfo = (PBITMAPINFO)ls::utils::aligned_malloc(sizeof(BITMAPINFO));
    if (!pInfo)
    {
        return -3;
    }

    ls::utils::fast_memset(pInfo, 0, sizeof(BITMAPINFO));

    pInfo->bmiHeader.biSize          = sizeof(BITMAPINFO);
    pInfo->bmiHeader.biWidth         = width;
    pInfo->bmiHeader.biHeight        = -height; // invert the Y-axis to maintain consistency with Xlib
    pInfo->bmiHeader.biPlanes        = 1;
    pInfo->bmiHeader.biBitCount      = 32; // bpp
    pInfo->bmiHeader.biCompression   = BI_RGB;
    pInfo->bmiHeader.biSizeImage     = width * height * sizeof(SR_ColorRGBA8);
    pInfo->bmiHeader.biXPelsPerMeter = 0;
    pInfo->bmiHeader.biYPelsPerMeter = 0;
    pInfo->bmiHeader.biClrUsed       = 0;
    pInfo->bmiHeader.biClrImportant  = 0;

    if (mTexture.init(SR_COLOR_RGBA_8U, width, height, 1) != 0)
    {
        ls::utils::aligned_free(pInfo);
        return -4;
    }

    mBitmapInfo = pInfo;

    return 0;
}



/*-------------------------------------
 * Clear all memory used by the backbuffer.
-------------------------------------*/
int SR_WindowBufferWin32::terminate() noexcept
{
    if (mBitmapInfo)
    {
        ls::utils::aligned_free(mBitmapInfo);
        mBitmapInfo = nullptr;

        mTexture.terminate();
    }

    return 0;
}
