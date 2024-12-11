#include "logging.h"
#include <iostream>

const std::string log_file_path = "logs/engine_log.txt";

std::ofstream open_log_file()
{

    std::ofstream logFile(log_file_path, std::ios::app);
    if (!logFile.is_open())
    {
        std::cerr << "Error: Unable to open log file." << std::endl;
    }
    return logFile;
}
