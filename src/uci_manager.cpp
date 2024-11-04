#include "uci_manager.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Implementation of UCIManager methods

void UCIManager::communicate_with_engine(const std::string &color, int to_engine_fd, int from_engine_fd)
{
  FILE *to_engine = fdopen(to_engine_fd, "w");
  FILE *from_engine = fdopen(from_engine_fd, "r");

  if (!to_engine || !from_engine)
  {
    perror("fdopen");
    exit(EXIT_FAILURE);
  }

  fprintf(to_engine, "uci\n");
  fflush(to_engine);

  char buffer[256];
  bool uciok_received = false;

  while (fgets(buffer, sizeof(buffer), from_engine))
  {
    std::string response(buffer);
    if (response.find("uciok") != std::string::npos)
    {
      std::cout << color << " replied with uciok" << std::endl;
      uciok_received = true;
      break;
    }
  }

  if (!uciok_received)
  {
    std::cout << color << " did not reply with uciok" << std::endl;
  }

  fclose(to_engine);
  fclose(from_engine);
}

pid_t UCIManager::start_engine(const std::string &engine_name, bool is_white)
{
  int to_engine[2];
  int from_engine[2];

  if (pipe(to_engine) != 0 || pipe(from_engine) != 0)
  {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();

  if (pid == -1)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    dup2(to_engine[0], STDIN_FILENO);
    dup2(from_engine[1], STDOUT_FILENO);

    close(to_engine[0]);
    close(to_engine[1]);
    close(from_engine[0]);
    close(from_engine[1]);

    execlp(engine_name.c_str(), engine_name.c_str(), (char *)NULL);
    perror("execlp");
    exit(EXIT_FAILURE);
  }
  else
  {
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

void UCIManager::communicate_with_white()
{
  communicate_with_engine("White", to_white_fd, from_white_fd);
}

void UCIManager::communicate_with_black()
{
  communicate_with_engine("Black", to_black_fd, from_black_fd);
}

bool UCIManager::is_engine_available(const std::string &engine_name)
{
  return engine_name == "stockfish";
}
