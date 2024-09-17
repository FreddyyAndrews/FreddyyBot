// uci_manager.h

#ifndef UCI_MANAGER_H
#define UCI_MANAGER_H

#include <string>
#include <unistd.h> // For pid_t

class UCIManager
{
private:
    // File descriptors for communication with the white and black engines
    int to_white_fd;
    int from_white_fd;
    int to_black_fd;
    int from_black_fd;

    // Function to communicate with the engine via UCI protocol
    void communicate_with_engine(const std::string &color);

public:
    // Start the engine process
    pid_t start_engine(const std::string &engine_name, bool is_white);

    // Check if an engine is available
    bool is_engine_available(const std::string &engine_name);

    // Communicate with white
    void communicate_with_white();

    // Communicate with black
    void communicate_with_black();
};

#endif // UCI_MANAGER_H
