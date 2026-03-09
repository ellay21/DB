#include <revenant/core/ring.hpp>

#include <cstdint>
#include <doctest/doctest.h>

using revenant::RingGeometry;

// RED: these tests drive RingGeometry design

TEST_CASE("RingGeometry rejects non-power-of-two capacity") {
    CHECK(!RingGeometry::make(0).ok());
    CHECK(!RingGeometry::make(100).ok());
    CHECK(!RingGeometry::make(1000).ok());
    CHECK(!RingGeometry::make(65535).ok());
}

TEST_CASE("RingGeometry accepts power-of-two capacities") {
    for (std::uint64_t cap : {1ULL, 2ULL, 4ULL, 64ULL, 1024ULL, 65536ULL, 1ULL << 20, 1ULL << 30}) {
        auto r = RingGeometry::make(cap);
        REQUIRE(r.ok());
        CHECK(r.value().capacity() == cap);
    }
}

TEST_CASE("index() maps position to ring offset via mask") {
    auto r = RingGeometry::make(1024).value();
    CHECK(r.index(revenant::Position{0}) == 0);
    CHECK(r.index(revenant::Position{1023}) == 1023);
    CHECK(r.index(revenant::Position{1024}) == 0); // wraps
    CHECK(r.index(revenant::Position{1025}) == 1);
    CHECK(r.index(revenant::Position{2047}) == 1023);
    CHECK(r.index(revenant::Position{2048}) == 0);
}

TEST_CASE("contiguous_space_to_end() is complementary to index()") {
    auto r = RingGeometry::make(1024).value();

    // At position 0: full ring contiguous
    CHECK(r.contiguous_space_to_end(revenant::Position{0}) == 1024);
    // At position 512: half ring left
    CHECK(r.contiguous_space_to_end(revenant::Position{512}) == 512);
    // At position 1023: one byte to end
    CHECK(r.contiguous_space_to_end(revenant::Position{1023}) == 1);
    // At position 1024 (wrapped back to 0): full ring again
    CHECK(r.contiguous_space_to_end(revenant::Position{1024}) == 1024);
}

TEST_CASE("straddles_ring_end() detects frames crossing the ring boundary") {
    auto r = RingGeometry::make(1024).value();

    // Frame at offset 1020 of length 8 straddles the end (1020 + 8 > 1024)
    CHECK(r.straddles_ring_end(revenant::Position{1020}, 8));
    // Frame at offset 1020 of length 4 exactly fits (1020 + 4 == 1024)
    CHECK(!r.straddles_ring_end(revenant::Position{1020}, 4));
    // Frame at offset 0 of any size <= capacity does not straddle
    CHECK(!r.straddles_ring_end(revenant::Position{0}, 1024));
}

TEST_CASE("straddles_block_boundary() detects frames crossing a block header") {
    // block_size = 64 KiB (log2=16), capacity = 1 MiB
    auto r = RingGeometry::make(1u << 20, 16).value();

    constexpr std::uint64_t block = 1u << 16; // 65536

    // Frame ending exactly at a block boundary — fine
    CHECK(!r.straddles_block_boundary(revenant::Position{block - 8}, 8));
    // Frame starting just before a block boundary and crossing it
    CHECK(r.straddles_block_boundary(revenant::Position{block - 4}, 8));
    // Frame well within one block
    CHECK(!r.straddles_block_boundary(revenant::Position{0}, 32));
}

TEST_CASE("RingGeometry::make() is noexcept") {
    CHECK(noexcept(RingGeometry::make(1024)));
}
