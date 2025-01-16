#include "logging.h"

// Constructor for Singleton (initializing logFile via initialization list)
ThreadSafeLogger::ThreadSafeLogger(const std::string &filePath)
    : logFile(filePath, std::ios::app), logMutex(), logFileName(filePath) // Initialize logFile
{
    if (!logFile.is_open())
    {
        throw std::ios_base::failure("Error: Unable to open log file.");
    }
}

// Static method to get the singleton instance
ThreadSafeLogger &ThreadSafeLogger::getInstance(const std::string &filePath)
{
    static ThreadSafeLogger instance(filePath);
    return instance;
}

// Destructor to close the log file
ThreadSafeLogger::~ThreadSafeLogger()
{
    if (logFile.is_open())
    {
        logFile.close();
    }
}

// Atomic write method with log level
void ThreadSafeLogger::write(const std::string &level, const std::string &message)
{
    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto local_time = *std::localtime(&time_t_now); // Convert to local time

    // Format the time
    std::ostringstream time_stream;
    time_stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");

    // Lock the mutex and write to the log
    std::lock_guard<std::mutex> lock(logMutex);
    logFile << "[" << time_stream.str() << "] [" << level << "]: " << message << std::endl;
}

// Flush method to ensure all data is written to the file
void ThreadSafeLogger::flush()
{
    std::lock_guard<std::mutex> lock(logMutex); // Lock the mutex
    logFile.flush();                            // Flush the buffer
}

void ThreadSafeLogger::clear()
{
    const std::size_t MAX_LOG_LINES = 1000;     // Use std::size_t for consistency
    std::lock_guard<std::mutex> lock(logMutex); // Ensure thread safety

    // Open the file for reading
    std::ifstream inFile(logFileName);
    if (!inFile.is_open())
    {
        throw std::ios_base::failure("Error: Unable to open log file for reading.");
    }

    // Read all lines into a vector
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line))
    {
        lines.push_back(line);
    }
    inFile.close(); // Close the input file

    // Open the file for writing (truncate mode)
    logFile.close();                            // Close current output file
    logFile.open(logFileName, std::ios::trunc); // Open in truncate mode
    if (!logFile.is_open())
    {
        throw std::ios_base::failure("Error: Unable to clear log file.");
    }

    // Write only the last MAX_LOG_LINES back to the file
    const std::size_t total_lines = lines.size(); // std::size_t to match vector's size type
    const std::size_t start = (total_lines > MAX_LOG_LINES) ? (total_lines - MAX_LOG_LINES) : 0;

    for (std::size_t i = start; i < total_lines; ++i)
    {
        logFile << lines[i] << std::endl;
    }

    logFile.close();                          // Close the file after truncating
    logFile.open(logFileName, std::ios::app); // Reopen in append mode
    if (!logFile.is_open())
    {
        throw std::ios_base::failure("Error: Unable to reopen log file after clearing.");
    }
}
