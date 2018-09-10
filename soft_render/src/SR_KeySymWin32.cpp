
#include "soft_render/SR_KeySymWin32.hpp"



const char* key_to_string(const SR_KeySymbol keySym) noexcept
{
    static thread_local char keyStr[2] = {'\0', '\0'};
    keyStr[0] = (char)MapVirtualKey((UINT)keySym, MAPVK_VK_TO_CHAR);

    return keyStr;
}
