
#ifndef SR_RENDERWINDOW_XCB_HPP
#define SR_RENDERWINDOW_XCB_HPP

#include <cstdint> // uint32_t

#include "soft_render/SR_RenderWindow.hpp"


struct _XDisplay; // Display typedef
struct xcb_connection_t;


/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SR_RenderWindowXCB final : public SR_RenderWindow
{
    friend class SR_WindowBufferXlib;

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
    virtual ~SR_RenderWindowXCB()  noexcept override;

    SR_RenderWindowXCB() noexcept;

    SR_RenderWindowXCB(const SR_RenderWindowXCB&) noexcept;

    SR_RenderWindowXCB(SR_RenderWindowXCB&&) noexcept;

    SR_RenderWindowXCB& operator=(const SR_RenderWindowXCB&) noexcept;

    SR_RenderWindowXCB& operator=(SR_RenderWindowXCB&&) noexcept;

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

    virtual SR_RenderWindow* clone() const noexcept override;

    virtual bool valid() const noexcept override;

    virtual WindowStateInfo state() const noexcept override;

    virtual void update() noexcept override;

    virtual bool pause() noexcept override;

    virtual bool run() noexcept override;

    virtual bool has_event() const noexcept override;

    virtual bool peek_event(SR_WindowEvent* const pEvent) noexcept override;

    virtual bool pop_event(SR_WindowEvent* const pEvent) noexcept override;

    virtual bool set_keys_repeat(bool doKeysRepeat) noexcept override;

    virtual bool keys_repeat() const noexcept override;

    virtual void render(SR_WindowBuffer& buffer) noexcept override;

    virtual void set_mouse_capture(bool isCaptured) noexcept override;

    virtual bool is_mouse_captured() const noexcept override;
};



/*-------------------------------------
 * Retrieve the window width
-------------------------------------*/
inline unsigned SR_RenderWindowXCB::width() const noexcept
{
    return mWidth;
}



/*-------------------------------------
 * Retrieve the window height
-------------------------------------*/
inline unsigned SR_RenderWindowXCB::height() const noexcept
{
    return mHeight;
}



/*-------------------------------------
 * Retrieve the window get_size
-------------------------------------*/
inline void SR_RenderWindowXCB::get_size(unsigned& w, unsigned& h) const noexcept
{
    w = width();
    h = height();
}



/*-------------------------------------
 * Get the window position (X)
-------------------------------------*/
inline int SR_RenderWindowXCB::x_position() const noexcept
{
    return mX;
}



/*-------------------------------------
 * Get the window position (Y)
-------------------------------------*/
inline int SR_RenderWindowXCB::y_position() const noexcept
{
    return mY;
}



/*-------------------------------------
 * Get the window position
-------------------------------------*/
inline bool SR_RenderWindowXCB::get_position(int& x, int& y) const noexcept
{
    x = x_position();
    y = y_position();

    return true;
}



/*-------------------------------------
 * Determine the windo state
-------------------------------------*/
inline WindowStateInfo SR_RenderWindowXCB::state() const noexcept
{
    return mCurrentState;
}



/*-------------------------------------
 * Check if keyboard keys repeat.
-------------------------------------*/
inline bool SR_RenderWindowXCB::keys_repeat() const noexcept
{
    return mKeysRepeat;
}



/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
inline bool SR_RenderWindowXCB::is_mouse_captured() const noexcept
{
    return mCaptureMouse;
}



#endif /* SR_RENDERWINDOW_XCB_HPP */
