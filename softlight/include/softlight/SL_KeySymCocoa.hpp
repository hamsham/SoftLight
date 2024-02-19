
#ifndef SL_KEYSYM_COCOA_HPP
#define SL_KEYSYM_COCOA_HPP

#include <cstdint> // uint32_t



/*-------------------------------------
 * @brief Keyboard Symbols
-------------------------------------*/
enum SL_KeySymbol : uint16_t;



/**
 * @brief Convert a key symbol into a string which can be used for later
 * reference.
 *
 * @param keySym
 * A cocoa keycode.
 *
 * @return A string, containing the string representation of a key symbol in
 * the SL_KeySymbol enumeration, or NULL if one doesn't exist.
 */
const char* sl_key_to_string_cocoa(const uint32_t keySym) noexcept;



/**
 * @brief Convert a keycode into an SL_KeySymbol.
 *
 * @param keycode
 * A keycode value from the native windowing backend.
 *
 * @return SL_KeySymbol
 */
SL_KeySymbol sl_keycode_to_keysym_cocoa(const uint32_t keycode) noexcept;



#endif /* SL_KEYSYM_COCOA_HPP */
