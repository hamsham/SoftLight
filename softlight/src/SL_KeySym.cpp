/*
 * File:   SL_KeySym.cpp
 * Author: hammy
 * Created on February 19, 2024, at 12:06 AM
 */

#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_KeySym.hpp"

#if defined(SL_HAVE_WIN32_BACKEND)
    #include "softlight/SL_KeySymWin32.hpp"
#endif

#if defined(SL_HAVE_COCOA_BACKEND)
    #include "softlight/SL_KeySymCocoa.hpp"
#endif

#if defined(SL_HAVE_XCB_BACKEND) || defined(SL_HAVE_X11_BACKEND)
    #include "softlight/SL_KeySymXlib.hpp"
#endif



/*-------------------------------------
 * Key Symbol to String
-------------------------------------*/
const char* sl_key_to_string(const SL_KeySymbol keySym) noexcept
{
    switch (keySym)
    {
        case KEY_SYM_ESCAPE: return "KEY_SYM_ESCAPE";

        case KEY_SYM_L_SHIFT: return "KEY_SYM_L_SHIFT";
        case KEY_SYM_L_CONTROL: return "KEY_SYM_L_CONTROL";
        case KEY_SYM_L_ALT: return "KEY_SYM_L_ALT";
        case KEY_SYM_L_SUPER: return "KEY_SYM_L_SUPER";
        case KEY_SYM_L_META: return "KEY_SYM_L_META";

        case KEY_SYM_R_SHIFT: return "KEY_SYM_R_SHIFT";
        case KEY_SYM_R_CONTROL: return "KEY_SYM_R_CONTROL";
        case KEY_SYM_R_ALT: return "KEY_SYM_R_ALT";
        case KEY_SYM_R_SUPER: return "KEY_SYM_R_SUPER";
        case KEY_SYM_R_META: return "KEY_SYM_R_META";

        case KEY_SYM_F1: return "KEY_SYM_F1";
        case KEY_SYM_F2: return "KEY_SYM_F2";
        case KEY_SYM_F3: return "KEY_SYM_F3";
        case KEY_SYM_F4: return "KEY_SYM_F4";
        case KEY_SYM_F5: return "KEY_SYM_F5";
        case KEY_SYM_F6: return "KEY_SYM_F6";
        case KEY_SYM_F7: return "KEY_SYM_F7";
        case KEY_SYM_F8: return "KEY_SYM_F8";
        case KEY_SYM_F9: return "KEY_SYM_F9";
        case KEY_SYM_F10: return "KEY_SYM_F10";
        case KEY_SYM_F11: return "KEY_SYM_F11";
        case KEY_SYM_F12: return "KEY_SYM_F12";
        case KEY_SYM_F13: return "KEY_SYM_F13";
        case KEY_SYM_F14: return "KEY_SYM_F14";
        case KEY_SYM_F15: return "KEY_SYM_F15";
        case KEY_SYM_F16: return "KEY_SYM_F16";
        case KEY_SYM_F17: return "KEY_SYM_F17";
        case KEY_SYM_F18: return "KEY_SYM_F18";
        case KEY_SYM_F19: return "KEY_SYM_F19";
        case KEY_SYM_F20: return "KEY_SYM_F20";
        case KEY_SYM_F21: return "KEY_SYM_F21";
        case KEY_SYM_F22: return "KEY_SYM_F22";
        case KEY_SYM_F23: return "KEY_SYM_F23";
        case KEY_SYM_F24: return "KEY_SYM_F24";

        case KEY_SYM_1: return "KEY_SYM_1";
        case KEY_SYM_2: return "KEY_SYM_2";
        case KEY_SYM_3: return "KEY_SYM_3";
        case KEY_SYM_4: return "KEY_SYM_4";
        case KEY_SYM_5: return "KEY_SYM_5";
        case KEY_SYM_6: return "KEY_SYM_6";
        case KEY_SYM_7: return "KEY_SYM_7";
        case KEY_SYM_8: return "KEY_SYM_8";
        case KEY_SYM_9: return "KEY_SYM_9";
        case KEY_SYM_0: return "KEY_SYM_0";

        case KEY_SYM_NUMPAD_1: return "KEY_SYM_NUMPAD_1";
        case KEY_SYM_NUMPAD_2: return "KEY_SYM_NUMPAD_2";
        case KEY_SYM_NUMPAD_3: return "KEY_SYM_NUMPAD_3";
        case KEY_SYM_NUMPAD_4: return "KEY_SYM_NUMPAD_4";
        case KEY_SYM_NUMPAD_5: return "KEY_SYM_NUMPAD_5";
        case KEY_SYM_NUMPAD_6: return "KEY_SYM_NUMPAD_6";
        case KEY_SYM_NUMPAD_7: return "KEY_SYM_NUMPAD_7";
        case KEY_SYM_NUMPAD_8: return "KEY_SYM_NUMPAD_8";
        case KEY_SYM_NUMPAD_9: return "KEY_SYM_NUMPAD_9";
        case KEY_SYM_NUMPAD_0: return "KEY_SYM_NUMPAD_0";
        case KEY_SYM_NUMPAD_END: return "KEY_SYM_NUMPAD_END";
        case KEY_SYM_NUMPAD_DOWN: return "KEY_SYM_NUMPAD_DOWN";
        case KEY_SYM_NUMPAD_PG_DOWN: return "KEY_SYM_NUMPAD_PG_DOWN";
        case KEY_SYM_NUMPAD_LEFT: return "KEY_SYM_NUMPAD_LEFT";
        case KEY_SYM_NUMPAD_BEGIN: return "KEY_SYM_NUMPAD_BEGIN";
        case KEY_SYM_NUMPAD_RIGHT: return "KEY_SYM_NUMPAD_RIGHT";
        case KEY_SYM_NUMPAD_HOME: return "KEY_SYM_NUMPAD_HOME";
        case KEY_SYM_NUMPAD_UP: return "KEY_SYM_NUMPAD_UP";
        case KEY_SYM_NUMPAD_PG_UP: return "KEY_SYM_NUMPAD_PG_UP";
        case KEY_SYM_NUMPAD_INSERT: return "KEY_SYM_NUMPAD_INSERT";
        case KEY_SYM_NUMPAD_ADD: return "KEY_SYM_NUMPAD_ADD";
        case KEY_SYM_NUMPAD_SUB: return "KEY_SYM_NUMPAD_SUB";
        case KEY_SYM_NUMPAD_MUL: return "KEY_SYM_NUMPAD_MUL";
        case KEY_SYM_NUMPAD_DIV: return "KEY_SYM_NUMPAD_DIV";
        case KEY_SYM_NUMPAD_EQUAL: return "KEY_SYM_NUMPAD_EQUAL";
        case KEY_SYM_NUMPAD_ENTER: return "KEY_SYM_NUMPAD_ENTER";
        case KEY_SYM_NUMPAD_DELETE: return "KEY_SYM_NUMPAD_DELETE";
        case KEY_SYM_NUMPAD_DECIMAL: return "KEY_SYM_NUMPAD_DECIMAL";
        case KEY_SYM_NUMPAD_SEP: return "KEY_SYM_NUMPAD_SEP";

        case KEY_SYM_PRINT_SCREEN: return "KEY_SYM_PRINT_SCREEN";
        case KEY_SYM_PAUSE: return "KEY_SYM_PAUSE";
        case KEY_SYM_SYS_REQ: return "KEY_SYM_SYS_REQ";
        case KEY_SYM_INSERT: return "KEY_SYM_INSERT";
        case KEY_SYM_DELETE: return "KEY_SYM_DELETE";
        case KEY_SYM_HOME: return "KEY_SYM_HOME";
        case KEY_SYM_END: return "KEY_SYM_END";
        case KEY_SYM_PG_UP: return "KEY_SYM_PG_UP";
        case KEY_SYM_PG_DOWN: return "KEY_SYM_PG_DOWN";
        case KEY_SYM_LEFT: return "KEY_SYM_LEFT";
        case KEY_SYM_RIGHT: return "KEY_SYM_RIGHT";
        case KEY_SYM_UP: return "KEY_SYM_UP";
        case KEY_SYM_DOWN: return "KEY_SYM_DOWN";
        case KEY_SYM_SPACE: return "KEY_SYM_SPACE";
        case KEY_SYM_BACKSPACE: return "KEY_SYM_BACKSPACE";
        case KEY_SYM_LINE_FEED: return "KEY_SYM_LINE_FEED";
        case KEY_SYM_RETURN: return "KEY_SYM_RETURN";
        case KEY_SYM_TAB: return "KEY_SYM_TAB";
        case KEY_SYM_CLEAR: return "KEY_SYM_CLEAR";

        case KEY_SYM_CAPS_LOCK: return "KEY_SYM_CAPS_LOCK";
        case KEY_SYM_NUM_LOCK: return "KEY_SYM_NUM_LOCK";
        case KEY_SYM_SCROLL_LOCK: return "KEY_SYM_SCROLL_LOCK";

        case KEY_SYM_SINGLE_QUOTE: return "KEY_SYM_SINGLE_QUOTE"; // '
        case KEY_SYM_DOUBLE_QUOTE: return "KEY_SYM_DOUBLE_QUOTE"; // "
        case KEY_SYM_TILDE: return "KEY_SYM_TILDE"; // ~
        case KEY_SYM_GRAVE: return "KEY_SYM_GRAVE"; // `
        case KEY_SYM_AT: return "KEY_SYM_AT"; // @
        case KEY_SYM_POUND: return "KEY_SYM_POUND"; // #
        case KEY_SYM_DOLLAR: return "KEY_SYM_DOLLAR"; // $
        case KEY_SYM_PERCENT: return "KEY_SYM_PERCENT"; // %
        case KEY_SYM_CARET: return "KEY_SYM_CARET"; // ^
        case KEY_SYM_AMPERSAND: return "KEY_SYM_AMPERSAND"; // &
        case KEY_SYM_ASTERISK: return "KEY_SYM_ASTERISK"; // *
        case KEY_SYM_UNDERSCORE: return "KEY_SYM_UNDERSCORE"; // _
        case KEY_SYM_HYPHEN: return "KEY_SYM_HYPHEN"; // -
        case KEY_SYM_PLUS: return "KEY_SYM_PLUS"; // +
        case KEY_SYM_EQUALS: return "KEY_SYM_EQUALS"; // =

        case KEY_SYM_PARENTHESIS_LEFT: return "KEY_SYM_PARENTHESIS_LEFT";
        case KEY_SYM_PARENTHESIS_RIGHT: return "KEY_SYM_PARENTHESIS_RIGHT";
        case KEY_SYM_BRACKET_LEFT: return "KEY_SYM_BRACKET_LEFT"; // [
        case KEY_SYM_BRACKET_RIGHT: return "KEY_SYM_BRACKET_RIGHT"; // ]
        case KEY_SYM_BRACE_LEFT: return "KEY_SYM_BRACE_LEFT"; // {
        case KEY_SYM_BRACE_RIGHT: return "KEY_SYM_BRACE_RIGHT"; // }
        case KEY_SYM_ANGLE_LEFT: return "KEY_SYM_ANGLE_LEFT"; // <
        case KEY_SYM_ANGLE_RIGHT: return "KEY_SYM_ANGLE_RIGHT"; // >
        case KEY_SYM_FORWARD_SLASH: return "KEY_SYM_FORWARD_SLASH";     /* / */
        case KEY_SYM_BACKWARD_SLASH: return "KEY_SYM_BACKWARD_SLASH"; /* \ */
        case KEY_SYM_VERTICAL_BAR: return "KEY_SYM_VERTICAL_BAR";       /* | */
        case KEY_SYM_SEMICOLON: return "KEY_SYM_SEMICOLON";
        case KEY_SYM_COLON: return "KEY_SYM_COLON";
        case KEY_SYM_COMMA: return "KEY_SYM_COMMA";
        case KEY_SYM_PERIOD: return "KEY_SYM_PERIOD";
        case KEY_SYM_EXCLAMATION: return "KEY_SYM_EXCLAMATION";
        case KEY_SYM_QUESTION: return "KEY_SYM_QUESTION";

        case KEY_SYM_a: return "KEY_SYM_a";
        case KEY_SYM_b: return "KEY_SYM_b";
        case KEY_SYM_c: return "KEY_SYM_c";
        case KEY_SYM_d: return "KEY_SYM_d";
        case KEY_SYM_e: return "KEY_SYM_e";
        case KEY_SYM_f: return "KEY_SYM_f";
        case KEY_SYM_g: return "KEY_SYM_g";
        case KEY_SYM_h: return "KEY_SYM_h";
        case KEY_SYM_i: return "KEY_SYM_i";
        case KEY_SYM_j: return "KEY_SYM_j";
        case KEY_SYM_k: return "KEY_SYM_k";
        case KEY_SYM_l: return "KEY_SYM_l";
        case KEY_SYM_m: return "KEY_SYM_m";
        case KEY_SYM_n: return "KEY_SYM_n";
        case KEY_SYM_o: return "KEY_SYM_o";
        case KEY_SYM_p: return "KEY_SYM_p";
        case KEY_SYM_q: return "KEY_SYM_q";
        case KEY_SYM_r: return "KEY_SYM_r";
        case KEY_SYM_s: return "KEY_SYM_s";
        case KEY_SYM_t: return "KEY_SYM_t";
        case KEY_SYM_u: return "KEY_SYM_u";
        case KEY_SYM_v: return "KEY_SYM_v";
        case KEY_SYM_w: return "KEY_SYM_w";
        case KEY_SYM_x: return "KEY_SYM_x";
        case KEY_SYM_y: return "KEY_SYM_y";
        case KEY_SYM_z: return "KEY_SYM_z";

        case KEY_SYM_A: return "KEY_SYM_A";
        case KEY_SYM_B: return "KEY_SYM_B";
        case KEY_SYM_C: return "KEY_SYM_C";
        case KEY_SYM_D: return "KEY_SYM_D";
        case KEY_SYM_E: return "KEY_SYM_E";
        case KEY_SYM_F: return "KEY_SYM_F";
        case KEY_SYM_G: return "KEY_SYM_G";
        case KEY_SYM_H: return "KEY_SYM_H";
        case KEY_SYM_I: return "KEY_SYM_I";
        case KEY_SYM_J: return "KEY_SYM_J";
        case KEY_SYM_K: return "KEY_SYM_K";
        case KEY_SYM_L: return "KEY_SYM_L";
        case KEY_SYM_M: return "KEY_SYM_M";
        case KEY_SYM_N: return "KEY_SYM_N";
        case KEY_SYM_O: return "KEY_SYM_O";
        case KEY_SYM_P: return "KEY_SYM_P";
        case KEY_SYM_Q: return "KEY_SYM_Q";
        case KEY_SYM_R: return "KEY_SYM_R";
        case KEY_SYM_S: return "KEY_SYM_S";
        case KEY_SYM_T: return "KEY_SYM_T";
        case KEY_SYM_U: return "KEY_SYM_U";
        case KEY_SYM_V: return "KEY_SYM_V";
        case KEY_SYM_W: return "KEY_SYM_W";
        case KEY_SYM_X: return "KEY_SYM_X";
        case KEY_SYM_Y: return "KEY_SYM_Y";
        case KEY_SYM_Z: return "KEY_SYM_Z";

        default:
            break;
    }

    return "KEY_SYM_UNKNOWN";
}



/*-------------------------------------
 * Native key sym to platform
-------------------------------------*/
const char* sl_key_to_string_native(const uint32_t keySym, const SL_WindowBackend backend) noexcept
{
    (void)keySym;

    switch (backend)
    {
        case SL_WindowBackend::WIN32_BACKEND:
            #if defined(SL_HAVE_WIN32_BACKEND)
                return sl_key_to_string_win32(keySym);
            #else
                break;
            #endif

        case SL_WindowBackend::COCOA_BACKEND:
            #if defined(SL_HAVE_COCOA_BACKEND)
                return sl_key_to_string_cocoa(keySym);
            #else
                break;
            #endif

        case SL_WindowBackend::XCB_BACKEND:
        case SL_WindowBackend::X11_BACKEND:
            #if defined(SL_HAVE_Xcb_BACKEND) || defined(SL_HAVE_X11_BACKEND)
                return sl_key_to_string_xkb(keySym);
            #else
                break;
            #endif

        default:
            break;
    }

    return "KEY_SYM_UNKNOWN";
}
