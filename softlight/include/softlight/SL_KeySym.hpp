
#ifndef SL_KEYSYM_HPP
#define SL_KEYSYM_HPP

#include <cstdint> // uint32_t



/*-------------------------------------
 * Forward declarations
-------------------------------------*/
enum class SL_WindowBackend;



/**------------------------------------
 * @brief Keyboard Symbols
-------------------------------------*/
enum SL_KeySymbol : uint16_t
{
  KEY_SYM_ESCAPE,

  KEY_SYM_L_SHIFT,
  KEY_SYM_L_CONTROL,
  KEY_SYM_L_ALT,
  KEY_SYM_L_SUPER,
  KEY_SYM_L_META,

  KEY_SYM_R_SHIFT,
  KEY_SYM_R_CONTROL,
  KEY_SYM_R_ALT,
  KEY_SYM_R_SUPER,
  KEY_SYM_R_META,

  KEY_SYM_F1,
  KEY_SYM_F2,
  KEY_SYM_F3,
  KEY_SYM_F4,
  KEY_SYM_F5,
  KEY_SYM_F6,
  KEY_SYM_F7,
  KEY_SYM_F8,
  KEY_SYM_F9,
  KEY_SYM_F10,
  KEY_SYM_F11,
  KEY_SYM_F12,
  KEY_SYM_F13,
  KEY_SYM_F14,
  KEY_SYM_F15,
  KEY_SYM_F16,
  KEY_SYM_F17,
  KEY_SYM_F18,
  KEY_SYM_F19,
  KEY_SYM_F20,
  KEY_SYM_F21,
  KEY_SYM_F22,
  KEY_SYM_F23,
  KEY_SYM_F24,

  KEY_SYM_1,
  KEY_SYM_2,
  KEY_SYM_3,
  KEY_SYM_4,
  KEY_SYM_5,
  KEY_SYM_6,
  KEY_SYM_7,
  KEY_SYM_8,
  KEY_SYM_9,
  KEY_SYM_0,

  KEY_SYM_NUMPAD_1,
  KEY_SYM_NUMPAD_2,
  KEY_SYM_NUMPAD_3,
  KEY_SYM_NUMPAD_4,
  KEY_SYM_NUMPAD_5,
  KEY_SYM_NUMPAD_6,
  KEY_SYM_NUMPAD_7,
  KEY_SYM_NUMPAD_8,
  KEY_SYM_NUMPAD_9,
  KEY_SYM_NUMPAD_0,
  KEY_SYM_NUMPAD_END,    // Numpad alias: 1
  KEY_SYM_NUMPAD_DOWN,   // Numpad alias: 2
  KEY_SYM_NUMPAD_PG_DOWN,   // Numpad alias: 3
  KEY_SYM_NUMPAD_LEFT,   // Numpad alias: 4
  KEY_SYM_NUMPAD_BEGIN,  // Numpad alias: 5
  KEY_SYM_NUMPAD_RIGHT,  // Numpad alias: 6
  KEY_SYM_NUMPAD_HOME,   // Numpad alias: 7
  KEY_SYM_NUMPAD_UP,     // Numpad alias: 8
  KEY_SYM_NUMPAD_PG_UP,  // Numpad alias: 9
  KEY_SYM_NUMPAD_INSERT, // Numpad alias: 0
  KEY_SYM_NUMPAD_ADD,
  KEY_SYM_NUMPAD_SUB,
  KEY_SYM_NUMPAD_MUL,
  KEY_SYM_NUMPAD_DIV,
  KEY_SYM_NUMPAD_EQUAL,
  KEY_SYM_NUMPAD_ENTER,
  KEY_SYM_NUMPAD_DELETE,
  KEY_SYM_NUMPAD_DECIMAL,
  KEY_SYM_NUMPAD_SEP,

  KEY_SYM_PRINT_SCREEN,
  KEY_SYM_PAUSE,
  KEY_SYM_SYS_REQ,
  KEY_SYM_INSERT,
  KEY_SYM_DELETE,
  KEY_SYM_HOME,
  KEY_SYM_END,
  KEY_SYM_PG_UP,
  KEY_SYM_PG_DOWN,
  KEY_SYM_LEFT,
  KEY_SYM_RIGHT,
  KEY_SYM_UP,
  KEY_SYM_DOWN,
  KEY_SYM_SPACE,
  KEY_SYM_BACKSPACE,
  KEY_SYM_LINE_FEED,
  KEY_SYM_RETURN,
  KEY_SYM_TAB,
  KEY_SYM_CLEAR,

  KEY_SYM_CAPS_LOCK,
  KEY_SYM_NUM_LOCK,
  KEY_SYM_SCROLL_LOCK,

  KEY_SYM_SINGLE_QUOTE, // '
  KEY_SYM_DOUBLE_QUOTE, // "
  KEY_SYM_TILDE, // ~
  KEY_SYM_GRAVE, // `
  KEY_SYM_AT, // @
  KEY_SYM_POUND, // #
  KEY_SYM_DOLLAR, // $
  KEY_SYM_PERCENT, // %
  KEY_SYM_CARET, // ^
  KEY_SYM_AMPERSAND, // &
  KEY_SYM_ASTERISK, // *
  KEY_SYM_UNDERSCORE, // _
  KEY_SYM_HYPHEN, // -
  KEY_SYM_PLUS, // +
  KEY_SYM_EQUALS, // =

  KEY_SYM_PARENTHESIS_LEFT,
  KEY_SYM_PARENTHESIS_RIGHT,
  KEY_SYM_BRACKET_LEFT, // [
  KEY_SYM_BRACKET_RIGHT, // ]
  KEY_SYM_BRACE_LEFT, // {
  KEY_SYM_BRACE_RIGHT, // }
  KEY_SYM_ANGLE_LEFT, // <
  KEY_SYM_ANGLE_RIGHT, // >
  KEY_SYM_FORWARD_SLASH,     /* / */
  KEY_SYM_BACKWARD_SLASH, /* \ */
  KEY_SYM_VERTICAL_BAR,       /* | */
  KEY_SYM_SEMICOLON,
  KEY_SYM_COLON,
  KEY_SYM_COMMA,
  KEY_SYM_PERIOD,
  KEY_SYM_EXCLAMATION,
  KEY_SYM_QUESTION,

  KEY_SYM_a,
  KEY_SYM_b,
  KEY_SYM_c,
  KEY_SYM_d,
  KEY_SYM_e,
  KEY_SYM_f,
  KEY_SYM_g,
  KEY_SYM_h,
  KEY_SYM_i,
  KEY_SYM_j,
  KEY_SYM_k,
  KEY_SYM_l,
  KEY_SYM_m,
  KEY_SYM_n,
  KEY_SYM_o,
  KEY_SYM_p,
  KEY_SYM_q,
  KEY_SYM_r,
  KEY_SYM_s,
  KEY_SYM_t,
  KEY_SYM_u,
  KEY_SYM_v,
  KEY_SYM_w,
  KEY_SYM_x,
  KEY_SYM_y,
  KEY_SYM_z,

  KEY_SYM_A,
  KEY_SYM_B,
  KEY_SYM_C,
  KEY_SYM_D,
  KEY_SYM_E,
  KEY_SYM_F,
  KEY_SYM_G,
  KEY_SYM_H,
  KEY_SYM_I,
  KEY_SYM_J,
  KEY_SYM_K,
  KEY_SYM_L,
  KEY_SYM_M,
  KEY_SYM_N,
  KEY_SYM_O,
  KEY_SYM_P,
  KEY_SYM_Q,
  KEY_SYM_R,
  KEY_SYM_S,
  KEY_SYM_T,
  KEY_SYM_U,
  KEY_SYM_V,
  KEY_SYM_W,
  KEY_SYM_X,
  KEY_SYM_Y,
  KEY_SYM_Z,

  KEY_SYM_UNKNOWN
};



/**
 * @brief Convert a key symbol into a string which can be used for later
 * reference.
 *
 * @param keySym
 * A value from the SL_KeySymbol enumeration.
 *
 * @return A string, containing the string representation of a key symbol in
 * the SL_KeySymbol enumeration, or NULL if one doesn't exist.
 */
const char* sl_key_to_string(const SL_KeySymbol keySym) noexcept;



/**
 * @brief Convert a key symbol into a string which can be used for later
 * reference. This method retrieves the backend-native string representation
 * of a raw key symbol
 *
 * @param keySym
 * A value from the SL_KeySymbol enumeration.
 *
 * @param backend
 * A value from the SL_WindowBackend enumeration.
 *
 * @return A string, containing the string representation of a key symbol in
 * the SL_KeySymbol enumeration, or NULL if one doesn't exist.
 */
const char* sl_key_to_string_native(const uint32_t keySym, const SL_WindowBackend backend) noexcept;



#endif /* SL_KEYSYM_HPP */
