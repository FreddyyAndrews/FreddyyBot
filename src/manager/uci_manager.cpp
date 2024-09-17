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
  void communicate_with_engine(const std::string &color, int to_engine_fd, int from_engine_fd)
  {
    // File pointers for writing to and reading from the engine
    FILE *to_engine = fdopen(to_engine_fd, "w");
    FILE *from_engine = fdopen(from_engine_fd, "r");

    // Check if the file pointers were created successfully
    if (!to_engine || !from_engine)
    {
      perror("fdopen");
      exit(EXIT_FAILURE);
    }

    // Send the UCI command to the engine
    fprintf(to_engine, "uci\n");
    // Ensure that the command is sent to the engine immediately by flushing the buffer
    fflush(to_engine);

    // Allocate buffer to store the response from the engine
    char buffer[256];
    bool uciok_received = false;

    // Read the response from the engine into the buffer
    while (fgets(buffer, sizeof(buffer), from_engine))
    {
      std::string response(buffer);
      // Check if the response contains "uciok"
      if (response.find("uciok") != std::string::npos)
      {
        // If "uciok" is found, print a message and set the flag to true
        std::cout << color << " replied with uciok" << std::endl;
        uciok_received = true;
        break;
      }
    }

    if (!uciok_received)
    {
      std::cout << color << " did not reply with uciok" << std::endl;
    }

    // Close the file pointers
    fclose(to_engine);
    fclose(from_engine);
  }

public:
  // Start the engine process
  pid_t start_engine(const std::string &engine_name, bool is_white)
  {
    // Define variables to store the file descriptors for the pipes
    int to_engine[2];
    int from_engine[2];

    // Create the pipes
    if (pipe(to_engine) != 0 || pipe(from_engine) != 0)
    {
      perror("pipe");
      exit(EXIT_FAILURE);
    }

    // Fork the process
    pid_t pid = fork();

    // Check if the fork was successful
    if (pid == -1)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    // If the fork was successful, check if the process is the child process
    else if (pid == 0)
    {
      // Duplicate read end of the pipe to stdin and write end of the pipe to stdout
      dup2(to_engine[0], STDIN_FILENO);
      dup2(from_engine[1], STDOUT_FILENO);

      // After redirection, close the original file descriptors
      close(to_engine[0]);
      close(to_engine[1]);
      close(from_engine[0]);
      close(from_engine[1]);

      // Replace the child process with the engine
      execlp(engine_name.c_str(), engine_name.c_str(), (char *)NULL);
      // If the engine cannot be started, print an error message and exit
      perror("execlp");
      exit(EXIT_FAILURE);
    }
    else
    {
      // Parent process
      close(to_engine[0]);
      close(from_engine[1]);

      if (is_white)
      {
        to_white_fd = to_engine[1];
        from_white_fd = from_engine[0];
      }
      else
      {
        to_black_fd = to_engine[1];
        from_black_fd = from_engine[0];
      }

      return pid;
    }
  }

  // Communicate with the white engine
  void communicate_with_white()
  {
    communicate_with_engine("White", to_white_fd, from_white_fd);
  }

  // Communicate with the black engine
  void communicate_with_black()
  {
    communicate_with_engine("Black", to_black_fd, from_black_fd);
  }

  bool is_engine_available(const std::string &engine_name)
  {
    return engine_name == "stockfish";
  }
};