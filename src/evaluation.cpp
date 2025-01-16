#include "evaluation.h"

#include <chrono>
#include <sstream>
#include <atomic>
#include <functional>
#include <vector>
#include <limits>

Evaluation find_best_move(BoardRepresentation &board_representation,
                          TranspositionTable &transposition_table,
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
                                   transposition_table,
                                   am_logging,
                                   time_stop_condition);
}

void ponder(BoardRepresentation &board_representation,
            TranspositionTable &transposition_table,
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
                                                             transposition_table,
                                                             am_logging,
                                                             ponder_stop_condition);

    // Extract best and ponder moves from the final evaluation
    best_move_found = position_evaluation.best_move;
    next_ponder_move = position_evaluation.ponder_move;
}

Evaluation run_iterative_deepening(BoardRepresentation &board_representation,
                                   TranspositionTable &transposition_table,
                                   bool am_logging,
                                   std::function<bool()> stop_condition)
{
    Evaluation position_evaluation;

    int depth = MIN_DEPTH_SEARCHED;
    auto start_time = std::chrono::steady_clock::now();
    ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

    double remaining_material_ratio = get_remaining_material(board_representation);

    bool stop_flag = false;

    std::vector<Evaluation> eval_by_depth;

    std::vector<Move> top_depth_moves;
    generate_legal_moves(board_representation, top_depth_moves);

    if (top_depth_moves.empty() || board_representation.threefold_map.hasAnyThreefold())
    {
        throw std::runtime_error("Cannot evaluate terminal position.");
    }
    sort_for_pruning(top_depth_moves, board_representation);

    do
    {
        position_evaluation = search(board_representation,
                                     transposition_table,
                                     top_depth_moves,
                                     depth,
                                     -std::numeric_limits<int>::max(),
                                     std::numeric_limits<int>::max(),
                                     remaining_material_ratio,
                                     depth,
                                     stop_condition,
                                     stop_flag);

        if (position_evaluation.best_move.is_instantiated())
        {
            eval_by_depth.push_back(position_evaluation);
            bump_best_move_to_front(top_depth_moves, position_evaluation.best_move);
        }
        ++depth;
    } while (!stop_condition());

    if (eval_by_depth.empty())
    {
        throw std::runtime_error("No search iterations completed. Cannot determine best move.");
    }

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
            oss << "Final evaluation: " << eval_by_depth.back().evaluation;
            logger.write("Debug", oss.str());
        }
    }

    transposition_table.age_table();

    // Check the last evaluation
    Evaluation &last_eval = eval_by_depth.back();

    if (!last_eval.best_move.is_instantiated())
    {
        throw std::runtime_error("This move should be instantiated.");
    }

    return last_eval;
}

// make sure move ordering is handled between iterations and interrupts are correctly handled
Evaluation search(BoardRepresentation &board_representation,
                  TranspositionTable &transposition_table,
                  std::vector<Move> &top_depth_moves,
                  int depth,
                  int alpha,
                  int beta,
                  double remaining_material_ratio,
                  int starting_depth,
                  const std::function<bool()> &should_stop,
                  bool &stop_flag)
{
    // Store the original alpha so we can decide on EntryType later
    int original_alpha = alpha;

    // Zobrist hash for the current position
    std::uint64_t hash_key = board_representation.zobrist_hash();

    if (depth != starting_depth) // starting position is already accounted for
    {
        board_representation.threefold_map.increment(hash_key);
    }

    // If this position has appeared at least 3 times, declare a draw
    if (board_representation.threefold_map.hasThreefold(hash_key))
    {
        // Decrement on returning
        board_representation.threefold_map.decrement(hash_key);
        return Evaluation(0);
    }

    // -----------------
    // Transposition Table Lookup
    // -----------------
    const TranspositionRow *entry = transposition_table.get(hash_key);
    Move precomputed_best_move;

    if (entry != nullptr)
    {
        if (entry->depth >= depth)
        {
            if (entry->entry_type == EntryType::Alpha && entry->eval <= alpha)
            {
                // Cleanup before return
                board_representation.threefold_map.decrement(hash_key);
                return Evaluation(entry->best_move, entry->best_response, entry->eval);
            }
            else if (entry->entry_type == EntryType::Beta && entry->eval >= beta)
            {
                // Cleanup before return
                board_representation.threefold_map.decrement(hash_key);
                return Evaluation(entry->best_move, entry->best_response, entry->eval);
            }
            else if (entry->entry_type == EntryType::PV)
            {
                // Cleanup before return
                board_representation.threefold_map.decrement(hash_key);
                return Evaluation(entry->best_move, entry->best_response, entry->eval);
            }
        }

        // Use the stored best move for reordering
        precomputed_best_move = entry->best_move;
    }

    // -----------------
    // Base Case
    // -----------------
    if (depth == 0)
    {
        int score = search_captures(board_representation, alpha, beta, remaining_material_ratio);

        // Decrement frequency map on return
        board_representation.threefold_map.decrement(hash_key);
        return Evaluation(score);
    }

    // -----------------
    // Move Generation
    // -----------------
    std::vector<Move> local_move_list;
    std::vector<Move> &move_list = (depth == starting_depth) ? top_depth_moves : local_move_list;

    if (depth != starting_depth)
    {
        generate_legal_moves(board_representation, move_list);
        sort_for_pruning(move_list, board_representation);
    }

    // -----------------
    // Checkmate / Stalemate
    // -----------------
    if (move_list.empty())
    {
        if (board_representation.is_in_check)
        {
            // Faster mates get higher scores
            int mate_score = -(MATE_SCORE - (starting_depth - depth));
            board_representation.threefold_map.decrement(hash_key);
            return Evaluation(mate_score);
        }
        else
        {
            // Stalemate
            board_representation.threefold_map.decrement(hash_key);
            return Evaluation(0);
        }
    }

    // -----------------
    // If we have a best move from TT, reorder
    // -----------------
    if (precomputed_best_move.is_instantiated())
    {
        bump_best_move_to_front(move_list, precomputed_best_move);
    }

    // -----------------
    // Alpha-Beta Loop
    // -----------------
    int best_score = -std::numeric_limits<int>::max();
    Move best_move, best_response;

    for (const Move &move : move_list)
    {
        // Check stop condition
        if (should_stop() && starting_depth != MIN_DEPTH_SEARCHED)
        {
            stop_flag = true;
            break;
        }

        board_representation.make_move(move);

        Evaluation evaluation = search(board_representation,
                                       transposition_table,
                                       top_depth_moves,
                                       depth - 1,
                                       -beta,
                                       -alpha,
                                       remaining_material_ratio,
                                       starting_depth,
                                       should_stop,
                                       stop_flag);

        int score = evaluation.evaluation * -1; // Minimax inverting

        board_representation.undo_move(move);

        if (stop_flag)
        {
            // Discard partial result
            break;
        }

        if (score > best_score)
        {
            best_score = score;
            best_move = move;
            best_response = evaluation.ponder_move;

            if (score > alpha)
            {
                alpha = score;
            }
            // Cutoff
            if (alpha >= beta)
            {
                break;
            }
        }
    }

    // -----------------
    // Decrement frequency map on return
    // -----------------
    if (depth != starting_depth) // input position is fully accounted for before searching
    {
        board_representation.threefold_map.decrement(hash_key);
    }

    // -----------------
    // Write to TT if not interrupted
    // -----------------
    if (depth >= MIN_TRANSPOSITION_DEPTH && !stop_flag)
    {
        EntryType entry_type;
        if (best_score <= original_alpha)
        {
            entry_type = EntryType::Alpha;
        }
        else if (best_score >= beta)
        {
            entry_type = EntryType::Beta;
        }
        else
        {
            entry_type = EntryType::PV;
        }

        transposition_table.insert(hash_key, best_score, depth, best_move, best_response, entry_type);
    }

    return Evaluation(best_move, best_response, best_score);
}

int search_captures(BoardRepresentation &board_representation,
                    int alpha,
                    int beta,
                    double remaining_material_ratio)
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

    for (const Move &move : capture_moves)
    {
        board_representation.make_move(move);

        int score = -search_captures(board_representation, -beta, -alpha, remaining_material_ratio);

        board_representation.undo_move(move);

        if (score >= beta)
        {
            return beta;
        }
        if (score > alpha)
        {
            alpha = score;
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

void bump_best_move_to_front(std::vector<Move> &move_list, const Move &best_move)
{
    if (!best_move.is_instantiated())
    {
        throw std::runtime_error("Best move not set.");
    }

    auto found_it = std::find(move_list.begin(), move_list.end(), best_move);

    if (found_it == move_list.end())
    {
        throw std::runtime_error("Best move does not exist in this vector.");
    }

    // Rotate to move the found move to the front
    std::rotate(move_list.begin(), found_it, found_it + 1);
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
