
#include <X11/Xlib.h>

#include "soft_render/SR_KeySymXlib.hpp"


const char* key_to_string(const SR_KeySymbol keySym) noexcept
{
  return XKeysymToString(keySym);
}
