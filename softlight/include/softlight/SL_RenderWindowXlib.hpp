
#ifndef SL_RENDER_WINDOW_XLIB_HPP
#define SL_RENDER_WINDOW_XLIB_HPP

#include "softlight/SL_RenderWindow.hpp"



struct _XDisplay; // Display typedef
union _XEvent; // XEvent typedef



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SL_RenderWindowXlib final : public SL_RenderWindow
{
    friend class SL_SwapchainXlib;

  private:
    _XDisplay* mDisplay;

    unsigned long mWindow;

    unsigned long mCloseAtom;

    _XEvent* mLastEvent;

    unsigned mWidth;

    unsigned mHeight;

    int mX;

    int mY;

    int mMouseX;

    int mMouseY;

    bool mKeysRepeat;

    bool mCaptureMouse;

    unsigned char* mClipboard;

    unsigned char* read_clipboard(const _XEvent*) const noexcept;

  public:
    virtual ~SL_RenderWindowXlib()  noexcept override;

    SL_RenderWindowXlib() noexcept;

    SL_RenderWindowXlib(const SL_RenderWindowXlib&) noexcept;

    SL_RenderWindowXlib(SL_RenderWindowXlib&&) noexcept;

    SL_RenderWindowXlib& operator=(const SL_RenderWindowXlib&) noexcept;

    SL_RenderWindowXlib& operator=(SL_RenderWindowXlib&&) noexcept;

    virtual SL_WindowBackend backend() const noexcept override;

    virtual int set_title(const char* const pName) noexcept override;

    virtual int init(unsigned width = 640, unsigned height = 480) noexcept override;

    virtual int destroy() noexcept override;

    virtual unsigned width() const noexcept override;

    virtual unsigned height() const noexcept override;

    virtual void get_size(unsigned& w, unsigned& h) const noexcept override;

    virtual bool set_size(unsigned w, unsigned h) noexcept override;

    virtual int x_position() const noexcept override;

    virtual int y_position() const noexcept override;

    virtual bool get_position(int& w, int& h) const noexcept override;

    virtual bool set_position(int x, int y) noexcept override;

    virtual SL_RenderWindow* clone() const noexcept override;

    virtual bool valid() const noexcept override;

    virtual WindowStateInfo state() const noexcept override;

    virtual void update() noexcept override;

    virtual bool pause() noexcept override;

    virtual bool run() noexcept override;

    virtual bool has_event() const noexcept override;

    virtual bool peek_event(SL_WindowEvent* const pEvent) noexcept override;

    virtual bool pop_event(SL_WindowEvent* const pEvent) noexcept override;

    virtual bool set_keys_repeat(bool doKeysRepeat) noexcept override;

    virtual bool keys_repeat() const noexcept override;

    virtual void render(SL_Swapchain& buffer) noexcept override;

    virtual void set_mouse_capture(bool isCaptured) noexcept override;

    virtual bool is_mouse_captured() const noexcept override;

    virtual void* native_handle() noexcept override;

    virtual const void* native_handle() const noexcept override;

    virtual unsigned dpi() const noexcept override;

    void request_clipboard() const noexcept override;
};



/*-------------------------------------
 * Retrieve the window width
-------------------------------------*/
inline unsigned SL_RenderWindowXlib::width() const noexcept
{
    return mWidth;
}



/*-------------------------------------
 * Retrieve the window height
-------------------------------------*/
inline unsigned SL_RenderWindowXlib::height() const noexcept
{
    return mHeight;
}



/*-------------------------------------
 * Retrieve the window get_size
-------------------------------------*/
inline void SL_RenderWindowXlib::get_size(unsigned& w, unsigned& h) const noexcept
{
    w = width();
    h = height();
}



/*-------------------------------------
 * Get the window position (X)
-------------------------------------*/
inline int SL_RenderWindowXlib::x_position() const noexcept
{
    return mX;
}



/*-------------------------------------
 * Get the window position (Y)
-------------------------------------*/
inline int SL_RenderWindowXlib::y_position() const noexcept
{
    return mY;
}



/*-------------------------------------
 * Get the window position
-------------------------------------*/
inline bool SL_RenderWindowXlib::get_position(int& x, int& y) const noexcept
{
    x = x_position();
    y = y_position();

    return true;
}



/*-------------------------------------
 * Determine the windo state
-------------------------------------*/
inline WindowStateInfo SL_RenderWindowXlib::state() const noexcept
{
    return mCurrentState;
}



/*-------------------------------------
 * Check if keyboard keys repeat.
-------------------------------------*/
inline bool SL_RenderWindowXlib::keys_repeat() const noexcept
{
    return mKeysRepeat;
}



/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
inline bool SL_RenderWindowXlib::is_mouse_captured() const noexcept
{
    return mCaptureMouse;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline void* SL_RenderWindowXlib::native_handle() noexcept
{
    return &mWindow;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline const void* SL_RenderWindowXlib::native_handle() const noexcept
{
    return &mWindow;
}



#endif /* SL_RENDER_WINDOW_XLIB_HPP */
