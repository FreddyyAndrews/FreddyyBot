#include "board_representation.h"
#include "evaluation.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sstream>
#include <atomic>

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

// Helper function to stop and join the pondering thread
void stopPondering(std::thread &ponder_thread, std::atomic<bool> &stop_pondering, ThreadSafeLogger &logger)
{
  if (ponder_thread.joinable())
  {
    logger.write("Debug", "Stopping pondering thread...");
    stop_pondering = true; // Signal the thread to stop

    auto start_time = std::chrono::steady_clock::now();

    // Join the thread to ensure it has finished execution
    ponder_thread.join();

    std::ostringstream oss;
    oss << "Pondering thread stopped in "
        << std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_time)
               .count()
        << " ms";
    logger.write("Debug", oss.str());
  }
}

// Main function to handle UCI communication
int main()
{
  BoardRepresentation board_representation;
  std::string input;
  Move best_move, ponder_move;

  // Pondering thread control
  std::thread ponder_thread;
  std::atomic<bool> stop_pondering(false);
  std::mutex ponder_mutex;
  Move next_ponder_move, best_move_pondered;

  ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

  while (true)
  {
    std::getline(std::cin, input);
    if (input.empty())
      continue;

    // Log the input
    logger.write("Input", input);

    std::vector<std::string> tokens = split(input, ' ');

    // Stop pondering for all commands except "ponderhit"
    if (tokens[0] != "ponderhit" && tokens[0] != "stop")
    {
      stopPondering(ponder_thread, stop_pondering, logger);
    }

    if (tokens[0] == "uci")
    {
      std::cout << "uciok" << std::endl;
      logger.write("Output", "uciok");
    }
    if (tokens[0] == "ucinewgame")
    {
      continue;
    }
    else if (tokens[0] == "isready")
    {
      std::cout << "readyok" << std::endl;
      logger.write("Output", "readyok");
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
        logger.write("Error", "Invalid position command");
      }
    }
    else if (tokens[0] == "go" && (tokens.size() == 1 || tokens[1] != "ponder")) // Move search, no ponder
    {
      int wtime = 30000, btime = 30000, winc = 0, binc = 0;

      // Parse the input tokens for time controls
      if (tokens.size() >= 5 && tokens[1] == "wtime" && tokens[3] == "btime")
      {
        wtime = std::stoi(tokens[2]);
        btime = std::stoi(tokens[4]);

        if (tokens.size() == 9 && tokens[5] == "winc" && tokens[7] == "binc")
        {
          winc = std::stoi(tokens[6]);
          binc = std::stoi(tokens[8]);
        }
      }

      // Call find_best_move with the parsed time controls
      best_move = find_best_move(board_representation, ponder_move, true, wtime, btime, winc, binc).best_move;

      if (!ponder_move.is_instantiated())
      {
        throw std::runtime_error("Ponder move not instantiated");
      }

      if (!best_move.is_instantiated())
      {
        throw std::runtime_error("Best move not instantiated");
      }

      std::cout << "bestmove " << best_move.to_UCI() << " ponder " << ponder_move.to_UCI() << std::endl;

      std::ostringstream oss;
      oss << "bestmove " << best_move.to_UCI() << " ponder " << ponder_move.to_UCI();
      logger.write("Output", oss.str());
    }
    else if (tokens[0] == "go" && tokens[1] == "ponder") // GUI asks to ponder the position
    {
      stop_pondering = false;
      ponder_thread = std::thread(ponder,
                                  std::ref(board_representation), // Pass by const reference
                                  std::ref(next_ponder_move),     // Pass by reference
                                  std::ref(best_move_pondered),   // Pass by reference
                                  true,                           // Pass by value
                                  std::cref(stop_pondering));     // Pass atomic<bool> by const reference
    }
    else if (tokens[0] == "ponderhit" || tokens[0] == "stop")
    {
      // Ensure the pondering thread finishes
      stopPondering(ponder_thread, stop_pondering, logger);

      {
        std::lock_guard<std::mutex> lock(ponder_mutex);
        best_move = best_move_pondered;
        ponder_move = next_ponder_move;
      }

      std::cout << "bestmove " << best_move.to_UCI() << " ponder " << ponder_move.to_UCI() << std::endl;

      std::ostringstream oss;
      oss << "bestmove " << best_move.to_UCI() << " ponder " << ponder_move.to_UCI();
      logger.write("Output", oss.str());
    }
    else if (tokens[0] == "quit")
    {
      break;
    }
    else
    {
      std::cerr << "Error: Unknown command" << std::endl;
      logger.write("Error", "Unknown command");
    }

    logger.flush();
  }

  logger.flush();
  return 0;
}
