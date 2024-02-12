
// Thanks again Visual Studio
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#ifndef NOMINMAX
    #define NOMINMAX
#endif /* NOMINMAX */

#include <windows.h>
#include <windowsx.h>
#include <winuser.h>

#include <string>

#include "lightsky/setup/Compiler.h"

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

#include "softlight/SL_Color.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_SwapchainWin32.hpp"
#include "softlight/SL_RenderWindowWin32.hpp"
#include "softlight/SL_WindowEvent.hpp"



/*-----------------------------------------------------------------------------
    Anonymous data
-----------------------------------------------------------------------------*/
namespace
{

LPCSTR SL_RENDER_WINDOW_WIN32_CLASS = "SL_RenderWindowWin32";



constexpr DWORD WINDOW_STYLE_EX =
    0
    | WS_EX_ACCEPTFILES
    | WS_EX_OVERLAPPEDWINDOW
    | 0;

constexpr DWORD WINDOW_STYLE =
    0
    | WS_BORDER
    | WS_OVERLAPPEDWINDOW
    | WS_VISIBLE
    | 0;



/*
std::wstring cstr_to_wstr(const char* pStr)
{
    std::wstring ret{};
    std::mbstate_t mbState = std::mbstate_t();
    std::size_t len = std::mbsrtowcs(nullptr, &pStr, 0, &mbState);

    if (len == static_cast<std::size_t>(-1))
    {
        return ret;
    }

    ret.resize(len + 1);
    std::mbsrtowcs(&ret[0], &pStr, ret.size(), &mbState);

    return ret;
}
*/


} // end anonymous data



/*-----------------------------------------------------------------------------
* SL_RenderWindowWin32 Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Window Event Callback
-------------------------------------*/
LRESULT CALLBACK SL_RenderWindowWin32::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SL_RenderWindowWin32* pNewWin;

    if (msg == WM_CREATE)
    {
        LPCREATESTRUCT pStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pNewWin = static_cast<SL_RenderWindowWin32*>(pStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pNewWin));
    }
    else
    {
        pNewWin = reinterpret_cast<SL_RenderWindowWin32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    return pNewWin->win_proc(hwnd, msg, wParam, lParam);
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_RenderWindowWin32::~SL_RenderWindowWin32() noexcept
{
    if (this->valid() && destroy() != 0)
    {
        LS_LOG_ERR("Unable to properly close the render window ", this, " during destruction.");
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_RenderWindowWin32::SL_RenderWindowWin32() noexcept :
    mWc(),
    mHwnd(nullptr),
    mLastMsg(),
    mMouseX{0},
    mMouseY{0},
    mKeysRepeat{true},
    mCaptureMouse{false}
{
    mLastMsg = MSG();
    mLastMsg.hwnd = nullptr;
    mLastMsg.message = WM_NULL;
}


/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_RenderWindowWin32::SL_RenderWindowWin32(const SL_RenderWindowWin32& rw) noexcept :
    SL_RenderWindowWin32{} // delegate
{
    // delegate some more
    *this = rw;
}


/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_RenderWindowWin32::SL_RenderWindowWin32(SL_RenderWindowWin32&& rw) noexcept :
    SL_RenderWindow{std::move(rw)},
    mWc(rw.mWc),
    mHwnd(rw.mHwnd),
    mLastMsg(rw.mLastMsg),
    mMouseX{rw.mMouseX},
    mMouseY{rw.mMouseY},
    mKeysRepeat{rw.mKeysRepeat},
    mCaptureMouse{rw.mCaptureMouse}
{
    rw.mWc = WNDCLASSEX();
    rw.mHwnd = nullptr;
    rw.mLastMsg = MSG();
    rw.mLastMsg.hwnd = nullptr;
    rw.mLastMsg.message = WM_NULL;
    rw.mMouseX = 0;
    rw.mMouseY = 0;
    rw.mKeysRepeat = true;
    rw.mCaptureMouse = false;
}


/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_RenderWindowWin32& SL_RenderWindowWin32::operator=(const SL_RenderWindowWin32& rw) noexcept
{
    if (this == &rw)
    {
        return *this;
    }

    this->destroy();

    SL_RenderWindowWin32* const pWindow = static_cast<SL_RenderWindowWin32*>(rw.clone());

    if (pWindow && pWindow->valid())
    {
        // handle the base class
        SL_RenderWindow::operator=(rw);
        *this = std::move(*pWindow);
    }

    delete pWindow;

    return *this;
}


/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_RenderWindowWin32& SL_RenderWindowWin32::operator=(SL_RenderWindowWin32&& rw) noexcept
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
    SL_RenderWindow::operator=(std::move(rw));

    mWc = rw.mWc;
    rw.mWc = WNDCLASSEX();

    mHwnd = rw.mHwnd;
    rw.mHwnd = nullptr;

    mLastMsg = rw.mLastMsg;
    rw.mLastMsg = MSG();
    rw.mLastMsg.hwnd = nullptr;
    rw.mLastMsg.message = WM_NULL;

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
 * Private: Window Creation callbacl
-------------------------------------*/
LRESULT SL_RenderWindowWin32::win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_COMMAND:
            break;

        case WM_CLOSE:
            mCurrentState = WindowStateInfo::WINDOW_CLOSING;
            break;

        case WM_DESTROY:
            destroy();
            break;

        case WM_MOVE:
        case WM_SIZE:
            mLastMsg.hwnd = mHwnd == hwnd ? hwnd : 0;
            mLastMsg.message = msg;
            mLastMsg.wParam = wParam;
            mLastMsg.lParam = lParam;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return FALSE;
}



/*-------------------------------------
 * Set the window title
-------------------------------------*/
int SL_RenderWindowWin32::set_title(const char* const pName) noexcept
{
    if (!valid())
    {
        return -1;
    }

    if (SetWindowText(mHwnd, pName) == FALSE)
    {
        return -2;
    }

    return 0;
}



/*-------------------------------------
 * Create a widnow
-------------------------------------*/
int SL_RenderWindowWin32::init(unsigned width, unsigned height) noexcept
{
    LS_ASSERT(!this->valid());

    LS_LOG_MSG("SL_RenderWindowWin32 ", this, " initializing");

    WNDCLASSEX wc;
    HWND hwnd;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = &SL_RenderWindowWin32::WinProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = static_cast<HINSTANCE>(GetModuleHandle(nullptr));
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = SL_RENDER_WINDOW_WIN32_CLASS;
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        LS_LOG_ERR("\tFailed to create a window class.");
        return -1;
    }

    // Make sure the client area is the size we need
    RECT clientArea;
    clientArea.left = 0;
    clientArea.right = width;
    clientArea.top = 0;
    clientArea.bottom = height;
    AdjustWindowRectEx(&clientArea, WINDOW_STYLE, FALSE, WINDOW_STYLE_EX);

    hwnd = CreateWindowEx(
        WINDOW_STYLE_EX,
        SL_RENDER_WINDOW_WIN32_CLASS,
        "",
        WINDOW_STYLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        clientArea.right,
        clientArea.bottom,
        nullptr,
        nullptr,
        wc.hInstance,
        static_cast<LPVOID>(this)
    );

    if (!hwnd)
    {
        UnregisterClass(SL_RENDER_WINDOW_WIN32_CLASS, wc.hInstance);
        LS_LOG_ERR("\tFailed to create a window handle.");
        return -2;
    }

    mCurrentState = WindowStateInfo::WINDOW_STARTED;
    mWc = wc;
    mHwnd = hwnd;

    mLastMsg = MSG();
    mLastMsg.hwnd = nullptr;
    mLastMsg.message = WM_NULL;
    mMouseX = 0;
    mMouseY = 0;

    LS_LOG_MSG(
        "Done. Successfully initialized SL_RenderWindowWin32 ", this, '.',
        "\n\tDisplay:    ", wc.lpszClassName,
        "\n\tWindow ID:  ", mHwnd,
        "\n\tResolution: ", clientArea.right, 'x', clientArea.bottom,
        "\n\tDPI/Scale:  ", this->dpi(),
        "\n\tPosition:   ", clientArea.left,  'x', clientArea.top);

    return 0;
}



/*-------------------------------------
 * Destroy the current window and release all resources
-------------------------------------*/
int SL_RenderWindowWin32::destroy() noexcept
{
    if (valid())
    {
        DestroyWindow(mHwnd);
        mWc = WNDCLASSEX();
        mHwnd = nullptr;
        mLastMsg = MSG();
        mLastMsg.hwnd = nullptr;
        mMouseX = 0;
        mMouseY = 0;
    }

    mCurrentState = WindowStateInfo::WINDOW_CLOSED;

    return 0;
}



/*-------------------------------------
* Retrieve the window width
-------------------------------------*/
unsigned SL_RenderWindowWin32::width() const noexcept
{
    unsigned w = 0;
    unsigned h = 0;
    get_size(w, h);
    return w;
}



/*-------------------------------------
* Retrieve the window height
-------------------------------------*/
unsigned SL_RenderWindowWin32::height() const noexcept
{
    unsigned w = 0;
    unsigned h = 0;
    get_size(w, h);
    return h;
}



/*-------------------------------------
* Retrieve the window size
-------------------------------------*/
void SL_RenderWindowWin32::get_size(unsigned& w, unsigned& h) const noexcept
{
    RECT clientDimens;

    if (valid() && GetClientRect(mHwnd, &clientDimens) == TRUE)
    {
        w = (unsigned)clientDimens.right;
        h = (unsigned)clientDimens.bottom;
    }
}



/*-------------------------------------
* Set the window size
-------------------------------------*/
bool SL_RenderWindowWin32::set_size(unsigned w, unsigned h) noexcept
{
    if (!valid())
    {
        return false;
    }

    RECT winRect;
    if (GetWindowRect(mHwnd, &winRect) == FALSE)
    {
        return false;
    }

    RECT clientArea;
    clientArea.left = winRect.right-winRect.left;
    clientArea.right = (int)w;
    clientArea.top = winRect.bottom-winRect.top;
    clientArea.bottom = (int)h;

    if (AdjustWindowRectEx(&clientArea,WINDOW_STYLE, FALSE, WINDOW_STYLE_EX) == FALSE)
    {
        return false;
    }


    if (SetWindowPos(mHwnd, nullptr, clientArea.left, clientArea.top, (int)clientArea.right, (int)clientArea.bottom, SWP_NOREPOSITION | SWP_NOSENDCHANGING))
    {
        return UpdateWindow(mHwnd) != FALSE;
    }

    return false;
}



/*-------------------------------------
* Get the window position (X)
-------------------------------------*/
int SL_RenderWindowWin32::x_position() const noexcept
{
    unsigned x = 0;
    unsigned y = 0;
    get_size(x, y);
    return x;
}



/*-------------------------------------
* Get the window position (Y)
-------------------------------------*/
int SL_RenderWindowWin32::y_position() const noexcept
{
    unsigned x = 0;
    unsigned y = 0;
    get_size(x, y);
    return y;
}



/*-------------------------------------
* Get the window position
-------------------------------------*/
bool SL_RenderWindowWin32::get_position(int& x, int& y) const noexcept
{
    RECT winDimens;

    if (!valid() || GetWindowRect(mHwnd, &winDimens) == FALSE)
    {
        return false;
    }

    x = winDimens.left;
    y = winDimens.top;

    return true;
}



/*-------------------------------------
* Set the window size
-------------------------------------*/
bool SL_RenderWindowWin32::set_position(int x, int y) noexcept
{
    if (!valid())
    {
        return false;
    }

    unsigned w = 0;
    unsigned h = 0;
    get_size(w, h);

    if (SetWindowPos(mHwnd, nullptr, x, y, w, h, SWP_NOSIZE | SWP_NOSENDCHANGING) != FALSE)
    {
        UpdateWindow(mHwnd);
    }

    return false;
}



/*-------------------------------------
-------------------------------------*/
SL_RenderWindow* SL_RenderWindowWin32::clone() const noexcept
{
    const SL_RenderWindowWin32& self = *this; // nullptr check

    SL_RenderWindowWin32* pWindow = new(std::nothrow) SL_RenderWindowWin32(self);

    return pWindow;
}



/*-------------------------------------
-------------------------------------*/
void SL_RenderWindowWin32::update() noexcept
{
    // sanity check
    if (!valid())
    {
        return;
    }

    BOOL msgStatus = 0;

    switch (mCurrentState)
    {
        // The window was starting to close in the last frame. Destroy it now
        case WindowStateInfo::WINDOW_CLOSING:
            destroy();
            return;

        case WindowStateInfo::WINDOW_STARTED:
            // fall-through
            run();

        case WindowStateInfo::WINDOW_RUNNING:
            // Perform a non-blocking poll if we're not paused
            msgStatus = PeekMessage(&mLastMsg, mHwnd, 0, 0, PM_REMOVE);
            break;

        case WindowStateInfo::WINDOW_PAUSED:
            // Perform a blocking event check while the window is paused.
            msgStatus = GetMessage(&mLastMsg, mHwnd, 0, 0);
            break;

        default:
            // We should not be in a "starting" or "closed" state
            LS_LOG_ERR("Encountered unexpected window state ", mCurrentState, '.');
            LS_ASSERT(false); // assertions are disabled on release builds
            mCurrentState = WindowStateInfo::WINDOW_CLOSING;
            return;
    }

    if (msgStatus > 0)
    {
        TranslateMessage(&mLastMsg);
        DispatchMessage(&mLastMsg);
    }

    if (mLastMsg.message == WM_MOUSEMOVE && mCaptureMouse)
    {
        unsigned w, h;
        POINT mousePos;
        RECT clientRect;

        GetCursorPos(&mousePos);
        GetClientRect(mHwnd, &clientRect);
        w = clientRect.right >> 1;
        h = clientRect.bottom >> 1;

        if (mousePos.x != (int)w && mousePos.y != (int)h)
        {
            POINT leftTop;
            POINT rightBottom;

            leftTop.x     = clientRect.left;
            leftTop.y     = clientRect.top;
            rightBottom.x = clientRect.right;
            rightBottom.y = clientRect.bottom;

            // Get the window border size to prevent the mouse getting stuck on a window edge
            #ifndef LS_COMPILER_MINGW
                int borderSize = GetSystemMetrics(SM_CYFRAME);
            #else
                constexpr int borderSize = 0;
            #endif

            ClientToScreen(mHwnd, &leftTop);
            ClientToScreen(mHwnd, &rightBottom);

            clientRect.left   = leftTop.x;
            clientRect.top    = leftTop.y;
            clientRect.right  = rightBottom.x - borderSize;
            clientRect.bottom = rightBottom.y - borderSize;

            ClipCursor(&clientRect);

            mousePos.x = w;
            mousePos.y = h;
            ClientToScreen(mHwnd, &mousePos);
            SetCursorPos(mousePos.x, mousePos.y);
        }
    }
}



/*-------------------------------------
-------------------------------------*/
bool SL_RenderWindowWin32::pause() noexcept
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
            ShowWindow(mHwnd, SW_SHOWDEFAULT);
            UpdateWindow(mHwnd);
            LS_LOG_MSG("Window started in a paused state.");
        case WindowStateInfo::WINDOW_RUNNING:
        case WindowStateInfo::WINDOW_PAUSED:
        case WindowStateInfo::WINDOW_CLOSING:
            mCurrentState = WindowStateInfo::WINDOW_PAUSED;
            break;

        // These states can't be used to transition to a paused state
        case WindowStateInfo::WINDOW_CLOSED:
        case WindowStateInfo::WINDOW_STARTING:
            LS_ASSERT(false); // fail in case of error
            break;
    }

    return mCurrentState == WindowStateInfo::WINDOW_PAUSED;
}



/*-------------------------------------
-------------------------------------*/
bool SL_RenderWindowWin32::run() noexcept
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
            ShowWindow(mHwnd, SW_SHOWDEFAULT);
            UpdateWindow(mHwnd);
            LS_LOG_MSG("Window started in a running state.");
        case WindowStateInfo::WINDOW_CLOSING:
        case WindowStateInfo::WINDOW_RUNNING:
        case WindowStateInfo::WINDOW_PAUSED:
            mCurrentState = WindowStateInfo::WINDOW_RUNNING;
        break;

        // These states can't be used to transition to a paused state
        case WindowStateInfo::WINDOW_CLOSED:
        case WindowStateInfo::WINDOW_STARTING:
            LS_ASSERT(false); // fail in case of error
            break;
    }

    return mCurrentState == WindowStateInfo::WINDOW_RUNNING;
}



/*-------------------------------------
-------------------------------------*/
bool SL_RenderWindowWin32::has_event() const noexcept
{
    return mLastMsg.hwnd != nullptr && mLastMsg.message != WM_NULL;
}



/*-------------------------------------
-------------------------------------*/
bool SL_RenderWindowWin32::peek_event(SL_WindowEvent* const pEvent) noexcept
{
    memset(pEvent, '\0', sizeof(SL_WindowEvent));
    if (!has_event())
    {
        return false;
    }

    RECT clientRect;
    pEvent->pNativeWindow = (ptrdiff_t)mLastMsg.hwnd;

    switch (mLastMsg.message)
    {
    case WM_NULL: // sentinel
        pEvent->type = SL_WinEventType::WIN_EVENT_NONE;
        break;

    case WM_PAINT:
        GetClientRect(mLastMsg.hwnd, &clientRect);
        pEvent->type = WIN_EVENT_EXPOSED;
        pEvent->window.x = (uint16_t)clientRect.left;
        pEvent->window.y = (uint16_t)clientRect.top;
        pEvent->window.width = (uint16_t)(clientRect.right - clientRect.left);
        pEvent->window.height = (uint16_t)(clientRect.bottom - clientRect.top);
        break;

    case WM_CHAR:
        // mKeysRepeat is basically turning this function into text mode
        if (mKeysRepeat)
        {
            pEvent->type = SL_WinEventType::WIN_EVENT_UNKNOWN;
            return false;
        }

        pEvent->type = WIN_EVENT_KEY_DOWN;
        pEvent->keyboard.keysym = (SL_KeySymbol)mLastMsg.wParam;
        pEvent->keyboard.key = mLastMsg.wParam & 0xFF;
        pEvent->keyboard.capsLock = (uint8_t)((GetKeyState(VK_CAPITAL) & 0x0001) != 0);
        pEvent->keyboard.numLock = (uint8_t)((GetKeyState(VK_NUMLOCK) & 0x0001) != 0);
        pEvent->keyboard.scrollLock = (uint8_t)((GetKeyState(VK_SCROLL) & 0x0001) != 0);
        break;

    case WM_KEYDOWN:
        // As per the windows documentation, bit #30 determines if the key had
        // previously been depressed.
        if (!mKeysRepeat && (mLastMsg.lParam & (1<<30)) != 0)
        {
            pEvent->type = SL_WinEventType::WIN_EVENT_UNKNOWN;
            return false;
        }

        pEvent->type = WIN_EVENT_KEY_DOWN;
        pEvent->keyboard.keysym = (SL_KeySymbol)mLastMsg.wParam;
        pEvent->keyboard.key = 0; // do no additional processing in non-text mode.
        pEvent->keyboard.capsLock = (uint8_t)((GetKeyState(VK_CAPITAL) & 0x0001) != 0);
        pEvent->keyboard.numLock = (uint8_t)((GetKeyState(VK_NUMLOCK) & 0x0001) != 0);
        pEvent->keyboard.scrollLock = (uint8_t)((GetKeyState(VK_SCROLL) & 0x0001) != 0);
        break;

    case WM_KEYUP:
        pEvent->type = WIN_EVENT_KEY_UP;
        pEvent->keyboard.keysym = (SL_KeySymbol)mLastMsg.wParam;
        pEvent->keyboard.key = 0; // do no additional processing in non-text mode.
        pEvent->keyboard.capsLock = (uint8_t)((GetKeyState(VK_CAPITAL) & 0x0001) != 0);
        pEvent->keyboard.numLock = (uint8_t)((GetKeyState(VK_NUMLOCK) & 0x0001) != 0);
        pEvent->keyboard.scrollLock = (uint8_t)((GetKeyState(VK_SCROLL) & 0x0001) != 0);
        break;

    case WM_LBUTTONDOWN:
        pEvent->type = WIN_EVENT_MOUSE_BUTTON_DOWN;
        pEvent->mouseButton.mouseButton1 = 1;
        pEvent->mouseButton.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mouseButton.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_RBUTTONDOWN:
        pEvent->type = WIN_EVENT_MOUSE_BUTTON_DOWN;
        pEvent->mouseButton.mouseButton2 = 1;
        pEvent->mouseButton.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mouseButton.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_MBUTTONDOWN:
        pEvent->type = WIN_EVENT_MOUSE_BUTTON_DOWN;
        pEvent->mouseButton.mouseButton3 = 1;
        pEvent->mouseButton.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mouseButton.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_LBUTTONUP:
        pEvent->type = WIN_EVENT_MOUSE_BUTTON_UP;
        pEvent->mouseButton.mouseButton1 = 1;
        pEvent->mouseButton.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mouseButton.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_RBUTTONUP:
        pEvent->type = WIN_EVENT_MOUSE_BUTTON_UP;
        pEvent->mouseButton.mouseButton2 = 1;
        pEvent->mouseButton.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mouseButton.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_MBUTTONUP:
        pEvent->type = WIN_EVENT_MOUSE_BUTTON_UP;
        pEvent->mouseButton.mouseButton3 = 1;
        pEvent->mouseButton.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mouseButton.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_MOUSEWHEEL:
        pEvent->type = SL_WinEventType::WIN_EVENT_MOUSE_WHEEL_MOVED;
        pEvent->wheel.direction = (float)GET_WHEEL_DELTA_WPARAM(mLastMsg.wParam) / (float)WHEEL_DELTA;
        pEvent->wheel.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->wheel.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_MOUSEMOVE:
        pEvent->type = WIN_EVENT_MOUSE_MOVED;
        pEvent->mousePos.x  = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mousePos.y  = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);

        if (!mCaptureMouse)
        {
            pEvent->mousePos.dx = (int16_t)(mMouseX - pEvent->mousePos.x);
            pEvent->mousePos.dy = (int16_t)(mMouseY - pEvent->mousePos.y);
            mMouseX = pEvent->mousePos.x;
            mMouseY = pEvent->mousePos.y;

        }
        else
        {
            unsigned w = 0;
            unsigned h = 0;
            get_size(w, h);
            const int w2 = w >> 1;
            const int h2 = h >> 1;
            const int dx = pEvent->mousePos.x;
            const int dy = pEvent->mousePos.y;
            pEvent->mousePos.dx = (int16_t)(w2 - dx);
            pEvent->mousePos.dy = (int16_t)(h2 - dy);
            mMouseX = dx;
            mMouseY = dy;
        }
        break;

    case WM_POINTERENTER:
        pEvent->type = SL_WinEventType::WIN_EVENT_MOUSE_ENTER;
        pEvent->mousePos.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mousePos.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_POINTERLEAVE:
        pEvent->type = SL_WinEventType::WIN_EVENT_MOUSE_LEAVE;
        pEvent->mousePos.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mousePos.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_MOVE:
        pEvent->type = SL_WinEventType::WIN_EVENT_MOVED;
        pEvent->window.x = (uint16_t)LOWORD(mLastMsg.lParam);
        pEvent->window.y = (uint16_t)HIWORD(mLastMsg.lParam);
        break;

    case WM_SIZE:
        pEvent->type = SL_WinEventType::WIN_EVENT_RESIZED;
        pEvent->window.width = (uint16_t)LOWORD(mLastMsg.lParam);
        pEvent->window.height = (uint16_t)HIWORD(mLastMsg.lParam);

        // Make sure the mouse stays in the window if requested
        if (mCaptureMouse)
        {
            RECT winCoords;
            GetWindowRect(mHwnd, &winCoords);
            ClipCursor(&winCoords);
        }
        break;

    default: // unhandled event
        pEvent->type = SL_WinEventType::WIN_EVENT_UNKNOWN;
        return false;
    }

    return true;
}


/*-------------------------------------
* Remove an event from the event queue
-------------------------------------*/
bool SL_RenderWindowWin32::pop_event(SL_WindowEvent* const pEvent) noexcept
{
    const bool ret = peek_event(pEvent);
    if (mLastMsg.hwnd)
    {
        mLastMsg.hwnd = nullptr;
        mLastMsg.message = WM_NULL;
    }

    return ret;
}


/*-------------------------------------
 * Enable or disable repeating keys
-------------------------------------*/
bool SL_RenderWindowWin32::set_keys_repeat(bool doKeysRepeat) noexcept
{
    mKeysRepeat = doKeysRepeat;
    return mKeysRepeat;
}


/*-------------------------------------
 * Render a framebuffer to the current window
-------------------------------------*/
void SL_RenderWindowWin32::render(SL_Swapchain& buffer) noexcept
{
    LS_ASSERT(this->valid());
    LS_ASSERT(buffer.native_handle() != nullptr);

    SL_SwapchainWin32& pWinBuffer = static_cast<SL_SwapchainWin32&>(buffer);

    RECT displayArea;
    GetClientRect(mHwnd, &displayArea);

    HDC winDc = GetDC(mHwnd);
    PBITMAPINFO pInfo = reinterpret_cast<PBITMAPINFO>(pWinBuffer.mBitmapInfo);

    const int numScanLines = SetDIBitsToDevice(
        winDc,
        displayArea.left,
        displayArea.top,
        displayArea.right,
        displayArea.bottom,
        0,
        0,
        0,
        pWinBuffer.height(),
        pWinBuffer.buffer(),
        pInfo,
        DIB_RGB_COLORS);

    if (numScanLines == 0)
    {
        LS_LOG_ERR("Failed to blit a framebuffer. Error code: ", GetLastError());
    }

    ReleaseDC(mHwnd, winDc);
}


/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
void SL_RenderWindowWin32::set_mouse_capture(bool isCaptured) noexcept
{
    if (!valid())
    {
        return;
    }

    mCaptureMouse = isCaptured;

    if (mCaptureMouse)
    {
        RECT winCoords;
        GetWindowRect(mHwnd, &winCoords);
        ClipCursor(&winCoords);
    }
    else
    {
        ClipCursor(nullptr);
    }
}


/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
bool SL_RenderWindowWin32::is_mouse_captured() const noexcept
{
    return mCaptureMouse;
}


/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
unsigned SL_RenderWindowWin32::dpi() const noexcept
{
    if (!valid())
    {
        return 0;
    }

    HDC winDc = GetDC(mHwnd);
    //return (unsigned)GetDeviceCaps(winDc, LOGPIXELSY);

    const float displayInches = (float)GetDeviceCaps(winDc, HORZRES) * 25.4;
    const float widthMM = (float)GetDeviceCaps(winDc, HORZSIZE);
    ReleaseDC(mHwnd, winDc);

    return (unsigned)(displayInches / widthMM + 0.5f); // round before truncate
}



/*-------------------------------------
 * Get the contents of the clipboard
-------------------------------------*/
void SL_RenderWindowWin32::request_clipboard() const noexcept
{
}
