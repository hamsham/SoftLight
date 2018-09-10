
#ifndef SR_KEYSYM_XLIB_HPP
#define SR_KEYSYM_XLIB_HPP

#include <cstdint> // uint16_t

#include <X11/keysym.h>



/**------------------------------------
 * @brief Keyboard Symbols
-------------------------------------*/
enum SR_KeySymbol : uint32_t
{
  KEY_SYM_ESCAPE = XK_Escape,

  KEY_SYM_L_SHIFT   = XK_Shift_L,
  KEY_SYM_L_CONTROL = XK_Control_L,
  KEY_SYM_L_ALT     = XK_Alt_L,
  KEY_SYM_L_SUPER   = XK_Super_L,
  KEY_SYM_L_META    = XK_Meta_L,

  KEY_SYM_R_SHIFT   = XK_Shift_R,
  KEY_SYM_R_CONTROL = XK_Control_R,
  KEY_SYM_R_ALT     = XK_Alt_R,
  KEY_SYM_R_SUPER   = XK_Super_R,
  KEY_SYM_R_META    = XK_Meta_R,

  KEY_SYM_F1  = XK_F1,
  KEY_SYM_F2  = XK_F2,
  KEY_SYM_F3  = XK_F3,
  KEY_SYM_F4  = XK_F4,
  KEY_SYM_F5  = XK_F5,
  KEY_SYM_F6  = XK_F6,
  KEY_SYM_F7  = XK_F7,
  KEY_SYM_F8  = XK_F8,
  KEY_SYM_F9  = XK_F9,
  KEY_SYM_F10 = XK_F10,
  KEY_SYM_F11 = XK_F11,
  KEY_SYM_F12 = XK_F12,
  KEY_SYM_F13 = XK_F13,
  KEY_SYM_F14 = XK_F14,
  KEY_SYM_F15 = XK_F15,
  KEY_SYM_F16 = XK_F16,
  KEY_SYM_F17 = XK_F17,
  KEY_SYM_F18 = XK_F18,
  KEY_SYM_F19 = XK_F19,
  KEY_SYM_F20 = XK_F20,
  KEY_SYM_F21 = XK_F21,
  KEY_SYM_F22 = XK_F22,
  KEY_SYM_F23 = XK_F23,
  KEY_SYM_F24 = XK_F24,

  KEY_SYM_1 = XK_1,
  KEY_SYM_2 = XK_2,
  KEY_SYM_3 = XK_3,
  KEY_SYM_4 = XK_4,
  KEY_SYM_5 = XK_5,
  KEY_SYM_6 = XK_6,
  KEY_SYM_7 = XK_7,
  KEY_SYM_8 = XK_8,
  KEY_SYM_9 = XK_9,
  KEY_SYM_0 = XK_0,

  KEY_SYM_NUMPAD_1       = XK_KP_1,
  KEY_SYM_NUMPAD_2       = XK_KP_2,
  KEY_SYM_NUMPAD_3       = XK_KP_3,
  KEY_SYM_NUMPAD_4       = XK_KP_4,
  KEY_SYM_NUMPAD_5       = XK_KP_5,
  KEY_SYM_NUMPAD_6       = XK_KP_6,
  KEY_SYM_NUMPAD_7       = XK_KP_7,
  KEY_SYM_NUMPAD_8       = XK_KP_8,
  KEY_SYM_NUMPAD_9       = XK_KP_9,
  KEY_SYM_NUMPAD_0       = XK_KP_0,
  KEY_SYM_NUMPAD_END     = XK_KP_End,    // Numpad alias: 1
  KEY_SYM_NUMPAD_DOWN    = XK_KP_Down,   // Numpad alias: 2
  KEY_SYM_NUMPAD_PG_DOWN = XK_KP_Next,   // Numpad alias: 3
  KEY_SYM_NUMPAD_LEFT    = XK_KP_Left,   // Numpad alias: 4
  KEY_SYM_NUMPAD_BEGIN   = XK_KP_Begin,  // Numpad alias: 5
  KEY_SYM_NUMPAD_RIGHT   = XK_KP_Right,  // Numpad alias: 6
  KEY_SYM_NUMPAD_HOME    = XK_KP_Home,   // Numpad alias: 7
  KEY_SYM_NUMPAD_UP      = XK_KP_Up,     // Numpad alias: 8
  KEY_SYM_NUMPAD_PG_UP   = XK_KP_Prior,  // Numpad alias: 9
  KEY_SYM_NUMPAD_INSERT  = XK_KP_Insert, // Numpad alias: 0
  KEY_SYM_NUMPAD_ADD     = XK_KP_Add,
  KEY_SYM_NUMPAD_SUB     = XK_KP_Subtract,
  KEY_SYM_NUMPAD_MUL     = XK_KP_Multiply,
  KEY_SYM_NUMPAD_DIV     = XK_KP_Divide,
  KEY_SYM_NUMPAD_EQUAL   = XK_KP_Equal,
  KEY_SYM_NUMPAD_ENTER   = XK_KP_Enter,
  KEY_SYM_NUMPAD_DELETE  = XK_KP_Delete,
  KEY_SYM_NUMPAD_DECIMAL = XK_KP_Decimal,
  KEY_SYM_NUMPAD_SEP     = XK_KP_Separator,

  KEY_SYM_PRINT_SCREEN = XK_Print,
  KEY_SYM_PAUSE        = XK_Pause,
  KEY_SYM_SYS_REQ      = XK_Sys_Req,
  KEY_SYM_INSERT       = XK_Insert,
  KEY_SYM_DELETE       = XK_Delete,
  KEY_SYM_HOME         = XK_Home,
  KEY_SYM_END          = XK_End,
  KEY_SYM_PG_UP        = XK_Page_Up,
  KEY_SYM_PG_DOWN      = XK_Page_Down,
  KEY_SYM_LEFT         = XK_Left,
  KEY_SYM_RIGHT        = XK_Right,
  KEY_SYM_UP           = XK_Up,
  KEY_SYM_DOWN         = XK_Down,
  KEY_SYM_SPACE        = XK_space,
  KEY_SYM_BACKSPACE    = XK_BackSpace,
  KEY_SYM_LINE_FEED    = XK_Linefeed,
  KEY_SYM_RETURN       = XK_Return, // Enter, return
  KEY_SYM_TAB          = XK_Tab,
  KEY_SYM_CLEAR        = XK_Clear,

  KEY_SYM_CAPS_LOCK   = XK_Caps_Lock,
  KEY_SYM_NUM_LOCK    = XK_Num_Lock,
  KEY_SYM_SCROLL_LOCK = XK_Scroll_Lock,

  KEY_SYM_SINGLE_QUOTE = XK_apostrophe, // '
  KEY_SYM_DOUBLE_QUOTE = XK_quotedbl, // "
  KEY_SYM_TILDE        = XK_asciitilde, // ~
  KEY_SYM_GRAVE        = XK_grave, // `
  KEY_SYM_AT           = XK_at, // @
  KEY_SYM_POUND        = XK_numbersign, // #
  KEY_SYM_DOLLAR       = XK_dollar, // $
  KEY_SYM_PERCENT      = XK_percent, // %
  KEY_SYM_CARET        = XK_asciicircum, // ^
  KEY_SYM_AMPERSAND    = XK_ampersand, // &
  KEY_SYM_ASTERISK     = XK_asterisk, // *
  KEY_SYM_UNDERSCORE   = XK_underscore, // _
  KEY_SYM_HYPHEN       = XK_minus, // -
  KEY_SYM_PLUS         = XK_plus, // +
  KEY_SYM_EQUALS       = XK_equal, // =

  KEY_SYM_PARENTHESIS_LEFT  = XK_parenleft,
  KEY_SYM_PARENTHESIS_RIGHT = XK_parenright,
  KEY_SYM_BRACKET_LEFT      = XK_bracketleft, // [
  KEY_SYM_BRACKET_RIGHT     = XK_bracketright, // ]
  KEY_SYM_BRACE_LEFT        = XK_braceleft, // {
  KEY_SYM_BRACE_RIGHT       = XK_braceright, // }
  KEY_SYM_ANGLE_LEFT        = XK_less, // <
  KEY_SYM_ANGLE_RIGHT       = XK_greater, // >
  KEY_SYM_FORWARD_SLASH     = XK_slash,     /* / */
  KEY_SYM_BACKWARD_SLASH    = XK_backslash, /* \ */
  KEY_SYM_VERTICAL_BAR      = XK_bar,       /* | */
  KEY_SYM_SEMICOLON         = XK_semicolon,
  KEY_SYM_COLON             = XK_colon,
  KEY_SYM_COMMA             = XK_comma,
  KEY_SYM_PERIOD            = XK_period,
  KEY_SYM_EXCLAMATION       = XK_exclam,
  KEY_SYM_QUESTION          = XK_question,

  KEY_SYM_a = XK_a,
  KEY_SYM_b = XK_b,
  KEY_SYM_c = XK_c,
  KEY_SYM_d = XK_d,
  KEY_SYM_e = XK_e,
  KEY_SYM_f = XK_f,
  KEY_SYM_g = XK_g,
  KEY_SYM_h = XK_h,
  KEY_SYM_i = XK_i,
  KEY_SYM_j = XK_j,
  KEY_SYM_k = XK_k,
  KEY_SYM_l = XK_l,
  KEY_SYM_m = XK_m,
  KEY_SYM_n = XK_n,
  KEY_SYM_o = XK_o,
  KEY_SYM_p = XK_p,
  KEY_SYM_q = XK_q,
  KEY_SYM_r = XK_r,
  KEY_SYM_s = XK_s,
  KEY_SYM_t = XK_t,
  KEY_SYM_u = XK_u,
  KEY_SYM_v = XK_v,
  KEY_SYM_w = XK_w,
  KEY_SYM_x = XK_x,
  KEY_SYM_y = XK_y,
  KEY_SYM_z = XK_z,

  KEY_SYM_A = XK_A,
  KEY_SYM_B = XK_B,
  KEY_SYM_C = XK_C,
  KEY_SYM_D = XK_D,
  KEY_SYM_E = XK_E,
  KEY_SYM_F = XK_F,
  KEY_SYM_G = XK_G,
  KEY_SYM_H = XK_H,
  KEY_SYM_I = XK_I,
  KEY_SYM_J = XK_J,
  KEY_SYM_K = XK_K,
  KEY_SYM_L = XK_L,
  KEY_SYM_M = XK_M,
  KEY_SYM_N = XK_N,
  KEY_SYM_O = XK_O,
  KEY_SYM_P = XK_P,
  KEY_SYM_Q = XK_Q,
  KEY_SYM_R = XK_R,
  KEY_SYM_S = XK_S,
  KEY_SYM_T = XK_T,
  KEY_SYM_U = XK_U,
  KEY_SYM_V = XK_V,
  KEY_SYM_W = XK_W,
  KEY_SYM_X = XK_X,
  KEY_SYM_Y = XK_Y,
  KEY_SYM_Z = XK_Z,

  KEY_SYM_UNKNOWN = XK_VoidSymbol
};



#endif /* SR_KEYSYM_XLIB_HPP */
