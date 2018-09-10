
#ifndef SR_WINDOWEVENT_HPP
#define SR_WINDOWEVENT_HPP

#include <cstdint> // fixed-width integers



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
enum SR_KeySymbol : uint32_t; // KeySym*.hpp



/*-----------------------------------------------------------------------------
 * Desriptors for window events
-----------------------------------------------------------------------------*/
enum SR_WinEventType : uint32_t
{
    WIN_EVENT_NONE = 0x00000001,

    // SR_MouseButtonEvent
    WIN_EVENT_MOUSE_BUTTON_DOWN = 0x00000001,
    WIN_EVENT_MOUSE_BUTTON_UP = 0x00000002,

    // SR_WheelEvent
    WIN_EVENT_MOUSE_WHEEL_MOVED = 0x00000004,

    // SR_MousePosEvent
    WIN_EVENT_MOUSE_MOVED = 0x00000008,
    WIN_EVENT_MOUSE_ENTER = 0x00000010,
    WIN_EVENT_MOUSE_LEAVE = 0x00000020,

    // SR_KeyEvent
    WIN_EVENT_KEY_DOWN = 0x00000040,
    WIN_EVENT_KEY_UP = 0x00000080,

    // SR_WinUpdateEvent
    WIN_EVENT_CLOSING = 0x00000100,
    WIN_EVENT_HIDDEN = 0x00000200,
    WIN_EVENT_EXPOSED = 0x00000400,
    WIN_EVENT_RESIZED = 0x00000800,
    WIN_EVENT_MOVED = 0x00001000,

    // No event data
    WIN_EVENT_UNKNOWN = 0xFE000000,
    WIN_EVENT_INVALID = 0xFF000000
};



/*-----------------------------------------------------------------------------
 * Event Structures
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Keyboard events (64-bits)
-------------------------------------*/
struct alignas(8) SR_KeyEvent
{
    SR_KeySymbol keysym; // symbolic key (common-use)
    uint8_t key; // raw-hardware key
    uint8_t capsLock;
    uint8_t numLock;
    uint8_t scrollLock;
};

/*-------------------------------------
 * Mouse Button events (64-bits)
-------------------------------------*/
struct alignas(8) SR_MouseButtonEvent
{
    uint8_t mouseButton1;
    uint8_t mouseButton2;
    uint8_t mouseButton3;
    uint8_t mouseButtonN;
    int16_t x;
    int16_t y;
};

/*-------------------------------------
 * Mouse wheel events (64-bits)
-------------------------------------*/
struct alignas(8) SR_WheelEvent
{
    int16_t x;
    int16_t y;
    int16_t up;
    int16_t down;
};

/*-------------------------------------
 * Mouse position events (32-bits)
-------------------------------------*/
struct alignas(4) SR_MousePosEvent
{
    int16_t x;
    int16_t y;
    int16_t dx;
    int16_t dy;
};

/*-------------------------------------
 * Window events (64-bits)
-------------------------------------*/
struct alignas(8) SR_WinUpdateEvent
{
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
};


/*-----------------------------------------------------------------------------
 * Generic event container
-----------------------------------------------------------------------------*/
struct SR_WindowEvent
{
    // 64
    alignas(8) SR_WinEventType type;

    // 32-64
    alignas(8) ptrdiff_t pNativeWindow;

    // 64
    union alignas(8)
    {
        SR_KeyEvent keyboard;
        SR_MouseButtonEvent mouseButton;
        SR_WheelEvent wheel;
        SR_MousePosEvent mousePos;
        SR_WinUpdateEvent window;
    };
};


#endif  /* SR_WINDOWEVENT_HPP */
