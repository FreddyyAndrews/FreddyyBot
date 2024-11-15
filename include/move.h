#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include <string>
#include <iostream>

struct Move
{
  Square start_square; // Starting square for the move
  Square to_square;    // Ending square for the move

  bool is_enpassant;
  bool is_castle;
  char promotion_piece;

  Move(Square new_start_square, Square new_to_square,
       bool enpassant = false, bool castle = false,
       char promotion = 'x')
      : start_square(new_start_square),
        to_square(new_to_square),
        is_enpassant(enpassant),
        is_castle(castle),
        promotion_piece(promotion)
  {
  }

  Move()
      : start_square(-1, -1),
        to_square(-1, -1),
        is_enpassant(false),
        is_castle(false),
        promotion_piece('x')
  {
  }
  std::string to_UCI() const;
};

#endif // MOVE_H
