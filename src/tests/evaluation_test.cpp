#include "evaluation.h"
#include <gtest/gtest.h>

TEST(EvaluationTest, TestMateIn1)
{
    BoardRepresentation board_representation = BoardRepresentation("8/8/8/8/kr5Q/8/8/1R5K w - - 0 1");
    Evaluation eval = find_best_move(board_representation);

    EXPECT_EQ("h4b4", eval.best_move.to_UCI());
    EXPECT_EQ(MATE_SCORE - 1, eval.evaluation);
}

TEST(EvaluationTest, TestCaptureChain)
{
    BoardRepresentation board_representation = BoardRepresentation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    Evaluation eval = find_best_move(board_representation);
    std::cout << "Best move: " << eval.best_move.to_UCI() << std::endl;
    EXPECT_EQ(0, 0);
}