
#ifndef SL_KEYSYM_HPP
#define SL_KEYSYM_HPP

#include <cstdint> // uint16_t

#include "lightsky/setup/OS.h" // OS detection

#if defined(LS_OS_WINDOWS)
    #include "softlight/SL_KeySymWin32.hpp"
#elif defined(SL_PREFER_COCOA)
    #include "softlight/SL_KeySymCocoa.hpp"
#else
    #include "softlight/SL_KeySymXlib.hpp"
#endif



/**------------------------------------
 * @brief Keyboard Symbols
-------------------------------------*/
enum SL_KeySymbol : uint32_t;



/**
 * @brief Convert a key symbol into a string which can be used for later
 * reference.
 *
 * @param keySym
 * A value from the SL_KeySymbol enumeration.
 *
 * @return A string, containing the string representation of a key symbol in
 * the SL_KeySymbol enumeration, or NULL if one doesn't exist.
 */
const char* key_to_string(const SL_KeySymbol keySym) noexcept;



#endif /* SL_KEYSYM_HPP */
