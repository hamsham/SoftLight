
#import <QuartzCore/QuartzCore.h>

#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_KeySymCocoa.hpp"



/*-------------------------------------
 * Keycode to string
-------------------------------------*/
const char* sl_key_to_string_cocoa(const SL_KeySymbol keySym) noexcept
{
    static thread_local UniChar charStr[17];
    UniCharCount count = 0;
    CGEventRef evt;

    for (unsigned i = sizeof(charStr); i--;)
    {
        charStr[i] = 0;
    }

    evt = CGEventCreateKeyboardEvent (nullptr, (CGKeyCode)keySym, false);
    CGEventKeyboardGetUnicodeString(evt, 16, &count, charStr);
    CFRelease(evt);

    return reinterpret_cast<const char*>(charStr);
}



/*-------------------------------------
 * Keycode to SL_KeySym
-------------------------------------*/
SL_KeySymbol sl_keycode_to_keysym_cocoa(const uint32_t keycode) noexcept
{
    switch (keycode)
    {
        case 0x35: return KEY_SYM_ESCAPE;

        case 0x38: return KEY_SYM_L_SHIFT;
        case 0x3B: return KEY_SYM_L_CONTROL;
        case 0x3A: return KEY_SYM_L_ALT;
        case 0x37: return KEY_SYM_L_SUPER;
        case 0x3F: return KEY_SYM_L_META;

        case 0x3C: return KEY_SYM_R_SHIFT;
        case 0x3E: return KEY_SYM_R_CONTROL;
        case 0x3D: return KEY_SYM_R_ALT;
        //case 0x37: return KEY_SYM_R_SUPER;
        //case 0x3F: return KEY_SYM_R_META;

        case 0x7A: return KEY_SYM_F1;
        case 0x78: return KEY_SYM_F2;
        case 0x63: return KEY_SYM_F3;
        case 0x76: return KEY_SYM_F4;
        case 0x60: return KEY_SYM_F5;
        case 0x61: return KEY_SYM_F6;
        case 0x62: return KEY_SYM_F7;
        case 0x64: return KEY_SYM_F8;
        case 0x65: return KEY_SYM_F9;
        case 0x6D: return KEY_SYM_F10;
        case 0x67: return KEY_SYM_F11;
        case 0x6F: return KEY_SYM_F12;
        case 0x69: return KEY_SYM_F13;
        case 0x6B: return KEY_SYM_F14;
        case 0x71: return KEY_SYM_F15;
        case 0x6A: return KEY_SYM_F16;
        case 0x40: return KEY_SYM_F17;
        case 0x4F: return KEY_SYM_F18;
        case 0x50: return KEY_SYM_F19;
        case 0x5A: return KEY_SYM_F20;
        case 0xF718: return KEY_SYM_F21;
        case 0xF719: return KEY_SYM_F22;
        case 0xF71A: return KEY_SYM_F23;
        case 0xF71B: return KEY_SYM_F24;

        case 0x12: return KEY_SYM_1;
        case 0x13: return KEY_SYM_2;
        case 0x14: return KEY_SYM_3;
        case 0x15: return KEY_SYM_4;
        case 0x17: return KEY_SYM_5;
        case 0x16: return KEY_SYM_6;
        case 0x1A: return KEY_SYM_7;
        case 0x1C: return KEY_SYM_8;
        case 0x19: return KEY_SYM_9;
        case 0x1D: return KEY_SYM_0;

        case 0x39: return KEY_SYM_CAPS_LOCK;
        case 0x47: return KEY_SYM_NUM_LOCK;
        //case KEY_SYM_F14: return KEY_SYM_SCROLL_LOCK;

        case 0x53: return KEY_SYM_NUMPAD_1;
        case 0x54: return KEY_SYM_NUMPAD_2;
        case 0x55: return KEY_SYM_NUMPAD_3;
        case 0x56: return KEY_SYM_NUMPAD_4;
        case 0x57: return KEY_SYM_NUMPAD_5;
        case 0x58: return KEY_SYM_NUMPAD_6;
        case 0x59: return KEY_SYM_NUMPAD_7;
        case 0x5B: return KEY_SYM_NUMPAD_8;
        case 0x5C: return KEY_SYM_NUMPAD_9;
        case 0x52: return KEY_SYM_NUMPAD_0;
        //case KEY_SYM_NUMPAD_1: return KEY_SYM_NUMPAD_END; // Numpad alias: 1
        //case KEY_SYM_NUMPAD_2: return KEY_SYM_NUMPAD_DOWN; // Numpad alias: 2
        //case KEY_SYM_NUMPAD_3: return KEY_SYM_NUMPAD_PG_DOWN; // Numpad alias: 3
        //case KEY_SYM_NUMPAD_4: return KEY_SYM_NUMPAD_LEFT; // Numpad alias: 4
        //case KEY_SYM_NUMPAD_5: return KEY_SYM_NUMPAD_BEGIN; // Numpad alias: 5
        //case KEY_SYM_NUMPAD_6: return KEY_SYM_NUMPAD_RIGHT; // Numpad alias: 6
        //case KEY_SYM_NUMPAD_7: return KEY_SYM_NUMPAD_HOME; // Numpad alias: 7
        //case KEY_SYM_NUMPAD_8: return KEY_SYM_NUMPAD_UP; // Numpad alias: 8
        //case KEY_SYM_NUMPAD_9: return KEY_SYM_NUMPAD_PG_UP; // Numpad alias: 9
        //case KEY_SYM_NUMPAD_0: return KEY_SYM_NUMPAD_INSERT; // Numpad alias: 0
        case 0x45: return KEY_SYM_NUMPAD_ADD;
        case 0x4E: return KEY_SYM_NUMPAD_SUB;
        case 0x43: return KEY_SYM_NUMPAD_MUL;
        case 0x4B: return KEY_SYM_NUMPAD_DIV;
        case 0x51: return KEY_SYM_NUMPAD_EQUAL;
        case 0x4C: return KEY_SYM_NUMPAD_ENTER;
        //case KEY_SYM_NUM_LOCK: return KEY_SYM_NUMPAD_DELETE;
        case 0x41: return KEY_SYM_NUMPAD_DECIMAL;
        case 0xFF: return KEY_SYM_NUMPAD_SEP;

        //case KEY_SYM_F13: return KEY_SYM_PRINT_SCREEN;
        //case KEY_SYM_F15: return KEY_SYM_PAUSE;
        //case KEY_SYM_F13: return KEY_SYM_SYS_REQ;
        //case 0x3F: return KEY_SYM_INSERT;
        case 0x75: return KEY_SYM_DELETE;
        case 0x73: return KEY_SYM_HOME;
        case 0x77: return KEY_SYM_END;
        case 0x74: return KEY_SYM_PG_UP;
        case 0x79: return KEY_SYM_PG_DOWN;
        case 0x7B: return KEY_SYM_LEFT;
        case 0x7C: return KEY_SYM_RIGHT;
        case 0x7E: return KEY_SYM_UP;
        case 0x7D: return KEY_SYM_DOWN;
        case 0x31: return KEY_SYM_SPACE;
        case 0x33: return KEY_SYM_BACKSPACE;
        case 0x24: KEY_SYM_RETURN; // Enter: return
        //case KEY_SYM_RETURN: return KEY_SYM_LINE_FEED;
        case 0x30: return KEY_SYM_TAB;
        //case KEY_SYM_NUM_LOCK: return KEY_SYM_CLEAR;

        case 0x27: return KEY_SYM_SINGLE_QUOTE; // '
        //case KEY_SYM_SINGLE_QUOTE: return KEY_SYM_DOUBLE_QUOTE; // "
        case 0x32: return KEY_SYM_GRAVE; // `
        //case KEY_SYM_GRAVE: return KEY_SYM_TILDE; // ~
        //case KEY_SYM_2: return KEY_SYM_AT; // @
        //case KEY_SYM_3: return KEY_SYM_POUND; // #
        //case KEY_SYM_4: return KEY_SYM_DOLLAR; // $
        //case KEY_SYM_5: return KEY_SYM_PERCENT; // %
        //case KEY_SYM_6: return KEY_SYM_CARET; // ^
        //case KEY_SYM_7: return KEY_SYM_AMPERSAND; // &
        //case KEY_SYM_8: return KEY_SYM_ASTERISK; // *
        case 0x1B: return KEY_SYM_HYPHEN; // -
        //case KEY_SYM_HYPHEN: return KEY_SYM_UNDERSCORE; // _
        case 0x18: return KEY_SYM_PLUS; // +
        //case KEY_SYM_PLUS: return KEY_SYM_EQUALS; // =

        //case KEY_SYM_9: return KEY_SYM_PARENTHESIS_LEFT;
        //case KEY_SYM_0: return KEY_SYM_PARENTHESIS_RIGHT;
        case 0x21: return KEY_SYM_BRACKET_LEFT; // [
        case 0x1E: return KEY_SYM_BRACKET_RIGHT; // ]
        //case KEY_SYM_BRACKET_LEFT: return KEY_SYM_BRACE_LEFT; // {
        //case KEY_SYM_BRACKET_RIGHT: return KEY_SYM_BRACE_RIGHT; // }
        //case 0x2B: return KEY_SYM_ANGLE_LEFT; // <
        //case 0x2F: return KEY_SYM_ANGLE_RIGHT; // >
        case 0x2C: return KEY_SYM_FORWARD_SLASH;     /* / */
        case 0x2A: return KEY_SYM_BACKWARD_SLASH; /* \ */
        case 0x0A: return KEY_SYM_VERTICAL_BAR;       /* | */
        case 0x29: return KEY_SYM_SEMICOLON;
        //case KEY_SYM_SEMICOLON: return KEY_SYM_COLON;
        case 0x2B: return KEY_SYM_COMMA;
        case 0x2F: return KEY_SYM_PERIOD;
        //case KEY_SYM_1: return KEY_SYM_EXCLAMATION;
        //case KEY_SYM_FORWARD_SLASH: return KEY_SYM_QUESTION;

        case 0x00: return KEY_SYM_a;
        case 0x0B: return KEY_SYM_b;
        case 0x08: return KEY_SYM_c;
        case 0x02: return KEY_SYM_d;
        case 0x0E: return KEY_SYM_e;
        case 0x03: return KEY_SYM_f;
        case 0x05: return KEY_SYM_g;
        case 0x04: return KEY_SYM_h;
        case 0x22: return KEY_SYM_i;
        case 0x26: return KEY_SYM_j;
        case 0x28: return KEY_SYM_k;
        case 0x25: return KEY_SYM_l;
        case 0x2E: return KEY_SYM_m;
        case 0x2D: return KEY_SYM_n;
        case 0x1F: return KEY_SYM_o;
        case 0x23: return KEY_SYM_p;
        case 0x0C: return KEY_SYM_q;
        case 0x0F: return KEY_SYM_r;
        case 0x01: return KEY_SYM_s;
        case 0x11: return KEY_SYM_t;
        case 0x20: return KEY_SYM_u;
        case 0x09: return KEY_SYM_v;
        case 0x0D: return KEY_SYM_w;
        case 0x07: return KEY_SYM_x;
        case 0x10: return KEY_SYM_y;
        case 0x06: return KEY_SYM_z;

        //case KEY_SYM_a: return KEY_SYM_A;
        //case KEY_SYM_b: return KEY_SYM_B;
        //case KEY_SYM_c: return KEY_SYM_C;
        //case KEY_SYM_d: return KEY_SYM_D;
        //case KEY_SYM_e: return KEY_SYM_E;
        //case KEY_SYM_f: return KEY_SYM_F;
        //case KEY_SYM_g: return KEY_SYM_G;
        //case KEY_SYM_h: return KEY_SYM_H;
        //case KEY_SYM_i: return KEY_SYM_I;
        //case KEY_SYM_j: return KEY_SYM_J;
        //case KEY_SYM_k: return KEY_SYM_K;
        //case KEY_SYM_l: return KEY_SYM_L;
        //case KEY_SYM_m: return KEY_SYM_M;
        //case KEY_SYM_n: return KEY_SYM_N;
        //case KEY_SYM_o: return KEY_SYM_O;
        //case KEY_SYM_p: return KEY_SYM_P;
        //case KEY_SYM_q: return KEY_SYM_Q;
        //case KEY_SYM_r: return KEY_SYM_R;
        //case KEY_SYM_s: return KEY_SYM_S;
        //case KEY_SYM_t: return KEY_SYM_T;
        //case KEY_SYM_u: return KEY_SYM_U;
        //case KEY_SYM_v: return KEY_SYM_V;
        //case KEY_SYM_w: return KEY_SYM_W;
        //case KEY_SYM_x: return KEY_SYM_X;
        //case KEY_SYM_y: return KEY_SYM_Y;
        //case KEY_SYM_z: return KEY_SYM_Z;
        
        default:
            break;
    }
    
    return KEY_SYM_UNKNOWN;
}
