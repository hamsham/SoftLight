
#import <QuartzCore/QuartzCore.h>

#include "soft_render/SR_KeySymCocoa.hpp"


const char* key_to_string(const SR_KeySymbol keySym) noexcept
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
