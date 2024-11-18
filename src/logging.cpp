#include "logging.h"
#include <iostream>

const std::string game_num_file_path = "logs/game_num_tracker.txt";
const std::string log_file_base_path = "logs/engine_log_game_";

void increment_game_num()
{
    int game_num = get_game_num();
    game_num++;

    std::ofstream outFile(game_num_file_path);
    if (outFile.is_open())
    {
        outFile << game_num;
        outFile.close();
    }
    else
    {
        std::cerr << "Error: Unable to open game number file for writing." << std::endl;
    }
}

int get_game_num()
{
    int game_num = 0;
    std::ifstream inFile(game_num_file_path);
    if (inFile.is_open())
    {
        inFile >> game_num;
        inFile.close();
    }
    else
    {
        std::cerr << "Warning: Game number file not found. Starting from 0." << std::endl;
    }
    return game_num;
}

std::ofstream open_log_file()
{
    int game_num = get_game_num();
    std::string log_file_path = log_file_base_path + std::to_string(game_num) + ".txt";

    std::ofstream logFile(log_file_path, std::ios::app);
    if (!logFile.is_open())
    {
        std::cerr << "Error: Unable to open log file." << std::endl;
    }
    return logFile;
}
