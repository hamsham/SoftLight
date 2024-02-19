/*
 * TODO:
 *
 * - Support window parenting
 *
 * - Integrate window buffers
 */

#ifndef SL_RENDER_WINDOW_HPP
#define SL_RENDER_WINDOW_HPP

#include <cstddef> // ptrdiff_t

#include "lightsky/utils/Pointer.h"



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SL_Swapchain;
struct SL_WindowEvent;



/*-----------------------------------------------------------------------------
 * Enumeration to capture the state of a window
-----------------------------------------------------------------------------*/
enum WindowStateInfo
{
    WINDOW_STARTING,
    WINDOW_STARTED,
    WINDOW_RUNNING,
    WINDOW_PAUSED,
    WINDOW_CLOSED,
    WINDOW_CLOSING
};



/*-----------------------------------------------------------------------------
 * Get the windowing backend
-----------------------------------------------------------------------------*/
enum class SL_WindowBackend
{
    WIN32,
    COCOA,
    XCB,
    X11,
};



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SL_RenderWindow
{
  public:
    static ls::utils::Pointer<SL_RenderWindow> create() noexcept;

  protected:
    WindowStateInfo mCurrentState;

  public:
    virtual ~SL_RenderWindow()  noexcept = 0;

    SL_RenderWindow() noexcept;

    SL_RenderWindow(const SL_RenderWindow&) noexcept;

    SL_RenderWindow(SL_RenderWindow&&) noexcept;

    SL_RenderWindow& operator=(const SL_RenderWindow&) noexcept;

    SL_RenderWindow& operator=(SL_RenderWindow&&) noexcept;

    virtual SL_WindowBackend backend() const noexcept = 0;

    virtual int set_title(const char* const pName) noexcept = 0;

    virtual int init(unsigned width = 640, unsigned height = 480) noexcept = 0;

    virtual int destroy() noexcept = 0;

    virtual unsigned width() const noexcept = 0;

    virtual unsigned height() const noexcept = 0;

    virtual void get_size(unsigned& width, unsigned& height) const noexcept = 0;

    virtual bool set_size(unsigned width, unsigned height) noexcept = 0;

    virtual int x_position() const noexcept = 0;

    virtual int y_position() const noexcept = 0;

    virtual bool get_position(int& width, int& height) const noexcept = 0;

    virtual bool set_position(int x, int y) noexcept = 0;

    virtual SL_RenderWindow* clone() const noexcept = 0;

    virtual bool valid() const noexcept = 0;

    virtual WindowStateInfo state() const noexcept = 0;

    virtual void update() noexcept = 0;

    virtual bool pause() noexcept = 0;

    virtual bool run() noexcept = 0;

    virtual bool has_event() const noexcept = 0;

    virtual bool peek_event(SL_WindowEvent* const pEvent) noexcept = 0;

    virtual bool pop_event(SL_WindowEvent* const pEvent) noexcept = 0;

    virtual bool set_keys_repeat(bool doKeysRepeat) noexcept = 0;

    virtual bool keys_repeat() const noexcept = 0;

    virtual void render(SL_Swapchain& buffer) noexcept = 0;

    virtual void set_mouse_capture(bool isCaptured) noexcept = 0;

    virtual bool is_mouse_captured() const noexcept = 0;

    virtual void* native_handle() noexcept = 0;

    virtual const void* native_handle() const noexcept = 0;

    virtual unsigned dpi() const noexcept = 0;

    virtual void request_clipboard() const noexcept = 0;
};



#endif /* SL_RENDER_WINDOW_HPP */
