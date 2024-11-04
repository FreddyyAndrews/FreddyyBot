#include "move_generator.h"
#include <iostream>
#include <cctype>

void generate_pawn_move(BoardRepresentation &board_representation, std::vector<Move> &move_list, Square on_square)
{
    int direction = board_representation.white_to_move ? -1 : 1; // Up for white, down for black

    int rank = on_square.rank;
    int file = on_square.file;

    int promotion_rank = board_representation.white_to_move ? 0 : 7;

    // Forward move
    int next_rank = rank + direction;
    if (next_rank >= 0 && next_rank <= 7 && board_representation.board[next_rank][file] == 'e') // if square in front is empty
    {
        if (next_rank == promotion_rank)
        {
            // Generate promotion moves
            char promotion_pieces[] = {'q', 'r', 'b', 'n'}; // Promote to queen, rook, bishop, or knight
            for (char promo_piece : promotion_pieces)
            {
                move_list.push_back(Move(rank, file, next_rank, file, false, true, promo_piece));
            }
        }
        else
        {
            move_list.push_back(Move(rank, file, next_rank, file));

            // Double move from starting position
            bool is_starting_rank = (board_representation.white_to_move && rank == 6) ||
                                    (!board_representation.white_to_move && rank == 1);
            int double_move_rank = rank + 2 * direction;
            if (is_starting_rank && board_representation.board[double_move_rank][file] == 'e' &&
                board_representation.board[next_rank][file] == 'e') // Both squares must be empty
            {
                move_list.push_back(Move(rank, file, double_move_rank, file));
            }
        }
    }

    // Captures (including en passant)
    for (int delta_file = -1; delta_file <= 1; delta_file += 2)
    {
        int capture_file = file + delta_file;
        if (capture_file >= 0 && capture_file < 8)
        {
            int target_rank = rank + direction;
            char target_piece = board_representation.board[target_rank][capture_file];

            // Normal capture
            if (target_piece != 'e' && board_representation.is_opponent_piece(target_piece))
            {
                if (target_rank == promotion_rank)
                {
                    // Promotion capture
                    char promotion_pieces[] = {'q', 'r', 'b', 'n'};
                    for (char promo_piece : promotion_pieces)
                    {
                        move_list.push_back(Move(rank, file, target_rank, capture_file, false, false, promo_piece));
                    }
                }
                else
                {
                    move_list.push_back(Move(rank, file, target_rank, capture_file));
                }
            }
            // En passant capture
            else
            {
                Square &en_passant_square = board_representation.en_passant_square;
                if (en_passant_square.rank == target_rank && en_passant_square.file == capture_file)
                {
                    move_list.push_back(Move(rank, file, target_rank, capture_file, true, false, 'x'));
                }
            }
        }
    }
}

void generate_rook_move(BoardRepresentation &board_representation, std::vector<Move> &move_list, Square on_square)
{
    int rank = on_square.rank;
    int file = on_square.file;

    // Rook movement directions: up, down, left, right
    int directions[4][2] = {
        {-1, 0}, // Down
        {1, 0},  // Up
        {0, -1}, // Left
        {0, 1}   // Right
    };

    // Iterate over each direction
    for (int i = 0; i < 4; ++i)
    {
        int delta_rank = directions[i][0];
        int delta_file = directions[i][1];

        int current_rank = rank + delta_rank;
        int current_file = file + delta_file;

        // Move along the direction until the edge of the board or a blocking piece
        while (current_rank >= 0 && current_rank <= 7 && current_file >= 0 && current_file <= 7)
        {
            char target_piece = board_representation.board[current_rank][current_file];

            if (target_piece == 'e')
            {
                // Empty square, rook can move here
                move_list.push_back(Move(rank, file, current_rank, current_file));
            }
            else if (board_representation.is_opponent_piece(target_piece))
            {
                // Capture opponent piece
                move_list.push_back(Move(rank, file, current_rank, current_file));
                break; // Cannot move past capturing a piece
            }
            else
            {
                // Own piece blocks further movement
                break;
            }

            // Move to the next square in the same direction
            current_rank += delta_rank;
            current_file += delta_file;
        }
    }
}

void generate_bishop_move(BoardRepresentation &board_representation, std::vector<Move> &move_list, Square on_square)
{
    int rank = on_square.rank;
    int file = on_square.file;

    // Bishop movement directions: four diagonals
    int directions[4][2] = {
        {-1, -1}, // Up-left
        {-1, 1},  // Up-right
        {1, -1},  // Down-left
        {1, 1}    // Down-right
    };

    // Iterate over each direction
    for (int i = 0; i < 4; ++i)
    {
        int delta_rank = directions[i][0];
        int delta_file = directions[i][1];

        int current_rank = rank + delta_rank;
        int current_file = file + delta_file;

        // Move along the direction until the edge of the board or a blocking piece
        while (current_rank >= 0 && current_rank <= 7 && current_file >= 0 && current_file <= 7)
        {
            char target_piece = board_representation.board[current_rank][current_file];

            if (target_piece == 'e')
            {
                // Empty square, bishop can move here
                move_list.push_back(Move(rank, file, current_rank, current_file));
            }
            else if (board_representation.is_opponent_piece(target_piece))
            {
                // Capture opponent piece
                move_list.push_back(Move(rank, file, current_rank, current_file));
                break; // Cannot move past capturing a piece
            }
            else
            {
                // Own piece blocks further movement
                break;
            }

            // Move to the next square in the same direction
            current_rank += delta_rank;
            current_file += delta_file;
        }
    }
}

void generate_knight_move(BoardRepresentation &board_representation, std::vector<Move> &move_list, Square on_square)
{
    int rank = on_square.rank;
    int file = on_square.file;

    // Possible knight moves relative to the current position
    int knight_moves[8][2] = {
        {-2, -1}, // 2 up, 1 left
        {-2, 1},  // 2 up, 1 right
        {-1, -2}, // 1 up, 2 left
        {-1, 2},  // 1 up, 2 right
        {1, -2},  // 1 down, 2 left
        {1, 2},   // 1 down, 2 right
        {2, -1},  // 2 down, 1 left
        {2, 1}    // 2 down, 1 right
    };

    // Iterate over all possible knight moves
    for (int i = 0; i < 8; ++i)
    {
        int new_rank = rank + knight_moves[i][0];
        int new_file = file + knight_moves[i][1];

        // Check if the new position is within the board boundaries
        if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
        {
            char target_piece = board_representation.board[new_rank][new_file];

            // Check if the square is empty or contains an opponent's piece
            if (target_piece == 'e' || board_representation.is_opponent_piece(target_piece))
            {
                // Add the move to the move list
                move_list.push_back(Move(rank, file, new_rank, new_file));
            }
            // If it's our own piece, the knight cannot move there
        }
    }
}

void generate_queen_move(BoardRepresentation &board_representation, std::vector<Move> &move_list, Square on_square)
{
    int rank = on_square.rank;
    int file = on_square.file;

    // Queen movement directions: combination of rook and bishop directions
    int directions[8][2] = {
        {-1, 0},  // Up
        {1, 0},   // Down
        {0, -1},  // Left
        {0, 1},   // Right
        {-1, -1}, // Up-left
        {-1, 1},  // Up-right
        {1, -1},  // Down-left
        {1, 1}    // Down-right
    };

    // Iterate over each direction
    for (int i = 0; i < 8; ++i)
    {
        int delta_rank = directions[i][0];
        int delta_file = directions[i][1];

        int current_rank = rank + delta_rank;
        int current_file = file + delta_file;

        // Move along the direction until the edge of the board or a blocking piece
        while (current_rank >= 0 && current_rank <= 7 && current_file >= 0 && current_file <= 7)
        {
            char target_piece = board_representation.board[current_rank][current_file];

            if (target_piece == 'e')
            {
                // Empty square, queen can move here
                move_list.push_back(Move(rank, file, current_rank, current_file));
            }
            else if (board_representation.is_opponent_piece(target_piece))
            {
                // Capture opponent piece
                move_list.push_back(Move(rank, file, current_rank, current_file));
                break; // Cannot move past capturing a piece
            }
            else
            {
                // Own piece blocks further movement
                break;
            }

            // Move to the next square in the same direction
            current_rank += delta_rank;
            current_file += delta_file;
        }
    }
}

void generate_king_move(BoardRepresentation &board_representation, std::vector<Move> &move_list, Square on_square)
{
    int rank = on_square.rank;
    int file = on_square.file;

    // King movement directions: one square in any direction
    int directions[8][2] = {
        {-1, 0},  // Up
        {1, 0},   // Down
        {0, -1},  // Left
        {0, 1},   // Right
        {-1, -1}, // Up-left
        {-1, 1},  // Up-right
        {1, -1},  // Down-left
        {1, 1}    // Down-right
    };

    // Iterate over all possible directions
    for (int i = 0; i < 8; ++i)
    {
        int new_rank = rank + directions[i][0];
        int new_file = file + directions[i][1];

        // Check if the new position is within the board boundaries
        if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
        {
            char target_piece = board_representation.board[new_rank][new_file];

            // Check if the square is empty or contains an opponent's piece
            if (target_piece == 'e' || board_representation.is_opponent_piece(target_piece))
            {
                // Add the move to the move list
                move_list.push_back(Move(rank, file, new_rank, new_file));
            }
            // If it's our own piece, the king cannot move there
        }
    }
}

void generate_castle(BoardRepresentation &board_representation, std::vector<Move> &move_list)
{
    // Determine the side to move
    bool is_white = board_representation.white_to_move;

    // King's starting position
    int king_rank = is_white ? 7 : 0;
    int king_file = 4; // 'e' file

    // Castling rights and conditions
    if (is_white)
    {
        // White's kingside castling
        if (board_representation.white_can_castle_kingside)
        {
            // Check if squares between king and rook are empty
            if (board_representation.board[king_rank][5] == 'e' &&
                board_representation.board[king_rank][6] == 'e')
            {
                // Create the castling move (king moves from e1 to g1)
                move_list.push_back(Move(king_rank, king_file, king_rank, 6, false, true));
            }
        }

        // White's queenside castling
        if (board_representation.white_can_castle_queenside)
        {
            // Check if squares between king and rook are empty
            if (board_representation.board[king_rank][3] == 'e' &&
                board_representation.board[king_rank][2] == 'e' &&
                board_representation.board[king_rank][1] == 'e')
            {
                // Create the castling move (king moves from e1 to c1)
                move_list.push_back(Move(king_rank, king_file, king_rank, 2, false, true));
            }
        }
    }
    else
    {
        // Black's kingside castling
        if (board_representation.black_can_castle_kingside)
        {
            // Check if squares between king and rook are empty
            if (board_representation.board[king_rank][5] == 'e' &&
                board_representation.board[king_rank][6] == 'e')
            {
                // Create the castling move (king moves from e8 to g8)
                move_list.push_back(Move(king_rank, king_file, king_rank, 6, false, true));
            }
        }

        // Black's queenside castling
        if (board_representation.black_can_castle_queenside)
        {
            // Check if squares between king and rook are empty
            if (board_representation.board[king_rank][3] == 'e' &&
                board_representation.board[king_rank][2] == 'e' &&
                board_representation.board[king_rank][1] == 'e')
            {
                // Create the castling move (king moves from e8 to c8)
                move_list.push_back(Move(king_rank, king_file, king_rank, 2, false, true));
            }
        }
    }
}

void generate_pseudo_legal_moves(BoardRepresentation &board_representation, std::vector<Move> &move_list)
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            char piece = board_representation.board[i][j];

            // Skip empty squares
            if (piece == 'e')
                continue;

            // Check if the piece belongs to the player whose turn it is
            bool is_white_piece = isupper(piece);
            if ((board_representation.white_to_move && !is_white_piece) ||
                (!board_representation.white_to_move && is_white_piece))
            {
                continue;
            }

            // Convert piece to lowercase for uniformity in switch
            char piece_type = tolower(piece, std::locale());

            // Call the appropriate move generator based on the piece type
            switch (piece_type)
            {
            case 'p': // Pawn
                generate_pawn_move(board_representation, move_list, Square(i, j));
                break;
            case 'n': // Knight
                generate_knight_move(board_representation, move_list, Square(i, j));
                break;
            case 'b': // Bishop
                generate_bishop_move(board_representation, move_list, Square(i, j));
                break;
            case 'r': // Rook
                generate_rook_move(board_representation, move_list, Square(i, j));
                break;
            case 'q': // Queen
                generate_queen_move(board_representation, move_list, Square(i, j));
                break;
            case 'k': // King
                generate_king_move(board_representation, move_list, Square(i, j));
                break;
            }
        }
    }

    generate_castle(board_representation, move_list);
}

int generate_legal_moves(BoardRepresentation &board_representation, std::vector<Move> &move_list)
{
    std::vector<Move> pseudo_legal_move_list;
    generate_pseudo_legal_moves(board_representation, pseudo_legal_move_list);

    for (Move move : pseudo_legal_move_list)
    {
        // For each generated move make the move in internal memory and find all opponent responses
        std::vector<Move> opp_move_list;
        // Make move handles changing turns so the next generated move will be the opponent
        board_representation.make_move(move);
        generate_pseudo_legal_moves(board_representation, opp_move_list);

        // Assume move is legal until finding counter example
        bool move_is_legal = true;

        // iterate through possible opponent responses
        for (Move opp_move : opp_move_list)
        {
            if (board_representation.move_captures_king(opp_move))
            {
                // Move is not legal if it allows your own king to be captured
                move_is_legal = false;
                break;
            }

            // Additional condition for castling moves
            if (move.is_castle)
            {
                // Determine the squares the king passes through during castling
                std::vector<Square> squares_to_check;
                int rank = move.start_square.rank;
                int to_file = move.to_square.file;

                if (to_file == 6) // Kingside castling
                {
                    // King landing square is already checked, check the squares in between
                    squares_to_check.push_back(Square(rank, 4));
                    squares_to_check.push_back(Square(rank, 5));
                }
                else if (to_file == 2) // Queenside castling
                {
                    // King landing square is already checked, check the squares in between
                    squares_to_check.push_back(Square(rank, 4));
                    squares_to_check.push_back(Square(rank, 3));
                }

                // Check if any of the squares the king passes through are attacked by the opponent
                for (Square sq : squares_to_check)
                {
                    if (opp_move.to_square == sq)
                    {
                        // Move is not legal if the king passes through or lands on a square under attack
                        move_is_legal = false;
                        break;
                    }
                }

                if (!move_is_legal)
                {
                    break;
                }
            }
        }

        if (move_is_legal)
        {
            move_list.push_back(move);
        }
        // Undo the generated move
        board_representation.undo_move(move);
    }

    return static_cast<int>(move_list.size());
}
