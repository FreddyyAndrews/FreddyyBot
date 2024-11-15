#include "move.h"

// Method to convert the move to UCI format
std::string Move::to_UCI() const
{

    if (!start_square.exists() || !to_square.exists())
    {
        throw std::runtime_error("Move not initialized");
    }

    std::string uci;
    uci += static_cast<char>('a' + start_square.file); // Convert file to character ('a' to 'h')
    uci += static_cast<char>('1' + start_square.rank); // Convert rank to character ('1' to '8')
    uci += static_cast<char>('a' + to_square.file);
    uci += static_cast<char>('1' + to_square.rank);

    // Add promotion piece if this is a promotion move
    if (promotion_piece != 'x')
    {
        uci += promotion_piece;
    }
    return uci;
}
