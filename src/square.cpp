#include "square.h"

bool Square::is_between(const Square &square_a, const Square &square_b) const
{
    // Check if all squares are aligned along the same rank (horizontal line)
    if (square_a.rank == square_b.rank && square_a.rank == this->rank)
    {
        // Get the minimum and maximum files to check if 'this' is between them
        int min_file = std::min(square_a.file, square_b.file);
        int max_file = std::max(square_a.file, square_b.file);

        // Check if 'this' square's file is between the files of square_a and square_b
        return this->file > min_file && this->file < max_file;
    }
    // Check if all squares are aligned along the same file (vertical line)
    else if (square_a.file == square_b.file && square_a.file == this->file)
    {
        // Get the minimum and maximum ranks to check if 'this' is between them
        int min_rank = std::min(square_a.rank, square_b.rank);
        int max_rank = std::max(square_a.rank, square_b.rank);

        // Check if 'this' square's rank is between the ranks of square_a and square_b
        return this->rank > min_rank && this->rank < max_rank;
    }
    // Check if all squares are aligned along a diagonal
    else if (std::abs(square_a.rank - square_b.rank) == std::abs(square_a.file - square_b.file) &&
             std::abs(square_a.rank - this->rank) == std::abs(square_a.file - this->file))
    {
        // Get the minimum and maximum ranks and files
        int min_rank = std::min(square_a.rank, square_b.rank);
        int max_rank = std::max(square_a.rank, square_b.rank);
        int min_file = std::min(square_a.file, square_b.file);
        int max_file = std::max(square_a.file, square_b.file);

        // Check if 'this' square is between square_a and square_b on the diagonal
        return this->rank > min_rank && this->rank<max_rank &&this->file> min_file && this->file < max_file;
    }
    else
    {
        // The squares are not aligned; 'this' square cannot be between them
        return false;
    }
}
