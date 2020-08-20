
#include <X11/Xlib.h>

#include "softlight/SL_KeySymXlib.hpp"


const char* key_to_string(const SL_KeySymbol keySym) noexcept
{
  return XKeysymToString(keySym);
}
