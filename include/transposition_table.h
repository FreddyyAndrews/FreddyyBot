// transposition_table.h
#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <unordered_map>
#include <cstdint>
#include <stdexcept>
#include <shared_mutex> // C++17 shared_mutex
#include "move.h"
#include "logging.h"
#include "zobrist_values.h"

/// Minimum depth at which we store positions
static constexpr int MIN_TRANSPOSITION_DEPTH = 2;
/// Maximum "age difference" after which old TT entries are pruned
static constexpr int OLDEST_AGE_TO_HOLD = 3;

enum class EntryType
{
    PV,
    Beta,
    Alpha
};

struct TranspositionRow
{
    int eval;
    int depth;
    int age;
    Move best_move;
    Move best_response;
    EntryType entry_type;

    TranspositionRow();
    TranspositionRow(int eval,
                     int depth,
                     const Move &best_move,
                     const Move &best_response,
                     EntryType entry_type,
                     int age);
};

class TranspositionTable
{
private:
    std::unordered_map<std::uint64_t, TranspositionRow> table;
    int current_age;
    mutable std::shared_mutex mutex_; // protects both table and current_age

public:
    TranspositionTable();

    /// Insert or update; thread-safe
    void insert(std::uint64_t hash,
                int eval,
                int depth,
                const Move &best_move,
                const Move &best_response,
                EntryType entry_type);

    /// Retrieve (and refresh age); thread-safe
    const TranspositionRow *get(std::uint64_t hash);

    /// Clear all entries; thread-safe
    void reset_table();

    /// Bump the global age; thread-safe
    void age_table();

    /// Prune old entries; thread-safe
    void maintain_table();
};

#endif // TRANSPOSITION_TABLE_H
