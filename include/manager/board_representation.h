#ifndef BOARD_REPRESENTATION_H
#define BOARD_REPRESENTATION_H

#include <string>
#include <vector>
#include "square.h"
#include "move.h"

class BoardRepresentation
{
public:
    // Constructors
    BoardRepresentation();                                // Initialize an empty or standard chess board
    explicit BoardRepresentation(const std::string &fen); // Initialize from a FEN string

    // Methods to handle FEN strings
    void input_fen_position(const std::string &fen); // Load a position from a FEN string
    std::string output_fen_position() const;         // Output the current position as a FEN string

    // Methods to handle moves
    std::vector<std::string> list_next_legal_moves() const; // Generate all legal moves from the current position
    bool make_move(const std::string &move);                // Apply a move (in UCI notation)

    // Methods for specific game states
    bool is_checkmate() const;             // Check if the current player is in checkmate
    bool is_stalemate() const;             // Check if the current player is in stalemate
    bool is_insufficient_material() const; // Check if the game is a draw due to insufficient material
    bool is_draw_by_repetition() const;    // Check if the game is a draw by repetition
    bool is_draw_by_fifty_moves() const;   // Check if the game is a draw by the fifty-move rule

    // Utility methods
    void reset();
    // Reset the board to the initial position
    void print_board() const; // Print the board for debugging purposes

private:
    // 2D array for storing pieces for lookup
    char board[8][8];

    // Castling rights and turn information
    bool white_can_castle_kingside, white_can_castle_queenside;
    bool black_can_castle_kingside, black_can_castle_queenside;
    bool white_to_move; // True if it's white's turn, false for black

    // En passant square
    int en_passant_square; // -1 if no en passant is available

    int halfmove_clock;  // Fifty-move rule counter
    int fullmove_number; // Number of full moves

    // Helper methods for legal move generation and game status checks
    wchar_t get_piece_at_square(int square) const;                                       // Get the piece at a square
    bool remove_piece_from_square(int square, char &moving_piece, bool &is_white_piece); // Remove a piece from a square
    void handle_captures_and_castling_rights(int to_square, bool is_white_piece);
    bool remove_white_piece_from_square(int square);
    bool remove_black_piece_from_square(int square);
    void handle_pawn_promotion(int to_square, char promotion_piece, bool is_white_piece);
    void handle_regular_move(int from_square, int to_square, char moving_piece, bool is_white_piece,
                             bool &is_castling_move, bool &is_double_pawn_push);
    void update_en_passant_square(char moving_piece, int from_square, int to_square, bool is_double_pawn_push);
    void remove_pawn_enpassant(int square, bool is_white_piece);
    bool make_move(Move move); // Play move internally
};

#endif // BOARD_REPRESENTATION_H
