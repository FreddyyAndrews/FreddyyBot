#include "evaluation.h"

Evaluation find_best_move(BoardRepresentation &board_representation, int wtime, int btime, int winc, int binc)
{
    Evaluation position_evaluation = Evaluation();
    int depth = 1;
    auto start_time = std::chrono::steady_clock::now();
    double remaining_material_ratio = get_remaining_material(board_representation);
    std::chrono::time_point<std::chrono::steady_clock> cutoff_time = find_time_condition(
        remaining_material_ratio, wtime, btime, winc, binc, board_representation.white_to_move);

    // Calculate allocated time in milliseconds
    auto allocated_time = std::chrono::duration_cast<std::chrono::milliseconds>(cutoff_time - start_time).count();
    // std::cout << "Allocated " << allocated_time << " milliseconds" << std::endl;

    // Prepare move list
    std::vector<Move> move_list;
    generate_legal_moves(board_representation, move_list);

    // Initial sorting of moves (e.g., ordering captures, etc.)
    sort_for_pruning(move_list, board_representation);

    // iterative deepening
    do
    {
        int alpha = -INT_MAX;
        int beta = INT_MAX;
        Move best_move;

        for (const Move &move : move_list)
        {
            board_representation.make_move(move);
            int evaluation = -search(board_representation, depth - 1, -beta, -alpha, remaining_material_ratio, depth);
            board_representation.undo_move(move);

            if (evaluation > alpha)
            {
                alpha = evaluation;
                best_move = move;
            }

            // handle mid search timeout
            if (depth != 1 && std::chrono::steady_clock::now() > cutoff_time)
            {
                // std::cout << "Timed out mid search " << depth << std::endl;
                break;
            }
        }

        position_evaluation.best_move = best_move;
        position_evaluation.evaluation = alpha;

        // Check elapsed time
        auto current_time = std::chrono::steady_clock::now();
        if (current_time > cutoff_time)
        {
            // std::cout << "Timed out at depth " << depth << std::endl;
            break;
        }

        // Re-sort move list to put best_move first for better move ordering
        swap_best_move_to_front(move_list, position_evaluation.best_move);

        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        // std::cout << "Searched depth " << depth << " in " << elapsed_time << " milliseconds" << std::endl;
        // std::cout << "The best move is " << position_evaluation.best_move.to_UCI() << std::endl;
        // std::cout << "The evaluation is " << position_evaluation.evaluation << std::endl;
        ++depth;
    } while (true);

    return position_evaluation;
}

std::chrono::time_point<std::chrono::steady_clock> find_time_condition(double remaining_material_ratio, int wtime, int btime,
                                                                       int winc, int binc, bool is_white_to_move)
{
    int remaining_time = is_white_to_move ? wtime : btime;
    int increment = is_white_to_move ? winc : binc;

    if (remaining_time < EMERGENCY_MS) // handle very low time
    {
        int time_per_move = std::max(10, increment - BUFFER_MS);
        auto start_time = std::chrono::steady_clock::now();
        return start_time + std::chrono::milliseconds(time_per_move);
    }

    // Determine the number of moves left based on the remaining material ratio
    // Assume full game length of 60 moves at the start and 30 moves near the end (conservative to avoid flagging)
    int moves_left = static_cast<int>(60 * remaining_material_ratio + 30 * (1 - remaining_material_ratio));

    // Basic time allocation per move
    int time_per_move = remaining_time / moves_left;

    // Adjust time per move based on increment
    time_per_move += increment;

    int max_time = 20000; // milliseconds (20 seconds)

    // Clamp the time_per_move within min and max limits
    time_per_move = std::max(10, std::min(time_per_move, max_time) - BUFFER_MS);

    // Calculate the cutoff time as a time point in the future
    auto start_time = std::chrono::steady_clock::now();
    return start_time + std::chrono::milliseconds(time_per_move);
}

double get_remaining_material(BoardRepresentation &board_representation)
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

    // Calculate the ratio of remaining material (starting total material is 7800)
    double remaining_material_ratio = static_cast<double>(material_count) / 7800.0;
    return remaining_material_ratio;
}

int search(BoardRepresentation &board_representation, int depth, int alpha, int beta, double remaining_material_ratio, int starting_depth)
{
    if (depth == 0)
    {
        return search_captures(board_representation, alpha, beta, remaining_material_ratio);
    }

    std::vector<Move> move_list;
    generate_legal_moves(board_representation, move_list);

    if (move_list.empty())
    {
        if (board_representation.is_in_check)
        {
            // Closer mates are worth more
            return -(MATE_SCORE - (starting_depth - depth));
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
        int evaluation = -search(board_representation, depth - 1, -beta, -alpha, remaining_material_ratio, starting_depth);
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

int search_captures(BoardRepresentation &board_representation, int alpha, int beta, double remaining_material_ratio)
{
    int evaluation = evaluate(board_representation, remaining_material_ratio);
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
        evaluation = -search_captures(board_representation, -beta, -alpha, remaining_material_ratio);
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
        return 100;

    case 'n': // Knight
        return 300;

    case 'b': // Bishop
        return 300;

    case 'r': // Rook
        return 500;

    case 'q': // Queen
        return 900;

    default:
        return 0;
    }
}

int evaluate(BoardRepresentation &board_representation, double remaining_material_ratio)
{
    int eval = 0;
    for (const Square &square : board_representation.non_empty_squares)
    {
        char piece = board_representation.board[square.rank][square.file];
        bool is_opponent_piece = board_representation.is_opponent_piece(piece);
        int eval_modifier = is_opponent_piece ? -1 : 1;

        // Get piece type in lowercase for uniformity
        char piece_type = to_lower(piece);

        // Add material value
        int material_value = get_piece_value(piece_type);
        eval += material_value * eval_modifier;

        // Get positional value from the piece-square table
        int position_value = 0;

        // Flip rank for black pieces
        int rank = square.rank;
        int file = square.file;

        // Adjust rank if the piece is white
        if (is_white_piece(piece))
        {
            rank = 7 - rank;
        }

        // Select the correct piece-square table
        switch (piece_type)
        {
        case 'p': // Pawn
            position_value = pawn_piece_square_table[rank][file];
            break;
        case 'n': // Knight
            position_value = knight_piece_square_table[rank][file];
            break;
        case 'b': // Bishop
            position_value = bishop_piece_square_table[rank][file];
            break;
        case 'r': // Rook
            position_value = rook_piece_square_table[rank][file];
            break;
        case 'q': // Queen
            position_value = queen_piece_square_table[rank][file];
            break;
        case 'k': // King
            if (remaining_material_ratio >= 0.8)
            {
                // Use only the regular king table
                position_value = king_piece_square_table[rank][file];
            }
            else if (remaining_material_ratio <= 0.2)
            {
                // Use only the endgame king table
                position_value = king_endgame_piece_square_table[rank][file];
            }
            else
            {
                // Weighted calculation between regular and endgame tables
                double weight_regular = (remaining_material_ratio - 0.2) / 0.6;
                double weight_endgame = (0.8 - remaining_material_ratio) / 0.6;

                int regular_value = king_piece_square_table[rank][file];
                int endgame_value = king_endgame_piece_square_table[rank][file];

                position_value = static_cast<int>(
                    weight_regular * regular_value + weight_endgame * endgame_value + 0.5); // +0.5 for rounding
            }
            break;
        default:
            position_value = 0;
            break;
        }

        // Add positional value
        eval += position_value * eval_modifier;
    }

    return eval;
}