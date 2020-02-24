
#ifndef SR_KEYSYM_COCOA_HPP
#define SR_KEYSYM_COCOA_HPP

#include <cstdint> // uint16_t



/**------------------------------------
 * @brief Keyboard Symbols
-------------------------------------*/
enum SR_KeySymbol : uint32_t
{
  KEY_SYM_ESCAPE = 0x35,

  KEY_SYM_L_SHIFT   = 0x38,
  KEY_SYM_L_CONTROL = 0x3B,
  KEY_SYM_L_ALT     = 0x3A,
  KEY_SYM_L_SUPER   = 0x37,
  KEY_SYM_L_META    = 0x3F,

  KEY_SYM_R_SHIFT   = 0x3C,
  KEY_SYM_R_CONTROL = 0x3E,
  KEY_SYM_R_ALT     = 0x3D,
  KEY_SYM_R_SUPER   = 0x37,
  KEY_SYM_R_META    = 0x3F,

  KEY_SYM_F1  = 0x7A,
  KEY_SYM_F2  = 0x78,
  KEY_SYM_F3  = 0x63,
  KEY_SYM_F4  = 0x76,
  KEY_SYM_F5  = 0x60,
  KEY_SYM_F6  = 0x61,
  KEY_SYM_F7  = 0x62,
  KEY_SYM_F8  = 0x64,
  KEY_SYM_F9  = 0x65,
  KEY_SYM_F10 = 0x6D,
  KEY_SYM_F11 = 0x67,
  KEY_SYM_F12 = 0x6F,
  KEY_SYM_F13 = 0x69,
  KEY_SYM_F14 = 0x6B,
  KEY_SYM_F15 = 0x71,
  KEY_SYM_F16 = 0x6A,
  KEY_SYM_F17 = 0x40,
  KEY_SYM_F18 = 0x4F,
  KEY_SYM_F19 = 0x50,
  KEY_SYM_F20 = 0x5A,
  KEY_SYM_F21 = 0xF718,
  KEY_SYM_F22 = 0xF719,
  KEY_SYM_F23 = 0xF71A,
  KEY_SYM_F24 = 0xF71B,

  KEY_SYM_1 = 0x12,
  KEY_SYM_2 = 0x13,
  KEY_SYM_3 = 0x14,
  KEY_SYM_4 = 0x15,
  KEY_SYM_5 = 0x17,
  KEY_SYM_6 = 0x16,
  KEY_SYM_7 = 0x1A,
  KEY_SYM_8 = 0x1C,
  KEY_SYM_9 = 0x19,
  KEY_SYM_0 = 0x1D,

  KEY_SYM_NUMPAD_1       = 0x53,
  KEY_SYM_NUMPAD_2       = 0x54,
  KEY_SYM_NUMPAD_3       = 0x55,
  KEY_SYM_NUMPAD_4       = 0x56,
  KEY_SYM_NUMPAD_5       = 0x57,
  KEY_SYM_NUMPAD_6       = 0x58,
  KEY_SYM_NUMPAD_7       = 0x59,
  KEY_SYM_NUMPAD_8       = 0x5B,
  KEY_SYM_NUMPAD_9       = 0x5C,
  KEY_SYM_NUMPAD_0       = 0x52,
  KEY_SYM_NUMPAD_END     = KEY_SYM_NUMPAD_1, // Numpad alias: 1
  KEY_SYM_NUMPAD_DOWN    = KEY_SYM_NUMPAD_2, // Numpad alias: 2
  KEY_SYM_NUMPAD_PG_DOWN = KEY_SYM_NUMPAD_3, // Numpad alias: 3
  KEY_SYM_NUMPAD_LEFT    = KEY_SYM_NUMPAD_4, // Numpad alias: 4
  KEY_SYM_NUMPAD_BEGIN   = KEY_SYM_NUMPAD_5, // Numpad alias: 5
  KEY_SYM_NUMPAD_RIGHT   = KEY_SYM_NUMPAD_6, // Numpad alias: 6
  KEY_SYM_NUMPAD_HOME    = KEY_SYM_NUMPAD_7, // Numpad alias: 7
  KEY_SYM_NUMPAD_UP      = KEY_SYM_NUMPAD_8, // Numpad alias: 8
  KEY_SYM_NUMPAD_PG_UP   = KEY_SYM_NUMPAD_9, // Numpad alias: 9
  KEY_SYM_NUMPAD_INSERT  = KEY_SYM_NUMPAD_0, // Numpad alias: 0
  KEY_SYM_NUMPAD_ADD     = 0x45,
  KEY_SYM_NUMPAD_SUB     = 0x4E,
  KEY_SYM_NUMPAD_MUL     = 0x43,
  KEY_SYM_NUMPAD_DIV     = 0x4B,
  KEY_SYM_NUMPAD_EQUAL   = 0x51,
  KEY_SYM_NUMPAD_ENTER   = 0x4C,
  KEY_SYM_NUMPAD_DELETE  = 0x47,
  KEY_SYM_NUMPAD_DECIMAL = 0x41,
  KEY_SYM_NUMPAD_SEP     = 0xFF,

  KEY_SYM_PRINT_SCREEN = KEY_SYM_F13,
  KEY_SYM_PAUSE        = KEY_SYM_F15,
  KEY_SYM_SYS_REQ      = KEY_SYM_F13,
  KEY_SYM_INSERT       = 0x3F,
  KEY_SYM_DELETE       = 0x75,
  KEY_SYM_HOME         = 0x73,
  KEY_SYM_END          = 0x77,
  KEY_SYM_PG_UP        = 0x74,
  KEY_SYM_PG_DOWN      = 0x79,
  KEY_SYM_LEFT         = 0x7B,
  KEY_SYM_RIGHT        = 0x7C,
  KEY_SYM_UP           = 0x7E,
  KEY_SYM_DOWN         = 0x7D,
  KEY_SYM_SPACE        = 0x31,
  KEY_SYM_BACKSPACE    = 0x33,
  KEY_SYM_LINE_FEED    = 0x24,
  KEY_SYM_RETURN       = 0x24, // Enter, return
  KEY_SYM_TAB          = 0x30,
  KEY_SYM_CLEAR        = 0x47,

  KEY_SYM_CAPS_LOCK   = 0x39,
  KEY_SYM_NUM_LOCK    = 0x47,
  KEY_SYM_SCROLL_LOCK = KEY_SYM_F14,

  KEY_SYM_SINGLE_QUOTE = 0x27, // '
  KEY_SYM_DOUBLE_QUOTE = 0x27, // "
  KEY_SYM_TILDE        = 0x32, // ~
  KEY_SYM_GRAVE        = 0x32, // `
  KEY_SYM_AT           = KEY_SYM_2, // @
  KEY_SYM_POUND        = KEY_SYM_3, // #
  KEY_SYM_DOLLAR       = KEY_SYM_4, // $
  KEY_SYM_PERCENT      = KEY_SYM_5, // %
  KEY_SYM_CARET        = KEY_SYM_6, // ^
  KEY_SYM_AMPERSAND    = KEY_SYM_7, // &
  KEY_SYM_ASTERISK     = KEY_SYM_8, // *
  KEY_SYM_UNDERSCORE   = 0x1B, // _
  KEY_SYM_HYPHEN       = 0x1B, // -
  KEY_SYM_PLUS         = 0x18, // +
  KEY_SYM_EQUALS       = 0x18, // =

  KEY_SYM_PARENTHESIS_LEFT  = KEY_SYM_9,
  KEY_SYM_PARENTHESIS_RIGHT = KEY_SYM_0,
  KEY_SYM_BRACKET_LEFT      = 0x21, // [
  KEY_SYM_BRACKET_RIGHT     = 0x1E, // ]
  KEY_SYM_BRACE_LEFT        = 0x21, // {
  KEY_SYM_BRACE_RIGHT       = 0x1E, // }
  KEY_SYM_ANGLE_LEFT        = 0x2B, // <
  KEY_SYM_ANGLE_RIGHT       = 0x2F, // >
  KEY_SYM_FORWARD_SLASH     = 0x2C,     /* / */
  KEY_SYM_BACKWARD_SLASH    = 0x2A, /* \ */
  KEY_SYM_VERTICAL_BAR      = 0x0A,       /* | */
  KEY_SYM_SEMICOLON         = 0x29,
  KEY_SYM_COLON             = 0x29,
  KEY_SYM_COMMA             = 0x2B,
  KEY_SYM_PERIOD            = 0x2F,
  KEY_SYM_EXCLAMATION       = KEY_SYM_1,
  KEY_SYM_QUESTION          = 0x2C,

  KEY_SYM_a = 0x00,
  KEY_SYM_b = 0x0B,
  KEY_SYM_c = 0x08,
  KEY_SYM_d = 0x02,
  KEY_SYM_e = 0x0E,
  KEY_SYM_f = 0x03,
  KEY_SYM_g = 0x05,
  KEY_SYM_h = 0x04,
  KEY_SYM_i = 0x22,
  KEY_SYM_j = 0x26,
  KEY_SYM_k = 0x28,
  KEY_SYM_l = 0x25,
  KEY_SYM_m = 0x2E,
  KEY_SYM_n = 0x2D,
  KEY_SYM_o = 0x1F,
  KEY_SYM_p = 0x23,
  KEY_SYM_q = 0x0C,
  KEY_SYM_r = 0x0F,
  KEY_SYM_s = 0x01,
  KEY_SYM_t = 0x11,
  KEY_SYM_u = 0x20,
  KEY_SYM_v = 0x09,
  KEY_SYM_w = 0x0D,
  KEY_SYM_x = 0x07,
  KEY_SYM_y = 0x10,
  KEY_SYM_z = 0x06,

  KEY_SYM_A = KEY_SYM_a,
  KEY_SYM_B = KEY_SYM_b,
  KEY_SYM_C = KEY_SYM_c,
  KEY_SYM_D = KEY_SYM_d,
  KEY_SYM_E = KEY_SYM_e,
  KEY_SYM_F = KEY_SYM_f,
  KEY_SYM_G = KEY_SYM_g,
  KEY_SYM_H = KEY_SYM_h,
  KEY_SYM_I = KEY_SYM_i,
  KEY_SYM_J = KEY_SYM_j,
  KEY_SYM_K = KEY_SYM_k,
  KEY_SYM_L = KEY_SYM_l,
  KEY_SYM_M = KEY_SYM_m,
  KEY_SYM_N = KEY_SYM_n,
  KEY_SYM_O = KEY_SYM_o,
  KEY_SYM_P = KEY_SYM_p,
  KEY_SYM_Q = KEY_SYM_q,
  KEY_SYM_R = KEY_SYM_r,
  KEY_SYM_S = KEY_SYM_s,
  KEY_SYM_T = KEY_SYM_t,
  KEY_SYM_U = KEY_SYM_u,
  KEY_SYM_V = KEY_SYM_v,
  KEY_SYM_W = KEY_SYM_w,
  KEY_SYM_X = KEY_SYM_x,
  KEY_SYM_Y = KEY_SYM_y,
  KEY_SYM_Z = KEY_SYM_z,

  KEY_SYM_UNKNOWN = 0XFF
};



#endif /* SR_KEYSYM_COCOA_HPP */
