
#ifndef SR_KEYSYM_HPP
#define SR_KEYSYM_HPP

#include <cstdint> // uint16_t

#include "lightsky/setup/OS.h" // OS detection

#if defined(LS_OS_WINDOWS)
    #include "soft_render/SR_KeySymWin32.hpp"
#elif defined(SR_PREFER_COCOA)
    #include "soft_render/SR_KeySymCocoa.hpp"
#else
    #include "soft_render/SR_KeySymXlib.hpp"
#endif



/**------------------------------------
 * @brief Keyboard Symbols
-------------------------------------*/
enum SR_KeySymbol : uint32_t;



/**
 * @brief Convert a key symbol into a string which can be used for later
 * reference.
 *
 * @param keySym
 * A value from the SR_KeySymbol enumeration.
 *
 * @return A string, containing the string representation of a key symbol in
 * the SR_KeySymbol enumeration, or NULL if one doesn't exist.
 */
const char* key_to_string(const SR_KeySymbol keySym) noexcept;



#endif /* SR_KEYSYM_HPP */
