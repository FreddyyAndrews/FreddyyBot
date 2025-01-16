#include "evaluation.h"
#include <gtest/gtest.h>

TEST(EvaluationTest, TestMateIn1)
{
    init_zobrist_keys();
    BoardRepresentation board_representation = BoardRepresentation("8/8/8/8/kr5Q/8/8/1R5K w - - 0 1");
    TranspositionTable transposition_table;
    Evaluation eval = find_best_move(board_representation, transposition_table);

    EXPECT_EQ("h4b4", eval.best_move.to_UCI());
    EXPECT_EQ(MATE_SCORE - 1, eval.evaluation);
}

TEST(EvaluationTest, TestMateIn2)
{
    init_zobrist_keys();
    BoardRepresentation board_representation = BoardRepresentation("2R5/2R5/8/8/8/7K/pn6/k1r3r1 w - - 0 1");
    TranspositionTable transpo;
    Evaluation eval = find_best_move(board_representation, transpo);

    EXPECT_EQ("c7c1", eval.best_move.to_UCI());
    EXPECT_EQ(MATE_SCORE - 3, eval.evaluation);
}

TEST(EvaluationTest, TestAvoidStalemate)
{
    init_zobrist_keys();
    BoardRepresentation board_representation = BoardRepresentation("6Q1/8/7k/8/4p3/PP2P3/4KPP1/8 w - - 0 1");
    TranspositionTable transpo;
    Evaluation eval = find_best_move(board_representation, transpo);
    ASSERT_NE("g2g4", eval.best_move.to_UCI());
}

TEST(EvaluationTest, KingSafetyTest1)
{
    BoardRepresentation board_representation = BoardRepresentation("1k1r1bnr/ppp5/2nq4/5b2/5B2/P1NQ4/P1P5/1K1R1BNR w - - 0 1");
    Square king_pos = Square(0, 1);
    Square opp_pos = Square(7, 1);
    std::vector<Square> friendly_pawns = {Square(2, 0), Square(1, 0), Square(1, 2)};
    std::vector<Square> opp_pawns = {Square(6, 0), Square(6, 1), Square(6, 2)};

    int king_safety_score = evaluate_king_safety(board_representation, 0.8, friendly_pawns, opp_pawns, king_pos, opp_pos);
    int expected = (CLOSE_PAWN_BONUS * 2) - OPEN_KING_FILE_PENALTY - (CLOSE_PAWN_BONUS * 3);
    EXPECT_EQ(king_safety_score, expected);
}

TEST(EvaluationTest, KingSafetyTest2)
{
    BoardRepresentation board_representation = BoardRepresentation("1k1r1bnr/ppp5/2nq4/5b2/5B2/P1NQ4/P1P5/1K1R1BNR b - - 0 1");
    Square king_pos = Square(7, 1);
    Square opp_pos = Square(0, 1);
    std::vector<Square> friendly_pawns = {Square(6, 0), Square(6, 1), Square(6, 2)};
    std::vector<Square> opp_pawns = {Square(2, 0), Square(1, 0), Square(1, 2)};

    int king_safety_score = evaluate_king_safety(board_representation, 0.8, friendly_pawns, opp_pawns, king_pos, opp_pos);
    int expected = -(CLOSE_PAWN_BONUS * 2) + OPEN_KING_FILE_PENALTY + (CLOSE_PAWN_BONUS * 3);
    EXPECT_EQ(king_safety_score, expected);
}

TEST(EvaluationTest, DoubledPawnsTest1)
{
    std::vector<Square> friendly_pawns = {Square(6, 0), Square(6, 1), Square(6, 2)};
    std::vector<Square> opp_pawns = {Square(2, 0), Square(1, 0), Square(1, 2)};

    int doubled_pawn_score = evaluate_doubled_pawns(friendly_pawns, opp_pawns);
    int expected = DOUBLED_PAWN_PENALTY;
    EXPECT_EQ(doubled_pawn_score, expected);
}

TEST(EvaluationTest, DoubledPawnsTest2)
{
    std::vector<Square> friendly_pawns = {Square(2, 0), Square(1, 0), Square(1, 2)};
    std::vector<Square> opp_pawns = {Square(6, 0), Square(6, 1), Square(6, 2)};

    int doubled_pawn_score = evaluate_doubled_pawns(friendly_pawns, opp_pawns);
    int expected = -DOUBLED_PAWN_PENALTY;
    EXPECT_EQ(doubled_pawn_score, expected);
}

TEST(EvaluationTest, ThreefoldTest)
{
    init_zobrist_keys();
    TranspositionTable transposition_table;
    std::vector<std::string> moves = {"h1g1", "a5a6", "g1h1", "a6a5", "h1g1", "a5a6", "g1h1"};
    BoardRepresentation board_representation = BoardRepresentation("8/8/8/k7/8/8/7N/7K w - - 0 1", moves);

    Evaluation eval = find_best_move(board_representation, transposition_table, true);

    EXPECT_EQ("a6a5", eval.best_move.to_UCI());
    EXPECT_EQ(0, eval.evaluation);
}
