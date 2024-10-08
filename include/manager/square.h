// Square.h

#ifndef SQUARE_H
#define SQUARE_H

struct Square
{
    int rank; // Rank (row) of the square, 0-based
    int file; // File (column) of the square, 0-based

    // Constructor to initialize rank and file
    Square(int r, int f) : rank(r), file(f) {}
};

#endif // SQUARE_H
