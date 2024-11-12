#include "board_representation.h"
#include <ncurses.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <locale.h>

// Default Constructor
BoardRepresentation::BoardRepresentation()
    : white_can_castle_kingside(false),
      white_can_castle_queenside(false),
      black_can_castle_kingside(false),
      black_can_castle_queenside(false),
      white_to_move(false),
      en_passant_square(-1, -1), // Assuming Square has a constructor that accepts two integers
      halfmove_clock(0),
      fullmove_number(0),
      move_stack()
{
    // Initialize using the standard starting position
    input_fen_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

// Constructor with FEN string
BoardRepresentation::BoardRepresentation(const std::string &fen)
    : white_can_castle_kingside(false),
      white_can_castle_queenside(false),
      black_can_castle_kingside(false),
      black_can_castle_queenside(false),
      white_to_move(false),
      en_passant_square(-1, -1),
      halfmove_clock(0),
      fullmove_number(0),
      move_stack()
{
    // Initialize using the provided FEN string
    input_fen_position(fen);
}

// Function to convert FEN position to 2D board representation and other variables
void BoardRepresentation::input_fen_position(const std::string &fen)
{
    std::istringstream fen_stream(fen);
    std::string board_part, active_color, castling_rights, en_passant_target, half_move_clock_str, full_move_number_str;

    // Split the FEN string into its components
    fen_stream >> board_part >> active_color >> castling_rights >> en_passant_target >> half_move_clock_str >> full_move_number_str;

    // Clear the 2D board (initialize all squares to 'e' for empty)
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            board[i][j] = 'e';
        }
    }

    // Parse the piece placement part of the FEN string
    std::vector<std::string> ranks;
    std::istringstream board_stream(board_part);
    std::string rank;
    while (std::getline(board_stream, rank, '/'))
    {
        ranks.push_back(rank);
    }

    // Populate the 2D board array based on the ranks
    for (int rank_index = 0; rank_index < 8; ++rank_index)
    {
        // Explicitly cast int to size_t
        const std::string &current_rank = ranks[static_cast<size_t>(7 - rank_index)]; // FEN ranks go from 8th rank to 1st rank
        int file_index = 0;

        for (char square : current_rank)
        {
            if (std::isdigit(square))
            {
                file_index += square - '0'; // Skip empty squares
            }
            else
            {
                // Place the piece directly on the board
                board[rank_index][file_index] = square;
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
        int8_t file = en_passant_target[0] - 'a';
        int8_t rank = en_passant_target[1] - '1';
        en_passant_square = Square(rank, file); // define en passant square
    }
    else
    {
        en_passant_square = Square(-1, -1); // No en passant square
    }

    // Parse halfmove clock and fullmove number
    halfmove_clock = std::stoi(half_move_clock_str);
    fullmove_number = std::stoi(full_move_number_str);
}

std::string BoardRepresentation::output_fen_position() const
{
    std::stringstream fen;

    // 1. Piece placement part
    for (int rank = 7; rank >= 0; --rank)
    {
        int empty_squares = 0;
        for (int file = 0; file < 8; ++file)
        {
            char piece = board[rank][file];

            if (piece == 'e') // Empty square
            {
                empty_squares++;
            }
            else
            {
                // If there are any accumulated empty squares, add them to the FEN
                if (empty_squares > 0)
                {
                    fen << empty_squares;
                    empty_squares = 0;
                }
                // Add the piece
                fen << piece;
            }

            // If we are at the end of the rank and have empty squares, append them
            if (file == 7 && empty_squares > 0)
            {
                fen << empty_squares;
            }
        }

        // Add a '/' after each rank except the last one
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
    if (!en_passant_square.exists())
    {
        fen << '-';
    }
    else
    {
        // Increment since we are using base 0 indexing
        fen << static_cast<char>('a' + en_passant_square.file) << (en_passant_square.rank + 1);
    }

    // 5. Half-move clock and 6. Full-move number
    fen << ' ' << halfmove_clock << ' ' << fullmove_number;

    return fen.str();
}

// Method to play move in internal memory
void BoardRepresentation::make_move(const Move &move)
{
    // References to indexes in the board representation array
    char &piece_on_start_square = board[move.start_square.rank][move.start_square.file];
    char &piece_on_target_square = board[move.to_square.rank][move.to_square.file];

    // Track the board status before the move to allow for redo
    move_stack.push(MoveState(
        white_can_castle_kingside,
        white_can_castle_queenside,
        black_can_castle_kingside,
        black_can_castle_queenside,
        en_passant_square,
        halfmove_clock,
        fullmove_number,
        piece_on_target_square, // If any piece is captured by this move
        white_to_move));

    // Determine which piece is moving and its color
    char moving_piece = board[move.start_square.rank][move.start_square.file];
    bool is_white = is_white_piece(moving_piece);

    // Calculate is_capture by checking if the to_square is occupied by an opponent's piece
    bool is_capture = (piece_on_target_square != 'e');
    bool is_pawn_move = (moving_piece == 'p' || moving_piece == 'P');

    // Handle castling rights for king moves
    if ((black_can_castle_kingside || black_can_castle_queenside) && (piece_on_start_square == 'k'))
    {
        black_can_castle_kingside = false;
        black_can_castle_queenside = false;
    }
    else if ((white_can_castle_kingside || white_can_castle_queenside) && (piece_on_start_square == 'K'))
    {
        white_can_castle_kingside = false;
        white_can_castle_queenside = false;
    }

    // Handle castling rights for rook moves and captures
    if (((move.start_square.rank == 0) && (move.start_square.file == 7)) ||
        ((move.to_square.rank == 0) && (move.to_square.file == 7)))
    {
        white_can_castle_kingside = false;
    }
    if (((move.start_square.rank == 0) && (move.start_square.file == 0)) ||
        ((move.to_square.rank == 0) && (move.to_square.file == 0)))
    {
        white_can_castle_queenside = false;
    }
    if (((move.start_square.rank == 7) && (move.start_square.file == 7)) ||
        ((move.to_square.rank == 7) && (move.to_square.file == 7)))
    {
        black_can_castle_kingside = false;
    }
    if (((move.start_square.rank == 7) && (move.start_square.file == 0)) ||
        ((move.to_square.rank == 7) && (move.to_square.file == 0)))
    {
        black_can_castle_queenside = false;
    }

    // All moves require clearing the starting square
    piece_on_start_square = 'e';

    // Set the value in the target square to that of the moving piece except for promotions
    if (move.promotion_piece == 'x')
    {
        piece_on_target_square = moving_piece;
    }
    else
    {
        if (is_white)
        {
            piece_on_target_square = move.promotion_piece - 32;
        }
        else
        {
            piece_on_target_square = move.promotion_piece;
        }
    }

    // Handle moving rook in castling case
    if (move.is_castle)
    {
        // White castles
        if (is_white)
        {
            if (move.to_square.file == 6)
            {
                board[0][5] = 'R';
                board[0][7] = 'e';
            }
            else
            {
                board[0][3] = 'R';
                board[0][0] = 'e';
            }
            // castling rights are already revoked for king moves
        }
        // black castles
        else
        {
            if (move.to_square.file == 6)
            {
                board[7][5] = 'r';
                board[7][7] = 'e';
            }
            else
            {
                board[7][3] = 'r';
                board[7][0] = 'e';
            }
        }
    }

    // remove opponent pawn for en passant move
    else if (move.is_enpassant)
    {
        if (is_white)
        {
            board[move.to_square.rank - 1][move.to_square.file] = 'e';
        }
        else
        {
            board[move.to_square.rank + 1][move.to_square.file] = 'e';
        }
    }

    // Add en passant square for double pawn push (white)
    if (moving_piece == 'P' && move.to_square.rank - move.start_square.rank == 2)
    {
        en_passant_square = Square(move.to_square.rank - 1, move.to_square.file);
    }

    // Add en passant square for double pawn push black
    else if (moving_piece == 'p' && move.to_square.rank - move.start_square.rank == -2)
    {
        en_passant_square = Square(move.to_square.rank + 1, move.to_square.file);
    }

    else if (en_passant_square.exists())
    {
        en_passant_square = Square(-1, -1);
    }

    // Update fullmove number
    if (!white_to_move)
    {
        // Increment fullmove number after Black's move
        fullmove_number++;
    }

    // Update the active color
    white_to_move = !white_to_move;

    if (is_pawn_move || is_capture)
    {
        halfmove_clock = 0;
    }
    else
    {
        halfmove_clock++;
    }
}

// Revert a move in the internal board
void BoardRepresentation::undo_move(const Move &move)
{
    // Retrieve the last saved state
    MoveState previous_state = move_stack.top();
    move_stack.pop();

    // Restore castling rights, en passant, and halfmove clock
    white_can_castle_kingside = previous_state.white_can_castle_kingside;
    white_can_castle_queenside = previous_state.white_can_castle_queenside;
    black_can_castle_kingside = previous_state.black_can_castle_kingside;
    black_can_castle_queenside = previous_state.black_can_castle_queenside;
    en_passant_square = previous_state.en_passant_square;
    halfmove_clock = previous_state.halfmove_clock;
    fullmove_number = previous_state.fullmove_number;
    white_to_move = previous_state.white_to_move;

    // Step 1: Revert the piece move from `to_square` back to `start_square`
    board[move.start_square.rank][move.start_square.file] = board[move.to_square.rank][move.to_square.file];
    board[move.to_square.rank][move.to_square.file] = previous_state.piece_on_target_square; // Restore captured piece or set empty

    // Step 2: Handle special cases
    if (move.is_enpassant)
    {
        // En passant: Place the captured pawn back on the appropriate square
        int captured_pawn_rank = (board[move.start_square.rank][move.start_square.file] == 'P') ? move.to_square.rank - 1 : move.to_square.rank + 1;
        board[captured_pawn_rank][move.to_square.file] = (board[move.start_square.rank][move.start_square.file] == 'P') ? 'p' : 'P';
        board[move.to_square.rank][move.to_square.file] = 'e'; // Empty the en passant target square
    }
    else if (move.promotion_piece != 'x')
    {
        // Promotion: Replace the promoted piece back with a pawn
        board[move.start_square.rank][move.start_square.file] = (white_to_move) ? 'P' : 'p';
    }
    else if (move.is_castle)
    {
        // Castling: Move the rook back to its original position
        if (move.to_square.file == 6)
        {                                                                                 // Kingside castling
            board[move.start_square.rank][5] = 'e';                                       // Clear the rook's castling square
            board[move.start_square.rank][7] = (move.start_square.rank == 0) ? 'R' : 'r'; // Restore rook on original square
        }
        else if (move.to_square.file == 2)
        {                                                                                 // Queenside castling
            board[move.start_square.rank][3] = 'e';                                       // Clear the rook's castling square
            board[move.start_square.rank][0] = (move.start_square.rank == 0) ? 'R' : 'r'; // Restore rook on original square
        }
    }
}

// Method to play move from UCI command
const Move BoardRepresentation::make_move(const std::string &move)
{
    // Extract the from and to squares from the move string
    char from_file = move[0]; // File of the 'from' square (e.g., 'e')
    char from_rank = move[1]; // Rank of the 'from' square (e.g., '2')
    char to_file = move[2];   // File of the 'to' square (e.g., 'e')
    char to_rank = move[3];   // Rank of the 'to' square (e.g., '4')

    char promotion_piece = 'x';

    // Handle promotion (e.g., "e7e8q")
    if (move.length() == 5)
    {
        promotion_piece = move[4];
    }

    // Convert UCI move to rank and file
    int8_t from_rank_int = from_rank - '1'; // Convert char rank to int (e.g., '2' -> 1)
    int8_t from_file_int = from_file - 'a'; // Convert char file to int (e.g., 'e' -> 4)

    int8_t to_rank_int = to_rank - '1'; // Convert char rank to int (e.g., '4' -> 3)
    int8_t to_file_int = to_file - 'a'; // Convert char file to int (e.g., 'e' -> 4)

    bool is_en_passant = false;
    bool is_castle = false;

    // If diagonal pawn move onto en passant square move is en passant
    if ((board[from_rank_int][from_file_int] == 'p' || board[from_rank_int][from_file_int] == 'P') &&
        from_file_int != to_file_int &&
        en_passant_square.rank == to_rank_int &&
        en_passant_square.file == to_file_int)
    {
        is_en_passant = true;
    }

    // If king move from e to g or e to c (only possible if a castle)
    if ((board[from_rank_int][from_file_int] == 'k' || board[from_rank_int][from_file_int] == 'K') &&
        ((from_file_int == 4 && to_file_int == 6) || (from_file_int == 4 && to_file_int == 2)))
    {
        is_castle = true;
    }

    // Create move structure
    Move struct_move = Move(from_rank_int, from_file_int, to_rank_int, to_file_int, is_en_passant, is_castle, promotion_piece);

    make_move(struct_move);
    return struct_move;
}

bool BoardRepresentation::move_captures_king(Move &move) const
{
    const char &piece_on_target_square = board[move.to_square.rank][move.to_square.file];

    return ((piece_on_target_square == 'K') || (piece_on_target_square == 'k'));
}

bool BoardRepresentation::is_opponent_piece(char &piece) const
{
    return ((white_to_move && is_black_piece(piece)) || (!white_to_move && is_white_piece(piece)));
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

// Utility methods
void BoardRepresentation::reset()
{
    // Dummy reset method
    std::cout << "Resetting board to initial position." << std::endl;
}

// Helper function to map the bitboard index to a Unicode chess piece symbol
wchar_t BoardRepresentation::get_piece_at_square(int square) const
{
    // Convert 1D square index to 2D board coordinates
    int rank = square / 8;
    int file = square % 8;

    // Get the piece from the board
    char piece = board[rank][file];

    // Map the piece to its corresponding Unicode chess symbol
    switch (piece)
    {
    case 'P':
        return L'♙'; // White Pawn
    case 'N':
        return L'♘'; // White Knight
    case 'B':
        return L'♗'; // White Bishop
    case 'R':
        return L'♖'; // White Rook
    case 'Q':
        return L'♕'; // White Queen
    case 'K':
        return L'♔'; // White King

    case 'p':
        return L'♟'; // Black Pawn
    case 'n':
        return L'♞'; // Black Knight
    case 'b':
        return L'♝'; // Black Bishop
    case 'r':
        return L'♜'; // Black Rook
    case 'q':
        return L'♛'; // Black Queen
    case 'k':
        return L'♚'; // Black King

    default:
        return L' '; // Empty square
    }
}

void BoardRepresentation::print_board() const
{
    // Initialize ncurses
    setlocale(LC_ALL, ""); // Set the locale to enable Unicode characters
    initscr();
    start_color(); // Enable color functionality in ncurses

    // Define color pairs for all combinations
    init_color(COLOR_GREEN, 710, 396, 114); // Define a custom color for the brown squares
    init_pair(1, COLOR_BLACK, COLOR_WHITE); // Empty white square
    init_pair(2, COLOR_WHITE, COLOR_GREEN); // Empty black square
    init_pair(3, COLOR_WHITE, COLOR_WHITE); // White piece on white square
    init_pair(4, COLOR_WHITE, COLOR_GREEN); // White piece on black square
    init_pair(5, COLOR_BLACK, COLOR_WHITE); // Black piece on white square
    init_pair(6, COLOR_BLACK, COLOR_GREEN); // Black piece on black square

    // Display the chess board
    for (int rank = 7; rank >= 0; --rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            int square = rank * 8 + file;

            // Check if it's a white or black square
            bool is_white_square = (rank + file) % 2 != 0;

            // Get the piece at the current square
            wchar_t piece = get_piece_at_square(square);

            int color_pair = 0;

            // Determine the color pair to use
            if (piece == L' ')
            {
                color_pair = is_white_square ? 1 : 2;
            }
            else
            {
                // Determine if it's a white or black piece
                bool is_white_piece = (piece >= L'♔' && piece <= L'♙');

                if (is_white_piece)
                {
                    color_pair = is_white_square ? 3 : 4;
                }
                else
                {
                    color_pair = is_white_square ? 5 : 6;
                }
            }

            // Set the color pair
            attron(COLOR_PAIR(color_pair));

            // Move to the appropriate position on the screen
            move(7 - rank, file * 2);

            // Print the piece (or space if empty)
            if (piece == L' ')
            {
                printw("  ");
            }
            else
            {
                printw("%lc ", piece);
            }

            // Turn off color attribute after printing
            attroff(COLOR_PAIR(color_pair));
        }
        printw("\n"); // Move to the next line after printing the rank
    }

    // Fetch the FEN string from the current board state
    std::string fen = output_fen_position();

    // Display the FEN string underneath the board
    attron(A_NORMAL); // Ensure default attributes
    mvprintw(9, 0, "FEN: %s", fen.c_str());

    // Refresh the screen to show the updates
    refresh();

    // Wait for a key press before exiting
    getch();

    // End ncurses mode
    endwin();
}

bool BoardRepresentation::is_only_between(const Square &square_a, const Square &square_b, const Square &between_square) const
{
    if (!between_square.is_between(square_a, square_b))
    {
        return false;
    }

    // Calculate the direction to step through the squares
    int delta_rank = (square_b.rank > square_a.rank) - (square_b.rank < square_a.rank);
    int delta_file = (square_b.file > square_a.file) - (square_b.file < square_a.file);

    int current_rank = square_a.rank + delta_rank;
    int current_file = square_a.file + delta_file;

    int non_empty_squares = 0;
    bool between_square_found = false;

    while (current_rank != square_b.rank || current_file != square_b.file)
    {
        // Safety check for board boundaries
        if (current_rank < 0 || current_rank > 7 || current_file < 0 || current_file > 7)
        {
            // Out of bounds, should not happen if squares are aligned correctly
            break;
        }

        // Check if this square is non-empty
        char piece = board[current_rank][current_file];
        if (piece != 'e') // Non-empty
        {
            non_empty_squares++;
            if (current_rank == between_square.rank && current_file == between_square.file)
            {
                between_square_found = true;
            }
        }

        current_rank += delta_rank;
        current_file += delta_file;
    }

    // After the loop, check if only the between_square is non-empty
    char piece_at_between_square = board[between_square.rank][between_square.file];
    if (non_empty_squares == 1 && between_square_found && piece_at_between_square != 'e')
    {
        return true;
    }
    else
    {
        return false;
    }
}
