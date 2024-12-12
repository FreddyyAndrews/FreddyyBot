#include "evaluation.h"

Evaluation find_best_move(BoardRepresentation &board_representation, Move &ponder_move,
                          bool am_logging, int wtime, int btime, int winc, int binc)
{
    Evaluation position_evaluation = Evaluation();
    int depth = 1;
    // auto start_time = std::chrono::steady_clock::now();
    double remaining_material_ratio = get_remaining_material(board_representation);
    auto start_time = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> cutoff_time = find_time_condition(
        remaining_material_ratio, wtime, btime, winc, binc, board_representation.white_to_move);

    int previous_iteration_nodes = -1;
    int current_iteration_nodes = -1;

    ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

    if (am_logging)
    {
        // Calculate allocated time in milliseconds
        auto allocated_time = std::chrono::duration_cast<std::chrono::milliseconds>(cutoff_time - start_time).count();
        std::ostringstream oss;
        oss << "Allocated " << allocated_time << " milliseconds";
        logger.write("Debug", oss.str());
    }

    // Prepare move list
    std::vector<Move> move_list;
    generate_legal_moves(board_representation, move_list);

    // Initial sorting of moves (e.g., ordering captures, etc.)
    sort_for_pruning(move_list, board_representation);

    // iterative deepening
    do
    {
        auto iteration_start_time = std::chrono::steady_clock::now();
        int alpha = -INT_MAX;
        int beta = INT_MAX;
        Move best_move;
        current_iteration_nodes = 0;
        bool stop = false;

        for (const Move &move : move_list)
        {
            // handle mid search timeout
            if (depth > MIN_DEPTH_SEARCHED && std::chrono::steady_clock::now() > cutoff_time)
            {
                stop = true;
                break;
            }

            board_representation.make_move(move);
            int evaluation = -search(board_representation, depth - 1, -beta,
                                     -alpha, remaining_material_ratio, depth,
                                     current_iteration_nodes, ponder_move, stop);
            board_representation.undo_move(move);

            if (evaluation > alpha)
            {
                alpha = evaluation;
                best_move = move;
            }
        }

        position_evaluation.best_move = best_move;
        position_evaluation.evaluation = alpha;

        // Re-sort move list to put best_move first for better move ordering
        swap_best_move_to_front(move_list, position_evaluation.best_move);

        if (depth != 1 && !should_continue_iterating(current_iteration_nodes, previous_iteration_nodes,
                                                     iteration_start_time, cutoff_time))
        {
            break;
        }

        ++depth;
        previous_iteration_nodes = current_iteration_nodes; // Set this iterations nodes to be the last iteration nodes

    } while (std::chrono::steady_clock::now() < cutoff_time);

    if (am_logging)
    {
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - start_time)
                                .count();
        std::ostringstream oss;
        oss << "Searched depth " << depth << " in " << elapsed_time << " milliseconds";
        logger.write("Debug", oss.str());
        std::ostringstream oss2;
        oss2 << "The evaluation is " << position_evaluation.evaluation;
        logger.write("Debug", oss2.str());
    }

    return position_evaluation;
}

void ponder(BoardRepresentation &board_representation,
            Move &next_ponder_move, Move &best_move_found,
            bool am_logging, const std::atomic<bool> &stop_pondering)
{
    // Set starting depth
    int depth = 1;
    // auto start_time = std::chrono::steady_clock::now();
    double remaining_material_ratio = get_remaining_material(board_representation);
    auto start_time = std::chrono::steady_clock::now();

    // Open the log file in append mode
    ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

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
        int current_iteration_nodes = 0;

        for (const Move &move : move_list)
        {
            if (stop_pondering.load()) // Time to stop pondering
            {
                break;
            }

            board_representation.make_move(move);
            int evaluation = -search(board_representation, depth - 1, -beta,
                                     -alpha, remaining_material_ratio, depth,
                                     current_iteration_nodes, next_ponder_move, stop_pondering);
            board_representation.undo_move(move);

            if (evaluation > alpha)
            {
                alpha = evaluation;
                best_move = move;
            }
        }

        best_move_found = best_move;

        // Re-sort move list to put best_move first for better move ordering
        swap_best_move_to_front(move_list, best_move_found);

        ++depth;
    } while (!stop_pondering.load());

    if (am_logging)
    {
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - start_time)
                                .count();
        std::ostringstream oss;
        oss << "Pondered to depth " << depth << " in " << elapsed_time << " milliseconds";
        logger.write("Debug", oss.str());
    }
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

int search(BoardRepresentation &board_representation, int depth,
           int alpha, int beta, double remaining_material_ratio,
           int starting_depth, int &current_iteration_nodes,
           Move &ponder_move, const std::atomic<bool> &stop_pondering)
{
    if (depth == 0)
    {
        ++current_iteration_nodes;
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

    Move best_response;
    for (const Move &move : move_list)
    {
        board_representation.make_move(move);
        int evaluation = -search(board_representation, depth - 1, -beta, -alpha,
                                 remaining_material_ratio, starting_depth,
                                 current_iteration_nodes, ponder_move, stop_pondering);
        board_representation.undo_move(move);

        if (evaluation >= beta)
        {
            return beta;
        }

        if (evaluation > alpha)
        {
            alpha = evaluation;
            best_response = move;
        }

        if (stop_pondering.load())
        {
            break;
        }
    }

    if (depth == starting_depth - 1) // If we are on the opponents first response move set ponder to best response
    {
        ponder_move = best_response;
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

    int our_material_total = 0;
    int opponent_material_total = 0;
    std::vector<Square> friendly_pawns, opp_pawns;
    Square king_pos, opp_king_pos;

    // **Material and Positional Evaluation**
    for (const Square &square : board_representation.non_empty_squares)
    {
        char piece = board_representation.board[square.rank][square.file];
        bool is_opponent_piece = board_representation.is_opponent_piece(piece);
        int eval_modifier = is_opponent_piece ? -1 : 1;

        // **Normalize Piece Type**
        char piece_type = to_lower(piece);

        // **Add Material Value**
        int material_value = get_piece_value(piece_type);
        eval += material_value * eval_modifier;

        // **Accumulate Material Totals**
        if (is_opponent_piece)
        {
            opponent_material_total += material_value;
        }
        else
        {
            our_material_total += material_value;
        }

        // **Get Positional Value from Piece-Square Table**
        int position_value = 0;
        int rank = square.rank;
        int file = square.file;

        // **Adjust Rank for White Pieces**
        if (is_white_piece(piece))
        {
            rank = 7 - rank;
        }

        // **Select the Correct Piece-Square Table**
        switch (piece_type)
        {
        case 'p': // Pawn
            position_value = pawn_piece_square_table[rank][file];

            // **Store Friendly and Opponent Pawn Locations**
            if (is_opponent_piece)
            {
                opp_pawns.push_back(square);
            }
            else
            {
                friendly_pawns.push_back(square);
            }
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
            if (is_opponent_piece)
            {
                opp_king_pos = square;
            }
            else
            {
                king_pos = square;
            }

            // **King Position Evaluation Based on Game Phase**
            if (remaining_material_ratio >= EARLY_GAME_MATERIAL_CONDITION)
            {
                position_value = king_piece_square_table[rank][file];
            }
            else if (remaining_material_ratio <= ENDGAME_MATERIAL_CONDITION)
            {
                position_value = king_endgame_piece_square_table[rank][file];
            }
            else
            {
                double weight_regular = (remaining_material_ratio - ENDGAME_MATERIAL_CONDITION) /
                                        (EARLY_GAME_MATERIAL_CONDITION - ENDGAME_MATERIAL_CONDITION);
                double weight_endgame = 1.0 - weight_regular;

                int regular_value = king_piece_square_table[rank][file];
                int endgame_value = king_endgame_piece_square_table[rank][file];

                position_value = static_cast<int>(
                    weight_regular * regular_value + weight_endgame * endgame_value + 0.5);
            }
            break;
        default:
            position_value = 0;
            break;
        }

        // **Add Positional Value**
        eval += position_value * eval_modifier;
    }

    // **Trade Bonus Calculation**
    int material_difference = our_material_total - opponent_material_total;
    double trade_bonus = material_difference * (1.0 - remaining_material_ratio) * TRADE_BONUS_FACTOR;
    eval += static_cast<int>(trade_bonus);

    // **King Safety Evaluation**
    int king_safety_bonus = evaluate_king_safety(board_representation,
                                                 remaining_material_ratio,
                                                 friendly_pawns,
                                                 opp_pawns,
                                                 king_pos,
                                                 opp_king_pos);
    eval += king_safety_bonus;

    // **Doubled Pawns Evaluation**
    int doubled_pawns_penalty = evaluate_doubled_pawns(friendly_pawns, opp_pawns);
    eval += doubled_pawns_penalty;

    return eval;
}

int evaluate_king_safety(const BoardRepresentation &board_representation,
                         double remaining_material_ratio,
                         const std::vector<Square> &friendly_pawns,
                         const std::vector<Square> &opp_pawns,
                         const Square &king_pos,
                         const Square &opp_king_pos)
{
    int white_king_safety_bonus = 0;
    int black_king_safety_bonus = 0;

    const std::vector<Square> &white_pawns = board_representation.white_to_move ? friendly_pawns : opp_pawns;
    const std::vector<Square> &black_pawns = board_representation.white_to_move ? opp_pawns : friendly_pawns;
    const Square &white_king = board_representation.white_to_move ? king_pos : opp_king_pos;
    const Square &black_king = board_representation.white_to_move ? opp_king_pos : king_pos;

    // **White King Safety Evaluation**
    if (!board_representation.white_can_castle_kingside &&
        !board_representation.white_can_castle_queenside &&
        remaining_material_ratio > ENDGAME_MATERIAL_CONDITION)
    {
        bool white_king_file_open = true;

        for (const Square &pawn : white_pawns)
        {
            int distance_to_king = std::max(std::abs(white_king.rank - pawn.rank),
                                            std::abs(white_king.file - pawn.file));

            if (distance_to_king <= 1)
            {
                white_king_safety_bonus += CLOSE_PAWN_BONUS;
            }

            if (pawn.file == white_king.file)
            {
                white_king_file_open = false;
            }
        }

        if (white_king_file_open)
        {
            white_king_safety_bonus -= OPEN_KING_FILE_PENALTY;
        }
    }

    // **Black King Safety Evaluation**
    if (!board_representation.black_can_castle_kingside &&
        !board_representation.black_can_castle_queenside &&
        remaining_material_ratio > ENDGAME_MATERIAL_CONDITION)
    {
        bool black_king_file_open = true;

        for (const Square &pawn : black_pawns)
        {
            int distance_to_king = std::max(std::abs(black_king.rank - pawn.rank),
                                            std::abs(black_king.file - pawn.file));

            if (distance_to_king <= 1)
            {
                black_king_safety_bonus += CLOSE_PAWN_BONUS;
            }

            if (pawn.file == black_king.file)
            {
                black_king_file_open = false;
            }
        }

        if (black_king_file_open)
        {
            black_king_safety_bonus -= OPEN_KING_FILE_PENALTY;
        }
    }

    // **Calculate the King Safety Difference**
    int king_safety_difference = white_king_safety_bonus - black_king_safety_bonus;
    int king_safety_bonus = board_representation.white_to_move ? king_safety_difference : -king_safety_difference;

    return king_safety_bonus;
}

int evaluate_doubled_pawns(const std::vector<Square> &friendly_pawns,
                           const std::vector<Square> &opp_pawns)
{
    int eval = 0;

    // **Initialize Arrays to Count Pawns on Each File**
    int friendly_pawn_count_by_file[8] = {0};
    int opponent_pawn_count_by_file[8] = {0};

    // **Count Friendly Pawns on Each File**
    for (const Square &pawn : friendly_pawns)
    {
        friendly_pawn_count_by_file[pawn.file]++;
    }

    // **Count Opponent Pawns on Each File**
    for (const Square &pawn : opp_pawns)
    {
        opponent_pawn_count_by_file[pawn.file]++;
    }

    // **Apply Penalties and Bonuses**
    for (int file = 0; file < 8; ++file)
    {
        int friendly_pawns_on_file = friendly_pawn_count_by_file[file];
        int opponent_pawns_on_file = opponent_pawn_count_by_file[file];

        // **Penalty for Doubled Friendly Pawns**
        if (friendly_pawns_on_file > 1)
        {
            eval -= (friendly_pawns_on_file - 1) * DOUBLED_PAWN_PENALTY;
        }

        // **Bonus if Opponent Has Doubled Pawns**
        if (opponent_pawns_on_file > 1)
        {
            eval += (opponent_pawns_on_file - 1) * DOUBLED_PAWN_PENALTY;
        }
    }

    return eval;
}
