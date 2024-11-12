#ifndef MOVE_GENERATOR
#define MOVE_GENERATOR

#include "board_representation.h"
#include "move.h"
#include "square.h" // Ensure Square is included
#include <vector>
#include <iostream>
#include <cctype>
#include <cassert>
#include "square_to_square_map.cpp"
#include <algorithm>
#include "char_utils.h"
#include "bit_utils.cpp"

typedef unsigned long long u64;

// Function declarations with variable names for clarity:

// Generates all pseudo-legal moves, considering pins and attacked squares
void generate_pseudo_legal_moves(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    std::vector<SquareToSquareMap> &attacked_squares,
    Square &king_position);

// Generates all legal moves from the current board state
u64 generate_legal_moves(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list);

// Generates pawn moves from a given square
void generate_pawn_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares);

// Generates rook moves from a given square
void generate_rook_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares);

// Generates bishop moves from a given square
void generate_bishop_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares);

// Generates knight moves from a given square
void generate_knight_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares);

// Generates queen moves from a given square
void generate_queen_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares);

// Generates king moves from a given square
void generate_king_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares);

// Generates castling moves if available
void generate_castle(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list);

#endif // MOVE_GENERATOR
