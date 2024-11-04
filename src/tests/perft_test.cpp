#include <gtest/gtest.h>
#include "move_generator.h"
#include <vector>

typedef unsigned long long u64;

// Courtesy of Chessprogrammingwiki
u64 perf_t(int depth, BoardRepresentation &board, int top_depth)
{
    std::vector<Move> move_list;
    u64 nodes = 0;

    if (depth == 0)
        return 1ULL;

    generate_legal_moves(board, move_list);
    for (Move move : move_list)
    {
        board.make_move(move);
        nodes += perf_t(depth - 1, board, top_depth);
        board.undo_move(move);
        if (depth == top_depth)
        {
            std::cout << move.to_UCI() << ": " << nodes << '\n';
        }
    }
    return nodes;
}

TEST(MoveGeneratorTest, PerftFromStarting)
{
    BoardRepresentation board = BoardRepresentation();
    u64 perf_t_result = perf_t(2, board, 2);
    EXPECT_EQ(perf_t_result, 400);
}

/*
TEST(MoveGeneratorTest, PerftFromStarting)
{
    BoardRepresentation board = BoardRepresentation();
    u64 perf_t_result = perf_t(7, board, 7);
    EXPECT_EQ(perf_t_result, 3195901860);
}


TEST(MoveGeneratorTest, PerftPosition2)
{
    BoardRepresentation board = BoardRepresentation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    u64 perf_t_result = perf_t(6, board, 6);
    EXPECT_EQ(perf_t_result, 8031647685);
}

TEST(MoveGeneratorTest, PerftPosition3)
{
    BoardRepresentation board = BoardRepresentation("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    u64 perf_t_result = perf_t(7, board, 7);
    EXPECT_EQ(perf_t_result, 178633661);
}

TEST(MoveGeneratorTest, PerftPosition4)
{
    BoardRepresentation board = BoardRepresentation("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    u64 perf_t_result = perf_t(6, board, 6);
    EXPECT_EQ(perf_t_result, 706045033);
}

TEST(MoveGeneratorTest, PerftPosition5)
{
    BoardRepresentation board = BoardRepresentation("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ");
    u64 perf_t_result = perf_t(5, board, 5);
    EXPECT_EQ(perf_t_result, 89941194);
}

TEST(MoveGeneratorTest, PerftPosition6)
{
    BoardRepresentation board = BoardRepresentation("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    u64 perf_t_result = perf_t(7, board, 7);
    EXPECT_EQ(perf_t_result, 287188994746);
}

*/
