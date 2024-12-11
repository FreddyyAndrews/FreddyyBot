#ifndef CLOCK_MANAGEMENT_H
#define CLOCK_MANAGEMENT_H

#include "move_generator.h"
#include "board_representation.h"
#include <climits>
#include <chrono>
#include <cmath>

typedef unsigned long long u64;

const int EMERGENCY_MS = 5000; // Safety buffer to prevent flagging (e.g., reserve some time)
const int MIN_MOVE_TIME = 50;
const int BUFFER_MS = 500;
const int TOP_REMAINING_MOVES_ASSUMPTION = 30;
const int BOTTOM_REMAINING_MOVES_ASSUMPTION = 10;

std::chrono::time_point<std::chrono::steady_clock> find_time_condition(double remaining_material_ratio, int wtime, int btime,
                                                                       int winc, int binc, bool is_white_to_move);

bool should_continue_iterating(int current_iteration_nodes, int previous_iteration_nodes,
                               std::chrono::steady_clock::time_point iteration_start_time,
                               std::chrono::steady_clock::time_point cutoff_time);

#endif // CLOCK_MANAGEMENT_H
