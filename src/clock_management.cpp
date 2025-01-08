#include "clock_management.h"

std::chrono::milliseconds find_time_condition(double remaining_material_ratio, int wtime, int btime,
                                              int winc, int binc, bool is_white_to_move)
{
    int remaining_time = is_white_to_move ? wtime : btime;
    int increment = is_white_to_move ? winc : binc;

    if (remaining_time + increment < EMERGENCY_MS) // handle very low time
    {
        int time_per_move = std::max(MIN_MOVE_TIME, increment - BUFFER_MS);
        return std::chrono::milliseconds(time_per_move);
    }

    // Determine the number of moves left based on the remaining material ratio
    // Assume full game length of 60 moves at the start and 30 moves near the end (conservative to avoid flagging)
    int moves_left = static_cast<int>(TOP_REMAINING_MOVES_ASSUMPTION * remaining_material_ratio + BOTTOM_REMAINING_MOVES_ASSUMPTION * (1 - remaining_material_ratio));

    // Basic time allocation per move
    int time_per_move = remaining_time / moves_left;

    // Adjust time per move based on increment
    time_per_move += increment;

    // Calculate the cutoff time as a time point in the future

    return std::chrono::milliseconds(time_per_move);
}

bool should_continue_iterating(int current_iteration_nodes, int previous_iteration_nodes,
                               std::chrono::steady_clock::time_point iteration_start_time,
                               std::chrono::steady_clock::time_point cutoff_time)
{
    double ebf = std::sqrt(double(current_iteration_nodes) / double(previous_iteration_nodes));

    // Estimate next iteration node count
    double expected_nodes_next = current_iteration_nodes * ebf;

    // Estimate average time per node from this iteration
    auto iteration_end_time = std::chrono::steady_clock::now();
    auto iteration_duration = std::chrono::duration_cast<std::chrono::milliseconds>(iteration_end_time - iteration_start_time).count();
    double avg_time_per_node = double(iteration_duration) / double(current_iteration_nodes);

    // Predict if next iteration will exceed cutoff
    double expected_next_iteration_time = expected_nodes_next * avg_time_per_node; // in milliseconds
    auto current_time = std::chrono::steady_clock::now();
    if (current_time + std::chrono::milliseconds((long long)expected_next_iteration_time) > cutoff_time)
    {
        return false;
    }
    return true;
}