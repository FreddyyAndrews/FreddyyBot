// square.h

#ifndef SQUARETOSQUAREMAP_H
#define SQUARETOSQUAREMAP_H

#include "square.h"

struct SquareToSquareMap
{
    Square attacked;
    Square attacker;

    // Default Constructor
    SquareToSquareMap() : attacked(Square()), attacker(Square()) {}

    // Constructor that takes two Square objects
    SquareToSquareMap(const Square &attacked_square, const Square &attacker_square)
        : attacked(attacked_square), attacker(attacker_square) {}
};

#endif // SQUARETOSQUAREMAP_H
