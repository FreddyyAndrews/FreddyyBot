#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include <string>

struct Move
{
  Square start_square; // Starting square for the move
  Square to_square;    // Ending square for the move

  bool is_enpassant;
  bool is_castle;
  char promotion_piece;

  // Constructor to initialize the move
  Move(int8_t start_rank, int8_t start_file, int8_t to_rank, int8_t to_file,
       bool enpassant = false, bool castle = false, char promotion = 'x')
      : start_square(start_rank, start_file), // Initialize start square
        to_square(to_rank, to_file),          // Initialize destination square
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
