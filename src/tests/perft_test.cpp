#include <gtest/gtest.h>
#include "move_generator.h"
#include <vector>
#include <atomic>
#include <chrono>
#include <iostream>
#include <cassert>

typedef unsigned long long u64;
std::atomic<u64> nodes_processed(0);
std::chrono::time_point<std::chrono::steady_clock> last_update_time, start_time;
int print_depth = 1;
bool print_divide = true;

// Courtesy of Chessprogrammingwiki
u64 perf_t(int depth, BoardRepresentation &board, u64 expected_nodes)
{
    std::vector<Move> move_list;
    u64 nodes = 0;

    if (depth == 1)
    {
        u64 generated_count = generate_legal_moves(board, move_list);
        if (print_divide && print_depth == 1)
        {
            for (Move move : move_list)
            {
                // Convert move to UCI notation (e.g., "e2e4")
                std::string move_str = move.to_UCI(); // Implement this method

                // Output the move and its node count
                std::cout << move_str << ": " << 1 << std::endl;
            }
        }

        // nodes_processed += generated_count;
        return generated_count;
    }

    generate_legal_moves(board, move_list);
    for (Move move : move_list)
    {
        board.make_move(move);
        u64 new_nodes;
        try
        {
            new_nodes = perf_t(depth - 1, board, expected_nodes);
            nodes += new_nodes;
            board.undo_move(move);
        }
        catch (const std::exception &e)
        {
            std::cout << "Move: " << move.to_UCI() << " led to king capture." << '\n';
            throw std::runtime_error("King captured.");
        }

        if (print_divide && depth == print_depth)
        {
            // Convert move to UCI notation (e.g., "e2e4")
            std::string move_str = move.to_UCI(); // Implement this method

            // Output the move and its node count
            std::cout << move_str << ": " << new_nodes << std::endl;
        }
    }
    return nodes;
}

// Define a test fixture class for Perft tests
class PerftTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize progress tracking variables before each test
        nodes_processed = 0;
        start_time = std::chrono::steady_clock::now();
        last_update_time = start_time;
    }

    // Wrapper method to call perf_t with progress tracking
    u64 run_perft(int depth, BoardRepresentation &board, u64 expected_nodes)
    {
        return perf_t(depth, board, expected_nodes);
    }
};

// Tests
TEST_F(PerftTest, PerftFromStarting)
{
    BoardRepresentation board;
    u64 expected_nodes = 119060324;
    int depth = print_depth = 6;
    u64 perf_t_result = run_perft(depth, board, expected_nodes);
    EXPECT_EQ(perf_t_result, expected_nodes);
}

TEST_F(PerftTest, PerftPosition2)
{
    BoardRepresentation board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    u64 expected_nodes = 8031647685;
    int depth = print_depth = 6;
    u64 perf_t_result = run_perft(depth, board, expected_nodes);
    EXPECT_EQ(perf_t_result, expected_nodes);
}


TEST_F(PerftTest, PerftPosition3)
{
    BoardRepresentation board("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    u64 expected_nodes = 11030083;
    int depth = print_depth = 6;
    u64 perf_t_result = run_perft(depth, board, expected_nodes);
    EXPECT_EQ(perf_t_result, expected_nodes);
}


TEST_F(PerftTest, PerftPosition4)
{
    BoardRepresentation board("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    u64 expected_nodes = 706045033;
    int depth = print_depth = 6;
    u64 perf_t_result = run_perft(depth, board, expected_nodes);
    EXPECT_EQ(perf_t_result, expected_nodes);
}

TEST_F(PerftTest, PerftPosition5)
{
    BoardRepresentation board("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ");
    u64 expected_nodes = 89941194;
    int depth = print_depth = 5;
    u64 perf_t_result = run_perft(depth, board, expected_nodes);
    EXPECT_EQ(perf_t_result, expected_nodes);
}

TEST_F(PerftTest, PerftPosition6)
{
    BoardRepresentation board("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    u64 expected_nodes = 6923051137;
    int depth = print_depth = 6;
    u64 perf_t_result = run_perft(depth, board, expected_nodes);
    EXPECT_EQ(perf_t_result, expected_nodes);
}

