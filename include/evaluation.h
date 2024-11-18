#ifndef EVALUATION_H
#define EVALUATION_H

#include "move_generator.h"
#include "board_representation.h"
#include <climits>
#include <chrono>
#include "piece_square_tables.h"
#include "logging.h"

typedef unsigned long long u64;

const int MATE_SCORE = 1000000;
const int EMERGENCY_MS = 5000; // Safety buffer to prevent flagging (e.g., reserve some time)
const int BUFFER_MS = 500;
const double TRADE_BONUS_FACTOR = 0.5;
const int DOUBLED_PAWN_PENALTY = 25;
const int CLOSE_PAWN_BONUS = 25;
const int OPEN_KING_FILE_PENALTY = 50;
const double ENDGAME_MATERIAL_CONDITION = 0.3;
const double EARLY_GAME_MATERIAL_CONDITION = 0.7;

struct Evaluation
{
    Move best_move;
    int evaluation;

    Evaluation(Move new_best_move, int new_eval) : best_move(new_best_move), evaluation(new_eval) {}
    Evaluation() : best_move(Move()), evaluation(0) {}
};

Evaluation find_best_move(BoardRepresentation &board_representation, const bool am_logging = false, const int wtime = 30000, const int btime = 30000, const int winc = 0, const int binc = 0);
int search(BoardRepresentation &board_representation, int depth, int alpha, int beta, double remaining_material_ratio, int starting_depth);
int search_captures(BoardRepresentation &board_representation, int alpha, int beta, double remaining_material_ratio);
void sort_for_pruning(std::vector<Move> &move_list, const BoardRepresentation &board_representation);
void swap_best_move_to_front(std::vector<Move> &move_list, const Move &best_move);
int get_piece_value(char piece);
int evaluate(BoardRepresentation &board_representation, double remaining_material_ratio);
double get_remaining_material(BoardRepresentation &board_representation);
std::chrono::time_point<std::chrono::steady_clock> find_time_condition(double remaining_material_ratio, int wtime, int btime,
                                                                       int winc, int binc, bool is_white_to_move);
int compute_move_score(const Move &move, const BoardRepresentation &board_representation);
int evaluate_king_safety(const BoardRepresentation &board_representation,
                         double remaining_material_ratio,
                         const std::vector<Square> &friendly_pawns,
                         const std::vector<Square> &opp_pawns,
                         const Square &king_pos,
                         const Square &opp_king_pos);
int evaluate_doubled_pawns(const std::vector<Square> &friendly_pawns,
                           const std::vector<Square> &opp_pawns);

#endif // EVALUATION_H
