#ifndef EVALUATION_H
#define EVALUATION_H

#include "move_generator.h"
#include "board_representation.h"
#include <climits>

typedef unsigned long long u64;

const int max_depth = 4;
const int MATE_SCORE = 100000;

struct Evaluation
{
    Move best_move;
    int evaluation;

    Evaluation(Move new_best_move, int new_eval) : best_move(new_best_move), evaluation(new_eval) {}
    Evaluation() : best_move(Move()), evaluation(0) {}
};

Evaluation find_best_move(BoardRepresentation &board_representation);
int search(Evaluation &position_evaluation, BoardRepresentation &board_representation, int depth, int alpha, int beta);
void sort_for_pruning(std::vector<Move> &move_list, BoardRepresentation &board_representation);
int get_piece_value(char piece);
int evaluate(BoardRepresentation &board_representation);

#endif // BOARD_REPRESENTATION_H
