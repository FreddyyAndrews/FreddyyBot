// transposition_table.cpp
#include "transposition_table.h"
#include <string>

// -----------------------
// TranspositionRow
// -----------------------
TranspositionRow::TranspositionRow()
    : eval(0), depth(0), age(0), best_move(), best_response(), entry_type(EntryType::Alpha)
{
}

TranspositionRow::TranspositionRow(int eval_,
                                   int depth_,
                                   const Move &best_move_,
                                   const Move &best_response_,
                                   EntryType entry_type_,
                                   int age_)
    : eval(eval_), depth(depth_), age(age_), best_move(best_move_), best_response(best_response_), entry_type(entry_type_)
{
}

// -----------------------
// TranspositionTable
// -----------------------
TranspositionTable::TranspositionTable()
    : table(), current_age(0), mutex_()
{
}

void TranspositionTable::insert(std::uint64_t hash,
                                int eval,
                                int depth,
                                const Move &best_move,
                                const Move &best_response,
                                EntryType entry_type)
{
    if (depth < MIN_TRANSPOSITION_DEPTH)
        throw std::runtime_error("Search not deep enough to store in TT.");

    std::unique_lock lock(mutex_);

    auto it = table.find(hash);
    if (it != table.end())
    {
        // refresh age
        it->second.age = current_age;
        // replace only if deeper
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
        table.emplace(hash,
                      TranspositionRow(eval,
                                       depth,
                                       best_move,
                                       best_response,
                                       entry_type,
                                       current_age));
    }
}

const TranspositionRow *TranspositionTable::get(std::uint64_t hash)
{
    std::unique_lock lock(mutex_);

    auto it = table.find(hash);
    if (it != table.end())
    {
        // refresh age
        it->second.age = current_age;
        return &it->second;
    }
    return nullptr;
}

void TranspositionTable::reset_table()
{
    std::unique_lock lock(mutex_);
    current_age = 0;
    table.clear();
}

void TranspositionTable::age_table()
{
    std::unique_lock lock(mutex_);
    ++current_age;
}

void TranspositionTable::maintain_table()
{
    std::unique_lock lock(mutex_);
    auto &logger = ThreadSafeLogger::getInstance("logs/app_log.txt");

    size_t initial_size = table.size();
    logger.write("Debug",
                 "Before prune: " + std::to_string(initial_size) +
                     " entries in TT");

    size_t deleted = 0;
    for (auto it = table.begin(); it != table.end();)
    {
        if (current_age - it->second.age > OLDEST_AGE_TO_HOLD)
        {
            it = table.erase(it);
            ++deleted;
        }
        else
        {
            ++it;
        }
    }

    logger.write("Debug",
                 "Pruned " + std::to_string(deleted) +
                     " old entries; remaining " +
                     std::to_string(table.size()));
}
