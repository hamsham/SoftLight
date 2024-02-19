
// Thanks again Visual Studio
#ifndef NOMINMAX
    #define NOMINMAX
#endif /* NOMINMAX */

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_KeySymWin32.hpp"



/*-------------------------------------
 * Keycode to string
-------------------------------------*/
const char* sl_key_to_string_win32(const uint32_t keySym) noexcept
{
    unsigned int scanCode = MapVirtualKey(keySym, MAPVK_VK_TO_VSC);

    // because MapVirtualKey strips the extended bit for some keys
    switch (keySym)
    {
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        case VK_PRIOR: // page up
        case VK_NEXT: // page down
        case VK_END:
        case VK_HOME:
        case VK_INSERT:
        case VK_DELETE:
        case VK_DIVIDE: // numpad slash
        case VK_NUMLOCK:
            scanCode |= 0x100; // set extended bit
        default:
            break;
    }

    static thread_local char keyName[64];
    if (GetKeyNameText((scanCode << 16), keyName, sizeof(keyName)) != 0)
    {
        return &keyName[0];
    }

    return "";
}



/*-------------------------------------
 * Keycode to SL_KeySym
-------------------------------------*/
SL_KeySymbol sl_keycode_to_keysym_win32(const uint32_t keycode) noexcept
{
    switch (keycode)
    {
        case VK_ESCAPE: return KEY_SYM_ESCAPE;

        case VK_LSHIFT: return KEY_SYM_L_SHIFT;
        case VK_LCONTROL: return KEY_SYM_L_CONTROL;
        case VK_LMENU: return KEY_SYM_L_ALT;
        case VK_LWIN: return KEY_SYM_L_SUPER;
        case VK_APPS: return KEY_SYM_L_META;

        case VK_RSHIFT: return KEY_SYM_R_SHIFT;
        case VK_RCONTROL: return KEY_SYM_R_CONTROL;
        case VK_MENU: return KEY_SYM_R_ALT;
        case VK_RWIN: return KEY_SYM_R_SUPER;
        //case VK_APPS: return KEY_SYM_R_META;

        case VK_F1: return KEY_SYM_F1;
        case VK_F2: return KEY_SYM_F2;
        case VK_F3: return KEY_SYM_F3;
        case VK_F4: return KEY_SYM_F4;
        case VK_F5: return KEY_SYM_F5;
        case VK_F6: return KEY_SYM_F6;
        case VK_F7: return KEY_SYM_F7;
        case VK_F8: return KEY_SYM_F8;
        case VK_F9: return KEY_SYM_F9;
        case VK_F10: return KEY_SYM_F10;
        case VK_F11: return KEY_SYM_F11;
        case VK_F12: return KEY_SYM_F12;
        case VK_F13: return KEY_SYM_F13;
        case VK_F14: return KEY_SYM_F14;
        case VK_F15: return KEY_SYM_F15;
        case VK_F16: return KEY_SYM_F16;
        case VK_F17: return KEY_SYM_F17;
        case VK_F18: return KEY_SYM_F18;
        case VK_F19: return KEY_SYM_F19;
        case VK_F20: return KEY_SYM_F20;
        case VK_F21: return KEY_SYM_F21;
        case VK_F22: return KEY_SYM_F22;
        case VK_F23: return KEY_SYM_F23;
        case VK_F24: return KEY_SYM_F24;

        case 0x31: return KEY_SYM_1;
        case 0x32: return KEY_SYM_2;
        case 0x33: return KEY_SYM_3;
        case 0x34: return KEY_SYM_4;
        case 0x35: return KEY_SYM_5;
        case 0x36: return KEY_SYM_6;
        case 0x37: return KEY_SYM_7;
        case 0x38: return KEY_SYM_8;
        case 0x39: return KEY_SYM_9;
        case 0x30: return KEY_SYM_0;

        case VK_NUMPAD1: return KEY_SYM_NUMPAD_1;
        case VK_NUMPAD2: return KEY_SYM_NUMPAD_2;
        case VK_NUMPAD3: return KEY_SYM_NUMPAD_3;
        case VK_NUMPAD4: return KEY_SYM_NUMPAD_4;
        case VK_NUMPAD5: return KEY_SYM_NUMPAD_5;
        case VK_NUMPAD6: return KEY_SYM_NUMPAD_6;
        case VK_NUMPAD7: return KEY_SYM_NUMPAD_7;
        case VK_NUMPAD8: return KEY_SYM_NUMPAD_8;
        case VK_NUMPAD9: return KEY_SYM_NUMPAD_9;
        case VK_NUMPAD0: return KEY_SYM_NUMPAD_0;
        //case VK_END: return KEY_SYM_NUMPAD_END;     // Numpad alias: 1
        //case VK_DOWN: return KEY_SYM_NUMPAD_DOWN;    // Numpad alias: 2
        //case VK_NEXT: return KEY_SYM_NUMPAD_PG_DOWN;    // Numpad alias: 3
        //case VK_LEFT: return KEY_SYM_NUMPAD_LEFT;    // Numpad alias: 4
        //case VK_NUMPAD5: return KEY_SYM_NUMPAD_BEGIN; // Numpad alias: 5
        //case VK_RIGHT: return KEY_SYM_NUMPAD_RIGHT;   // Numpad alias: 6
        //case VK_HOME: return KEY_SYM_NUMPAD_HOME;    // Numpad alias: 7
        //case VK_UP: return KEY_SYM_NUMPAD_UP;      // Numpad alias: 8
        //case VK_PRIOR: return KEY_SYM_NUMPAD_PG_UP;   // Numpad alias: 9
        //case VK_INSERT: return KEY_SYM_NUMPAD_INSERT;  // Numpad alias: 0
        case VK_ADD: return KEY_SYM_NUMPAD_ADD;
        case VK_SUBTRACT: return KEY_SYM_NUMPAD_SUB;
        case VK_MULTIPLY: return KEY_SYM_NUMPAD_MUL;
        case VK_DIVIDE: return KEY_SYM_NUMPAD_DIV;
        //case VK_OEM_PLUS: return KEY_SYM_NUMPAD_EQUAL;
        //case VK_RETURN: return KEY_SYM_NUMPAD_ENTER;
        //case VK_DELETE: return KEY_SYM_NUMPAD_DELETE;
        case VK_DECIMAL: return KEY_SYM_NUMPAD_DECIMAL;
        //case VK_OEM_5: return KEY_SYM_NUMPAD_SEP;

        case VK_SNAPSHOT: return KEY_SYM_PRINT_SCREEN;
        case VK_PAUSE: return KEY_SYM_PAUSE;
        case VK_EXECUTE: return KEY_SYM_SYS_REQ;
        case VK_INSERT: return KEY_SYM_INSERT;
        case VK_DELETE: return KEY_SYM_DELETE;
        case VK_HOME: return KEY_SYM_HOME;
        case VK_END: return KEY_SYM_END;
        case VK_PRIOR: return KEY_SYM_PG_UP;
        case VK_NEXT: return KEY_SYM_PG_DOWN;
        case VK_LEFT: return KEY_SYM_LEFT;
        case VK_RIGHT: return KEY_SYM_RIGHT;
        case VK_UP: return KEY_SYM_UP;
        case VK_DOWN: return KEY_SYM_DOWN;
        case VK_SPACE: return KEY_SYM_SPACE;
        case VK_BACK: return KEY_SYM_BACKSPACE;
        //case VK_RETURN: return KEY_SYM_LINE_FEED;
        case VK_RETURN: return KEY_SYM_RETURN; // Enter
        case VK_TAB: return KEY_SYM_TAB;
        case VK_CLEAR: return KEY_SYM_CLEAR;

        case VK_CAPITAL: return KEY_SYM_CAPS_LOCK;
        case VK_NUMLOCK: return KEY_SYM_NUM_LOCK;
        case VK_SCROLL: return KEY_SYM_SCROLL_LOCK;

        case VK_OEM_7: return KEY_SYM_SINGLE_QUOTE; // '
        //case VK_OEM_7: return KEY_SYM_DOUBLE_QUOTE; // "
        case VK_OEM_3: return KEY_SYM_GRAVE; // `
        //case VK_OEM_3: return KEY_SYM_TILDE; // ~
        //case 0x32: return KEY_SYM_AT; // @
        //case 0x33: return KEY_SYM_POUND; // #
        //case 0x34: return KEY_SYM_DOLLAR; // $
        //case 0x35: return KEY_SYM_PERCENT; // %
        //case 0x36: return KEY_SYM_CARET; // ^
        //case 0x37: return KEY_SYM_AMPERSAND; // &
        //case 0x38: return KEY_SYM_ASTERISK; // *
        case VK_OEM_MINUS: return KEY_SYM_HYPHEN; // -
        //case VK_OEM_MINUS: return KEY_SYM_UNDERSCORE; // _
        case VK_OEM_PLUS: return KEY_SYM_PLUS; // +
        //case VK_OEM_PLUS: return KEY_SYM_EQUALS; // =

        //case 0x39: return KEY_SYM_PARENTHESIS_LEFT;
        //case 0x39: return KEY_SYM_PARENTHESIS_RIGHT;
        case VK_OEM_4: return KEY_SYM_BRACKET_LEFT; // [
        case VK_OEM_6: return KEY_SYM_BRACKET_RIGHT; // ]
        //case VK_OEM_4: return KEY_SYM_BRACE_LEFT; // {
        //case VK_OEM_6: return KEY_SYM_BRACE_RIGHT; // }
        //case VK_OEM_COMMA: return KEY_SYM_ANGLE_LEFT; // <
        //case VK_OEM_PERIOD: return KEY_SYM_ANGLE_RIGHT; // >
        case VK_OEM_2: return KEY_SYM_FORWARD_SLASH;     /* / */
        case VK_OEM_5: return KEY_SYM_BACKWARD_SLASH; /* \ */
        //case VK_OEM_5: return KEY_SYM_VERTICAL_BAR;       /* | */
        case VK_OEM_1: return KEY_SYM_SEMICOLON;
        //case VK_OEM_1: return KEY_SYM_COLON;
        case VK_OEM_COMMA: return KEY_SYM_COMMA;
        case VK_OEM_PERIOD: return KEY_SYM_PERIOD;
        case VK_OEM_8: return KEY_SYM_EXCLAMATION;
        //case VK_OEM_2: return KEY_SYM_QUESTION;

        //case 0x41: return KEY_SYM_A;
        //case 0x42: return KEY_SYM_B;
        //case 0x43: return KEY_SYM_C;
        //case 0x44: return KEY_SYM_D;
        //case 0x45: return KEY_SYM_E;
        //case 0x46: return KEY_SYM_F;
        //case 0x47: return KEY_SYM_G;
        //case 0x48: return KEY_SYM_H;
        //case 0x49: return KEY_SYM_I;
        //case 0x4A: return KEY_SYM_J;
        //case 0x4B: return KEY_SYM_K;
        //case 0x4C: return KEY_SYM_L;
        //case 0x4D: return KEY_SYM_M;
        //case 0x4E: return KEY_SYM_N;
        //case 0x4F: return KEY_SYM_O;
        //case 0x50: return KEY_SYM_P;
        //case 0x51: return KEY_SYM_Q;
        //case 0x52: return KEY_SYM_R;
        //case 0x53: return KEY_SYM_S;
        //case 0x54: return KEY_SYM_T;
        //case 0x55: return KEY_SYM_U;
        //case 0x56: return KEY_SYM_V;
        //case 0x57: return KEY_SYM_W;
        //case 0x58: return KEY_SYM_X;
        //case 0x59: return KEY_SYM_Y;
        //case 0x5A: return KEY_SYM_Z;

        case 0x41: return KEY_SYM_a;
        case 0x42: return KEY_SYM_b;
        case 0x43: return KEY_SYM_c;
        case 0x44: return KEY_SYM_d;
        case 0x45: return KEY_SYM_e;
        case 0x46: return KEY_SYM_f;
        case 0x47: return KEY_SYM_g;
        case 0x48: return KEY_SYM_h;
        case 0x49: return KEY_SYM_i;
        case 0x4A: return KEY_SYM_j;
        case 0x4B: return KEY_SYM_k;
        case 0x4C: return KEY_SYM_l;
        case 0x4D: return KEY_SYM_m;
        case 0x4E: return KEY_SYM_n;
        case 0x4F: return KEY_SYM_o;
        case 0x50: return KEY_SYM_p;
        case 0x51: return KEY_SYM_q;
        case 0x52: return KEY_SYM_r;
        case 0x53: return KEY_SYM_s;
        case 0x54: return KEY_SYM_t;
        case 0x55: return KEY_SYM_u;
        case 0x56: return KEY_SYM_v;
        case 0x57: return KEY_SYM_w;
        case 0x58: return KEY_SYM_x;
        case 0x59: return KEY_SYM_y;
        case 0x5A: return KEY_SYM_z;

        default:
            break;
    }
    
    return KEY_SYM_UNKNOWN;
}