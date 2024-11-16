#include "evaluation.h"

Evaluation find_best_move(BoardRepresentation &board_representation, int wtime, int btime, int winc, int binc)
{
    Evaluation position_evaluation = Evaluation();
    int i = 1;
    auto start_time = std::chrono::steady_clock::now();
    bool is_endgame_condition = is_endgame(board_representation);
    std::chrono::time_point<std::chrono::steady_clock> cutoff_time = find_time_condition(
        is_endgame_condition, wtime, btime, winc, binc, board_representation.white_to_move);

    // Calculate allocated time in milliseconds
    auto allocated_time = std::chrono::duration_cast<std::chrono::milliseconds>(cutoff_time - start_time).count();
    std::cout << "Allocated " << allocated_time << " milliseconds" << std::endl;

    do
    {
        search(position_evaluation, board_representation, 0, -INT_MAX, INT_MAX, i,
               is_endgame_condition, cutoff_time);

        // Calculate elapsed time since start_time
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        std::cout << "Reached depth " << i << " in " << elapsed_time << " milliseconds" << std::endl;

        ++i;
    } while (std::chrono::steady_clock::now() <= cutoff_time);

    return position_evaluation;
}

std::chrono::time_point<std::chrono::steady_clock> find_time_condition(bool is_endgame_condition, int wtime, int btime,
                                                                       int winc, int binc, bool is_white_to_move)
{
    int remaining_time = is_white_to_move ? wtime : btime;
    int increment = is_white_to_move ? winc : binc;

    // Determine the number of moves left; assume average game length
    int moves_left = is_endgame_condition ? 20 : 40;

    // Basic time allocation per move
    int time_per_move = remaining_time / moves_left;

    // Adjust time per move based on increment
    time_per_move += increment;

    // Ensure we don't use all the remaining time
    time_per_move = std::min(time_per_move, remaining_time - MOVE_TIME_SAFETY_BUFFER_MS);

    // Set minimum and maximum thinking time
    int min_time = 10;    // milliseconds
    int max_time = 15000; // milliseconds

    // Clamp the time_per_move within min and max limits
    time_per_move = std::max(min_time, std::min(time_per_move, max_time));

    // Calculate the cutoff time as a time point in the future
    auto start_time = std::chrono::steady_clock::now();
    return start_time + std::chrono::milliseconds(time_per_move);
}

bool is_endgame(BoardRepresentation &board_representation)
{
    int material_count = 0;

    for (const Square &square : board_representation.non_empty_squares)
    {
        char piece = board_representation.board[square.rank][square.file];
        int piece_value = get_piece_value(piece);

        // Exclude kings from the material count
        if (tolower(piece) != 'k')
        {
            material_count += piece_value;
        }
    }

    return material_count <= ENDGAME_MATERIAL_CONDITION;
}

int search(Evaluation &position_evaluation, BoardRepresentation &board_representation,
           const int depth, int alpha, int beta, const int max_depth, const bool is_endgame_condition,
           const std::chrono::time_point<std::chrono::steady_clock> &cutoff_time)
{
    if (depth == max_depth)
    {
        return search_captures(board_representation, alpha, beta, is_endgame_condition);
    }

    std::vector<Move> move_list;
    // populate move list
    generate_legal_moves(board_representation, move_list);

    if (move_list.empty())
    {
        if (board_representation.is_in_check)
        {
            // closer mates are worth more
            return -(MATE_SCORE - depth);
        }
        else
        {
            return 0;
        }
    }

    // if we are on the top recursive call and not on the first search in iterative deepening we need to sort
    // the moves such that the best move from the previous iterative deepening is checked first
    // this value is stored in position_evaluation.best_move
    if (depth == 0 && max_depth != 1)
    {
        if (!position_evaluation.best_move.is_instantiated())
        {
            throw std::runtime_error("Best move from previous iteration not instantiated.");
        }

        sort_for_pruning(move_list, board_representation, position_evaluation.best_move);
    }
    else
    {
        sort_for_pruning(move_list, board_representation);
    }

    for (const Move &move : move_list)
    {
        board_representation.make_move(move);
        int evaluation = -search(position_evaluation, board_representation, depth + 1, -beta,
                                 -alpha, max_depth, is_endgame_condition, cutoff_time);
        board_representation.undo_move(move);

        if (evaluation >= beta)
        {
            return beta;
        }

        if (evaluation > alpha)
        {
            alpha = evaluation;

            // on top recursion, find best move
            if (depth == 0)
            {
                position_evaluation.best_move = move;
                position_evaluation.evaluation = alpha;
            }
        }

        // if we are timed out stop evaluating
        // this doesn't count on first iterative deepening call, we always need to complete that
        if (std::chrono::steady_clock::now() > cutoff_time && depth == 0 && max_depth != 1)
        {
            std::cout << "Timed out at " << max_depth << std::endl;
            return alpha;
        }
    }

    return alpha;
}

int search_captures(BoardRepresentation &board_representation, int alpha, int beta, bool endgame_condition)
{
    int evaluation = evaluate(board_representation, endgame_condition);
    if (evaluation >= beta)
    {
        return beta;
    }

    alpha = std::max(alpha, evaluation);
    std::vector<Move> capture_moves;
    generate_legal_moves(board_representation, capture_moves, true);
    sort_for_pruning(capture_moves, board_representation);

    for (const Move &move : capture_moves)
    {
        board_representation.make_move(move);
        evaluation = -search_captures(board_representation, -beta, -alpha, endgame_condition);
        board_representation.undo_move(move);

        if (evaluation >= beta)
        {
            return beta;
        }
        alpha = std::max(alpha, evaluation);
    }

    return alpha;
}

// do more with this
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

void sort_for_pruning(std::vector<Move> &move_list, BoardRepresentation &board_representation, Move &previous_best_move)
{
    std::sort(move_list.begin(), move_list.end(), [&board_representation, &previous_best_move](const Move &a, const Move &b)
              {
        int value_a = (a == previous_best_move) ? INT_MAX : std::max(get_piece_value(board_representation.board[a.to_square.rank][a.to_square.file]) -
                      get_piece_value(board_representation.board[a.start_square.rank][a.start_square.file]), 0);
        int value_b = (b == previous_best_move) ? INT_MAX : std::max(get_piece_value(board_representation.board[b.to_square.rank][b.to_square.file]) -
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

int evaluate(BoardRepresentation &board_representation, bool endgame_condition)
{
    int eval = 0;
    for (const Square &square : board_representation.non_empty_squares)
    {
        char piece = board_representation.board[square.rank][square.file];
        int eval_modifier = board_representation.is_opponent_piece(piece) ? -1 : 1;
        eval += get_piece_value(piece) * eval_modifier;
    }

    return eval;
}