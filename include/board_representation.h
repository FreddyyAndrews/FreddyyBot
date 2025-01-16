#ifndef BOARD_REPRESENTATION_H
#define BOARD_REPRESENTATION_H

#include <string>
#include <vector>
#include "square.h"
#include "move.h"
#include <stack>
#include "move_state.h"
#include "char_utils.h"
#include <unordered_set>
#include "zobrist_values.h"
#include "threefold_map.h"

typedef unsigned long long u64;
const std::string START_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

class BoardRepresentation
{
public:
    // Constructors
    BoardRepresentation();                                // Initialize an empty or standard chess board
    explicit BoardRepresentation(const std::string &fen); // Initialize from a FEN string
    BoardRepresentation(const std::string &fen, const std::vector<std::string> &moves);
    BoardRepresentation(const std::vector<std::string> &moves);

    // Methods to handle FEN strings
    void input_fen_position(const std::string &fen); // Load a position from a FEN string
    std::string output_fen_position() const;         // Output the current position as a FEN string

    // Methods to handle moves
    const Move make_move(const std::string &move);   // Apply a move (in UCI notation)
    void make_move(const Move &move);                // Play move internally
    void make_move_literal(const std::string &move); // make move and store position
    void undo_move(const Move &move);                // Undo a move internally

    // Methods for specific game states
    bool move_captures_king(const Move &) const; // Check if a move captures a king piece

    // void print_board() const; // Print the board for debugging purposes

    // 2D array for storing pieces for lookup
    char board[8][8];
    // Castling rights and turn information
    bool white_can_castle_kingside, white_can_castle_queenside;
    bool black_can_castle_kingside, black_can_castle_queenside;
    bool white_to_move;                   // True if it's white's turn, false for black
    bool is_opponent_piece(char &) const; // Check if a piece is an opponent piece
    bool is_only_between(const Square &square_a, const Square &square_b, const Square &between_square) const;
    std::uint64_t zobrist_hash() const;

    // En passant square
    Square en_passant_square; // -1 if no en passant is available

    int halfmove_clock;  // Fifty-move rule counter
    int fullmove_number; // Number of full moves
    std::unordered_set<Square> non_empty_squares;
    bool is_in_check;
    ThreefoldMap threefold_map;

private:
    // Helper methods for legal move generation and game status checks
    wchar_t get_piece_at_square(int square) const; // Get the piece at a square
    void set_non_empty_squares();

    std::stack<MoveState> move_stack;
};

#endif // BOARD_REPRESENTATION_H
