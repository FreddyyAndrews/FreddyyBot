#include "manager/board_representation.h"
#include <ncurses.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <locale.h>

// Constructors
BoardRepresentation::BoardRepresentation()
{
    input_fen_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // Initialize using standard starting position
}

BoardRepresentation::BoardRepresentation(const std::string &fen)
{
    input_fen_position(fen); // Initialize using FEN string
}

// Function to convert FEN position to bitboards and other variables
void BoardRepresentation::input_fen_position(const std::string &fen)
{
    std::istringstream fen_stream(fen);
    std::string board_part, active_color, castling_rights, en_passant_target, half_move_clock, full_move_number;

    // Split the FEN string into its components
    fen_stream >> board_part >> active_color >> castling_rights >> en_passant_target >> half_move_clock >> full_move_number;

    // Clear the bitboards
    white_pawns = white_knights = white_bishops = white_rooks = white_queens = white_king = 0;
    black_pawns = black_knights = black_bishops = black_rooks = black_queens = black_king = 0;

    // Parse the piece placement part of the FEN string
    std::vector<std::string> ranks;
    std::istringstream board_stream(board_part);
    std::string rank;
    while (std::getline(board_stream, rank, '/'))
    {
        ranks.push_back(rank);
    }

    // Populate the bitboards based on the ranks
    for (int rank_index = 0; rank_index < 8; ++rank_index)
    {
        const std::string &current_rank = ranks[7 - rank_index]; // FEN ranks go from 8th rank to 1st rank
        int file_index = 0;

        for (char square : current_rank)
        {
            if (std::isdigit(square))
            {
                file_index += square - '0'; // Empty squares
            }
            else
            {
                int bit_index = square_to_bit_index(rank_index, file_index);

                switch (square)
                {
                // White pieces
                case 'P':
                    white_pawns |= (1ULL << bit_index);
                    break;
                case 'N':
                    white_knights |= (1ULL << bit_index);
                    break;
                case 'B':
                    white_bishops |= (1ULL << bit_index);
                    break;
                case 'R':
                    white_rooks |= (1ULL << bit_index);
                    break;
                case 'Q':
                    white_queens |= (1ULL << bit_index);
                    break;
                case 'K':
                    white_king |= (1ULL << bit_index);
                    break;
                // Black pieces
                case 'p':
                    black_pawns |= (1ULL << bit_index);
                    break;
                case 'n':
                    black_knights |= (1ULL << bit_index);
                    break;
                case 'b':
                    black_bishops |= (1ULL << bit_index);
                    break;
                case 'r':
                    black_rooks |= (1ULL << bit_index);
                    break;
                case 'q':
                    black_queens |= (1ULL << bit_index);
                    break;
                case 'k':
                    black_king |= (1ULL << bit_index);
                    break;
                }

                ++file_index;
            }
        }
    }

    // Set the active color (who is to move)
    white_to_move = (active_color == "w");

    // Set castling rights
    white_can_castle_kingside = castling_rights.find('K') != std::string::npos;
    white_can_castle_queenside = castling_rights.find('Q') != std::string::npos;
    black_can_castle_kingside = castling_rights.find('k') != std::string::npos;
    black_can_castle_queenside = castling_rights.find('q') != std::string::npos;

    // Set en passant target square (if any)
    if (en_passant_target != "-")
    {
        int file = en_passant_target[0] - 'a';
        int rank = en_passant_target[1] - '1';
        en_passant_square = square_to_bit_index(rank, file);
    }
    else
    {
        en_passant_square = -1; // No en passant square
    }

    // Output parsed data for debugging
    std::cout << "FEN loaded successfully!" << std::endl;
    print_board();
}

// Convert bitboard and other internal representation to FEN string
std::string BoardRepresentation::output_fen_position() const
{
    std::stringstream fen;

    // 1. Piece placement part
    for (int rank = 7; rank >= 0; --rank)
    {
        int empty_squares = 0;
        for (int file = 0; file < 8; ++file)
        {
            int square_index = rank * 8 + file;

            // Check which piece (if any) is on this square and add it to FEN
            char piece = '\0';
            if (white_pawns & (1ULL << square_index))
                piece = 'P';
            else if (white_knights & (1ULL << square_index))
                piece = 'N';
            else if (white_bishops & (1ULL << square_index))
                piece = 'B';
            else if (white_rooks & (1ULL << square_index))
                piece = 'R';
            else if (white_queens & (1ULL << square_index))
                piece = 'Q';
            else if (white_king & (1ULL << square_index))
                piece = 'K';
            else if (black_pawns & (1ULL << square_index))
                piece = 'p';
            else if (black_knights & (1ULL << square_index))
                piece = 'n';
            else if (black_bishops & (1ULL << square_index))
                piece = 'b';
            else if (black_rooks & (1ULL << square_index))
                piece = 'r';
            else if (black_queens & (1ULL << square_index))
                piece = 'q';
            else if (black_king & (1ULL << square_index))
                piece = 'k';
            else
                empty_squares++; // Count empty squares

            // If there's a piece, append the number of empty squares (if any) followed by the piece
            if (piece != '\0')
            {
                if (empty_squares > 0)
                {
                    fen << empty_squares;
                    empty_squares = 0;
                }
                fen << piece;
            }

            // If we're at the end of the rank and there are empty squares, append them
            if (file == 7 && empty_squares > 0)
            {
                fen << empty_squares;
            }
        }

        // Add a '/' after each rank, except for the last one
        if (rank > 0)
        {
            fen << '/';
        }
    }

    // 2. Active color part
    fen << ' ' << (white_to_move ? 'w' : 'b');

    // 3. Castling rights part
    fen << ' ';
    if (white_can_castle_kingside)
        fen << 'K';
    if (white_can_castle_queenside)
        fen << 'Q';
    if (black_can_castle_kingside)
        fen << 'k';
    if (black_can_castle_queenside)
        fen << 'q';
    if (!white_can_castle_kingside && !white_can_castle_queenside &&
        !black_can_castle_kingside && !black_can_castle_queenside)
    {
        fen << '-';
    }

    // 4. En passant target square part
    fen << ' ';
    if (en_passant_square == -1)
    {
        fen << '-';
    }
    else
    {
        int file = en_passant_square % 8;
        int rank = en_passant_square / 8;
        fen << static_cast<char>('a' + file) << (rank + 1);
    }

    // 5. Half-move clock and 6. Full-move number
    fen << " 0 1"; // For now, just output 0 and 1 as dummy values

    return fen.str();
}

// Methods to handle moves
std::vector<std::string> BoardRepresentation::list_next_legal_moves() const
{
    // Return a dummy list of legal moves
    return {"e2e4", "d2d4", "g1f3"};
}

bool BoardRepresentation::make_move(const std::string &move)
{
    // Dummy method to apply a move (always returns true for now)
    std::cout << "Making move: " << move << std::endl;
    return true;
}

// Methods for specific game states
bool BoardRepresentation::is_checkmate() const
{
    // Dummy method (always return false)
    return false;
}

bool BoardRepresentation::is_stalemate() const
{
    // Dummy method (always return false)
    return false;
}

bool BoardRepresentation::is_timeout() const
{
    // Dummy method (always return false)
    return false;
}

bool BoardRepresentation::is_insufficient_material() const
{
    // Dummy method (always return false)
    return false;
}

bool BoardRepresentation::is_draw_by_repetition() const
{
    // Dummy method (always return false)
    return false;
}

bool BoardRepresentation::is_draw_by_fifty_moves() const
{
    // Dummy method (always return false)
    return false;
}

bool BoardRepresentation::is_square_attacked(int square, bool by_white) const
{
    // Dummy method (always return false)
    return false;
}

bool BoardRepresentation::can_castle_kingside(bool white) const
{
    // Dummy method (always return true)
    return true;
}

bool BoardRepresentation::can_castle_queenside(bool white) const
{
    // Dummy method (always return true)
    return true;
}

// Utility methods
void BoardRepresentation::reset()
{
    // Dummy reset method
    std::cout << "Resetting board to initial position." << std::endl;
}

// Helper function to map the bitboard index to a Unicode chess piece symbol
wchar_t BoardRepresentation::get_piece_at_square(int square) const
{
    if (white_pawns & (1ULL << square))
        return L'♙'; // White Pawn
    if (white_knights & (1ULL << square))
        return L'♘'; // White Knight
    if (white_bishops & (1ULL << square))
        return L'♗'; // White Bishop
    if (white_rooks & (1ULL << square))
        return L'♖'; // White Rook
    if (white_queens & (1ULL << square))
        return L'♕'; // White Queen
    if (white_king & (1ULL << square))
        return L'♔'; // White King

    if (black_pawns & (1ULL << square))
        return L'♟'; // Black Pawn
    if (black_knights & (1ULL << square))
        return L'♞'; // Black Knight
    if (black_bishops & (1ULL << square))
        return L'♝'; // Black Bishop
    if (black_rooks & (1ULL << square))
        return L'♜'; // Black Rook
    if (black_queens & (1ULL << square))
        return L'♛'; // Black Queen
    if (black_king & (1ULL << square))
        return L'♚'; // Black King

    return L' '; // Empty square
}

void BoardRepresentation::print_board() const
{
    // Initialize ncurses
    setlocale(LC_ALL, ""); // Set the locale to enable Unicode characters
    initscr();
    start_color(); // Enable color functionality in ncurses

    // Define color pairs for the board and pieces
    init_pair(1, COLOR_BLACK, COLOR_WHITE);  // White squares
    init_pair(2, COLOR_WHITE, COLOR_BLACK);  // Black squares
    init_pair(3, COLOR_WHITE, COLOR_BLACK);  // White pieces
    init_pair(4, COLOR_YELLOW, COLOR_BLACK); // Black pieces

    // Display the chess board
    for (int rank = 7; rank >= 0; --rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            int square = rank * 8 + file;

            // Check if it's a white or black square
            bool is_white_square = (rank + file) % 2 != 0;

            // Get the piece at the current square using the class member function
            wchar_t piece = get_piece_at_square(square); // Call the const member function

            // Set the color based on whether it's a white or black square
            if (is_white_square)
            {
                attron(COLOR_PAIR(1)); // White square
            }
            else
            {
                attron(COLOR_PAIR(2)); // Black square
            }

            // Move to the appropriate position on the screen
            move(7 - rank, file * 2);

            // Print the piece (or space if empty)
            if (piece == ' ')
            {
                printw("  ");
            }
            else
            {
                if (isupper(piece))
                {
                    attron(COLOR_PAIR(3)); // White piece
                }
                else
                {
                    attron(COLOR_PAIR(4)); // Black piece
                }
                printw("%lc ", piece);
                attroff(COLOR_PAIR(3) | COLOR_PAIR(4)); // Turn off piece color after printing
            }

            // Turn off square color after printing
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2));
        }
        printw("\n"); // Move to the next line after printing the rank
    }

    // Refresh the screen to show the board
    refresh();

    // Wait for a key press before exiting
    getch();

    // End ncurses mode
    endwin();
}

std::string BoardRepresentation::bitboard_to_fen(uint64_t bitboard, int rank, int file)
{
    int index = (rank * 8) + file;
    return (bitboard & (1ULL << index)) ? "1" : "0";
}

int BoardRepresentation::square_to_bit_index(int rank, int file)
{
    return (rank * 8) + file;
}

// Helper methods
void BoardRepresentation::update_attacking_defending_squares()
{
    // Dummy method: Does nothing
}

bool BoardRepresentation::is_legal_move(const std::string &move) const
{
    // Dummy method: Always returns true
    return true;
}

void BoardRepresentation::apply_move(const std::string &move)
{
    // Dummy method: Does nothing
}
