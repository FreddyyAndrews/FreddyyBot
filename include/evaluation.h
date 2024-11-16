#ifndef EVALUATION_H
#define EVALUATION_H

#include "move_generator.h"
#include "board_representation.h"
#include <climits>
#include <chrono>

typedef unsigned long long u64;

const int MATE_SCORE = 100000;
const int ENDGAME_MATERIAL_CONDITION = 13;
const int MOVE_TIME_SAFETY_BUFFER_MS = 200; // Safety buffer to prevent flagging (e.g., reserve some time)

struct Evaluation
{
    Move best_move;
    int evaluation;

    Evaluation(Move new_best_move, int new_eval) : best_move(new_best_move), evaluation(new_eval) {}
    Evaluation() : best_move(Move()), evaluation(0) {}
};

Evaluation find_best_move(BoardRepresentation &board_representation, const int wtime = 30000, const int btime = 30000, const int winc = 0, const int binc = 0);
int search(BoardRepresentation &board_representation, int depth, int alpha, int beta, bool is_endgame_condition);
int search_captures(BoardRepresentation &board_representation, int alpha, int beta, bool endgame_condition);
void sort_for_pruning(std::vector<Move> &move_list, const BoardRepresentation &board_representation);
void swap_best_move_to_front(std::vector<Move> &move_list, const Move &best_move);
int get_piece_value(char piece);
int evaluate(BoardRepresentation &board_representation, bool endgame_condition);
bool is_endgame(BoardRepresentation &board_representation);
std::chrono::time_point<std::chrono::steady_clock> find_time_condition(bool is_endgame_condition, int wtime, int btime,
                                                                       int winc, int binc, bool is_white_to_move);
int compute_move_score(const Move &move, const BoardRepresentation &board_representation);

#endif // EVALUATION_H
