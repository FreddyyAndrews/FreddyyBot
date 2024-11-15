#include "board_representation.h"
#include "evaluation.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream> // Included for file operations

// Function to split a string by spaces
std::vector<std::string> split(const std::string &str, char delimiter)
{
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delimiter))
  {
    tokens.push_back(token);
  }
  return tokens;
}

// Main function to handle UCI communication
int main(int argc, char **argv)
{
  BoardRepresentation board_representation;
  std::string input;

  // Open the log file in append mode
  std::ofstream log_file("engine_log.txt", std::ios::app);
  if (!log_file)
  {
    std::cerr << "Error: Unable to open log file." << std::endl;
    return 1;
  }

  while (true)
  {
    std::getline(std::cin, input);
    if (input.empty())
      continue;

    // Log the input
    log_file << "Received: " << input << std::endl;

    std::vector<std::string> tokens = split(input, ' ');

    if (tokens[0] == "uci")
    {
      std::cout << "uciok" << std::endl;

      // Log the output
      log_file << "Sent: uciok" << std::endl;
    }
    else if (tokens[0] == "isready")
    {
      std::cout << "readyok" << std::endl;

      // Log the output
      log_file << "Sent: readyok" << std::endl;
    }
    else if (tokens[0] == "position")
    {
      // Handle setting the board position
      if (tokens[1] == "startpos")
      {
        board_representation = BoardRepresentation();
        if (tokens.size() > 2 && tokens[2] == "moves")
        {
          for (size_t i = 3; i < tokens.size(); ++i)
          {
            board_representation.make_move(tokens[i]);
          }
        }
      }
      else if (tokens[1] == "fen")
      {
        std::string fen;
        for (size_t i = 2; i < tokens.size() && tokens[i] != "moves"; ++i)
        {
          fen += tokens[i] + " ";
        }
        board_representation = BoardRepresentation(fen);
        size_t moves_index = 0;
        for (size_t i = 2; i < tokens.size(); ++i)
        {
          if (tokens[i] == "moves")
          {
            moves_index = i;
            break;
          }
        }
        if (moves_index > 0)
        {
          for (size_t i = moves_index + 1; i < tokens.size(); ++i)
          {
            board_representation.make_move(tokens[i]);
          }
        }
      }
      else
      {
        std::cerr << "Error: Invalid position command" << std::endl;

        // Log the error
        log_file << "Error: Invalid position command" << std::endl;
      }
    }
    else if (tokens[0] == "go")
    {
      if (tokens.size() == 1)
      {
        Evaluation best_move = find_best_move(board_representation);
        std::cout << "bestmove " << best_move.best_move.to_UCI() << std::endl;

        // Log the output
        log_file << "Sent: bestmove " << best_move.best_move.to_UCI() << std::endl;
        log_file << "My eval: " << best_move.evaluation << std::endl;
      }
      else if (tokens.size() == 5 && tokens[1] == "wtime" && tokens[3] == "btime")
      {
        int wtime = std::stoi(tokens[2]);
        int btime = std::stoi(tokens[4]);
        Evaluation best_move = find_best_move(board_representation, wtime, btime);
        std::cout << "bestmove " << best_move.best_move.to_UCI() << std::endl;

        // Log the output
        log_file << "Sent: bestmove " << best_move.best_move.to_UCI() << std::endl;
        log_file << "My eval: " << best_move.evaluation << std::endl;
      }
      else if (tokens.size() == 9 && tokens[1] == "wtime" && tokens[3] == "btime" && tokens[5] == "winc" && tokens[7] == "binc")
      {
        int wtime = std::stoi(tokens[2]);
        int btime = std::stoi(tokens[4]);
        int winc = std::stoi(tokens[6]);
        int binc = std::stoi(tokens[8]);

        // Here, you would add your logic for the engine to calculate the best move based on the provided time control.
        // For now, just output a dummy best move.
        Evaluation best_move = find_best_move(board_representation, wtime, btime, winc, binc);
        std::cout << "bestmove " << best_move.best_move.to_UCI() << std::endl;

        // Log the output
        log_file << "Sent: bestmove " << best_move.best_move.to_UCI() << std::endl;
        log_file << "My eval: " << best_move.evaluation << std::endl;
      }
      else
      {
        std::cerr << "Error: Invalid go command format" << std::endl;

        // Log the error
        log_file << "Error: Invalid go command format" << std::endl;
      }
    }
    else if (tokens[0] == "quit")
    {
      break;
    }
    else if (tokens[0] == "ucinewgame")
    {
      continue;
    }
    else
    {
      std::cerr << "Error: Unknown command" << std::endl;

      // Log the error
      log_file << "Error: Unknown command" << std::endl;
    }

    // Flush the log file to ensure data is written
    log_file.flush();
  }

  // Close the log file
  log_file.close();

  return 0;
}
