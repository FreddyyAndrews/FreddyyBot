#include "logging.h"

// Constructor for Singleton (initializing logFile via initialization list)
ThreadSafeLogger::ThreadSafeLogger(const std::string &filePath)
    : logFile(filePath, std::ios::app), logMutex() // Initialize logFile
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
