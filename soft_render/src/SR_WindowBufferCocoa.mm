
#import <Cocoa/Cocoa.h>

#include "lightsky/utils/Log.h"

#import "soft_render/SR_RenderWindowCocoa.hpp"
#include "soft_render/SR_WindowBufferCocoa.hpp"



/*-----------------------------------------------------------------------------
 * Cocoa Image Access Callbacks
-----------------------------------------------------------------------------*/
inline size_t SR_CocoaDataGetBytesAtOffset(void* info, void* buffer, off_t offset, size_t count)
{
    const uint8_t* pIn = reinterpret_cast<const uint8_t*>(info) + offset;
    const uint8_t* pEnd = pIn + sizeof(uint8_t) * count;
    uint8_t* pOut = reinterpret_cast<uint8_t*>(buffer);

    while (pIn != pEnd)
    {
        *pOut++ = *pIn++;
    }

    return count;
}



/*-----------------------------------------------------------------------------
 * Cocoa backbuffer implementation.
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_WindowBufferCocoa::~SR_WindowBufferCocoa() noexcept
{
    terminate();
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_WindowBufferCocoa::SR_WindowBufferCocoa() noexcept :
    SR_WindowBuffer{},
    mImageProvider{nullptr},
    mColorSpace{nullptr}
    //mImageRef{nullptr}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_WindowBufferCocoa::SR_WindowBufferCocoa(SR_WindowBufferCocoa&& wb) noexcept :
    SR_WindowBuffer{std::move(wb)}, // This handles the internal texture
    mImageProvider{wb.mImageProvider},
    mColorSpace{wb.mColorSpace}
    //mImageRef{wb.mImageRef}
{
    wb.mImageProvider = nullptr;
    wb.mColorSpace = nullptr;
    //wb.mImageRef = nullptr;
}




/*-------------------------------------
 * Move Operator
-------------------------------------*/
SR_WindowBufferCocoa& SR_WindowBufferCocoa::operator=(SR_WindowBufferCocoa&& wb) noexcept
{
    if (this != &wb)
    {
        // This handles the internal texture
        SR_WindowBuffer::operator=(std::move(wb));

        mImageProvider = wb.mImageProvider;
        wb.mImageProvider = nullptr;

        mColorSpace = wb.mColorSpace;
        wb.mColorSpace = nullptr;

        //mImageRef = wb.mImageRef;
        //wb.mImageRef = nullptr;
    }

    return *this;
}



/*-------------------------------------
 * Initialize the backbuffer
-------------------------------------*/
int SR_WindowBufferCocoa::init(SR_RenderWindow& win, unsigned width, unsigned height) noexcept
{
    if (mTexture.data() != nullptr)
    {
        return -1;
    }

    if (mTexture.init(SR_COLOR_RGBA_8U, width, height, 1) != 0)
    {
        LS_LOG_ERR("Error: Unable to create the backing storage for a Cocoa widow buffer.");
        return -2;
    }

    SR_RenderWindowCocoa* pWin = dynamic_cast<SR_RenderWindowCocoa*>(&win);
    if (!pWin)
    {
        LS_LOG_ERR("Error: Unknown Render Window type.");
        return -3;
    }

    NSWindow* pNativeWin = (NSWindow*)pWin->mWindow;
    if (!pNativeWin)
    {
        LS_LOG_ERR("Error: Could not initialize a Cocoa window buffer with an uninitialized Cocoa window.");
        return -4;
    }

    // Provider
    struct CGDataProviderDirectCallbacks cbs;
    cbs.version = 0;
    cbs.getBytePointer = nullptr;
    cbs.releaseBytePointer = nullptr;
    cbs.getBytesAtPosition = &SR_CocoaDataGetBytesAtOffset;
    cbs.releaseInfo = nullptr;

    unsigned bpp = (unsigned)mTexture.bpp();
    unsigned numBytes = width * height * bpp;
    CGDataProviderRef provider = CGDataProviderCreateDirect(mTexture.data(), numBytes, &cbs);
    if (!provider)
    {
        LS_LOG_ERR("Error: Unable to create a CGDataProvider to store a window buffer.");
        mTexture.terminate();
        return -5;
    }
    CGDataProviderRetain(provider);

    CGColorSpaceRef colorSpace = [[pNativeWin colorSpace] CGColorSpace];
    if (!colorSpace)
    {
        LS_LOG_ERR("Error: Could not retrieve a color-space reference to load a Cocoa window buffer.");
        mTexture.terminate();
        CGDataProviderRelease(provider);
        return -6;
    }
    CGColorSpaceRetain(colorSpace);

    /*
    CGImageRef imgRef = CGImageCreate(
        width,
        height,
        CHAR_BIT,
        CHAR_BIT*bpp,
        bpp*width,
        colorSpace,
        kCGImageAlphaNoneSkipFirst|kCGImageByteOrder32Little,
        provider,
        nullptr,
        NO,
        kCGRenderingIntentDefault);

    if (!imgRef)
    {
        LS_LOG_ERR("Error: Could not create a native Cocoa window buffer.");
        mTexture.terminate();
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpace);
        return -7;
    }
    */

    mImageProvider = (void*)provider;
    mColorSpace    = (void*)colorSpace;
    //mImageRef      = (void*)imgRef;

    return 0;
}



/*-------------------------------------
 * Clear all memory used by the backbuffer.
-------------------------------------*/
int SR_WindowBufferCocoa::terminate() noexcept
{
    //CGImageRelease((CGImageRef)mImageRef);
    CGColorSpaceRelease((CGColorSpaceRef)mColorSpace);
    CGDataProviderRelease((CGDataProviderRef)mImageProvider);

    mTexture.terminate();

    return 0;
}

