#include "evaluation.h"
#include <gtest/gtest.h>

TEST(EvaluationTest, TestMateIn1)
{
    BoardRepresentation board_representation = BoardRepresentation("8/8/8/8/kr5Q/8/8/1R5K w - - 0 1");
    Evaluation eval = find_best_move(board_representation);

    EXPECT_EQ("h4b4", eval.best_move.to_UCI());
    EXPECT_EQ(MATE_SCORE - 1, eval.evaluation);
}

TEST(EvaluationTest, TestMateIn2)
{
    BoardRepresentation board_representation = BoardRepresentation("2R5/2R5/8/8/8/7K/pn6/k1r3r1 w - - 0 1");
    Evaluation eval = find_best_move(board_representation);

    EXPECT_EQ("c7c1", eval.best_move.to_UCI());
    EXPECT_EQ(MATE_SCORE - 3, eval.evaluation);
}