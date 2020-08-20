
#ifndef SL_RENDER_WINDOW_WIN32_HPP
#define SL_RENDER_WINDOW_WIN32_HPP

// Thanks again Visual Studio
#ifndef NOMINMAX
#define NOMINMAX
#endif /* NOMINMAX */

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include "softlight/SL_RenderWindow.hpp"



/*-----------------------------------------------------------------------------
 * Win32 Render Window
-----------------------------------------------------------------------------*/
class SL_RenderWindowWin32 final : public SL_RenderWindow
{
    friend class SL_WindowBufferWin32;
    friend class SL_BlitProcessor;

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
    virtual ~SL_RenderWindowWin32() noexcept override;

    SL_RenderWindowWin32() noexcept;

    SL_RenderWindowWin32(const SL_RenderWindowWin32&) noexcept;

    SL_RenderWindowWin32(SL_RenderWindowWin32&&) noexcept;

    SL_RenderWindowWin32& operator=(const SL_RenderWindowWin32&) noexcept;

    SL_RenderWindowWin32& operator=(SL_RenderWindowWin32&&) noexcept;

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

    SL_RenderWindow* clone() const noexcept override;

    bool valid() const noexcept override;

    WindowStateInfo state() const noexcept override;

    void update() noexcept override;

    bool pause() noexcept override;

    bool run() noexcept override;

    bool has_event() const noexcept override;

    bool peek_event(SL_WindowEvent* const pEvent) noexcept override;

    bool pop_event(SL_WindowEvent* const pEvent) noexcept override;

    bool set_keys_repeat(bool doKeysRepeat) noexcept override;

    bool keys_repeat() const noexcept override;

    void render(SL_WindowBuffer& buffer) noexcept override;

    void set_mouse_capture(bool isCaptured) noexcept override;

    bool is_mouse_captured() const noexcept override;

    void* native_handle() noexcept override;

    const void* native_handle() const noexcept override;

    unsigned dpi() const noexcept override;
};



/*-------------------------------------
 * Ensure a window is available
-------------------------------------*/
inline bool SL_RenderWindowWin32::valid() const noexcept
{
    return mHwnd != nullptr;
}


/*-------------------------------------
* Determine the windo state
-------------------------------------*/
inline WindowStateInfo SL_RenderWindowWin32::state() const noexcept
{
    return mCurrentState;
}


/*-------------------------------------
 * Check if keyboard keys repeat.
-------------------------------------*/
inline bool SL_RenderWindowWin32::keys_repeat() const noexcept
{
    return mKeysRepeat;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline void*SL_RenderWindowWin32::native_handle() noexcept
{
    return mHwnd;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline const void* SL_RenderWindowWin32::native_handle() const noexcept
{
    return mHwnd;
}



#endif /* SL_RENDER_WINDOW_WIN32_HPP */
