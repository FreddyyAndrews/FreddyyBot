#include "move_generator.h"
#include <iostream>
#include <cctype>

int generate_pawn_move(BoardRepresentation &board_representation, Move *move_list, Square on_square)
{
    board_representation.print_board();
    std::cout << move_list;
    return 0;
}

int generate_rook_move(BoardRepresentation &board_representation, Move *move_list, Square on_square)
{
    board_representation.print_board();
    std::cout << move_list;
    return 0;
}

int generate_bishop_move(BoardRepresentation &board_representation, Move *move_list, Square on_square)
{
    board_representation.print_board();
    std::cout << move_list;
    return 0;
}

int generate_knight_move(BoardRepresentation &board_representation, Move *move_list, Square on_square)
{
    board_representation.print_board();
    std::cout << move_list;
    return 0;
}

int generate_queen_move(BoardRepresentation &board_representation, Move *move_list, Square on_square)
{
    board_representation.print_board();
    std::cout << move_list;
    return 0;
}

int generate_king_move(BoardRepresentation &board_representation, Move *move_list, Square on_square)
{
    board_representation.print_board();
    std::cout << move_list;
    return 0;
}

int generate_castle(BoardRepresentation &board_representation, Move *move_list)
{
}

int generate_pseudo_legal_moves(BoardRepresentation &board_representation, Move *move_list)
{
    int move_count = 0; // Track the number of moves generated

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
            char piece_type = tolower(piece);

            // Call the appropriate move generator based on the piece type
            switch (piece_type)
            {
            case 'p': // Pawn
                move_count += generate_pawn_move(board_representation, move_list, Square(i, j));
                break;
            case 'n': // Knight
                move_count += generate_knight_move(board_representation, move_list, Square(i, j));
                break;
            case 'b': // Bishop
                move_count += generate_bishop_move(board_representation, move_list, Square(i, j));
                break;
            case 'r': // Rook
                move_count += generate_rook_move(board_representation, move_list, Square(i, j));
                break;
            case 'q': // Queen
                move_count += generate_queen_move(board_representation, move_list, Square(i, j));
                break;
            case 'k': // King
                move_count += generate_king_move(board_representation, move_list, Square(i, j));
                break;
            }
        }
    }

    move_count += generate_castle(board_representation, move_list);

    return move_count;
}

int generate_legal_moves(BoardRepresentation &board_representation, Move *move_list)
{
    Move pseudo_legal_move_list[256];
    int n_pseudo_legal_moves = generate_pseudo_legal_moves(board_representation, pseudo_legal_move_list);
    int legal_moves_count = 0;

    for (int i = 0; i < n_pseudo_legal_moves; ++i)
    {
        // For each generated move make the move in internal memory and find all opponent responses
        Move opp_move_list[218];
        // Make move handles changing turns so the next generated move will be the opponent
        board_representation.make_move(pseudo_legal_move_list[i]);
        int n_opp_moves = generate_pseudo_legal_moves(board_representation, opp_move_list);

        // Assume move is legal until finding counter example
        bool move_is_legal = true;

        // iterate through possible opponent responses
        for (int j = 0; j < n_opp_moves; j++)
        {
            if (board_representation.move_captures_king(opp_move_list[j]))
            {
                // Move is not legal if it allows your own king to be captured
                move_is_legal = false;
                break;
            }
        }

        if (move_is_legal)
        {
            move_list[legal_moves_count] = pseudo_legal_move_list[i];
            legal_moves_count++;
        }
        // Undo the generated move
        board_representation.undo_move(pseudo_legal_move_list[i]);
    }

    return legal_moves_count;
}
