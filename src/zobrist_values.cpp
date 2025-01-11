#include "zobrist_values.h"
#include <random>

std::uint64_t ZOBRIST_PIECE[12][64];
std::uint64_t ZOBRIST_SIDE_TO_MOVE;
std::uint64_t ZOBRIST_CASTLING[4];
std::uint64_t ZOBRIST_EN_PASSANT[8];

void init_zobrist_keys(std::uint64_t seed)
{
    std::mt19937_64 rng(seed);
    for (int pieceIndex = 0; pieceIndex < 12; ++pieceIndex)
    {
        for (int square = 0; square < 64; ++square)
        {
            ZOBRIST_PIECE[pieceIndex][square] = rng();
        }
    }

    ZOBRIST_SIDE_TO_MOVE = rng();

    for (int i = 0; i < 4; ++i)
    {
        ZOBRIST_CASTLING[i] = rng();
    }

    for (int i = 0; i < 8; ++i)
    {
        ZOBRIST_EN_PASSANT[i] = rng();
    }
}
