#ifndef ZOBRIST_VALUES_H
#define ZOBRIST_VALUES_H

#include <cstdint>

extern std::uint64_t ZOBRIST_PIECE[12][64];
extern std::uint64_t ZOBRIST_SIDE_TO_MOVE;
extern std::uint64_t ZOBRIST_CASTLING[4];
extern std::uint64_t ZOBRIST_EN_PASSANT[8];

void init_zobrist_keys(std::uint64_t seed = 20230801);

#endif
