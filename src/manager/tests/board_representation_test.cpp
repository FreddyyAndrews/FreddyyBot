#include <gtest/gtest.h>
#include "manager/board_representation.h"

// Test to verify that input FEN matches output FEN
TEST(BoardRepresentationTest, InputFenEqualsOutputFen)
{
    // Initialize your BoardRepresentation class
    BoardRepresentation board;

    // Example FEN string representing a starting position
    std::string input_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Call the input_fen_position method
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string output_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, output_fen);
}
