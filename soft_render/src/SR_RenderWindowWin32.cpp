
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>

#include <cassert>
#include <iostream>
#include <string>

#include "lightsky/setup/Compiler.h"

#include "soft_render/SR_Color.hpp"

#include "soft_render/SR_KeySym.hpp"
#include "soft_render/SR_RenderWindowWin32.hpp"
#include "soft_render/SR_WindowBufferWin32.hpp"



/*-----------------------------------------------------------------------------
    Anonymous data
-----------------------------------------------------------------------------*/
namespace
{

LPCSTR SR_RENDER_WINDOW_WIN32_CLASS = "SR_RenderWindowWin32";



constexpr DWORD WINDOW_STYLE_EX =
    0
    | WS_EX_ACCEPTFILES
    | WS_EX_CLIENTEDGE
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
* SR_RenderWindowWin32 Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Window Event Callback
-------------------------------------*/
LRESULT CALLBACK SR_RenderWindowWin32::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SR_RenderWindowWin32* pNewWin;

    if (msg == WM_CREATE)
    {
        LPCREATESTRUCT pStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pNewWin = static_cast<SR_RenderWindowWin32*>(pStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pNewWin));
    }
    else
    {
        pNewWin = reinterpret_cast<SR_RenderWindowWin32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    return pNewWin->win_proc(hwnd, msg, wParam, lParam);
}



/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_RenderWindowWin32::~SR_RenderWindowWin32() noexcept
{
    if (this->valid() && destroy() != 0)
    {
        std::cerr
            << "Unable to properly close the render window "
            << this
            << " during destruction."
            << std::endl;
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_RenderWindowWin32::SR_RenderWindowWin32() noexcept :
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
SR_RenderWindowWin32::SR_RenderWindowWin32(const SR_RenderWindowWin32& rw) noexcept :
    SR_RenderWindowWin32{} // delegate
{
    // delegate some more
    *this = rw;
}


/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_RenderWindowWin32::SR_RenderWindowWin32(SR_RenderWindowWin32&& rw) noexcept :
    SR_RenderWindow{std::move(rw)},
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
SR_RenderWindowWin32& SR_RenderWindowWin32::operator=(const SR_RenderWindowWin32& rw) noexcept
{
    if (this == &rw)
    {
        return *this;
    }

    this->destroy();

    SR_RenderWindowWin32* const pWindow = static_cast<SR_RenderWindowWin32*>(rw.clone());

    if (pWindow && pWindow->valid())
    {
        // handle the base class
        SR_RenderWindow::operator=(rw);
        *this = std::move(*pWindow);
    }

    return *this;
}


/*-------------------------------------
 * Move Operator
-------------------------------------*/
SR_RenderWindowWin32& SR_RenderWindowWin32::operator=(SR_RenderWindowWin32&& rw) noexcept
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
LRESULT SR_RenderWindowWin32::win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_COMMAND:
            std::cout << "Command: " << wParam << std::endl;
            break;

        case WM_CLOSE:
            mCurrentState = WindowStateInfo::WINDOW_CLOSING;
            break;

        case WM_DESTROY:
            destroy();
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return FALSE;
}



/*-------------------------------------
 * Set the window title
-------------------------------------*/
int SR_RenderWindowWin32::set_title(const char* const pName) noexcept
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
int SR_RenderWindowWin32::init(unsigned width, unsigned height) noexcept
{
    assert(!this->valid());

    std::cout << "SR_RenderWindowWin32 " << this << " initializing" << std::endl;

    WNDCLASSEX wc;
    HWND hwnd;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = &SR_RenderWindowWin32::WinProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = static_cast<HINSTANCE>(GetModuleHandle(nullptr));
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = SR_RENDER_WINDOW_WIN32_CLASS;
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        std::cerr << "\tFailed to create a window class." << std::endl;
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
        SR_RENDER_WINDOW_WIN32_CLASS,
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
        UnregisterClass(SR_RENDER_WINDOW_WIN32_CLASS, wc.hInstance);
        std::cerr << "\tFailed to create a window handle." << std::endl;
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

    std::cout
        << "Done. Successfully initialized SR_RenderWindowWin32 " << this << '.'
        << "\n\tDisplay:    " << wc.lpszClassName
        << "\n\tWindow ID:  " << mHwnd
        << "\n\tResolution: " << clientArea.right << 'x' << clientArea.bottom
        << "\n\tPosition:   " << clientArea.left << 'x' << clientArea.top
        << std::endl;

    return 0;
}



/*-------------------------------------
 * Destroy the current window and release all resources
-------------------------------------*/
int SR_RenderWindowWin32::destroy() noexcept
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
unsigned SR_RenderWindowWin32::width() const noexcept
{
    unsigned w = 0;
    unsigned h = 0;
    get_size(w, h);
    return w;
}



/*-------------------------------------
* Retrieve the window height
-------------------------------------*/
unsigned SR_RenderWindowWin32::height() const noexcept
{
    unsigned w = 0;
    unsigned h = 0;
    get_size(w, h);
    return h;
}



/*-------------------------------------
* Retrieve the window size
-------------------------------------*/
void SR_RenderWindowWin32::get_size(unsigned& w, unsigned& h) const noexcept
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
bool SR_RenderWindowWin32::set_size(unsigned w, unsigned h) noexcept
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
    clientArea.left = 0;
    clientArea.right = (int)w;
    clientArea.top = 0;
    clientArea.bottom = (int)h;
    if (AdjustWindowRectEx(&clientArea, WINDOW_STYLE, FALSE, WINDOW_STYLE_EX) == FALSE)
    {
        return false;
    }


    return FALSE != SetWindowPos(
        mHwnd,
        nullptr,
        winRect.left,
        winRect.top,
        (int)clientArea.right,
        (int)clientArea.bottom,
        SWP_NOREPOSITION | SWP_NOSENDCHANGING
    );
}



/*-------------------------------------
* Get the window position (X)
-------------------------------------*/
int SR_RenderWindowWin32::x_position() const noexcept
{
    unsigned x = 0;
    unsigned y = 0;
    get_size(x, y);
    return x;
}



/*-------------------------------------
* Get the window position (Y)
-------------------------------------*/
int SR_RenderWindowWin32::y_position() const noexcept
{
    unsigned x = 0;
    unsigned y = 0;
    get_size(x, y);
    return y;
}



/*-------------------------------------
* Get the window position
-------------------------------------*/
bool SR_RenderWindowWin32::get_position(int& x, int& y) const noexcept
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
bool SR_RenderWindowWin32::set_position(int x, int y) noexcept
{
    if (!valid())
    {
        return false;
    }

    unsigned w = 0;
    unsigned h = 0;
    get_size(w, h);

    return SetWindowPos(mHwnd, nullptr, x, y, w, h, SWP_NOSIZE | SWP_NOSENDCHANGING) != FALSE;
}



/*-------------------------------------
-------------------------------------*/
SR_RenderWindow* SR_RenderWindowWin32::clone() const noexcept
{
    const SR_RenderWindowWin32& self = *this; // nullptr check

    SR_RenderWindowWin32* pWindow = new(std::nothrow) SR_RenderWindowWin32(self);

    return pWindow;
}



/*-------------------------------------
-------------------------------------*/
void SR_RenderWindowWin32::update() noexcept
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
            std::cerr
                << "Encountered unexpected window state " << mCurrentState << '.'
                << std::endl;
            assert(false); // assertions are disabled on release builds
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
bool SR_RenderWindowWin32::pause() noexcept
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
            std::cout << "Window started in a paused state." << std::endl;
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
-------------------------------------*/
bool SR_RenderWindowWin32::run() noexcept
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
            std::cout << "Window started in a running state." << std::endl;
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
-------------------------------------*/
bool SR_RenderWindowWin32::has_event() const noexcept
{
    return mLastMsg.hwnd != nullptr && mLastMsg.message != WM_NULL;
}



/*-------------------------------------
-------------------------------------*/
bool SR_RenderWindowWin32::peek_event(SR_WindowEvent* const pEvent) noexcept
{
    if (!has_event())
    {
        return false;
    }

    RECT clientRect;

    pEvent->pNativeWindow = (ptrdiff_t)mLastMsg.hwnd;

    switch (mLastMsg.message)
    {
    case WM_NULL: // sentinel
        pEvent->type = SR_WinEventType::WIN_EVENT_NONE;
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
            pEvent->type = SR_WinEventType::WIN_EVENT_UNKNOWN;
            return false;
        }

        pEvent->type = WIN_EVENT_KEY_DOWN;
        pEvent->keyboard.keysym = (SR_KeySymbol)mLastMsg.wParam;
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
            pEvent->type = SR_WinEventType::WIN_EVENT_UNKNOWN;
            return false;
        }

        pEvent->type = WIN_EVENT_KEY_DOWN;
        pEvent->keyboard.keysym = (SR_KeySymbol)mLastMsg.wParam;
        pEvent->keyboard.key = 0; // do no additional processing in non-text mode.
        pEvent->keyboard.capsLock = (uint8_t)((GetKeyState(VK_CAPITAL) & 0x0001) != 0);
        pEvent->keyboard.numLock = (uint8_t)((GetKeyState(VK_NUMLOCK) & 0x0001) != 0);
        pEvent->keyboard.scrollLock = (uint8_t)((GetKeyState(VK_SCROLL) & 0x0001) != 0);
        break;

    case WM_KEYUP:
        pEvent->type = WIN_EVENT_KEY_UP;
        pEvent->keyboard.keysym = (SR_KeySymbol)mLastMsg.wParam;
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

    case WM_MOUSEHWHEEL:
        pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_WHEEL_MOVED;
        pEvent->wheel.x = 0;
        pEvent->wheel.y = (int16_t)GET_WHEEL_DELTA_WPARAM(mLastMsg.wParam) / WHEEL_DELTA;
        pEvent->wheel.up = GET_WHEEL_DELTA_WPARAM(mLastMsg.wParam) >= 0;
        pEvent->wheel.down = GET_WHEEL_DELTA_WPARAM(mLastMsg.wParam) < 0;
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
            #ifndef LS_COMPILER_MINGW
                const unsigned dpi = GetDpiForWindow(mHwnd) >> 1;
            #else
                constexpr unsigned dpi = 72;
            #endif
            unsigned w = 0;
            unsigned h = 0;
            get_size(w, h);
            const int w2 = w >> 1;
            const int h2 = h >> 1;
            const int dx = pEvent->mousePos.x;
            const int dy = pEvent->mousePos.y;
            pEvent->mousePos.dx = (int16_t)(w2 - dx) * dpi;
            pEvent->mousePos.dy = (int16_t)(h2 - dy) * dpi;
            mMouseX = dx;
            mMouseY = dy;
        }
        break;

    case WM_POINTERENTER:
        pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_ENTER;
        pEvent->mousePos.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mousePos.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_POINTERLEAVE:
        pEvent->type = SR_WinEventType::WIN_EVENT_MOUSE_LEAVE;
        pEvent->mousePos.x = (int16_t)GET_X_LPARAM(mLastMsg.lParam);
        pEvent->mousePos.y = (int16_t)GET_Y_LPARAM(mLastMsg.lParam);
        break;

    case WM_MOVE:
        pEvent->type = SR_WinEventType::WIN_EVENT_MOVED;
        pEvent->window.x = (uint16_t)LOWORD(mLastMsg.lParam);
        pEvent->window.y = (uint16_t)HIWORD(mLastMsg.lParam);
        break;

    case WM_SIZE:
        pEvent->type = SR_WinEventType::WIN_EVENT_RESIZED;
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
        pEvent->type = SR_WinEventType::WIN_EVENT_UNKNOWN;
        return false;
    }

    return true;
}


/*-------------------------------------
* Remove an event from the event queue
-------------------------------------*/
bool SR_RenderWindowWin32::pop_event(SR_WindowEvent* const pEvent) noexcept
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
bool SR_RenderWindowWin32::set_keys_repeat(bool doKeysRepeat) noexcept
{
    mKeysRepeat = doKeysRepeat;
    return mKeysRepeat;
}


/*-------------------------------------
 * Render a framebuffer to the current window
-------------------------------------*/
void SR_RenderWindowWin32::render(SR_WindowBuffer& buffer) noexcept
{
    assert(this->valid());
    assert(buffer.native_handle() != nullptr);

    SR_WindowBufferWin32& pWinBuffer = static_cast<SR_WindowBufferWin32&>(buffer);

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
        std::cerr << "Failed to blit a framebuffer. Error code: " << GetLastError() << std::endl;
    }

    ReleaseDC(mHwnd, winDc);
}


/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
void SR_RenderWindowWin32::set_mouse_capture(bool isCaptured) noexcept
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
bool SR_RenderWindowWin32::is_mouse_captured() const noexcept
{
    return mCaptureMouse;
}
