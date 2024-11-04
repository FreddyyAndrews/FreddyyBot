#include <gtest/gtest.h>
#include "board_representation.h"

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

void test_move(BoardRepresentation &board, const std::string &input_fen,
               const std::string &move, const std::string &expected_fen)
{
    // Load the initial FEN position
    board.input_fen_position(input_fen);

    // Step 1: Assert input FEN is consistent with board's output FEN
    EXPECT_EQ(input_fen, board.output_fen_position());

    // Step 2: Make the move and check FEN after the move
    Move move_struct = board.make_move(move);
    EXPECT_EQ(expected_fen, board.output_fen_position());

    // Step 3: Undo the move and check FEN is reverted to the input FEN
    board.undo_move(move_struct);
    EXPECT_EQ(input_fen, board.output_fen_position());
}

TEST(BoardRepresentationTest, WhitePushesPawn)
{
    BoardRepresentation board;
    test_move(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
              "e2e4", "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

TEST(BoardRepresentationTest, BlackPushesPawn)
{
    BoardRepresentation board;
    test_move(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
              "h7h5", "rnbqkbnr/ppppppp1/8/7p/8/8/PPPPPPPP/RNBQKBNR w KQkq h6 0 2");
}

TEST(BoardRepresentationTest, WhiteCastlesKingSide)
{
    BoardRepresentation board;
    test_move(board, "k7/8/8/8/8/8/8/4K2R w K - 0 1",
              "e1g1", "k7/8/8/8/8/8/8/5RK1 b - - 1 1");
}

TEST(BoardRepresentationTest, BlackCapturesWhite)
{
    BoardRepresentation board;
    test_move(board, "rnbqkbnr/pppp1ppp/4p3/8/1P6/8/P1PPPPPP/RNBQKBNR b KQkq - 0 1",
              "f8b4", "rnbqk1nr/pppp1ppp/4p3/8/1b6/8/P1PPPPPP/RNBQKBNR w KQkq - 0 2");
}

TEST(BoardRepresentationTest, WhiteCapturesBlack)
{
    BoardRepresentation board;
    test_move(board, "6k1/8/2n5/8/8/8/8/1KR5 w - - 0 1",
              "c1c6", "6k1/8/2R5/8/8/8/8/1K6 b - - 0 1");
}

TEST(BoardRepresentationTest, WhiteMovesKnight)
{
    BoardRepresentation board;
    test_move(board, "rnbqkbnr/pppppppp/8/8/3N4/8/PPPPPPPP/R1BQKBNR w KQkq - 0 1",
              "d4f5", "rnbqkbnr/pppppppp/8/5N2/8/8/PPPPPPPP/R1BQKBNR b KQkq - 1 1");
}

TEST(BoardRepresentationTest, WhitePromotesPawn)
{
    BoardRepresentation board;
    test_move(board, "8/P7/8/8/8/8/8/k6K w - - 0 1",
              "a7a8q", "Q7/8/8/8/8/8/8/k6K b - - 0 1");
}

TEST(BoardRepresentationTest, BlackPromotesPawn)
{
    BoardRepresentation board;
    test_move(board, "k6K/8/8/8/8/8/p7/8 b - - 0 1",
              "a2a1q", "k6K/8/8/8/8/8/8/q7 w - - 0 2");
}

TEST(BoardRepresentationTest, BlackCastlesKingSide)
{
    BoardRepresentation board;
    test_move(board, "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1",
              "e8g8", "r4rk1/8/8/8/8/8/8/4K3 w - - 1 2");
}

TEST(BoardRepresentationTest, WhiteEnPassant)
{
    BoardRepresentation board;
    test_move(board, "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3",
              "e5d6", "rnbqkbnr/ppp1pppp/3P4/8/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3");
}

TEST(BoardRepresentationTest, BlackEnPassant)
{
    BoardRepresentation board;
    test_move(board, "8/8/8/8/3pP3/8/8/8 b - e3 0 2",
              "d4e3", "8/8/8/8/8/4p3/8/8 w - - 0 3");
}

TEST(BoardRepresentationTest, BlackCastlesQueenSide)
{
    BoardRepresentation board;
    test_move(board, "r3k3/8/8/8/8/8/8/K7 b kq - 0 1",
              "e8c8", "2kr4/8/8/8/8/8/8/K7 w - - 1 2");
}

TEST(BoardRepresentationTest, WhiteCastlesQueenSide)
{
    BoardRepresentation board;
    test_move(board, "7k/8/8/8/8/8/8/R3K3 w KQ - 0 1",
              "e1c1", "7k/8/8/8/8/8/8/2KR4 b - - 1 1");
}

TEST(BoardRepresentationTest, WhiteCapturesBlackRookKingside)
{
    BoardRepresentation board;
    test_move(board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              "h1h8", "r3k2R/8/8/8/8/8/8/R3K3 b Qq - 0 1");
}

TEST(BoardRepresentationTest, WhiteCapturesBlackRookQueenside)
{
    BoardRepresentation board;
    test_move(board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
              "a1a8", "R3k2r/8/8/8/8/8/8/4K2R b Kk - 0 1");
}

TEST(BoardRepresentationTest, BlackCapturesWhiteRookQueenside)
{
    BoardRepresentation board;
    test_move(board, "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
              "a8a1", "4k2r/8/8/8/8/8/8/r3K2R w Kk - 0 2");
}

TEST(BoardRepresentationTest, BlackCapturesWhiteRookKingside)
{
    BoardRepresentation board;
    test_move(board, "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
              "h8h1", "r3k3/8/8/8/8/8/8/R3K2r w Qq - 0 2");
}
