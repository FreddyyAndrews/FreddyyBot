#include "transposition_table.h"
#include <string> // for std::to_string

// -----------------------
// TranspositionRow
// -----------------------
TranspositionRow::TranspositionRow()
    : eval(0),
      depth(0),
      best_move(Move()),
      best_response(Move()),
      entry_type(EntryType::Alpha),
      age(0)
{
}

TranspositionRow::TranspositionRow(int eval,
                                   int depth,
                                   const Move &best_move,
                                   const Move &best_response,
                                   EntryType entry_type,
                                   int age)
    : eval(eval),
      depth(depth),
      best_move(best_move),
      best_response(best_response),
      entry_type(entry_type),
      age(age)
{
}

// -----------------------
// TranspositionTable
// -----------------------
TranspositionTable::TranspositionTable()
    : table(), current_age(0)
{
}

void TranspositionTable::insert(std::uint64_t hash,
                                int eval,
                                int depth,
                                const Move &best_move,
                                const Move &best_response,
                                const EntryType &entry_type)
{
    if (depth < MIN_TRANSPOSITION_DEPTH)
    {
        throw std::runtime_error("Search not deep enough to store in TT.");
    }

    auto it = table.find(hash);
    if (it != table.end())
    {
        // If entry already exists, refresh its age
        it->second.age = current_age;

        // Overwrite if the new depth is greater
        if (depth > it->second.depth)
        {
            it->second.eval = eval;
            it->second.depth = depth;
            it->second.best_move = best_move;
            it->second.best_response = best_response;
            it->second.entry_type = entry_type;
        }
    }
    else
    {
        // Insert a brand new entry
        TranspositionRow row(eval, depth, best_move, best_response, entry_type, current_age);
        table[hash] = row;
    }
}

const TranspositionRow *TranspositionTable::get(std::uint64_t hash)
{
    auto it = table.find(hash);
    if (it != table.end())
    {
        // Refresh the age on access
        it->second.age = current_age;
        return &it->second;
    }
    return nullptr;
}

void TranspositionTable::reset_table()
{
    current_age = 0;
    table.clear();
}

void TranspositionTable::age_table()
{
    // Mark we're entering a new "search iteration" or generation
    current_age++;
}

void TranspositionTable::maintain_table()
{
    ThreadSafeLogger &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

    size_t initial_size = table.size();
    logger.write("Debug", "There are currently " + std::to_string(initial_size) +
                              " unique positions in the transposition table.");

    size_t deleted_count = 0;

    for (auto it = table.begin(); it != table.end();)
    {
        // If this entry's age is too old, remove it
        if (current_age - it->second.age > OLDEST_AGE_TO_HOLD)
        {
            it = table.erase(it);
            ++deleted_count;
        }
        else
        {
            ++it;
        }
    }

    logger.write("Debug", "Deleted " + std::to_string(deleted_count) +
                              " positions from the transposition table.");

    size_t final_size = table.size();
    logger.write("Debug", "There are now " + std::to_string(final_size) +
                              " unique positions in the transposition table.");
}
