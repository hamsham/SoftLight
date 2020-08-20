
#include "softlight/SL_KeySymWin32.hpp"



const char* key_to_string(const SL_KeySymbol keySym) noexcept
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
