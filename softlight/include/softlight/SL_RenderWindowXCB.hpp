
#ifndef SL_RENDER_WINDOW_XCB_HPP
#define SL_RENDER_WINDOW_XCB_HPP

#include <cstdint> // uint32_t

#include "softlight/SL_RenderWindow.hpp"


struct _XDisplay; // Display typedef
struct xcb_connection_t;


/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SL_RenderWindowXCB final : public SL_RenderWindow
{
    friend class SL_WindowBufferXlib;

  private:
    _XDisplay* mDisplay;

    xcb_connection_t* mConnection;

    uint32_t mWindow;

    uint32_t mContext;

    unsigned long mCloseAtom;

    void* mLastEvent;

    void* mPeekedEvent;

    unsigned mWidth;

    unsigned mHeight;

    int mX;

    int mY;

    int mMouseX;

    int mMouseY;

    bool mKeysRepeat;

    bool mCaptureMouse;

  public:
    virtual ~SL_RenderWindowXCB()  noexcept override;

    SL_RenderWindowXCB() noexcept;

    SL_RenderWindowXCB(const SL_RenderWindowXCB&) noexcept;

    SL_RenderWindowXCB(SL_RenderWindowXCB&&) noexcept;

    SL_RenderWindowXCB& operator=(const SL_RenderWindowXCB&) noexcept;

    SL_RenderWindowXCB& operator=(SL_RenderWindowXCB&&) noexcept;

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

    virtual void render(SL_WindowBuffer& buffer) noexcept override;

    virtual void set_mouse_capture(bool isCaptured) noexcept override;

    virtual bool is_mouse_captured() const noexcept override;

    virtual void* native_handle() noexcept override;

    virtual const void* native_handle() const noexcept override;

    virtual unsigned dpi() const noexcept override;
};



/*-------------------------------------
 * Retrieve the window width
-------------------------------------*/
inline unsigned SL_RenderWindowXCB::width() const noexcept
{
    return mWidth;
}



/*-------------------------------------
 * Retrieve the window height
-------------------------------------*/
inline unsigned SL_RenderWindowXCB::height() const noexcept
{
    return mHeight;
}



/*-------------------------------------
 * Retrieve the window get_size
-------------------------------------*/
inline void SL_RenderWindowXCB::get_size(unsigned& w, unsigned& h) const noexcept
{
    w = width();
    h = height();
}



/*-------------------------------------
 * Get the window position (X)
-------------------------------------*/
inline int SL_RenderWindowXCB::x_position() const noexcept
{
    return mX;
}



/*-------------------------------------
 * Get the window position (Y)
-------------------------------------*/
inline int SL_RenderWindowXCB::y_position() const noexcept
{
    return mY;
}



/*-------------------------------------
 * Get the window position
-------------------------------------*/
inline bool SL_RenderWindowXCB::get_position(int& x, int& y) const noexcept
{
    x = x_position();
    y = y_position();

    return true;
}



/*-------------------------------------
 * Determine the windo state
-------------------------------------*/
inline WindowStateInfo SL_RenderWindowXCB::state() const noexcept
{
    return mCurrentState;
}



/*-------------------------------------
 * Check if keyboard keys repeat.
-------------------------------------*/
inline bool SL_RenderWindowXCB::keys_repeat() const noexcept
{
    return mKeysRepeat;
}



/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
inline bool SL_RenderWindowXCB::is_mouse_captured() const noexcept
{
    return mCaptureMouse;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline void* SL_RenderWindowXCB::native_handle() noexcept
{
    return mConnection;
}



/*-------------------------------------
 * Get the native window handle
-------------------------------------*/
inline const void* SL_RenderWindowXCB::native_handle() const noexcept
{
    return mConnection;
}



#endif /* SL_RENDER_WINDOW_XCB_HPP */
