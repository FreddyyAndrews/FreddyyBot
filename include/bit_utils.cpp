#ifndef BIT_UTILS
#define BIT_UTILS

#include <cstdint>

inline int get_LSB_index(uint64_t bitboard)
{
    if (bitboard == 0)
        return -1;                    // Return -1 if the bitboard is zero to indicate an error or end condition
    return __builtin_ctzll(bitboard); // Count trailing zeros to find the index of the LSB
}

#endif // CHAR_UTILS
