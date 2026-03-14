#include <atomic>
#include <doctest/doctest.h>
#include <random>
#include <vector>

#include "revenant/core/block_index.hpp"

using namespace rvn::core;

TEST_CASE("Property: BlockIndex entries always valid or unset") {
    std::vector<std::atomic<std::uint32_t>> memory(16);
    for (auto& a : memory)
        a.store(BlockIndex::kUnset, std::memory_order_relaxed);

    std::uint32_t block_size_log2 = 12;                                 // 4KB blocks
    std::uint64_t capacity = memory.size() * (1ULL << block_size_log2); // 64KB total
    BlockIndex index(memory, block_size_log2);

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<std::uint64_t> dist(0, 1000 * capacity);

    for (int i = 0; i < 10000; ++i) {
        std::uint64_t pos = dist(rng);
        index.set(pos);

        std::uint64_t block_number = pos >> block_size_log2;
        std::uint32_t block_idx = static_cast<std::uint32_t>(block_number % memory.size());
        std::uint32_t expected_gen = static_cast<std::uint32_t>(block_number / memory.size());
        std::uint32_t expected_offset =
            static_cast<std::uint32_t>(pos & ((1ULL << block_size_log2) - 1));

        auto res = index.get(block_idx, expected_gen);
        REQUIRE(res.has_value());
        CHECK(*res == expected_offset);

        // Check with wrong generation
        auto wrong_gen = expected_gen + 1;
        auto res_wrong = index.get(block_idx, wrong_gen);
        CHECK(!res_wrong.has_value());
    }
}
