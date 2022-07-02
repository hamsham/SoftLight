
#ifndef SL_UTILITY_HPP
#define SL_UTILITY_HPP

#include <cstdint>
#include <vector>

#include "lightsky/utils/AlignedAllocator.hpp"



typedef int16_t sl_lowp_t;
typedef int32_t sl_highp_t;

constexpr sl_highp_t SL_FIXED_BITS = 16;
constexpr sl_highp_t SL_MASK_BITS = 0x8000;



template <typename T>
using SL_AlignedVector = std::vector<T, ls::utils::AlignedAllocator<T>>;



#endif /* SL_UTILITY_HPP */
