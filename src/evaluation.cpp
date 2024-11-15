#include "evaluation.h"

Evaluation find_best_move(BoardRepresentation &board_representation)
{
    Evaluation position_evaluation = Evaluation();
    search(position_evaluation, board_representation, max_depth, -INT_MAX, INT_MAX);
    return position_evaluation;
}

int search(Evaluation &position_evaluation, BoardRepresentation &board_representation, int depth, int alpha, int beta)
{
    if (depth == 0)
    {
        return evaluate(board_representation);
    }

    std::vector<Move> move_list;
    // populate move list
    generate_legal_moves(board_representation, move_list);

    if (move_list.empty())
    {
        if (board_representation.is_in_check)
        {
            // closer mates are worth more
            return -(INT_MAX - depth);
        }
        else
        {
            return 0;
        }
    }

    sort_for_pruning(move_list, board_representation);

    for (Move move : move_list)
    {
        board_representation.make_move(move);
        int evaluation = -search(position_evaluation, board_representation, depth - 1, -alpha, -beta);
        board_representation.undo_move(move);

        if (evaluation >= beta)
        {
            return beta;
        }

        if (evaluation > alpha)
        {
            alpha = evaluation;

            // on top recursion, find best move
            if (depth == max_depth)
            {
                position_evaluation.best_move = move;
                position_evaluation.evaluation = alpha;
            }
        }
    }

    return alpha;
}

void sort_for_pruning(std::vector<Move> &move_list, BoardRepresentation &board_representation)
{
    std::sort(move_list.begin(), move_list.end(), [&board_representation](const Move &a, const Move &b)
              {
        int value_a = std::max(get_piece_value(board_representation.board[a.to_square.rank][a.to_square.file]) -
                      get_piece_value(board_representation.board[a.start_square.rank][a.start_square.file]), 0);
        int value_b = std::max(get_piece_value(board_representation.board[b.to_square.rank][b.to_square.file]) -
                      get_piece_value(board_representation.board[b.start_square.rank][b.start_square.file]), 0);
        return value_a > value_b; });
}

int get_piece_value(char piece)
{
    char piece_type = to_lower(piece);

    switch (piece_type)
    {
    case 'p': // Pawn
        return 1;

    case 'n': // Knight
        return 3;

    case 'b': // Bishop
        return 3;

    case 'r': // Rook
        return 5;

    case 'q': // Queen
        return 9;

    default:
        return 0;
    }
}

int evaluate(BoardRepresentation &board_representation)
{
    int eval = 0;
    for (Square square : board_representation.non_empty_squares)
    {
        char piece = board_representation.board[square.rank][square.file];
        int eval_modifier = board_representation.is_opponent_piece(piece) ? -1 : 1;
        eval += get_piece_value(piece) * eval_modifier;
    }

    return eval;
}

// Methods for specific game states
bool is_checkmate(const BoardRepresentation &)
{
    // Dummy method (always return false)
    return false;
}

bool is_stalemate(const BoardRepresentation &)
{
    // Dummy method (always return false)
    return false;
}

bool is_insufficient_material(const BoardRepresentation &)
{
    // Dummy method (always return false)
    return false;
}

bool is_draw_by_repetition(const BoardRepresentation &)
{
    // Dummy method (always return false)
    return false;
}

bool is_draw_by_fifty_moves(const BoardRepresentation &)
{
    // Dummy method (always return false)
    return false;
}