
#include <cassert> // assert
#include <cstdlib> // std::getenv
#include <cstring> // std::strlen
#include <limits> // numeric_limits<>
#include <memory> // std::move
#include <new> // std::nothrow

// XLib should use an un-mangled interface
extern "C"
{
    #include <xcb/xcb.h>
    #include <xcb/xproto.h>
    #include <X11/Xlib-xcb.h>
    #include <X11/XKBlib.h> // XkbKeycodeToKeysym

    #if SR_ENABLE_XSHM != 0
        #include <xcb/shm.h>
        #include <xcb/xcb_image.h>
    #endif
}

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

#include "soft_render/SR_WindowBufferXCB.hpp"
#include "soft_render/SR_RenderWindowXCB.hpp"
#include "soft_render/SR_WindowEvent.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous helper functions
-----------------------------------------------------------------------------*/
namespace utils = ls::utils;



namespace
{

inline LS_INLINE int _get_xcb_event(const void* pEvent) noexcept
{
    return pEvent ? (~0x80 & reinterpret_cast<const xcb_generic_event_t*>(pEvent)->response_type) : XCB_NONE;
}

} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SR_RenderWindowXCB
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_RenderWindowXCB::~SR_RenderWindowXCB() noexcept
{
    if (this->valid() && destroy() != 0)
    {
        LS_LOG_ERR("Unable to properly close the render window ", this, " during destruction.");
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_RenderWindowXCB::SR_RenderWindowXCB() noexcept :
    SR_RenderWindow{},
    mDisplay{nullptr},
    mConnection{nullptr},
    mWindow{0},
    mContext{0},
    mCloseAtom{0},
    mLastEvent{nullptr},
    mPeekedEvent{nullptr},
    mWidth{0},
    mHeight{0},
    mX{0},
    mY{0},
    mMouseX{0},
    mMouseY{0},
    mKeysRepeat{true},
    mCaptureMouse{false}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SR_RenderWindowXCB::SR_RenderWindowXCB(const SR_RenderWindowXCB& rw) noexcept :
    SR_RenderWindowXCB{} // delegate
{
    // delegate some more
    *this = rw;
}


/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_RenderWindowXCB::SR_RenderWindowXCB(SR_RenderWindowXCB&& rw) noexcept :
    SR_RenderWindow{std::move(rw)},
    mDisplay{rw.mDisplay},
    mConnection{rw.mConnection},
    mWindow{rw.mWindow},
    mContext{rw.mContext},
    mCloseAtom{rw.mCloseAtom},
    mLastEvent{rw.mLastEvent},
    mPeekedEvent{rw.mPeekedEvent},
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
    rw.mConnection = nullptr;
    rw.mWindow = 0;
    rw.mContext = 0;
    rw.mCloseAtom = 0;
    rw.mLastEvent = nullptr;
    rw.mPeekedEvent = nullptr;
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
SR_RenderWindowXCB& SR_RenderWindowXCB::operator=(const SR_RenderWindowXCB& rw) noexcept
{
    if (this == &rw)
    {
        return *this;
    }

    this->destroy();

    SR_RenderWindowXCB* const pWindow = static_cast<SR_RenderWindowXCB*>(rw.clone());

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
SR_RenderWindowXCB& SR_RenderWindowXCB::operator=(SR_RenderWindowXCB&& rw) noexcept
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

    mDisplay = rw.mDisplay;
    rw.mDisplay = nullptr;

    this->mConnection = rw.mConnection;
    rw.mConnection = nullptr;

    this->mWindow = rw.mWindow;
    rw.mWindow = 0;

    this->mContext = rw.mContext;
    rw.mContext = 0;

    this->mCloseAtom = rw.mCloseAtom;
    rw.mCloseAtom = 0;

    this->mLastEvent = rw.mLastEvent;
    rw.mLastEvent = nullptr;

    this->mPeekedEvent = rw.mPeekedEvent;
    rw.mPeekedEvent = nullptr;

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
int SR_RenderWindowXCB::set_title(const char* const pName) noexcept
{
    if (!valid())
    {
        return -1;
    }

    xcb_void_cookie_t result = xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindow, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(pName), pName);

    if (xcb_request_check(mConnection, result))
    {
        return -2;
    }

    return 0;
}



/*-------------------------------------
 * Window Initialization
-------------------------------------*/
int SR_RenderWindowXCB::init(unsigned width, unsigned height) noexcept
{
    const char* const         pDisplayName         = std::getenv("DISPLAY");
    int                       errCode              = 0;
    Display*                  pDisplay             = nullptr;
    xcb_connection_t*         pConnection          = nullptr;
    const xcb_setup_t*        pSetup               = nullptr;
    xcb_screen_iterator_t     screenIter;
    xcb_screen_t*             pScreen;
    xcb_window_t              windowId             = 0;
    xcb_gcontext_t            context              = 0;
    Atom                      atomDelete           = None;
    static const char*        WIN_MGR_DELETE_MSG   = {"WM_DELETE_WINDOW"};
    xcb_generic_event_t*      pEvent               = nullptr;
    xcb_get_geometry_cookie_t geomCookie;
    xcb_get_geometry_reply_t* pGeom                = nullptr;

    static constexpr uint32_t eventMask =
        0
        | XCB_EVENT_MASK_KEY_PRESS
        | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_PROPERTY_CHANGE
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        | XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_BUTTON_MOTION
        | XCB_EVENT_MASK_ENTER_WINDOW
        | XCB_EVENT_MASK_LEAVE_WINDOW
        | XCB_EVENT_MASK_VISIBILITY_CHANGE
        | XCB_EVENT_MASK_FOCUS_CHANGE
        | XCB_EVENT_MASK_OWNER_GRAB_BUTTON
        | 0;

    const auto windowError = [&](const char* errMsg) -> int
    {
        LS_LOG_ERR(errMsg);
        if (pGeom)
        {
            free(pGeom);
        }

        if (pEvent)
        {
            delete pEvent;
        }

        if (windowId)
        {
            xcb_destroy_window(pConnection, windowId);
        }

        if (pConnection)
        {
            xcb_disconnect(pConnection);
        }

        if (pDisplay)
        {
            XCloseDisplay(pDisplay);
        }

        return errCode;
    };

    assert(!this->valid());

    LS_LOG_MSG("SR_RenderWindowXCB ", this, " initializing");
    {
        LS_LOG_MSG("Creating XCB connection to \"", pDisplayName, "\".");
        pDisplay = XOpenDisplay(pDisplayName);
        if (!pDisplay)
        {
            errCode = -1;
            return windowError("\tUnable to connect to the X server.");
        }

        pConnection = XGetXCBConnection(pDisplay);
        if (!pConnection)
        {
            errCode = -1;
            return windowError("\tUnable to create an XCB connection.");
        }
        LS_LOG_MSG("\tDone.");
    }
    {
        LS_LOG_MSG("Configuring XCB screen.");

        pSetup = xcb_get_setup(pConnection);
        if (!pSetup)
        {
            errCode = -2;
            return windowError("\tUnable to setup the XCB screen.");
        }

        screenIter = xcb_setup_roots_iterator(pSetup);
        pScreen = screenIter.data;

        if (!pScreen)
        {
            errCode = -3;
            return windowError("\tFailed to locate a screen for opening a window.");
        }
        LS_LOG_MSG("\tDone.");
    }
    {
        LS_LOG_MSG("Configuring X window attributes.");
        windowId = xcb_generate_id(pConnection);

        if (!windowId)
        {
            errCode = -4;
            return windowError("\tFailed to generate an XCB window ID.");
        }
        LS_LOG_MSG("\tCreated window ID ", windowId, '.');

        xcb_create_window(pConnection, XCB_COPY_FROM_PARENT, windowId, pScreen->root, 0, 0, (uint16_t)width, (uint16_t)height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, pScreen->root_visual, XCB_CW_EVENT_MASK, &eventMask);
        LS_LOG_MSG("\tDone.");
    }
    xcb_map_window(pConnection, windowId);
    xcb_flush(pConnection);

    {
        LS_LOG_MSG("Generating a graphics context.");
        context = xcb_generate_id(pConnection);
        if (!context)
        {
            errCode = -5;
            return windowError("\tFailed to create a graphics context.");
        }
        xcb_create_gc(pConnection, context, windowId, XCB_GC_FOREGROUND, &pScreen->black_pixel);
        LS_LOG_MSG("\tDone.");
    }
    {
        LS_LOG_MSG("Inspecting window for dimensions.");
        geomCookie = xcb_get_geometry(pConnection, windowId);
        pGeom = xcb_get_geometry_reply(pConnection, geomCookie, nullptr);
        if (!pGeom)
        {
            errCode = -6;
            return windowError("Unable to retrieve dimensions of a new window.");
        }

        LS_LOG_MSG("\tDone.");
    }
    {
        LS_LOG_MSG("Setting up window-close detection.");

        atomDelete = XInternAtom(pDisplay, WIN_MGR_DELETE_MSG, False);
        XSetWMProtocols(pDisplay, windowId, &atomDelete, 1);

        if (!atomDelete)
        {
            errCode = -7;
            return windowError("\tUnable to request client-side window deletion from X server.");
        }
        LS_LOG_MSG("\tDone.");
    }

    xcb_flush(pConnection);

    mCurrentState = WindowStateInfo::WINDOW_STARTED;
    mDisplay      = pDisplay;
    mConnection   = pConnection;
    mWindow       = windowId;
    mContext      = context;
    mCloseAtom    = atomDelete;
    mLastEvent    = nullptr;
    mPeekedEvent  = nullptr;
    mKeysRepeat   = false;
    mWidth        = pGeom->width;
    mHeight       = pGeom->height;
    mX            = pGeom->x;
    mY            = pGeom->y;
    mMouseX       = 0;
    mMouseY       = 0;

    // Thanks Valgrind!
    free(pGeom);

    LS_LOG_MSG(
        "Done. Successfully initialized SR_RenderWindowXCB ", this, '.',
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
int SR_RenderWindowXCB::destroy() noexcept
{
    if (mWindow)
    {
        xcb_destroy_window(mConnection, mWindow);
        mWindow = 0;

        //free(mCloseAtom);
        //mCloseAtom = nullptr;
        mCloseAtom = 0;

        mContext = 0;

        if (mConnection)
        {
            //xcb_disconnect(mConnection);
            mConnection = nullptr;
        }

        if (mDisplay)
        {
            XCloseDisplay(mDisplay);
            mDisplay = nullptr;
        }

        free(mLastEvent);
        mLastEvent = nullptr;

        free(mPeekedEvent);
        mPeekedEvent = nullptr;

        mWidth = 0;
        mHeight = 0;
        mX = 0;
        mY = 0;
        mMouseX = 0;
        mMouseY = 0;

        mKeysRepeat = true;
        mCaptureMouse = false;
    }

    mCurrentState = WindowStateInfo::WINDOW_CLOSED;

    return 0;
}



/*-------------------------------------
 * Set the window size
-------------------------------------*/
bool SR_RenderWindowXCB::set_size(unsigned w, unsigned h) noexcept
{
    assert(w <= std::numeric_limits<uint16_t>::max());
    assert(h <= std::numeric_limits<uint16_t>::max());

    if (!valid() || !w || !h)
    {
        return false;
    }

    if (mWidth == w && mHeight == h)
    {
        LS_LOG_ERR("Window size unchanged.");
        return true;
    }

    const uint32_t dimens[] = {(uint32_t)w, (uint32_t)h};
    xcb_void_cookie_t result = xcb_configure_window(mConnection, mWindow, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, dimens);

    xcb_flush(mConnection);
    if (xcb_request_check(mConnection, result))
    {
        return false;
    }

    return true;
}



/*-------------------------------------
 * Set the window position
-------------------------------------*/
bool SR_RenderWindowXCB::set_position(int x, int y) noexcept
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

    const uint32_t pos[] = {(uint32_t)x, (uint32_t)y};
    xcb_void_cookie_t result = xcb_configure_window(mConnection, mWindow, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, pos);

    xcb_flush(mConnection);
    if (xcb_request_check(mConnection, result))
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
SR_RenderWindow* SR_RenderWindowXCB::clone() const noexcept
{
    const SR_RenderWindowXCB& self = *this; // nullptr check

    SR_RenderWindowXCB* pWindow = new(std::nothrow) SR_RenderWindowXCB(self);

    return pWindow;
}



/*-------------------------------------
 * Check if the window is open
-------------------------------------*/
bool SR_RenderWindowXCB::valid() const noexcept
{
    return mWindow != 0;
}



/*-------------------------------------
 * Run the window's event queue
-------------------------------------*/
void SR_RenderWindowXCB::update() noexcept
{
    // sanity check
    if (!valid())
    {
        return;
    }

    if (mLastEvent)
    {
        free(mLastEvent);
        mLastEvent = nullptr;
    }

    // The window was starting to close in the last frame. Destroy it now
    if (mCurrentState == WindowStateInfo::WINDOW_CLOSING)
    {
        destroy();
        return;
    }

    if (mCurrentState == WindowStateInfo::WINDOW_STARTED)
    {
        run();
    }

    if (mCurrentState == WindowStateInfo::WINDOW_RUNNING)
    {
        mLastEvent = mPeekedEvent ? mPeekedEvent : xcb_poll_for_event(mConnection);
    }
    else if (mCurrentState == WindowStateInfo::WINDOW_PAUSED)
    {
        mLastEvent = mPeekedEvent ? mPeekedEvent : xcb_wait_for_event(mConnection);
    }

    mPeekedEvent = nullptr;

    // Warp the mouse only if there are no other pending events.
    // Otherwise, performance falls to the point where the event
    // loop can't even run.
    if (!mLastEvent && mCaptureMouse)
    {
        xcb_request_check(mConnection, xcb_warp_pointer_checked(mConnection, 0, mWindow, 0, 0, mWidth, mHeight, mWidth / 2, mHeight / 2));
    }

    // Make sure keys don't repeat when requested.
    if (!mKeysRepeat && _get_xcb_event(mLastEvent) == XCB_KEY_PRESS)
    {
        mPeekedEvent = xcb_poll_for_event(mConnection);
        if (!mPeekedEvent)
        {
            return;
        }

        if (_get_xcb_event(mPeekedEvent) == XCB_KEY_PRESS
        && ((xcb_key_press_event_t*)mPeekedEvent)->detail == ((xcb_key_press_event_t*)mLastEvent)->detail
        && ((xcb_key_press_event_t*)mPeekedEvent)->time == ((xcb_key_press_event_t*)mLastEvent)->time)
        {
            /* Key wasn’t actually released */
            free(mPeekedEvent);
            mPeekedEvent = nullptr;
            return;
        }
    }

    // Ignore when the mouse goes to the center of the window when
    // mouse capturing is enabled. The center of the window is where
    // the mouse is supposed to rest but resetting the mouse position
    // causes the event queue to fill up with MotionNotify events.
    if (_get_xcb_event(mLastEvent) == XCB_MOTION_NOTIFY && mCaptureMouse)
    {
        const xcb_motion_notify_event_t* motion = reinterpret_cast<xcb_motion_notify_event_t*>(mLastEvent);
        if (motion->event_x == (int)mWidth/2 && motion->event_y == (int)mHeight/2)
        {
            free(mLastEvent);
            mLastEvent = nullptr;
        }
    }

}



/*-------------------------------------
 * Pause the window (run in interrupt mode)
-------------------------------------*/
bool SR_RenderWindowXCB::pause() noexcept
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
            xcb_flush(mConnection);
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
bool SR_RenderWindowXCB::run() noexcept
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
            xcb_flush(mConnection);
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
bool SR_RenderWindowXCB::has_event() const noexcept
{
    return mLastEvent && (~0x80 & reinterpret_cast<xcb_generic_event_t*>(mLastEvent)->response_type) != XCB_NONE;
}



/*-------------------------------------
 * Check the next event within the event queue
-------------------------------------*/
bool SR_RenderWindowXCB::peek_event(SR_WindowEvent* const pEvent) noexcept
{
    xcb_expose_event_t* pExpose;
    xcb_key_press_event_t* pKeyPress;
    xcb_key_release_event_t* pKeyRelease;
    xcb_button_press_event_t* pButtonPress;
    xcb_button_release_event_t* pButtonRelease;
    xcb_motion_notify_event_t* pMotion;
    xcb_destroy_notify_event_t* pDestroy;
    xcb_client_message_event_t* pMessage;
    xcb_enter_notify_event_t* pEnter;
    xcb_leave_notify_event_t* pLeave;
    xcb_configure_notify_event_t* pConfig;

    unsigned keyMods = 0;
    unsigned long keySym;

    if (!has_event())
    {
        return false;
    }

    memset(pEvent, '\0', sizeof(SR_WindowEvent));
    switch (_get_xcb_event(mLastEvent))
    {
        case XCB_NONE: // sentinel
            pEvent->type = SR_WinEventType::WIN_EVENT_NONE;
            break;

        case XCB_EXPOSE:
            pExpose = reinterpret_cast<xcb_expose_event_t*>(mLastEvent);
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

        case XCB_KEYMAP_NOTIFY:
            break;

        case XCB_KEY_PRESS:
            pKeyPress = reinterpret_cast<xcb_key_press_event_t*>(mLastEvent);

            // Additional key processing is only performed in text-mode
            XkbLookupKeySym(mDisplay, pKeyPress->detail, pKeyPress->state, &keyMods, &keySym);
            pEvent->type = WIN_EVENT_KEY_DOWN;
            pEvent->pNativeWindow = pKeyPress->event;
            pEvent->keyboard.keysym = (SR_KeySymbol)keySym;
            pEvent->keyboard.key = mKeysRepeat ? 0 : (uint8_t)pKeyPress->detail; // only get key names in text mode
            pEvent->keyboard.capsLock = (uint8_t)((pKeyPress->state & LockMask) > 0);
            pEvent->keyboard.numLock = (uint8_t)((pKeyPress->state & Mod2Mask) > 0);
            pEvent->keyboard.scrollLock = (uint8_t)((pKeyPress->state & Mod3Mask) > 0);
            break;

        case XCB_KEY_RELEASE:
            pKeyRelease = reinterpret_cast<xcb_key_press_event_t*>(mLastEvent);

            // Additional key processing is only performed in text-mode
            XkbLookupKeySym(mDisplay, pKeyRelease->detail, pKeyRelease->state, &keyMods, &keySym);
            pEvent->type = WIN_EVENT_KEY_UP;
            pEvent->pNativeWindow = pKeyRelease->event;
            pEvent->keyboard.keysym = (SR_KeySymbol)keySym;
            pEvent->keyboard.key = mKeysRepeat ? 0 : (uint8_t)pKeyRelease->detail; // only get key names in text mode
            pEvent->keyboard.capsLock = (uint8_t)((pKeyRelease->state & LockMask) > 0);
            pEvent->keyboard.numLock = (uint8_t)((pKeyRelease->state & Mod2Mask) > 0);
            pEvent->keyboard.scrollLock = (uint8_t)((pKeyRelease->state & Mod3Mask) > 0);
            break;

        case XCB_BUTTON_PRESS:
            pButtonPress = reinterpret_cast<xcb_button_press_event_t*>(mLastEvent);
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_DOWN;
            pEvent->pNativeWindow = pButtonPress->event;

            switch (pButtonPress->detail)
            {
                case XCB_BUTTON_INDEX_1:
                    pEvent->mouseButton.mouseButton1 = 1;
                    pEvent->mouseButton.x = (int16_t)pButtonPress->event_x;
                    pEvent->mouseButton.y = (int16_t)pButtonPress->event_y;
                    break;

                case XCB_BUTTON_INDEX_2:
                    pEvent->mouseButton.mouseButton2 = 1;
                    pEvent->mouseButton.x = (int16_t)pButtonPress->event_x;
                    pEvent->mouseButton.y = (int16_t)pButtonPress->event_y;
                    break;

                case XCB_BUTTON_INDEX_3:
                    pEvent->mouseButton.mouseButton3 = 1;
                    pEvent->mouseButton.x = (int16_t)pButtonPress->event_x;
                    pEvent->mouseButton.y = (int16_t)pButtonPress->event_y;
                    break;

                case XCB_BUTTON_INDEX_4:
                case XCB_BUTTON_INDEX_5:
                    pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_WHEEL_MOVED;
                    pEvent->wheel.x = (int16_t)pButtonPress->event_x;
                    pEvent->wheel.y = (int16_t)pButtonPress->event_y;
                    pEvent->wheel.up = (pButtonPress->detail == XCB_BUTTON_INDEX_4);
                    pEvent->wheel.down = (pButtonPress->detail == XCB_BUTTON_INDEX_5);
                    break;

                default:
                    pEvent->mouseButton.mouseButtonN = (uint8_t)pButtonPress->detail;
                    pEvent->mouseButton.x = (int16_t)pButtonPress->event_x;
                    pEvent->mouseButton.y = (int16_t)pButtonPress->event_y;
                    break;
            }
            break;

        case XCB_BUTTON_RELEASE:
            pButtonRelease = reinterpret_cast<xcb_button_release_event_t*>(mLastEvent);
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_UP;
            pEvent->pNativeWindow = pButtonRelease->event;

            switch (pButtonRelease->detail)
            {
                case XCB_BUTTON_INDEX_1:
                    pEvent->mouseButton.mouseButton1 = 1;
                    pEvent->mouseButton.x = (int16_t)pButtonRelease->event_x;
                    pEvent->mouseButton.y = (int16_t)pButtonRelease->event_y;
                    break;

                case XCB_BUTTON_INDEX_2:
                    pEvent->mouseButton.mouseButton2 = 1;
                    pEvent->mouseButton.x = (int16_t)pButtonRelease->event_x;
                    pEvent->mouseButton.y = (int16_t)pButtonRelease->event_y;
                    break;

                case XCB_BUTTON_INDEX_3:
                    pEvent->mouseButton.mouseButton3 = 1;
                    pEvent->mouseButton.x = (int16_t)pButtonRelease->event_x;
                    pEvent->mouseButton.y = (int16_t)pButtonRelease->event_y;
                    break;

                case XCB_BUTTON_INDEX_4:
                case XCB_BUTTON_INDEX_5:
                    pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_WHEEL_MOVED;
                    pEvent->wheel.x = (int16_t)pButtonRelease->event_x;
                    pEvent->wheel.y = (int16_t)pButtonRelease->event_y;
                    pEvent->wheel.up = (pButtonRelease->detail == XCB_BUTTON_INDEX_4);
                    pEvent->wheel.down = (pButtonRelease->detail == XCB_BUTTON_INDEX_5);
                    break;

                default:
                    pEvent->mouseButton.mouseButtonN = (uint8_t)pButtonRelease->detail;
                    pEvent->mouseButton.x = (int16_t)pButtonRelease->event_x;
                    pEvent->mouseButton.y = (int16_t)pButtonRelease->event_y;
                    break;
            }
            break;

        case XCB_MOTION_NOTIFY:
            pMotion = reinterpret_cast<xcb_motion_notify_event_t*>(mLastEvent);
            pEvent->type = WIN_EVENT_MOUSE_MOVED;
            pEvent->pNativeWindow = pMotion->event;
            pEvent->mousePos.x = (int16_t)pMotion->event_x;
            pEvent->mousePos.y = (int16_t)pMotion->event_y;

            if (!mCaptureMouse)
            {
                pEvent->mousePos.dx = (int16_t)(mMouseX - pMotion->event_x);
                pEvent->mousePos.dy = (int16_t)(mMouseY - pMotion->event_y);
                mMouseX = pMotion->event_x;
                mMouseY = pMotion->event_y;

            }
            else
            {
                const int w2 = mWidth / 2;
                const int h2 = mHeight / 2;
                const int dx = pMotion->event_x;
                const int dy = pMotion->event_y;
                pEvent->mousePos.dx = (int16_t)(w2 - dx);
                pEvent->mousePos.dy = (int16_t)(h2 - dy);
                mMouseX = dx;
                mMouseY = dy;
            }
            break;

        case XCB_ENTER_NOTIFY:
            pEnter = reinterpret_cast<xcb_enter_notify_event_t*>(mLastEvent);
            pEvent->pNativeWindow = pEnter->event;
            pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_ENTER;
            pEvent->mousePos.x = (int16_t)pEnter->event_x;
            pEvent->mousePos.y = (int16_t)pEnter->event_y;
            break;

        case XCB_LEAVE_NOTIFY:
            pLeave = reinterpret_cast<xcb_leave_notify_event_t*>(mLastEvent);
            pEvent->pNativeWindow = pLeave->event;
            pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_LEAVE;
            pEvent->mousePos.x = (int16_t)pLeave->event_x;
            pEvent->mousePos.y = (int16_t)pLeave->event_y;
            break;

        case XCB_CLIENT_MESSAGE:
            pMessage = reinterpret_cast<xcb_client_message_event_t*>(mLastEvent);
            if ((unsigned long)(pMessage->data.data32[0]) == mCloseAtom)
            {
                mCurrentState = WindowStateInfo::WINDOW_CLOSING;
                pEvent->pNativeWindow = pMessage->window;
            }
            break;

        case XCB_DESTROY_NOTIFY:
            pDestroy = reinterpret_cast<xcb_destroy_notify_event_t*>(mLastEvent);
            mCurrentState = WindowStateInfo::WINDOW_CLOSING;
            pEvent->type = SR_WinEventType::WIN_EVENT_CLOSING;
            pEvent->pNativeWindow = pDestroy->window;
            break;

        case XCB_CONFIGURE_NOTIFY:
            pConfig = reinterpret_cast<xcb_configure_notify_event_t*>(mLastEvent);
            pEvent->pNativeWindow = pConfig->window;

            if (mX != pConfig->x || mY != pConfig->y)
            {
                pEvent->type = SR_WinEventType::WIN_EVENT_MOVED;
                mX = pConfig->x;
                mY = pConfig->y;
                pEvent->window.x = (int16_t)pConfig->x;
                pEvent->window.y = (int16_t)pConfig->y;
            }

            if (mWidth != (unsigned)pConfig->width || mHeight != (unsigned)pConfig->height)
            {
                pEvent->type = SR_WinEventType::WIN_EVENT_RESIZED;
                mWidth = (unsigned)pConfig->width;
                mHeight = (unsigned)pConfig->height;
                pEvent->window.width = (uint16_t)pConfig->width;
                pEvent->window.height = (uint16_t)pConfig->height;
            }

            break;

        default: // unhandled event
            pEvent->type = SR_WinEventType::WIN_EVENT_UNKNOWN;
            return false;
    }

    return true;
}



/*-------------------------------------
 * Remove an event from the event queue
-------------------------------------*/
bool SR_RenderWindowXCB::pop_event(SR_WindowEvent* const pEvent) noexcept
{
    const bool ret = peek_event(pEvent);
    if (mLastEvent)
    {
        free(mLastEvent);
        mLastEvent = nullptr;
    }

    return ret;
}



/*-------------------------------------
 * Enable or disable repeating keys
-------------------------------------*/
bool SR_RenderWindowXCB::set_keys_repeat(bool doKeysRepeat) noexcept
{
    mKeysRepeat = XkbSetDetectableAutoRepeat(mDisplay, !doKeysRepeat, nullptr) == False;

    return mKeysRepeat;
}



/*-------------------------------------
 * Render a framebuffer to the current window
-------------------------------------*/
void SR_RenderWindowXCB::render(SR_WindowBuffer& buffer) noexcept
{
    assert(this->valid());
    assert(buffer.native_handle() != nullptr);

    const uint32_t w = (uint32_t)width();
    const uint32_t h = (uint32_t)height();

    #if SR_ENABLE_XSHM != 0
        xcb_shm_segment_info_t* pShmInfo = (xcb_shm_segment_info_t*)((SR_WindowBufferXCB*)&buffer)->mShmInfo;

        xcb_shm_put_image(
            mConnection,
            mWindow,
            mContext,
            (uint16_t)w,
            (uint16_t)h,
            0, 0,
            (uint16_t)w,
            (uint16_t)h,
            0, 0,
            24,
            XCB_IMAGE_FORMAT_Z_PIXMAP,
            0,
            pShmInfo->shmseg,
            0
        );

        xcb_flush(mConnection);
    #else
        xcb_put_image(
            mConnection,
            XCB_IMAGE_FORMAT_Z_PIXMAP,
            mWindow,
            mContext,
            (uint16_t)w,
            (uint16_t)h,
            0, 0,
            0, 24,
            sizeof(SR_ColorRGBA8)*w*h,
            reinterpret_cast<const uint8_t*>(buffer.buffer())
        );
    #endif
}




/*-------------------------------------
 * Mouse Grabbing
-------------------------------------*/
void SR_RenderWindowXCB::set_mouse_capture(bool isCaptured) noexcept
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
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_POINTER_MOTION
        | 0;

        xcb_grab_pointer(mConnection, 1, mWindow, captureFlags, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, mWindow, XCB_NONE, XCB_CURRENT_TIME);
    }
    else
    {
        xcb_ungrab_pointer(mConnection, XCB_CURRENT_TIME);
    }
}



/*-------------------------------------
 * Get the current scaling factor for the display
-------------------------------------*/
unsigned SR_RenderWindowXCB::dpi() const noexcept
{
    int screenId = DefaultScreen(mDisplay);
    const float displayInches = (float)DisplayWidth(mDisplay, screenId) * 25.4f;
    const float widthMM = (float)DisplayWidthMM(mDisplay, screenId);

    return (unsigned)(displayInches / widthMM + 0.5f); // round before truncate
}
