#include <atomic>
#include <doctest/doctest.h>
#include <vector>

#include "revenant/core/block_index.hpp"

using namespace rvn::core;

TEST_CASE("BlockIndex: unset entry returns nullopt") {
    std::vector<std::atomic<std::uint32_t>> memory(4);
    for (auto& a : memory)
        a.store(BlockIndex::kUnset, std::memory_order_relaxed);

    BlockIndex index(memory, 16); // 64KB blocks
    auto res = index.get(0, 0);   // block 0, expected gen 0
    CHECK(!res.has_value());
}

TEST_CASE("BlockIndex: sets and gets correctly with generation tag") {
    std::vector<std::atomic<std::uint32_t>> memory(4);
    for (auto& a : memory)
        a.store(BlockIndex::kUnset, std::memory_order_relaxed);

    BlockIndex index(memory, 16); // 64KB blocks, total capacity 256KB

    // Position 0x10005 (block 1, offset 5, gen 0)
    index.set(0x10005);

    auto res = index.get(1, 0);
    REQUIRE(res.has_value());
    CHECK(*res == 5);

    // Should not overwrite if not first in block
    // Actually, set_if_first relies on the caller to only call it when crossing a block boundary,
    // or we can make it idempotent. Let's just test `set`.
    index.set(0x10050);
    auto res2 = index.get(1, 0);
    REQUIRE(res2.has_value());
    CHECK(*res2 == 0x50); // Overwritten by set
}

TEST_CASE("BlockIndex: generation mismatch returns nullopt") {
    std::vector<std::atomic<std::uint32_t>> memory(4);
    for (auto& a : memory)
        a.store(BlockIndex::kUnset, std::memory_order_relaxed);

    BlockIndex index(memory, 16);
    index.set(0x50005); // block 1 (0x50000 / 0x10000 = 5. 5 % 4 = 1). Gen = 5 / 4 = 1. Offset = 5.

    // Expected gen 0 -> mismatch
    auto res = index.get(1, 0);
    CHECK(!res.has_value());

    // Expected gen 1 -> match
    auto res_match = index.get(1, 1);
    REQUIRE(res_match.has_value());
    CHECK(*res_match == 5);
}
