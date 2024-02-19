
#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_KeySymXlib.hpp"



/*-------------------------------------
 * Keycode to string
-------------------------------------*/
const char* sl_key_to_string_xkb(const SL_KeySymbol keySym) noexcept
{
  return XKeysymToString(keySym);
}



/*-------------------------------------
 * Keycode to SL_KeySym
-------------------------------------*/
SL_KeySymbol sl_keycode_to_keysym_xkb(const uint32_t keycode) noexcept
{
    switch (keycode)
    {
        case XK_Escape: return KEY_SYM_ESCAPE;

        case XK_Shift_L: return KEY_SYM_L_SHIFT;
        case XK_Control_L: return KEY_SYM_L_CONTROL;
        case XK_Alt_L: return KEY_SYM_L_ALT;
        case XK_Super_L: return KEY_SYM_L_SUPER;
        case XK_Meta_L: return KEY_SYM_L_META;

        case XK_Shift_R: return KEY_SYM_R_SHIFT;
        case XK_Control_R: return KEY_SYM_R_CONTROL;
        case XK_Alt_R: return KEY_SYM_R_ALT;
        case XK_Super_R: return KEY_SYM_R_SUPER;
        case XK_Meta_R: return KEY_SYM_R_META;

        case XK_F1: return KEY_SYM_F1;
        case XK_F2: return KEY_SYM_F2;
        case XK_F3: return KEY_SYM_F3;
        case XK_F4: return KEY_SYM_F4;
        case XK_F5: return KEY_SYM_F5;
        case XK_F6: return KEY_SYM_F6;
        case XK_F7: return KEY_SYM_F7;
        case XK_F8: return KEY_SYM_F8;
        case XK_F9: return KEY_SYM_F9;
        case XK_F10: return KEY_SYM_F10;
        case XK_F11: return KEY_SYM_F11;
        case XK_F12: return KEY_SYM_F12;
        case XK_F13: return KEY_SYM_F13;
        case XK_F14: return KEY_SYM_F14;
        case XK_F15: return KEY_SYM_F15;
        case XK_F16: return KEY_SYM_F16;
        case XK_F17: return KEY_SYM_F17;
        case XK_F18: return KEY_SYM_F18;
        case XK_F19: return KEY_SYM_F19;
        case XK_F20: return KEY_SYM_F20;
        case XK_F21: return KEY_SYM_F21;
        case XK_F22: return KEY_SYM_F22;
        case XK_F23: return KEY_SYM_F23;
        case XK_F24: return KEY_SYM_F24;

        case XK_1: return KEY_SYM_1;
        case XK_2: return KEY_SYM_2;
        case XK_3: return KEY_SYM_3;
        case XK_4: return KEY_SYM_4;
        case XK_5: return KEY_SYM_5;
        case XK_6: return KEY_SYM_6;
        case XK_7: return KEY_SYM_7;
        case XK_8: return KEY_SYM_8;
        case XK_9: return KEY_SYM_9;
        case XK_0: return KEY_SYM_0;

        case XK_KP_1: return KEY_SYM_NUMPAD_1;
        case XK_KP_2: return KEY_SYM_NUMPAD_2;
        case XK_KP_3: return KEY_SYM_NUMPAD_3;
        case XK_KP_4: return KEY_SYM_NUMPAD_4;
        case XK_KP_5: return KEY_SYM_NUMPAD_5;
        case XK_KP_6: return KEY_SYM_NUMPAD_6;
        case XK_KP_7: return KEY_SYM_NUMPAD_7;
        case XK_KP_8: return KEY_SYM_NUMPAD_8;
        case XK_KP_9: return KEY_SYM_NUMPAD_9;
        case XK_KP_0: return KEY_SYM_NUMPAD_0;
        case XK_KP_End: return KEY_SYM_NUMPAD_END;
        case XK_KP_Down: return KEY_SYM_NUMPAD_DOWN;
        case XK_KP_Next: return KEY_SYM_NUMPAD_PG_DOWN;
        case XK_KP_Left: return KEY_SYM_NUMPAD_LEFT;
        case XK_KP_Begin: return KEY_SYM_NUMPAD_BEGIN;
        case XK_KP_Right: return KEY_SYM_NUMPAD_RIGHT;
        case XK_KP_Home: return KEY_SYM_NUMPAD_HOME;
        case XK_KP_Up: return KEY_SYM_NUMPAD_UP;
        case XK_KP_Prior: return KEY_SYM_NUMPAD_PG_UP;
        case XK_KP_Insert: return KEY_SYM_NUMPAD_INSERT;
        case XK_KP_Add: return KEY_SYM_NUMPAD_ADD;
        case XK_KP_Subtract: return KEY_SYM_NUMPAD_SUB;
        case XK_KP_Multiply: return KEY_SYM_NUMPAD_MUL;
        case XK_KP_Divide: return KEY_SYM_NUMPAD_DIV;
        case XK_KP_Equal: return KEY_SYM_NUMPAD_EQUAL;
        case XK_KP_Enter: return KEY_SYM_NUMPAD_ENTER;
        case XK_KP_Delete: return KEY_SYM_NUMPAD_DELETE;
        case XK_KP_Decimal: return KEY_SYM_NUMPAD_DECIMAL;
        case XK_KP_Separator: return KEY_SYM_NUMPAD_SEP;

        case XK_Print: return KEY_SYM_PRINT_SCREEN;
        case XK_Pause: return KEY_SYM_PAUSE;
        case XK_Sys_Req: return KEY_SYM_SYS_REQ;
        case XK_Insert: return KEY_SYM_INSERT;
        case XK_Delete: return KEY_SYM_DELETE;
        case XK_Home: return KEY_SYM_HOME;
        case XK_End: return KEY_SYM_END;
        case XK_Page_Up: return KEY_SYM_PG_UP;
        case XK_Page_Down: return KEY_SYM_PG_DOWN;
        case XK_Left: return KEY_SYM_LEFT;
        case XK_Right: return KEY_SYM_RIGHT;
        case XK_Up: return KEY_SYM_UP;
        case XK_Down: return KEY_SYM_DOWN;
        case XK_space: return KEY_SYM_SPACE;
        case XK_BackSpace: return KEY_SYM_BACKSPACE;
        case XK_Linefeed: return KEY_SYM_LINE_FEED;
        case XK_Return: return KEY_SYM_RETURN; // Enter: return KEY_SYM_RETURN; return
        case XK_Tab: return KEY_SYM_TAB;
        case XK_Clear: return KEY_SYM_CLEAR;

        case XK_Caps_Lock: return KEY_SYM_CAPS_LOCK;
        case XK_Num_Lock: return KEY_SYM_NUM_LOCK;
        case XK_Scroll_Lock: return KEY_SYM_SCROLL_LOCK;

        case XK_apostrophe: return KEY_SYM_SINGLE_QUOTE; // '
        case XK_quotedbl: return KEY_SYM_DOUBLE_QUOTE; // "
        case XK_asciitilde: return KEY_SYM_TILDE; // ~
        case XK_grave: return KEY_SYM_GRAVE; // `
        case XK_at: return KEY_SYM_AT; // @
        case XK_numbersign: return KEY_SYM_POUND; // #
        case XK_dollar: return KEY_SYM_DOLLAR; // $
        case XK_percent: return KEY_SYM_PERCENT; // %
        case XK_asciicircum: return KEY_SYM_CARET; // ^
        case XK_ampersand: return KEY_SYM_AMPERSAND; // &
        case XK_asterisk: return KEY_SYM_ASTERISK; // *
        case XK_underscore: return KEY_SYM_UNDERSCORE; // _
        case XK_minus: return KEY_SYM_HYPHEN; // -
        case XK_plus: return KEY_SYM_PLUS; // +
        case XK_equal: return KEY_SYM_EQUALS; // =

        case XK_parenleft: return KEY_SYM_PARENTHESIS_LEFT;
        case XK_parenright: return KEY_SYM_PARENTHESIS_RIGHT;
        case XK_bracketleft: return KEY_SYM_BRACKET_LEFT; // [
        case XK_bracketright: return KEY_SYM_BRACKET_RIGHT; // ]
        case XK_braceleft: return KEY_SYM_BRACE_LEFT; // {
        case XK_braceright: return KEY_SYM_BRACE_RIGHT; // }
        case XK_less: return KEY_SYM_ANGLE_LEFT; // <
        case XK_greater: return KEY_SYM_ANGLE_RIGHT; // >
        case XK_slash: return KEY_SYM_FORWARD_SLASH;     /* / */
        case XK_backslash: return KEY_SYM_BACKWARD_SLASH; /* \ */
        case XK_bar: return KEY_SYM_VERTICAL_BAR;       /* | */
        case XK_semicolon: return KEY_SYM_SEMICOLON;
        case XK_colon: return KEY_SYM_COLON;
        case XK_comma: return KEY_SYM_COMMA;
        case XK_period: return KEY_SYM_PERIOD;
        case XK_exclam: return KEY_SYM_EXCLAMATION;
        case XK_question: return KEY_SYM_QUESTION;

        case XK_a: return KEY_SYM_a;
        case XK_b: return KEY_SYM_b;
        case XK_c: return KEY_SYM_c;
        case XK_d: return KEY_SYM_d;
        case XK_e: return KEY_SYM_e;
        case XK_f: return KEY_SYM_f;
        case XK_g: return KEY_SYM_g;
        case XK_h: return KEY_SYM_h;
        case XK_i: return KEY_SYM_i;
        case XK_j: return KEY_SYM_j;
        case XK_k: return KEY_SYM_k;
        case XK_l: return KEY_SYM_l;
        case XK_m: return KEY_SYM_m;
        case XK_n: return KEY_SYM_n;
        case XK_o: return KEY_SYM_o;
        case XK_p: return KEY_SYM_p;
        case XK_q: return KEY_SYM_q;
        case XK_r: return KEY_SYM_r;
        case XK_s: return KEY_SYM_s;
        case XK_t: return KEY_SYM_t;
        case XK_u: return KEY_SYM_u;
        case XK_v: return KEY_SYM_v;
        case XK_w: return KEY_SYM_w;
        case XK_x: return KEY_SYM_x;
        case XK_y: return KEY_SYM_y;
        case XK_z: return KEY_SYM_z;

        case XK_A: return KEY_SYM_A;
        case XK_B: return KEY_SYM_B;
        case XK_C: return KEY_SYM_C;
        case XK_D: return KEY_SYM_D;
        case XK_E: return KEY_SYM_E;
        case XK_F: return KEY_SYM_F;
        case XK_G: return KEY_SYM_G;
        case XK_H: return KEY_SYM_H;
        case XK_I: return KEY_SYM_I;
        case XK_J: return KEY_SYM_J;
        case XK_K: return KEY_SYM_K;
        case XK_L: return KEY_SYM_L;
        case XK_M: return KEY_SYM_M;
        case XK_N: return KEY_SYM_N;
        case XK_O: return KEY_SYM_O;
        case XK_P: return KEY_SYM_P;
        case XK_Q: return KEY_SYM_Q;
        case XK_R: return KEY_SYM_R;
        case XK_S: return KEY_SYM_S;
        case XK_T: return KEY_SYM_T;
        case XK_U: return KEY_SYM_U;
        case XK_V: return KEY_SYM_V;
        case XK_W: return KEY_SYM_W;
        case XK_X: return KEY_SYM_X;
        case XK_Y: return KEY_SYM_Y;
        case XK_Z: return KEY_SYM_Z;

        default:
            break;
    }

    return KEY_SYM_UNKNOWN;
}
