// square.h

#ifndef SQUARE_H
#define SQUARE_H

#include <functional> // For std::hash
#include <cstddef>    // For std::size_t
#include <cmath>
#include <string>

struct Square
{
    int8_t rank; // Rank (row) of the square, 0-based
    int8_t file; // File (column) of the square, 0-based

    // Constructor to initialize rank and file
    Square(int8_t r, int8_t f) : rank(r), file(f) {}

    // Default Constructor
    Square() : rank(-1), file(-1) {}

    // Copy Constructor
    Square(const Square &other) : rank(other.rank), file(other.file) {}

    // Copy Assignment Operator
    Square &operator=(const Square &other)
    {
        if (this != &other) // Self-assignment check
        {
            rank = other.rank;
            file = other.file;
        }
        return *this;
    }

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

    // Convert the square to standard chess notation (e.g., "a3")
    std::string to_string() const
    {
        if (!exists())
        {
            return "invalid"; // Return "invalid" if the square does not exist
        }
        char file_char = static_cast<char>('a' + file); // Convert 0-based file to 'a'-'h'
        int8_t rank_num = rank + 1;                     // Convert 0-based rank to 1-based rank
        return std::string(1, file_char) + std::to_string(rank_num);
    }

    bool is_between(const Square &square_a, const Square &square_b) const;
};

namespace std
{
    template <>
    struct hash<Square>
    {
        std::size_t operator()(const Square &s) const
        {
            // Combine the hash of rank and file using a common hash combining technique
            return (std::hash<int8_t>()(s.rank) ^ (std::hash<int8_t>()(s.file) << 1));
        }
    };
}

#endif // SQUARE_H
