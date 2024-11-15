#ifndef MOVESTATE_H
#define MOVESTATE_H

#include "square.h"

struct MoveState
{
  bool white_can_castle_kingside;
  bool white_can_castle_queenside;
  bool black_can_castle_kingside;
  bool black_can_castle_queenside;
  Square en_passant_square; // Record the en passant square
  int halfmove_clock;       // To restore 50-move rule count
  int fullmove_number;
  char piece_on_target_square; // Record any captured piece
  bool white_to_move;

  MoveState(bool w_ks, bool w_qs, bool b_ks, bool b_qs, Square ep_square, int halfmove, int fullmove, char captured, bool white_to_move)
      : white_can_castle_kingside(w_ks), white_can_castle_queenside(w_qs),
        black_can_castle_kingside(b_ks), black_can_castle_queenside(b_qs),
        en_passant_square(ep_square), halfmove_clock(halfmove), fullmove_number(fullmove),
        piece_on_target_square(captured), white_to_move(white_to_move) {}
};

#endif // MOVESTATE_H