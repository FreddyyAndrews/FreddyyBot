#ifndef MOVE_GENERATOR
#define MOVE_GENERATOR

#include "board_representation.h"
#include "move.h"

int generate_pseudo_legal_moves(BoardRepresentation&, Move*);
int generate_legal_moves(BoardRepresentation&, Move*);

#endif // MOVE_GENERATOR
