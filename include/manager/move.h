#ifndef MOVE_H
#define MOVE_H

#include "square.h"

struct Move
{
    Square start_square; // Starting square for the move
    Square to_square;    // Ending square for the move

    bool is_enpassant;
    bool is_castle;
    char promotion_piece;

    // Constructor to initialize the move
    Move(int start_rank, int start_file, int to_rank, int to_file,
         bool enpassant = false, bool castle = false, char promotion = 'x')
        : start_square(start_rank, start_file), // Initialize start square
          to_square(to_rank, to_file),          // Initialize destination square
          is_enpassant(enpassant),
          is_castle(castle),
          promotion_piece(promotion)
    {
    }
};

#endif // MOVE_H
