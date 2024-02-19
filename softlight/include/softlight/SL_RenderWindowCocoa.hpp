
#ifndef SL_RENDER_WINDOW_COCOA_HPP
#define SL_RENDER_WINDOW_COCOA_HPP

#include "softlight/SL_RenderWindow.hpp"



/*-----------------------------------------------------------------------------
 * Cocoa Render Window
-----------------------------------------------------------------------------*/
class SL_RenderWindowCocoa final : public SL_RenderWindow
{
    friend class SL_SwapchainCocoa;
    friend class SL_BlitProcessor;

  private:
    void* mWindow; // NSWindow

    void* mDelegate; // NSWindowDelegate

    void* mLastEvent; // NSEvent;

    bool mKeysRepeat;

    bool mCaptureMouse;

    int mMouseX;

    int mMouseY;

  public:
    virtual ~SL_RenderWindowCocoa() noexcept override;

    SL_RenderWindowCocoa() noexcept;

    SL_RenderWindowCocoa(const SL_RenderWindowCocoa&) noexcept;

    SL_RenderWindowCocoa(SL_RenderWindowCocoa&&) noexcept;

    SL_RenderWindowCocoa& operator=(const SL_RenderWindowCocoa&) noexcept;

    SL_RenderWindowCocoa& operator=(SL_RenderWindowCocoa&&) noexcept;

    virtual SL_WindowBackend backend() const noexcept override;

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

    void render(SL_Swapchain& buffer) noexcept override;

    void set_mouse_capture(bool isCaptured) noexcept override;

    bool is_mouse_captured() const noexcept override;

    void* native_handle() noexcept override;

    const void* native_handle() const noexcept override;

    unsigned dpi() const noexcept override;

    void request_clipboard() const noexcept override;
};



/*-------------------------------------
 * Ensure a window is available
-------------------------------------*/
inline bool SL_RenderWindowCocoa::valid() const noexcept
{
    return mWindow != nullptr;
}


/*-------------------------------------
* Determine the windo state
-------------------------------------*/
inline WindowStateInfo SL_RenderWindowCocoa::state() const noexcept
{
    return mCurrentState;
}


/*-------------------------------------
 * Check if keyboard keys repeat.
-------------------------------------*/
inline bool SL_RenderWindowCocoa::keys_repeat() const noexcept
{
    return mKeysRepeat;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline void*SL_RenderWindowCocoa::native_handle() noexcept
{
    return mWindow;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline const void* SL_RenderWindowCocoa::native_handle() const noexcept
{
    return mWindow;
}



#endif /* SL_RENDER_WINDOW_COCOA_HPP */
