#include <gtest/gtest.h>
#include "manager/board_representation.h"

// Test to verify that input FEN matches output FEN
TEST(BoardRepresentationTest, InputFenEqualsOutputFenStartingPos)
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

TEST(BoardRepresentationTest, WhitePushesPawn)
{
    BoardRepresentation board;

    // FEN string representing a starting position
    std::string input_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Load the starting position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Make move with UCI format
    std::string move = "e2e4";
    board.make_move(move);

    // Expected output FEN after e2e4
    std::string expected_fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, BlackPushesPawn)
{
    BoardRepresentation board;

    // FEN string representing a starting position
    std::string input_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";

    // Load the starting position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Make move with UCI format
    std::string move = "h7h5";
    board.make_move(move);

    // Expected output FEN after e2e4
    std::string expected_fen = "rnbqkbnr/ppppppp1/8/7p/8/8/PPPPPPPP/RNBQKBNR w KQkq h6 0 2";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, WhiteCastlesKingSide)
{
    BoardRepresentation board;

    // FEN string representing a starting position
    std::string input_fen = "k7/8/8/8/8/8/8/4K2R w K - 0 1";

    // Load the starting position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Make move with UCI format
    std::string move = "e1g1";
    board.make_move(move);

    // Expected output FEN after e2e4
    std::string expected_fen = "k7/8/8/8/8/8/8/5RK1 b - - 1 1";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, BlackCapturesWhite)
{
    BoardRepresentation board;

    // FEN string where black pawn can capture white pawn
    std::string input_fen = "rnbqkbnr/pppp1ppp/4p3/8/1P6/8/P1PPPPPP/RNBQKBNR b KQkq - 0 1";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Black pawn captures white pawn on e4
    std::string move = "f8b4";
    board.make_move(move);

    // Expected FEN after the capture
    std::string expected_fen = "rnbqk1nr/pppp1ppp/4p3/8/1b6/8/P1PPPPPP/RNBQKBNR w KQkq - 0 2";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, WhiteCapturesBlack)
{
    BoardRepresentation board;

    // FEN string where black pawn can capture white pawn
    std::string input_fen = "6k1/8/2n5/8/8/8/8/1KR5 w - - 0 1";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Black pawn captures white pawn on e4
    std::string move = "c1c6";
    board.make_move(move);

    // Expected FEN after the capture
    std::string expected_fen = "6k1/8/2R5/8/8/8/8/1K6 b - - 0 1";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, WhiteMovesKnight)
{
    BoardRepresentation board;

    // FEN string where white knight can capture black pawn
    std::string input_fen = "rnbqkbnr/pppppppp/8/8/3N4/8/PPPPPPPP/R1BQKBNR w KQkq - 0 1";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // White knight captures black pawn on f5
    std::string move = "d4f5";
    board.make_move(move);

    // Expected FEN after the capture
    std::string expected_fen = "rnbqkbnr/pppppppp/8/5N2/8/8/PPPPPPPP/R1BQKBNR b KQkq - 1 1";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, WhitePromotesPawn)
{
    BoardRepresentation board;

    // FEN string where white pawn is ready to promote
    std::string input_fen = "8/P7/8/8/8/8/8/k6K w - - 0 1";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // White pawn moves from a7 to a8 and promotes to queen
    std::string move = "a7a8q";
    board.make_move(move);

    // Expected FEN after promotion
    std::string expected_fen = "Q7/8/8/8/8/8/8/k6K b - - 0 1";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, BlackPromotesPawn)
{
    BoardRepresentation board;

    // FEN string where black pawn is ready to promote
    std::string input_fen = "k6K/8/8/8/8/8/p7/8 b - - 0 1";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Black pawn moves from a2 to a1 and promotes to queen
    std::string move = "a2a1q";
    board.make_move(move);

    // Expected FEN after promotion
    std::string expected_fen = "k6K/8/8/8/8/8/8/q7 w - - 0 2";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, BlackCastlesKingSide)
{
    BoardRepresentation board;

    // FEN string where black can castle kingside
    std::string input_fen = "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Black castles kingside
    std::string move = "e8g8";
    board.make_move(move);

    // Expected FEN after castling
    std::string expected_fen = "r4rk1/8/8/8/8/8/8/4K3 w - - 1 2";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, WhiteEnPassant)
{
    BoardRepresentation board;

    // FEN where en passant is possible for white
    std::string input_fen = "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // White pawn captures en passant on d6
    std::string move = "e5d6";
    board.make_move(move);

    // Expected FEN after en passant capture
    std::string expected_fen = "rnbqkbnr/ppp1pppp/3P4/8/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, BlackEnPassant)
{
    BoardRepresentation board;

    // FEN where en passant is possible for black
    std::string input_fen = "8/8/8/8/3pP3/8/8/8 b - e3 0 2";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Black pawn captures en passant on d3
    std::string move = "d4e3";
    board.make_move(move);

    // Expected FEN after en passant capture
    std::string expected_fen = "8/8/8/8/8/4p3/8/8 w - - 0 3";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, BlackCastlesQueenSide)
{
    BoardRepresentation board;

    // FEN string where black can castle kingside
    std::string input_fen = "r3k3/8/8/8/8/8/8/K7 b kq - 0 1";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Black castles kingside
    std::string move = "e8c8";
    board.make_move(move);

    // Expected FEN after castling
    std::string expected_fen = "2kr4/8/8/8/8/8/8/K7 w - - 1 2";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}

TEST(BoardRepresentationTest, WhiteCastlesQueenSide)
{
    BoardRepresentation board;

    // FEN string where black can castle kingside
    std::string input_fen = "7k/8/8/8/8/8/8/R3K3 w KQ - 0 1";

    // Load the position
    board.input_fen_position(input_fen);

    // Call the output_fen_position method and store the result
    std::string test_fen = board.output_fen_position();

    // Check if the input FEN matches the output FEN
    EXPECT_EQ(input_fen, test_fen);

    // Black castles kingside
    std::string move = "e1c1";
    board.make_move(move);

    // Expected FEN after castling
    std::string expected_fen = "7k/8/8/8/8/8/8/2KR4 b - - 1 1";

    // Get the output FEN after the move
    std::string output_fen = board.output_fen_position();

    // Check if the output FEN matches the expected FEN
    EXPECT_EQ(expected_fen, output_fen);
}
