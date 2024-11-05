#ifndef MOVE_GENERATOR
#define MOVE_GENERATOR

#include "board_representation.h"
#include "move.h"
#include <vector>

typedef unsigned long long u64;

void generate_pseudo_legal_moves(BoardRepresentation &, std::vector<Move> &);
u64 generate_legal_moves(BoardRepresentation &, std::vector<Move> &);
void generate_pawn_move(BoardRepresentation &, std::vector<Move> &, Square);
void generate_rook_move(BoardRepresentation &, std::vector<Move> &, Square);
void generate_bishop_move(BoardRepresentation &, std::vector<Move> &, Square);
void generate_knight_move(BoardRepresentation &, std::vector<Move> &, Square);
void generate_queen_move(BoardRepresentation &, std::vector<Move> &, Square);
void generate_king_move(BoardRepresentation &, std::vector<Move> &, Square);
void generate_castle(BoardRepresentation &, std::vector<Move> &);

#endif // MOVE_GENERATOR
