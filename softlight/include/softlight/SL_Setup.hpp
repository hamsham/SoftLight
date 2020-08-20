
#ifndef SL_UTILITY_HPP
#define SL_UTILITY_HPP

#include <cstdint>
#include <vector>

#include "lightsky/utils/AlignedAllocator.hpp"


typedef uint32_t SL_IdType;

typedef int16_t coord_shrt_t;
typedef int32_t coord_long_t;

constexpr coord_long_t FIXED_BITS = 16;
constexpr coord_long_t MASK_BITS = 0x8000;



template <typename T>
using SL_AlignedVector = std::vector<T, ls::utils::AlignedAllocator<T>>;



#endif /* SL_UTILITY_HPP */
