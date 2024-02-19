
#ifndef SL_WINDOWEVENT_HPP
#define SL_WINDOWEVENT_HPP

#include <cstdint> // fixed-width integers



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
enum SL_KeySymbol : uint16_t; // KeySym*.hpp



/*-----------------------------------------------------------------------------
 * Desriptors for window events
-----------------------------------------------------------------------------*/
enum SL_WinEventType : uint32_t
{
    WIN_EVENT_NONE              = 0x00000000, // No usable event data
    WIN_EVENT_UNKNOWN           = 0xFE000000,
    WIN_EVENT_INVALID           = 0xFF000000,

    WIN_EVENT_MOUSE_BUTTON_DOWN = 0x00000001, // SL_MouseButtonEvent
    WIN_EVENT_MOUSE_BUTTON_UP   = 0x00000002,

    WIN_EVENT_MOUSE_WHEEL_MOVED = 0x00000004, // SL_WheelEvent

    WIN_EVENT_MOUSE_MOVED       = 0x00000008, // SL_MousePosEvent
    WIN_EVENT_MOUSE_ENTER       = 0x00000010,
    WIN_EVENT_MOUSE_LEAVE       = 0x00000020,

    WIN_EVENT_KEY_DOWN          = 0x00000040, // SL_KeyEvent
    WIN_EVENT_KEY_UP            = 0x00000080,

    WIN_EVENT_CLOSING           = 0x00000100, // SL_WinUpdateEvent
    WIN_EVENT_HIDDEN            = 0x00000200,
    WIN_EVENT_EXPOSED           = 0x00000400,
    WIN_EVENT_RESIZED           = 0x00000800,
    WIN_EVENT_MOVED             = 0x00001000,

    WIN_EVENT_CLIPBOARD_PASTE   = 0x00002000,
};



/*-----------------------------------------------------------------------------
 * Event Structures
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Keyboard events (64-bits)
-------------------------------------*/
struct alignas(8) SL_KeyEvent
{
    SL_KeySymbol keySym; // symbolic key (common-use)
    uint16_t keyPlatform; // platform-specific symbolic key
    uint8_t keyRaw; // raw-hardware key
    uint8_t capsLock;
    uint8_t numLock;
    uint8_t scrollLock;
};

/*-------------------------------------
 * Mouse Button events (64-bits)
-------------------------------------*/
struct alignas(8) SL_MouseButtonEvent
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
struct alignas(8) SL_WheelEvent
{
    float direction;
    int16_t x;
    int16_t y;
};

/*-------------------------------------
 * Mouse position events (32-bits)
-------------------------------------*/
struct alignas(4) SL_MousePosEvent
{
    int16_t x;
    int16_t y;
    int16_t dx;
    int16_t dy;
};

/*-------------------------------------
 * Window events (64-bits)
-------------------------------------*/
struct alignas(8) SL_WinUpdateEvent
{
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
};



/*-------------------------------------
 * Clipboard paste events (64-bits)
-------------------------------------*/
struct alignas(8) SL_ClipboardEvent
{
    const unsigned char* paste;
};


/*-----------------------------------------------------------------------------
 * Generic event container
-----------------------------------------------------------------------------*/
struct SL_WindowEvent
{
    // 64
    alignas(8) SL_WinEventType type;

    // 32-64
    alignas(8) ptrdiff_t pNativeWindow;

    // 64
    union alignas(8)
    {
        SL_KeyEvent keyboard;
        SL_MouseButtonEvent mouseButton;
        SL_WheelEvent wheel;
        SL_MousePosEvent mousePos;
        SL_WinUpdateEvent window;
        SL_ClipboardEvent clipboard;
    };
};


#endif  /* SL_WINDOWEVENT_HPP */
