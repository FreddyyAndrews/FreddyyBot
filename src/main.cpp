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
void stopPondering(std::thread &ponder_thread,
                   std::atomic<bool> &stop_condition,
                   ThreadSafeLogger &logger)
{
  if (ponder_thread.joinable())
  {
    logger.write("Debug", "Stopping pondering thread...");
    stop_condition = true; // Signal the thread to stop

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
  std::atomic<bool> hard_stop_pondering(false);
  std::atomic<bool> ponder_hit(false);
  std::mutex ponder_mutex;
  Move next_ponder_move, best_move_pondered;

  ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

  try
  {
    while (true)
    {
      std::getline(std::cin, input);
      if (input.empty())
        continue;

      logger.write("Input", input);
      std::vector<std::string> tokens = split(input, ' ');

      // Stop pondering for all commands except "ponderhit" or "stop"
      if (tokens[0] != "ponderhit" && tokens[0] != "stop")
      {
        stopPondering(ponder_thread, hard_stop_pondering, logger);
      }

      if (tokens[0] == "uci")
      {
        std::cout << "uciok" << std::endl;
        logger.write("Output", "uciok");
      }
      if (tokens[0] == "ucinewgame")
      {
        logger.clear();
        continue;
      }
      else if (tokens[0] == "isready")
      {
        std::cout << "readyok" << std::endl;
        logger.write("Output", "readyok");
      }
      else if (tokens[0] == "position")
      {
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
          // Collect all parts of the FEN until we reach the "moves" token
          std::string fen;
          size_t moves_index = 0;
          for (size_t i = 2; i < tokens.size(); ++i)
          {
            if (tokens[i] == "moves")
            {
              moves_index = i;
              break;
            }
            fen += tokens[i] + " ";
          }

          board_representation = BoardRepresentation(fen);

          // If we found the "moves" token, apply those moves
          if (moves_index != 0 && moves_index < tokens.size())
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
      else if (tokens[0] == "go" && (tokens.size() == 1 || tokens[1] != "ponder"))
      {
        int wtime = 30000, btime = 30000, winc = 0, binc = 0;

        if (tokens.size() >= 5 && tokens[1] == "wtime" && tokens[3] == "btime")
        {
          wtime = std::stoi(tokens[2]);
          btime = std::stoi(tokens[4]);

          if (tokens.size() >= 9 && tokens[5] == "winc" && tokens[7] == "binc")
          {
            winc = std::stoi(tokens[6]);
            binc = std::stoi(tokens[8]);
          }
        }

        int forced_time;
        if (tokens.size() == 1)
        {
          forced_time = DEFAULT_SEARCH_TIME_MS;
        }
        else
        {
          forced_time = -1;
        }

        Evaluation position_evaluation =
            find_best_move(board_representation, true,
                           wtime, btime, winc, binc,
                           forced_time);

        best_move = position_evaluation.best_move;
        ponder_move = position_evaluation.ponder_move;

        if (!best_move.is_instantiated())
        {
          throw std::runtime_error("Best move not instantiated");
        }

        if (ponder_move.is_instantiated())
        {
          std::cout << "bestmove " << best_move.to_UCI()
                    << " ponder " << ponder_move.to_UCI() << std::endl;
          std::ostringstream oss;
          oss << "bestmove " << best_move.to_UCI()
              << " ponder " << ponder_move.to_UCI();
          logger.write("Output", oss.str());
        }
        else
        {
          std::cout << "bestmove " << best_move.to_UCI() << std::endl;
          std::ostringstream oss;
          oss << "bestmove " << best_move.to_UCI();
          logger.write("Output", oss.str());
        }
      }
      else if (tokens[0] == "go" && tokens[1] == "ponder")
      {
        ponder_hit = false;
        hard_stop_pondering = false;

        // Parse time arguments similarly to the normal 'go' command
        // Format: go ponder wtime <int> btime <int> [winc <int> binc <int>]
        int wtime = 30000, btime = 30000, winc = 0, binc = 0;
        int forced_time = -1;

        // tokens: ["go", "ponder", "wtime", "<val>", "btime", "<val>", "winc", "<val>", "binc", "<val>", ...]
        // Make sure we have enough tokens to parse
        if (tokens.size() >= 5 && tokens[2] == "wtime" && tokens[4] == "btime")
        {
          wtime = std::stoi(tokens[3]);
          btime = std::stoi(tokens[5]);

          // If we have at least 9 tokens, we might have winc/binc
          if (tokens.size() >= 9 && tokens[6] == "winc" && tokens[8] == "binc")
          {
            winc = std::stoi(tokens[7]);
            binc = std::stoi(tokens[9]);
          }
          // If we have an 11th token, it might be forced_time, or it can be set manually
          if (tokens.size() >= 11)
          {
            forced_time = std::stoi(tokens[10]);
          }
        }
        else
        {
          // If no arguments are provided, just default forced_time
          forced_time = DEFAULT_SEARCH_TIME_MS;
        }

        logger.write("Debug", "Started pondering.");

        ponder_thread = std::thread(ponder,
                                    std::ref(board_representation),
                                    std::ref(next_ponder_move),
                                    std::ref(best_move_pondered),
                                    true,
                                    std::cref(ponder_hit),
                                    std::cref(hard_stop_pondering),
                                    wtime,
                                    btime,
                                    winc,
                                    binc,
                                    forced_time);
      }
      else if (tokens[0] == "ponderhit" || tokens[0] == "stop")
      {
        std::atomic<bool> &stop_condition = (tokens[0] == "ponderhit") ? ponder_hit : hard_stop_pondering;
        stopPondering(ponder_thread, stop_condition, logger);

        {
          std::lock_guard<std::mutex> lock(ponder_mutex);
          best_move = best_move_pondered;
          ponder_move = next_ponder_move;
        }

        if (!best_move.is_instantiated())
        {
          throw std::runtime_error("Best move not instantiated");
        }

        if (ponder_move.is_instantiated())
        {
          std::cout << "bestmove " << best_move.to_UCI()
                    << " ponder " << ponder_move.to_UCI() << std::endl;
          std::ostringstream oss;
          oss << "bestmove " << best_move.to_UCI()
              << " ponder " << ponder_move.to_UCI();
          logger.write("Output", oss.str());
        }
        else
        {
          std::cout << "bestmove " << best_move.to_UCI() << std::endl;
          std::ostringstream oss;
          oss << "bestmove " << best_move.to_UCI();
          logger.write("Output", oss.str());
        }
      }
      else if (tokens[0] == "quit")
      {
        break;
      }
      else
      {
        logger.write("Warning", "Unknown command");
      }

      logger.flush();
    }
  }
  catch (const std::exception &e)
  {
    logger.write("ERROR", e.what());
    logger.flush();
    return 1;
  }

  logger.flush();
  return 0;
}
