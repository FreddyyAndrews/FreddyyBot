#include "evaluation.h"

int nodes_explored = 0;

Evaluation find_best_move(BoardRepresentation &board_representation, int wtime, int btime, int winc, int binc)
{
    nodes_explored = 0;
    Evaluation position_evaluation = Evaluation();
    int depth = 1;
    auto start_time = std::chrono::steady_clock::now();
    bool is_endgame_condition = is_endgame(board_representation);
    std::chrono::time_point<std::chrono::steady_clock> cutoff_time = find_time_condition(
        is_endgame_condition, wtime, btime, winc, binc, board_representation.white_to_move);

    // Calculate allocated time in milliseconds
    auto allocated_time = std::chrono::duration_cast<std::chrono::milliseconds>(cutoff_time - start_time).count();
    std::cout << "Allocated " << allocated_time << " milliseconds" << std::endl;

    // Prepare move list
    std::vector<Move> move_list;
    generate_legal_moves(board_representation, move_list);

    // Initial sorting of moves (e.g., ordering captures, etc.)
    sort_for_pruning(move_list, board_representation);

    // iterative deepening
    do
    {
        nodes_explored = 0;
        int alpha = -INT_MAX;
        int beta = INT_MAX;
        Move best_move;

        for (const Move &move : move_list)
        {
            board_representation.make_move(move);
            int evaluation = -search(board_representation, depth - 1, -beta, -alpha, is_endgame_condition);
            board_representation.undo_move(move);

            if (evaluation > alpha)
            {
                alpha = evaluation;
                best_move = move;
            }

            // handle mid search timeout
            if (depth != 1 && std::chrono::steady_clock::now() > cutoff_time)
            {
                std::cout << "Timed out mid search " << depth << std::endl;
                break;
            }
        }

        position_evaluation.best_move = best_move;
        position_evaluation.evaluation = alpha;

        // Check elapsed time
        auto current_time = std::chrono::steady_clock::now();
        if (current_time > cutoff_time)
        {
            std::cout << "Timed out at depth " << depth << std::endl;
            break;
        }

        // Re-sort move list to put best_move first for better move ordering
        swap_best_move_to_front(move_list, position_evaluation.best_move);

        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        std::cout << "Searched depth " << depth << " in " << elapsed_time << " milliseconds" << std::endl;
        std::cout << "best move is " << position_evaluation.best_move.to_UCI() << std::endl;
        std::cout << "Total nodes explored " << nodes_explored << std::endl;
        ++depth;
    } while (true);

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
    int max_time = 30000; // milliseconds

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

int search(BoardRepresentation &board_representation, int depth, int alpha, int beta, bool is_endgame_condition)
{
    if (depth == 0)
    {
        ++nodes_explored; // not precise since it doesn't count capture nodes but gives an idea of move ordering quality
        return search_captures(board_representation, alpha, beta, is_endgame_condition);
    }

    std::vector<Move> move_list;
    generate_legal_moves(board_representation, move_list);

    if (move_list.empty())
    {
        if (board_representation.is_in_check)
        {
            // Closer mates are worth more
            return -(MATE_SCORE - depth);
        }
        else
        {
            return 0;
        }
    }

    sort_for_pruning(move_list, board_representation);

    for (const Move &move : move_list)
    {
        board_representation.make_move(move);
        int evaluation = -search(board_representation, depth - 1, -beta, -alpha, is_endgame_condition);
        board_representation.undo_move(move);

        if (evaluation >= beta)
        {
            return beta;
        }

        if (evaluation > alpha)
        {
            alpha = evaluation;
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

int compute_move_score(const Move &move, const BoardRepresentation &board_representation)
{
    int score = 0;

    // Determine if move is a capture
    char captured_piece = board_representation.board[move.to_square.rank][move.to_square.file];
    bool is_capture = captured_piece != 'e';

    // Get moving piece
    char moving_piece = board_representation.board[move.start_square.rank][move.start_square.file];

    // Get piece values
    int captured_value = get_piece_value(captured_piece);
    int moving_value = get_piece_value(moving_piece);

    // Determine if move is a pawn promotion
    bool is_promotion = move.promotion_piece != 'x';

    if (is_capture)
    {
        int gain = captured_value - moving_value;

        int mvv_lva_score = (captured_value * 10) - moving_value;

        if (gain > 0)
        {
            // Winning capture
            score = 5000 + mvv_lva_score;
        }
        else if (gain == 0)
        {
            // Equal capture
            score = 3000 + mvv_lva_score;
        }
        else // gain < 0
        {
            // Losing capture
            score = 2000 - mvv_lva_score;
        }
    }
    else if (is_promotion)
    {
        // Pawn promotion
        score = 4000;
    }
    else if (move.is_castle)
    {
        score = 1500;
    }
    else
    {
        // Everything else
        score = 1000;
    }

    return score;
}

void sort_for_pruning(std::vector<Move> &move_list, const BoardRepresentation &board_representation)
{
    std::sort(move_list.begin(), move_list.end(), [&board_representation](const Move &a, const Move &b)
              {
                  int score_a = compute_move_score(a, board_representation);
                  int score_b = compute_move_score(b, board_representation);
                  return score_a > score_b; // Sort in descending order
              });
}

void swap_best_move_to_front(std::vector<Move> &move_list, const Move &best_move)
{
    if (!best_move.is_instantiated())
    {
        throw std::runtime_error("Best move not set.");
    }
    for (size_t i = 0; i < move_list.size(); ++i)
    {
        if (move_list[i] == best_move)
        {
            if (i != 0)
            {
                std::swap(move_list[0], move_list[i]);
            }
            break; // Exit the loop once the best move is found and moved
        }
    }
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