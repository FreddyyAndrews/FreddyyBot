// Square.h

#ifndef SQUARE_H
#define SQUARE_H

struct Square
{
    int rank; // Rank (row) of the square, 0-based
    int file; // File (column) of the square, 0-based

    // Constructor to initialize rank and file
    Square(int r, int f) : rank(r), file(f) {}

    // Default Constructor
    Square() : rank(-1), file(-1) {}

    // Overload the equality operator
    bool operator==(const Square &other) const
    {
        return rank == other.rank && file == other.file;
    }

    // Check if en passant square is defined
    bool exists() const
    {
        return (rank != -1 && file != -1);
    }
};

#endif // SQUARE_H
