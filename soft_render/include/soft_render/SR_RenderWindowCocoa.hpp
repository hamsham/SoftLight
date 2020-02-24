
#ifndef SR_RENDER_WINDOW_COCOA_HPP
#define SR_RENDER_WINDOW_COCOA_HPP

#include "soft_render/SR_RenderWindow.hpp"



/*-----------------------------------------------------------------------------
 * Cocoa Render Window
-----------------------------------------------------------------------------*/
class SR_RenderWindowCocoa final : public SR_RenderWindow
{
    friend class SR_WindowBufferCocoa;
    friend class SR_BlitProcessor;

  private:
    void* mWindow; // NSWindow

    void* mDelegate; // NSWindowDelegate

    void* mLastEvent; // NSEvent;

    bool mKeysRepeat;

    bool mCaptureMouse;

    int mMouseX;

    int mMouseY;

  public:
    virtual ~SR_RenderWindowCocoa() noexcept override;

    SR_RenderWindowCocoa() noexcept;

    SR_RenderWindowCocoa(const SR_RenderWindowCocoa&) noexcept;

    SR_RenderWindowCocoa(SR_RenderWindowCocoa&&) noexcept;

    SR_RenderWindowCocoa& operator=(const SR_RenderWindowCocoa&) noexcept;

    SR_RenderWindowCocoa& operator=(SR_RenderWindowCocoa&&) noexcept;

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
inline bool SR_RenderWindowCocoa::valid() const noexcept
{
    return mWindow != nullptr;
}


/*-------------------------------------
* Determine the windo state
-------------------------------------*/
inline WindowStateInfo SR_RenderWindowCocoa::state() const noexcept
{
    return mCurrentState;
}


/*-------------------------------------
 * Check if keyboard keys repeat.
-------------------------------------*/
inline bool SR_RenderWindowCocoa::keys_repeat() const noexcept
{
    return mKeysRepeat;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline void*SR_RenderWindowCocoa::native_handle() noexcept
{
    return mWindow;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline const void* SR_RenderWindowCocoa::native_handle() const noexcept
{
    return mWindow;
}



#endif /* SR_RENDER_WINDOW_COCOA_HPP */
