
#ifndef SL_KEYSYM_WIN32_HPP
#define SL_KEYSYM_WIN32_HPP

// Thanks again Visual Studio
#ifndef NOMINMAX
    #define NOMINMAX
#endif /* NOMINMAX */

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include <cstdint> // uint16_t



/**------------------------------------
 * @brief Keyboard Symbols
-------------------------------------*/
enum SL_KeySymbol : uint32_t
{
  KEY_SYM_ESCAPE = VK_ESCAPE,

  KEY_SYM_L_SHIFT   = VK_LSHIFT,
  KEY_SYM_L_CONTROL = VK_LCONTROL,
  KEY_SYM_L_ALT     = VK_LMENU,
  KEY_SYM_L_SUPER   = VK_LWIN,
  KEY_SYM_L_META    = VK_APPS,

  KEY_SYM_R_SHIFT   = VK_RSHIFT,
  KEY_SYM_R_CONTROL = VK_RCONTROL,
  KEY_SYM_R_ALT     = VK_MENU,
  KEY_SYM_R_SUPER   = VK_RWIN,
  KEY_SYM_R_META    = VK_APPS,

  KEY_SYM_F1  = VK_F1,
  KEY_SYM_F2  = VK_F2,
  KEY_SYM_F3  = VK_F3,
  KEY_SYM_F4  = VK_F4,
  KEY_SYM_F5  = VK_F5,
  KEY_SYM_F6  = VK_F6,
  KEY_SYM_F7  = VK_F7,
  KEY_SYM_F8  = VK_F8,
  KEY_SYM_F9  = VK_F9,
  KEY_SYM_F10 = VK_F10,
  KEY_SYM_F11 = VK_F11,
  KEY_SYM_F12 = VK_F12,
  KEY_SYM_F13 = VK_F13,
  KEY_SYM_F14 = VK_F14,
  KEY_SYM_F15 = VK_F15,
  KEY_SYM_F16 = VK_F16,
  KEY_SYM_F17 = VK_F17,
  KEY_SYM_F18 = VK_F18,
  KEY_SYM_F19 = VK_F19,
  KEY_SYM_F20 = VK_F20,
  KEY_SYM_F21 = VK_F21,
  KEY_SYM_F22 = VK_F22,
  KEY_SYM_F23 = VK_F23,
  KEY_SYM_F24 = VK_F24,

  KEY_SYM_1 = 0x31,
  KEY_SYM_2 = 0x32,
  KEY_SYM_3 = 0x33,
  KEY_SYM_4 = 0x34,
  KEY_SYM_5 = 0x35,
  KEY_SYM_6 = 0x36,
  KEY_SYM_7 = 0x37,
  KEY_SYM_8 = 0x38,
  KEY_SYM_9 = 0x39,
  KEY_SYM_0 = 0x30,

  KEY_SYM_NUMPAD_1       = VK_NUMPAD1,
  KEY_SYM_NUMPAD_2       = VK_NUMPAD2,
  KEY_SYM_NUMPAD_3       = VK_NUMPAD3,
  KEY_SYM_NUMPAD_4       = VK_NUMPAD4,
  KEY_SYM_NUMPAD_5       = VK_NUMPAD5,
  KEY_SYM_NUMPAD_6       = VK_NUMPAD6,
  KEY_SYM_NUMPAD_7       = VK_NUMPAD7,
  KEY_SYM_NUMPAD_8       = VK_NUMPAD8,
  KEY_SYM_NUMPAD_9       = VK_NUMPAD9,
  KEY_SYM_NUMPAD_0       = VK_NUMPAD0,
  KEY_SYM_NUMPAD_END     = VK_END,     // Numpad alias: 1
  KEY_SYM_NUMPAD_DOWN    = VK_DOWN,    // Numpad alias: 2
  KEY_SYM_NUMPAD_PG_DOWN = VK_NEXT,    // Numpad alias: 3
  KEY_SYM_NUMPAD_LEFT    = VK_LEFT,    // Numpad alias: 4
  KEY_SYM_NUMPAD_BEGIN   = VK_NUMPAD5, // Numpad alias: 5
  KEY_SYM_NUMPAD_RIGHT   = VK_RIGHT,   // Numpad alias: 6
  KEY_SYM_NUMPAD_HOME    = VK_HOME,    // Numpad alias: 7
  KEY_SYM_NUMPAD_UP      = VK_UP,      // Numpad alias: 8
  KEY_SYM_NUMPAD_PG_UP   = VK_PRIOR,   // Numpad alias: 9
  KEY_SYM_NUMPAD_INSERT  = VK_INSERT,  // Numpad alias: 0
  KEY_SYM_NUMPAD_ADD     = VK_ADD,
  KEY_SYM_NUMPAD_SUB     = VK_SUBTRACT,
  KEY_SYM_NUMPAD_MUL     = VK_MULTIPLY,
  KEY_SYM_NUMPAD_DIV     = VK_DIVIDE,
  KEY_SYM_NUMPAD_EQUAL   = VK_OEM_PLUS,
  KEY_SYM_NUMPAD_ENTER   = VK_RETURN,
  KEY_SYM_NUMPAD_DELETE  = VK_DELETE,
  KEY_SYM_NUMPAD_DECIMAL = VK_DECIMAL,
  KEY_SYM_NUMPAD_SEP     = VK_OEM_5,

  KEY_SYM_PRINT_SCREEN = VK_SNAPSHOT,
  KEY_SYM_PAUSE        = VK_PAUSE,
  KEY_SYM_SYS_REQ      = VK_EXECUTE,
  KEY_SYM_INSERT       = VK_INSERT,
  KEY_SYM_DELETE       = VK_DELETE,
  KEY_SYM_HOME         = VK_HOME,
  KEY_SYM_END          = VK_END,
  KEY_SYM_PG_UP        = VK_PRIOR,
  KEY_SYM_PG_DOWN      = VK_NEXT,
  KEY_SYM_LEFT         = VK_LEFT,
  KEY_SYM_RIGHT        = VK_RIGHT,
  KEY_SYM_UP           = VK_UP,
  KEY_SYM_DOWN         = VK_DOWN,
  KEY_SYM_SPACE        = VK_SPACE,
  KEY_SYM_BACKSPACE    = VK_BACK,
  KEY_SYM_LINE_FEED    = VK_RETURN,
  KEY_SYM_RETURN       = VK_RETURN, // Enter, return
  KEY_SYM_TAB          = VK_TAB,
  KEY_SYM_CLEAR        = VK_CLEAR,

  KEY_SYM_CAPS_LOCK   = VK_CAPITAL,
  KEY_SYM_NUM_LOCK    = VK_NUMLOCK,
  KEY_SYM_SCROLL_LOCK = VK_SCROLL,

  KEY_SYM_SINGLE_QUOTE = VK_OEM_7, // '
  KEY_SYM_DOUBLE_QUOTE = VK_OEM_7, // "
  KEY_SYM_TILDE        = VK_OEM_3, // ~
  KEY_SYM_GRAVE        = VK_OEM_3, // `
  KEY_SYM_AT           = 0x32, // @
  KEY_SYM_POUND        = 0x33, // #
  KEY_SYM_DOLLAR       = 0x34, // $
  KEY_SYM_PERCENT      = 0x35, // %
  KEY_SYM_CARET        = 0x36, // ^
  KEY_SYM_AMPERSAND    = 0x37, // &
  KEY_SYM_ASTERISK     = 0x38, // *
  KEY_SYM_UNDERSCORE   = VK_OEM_MINUS, // _
  KEY_SYM_HYPHEN       = VK_OEM_MINUS, // -
  KEY_SYM_PLUS         = VK_OEM_PLUS, // +
  KEY_SYM_EQUALS       = VK_OEM_PLUS, // =

  KEY_SYM_PARENTHESIS_LEFT  = 0x39,
  KEY_SYM_PARENTHESIS_RIGHT = 0x39,
  KEY_SYM_BRACKET_LEFT      = VK_OEM_4, // [
  KEY_SYM_BRACKET_RIGHT     = VK_OEM_6, // ]
  KEY_SYM_BRACE_LEFT        = VK_OEM_4, // {
  KEY_SYM_BRACE_RIGHT       = VK_OEM_6, // }
  KEY_SYM_ANGLE_LEFT        = VK_OEM_COMMA, // <
  KEY_SYM_ANGLE_RIGHT       = VK_OEM_PERIOD, // >
  KEY_SYM_FORWARD_SLASH     = VK_OEM_2,     /* / */
  KEY_SYM_BACKWARD_SLASH    = VK_OEM_5, /* \ */
  KEY_SYM_VERTICAL_BAR      = VK_OEM_5,       /* | */
  KEY_SYM_SEMICOLON         = VK_OEM_1,
  KEY_SYM_COLON             = VK_OEM_1,
  KEY_SYM_COMMA             = VK_OEM_COMMA,
  KEY_SYM_PERIOD            = VK_OEM_PERIOD,
  KEY_SYM_EXCLAMATION       = VK_OEM_8,
  KEY_SYM_QUESTION          = VK_OEM_2,

  KEY_SYM_a = 0x41,
  KEY_SYM_b = 0x42,
  KEY_SYM_c = 0x43,
  KEY_SYM_d = 0x44,
  KEY_SYM_e = 0x45,
  KEY_SYM_f = 0x46,
  KEY_SYM_g = 0x47,
  KEY_SYM_h = 0x48,
  KEY_SYM_i = 0x49,
  KEY_SYM_j = 0x4A,
  KEY_SYM_k = 0x4B,
  KEY_SYM_l = 0x4C,
  KEY_SYM_m = 0x4D,
  KEY_SYM_n = 0x4E,
  KEY_SYM_o = 0x4F,
  KEY_SYM_p = 0x50,
  KEY_SYM_q = 0x51,
  KEY_SYM_r = 0x52,
  KEY_SYM_s = 0x53,
  KEY_SYM_t = 0x54,
  KEY_SYM_u = 0x55,
  KEY_SYM_v = 0x56,
  KEY_SYM_w = 0x57,
  KEY_SYM_x = 0x58,
  KEY_SYM_y = 0x59,
  KEY_SYM_z = 0x5A,

  KEY_SYM_A = 0x41,
  KEY_SYM_B = 0x42,
  KEY_SYM_C = 0x43,
  KEY_SYM_D = 0x44,
  KEY_SYM_E = 0x45,
  KEY_SYM_F = 0x46,
  KEY_SYM_G = 0x47,
  KEY_SYM_H = 0x48,
  KEY_SYM_I = 0x49,
  KEY_SYM_J = 0x4A,
  KEY_SYM_K = 0x4B,
  KEY_SYM_L = 0x4C,
  KEY_SYM_M = 0x4D,
  KEY_SYM_N = 0x4E,
  KEY_SYM_O = 0x4F,
  KEY_SYM_P = 0x50,
  KEY_SYM_Q = 0x51,
  KEY_SYM_R = 0x52,
  KEY_SYM_S = 0x53,
  KEY_SYM_T = 0x54,
  KEY_SYM_U = 0x55,
  KEY_SYM_V = 0x56,
  KEY_SYM_W = 0x57,
  KEY_SYM_X = 0x58,
  KEY_SYM_Y = 0x59,
  KEY_SYM_Z = 0x5A,

  KEY_SYM_UNKNOWN = 0x00
};



#endif /* SL_KEYSYM_WIN32_HPP */
