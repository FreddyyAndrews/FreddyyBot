#ifndef MOVE_GENERATOR
#define MOVE_GENERATOR

#include "board_representation.h"
#include "move.h"

class MoveGenerator
{
public:
    MoveGenerator();
    ~MoveGenerator();
    Move *generate_moves();
private:
    Move *move_heap;
};
#endif // MOVE_GENERATOR
