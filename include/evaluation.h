#ifndef EVALUATION_H
#define EVALUATION_H

#include "move_generator.h"
#include "board_representation.h"
#include <climits>
#include <chrono>
#include "piece_square_tables.h"
#include "logging.h"
#include "clock_management.h"
#include <sstream>
#include <atomic>
#include <vector>
#include "transposition_table.h"

typedef unsigned long long u64;

const int MATE_SCORE = 1000000;
const double TRADE_BONUS_FACTOR = 0.5;
const int DOUBLED_PAWN_PENALTY = 25;
const int CLOSE_PAWN_BONUS = 25;
const int OPEN_KING_FILE_PENALTY = 50;
const double ENDGAME_MATERIAL_CONDITION = 0.3;
const double EARLY_GAME_MATERIAL_CONDITION = 0.7;
const int MIN_DEPTH_SEARCHED = 1;
const float KING_PIECE_SQUARE_MAP_MODIFIER = 1.5; // increase king safety weight
const int DEFAULT_SEARCH_TIME_MS = 1000;

struct Evaluation
{
    Move best_move;
    Move ponder_move;
    int evaluation;

    Evaluation() : best_move(), ponder_move(), evaluation(0) {}
    Evaluation(const int evaluation) : best_move(), ponder_move(), evaluation(evaluation) {}
    Evaluation(const int evaluation, const Move &best_move) : best_move(best_move), ponder_move(), evaluation(evaluation) {}

    Evaluation(const Move &best_move,
               const Move &ponder_move,
               const int evaluation) : best_move(best_move),
                                       ponder_move(ponder_move),
                                       evaluation(evaluation)
    {
    }
};

Evaluation find_best_move(BoardRepresentation &board_representation,
                          TranspositionTable &transposition_table,
                          const bool am_logging = false,
                          const int wtime = 30000,
                          const int btime = 30000,
                          const int winc = 0,
                          const int binc = 0,
                          const int forced_time = -1);

void ponder(BoardRepresentation &board_representation,
            TranspositionTable &transposition_table,
            Move &next_ponder_move,
            Move &best_move_found,
            bool am_logging,
            const std::atomic<bool> &stop_pondering,
            const std::atomic<bool> &hard_stop,
            const int wtime = 30000,
            const int btime = 30000,
            const int winc = 0,
            const int binc = 0,
            const int forced_time = -1);

Evaluation run_iterative_deepening(BoardRepresentation &board_representation,
                                   TranspositionTable &transposition_table,
                                   bool am_logging,
                                   std::function<bool()> stop_condition);

Evaluation search(BoardRepresentation &board_representation,
                  TranspositionTable &transposition_table,
                  std::vector<Move> &top_depth_moves,
                  int depth,
                  int alpha,
                  int beta,
                  double remaining_material_ratio,
                  int starting_depth,
                  const std::function<bool()> &should_stop,
                  bool &stop_flag);

int search_captures(BoardRepresentation &board_representation,
                    int alpha,
                    int beta,
                    double remaining_material_ratio);

void sort_for_pruning(std::vector<Move> &move_list,
                      const BoardRepresentation &board_representation);

void bump_best_move_to_front(std::vector<Move> &move_list,
                             const Move &best_move);

int get_piece_value(char piece);

int evaluate(BoardRepresentation &board_representation,
             double remaining_material_ratio);

double get_remaining_material(BoardRepresentation &board_representation);

int compute_move_score(const Move &move,
                       const BoardRepresentation &board_representation);

int evaluate_king_safety(const BoardRepresentation &board_representation,
                         double remaining_material_ratio,
                         const std::vector<Square> &friendly_pawns,
                         const std::vector<Square> &opp_pawns,
                         const Square &king_pos,
                         const Square &opp_king_pos);

int evaluate_doubled_pawns(const std::vector<Square> &friendly_pawns,
                           const std::vector<Square> &opp_pawns);

#endif // EVALUATION_H
