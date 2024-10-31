#include "move_generator.h"
#include <iostream>

int generate_pseudo_legal_moves(BoardRepresentation& board_representation, Move* move_list)
{
    board_representation.print_board();
    std::cout << move_list;
    return 0;
}

int generate_legal_moves(BoardRepresentation& board_representation, Move* move_list)
{
    int n_moves = generate_pseudo_legal_moves(board_representation, move_list);
    return n_moves;
}

