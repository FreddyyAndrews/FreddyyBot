#ifndef THREEFOLD_MAP_H
#define THREEFOLD_MAP_H

#include <unordered_map>
#include <cstdint>

class ThreefoldMap
{
private:
    std::unordered_map<std::uint64_t, int> frequency_map_;

public:
    // Default constructor
    ThreefoldMap() : frequency_map_() {}

    /**
     * @brief Increments the frequency of a given hash key.
     *        If the key does not exist, it is created with frequency 1.
     *
     * @param hashKey The Zobrist hash or position key.
     */
    void increment(std::uint64_t hashKey)
    {
        auto it = frequency_map_.find(hashKey);
        if (it == frequency_map_.end())
        {
            frequency_map_[hashKey] = 1;
        }
        else
        {
            it->second++;
        }
    }

    /**
     * @brief Decrements the frequency of a given hash key.
     *        If the frequency falls to 0, the key is erased from the map.
     *
     * @param hashKey The Zobrist hash or position key.
     */
    void decrement(std::uint64_t hashKey)
    {
        auto it = frequency_map_.find(hashKey);
        if (it == frequency_map_.end())
        {
            // Optionally handle the error or ignore if it's acceptable
            return;
        }
        it->second--;
        if (it->second <= 0)
        {
            frequency_map_.erase(it);
        }
    }

    /**
     * @brief Retrieves the current frequency for a given hash key.
     *        Returns 0 if the key is not present in the map.
     *
     * @param hashKey The Zobrist hash or position key.
     * @return int The current frequency of the hash key.
     */
    int getFrequency(std::uint64_t hashKey) const
    {
        auto it = frequency_map_.find(hashKey);
        return (it != frequency_map_.end()) ? it->second : 0;
    }

    /**
     * @brief Checks whether a given hash key has appeared at least three times.
     *
     * @param hashKey The Zobrist hash or position key.
     * @return true if frequency >= 3, false otherwise.
     */
    bool hasThreefold(std::uint64_t hashKey) const
    {
        auto it = frequency_map_.find(hashKey);
        return (it != frequency_map_.end() && it->second >= 3);
    }

    /**
     * @brief Checks if any key in the map has a frequency of at least 3.
     *
     * @return true if at least one key has a frequency >= 3, false otherwise.
     */
    bool hasAnyThreefold() const
    {
        for (const auto &[hash, count] : frequency_map_)
        {
            if (count >= 3)
            {
                return true;
            }
        }
        return false;
    }

    // Expose iterators to allow range-based for loops
    auto begin() const { return frequency_map_.begin(); }
    auto end() const { return frequency_map_.end(); }
    auto begin() { return frequency_map_.begin(); }
    auto end() { return frequency_map_.end(); }
};

#endif // THREEFOLD_MAP_H
