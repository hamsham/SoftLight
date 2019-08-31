
#ifndef SR_RENDERWINDOWWIN32_HPP
#define SR_RENDERWINDOWWIN32_HPP

// Thanks again Visual Studio
#ifndef NOMINMAX
#define NOMINMAX
#endif /* NOMINMAX */

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include "soft_render/SR_RenderWindow.hpp"



/*-----------------------------------------------------------------------------
 * Win32 Render Window
-----------------------------------------------------------------------------*/
class SR_RenderWindowWin32 final : public SR_RenderWindow
{
    friend class SR_WindowBufferWin32;
    friend class SR_BlitProcessor;

  private:
    WNDCLASSEX mWc;

    HWND mHwnd;

    MSG mLastMsg;

    int mMouseX;

    int mMouseY;

    bool mKeysRepeat;

    bool mCaptureMouse;

    static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  public:
    virtual ~SR_RenderWindowWin32() noexcept override;

    SR_RenderWindowWin32() noexcept;

    SR_RenderWindowWin32(const SR_RenderWindowWin32&) noexcept;

    SR_RenderWindowWin32(SR_RenderWindowWin32&&) noexcept;

    SR_RenderWindowWin32& operator=(const SR_RenderWindowWin32&) noexcept;

    SR_RenderWindowWin32& operator=(SR_RenderWindowWin32&&) noexcept;

    int set_title(const char* const pName) noexcept override;

    int init(unsigned width = 640, unsigned height = 480) noexcept override;

    int destroy() noexcept override;

    unsigned width() const noexcept override;

    unsigned height() const noexcept override;

    void get_size(unsigned& w, unsigned& h) const noexcept override;

    bool set_size(unsigned w, unsigned h) noexcept override;

    int x_position() const noexcept override;

    int y_position() const noexcept override;

    bool get_position(int& w, int& h) const noexcept override;

    bool set_position(int x, int y) noexcept override;

    SR_RenderWindow* clone() const noexcept override;

    bool valid() const noexcept override;

    WindowStateInfo state() const noexcept override;

    void update() noexcept override;

    bool pause() noexcept override;

    bool run() noexcept override;

    bool has_event() const noexcept override;

    bool peek_event(SR_WindowEvent* const pEvent) noexcept override;

    bool pop_event(SR_WindowEvent* const pEvent) noexcept override;

    bool set_keys_repeat(bool doKeysRepeat) noexcept override;

    bool keys_repeat() const noexcept override;

    void render(SR_WindowBuffer& buffer) noexcept override;

    void set_mouse_capture(bool isCaptured) noexcept override;

    bool is_mouse_captured() const noexcept override;

    void* native_handle() noexcept override;

    const void* native_handle() const noexcept override;

    unsigned dpi() const noexcept override;
};



/*-------------------------------------
 * Ensure a window is available
-------------------------------------*/
inline bool SR_RenderWindowWin32::valid() const noexcept
{
    return mHwnd != nullptr;
}


/*-------------------------------------
* Determine the windo state
-------------------------------------*/
inline WindowStateInfo SR_RenderWindowWin32::state() const noexcept
{
    return mCurrentState;
}


/*-------------------------------------
 * Check if keyboard keys repeat.
-------------------------------------*/
inline bool SR_RenderWindowWin32::keys_repeat() const noexcept
{
    return mKeysRepeat;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline void*SR_RenderWindowWin32::native_handle() noexcept
{
    return mHwnd;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline const void* SR_RenderWindowWin32::native_handle() const noexcept
{
    return mHwnd;
}



#endif /* SR_RENDERWINDOWWIN32_HPP */
