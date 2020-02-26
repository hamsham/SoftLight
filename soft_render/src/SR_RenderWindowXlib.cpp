
#include <cassert> // assert
#include <cstdlib> // std::getenv
#include <limits> // numeric_limits<>
#include <memory> // std::move
#include <new> // std::nothrow

// XLib should use an un-mangled interface
extern "C"
{
    #include <X11/Xlib.h>
    #include <X11/Xutil.h> // XVisualInfo, <X11/keysym.h>
    #include <X11/XKBlib.h> // XkbKeycodeToKeysym

    #if SR_ENABLE_XSHM != 0
        #include <X11/extensions/XShm.h>
    #endif
}

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Log.h"

#include "soft_render/SR_RenderWindowXlib.hpp"
#include "soft_render/SR_WindowBufferXlib.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace utils = ls::utils;

namespace
{



/*-------------------------------------
 * Window Positioning
-------------------------------------*/
bool _xlib_get_position(_XDisplay* const pDisplay, const unsigned long window, int& x, int& y) noexcept
{
    Window child;

    #ifdef LS_ARCH_X86
        alignas(sizeof(__m256)) XWindowAttributes attribs;
    #else
        XWindowAttributes attribs;
    #endif

    utils::fast_memset(&attribs, 0, sizeof(XWindowAttributes));

    int tempX, tempY;

    if (True != XGetWindowAttributes(pDisplay, window, &attribs)
    || True != XTranslateCoordinates(pDisplay, window, RootWindowOfScreen(attribs.screen), 0, 0, &tempX, &tempY, &child)
    //|| True != XGetWindowAttributes(pDisplay, child, &attribs)
    )
    {
        return false;
    }

    x = attribs.x;
    y = attribs.y;

    return true;
}



} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SR_RenderWindowXlib
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_RenderWindowXlib::~SR_RenderWindowXlib() noexcept
{
    if (this->valid() && destroy() != 0)
    {
        LS_LOG_ERR("Unable to properly close the render window ", this, " during destruction.");
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_RenderWindowXlib::SR_RenderWindowXlib() noexcept :
    SR_RenderWindow{},
    mDisplay{nullptr},
    mWindow{None},
    mCloseAtom{None},
    mLastEvent{nullptr},
    mWidth{0},
    mHeight{0},
    mX{0},
    mY{0},
    mMouseX{0},
    mMouseY{0},
    mKeysRepeat{true},
    mCaptureMouse{false}
{
    ls::utils::runtime_assert(XInitThreads() != False, ls::utils::LS_WARNING, "Unable to initialize Xlib for threading.");
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SR_RenderWindowXlib::SR_RenderWindowXlib(const SR_RenderWindowXlib& rw) noexcept :
    SR_RenderWindowXlib{} // delegate
{
    // delegate some more
    *this = rw;
}


/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_RenderWindowXlib::SR_RenderWindowXlib(SR_RenderWindowXlib&& rw) noexcept :
    SR_RenderWindow{std::move(rw)},
    mDisplay{rw.mDisplay},
    mWindow{rw.mWindow},
    mCloseAtom{rw.mCloseAtom},
    mLastEvent{rw.mLastEvent},
    mWidth{rw.mWidth},
    mHeight{rw.mHeight},
    mX{rw.mX},
    mY{rw.mY},
    mMouseX{rw.mMouseX},
    mMouseY{rw.mMouseY},
    mKeysRepeat{rw.mKeysRepeat},
    mCaptureMouse{rw.mCaptureMouse}
{
    rw.mDisplay = nullptr;
    rw.mWindow = None;
    rw.mCloseAtom = None;
    rw.mLastEvent = nullptr;
    rw.mWidth = 0;
    rw.mHeight = 0;
    rw.mX = 0;
    rw.mY = 0;
    rw.mMouseX = 0;
    rw.mMouseY = 0;
    rw.mKeysRepeat = true;
    rw.mCaptureMouse = false;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SR_RenderWindowXlib& SR_RenderWindowXlib::operator=(const SR_RenderWindowXlib& rw) noexcept
{
    if (this == &rw)
    {
        return *this;
    }

    this->destroy();

    SR_RenderWindowXlib* const pWindow = static_cast<SR_RenderWindowXlib*>(rw.clone());

    if (pWindow && pWindow->valid())
    {
        // handle the base class
        SR_RenderWindow::operator=(rw);
        *this = std::move(*pWindow);
    }

    delete pWindow;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SR_RenderWindowXlib& SR_RenderWindowXlib::operator=(SR_RenderWindowXlib&& rw) noexcept
{
    if (this == &rw)
    {
        return *this;
    }

    if (this->valid())
    {
        this->destroy();
    }

    // handle the base class
    SR_RenderWindow::operator=(std::move(rw));

    this->mDisplay = rw.mDisplay;
    rw.mDisplay = nullptr;

    this->mWindow = rw.mWindow;
    rw.mWindow = None;

    this->mCloseAtom = rw.mCloseAtom;
    rw.mCloseAtom = None;

    this->mLastEvent = rw.mLastEvent;
    rw.mLastEvent = nullptr;

    this->mWidth = rw.mWidth;
    rw.mWidth = 0;

    this->mHeight = rw.mHeight;
    rw.mHeight = 0;

    this->mX = rw.mX;
    rw.mX = 0;

    this->mY = rw.mY;
    rw.mY = 0;

    this->mMouseX = rw.mMouseX;
    rw.mMouseX = 0;

    this->mMouseY = rw.mMouseY;
    rw.mMouseY = 0;

    this->mKeysRepeat = rw.mKeysRepeat;
    rw.mKeysRepeat = true;

    this->mCaptureMouse = rw.mCaptureMouse;
    rw.mCaptureMouse = false;

    return *this;
}



/*-------------------------------------
 * Window Initialization
-------------------------------------*/
int SR_RenderWindowXlib::set_title(const char* const pName) noexcept
{
    if (!valid())
    {
        return -1;
    }

    XTextProperty textData;
    const Status retCode = XStringListToTextProperty(const_cast<char**>(&pName), 1, &textData);

    if (retCode != True)
    {
        return -2;
    }

    XSetWMName(mDisplay, mWindow, &textData);
    XFree(textData.value);

    return 0;
}



/*-------------------------------------
 * Window Initialization
-------------------------------------*/
int SR_RenderWindowXlib::init(unsigned width, unsigned height) noexcept
{
    #ifdef LS_ARCH_X86
        alignas(sizeof(__m256)) XVisualInfo visualTemplate;
        alignas(sizeof(__m256)) XSetWindowAttributes windowAttribs;
    #else
        XVisualInfo visualTemplate;
        XSetWindowAttributes windowAttribs;
    #endif

    const char* const  pDisplayName       = std::getenv("DISPLAY");
    int                errCode            = 0;
    Display*           pDisplay           = nullptr;
    constexpr long     visualMatchMask    = VisualScreenMask|VisualClassMask|VisualRedMaskMask|VisualGreenMaskMask|VisualBlueMaskMask;//|VisualBitsPerRGBMask;
    XVisualInfo*       pVisualInfo        = nullptr;
    Window             windowId           = None;
    Atom               atomDelete         = None;
    static const char* WIN_MGR_DELETE_MSG = {"WM_DELETE_WINDOW"};
    XEvent*            pEvent             = nullptr;
    Colormap           colorMap;
    int                x;
    int                y;
    unsigned           w;
    unsigned           h;
    unsigned           borderWidth;
    unsigned           depth;
    Window             root;

    const auto windowError = [&](const char* errMsg) -> int
    {
        LS_LOG_ERR(errMsg);
        if (pEvent)
        {
            delete pEvent;
        }

        if (windowId)
        {
            XDestroyWindow(pDisplay, windowId);
        }

        if (pVisualInfo)
        {
            XFree(pVisualInfo);
        }

        if (pDisplay)
        {
            XCloseDisplay(pDisplay);
        }

        return errCode;
    };

    static constexpr int XLIB_EVENT_MASK =
        0
        | KeyPressMask
        | KeyReleaseMask
        | KeymapStateMask
        | StructureNotifyMask
        | SubstructureNotifyMask
        | ExposureMask
        | PointerMotionMask
        | ButtonPressMask
        | ButtonReleaseMask
        | ButtonMotionMask
        | EnterWindowMask
        | LeaveWindowMask
        | VisibilityChangeMask
        | FocusChangeMask
        | OwnerGrabButtonMask
        | 0;

    assert(!this->valid());

    utils::fast_memset(&visualTemplate, 0, sizeof(XVisualInfo));
    utils::fast_memset(&windowAttribs, 0, sizeof(XSetWindowAttributes));

    LS_LOG_MSG("SR_RenderWindowXlib ", this, " initializing");
    {
        LS_LOG_MSG("Connecting to X display \"", pDisplayName, "\".");
        pDisplay = XOpenDisplay(pDisplayName);
        if (!pDisplay)
        {
            errCode = -1;
            return windowError("\tUnable to connect to the X server.");
        }
        LS_LOG_MSG("\tDone.");
    }
    {
    LS_LOG_MSG("Querying X server for display configuration.");
        int numVisuals;
        visualTemplate.screen       = DefaultScreen(pDisplay);
        visualTemplate.depth        = 24;
        visualTemplate.c_class      = TrueColor;
        visualTemplate.red_mask     = 0x00FF0000;
        visualTemplate.green_mask   = 0x0000FF00;
        visualTemplate.blue_mask    = 0x000000FF;
        visualTemplate.bits_per_rgb = 8;

        pVisualInfo = XGetVisualInfo(pDisplay, visualMatchMask, &visualTemplate, &numVisuals);
        if (!pVisualInfo)
        {
            errCode = -2;
            return windowError("\tFailed to get display information from the X server.");
        }
        LS_LOG_MSG(
            "\tDone. Retrieved ", numVisuals, " configurations Using the default:"
            "\n\t\tConfig ID:      ", pVisualInfo->visualid,
            "\n\t\tScreen ID:      ", pVisualInfo->screen,
            "\n\t\tBit Depth:      ", pVisualInfo->depth,
            "\n\t\tRed Bits:       ", pVisualInfo->red_mask,
            "\n\t\tGreen Bits:     ", pVisualInfo->green_mask,
            "\n\t\tBlue bits:      ", pVisualInfo->blue_mask,
            "\n\t\tColorMap Size:  ", pVisualInfo->colormap_size,
            "\n\t\tBits per Pixel: ", pVisualInfo->bits_per_rgb);
    }
    {
        LS_LOG_MSG("Configuring X window attributes.");

        colorMap = XCreateColormap(pDisplay, RootWindow(pDisplay, visualTemplate.screen), pVisualInfo->visual, AllocNone);

        windowAttribs.background_pixel  = 0x0; // black
        windowAttribs.border_pixel      = 0;
        windowAttribs.save_under        = False;
        windowAttribs.event_mask        = XLIB_EVENT_MASK;
        windowAttribs.override_redirect = True;
        windowAttribs.colormap          = colorMap;

        windowId = XCreateWindow(
            pDisplay,
            RootWindow(pDisplay, pVisualInfo->screen),
            0, // x
            0, // y
            width,
            height,
            0, // border_width
            pVisualInfo->depth,
            InputOutput,
            pVisualInfo->visual,
            CWBackPixel | CWBorderPixel | CWEventMask | CWColormap,
            &windowAttribs
        );

        if (!windowId)
        {
            errCode = -3;
            return windowError("\tFailed to create X window from display.");
        }

        LS_LOG_MSG("\tCreated window ", windowId, '.');

        XSelectInput(pDisplay, windowId, XLIB_EVENT_MASK);
        atomDelete = XInternAtom(pDisplay, WIN_MGR_DELETE_MSG, False);
        XSetWMProtocols(pDisplay, windowId, &atomDelete, 1);
        XMapWindow(pDisplay, windowId);

        if (atomDelete == None)
        {
            errCode = -4;
            return windowError("\tUnable to request client-side window deletion from X server.");
        }

        pEvent = new(std::nothrow) XEvent;
        if (!pEvent)
        {
            errCode = -5;
            return windowError("\tUnable to allocate memory for event-handling.");
        }
        pEvent->type = None;

        LS_LOG_MSG("\tDone.");
    }

    XFlush(pDisplay);
    XFree(pVisualInfo);

    {
        LS_LOG_MSG("Inspecting window for dimensions.");
        if (XGetGeometry(pDisplay, windowId, &root, &x, &y, &w, &h, &borderWidth, &depth) != True)
        {
            errCode = -6;
            return windowError("Unable to retrieve dimensions of a new window.");
        }
        _xlib_get_position(pDisplay, windowId, x, y);
        LS_LOG_MSG("\tSuccessfully created a window through X11.");
    }

    mCurrentState = WindowStateInfo::WINDOW_STARTED;
    mDisplay      = pDisplay;
    mWindow       = windowId;
    mCloseAtom    = atomDelete;
    mLastEvent    = pEvent;
    mKeysRepeat   = (XkbSetDetectableAutoRepeat(mDisplay, False, nullptr) == False);
    mWidth        = w;
    mHeight       = h;
    mX            = x;
    mY            = y;
    mMouseX       = 0;
    mMouseY       = 0;

    LS_LOG_MSG(
        "Done. Successfully initialized SR_RenderWindowXlib ", this, '.',
        "\n\tDisplay:    ", pDisplayName,
        "\n\tWindow ID:  ", mWindow,
        "\n\tResolution: ", mWidth, 'x', mHeight,
        "\n\tDPI/Scale:  ", this->dpi(),
        "\n\tPosition:   ", mX, 'x', mY);
    return 0;
}



/*-------------------------------------
 * Window Destructon/Close
-------------------------------------*/
int SR_RenderWindowXlib::destroy() noexcept
{
    if (mWindow != None)
    {
        XDestroyWindow(mDisplay, mWindow);
        mWindow = None;
        mCloseAtom = None;

        delete mLastEvent;
        mLastEvent = nullptr;

        mWidth = 0;
        mHeight = 0;
        mX = 0;
        mY = 0;
        mMouseX = 0;
        mMouseY = 0;

        mKeysRepeat = true;
        mCaptureMouse = false;
    }

    if (mDisplay)
    {
        XCloseDisplay(mDisplay);
        mDisplay = nullptr;
    }

    mCurrentState = WindowStateInfo::WINDOW_CLOSED;

    return 0;
}



/*-------------------------------------
 * Set the window size
-------------------------------------*/
bool SR_RenderWindowXlib::set_size(unsigned width, unsigned height) noexcept
{
    assert(width <= std::numeric_limits<uint16_t>::max());
    assert(height <= std::numeric_limits<uint16_t>::max());

    if (!valid() || !width || !height)
    {
        return false;
    }

    if (mWidth == width && mHeight == height)
    {
        LS_LOG_ERR("Window size unchanged.");
        return true;
    }

    if (Success == XResizeWindow(mDisplay, mWindow, width, height))
    {
        mWidth = width;
        mHeight = height;
        return true;
    }

    return false;
}



/*-------------------------------------
 * Set the window position
-------------------------------------*/
bool SR_RenderWindowXlib::set_position(int x, int y) noexcept
{
    if (!valid())
    {
        return false;
    }

    if (mX == x && mY == y)
    {
        LS_LOG_ERR("Window position unchanged.");
        return true;
    }

    if (Success != XMoveWindow(mDisplay, mWindow, x, y))
    {
        return false;
    }

    mX = x;
    mY = y;

    return true;
}



/*-------------------------------------
 * Clone/Duplicate a window
-------------------------------------*/
SR_RenderWindow* SR_RenderWindowXlib::clone() const noexcept
{
    const SR_RenderWindowXlib& self = *this; // nullptr check

    SR_RenderWindowXlib* pWindow = new(std::nothrow) SR_RenderWindowXlib(self);

    return pWindow;
}



/*-------------------------------------
 * Check if the window is open
-------------------------------------*/
bool SR_RenderWindowXlib::valid() const noexcept
{
    return mWindow != None;
}



/*-------------------------------------
 * Run the window's event queue
-------------------------------------*/
void SR_RenderWindowXlib::update() noexcept
{
    // sanity check
    if (!valid())
    {
        return;
    }

    // error check
    int evtStatus = Success;

    switch (mCurrentState)
    {
        // The window was starting to close in the last frame. Destroy it now
        case WindowStateInfo::WINDOW_CLOSING:
            destroy();
            break;

        case WindowStateInfo::WINDOW_STARTED:
            // fall-through
            run();

        case WindowStateInfo::WINDOW_RUNNING:
            // Perform a non-blocking poll if we're not paused
            if (XPending(mDisplay) == 0)
            {
                mLastEvent->type = None;

                // warp the mouse only if there are no other pending events.
                // Otherwise, performance falls to the point where the event
                // loop can't even run.
                if (mCaptureMouse)
                {
                    XWarpPointer(mDisplay, None, mWindow, 0, 0, mWidth, mHeight, mWidth / 2, mHeight / 2);
                }
                break;
            }
            // fall-through

        case WindowStateInfo::WINDOW_PAUSED:
            // Make sure keys don't repeat when requested.
            if (!mKeysRepeat && mLastEvent->type == KeyRelease && XEventsQueued(mDisplay, QueuedAlready))
            {
                XEvent nev;
                XPeekEvent(mDisplay, &nev);

                if (nev.type == KeyPress && nev.xkey.time == mLastEvent->xkey.time &&
                    nev.xkey.keycode == mLastEvent->xkey.keycode)
                {
                    /* Key wasn’t actually released */
                    break;
                }
            }

            // Perform a blocking event check while the window is paused.
            evtStatus = XNextEvent(mDisplay, mLastEvent);

            // Ignore when the mouse goes to the center of the window when
            // mouse capturing is enabled. The center of the window is where
            // the mouse is supposed to rest but resetting the mouse position
            // causes the event queue to fill up with MotionNotify events.
            if (mLastEvent->type == MotionNotify && mCaptureMouse)
            {
                const XMotionEvent& motion = mLastEvent->xmotion;
                if (motion.x == (int)mWidth/2 && motion.y == (int)mHeight/2)
                {
                    mLastEvent->type = None;
                }
            }

            break;

        default:
            // We should not be in a "starting" or "closed" state
            LS_LOG_ERR("Encountered unexpected window state ", mCurrentState, '.');
            assert(false); // assertions are disabled on release builds
            mCurrentState = WindowStateInfo::WINDOW_CLOSING;
            break;
    }

    if (evtStatus != Success)
    {
        LS_LOG_ERR("X server connection error. Shutting down X connection.");
        mCurrentState = WindowStateInfo::WINDOW_CLOSING;
        destroy();
    }
}



/*-------------------------------------
 * Pause the window (run in interrupt mode)
-------------------------------------*/
bool SR_RenderWindowXlib::pause() noexcept
{
    // state should only be changed for running windows
    // Otherwise, the window is either starting or stopping
    if (!valid())
    {
        return false;
    }

    // Use a switch statement so the compiler can provide a warning in case
    // a state isn't being properly handled.
    switch (mCurrentState)
    {
        // Only these cases can be used to go into a paused state
        case WindowStateInfo::WINDOW_STARTED:
            XFlush(mDisplay);
        case WindowStateInfo::WINDOW_RUNNING:
        case WindowStateInfo::WINDOW_PAUSED:
        case WindowStateInfo::WINDOW_CLOSING:
            mCurrentState = WindowStateInfo::WINDOW_PAUSED;
            break;

            // These states can't be used to transition to a paused state
        case WindowStateInfo::WINDOW_CLOSED:
        case WindowStateInfo::WINDOW_STARTING:
            assert(false); // fail in case of error
            break;
    }

    return mCurrentState == WindowStateInfo::WINDOW_PAUSED;
}



/*-------------------------------------
 * Run the window (set to polling mode)
-------------------------------------*/
bool SR_RenderWindowXlib::run() noexcept
{
    // state should only be changed for running windows
    // Otherwise, the window is either starting or stopping
    if (!valid())
    {
        return false;
    }

    // Use a switch statement so the compiler can provide a warning in case
    // a state isn't being properly handled.
    switch (mCurrentState)
    {
        // Only these cases can be used to go into a paused state
        case WindowStateInfo::WINDOW_STARTED:
            XFlush(mDisplay);
        case WindowStateInfo::WINDOW_CLOSING:
        case WindowStateInfo::WINDOW_RUNNING:
        case WindowStateInfo::WINDOW_PAUSED:
            mCurrentState = WindowStateInfo::WINDOW_RUNNING;
            break;

            // These states can't be used to transition to a paused state
        case WindowStateInfo::WINDOW_CLOSED:
        case WindowStateInfo::WINDOW_STARTING:
            assert(false); // fail in case of error
            break;
    }

    return mCurrentState == WindowStateInfo::WINDOW_RUNNING;
}



/*-------------------------------------
 * Check if there's an event available
-------------------------------------*/
bool SR_RenderWindowXlib::has_event() const noexcept
{
    return mLastEvent && mLastEvent->type != None;
}



/*-------------------------------------
 * Check the next event within the event queue
-------------------------------------*/
bool SR_RenderWindowXlib::peek_event(SR_WindowEvent* const pEvent) noexcept
{
    utils::fast_memset(pEvent, '\0', sizeof(SR_WindowEvent));
    if (!has_event())
    {
        return false;
    }

    XKeyEvent* pKey;
    XButtonEvent* pButton;
    XMotionEvent* pMotion;
    XExposeEvent* pExpose;
    XDestroyWindowEvent* pDestroy;
    XClientMessageEvent* pMessage;
    XCrossingEvent* pCross;
    XConfigureEvent* pConfig;

    unsigned keyMods = 0;
    KeySym keySym;

    switch (mLastEvent->type)
    {
        case None: // sentinel
            pEvent->type = SR_WinEventType::WIN_EVENT_NONE;
            break;

        case Expose:
            pExpose = &mLastEvent->xexpose;
            if (pExpose->count == 0)
            {
                pEvent->type = WIN_EVENT_EXPOSED;
                pEvent->pNativeWindow = pExpose->window;
                pEvent->window.x = (uint16_t)pExpose->x;
                pEvent->window.y = (uint16_t)pExpose->y;
                pEvent->window.width = (uint16_t)pExpose->width;
                pEvent->window.height = (uint16_t)pExpose->height;
            }
            break;

        case KeymapNotify:
            XRefreshKeyboardMapping(&mLastEvent->xmapping);
            pEvent->pNativeWindow = mLastEvent->xmapping.window;
            pEvent->type = SR_WinEventType::WIN_EVENT_NONE;
            break;

        case KeyPress:
            pKey = &mLastEvent->xkey;

            // Additional key processing is only performed in text-mode
            XkbLookupKeySym(mDisplay, pKey->keycode, pKey->state, &keyMods, &keySym);
            pEvent->type = WIN_EVENT_KEY_DOWN;
            pEvent->pNativeWindow = pKey->window;
            pEvent->keyboard.keysym = (SR_KeySymbol)keySym;
            pEvent->keyboard.key = mKeysRepeat ? 0 : (uint8_t)pKey->keycode; // only get key names in text mode
            pEvent->keyboard.capsLock = (uint8_t)((pKey->state & LockMask) > 0);
            pEvent->keyboard.numLock = (uint8_t)((pKey->state & Mod2Mask) > 0);
            pEvent->keyboard.scrollLock = (uint8_t)((pKey->state & Mod3Mask) > 0);
            break;

        case KeyRelease:
            pKey = &mLastEvent->xkey;

            // Additional key processing is only performed in text-mode
            XkbLookupKeySym(mDisplay, pKey->keycode, pKey->state, &keyMods, &keySym);
            pEvent->type = WIN_EVENT_KEY_UP;
            pEvent->pNativeWindow = pKey->window;
            pEvent->keyboard.keysym = (SR_KeySymbol)keySym;
            pEvent->keyboard.key = mKeysRepeat ? 0 : (uint8_t)pKey->keycode; // only get key names in text mode
            pEvent->keyboard.capsLock = (uint8_t)((pKey->state & LockMask) > 0);
            pEvent->keyboard.numLock = (uint8_t)((pKey->state & Mod2Mask) > 0);
            pEvent->keyboard.scrollLock = (uint8_t)((pKey->state & Mod3Mask) > 0);
            break;

        case ButtonPress:
            pButton = &mLastEvent->xbutton;
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_DOWN;
            pEvent->pNativeWindow = pButton->window;

            switch (pButton->button)
            {
                case Button1:
                    pEvent->mouseButton.mouseButton1 = 1;
                    pEvent->mouseButton.x = (int16_t)pButton->x;
                    pEvent->mouseButton.y = (int16_t)pButton->y;
                    break;

                case Button2:
                    pEvent->mouseButton.mouseButton2 = 1;
                    pEvent->mouseButton.x = (int16_t)pButton->x;
                    pEvent->mouseButton.y = (int16_t)pButton->y;
                    break;

                case Button3:
                    pEvent->mouseButton.mouseButton3 = 1;
                    pEvent->mouseButton.x = (int16_t)pButton->x;
                    pEvent->mouseButton.y = (int16_t)pButton->y;
                    break;

                case Button4:
                case Button5:
                    pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_WHEEL_MOVED;
                    pEvent->wheel.x = (int16_t)pButton->x;
                    pEvent->wheel.y = (int16_t)pButton->y;
                    pEvent->wheel.up = (pButton->button == Button4);
                    pEvent->wheel.down = (pButton->button == Button5);
                    break;

                default:
                    pEvent->mouseButton.mouseButtonN = (uint8_t)pButton->button;
                    pEvent->mouseButton.x = (int16_t)pButton->x;
                    pEvent->mouseButton.y = (int16_t)pButton->y;
                    break;
            }
            break;

        case ButtonRelease:
            pButton = &mLastEvent->xbutton;
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_UP;
            pEvent->pNativeWindow = pButton->window;

            switch (pButton->button)
            {
                case Button1:
                    pEvent->mouseButton.mouseButton1 = 1;
                    pEvent->mouseButton.x = (int16_t)pButton->x;
                    pEvent->mouseButton.y = (int16_t)pButton->y;
                    break;

                case Button2:
                    pEvent->mouseButton.mouseButton2 = 1;
                    pEvent->mouseButton.x = (int16_t)pButton->x;
                    pEvent->mouseButton.y = (int16_t)pButton->y;
                    break;

                case Button3:
                    pEvent->mouseButton.mouseButton3 = 1;
                    pEvent->mouseButton.x = (int16_t)pButton->x;
                    pEvent->mouseButton.y = (int16_t)pButton->y;
                    break;

                case Button4:
                case Button5:
                    // Buttons 4 & 5 correspond to the mouse wheel. We only update those
                    // then "pressed" or the mouse wheel has scrolled.
                    break;

                default:
                    pEvent->mouseButton.mouseButtonN = (uint8_t)pButton->button;
                    pEvent->mouseButton.x = (int16_t)pButton->x;
                    pEvent->mouseButton.y = (int16_t)pButton->y;
                    break;
            }
            break;

        case MotionNotify:
            pMotion = &mLastEvent->xmotion;
            pEvent->type = WIN_EVENT_MOUSE_MOVED;
            pEvent->pNativeWindow = pMotion->window;
            pEvent->mousePos.x = (int16_t)pMotion->x;
            pEvent->mousePos.y = (int16_t)pMotion->y;

            if (!mCaptureMouse)
            {
                pEvent->mousePos.dx = (int16_t)(mMouseX - pMotion->x);
                pEvent->mousePos.dy = (int16_t)(mMouseY - pMotion->y);
                mMouseX = pMotion->x;
                mMouseY = pMotion->y;

            }
            else
            {
                const int w2 = mWidth / 2;
                const int h2 = mHeight / 2;
                const int dx = pMotion->x;
                const int dy = pMotion->y;
                pEvent->mousePos.dx = (int16_t)(w2 - dx);
                pEvent->mousePos.dy = (int16_t)(h2 - dy);
                mMouseX = dx;
                mMouseY = dy;
            }
            break;

        case EnterNotify:
            pCross = &mLastEvent->xcrossing;
            pEvent->pNativeWindow = pCross->window;
            pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_ENTER;
            pEvent->mousePos.x = (int16_t)pCross->x;
            pEvent->mousePos.y = (int16_t)pCross->y;
            break;

        case LeaveNotify:
            pCross = &mLastEvent->xcrossing;
            pEvent->pNativeWindow = pCross->window;
            pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_LEAVE;
            pEvent->mousePos.x = (int16_t)pCross->x;
            pEvent->mousePos.y = (int16_t)pCross->y;
            break;

        case ClientMessage:
            pMessage = &mLastEvent->xclient;
            if ((Atom)pMessage->data.l[0] == (Atom)mCloseAtom)
            {
                mCurrentState = WindowStateInfo::WINDOW_CLOSING;
                pEvent->pNativeWindow = pMessage->window;
            }
            break;

        case DestroyNotify:
            pDestroy = &mLastEvent->xdestroywindow;
            mCurrentState = WindowStateInfo::WINDOW_CLOSING;
            pEvent->type = SR_WinEventType::WIN_EVENT_CLOSING;
            pEvent->pNativeWindow = pDestroy->window;
            break;

        case ConfigureNotify:
            pConfig = &mLastEvent->xconfigure;
            pEvent->pNativeWindow = pConfig->window;

            if (mX != pConfig->x || mY != pConfig->y)
            {
                pEvent->type = SR_WinEventType::WIN_EVENT_MOVED;
                mX = pConfig->x;
                mY = pConfig->y;
                pEvent->window.x = (int16_t)pConfig->x;
                pEvent->window.y = (int16_t)pConfig->y;
                break;
            }
            else if (mWidth != (unsigned)pConfig->width || mHeight != (unsigned)pConfig->height)
            {
                pEvent->type = SR_WinEventType::WIN_EVENT_RESIZED;
                mWidth = (unsigned)pConfig->width;
                mHeight = (unsigned)pConfig->height;
                pEvent->window.width = (uint16_t)pConfig->width;
                pEvent->window.height = (uint16_t)pConfig->height;
                break;
            }

        default: // unhandled event
            pEvent->type = SR_WinEventType::WIN_EVENT_UNKNOWN;
            return false;
    }

    return true;
}



/*-------------------------------------
 * Remove an event from the event queue
-------------------------------------*/
bool SR_RenderWindowXlib::pop_event(SR_WindowEvent* const pEvent) noexcept
{
    const bool ret = peek_event(pEvent);
    if (mLastEvent)
    {
        mLastEvent->type = None;
    }

    return ret;
}



/*-------------------------------------
 * Enable or disable repeating keys
-------------------------------------*/
bool SR_RenderWindowXlib::set_keys_repeat(bool doKeysRepeat) noexcept
{
    mKeysRepeat = XkbSetDetectableAutoRepeat(mDisplay, !doKeysRepeat, nullptr) == False;

    return mKeysRepeat;
}



/*-------------------------------------
 * Render a framebuffer to the current window
-------------------------------------*/
void SR_RenderWindowXlib::render(SR_WindowBuffer& buffer) noexcept
{
    assert(this->valid());
    assert(buffer.native_handle() != nullptr);

    #if SR_ENABLE_XSHM != 0
        XShmPutImage(
            mDisplay,
            mWindow,
            DefaultGC(mDisplay, DefaultScreen(mDisplay)),
            reinterpret_cast<XImage*>(buffer.native_handle()),
            0, 0,
            0, 0,
            width(),
            height(),
            False
        );
    #else
        XPutImage(
            mDisplay,
            mWindow,
            DefaultGC(mDisplay, DefaultScreen(mDisplay)),
            reinterpret_cast<XImage*>(buffer.native_handle()),
            0, 0,
            0, 0,
            width(),
            height()
        );
    #endif /* SR_ENABLE_XSHM */
}




/*-------------------------------------
 * Mouse Grabbing
-------------------------------------*/
void SR_RenderWindowXlib::set_mouse_capture(bool isCaptured) noexcept
{
    if (valid())
    {
        mCaptureMouse = isCaptured;
    }
    else
    {
        mCaptureMouse = false;
    }

    if (mCaptureMouse)
    {
        constexpr unsigned captureFlags =
        0
        | ButtonPressMask
        | ButtonReleaseMask
        | PointerMotionMask
        | FocusChangeMask
        | 0;

        XGrabPointer(mDisplay, mWindow, False, captureFlags, GrabModeAsync, GrabModeAsync, mWindow, None, CurrentTime);
    }
    else
    {
        XUngrabPointer(mDisplay, CurrentTime);
    }
}



/*-------------------------------------
 * Get the current scaling factor for the display
-------------------------------------*/
unsigned SR_RenderWindowXlib::dpi() const noexcept
{
    int screenId = DefaultScreen(mDisplay);
    const float displayInches = (float)DisplayWidth(mDisplay, screenId) * 25.4f;
    const float widthMM = (float)DisplayWidthMM(mDisplay, screenId);

    return (unsigned)(displayInches / widthMM + 0.5f); // round before truncate
}
