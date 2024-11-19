#include "move_generator.h"

void generate_pawn_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares)
{
    int8_t rank = on_square.rank;
    int8_t file = on_square.file;

    int8_t direction = board_representation.white_to_move ? 1 : -1; // Up for white, down for black

    // if opponents piece determine attacked squares
    if (board_representation.is_opponent_piece(board_representation.board[rank][file]))
    {
        if (file + 1 <= 7)
        {
            attacked_squares.push_back(SquareToSquareMap(Square(rank - direction, file + 1), Square(rank, file)));
        }
        if (file - 1 >= 0)
        {
            attacked_squares.push_back(SquareToSquareMap(Square(rank - direction, file - 1), Square(rank, file)));
        }

        return; // don't generate any real moves if not turn
    }

    int8_t promotion_rank = board_representation.white_to_move ? 7 : 0;

    // Forward move
    int8_t next_rank = rank + direction;
    if (next_rank >= 0 && next_rank <= 7 && board_representation.board[next_rank][file] == 'e') // if square in front is empty
    {
        if (next_rank == promotion_rank)
        {
            // Generate promotion moves
            char promotion_pieces[] = {'q', 'r', 'b', 'n'}; // Promote to queen, rook, bishop, or knight
            for (char promo_piece : promotion_pieces)
            {
                move_list.push_back(Move(on_square, Square(next_rank, file), false, false, promo_piece));
            }
        }
        else
        {
            move_list.push_back(Move(on_square, Square(next_rank, file)));

            // Double move from starting position
            bool is_starting_rank = (board_representation.white_to_move && rank == 1) ||
                                    (!board_representation.white_to_move && rank == 6);
            int8_t double_move_rank = static_cast<int8_t>(rank + 2 * direction);
            if (is_starting_rank && board_representation.board[double_move_rank][file] == 'e' &&
                board_representation.board[next_rank][file] == 'e') // Both squares must be empty
            {
                move_list.push_back(Move(on_square, Square(double_move_rank, file)));
            }
        }
    }

    // Captures (including en passant)
    for (int8_t delta_file = -1; delta_file <= 1; delta_file += 2)
    {
        int8_t capture_file = file + delta_file;
        if (capture_file >= 0 && capture_file < 8)
        {
            int8_t target_rank = rank + direction;
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
                        move_list.push_back(Move(on_square, Square(target_rank, capture_file), false, false, promo_piece));
                    }
                }
                else
                {
                    move_list.push_back(Move(on_square, Square(target_rank, capture_file)));
                }
            }
            // En passant capture
            else
            {
                Square &en_passant_square = board_representation.en_passant_square;
                if (en_passant_square.rank == target_rank && en_passant_square.file == capture_file)
                {
                    move_list.push_back(Move(on_square, Square(target_rank, capture_file), true, false, 'x'));
                }
            }
        }
    }
}

void generate_rook_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares)

{
    int8_t rank = on_square.rank;
    int8_t file = on_square.file;

    // Rook movement directions: up, down, left, right
    constexpr int8_t directions[4][2] = {
        {-1, 0}, // Down
        {1, 0},  // Up
        {0, -1}, // Left
        {0, 1}   // Right
    };

    bool generating_attacked_squares = board_representation.is_opponent_piece(board_representation.board[rank][file]);

    // Iterate over each direction
    for (int8_t i = 0; i < 4; ++i)
    {
        int8_t delta_rank = directions[i][0];
        int8_t delta_file = directions[i][1];

        int8_t current_rank = rank + delta_rank;
        int8_t current_file = file + delta_file;

        // Move along the direction until the edge of the board or a blocking piece
        while (current_rank >= 0 && current_rank <= 7 && current_file >= 0 && current_file <= 7)
        {
            char target_piece = board_representation.board[current_rank][current_file];

            if (target_piece == 'e')
            {
                if (!generating_attacked_squares)
                {
                    // Empty square, rook can move here
                    move_list.push_back(Move(on_square, Square(current_rank, current_file)));
                }
                else
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
            }
            else if (board_representation.is_opponent_piece(target_piece))
            {
                if (!generating_attacked_squares)
                {
                    // Empty square, rook can move here
                    move_list.push_back(Move(on_square, Square(current_rank, current_file)));
                }
                else
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
                break; // Cannot move past capturing a piece
            }
            else
            {
                if (generating_attacked_squares)
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
                // Own piece blocks further movement
                break;
            }

            // Move to the next square in the same direction
            current_rank += delta_rank;
            current_file += delta_file;
        }
    }
}

void generate_bishop_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares)
{
    int8_t rank = on_square.rank;
    int8_t file = on_square.file;

    bool generating_attacked_squares = board_representation.is_opponent_piece(board_representation.board[rank][file]);

    // Bishop movement directions: four diagonals
    constexpr int8_t directions[4][2] = {
        {-1, -1}, // Up-left
        {-1, 1},  // Up-right
        {1, -1},  // Down-left
        {1, 1}    // Down-right
    };

    // Iterate over each direction
    for (int8_t i = 0; i < 4; ++i)
    {
        int8_t delta_rank = directions[i][0];
        int8_t delta_file = directions[i][1];

        int8_t current_rank = rank + delta_rank;
        int8_t current_file = file + delta_file;

        // Move along the direction until the edge of the board or a blocking piece
        while (current_rank >= 0 && current_rank <= 7 && current_file >= 0 && current_file <= 7)
        {
            char target_piece = board_representation.board[current_rank][current_file];

            if (target_piece == 'e')
            {
                if (!generating_attacked_squares)
                {
                    // Empty square, rook can move here
                    move_list.push_back(Move(on_square, Square(current_rank, current_file)));
                }
                else
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
            }
            else if (board_representation.is_opponent_piece(target_piece))
            {
                if (!generating_attacked_squares)
                {
                    // Empty square, rook can move here
                    move_list.push_back(Move(on_square, Square(current_rank, current_file)));
                }
                else
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
                break; // Cannot move past capturing a piece
            }
            else
            {
                if (generating_attacked_squares)
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
                // Own piece blocks further movement
                break;
            }

            // Move to the next square in the same direction
            current_rank += delta_rank;
            current_file += delta_file;
        }
    }
}

void generate_knight_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares)
{
    int8_t rank = on_square.rank;
    int8_t file = on_square.file;

    // Possible knight moves relative to the current position
    constexpr int8_t knight_moves[8][2] = {
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
    for (int8_t i = 0; i < 8; ++i)
    {
        int8_t new_rank = rank + knight_moves[i][0];
        int8_t new_file = file + knight_moves[i][1];

        // Check if the new position is within the board boundaries
        if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
        {
            char target_piece = board_representation.board[new_rank][new_file];

            // generate attacked squares
            if (board_representation.is_opponent_piece(board_representation.board[rank][file]))
            {
                attacked_squares.push_back(SquareToSquareMap(Square(new_rank, new_file), Square(rank, file)));
            }
            // Check if the square is empty or contains an opponent's piece
            else if (target_piece == 'e' || board_representation.is_opponent_piece(target_piece))
            {
                // Add the move to the move list
                move_list.push_back(Move(on_square, Square(new_rank, new_file)));
            }
            // If it's our own piece, the knight cannot move there
        }
    }
}

void generate_queen_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares)
{
    int8_t rank = on_square.rank;
    int8_t file = on_square.file;

    bool generating_attacked_squares = board_representation.is_opponent_piece(board_representation.board[rank][file]);

    // Queen movement directions: combination of rook and bishop directions
    constexpr int8_t directions[8][2] = {
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
    for (int8_t i = 0; i < 8; ++i)
    {
        int8_t delta_rank = directions[i][0];
        int8_t delta_file = directions[i][1];

        int8_t current_rank = rank + delta_rank;
        int8_t current_file = file + delta_file;

        // Move along the direction until the edge of the board or a blocking piece
        while (current_rank >= 0 && current_rank <= 7 && current_file >= 0 && current_file <= 7)
        {
            char target_piece = board_representation.board[current_rank][current_file];

            if (target_piece == 'e')
            {
                if (!generating_attacked_squares)
                {
                    // Empty square, rook can move here
                    move_list.push_back(Move(on_square, Square(current_rank, current_file)));
                }
                else
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
            }
            else if (board_representation.is_opponent_piece(target_piece))
            {
                if (!generating_attacked_squares)
                {
                    // Empty square, rook can move here
                    move_list.push_back(Move(on_square, Square(current_rank, current_file)));
                }
                else
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
                break; // Cannot move past capturing a piece
            }
            else
            {
                if (generating_attacked_squares)
                {
                    attacked_squares.push_back(SquareToSquareMap(Square(current_rank, current_file), Square(rank, file)));
                }
                // Own piece blocks further movement
                break;
            }

            // Move to the next square in the same direction
            current_rank += delta_rank;
            current_file += delta_file;
        }
    }
}

void generate_king_move(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    const Square &on_square,
    std::vector<SquareToSquareMap> &attacked_squares)
{
    int8_t rank = on_square.rank;
    int8_t file = on_square.file;

    // King movement directions: one square in any direction
    constexpr int8_t directions[8][2] = {
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
    for (int8_t i = 0; i < 8; ++i)
    {
        int8_t new_rank = rank + directions[i][0];
        int8_t new_file = file + directions[i][1];

        // Check if the new position is within the board boundaries
        if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
        {
            char target_piece = board_representation.board[new_rank][new_file];

            // generate attacked squares
            if (board_representation.is_opponent_piece(board_representation.board[rank][file]))
            {
                attacked_squares.push_back(SquareToSquareMap(Square(new_rank, new_file), Square(rank, file)));
            }
            // Check if the square is empty or contains an opponent's piece
            else if (target_piece == 'e' || board_representation.is_opponent_piece(target_piece))
            {
                // Add the move to the move list
                move_list.push_back(Move(on_square, Square(new_rank, new_file)));
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
    int8_t king_rank = is_white ? 0 : 7;
    int8_t king_file = 4; // 'e' file

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
                move_list.push_back(Move(Square(king_rank, king_file), Square(king_rank, 6), false, true));
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
                move_list.push_back(Move(Square(king_rank, king_file), Square(king_rank, 2), false, true));
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
                move_list.push_back(Move(Square(king_rank, king_file), Square(king_rank, 6), false, true));
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
                move_list.push_back(Move(Square(king_rank, king_file), Square(king_rank, 2), false, true));
            }
        }
    }
}

void generate_pseudo_legal_moves(
    BoardRepresentation &board_representation,
    std::vector<Move> &move_list,
    std::vector<SquareToSquareMap> &attacked_squares,
    Square &king_position)
{
    for (const Square &current_square : board_representation.non_empty_squares)
    {

        char piece = board_representation.board[current_square.rank][current_square.file];

        // Convert piece to lowercase for uniformity in switch
        char piece_type = to_lower(piece);

        if (piece_type == 'e')
        {
            throw std::runtime_error("Empty square in non-empty squares.");
        }

        // Call the appropriate move generator based on the piece type
        switch (piece_type)
        {
        case 'p': // Pawn
            generate_pawn_move(
                board_representation,
                move_list,
                current_square,
                attacked_squares);
            break;

        case 'n': // Knight
            generate_knight_move(
                board_representation,
                move_list,
                current_square,
                attacked_squares);
            break;

        case 'b': // Bishop
            generate_bishop_move(
                board_representation,
                move_list,
                current_square,
                attacked_squares);
            break;

        case 'r': // Rook
            generate_rook_move(
                board_representation,
                move_list,
                current_square,
                attacked_squares);
            break;

        case 'q': // Queen
            generate_queen_move(
                board_representation,
                move_list,
                current_square,
                attacked_squares);
            break;

        case 'k': // King
            if (!board_representation.is_opponent_piece(board_representation.board[current_square.rank][current_square.file]))
            {
                // store friendly king location
                king_position = current_square;
            }
            generate_king_move(
                board_representation,
                move_list,
                current_square,
                attacked_squares);
            break;
        }
    }

    generate_castle(board_representation, move_list);
}

// Function to count occurrences of a Square in the 'attacked' field
bool is_square_attacked(const std::vector<SquareToSquareMap> &maps, const Square &target_square)
{
    return std::any_of(maps.begin(), maps.end(), [&](const SquareToSquareMap &map)
                       { return map.attacked == target_square; });
}

int count_square_attacks(const std::vector<SquareToSquareMap> &maps, const Square &target_square)
{
    return static_cast<int8_t>(std::count_if(maps.begin(), maps.end(), [&](const SquareToSquareMap &map)
                                             { return map.attacked == target_square; }));
}

Square get_attacker(const std::vector<SquareToSquareMap> &maps, const Square &attacked_square)
{
    for (const SquareToSquareMap &map : maps)
    {
        if (map.attacked == attacked_square)
        {
            return map.attacker;
        }
    }

    throw std::runtime_error("Square not under attack.");
}

std::vector<Square> get_all_attackers(const std::vector<SquareToSquareMap> &maps, const Square &attacked_square)
{
    std::vector<Square> attackers;

    for (const SquareToSquareMap &map : maps)
    {
        if (map.attacked == attacked_square)
        {
            attackers.push_back(map.attacker);
        }
    }

    return attackers;
}

u64 generate_legal_moves(BoardRepresentation &board_representation, std::vector<Move> &move_list, bool only_captures)
{
    // illegal cases
    // 1. Castling through check
    // 2. Leaving king in check.
    // 3. Moving a pinned piece revealing the king
    // 4. Moving king into check.

    std::vector<Move> pseudo_legal_move_list;
    std::vector<SquareToSquareMap> attacked_squares;
    Square king_position;

    generate_pseudo_legal_moves(board_representation, pseudo_legal_move_list, attacked_squares, king_position);

    // assert(king_position.exists());
    // assert(attacked_squares.size() > 0);

    for (const Move &move : pseudo_legal_move_list)
    {
        // check attacked squares to make sure castle is legal
        if (move.is_castle)
        {
            // check white kingside castle
            if (board_representation.white_to_move && move.to_square.file == 6)
            {
                if (is_square_attacked(attacked_squares, Square(0, 4)) > 0 || // King's starting square
                    is_square_attacked(attacked_squares, Square(0, 5)) > 0 || // Square the king moves through
                    is_square_attacked(attacked_squares, Square(0, 6)) > 0)   // Square the king ends on
                {
                    continue; // Castle is not legal
                }
            }
            // check white queenside castle
            else if (board_representation.white_to_move && move.to_square.file == 2)
            {
                if (is_square_attacked(attacked_squares, Square(0, 4)) > 0 || // King's starting square
                    is_square_attacked(attacked_squares, Square(0, 3)) > 0 || // Square the king moves through
                    is_square_attacked(attacked_squares, Square(0, 2)) > 0)   // Square the king ends on
                {
                    continue; // Castle is not legal
                }
            }
            // check black kingside castle
            else if (!board_representation.white_to_move && move.to_square.file == 6)
            {
                if (is_square_attacked(attacked_squares, Square(7, 4)) > 0 || // King's starting square
                    is_square_attacked(attacked_squares, Square(7, 5)) > 0 || // Square the king moves through
                    is_square_attacked(attacked_squares, Square(7, 6)) > 0)   // Square the king ends on
                {
                    continue; // Castle is not legal
                }
            }
            // check black queenside castle
            else if (!board_representation.white_to_move && move.to_square.file == 2)
            {
                if (is_square_attacked(attacked_squares, Square(7, 4)) > 0 || // King's starting square
                    is_square_attacked(attacked_squares, Square(7, 3)) > 0 || // Square the king moves through
                    is_square_attacked(attacked_squares, Square(7, 2)) > 0)   // Square the king ends on
                {
                    continue; // Castle is not legal
                }
            }
        }
        else // different checks for none castling moves
        {
            // check king move is not moving onto attacked square
            if (move.start_square == king_position)
            {
                if (is_square_attacked(attacked_squares, move.to_square) > 0)
                {
                    // move is not legal, look at the next move
                    continue;
                }

                std::vector<Square> attacker_squares = get_all_attackers(attacked_squares, king_position);
                bool king_moves_along_checked_line = false;

                for (const Square &attacker_square : attacker_squares)
                {
                    char piece = to_lower(board_representation.board[attacker_square.rank][attacker_square.file]);

                    if (piece == 'q' || piece == 'b' || piece == 'r')
                    {
                        if (move.start_square.is_between(attacker_square, move.to_square))
                        {
                            king_moves_along_checked_line = true;
                            break;
                        }
                    }
                }

                if (king_moves_along_checked_line)
                    continue;
            }
            // if king is in check and the move is not a king move (checked in above condition)
            else if (is_square_attacked(attacked_squares, king_position) > 0)
            {
                // these moves must either block or capture the attacking piece or else they are illegal

                // Double check requires a king move
                if (count_square_attacks(attacked_squares, king_position) > 1)
                {
                    continue;
                }

                Square attacker_square = get_attacker(attacked_squares, king_position);
                char attacker = to_lower(board_representation.board[attacker_square.rank][attacker_square.file]);

                // if we are not capturing the checking piece we must block the checking piece
                if (!(move.to_square == attacker_square) && !move.is_enpassant)
                {
                    // this check only applies to sliding attacker pieces
                    // Therefor if the attacker is not a queen, rook, or bishop the move is not valid
                    if (attacker != 'q' && attacker != 'b' && attacker != 'r')
                    {
                        continue;
                    }

                    // check if piece is moving between king and attacker
                    if (!move.to_square.is_between(attacker_square, king_position))
                    {
                        // If not check the next move
                        continue;
                    }
                }
                else if (move.is_enpassant)
                {
                    // if we are in check and doing en passant we must be capturing the attacking square
                    int rank_offset = board_representation.white_to_move ? 1 : -1;
                    if (!(attacker_square.file == board_representation.en_passant_square.file &&
                          attacker_square.rank + rank_offset == board_representation.en_passant_square.rank))
                    {
                        continue;
                    }
                }
            }

            // Handle pinned pieces
            // check if piece is attacked
            if (is_square_attacked(attacked_squares, move.start_square))
            {
                bool respects_pin = true;
                // If attacked check the pieces that are attacking it
                // iterate through attacker squares for the attacked piece
                for (const SquareToSquareMap &map : attacked_squares)
                {
                    if (map.attacked == move.start_square)
                    {
                        // if being attacked by sliding pieces we need to check if this piece is the only piece between attacker and king
                        // if the piece is the only piece between attacker and king the only legal moves are between the attacker and king
                        // or capturing the attacker
                        char piece = to_lower(board_representation.board[map.attacker.rank][map.attacker.file]);
                        if (piece == 'q' || piece == 'r' || piece == 'b')
                        {
                            // check if moving piece is only occupied square between attacker and king
                            if (board_representation.is_only_between(map.attacker, king_position, move.start_square))
                            {
                                if (!move.to_square.is_between(map.attacker, king_position) && !(move.to_square == map.attacker))
                                {
                                    respects_pin = false;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (!respects_pin)
                    continue;
            }

            // special pin handling for en passant
            // special pin handling for en passant
            if (move.is_enpassant)
            {
                bool en_passant_valid = true;
                // check if en passant might open king rank
                if (move.start_square.rank == king_position.rank)
                {
                    int direction = (king_position.file - move.start_square.file > 0) ? -1 : 1;
                    int begin_iteration_file = (direction == 1) ? king_position.file + 1 : king_position.file - 1;

                    // Generalized loop for both directions
                    for (int i = begin_iteration_file; i >= 0 && i < 8; i += direction)
                    {
                        if (i == move.start_square.file || i == board_representation.en_passant_square.file)
                        {
                            continue;
                        }

                        if (board_representation.board[move.start_square.rank][i] == 'e')
                        {
                            continue;
                        }

                        // if we find a friendly piece, the en passant is valid
                        if (!board_representation.is_opponent_piece(board_representation.board[move.start_square.rank][i]))
                        {
                            break;
                        }

                        char opponent_piece = to_lower(board_representation.board[move.start_square.rank][i]);
                        if (opponent_piece == 'r' || opponent_piece == 'q')
                        {
                            en_passant_valid = false;
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                if (!en_passant_valid)
                {
                    continue;
                }
            }
        }

        if (board_representation.move_captures_king(move))
        {
            std::cout << "Move: " << move.to_UCI() << " captured the king.";
            throw std::runtime_error("king was captured.");
        }

        // if the move has not been rejected to this point it is valid
        // In only captures case check this condition
        if (only_captures)
        {
            if (board_representation.board[move.to_square.rank][move.to_square.file] != 'e')
            {
                move_list.push_back(move);
            }
        }
        else
        {
            move_list.push_back(move);
        }
    }

    if (is_square_attacked(attacked_squares, king_position))
    {
        board_representation.is_in_check = true;
    }
    else
    {
        board_representation.is_in_check = false;
    }

    return static_cast<u64>(move_list.size());
}