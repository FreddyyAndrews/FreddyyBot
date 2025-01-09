#include "evaluation.h"

#include <chrono>
#include <sstream>
#include <atomic>
#include <functional>
#include <vector>
#include <limits>

Evaluation run_iterative_deepening(BoardRepresentation &board_representation,
                                   bool am_logging,
                                   std::function<bool()> stop_condition)
{
    Evaluation position_evaluation;

    int depth = MIN_DEPTH_SEARCHED;
    auto start_time = std::chrono::steady_clock::now();
    ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

    // Generate initial move list
    std::vector<Move> move_list;
    generate_legal_moves(board_representation, move_list);
    sort_for_pruning(move_list, board_representation);

    double remaining_material_ratio = get_remaining_material(board_representation);

    do
    {
        int alpha = -std::numeric_limits<int>::max();
        int beta = std::numeric_limits<int>::max();

        bool stop_flag = false;
        Move best_move;

        // store the best line at this iteration
        std::vector<Move> best_line_at_this_depth;

        for (const Move &move : move_list)
        {
            if (stop_condition() && depth != MIN_DEPTH_SEARCHED)
            {
                break;
            }

            board_representation.make_move(move);

            // local principal variation for the move
            std::vector<Move> pv_child;
            int evaluation = -search(board_representation,
                                     depth - 1,
                                     -beta,
                                     -alpha,
                                     remaining_material_ratio,
                                     depth,
                                     pv_child,
                                     stop_condition,
                                     stop_flag);

            board_representation.undo_move(move);

            if (stop_flag)
            {
                // The search for this move was interrupted, discard it
                break;
            }

            if (evaluation > alpha)
            {
                alpha = evaluation;
                best_move = move;

                // build best_line_at_this_depth from this move + its child line
                best_line_at_this_depth.clear();
                best_line_at_this_depth.push_back(move);
                best_line_at_this_depth.insert(
                    best_line_at_this_depth.end(),
                    pv_child.begin(),
                    pv_child.end());
            }
        }

        // If we found a fully-searched move
        if (best_move.is_instantiated())
        {
            // Record the final evaluation
            position_evaluation.evaluation = alpha;
            // Record the best move at this depth
            position_evaluation.best_move = best_move;

            // If the line has at least two moves and it's not mate-in-1
            // we treat the second move as the “ponder” move
            if (best_line_at_this_depth.size() >= 2)
            {
                // Check for mate-in-1. This is optional; you might detect it via alpha == MATE_SCORE or similar.
                // For demonstration, let's say we only skip the ponder if alpha is a checkmate score near MATE_SCORE
                bool mate_in_one = (std::abs(alpha) >= (MATE_SCORE - 1));
                if (!mate_in_one)
                {
                    position_evaluation.ponder_move = best_line_at_this_depth[1];
                }
            }

            {
                std::ostringstream oss;
                oss << "Principal Variation at depth " << depth;

                // Example: position_evaluation.final_pv holds the entire line of moves
                for (const Move &m : position_evaluation.pv)
                {
                    oss << " " << m.to_UCI() << " ";
                }
                logger.write("Debug", oss.str());
            }

            position_evaluation.pv = best_line_at_this_depth;

            // Move the best move to the front for next iteration's ordering
            swap_best_move_to_front(move_list, best_move);
        }
        else
        {
            // Could not find any fully searched move; keep previous iteration’s best
            logger.write("Debug", "Search interrupted before any move was fully evaluated at this depth.");
        }

        ++depth;

    } while (!stop_condition());

    // Logging ...
    if (am_logging)
    {
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - start_time)
                                .count();

        {
            std::ostringstream oss;
            oss << "Searched depth " << depth << " in " << elapsed_time << " milliseconds";
            logger.write("Debug", oss.str());
        }

        {
            std::ostringstream oss;
            oss << "Final evaluation: " << position_evaluation.evaluation;
            logger.write("Debug", oss.str());
        }

        // Instead of logging best moves by depth and best opponent responses,
        //      just log the entire final PV from your last completed iteration.
        {
            std::ostringstream oss;
            oss << "Principal Variation from the final search: ";

            // Example: position_evaluation.final_pv holds the entire line of moves
            for (const Move &m : position_evaluation.pv)
            {
                oss << m.to_UCI() << " ";
            }
            logger.write("Debug", oss.str());
        }
    }

    assert(position_evaluation.best_move.is_instantiated());
    return position_evaluation;
}

Evaluation find_best_move(BoardRepresentation &board_representation,
                          bool am_logging,
                          int wtime, int btime,
                          int winc, int binc,
                          int forced_time)
{
    // Capture start time (used for logging allocated time)
    auto start_time = std::chrono::steady_clock::now();

    // Create cutoff time using your time allocation logic
    double remaining_material_ratio = get_remaining_material(board_representation);
    std::chrono::milliseconds cutoff_ms = (forced_time < 0)
                                              ? find_time_condition(
                                                    remaining_material_ratio,
                                                    wtime, btime,
                                                    winc, binc,
                                                    board_representation.white_to_move)
                                              : std::chrono::milliseconds(forced_time);

    std::chrono::time_point<std::chrono::steady_clock> cutoff_time = start_time + cutoff_ms;

    ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

    // Log the allocated time in milliseconds
    if (am_logging)
    {
        auto allocated_time = cutoff_ms.count();
        std::ostringstream oss;
        oss << "Allocated " << allocated_time << " milliseconds";
        logger.write("Debug", oss.str());
    }

    // Create the time-based stop condition
    auto time_stop_condition = [cutoff_time]()
    {
        return std::chrono::steady_clock::now() >= cutoff_time;
    };

    // Call the unified function
    return run_iterative_deepening(board_representation,
                                   am_logging,
                                   time_stop_condition);
}

void ponder(BoardRepresentation &board_representation,
            Move &next_ponder_move,
            Move &best_move_found,
            bool am_logging,
            const std::atomic<bool> &ponder_hit,
            const std::atomic<bool> &hard_stop,
            int wtime,
            int btime,
            int winc,
            int binc,
            int forced_time)
{
    ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

    // Get remaining material ratio for time calculation
    double remaining_material_ratio = get_remaining_material(board_representation);

    // Determine how much time (in ms) we would allocate if we were to start timing now
    // find_time_condition now returns a std::chrono::milliseconds duration
    std::chrono::milliseconds allocated_time = (forced_time < 0)
                                                   ? find_time_condition(remaining_material_ratio,
                                                                         wtime, btime,
                                                                         winc, binc,
                                                                         board_representation.white_to_move)
                                                   : std::chrono::milliseconds(forced_time);

    auto base_allocated_ms = allocated_time.count();

    if (am_logging)
    {
        std::ostringstream oss;
        oss << "Pondering will have up to " << base_allocated_ms
            << " ms after receiving ponderhit";
        logger.write("Debug", oss.str());
    }

    // We do not start applying the allocated time until we detect stop_pondering == true
    bool time_limit_active = false;
    std::chrono::steady_clock::time_point actual_cutoff;

    // Stop condition logic for indefinite pondering until stop_pondering is set
    auto ponder_stop_condition = [&]()
    {
        if (hard_stop.load()) // hard stop will always stop execution right away
        {
            logger.write("Debug", "Ponder received stop signal. Stopping immediately.");
            return true;
        }

        // Phase 1: indefinite pondering
        if (!ponder_hit.load())
        {
            return false;
        }
        // Phase 2: once stop signal is received, switch to time-limited search
        if (!time_limit_active)
        {
            time_limit_active = true;
            auto now = std::chrono::steady_clock::now();
            actual_cutoff = now + std::chrono::milliseconds(base_allocated_ms);

            if (am_logging)
            {
                std::ostringstream oss;
                oss << "Pondering received ponderhit signal. Now searching up to "
                    << base_allocated_ms << " ms more.";
                logger.write("Debug", oss.str());
            }
        }
        // Stop if we've reached our newly enforced time cutoff
        return (std::chrono::steady_clock::now() >= actual_cutoff);
    };

    // Use the same iterative deepening search
    Evaluation position_evaluation = run_iterative_deepening(board_representation,
                                                             am_logging,
                                                             ponder_stop_condition);

    // Extract best and ponder moves from the final evaluation
    best_move_found = position_evaluation.best_move;
    next_ponder_move = position_evaluation.ponder_move;
}

int search(BoardRepresentation &board_representation,
           int depth,
           int alpha,
           int beta,
           double remaining_material_ratio,
           int starting_depth,
           std::vector<Move> &pv, // <--- principal variation
           const std::function<bool()> &should_stop,
           bool &stop_flag)
{
    // Base case: if depth == 0, call search_captures with a local PV vector
    if (depth == 0)
    {
        std::vector<Move> captures_pv; // child PV
        int score = search_captures(board_representation, alpha, beta, remaining_material_ratio, captures_pv);

        // Merge captures_pv into our top-level pv
        // For captures, we typically only return the best sequence of captures
        // but it’s often just 1 or 2 moves. If you’d like the full chain, do the same:
        pv = captures_pv; // Copy the line found in captures

        return score;
    }

    std::vector<Move> move_list;
    generate_legal_moves(board_representation, move_list);

    // If no moves, it’s checkmate or stalemate
    if (move_list.empty())
    {
        if (board_representation.is_in_check)
        {
            // Mate score offset by how deep we are, so we prefer faster mates
            return -(MATE_SCORE - (starting_depth - depth));
        }
        else
        {
            return 0; // stalemate
        }
    }

    sort_for_pruning(move_list, board_representation);

    int best_score = -std::numeric_limits<int>::max();
    Move best_move;

    // local vector to store the child PV
    std::vector<Move> child_pv;

    for (const Move &move : move_list)
    {
        // Check stop condition if not in first iteration
        if (should_stop() && starting_depth != MIN_DEPTH_SEARCHED)
        {
            stop_flag = true;
            break;
        }

        board_representation.make_move(move);

        // Clear child_pv each time we search a child
        child_pv.clear();

        int score = -search(board_representation,
                            depth - 1,
                            -beta,
                            -alpha,
                            remaining_material_ratio,
                            starting_depth,
                            child_pv,
                            should_stop,
                            stop_flag);

        board_representation.undo_move(move);

        if (stop_flag)
        {
            // If search was interrupted while evaluating this move, discard its partial result
            break;
        }

        if (score > best_score)
        {
            best_score = score;
            best_move = move;

            if (score > alpha)
            {
                alpha = score;
            }

            // rebuild our principal variation by prepending this move to the child’s PV
            pv.clear();
            pv.push_back(move);
            pv.insert(pv.end(), child_pv.begin(), child_pv.end());

            // Alpha-beta cutoff
            if (alpha >= beta)
            {
                break;
            }
        }
    }

    return best_score;
}

int search_captures(BoardRepresentation &board_representation,
                    int alpha,
                    int beta,
                    double remaining_material_ratio,
                    std::vector<Move> &pv)
{
    // Evaluate current position
    int evaluation = evaluate(board_representation, remaining_material_ratio);

    if (evaluation >= beta)
    {
        return beta;
    }

    if (evaluation > alpha)
    {
        alpha = evaluation;
    }

    // Generate only capture moves
    std::vector<Move> capture_moves;
    generate_legal_moves(board_representation, capture_moves, /* capturesOnly = */ true);
    sort_for_pruning(capture_moves, board_representation);

    // local vector to store child captures PV
    std::vector<Move> child_pv;

    for (const Move &move : capture_moves)
    {
        board_representation.make_move(move);

        child_pv.clear();
        int score = -search_captures(board_representation, -beta, -alpha, remaining_material_ratio, child_pv);

        board_representation.undo_move(move);

        if (score >= beta)
        {
            return beta;
        }
        if (score > alpha)
        {
            alpha = score;

            // build new PV for captures by prepending this move
            pv.clear();
            pv.push_back(move);
            pv.insert(pv.end(), child_pv.begin(), child_pv.end());
        }
    }

    return alpha;
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
    int king_safety_bonus = 0;
    if (remaining_material_ratio > 0.5) // If more than half the material is on the board do the additional king safety checks
    {
        king_safety_bonus = evaluate_king_safety(board_representation,
                                                 remaining_material_ratio,
                                                 friendly_pawns,
                                                 opp_pawns,
                                                 king_pos,
                                                 opp_king_pos);
    }

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
