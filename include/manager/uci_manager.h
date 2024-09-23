#ifndef UCIMANAGER_H
#define UCIMANAGER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

class UCIManager
{
private:
  // Define variables to store the file descriptors for communication with the engines
  int to_white_fd, from_white_fd;
  int to_black_fd, from_black_fd;

  // Function to communicate with the engine via UCI protocol
  void communicate_with_engine(const std::string &color, int to_engine_fd, int from_engine_fd);

public:
  // Start the engine process
  pid_t start_engine(const std::string &engine_name, bool is_white);

  // Communicate with the white engine
  void communicate_with_white();

  // Communicate with the black engine
  void communicate_with_black();

  // Check if an engine is available
  bool is_engine_available(const std::string &engine_name);
};

#endif // UCIMANAGER_H
