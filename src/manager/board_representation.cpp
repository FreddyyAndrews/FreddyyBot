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
        const std::string &current_rank = ranks[7 - rank_index]; // FEN ranks go from 8th rank to 1st rank
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
        int file = en_passant_target[0] - 'a';
        int rank = en_passant_target[1] - '1';
        en_passant_square = rank * 8 + file; // Linear index for the 8x8 board
    }
    else
    {
        en_passant_square = -1; // No en passant square
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
    fen << ' ' << halfmove_clock << ' ' << fullmove_number;

    return fen.str();
}

// Methods to handle moves
std::vector<std::string> BoardRepresentation::list_next_legal_moves() const
{
    // Return a dummy list of legal moves
    return {"e2e4", "d2d4", "g1f3"};
}

// TODO: Implement this method
bool BoardRepresentation::make_move(Move move)
{
    return false;
}

bool BoardRepresentation::make_move(const std::string &move)
{
    print_board();
    // Extract the from and to squares from the move string
    std::string from_square_str = move.substr(0, 2);
    std::string to_square_str = move.substr(2, 2);
    char promotion_piece = '\0';

    // Handle promotion (e.g., "e7e8q")
    if (move.length() == 5)
    {
        promotion_piece = move[4];
    }

    // Convert algebraic notation to square indices (0-63)
    int from_square = algebraic_to_square(from_square_str);
    int to_square = algebraic_to_square(to_square_str);

    if (from_square == -1 || to_square == -1)
    {
        std::cerr << "Invalid move: " << move << std::endl;
        return false;
    }

    // Determine which piece is moving and its color
    char moving_piece = '\0';
    bool is_white_piece = false;

    // Flags for special moves
    bool is_castling_move = false;
    bool is_double_pawn_push = false;

    // Identify the moving piece and update the corresponding bitboard
    if (!remove_piece_from_square(from_square, moving_piece, is_white_piece))
    {
        std::cerr << "No piece found at square " << from_square_str << std::endl;
        return false;
    }

    // Convert the 1D index to 2D board coordinates
    int to_rank = to_square / 8;
    int to_file = to_square % 8;

    // Calculate is_capture by checking if the to_square is occupied by an opponent's piece
    bool is_capture = false;
    uint64_t opponent_pieces = 0;

    // Check if the target square is occupied by an opponent's piece
    char piece_on_target_square = board[to_rank][to_file];

    if (is_white_piece)
    {
        // Check if the target square contains a black piece
        if (piece_on_target_square >= 'a' && piece_on_target_square <= 'z') // Lowercase letters represent black pieces
        {
            is_capture = true;
        }
    }
    else
    {
        // Check if the target square contains a white piece
        if (piece_on_target_square >= 'A' && piece_on_target_square <= 'Z') // Uppercase letters represent white pieces
        {
            is_capture = true;
        }
    }

    if (opponent_pieces & (1ULL << to_square))
    {
        is_capture = true;
    }

    // Determine if the move is an en passant capture
    bool is_en_passant_capture = false;
    if ((moving_piece == 'P' || moving_piece == 'p') && abs(from_square - to_square) % 8 != 0)
    {
        // Pawn is moving diagonally
        if (en_passant_square == to_square)
        {
            // Destination square matches en passant square
            is_en_passant_capture = true;
            is_capture = true; // Set capture flag
        }
    }

    // Handle captures and en passant
    if (is_capture)
    {
        if (is_en_passant_capture)
        {
            // Remove the opponent's pawn from the square behind the en passant square
            int captured_pawn_square = is_white_piece ? to_square - 8 : to_square + 8;
            remove_pawn_enpassant(captured_pawn_square, is_white_piece);
        }
        else
        {
            // Handle normal captures
            handle_captures_and_castling_rights(to_square, is_white_piece);
        }
    }

    // Handle pawn promotions and regular moves
    uint64_t to_bit = (1ULL << to_square);
    if (promotion_piece != '\0' && (moving_piece == 'P' || moving_piece == 'p'))
    {
        handle_pawn_promotion(to_bit, promotion_piece, is_white_piece);
    }
    else
    {
        handle_regular_move(from_square, to_square, moving_piece, is_white_piece, is_castling_move, is_double_pawn_push);
    }

    // Update en passant square
    update_en_passant_square(moving_piece, from_square, to_square, is_double_pawn_push);

    // Update fullmove number
    if (!white_to_move)
    {
        // Increment fullmove number after Black's move
        fullmove_number++;
    }

    // Update the active color
    white_to_move = !white_to_move;

    // Handle halfmove clock
    bool is_pawn_move = (moving_piece == 'P' || moving_piece == 'p');

    if (is_pawn_move || is_capture)
    {
        halfmove_clock = 0;
    }
    else
    {
        halfmove_clock++;
    }

    print_board();
    return true;
}

void BoardRepresentation::remove_pawn_enpassant(int square, bool is_white_piece)
{
    // Convert the 1D square index to 2D board coordinates
    int rank = square / 8;
    int file = square % 8;

    char &piece_on_square = board[rank][file];

    if (is_white_piece)
    {
        // Opponent is black
        if (piece_on_square == 'p') // Black pawn
        {
            piece_on_square = 'e'; // Set the square to empty
        }
        else
        {
            std::cerr << "Error: No black pawn to capture en passant on square " << square_to_algebraic(square) << std::endl;
        }
    }
    else
    {
        // Opponent is white
        if (piece_on_square == 'P') // White pawn
        {
            piece_on_square = 'e'; // Set the square to empty
        }
        else
        {
            std::cerr << "Error: No white pawn to capture en passant on square " << square_to_algebraic(square) << std::endl;
        }
    }
}

bool BoardRepresentation::remove_piece_from_square(int square, char &moving_piece, bool &is_white_piece)
{
    // Convert 1D square index to 2D board coordinates
    int rank = square / 8;
    int file = square % 8;

    // Get the piece from the board
    char piece_on_square = board[rank][file];

    // Check if there's a piece on the square
    if (piece_on_square == 'e') // Empty square
    {
        return false;
    }

    // Set the moving piece and determine the color
    moving_piece = piece_on_square;
    is_white_piece = (piece_on_square >= 'A' && piece_on_square <= 'Z'); // Uppercase = white piece

    // Remove the piece from the board by setting the square to 'e' (empty)
    board[rank][file] = 'e';

    return true;
}

void BoardRepresentation::handle_captures_and_castling_rights(int to_square, bool is_white_piece)
{
    bool piece_captured = false;
    if (is_white_piece)
    {
        // Check for black pieces to capture
        if (remove_black_piece_from_square(to_square))
        {
            piece_captured = true;
            // Update black castling rights if a rook is captured
            if (to_square == algebraic_to_square("a8"))
                black_can_castle_queenside = false;
            else if (to_square == algebraic_to_square("h8"))
                black_can_castle_kingside = false;
        }
    }
    else
    {
        // Check for white pieces to capture
        if (remove_white_piece_from_square(to_square))
        {
            piece_captured = true;
            // Update white castling rights if a rook is captured
            if (to_square == algebraic_to_square("a1"))
                white_can_castle_queenside = false;
            else if (to_square == algebraic_to_square("h1"))
                white_can_castle_kingside = false;
        }
    }
}

bool BoardRepresentation::remove_white_piece_from_square(int square)
{
    // Convert 1D square index to 2D board coordinates
    int rank = square / 8;
    int file = square % 8;

    // Check if the piece is a white piece (uppercase letter) on the board
    char piece_on_square = board[rank][file];
    if (piece_on_square == 'P' || piece_on_square == 'N' || piece_on_square == 'B' ||
        piece_on_square == 'R' || piece_on_square == 'Q' || piece_on_square == 'K')
    {
        // Remove the piece by setting the square to 'e' (empty)
        board[rank][file] = 'e';
        return true;
    }

    return false;
}

bool BoardRepresentation::remove_black_piece_from_square(int square)
{
    // Convert 1D square index to 2D board coordinates
    int rank = square / 8;
    int file = square % 8;

    // Check if the piece is a black piece (lowercase letter) on the board
    char piece_on_square = board[rank][file];
    if (piece_on_square == 'p' || piece_on_square == 'n' || piece_on_square == 'b' ||
        piece_on_square == 'r' || piece_on_square == 'q' || piece_on_square == 'k')
    {
        // Remove the piece by setting the square to 'e' (empty)
        board[rank][file] = 'e';
        return true;
    }

    return false;
}

void BoardRepresentation::handle_pawn_promotion(int to_square, char promotion_piece, bool is_white_piece)
{
    // Convert the 1D to_square index to 2D board coordinates
    int rank = to_square / 8;
    int file = to_square % 8;

    // If the piece is white, convert promotion_piece to uppercase
    if (is_white_piece)
    {
        promotion_piece = std::toupper(promotion_piece); // Ensure uppercase for white pieces
    }
    else
    {
        promotion_piece = std::tolower(promotion_piece); // Ensure lowercase for black pieces
    }

    // Set the promoted piece in the board array
    board[rank][file] = promotion_piece;
}

void BoardRepresentation::handle_regular_move(int from_square, int to_square, char moving_piece, bool is_white_piece,
                                              bool &is_castling_move, bool &is_double_pawn_push)
{
    // Convert 1D indices to 2D board coordinates
    int from_rank = from_square / 8, from_file = from_square % 8;
    int to_rank = to_square / 8, to_file = to_square % 8;

    // Move the piece to the new square
    board[to_rank][to_file] = moving_piece;
    board[from_rank][from_file] = 'e'; // Set the from square to empty

    // Handle double pawn push
    if (moving_piece == 'P' && from_rank == 1 && to_rank == 3)
        is_double_pawn_push = true;
    else if (moving_piece == 'p' && from_rank == 6 && to_rank == 4)
        is_double_pawn_push = true;

    // Handle castling
    if (moving_piece == 'K' && from_square == algebraic_to_square("e1"))
    {
        white_can_castle_kingside = white_can_castle_queenside = false;
        if (to_square == algebraic_to_square("g1"))
        {
            is_castling_move = true;
            board[7][5] = 'R'; // Move rook
            board[7][7] = 'e'; // Clear the original rook square
        }
        else if (to_square == algebraic_to_square("c1"))
        {
            is_castling_move = true;
            board[7][3] = 'R'; // Move rook
            board[7][0] = 'e'; // Clear the original rook square
        }
    }
    else if (moving_piece == 'k' && from_square == algebraic_to_square("e8"))
    {
        black_can_castle_kingside = black_can_castle_queenside = false;
        if (to_square == algebraic_to_square("g8"))
        {
            is_castling_move = true;
            board[0][5] = 'r'; // Move rook
            board[0][7] = 'e'; // Clear the original rook square
        }
        else if (to_square == algebraic_to_square("c8"))
        {
            is_castling_move = true;
            board[0][3] = 'r'; // Move rook
            board[0][0] = 'e'; // Clear the original rook square
        }
    }

    // Handle rook moves that affect castling rights
    if (moving_piece == 'R' && from_square == algebraic_to_square("a1"))
        white_can_castle_queenside = false;
    if (moving_piece == 'R' && from_square == algebraic_to_square("h1"))
        white_can_castle_kingside = false;
    if (moving_piece == 'r' && from_square == algebraic_to_square("a8"))
        black_can_castle_queenside = false;
    if (moving_piece == 'r' && from_square == algebraic_to_square("h8"))
        black_can_castle_kingside = false;
}

void BoardRepresentation::update_en_passant_square(char moving_piece, int from_square, int to_square, bool is_double_pawn_push)
{
    if (moving_piece == 'P' || moving_piece == 'p')
    {
        if (is_double_pawn_push)
        {
            int rank_diff = to_square / 8 - from_square / 8;
            en_passant_square = from_square + (rank_diff / 2) * 8;
        }
        else
        {
            en_passant_square = -1;
        }
    }
    else
    {
        en_passant_square = -1;
    }
}

int BoardRepresentation::algebraic_to_square(const std::string &square_str) const
{
    if (square_str.length() != 2)
    {
        return -1;
    }
    char file_char = square_str[0];
    char rank_char = square_str[1];
    int file = file_char - 'a';
    int rank = rank_char - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7)
    {
        return -1;
    }
    return rank * 8 + file;
}

std::string BoardRepresentation::square_to_algebraic(int square) const
{
    int file = square % 8;
    int rank = square / 8;
    std::string algebraic;
    algebraic += ('a' + file);
    algebraic += ('1' + rank);
    return algebraic;
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
