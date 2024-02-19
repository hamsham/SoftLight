
#ifndef SL_KEYSYM_WIN32_HPP
#define SL_KEYSYM_WIN32_HPP

#include <cstdint> // uint32_t



/*-------------------------------------
 * @brief Keyboard Symbols
-------------------------------------*/
enum SL_KeySymbol : uint32_t;



/**
 * @brief Convert a key symbol into a string which can be used for later
 * reference.
 *
 * @param keySym
 * A win32 keycode.
 *
 * @return A string, containing the string representation of a key symbol in
 * the SL_KeySymbol enumeration, or NULL if one doesn't exist.
 */
const char* sl_key_to_string_win32(const uint32_t keySym) noexcept;



/**
 * @brief Convert a keycode into an SL_KeySymbol.
 *
 * @param keycode
 * A keycode value from the native windowing backend.
 *
 * @return SL_KeySymbol
 */
SL_KeySymbol sl_keycode_to_keysym_win32(const uint32_t keycode) noexcept;



#endif /* SL_KEYSYM_WIN32_HPP */
