#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <thread>

class ThreadSafeLogger
{
private:
    std::ofstream logFile;
    std::mutex logMutex;

    // Private constructor for Singleton
    ThreadSafeLogger(const std::string &filePath);

public:
    // Deleted methods to prevent copying or moving the logger
    ThreadSafeLogger(const ThreadSafeLogger &) = delete;
    ThreadSafeLogger &operator=(const ThreadSafeLogger &) = delete;

    // Static method to get the singleton instance
    static ThreadSafeLogger &getInstance(const std::string &filePath = "logs/engine_log.txt");

    // Destructor to close the log file
    ~ThreadSafeLogger();

    // Atomic write method with log level and thread ID
    void write(const std::string &level, const std::string &message);

    void flush();
};

#endif // LOGGING_H
