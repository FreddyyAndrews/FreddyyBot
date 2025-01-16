#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <unordered_map>
#include <cstdint>
#include <stdexcept> // for std::runtime_error
#include "move.h"
#include "logging.h"
#include "zobrist_values.h"

/// Minimum depth at which we store positions
const int MIN_TRANSPOSITION_DEPTH = 2;
/// Maximum "age difference" after which old TT entries are pruned
const int OLDEST_AGE_TO_HOLD = 3;

/// Enumeration for the type of TT entry (Alpha, Beta, or PV)
enum class EntryType
{
    PV,   // Principal Variation
    Beta, // Beta cutoff
    Alpha // Alpha bound
};

/// A single row in the transposition table, describing a searched position.
struct TranspositionRow
{
    int eval;
    int depth;
    Move best_move;
    Move best_response;
    EntryType entry_type;
    int age;

    // Default constructor
    TranspositionRow();

    // Constructor with arguments
    TranspositionRow(int eval,
                     int depth,
                     const Move &best_move,
                     const Move &best_response,
                     EntryType entry_type,
                     int age);
};

/// A Transposition Table class that stores positions using a hash-to-TranspositionRow map.
class TranspositionTable
{
private:
    std::unordered_map<std::uint64_t, TranspositionRow> table;
    int current_age; ///< Increments on each "age_table()" call

public:
    /// Default constructor
    TranspositionTable();

    /**
     * \brief Insert (or update) an entry in the transposition table.
     * \param hash          Zobrist hash of the position
     * \param eval          Evaluation score
     * \param depth         Depth at which this score was obtained
     * \param best_move     Best move found at this position
     * \param best_response Best response move (aka ponder move)
     * \param entry_type    Enum indicating PV, Beta, or Alpha bound
     * \throws std::runtime_error If depth < MIN_TRANSPOSITION_DEPTH
     */
    void insert(std::uint64_t hash,
                int eval,
                int depth,
                const Move &best_move,
                const Move &best_response,
                const EntryType &entry_type);

    /**
     * \brief Retrieve an entry from the table if it exists, otherwise nullptr.
     * \param hash  Zobrist hash of the position
     * \return Pointer to the TranspositionRow or nullptr if not found
     */
    const TranspositionRow *get(std::uint64_t hash);

    /// Clear all entries and reset age to 0
    void reset_table();

    /// Increment the current_age by 1 (used to mark new search iterations)
    void age_table();

    /**
     * \brief Remove old entries whose age is more than OLDEST_AGE_TO_HOLD
     *        behind the current_age. Logs the removal count.
     */
    void maintain_table();
};

#endif // TRANSPOSITION_TABLE_H
