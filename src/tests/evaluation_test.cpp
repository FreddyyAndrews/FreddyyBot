#include "evaluation.h"
#include <gtest/gtest.h>

TEST(EvaluationTest, TestMateIn1)
{
    BoardRepresentation board_representation = BoardRepresentation("8/8/8/8/kr5Q/8/8/1R5K w - - 0 1");
    Evaluation eval = find_best_move(board_representation);

    EXPECT_EQ("h4b4", eval.best_move.to_UCI());
    EXPECT_EQ(INT_MAX - 1, eval.evaluation);
}