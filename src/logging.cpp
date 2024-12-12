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

// Atomic write method with log level and thread ID
void ThreadSafeLogger::write(const std::string &level, const std::string &message)
{
    std::lock_guard<std::mutex> lock(logMutex); // Lock the mutex
    logFile << "[" << level << "]: " << message << std::endl;
}

// Flush method to ensure all data is written to the file
void ThreadSafeLogger::flush()
{
    std::lock_guard<std::mutex> lock(logMutex); // Lock the mutex
    logFile.flush();                            // Flush the buffer
}

void ThreadSafeLogger::clear()
{
    std::lock_guard<std::mutex> lock(logMutex); // Ensure thread safety
    logFile.close();                            // Close the current log file
    logFile.open(logFileName, std::ios::trunc); // Open in truncate mode to clear contents
    if (!logFile.is_open())
    {
        throw std::ios_base::failure("Error: Unable to clear log file.");
    }
    logFile.close();                          // Close after truncating
    logFile.open(logFileName, std::ios::app); // Reopen in append mode for further logging
    if (!logFile.is_open())
    {
        throw std::ios_base::failure("Error: Unable to reopen log file after clearing.");
    }
}
