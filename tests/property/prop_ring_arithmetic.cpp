#include <doctest/doctest.h>

// RapidCheck headers have sign-conversion and useless-cast warnings;
// suppress them here since this is a test file including a third-party library.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <rapidcheck.h>
#pragma GCC diagnostic pop

#include <revenant/core/position.hpp>
#include <revenant/core/ring.hpp>

#include <cstdint>

using revenant::Position;
using revenant::RingGeometry;

static RingGeometry make_ring(int shift) {
    return RingGeometry::make(1ULL << shift).value();
}

template <typename Fn> static void prop(const char* desc, Fn&& fn) {
    rc::check(desc, std::forward<Fn>(fn));
}

TEST_CASE("prop: index(p + capacity) == index(p)") {
    prop("index wrap identity", [] {
        auto ring = make_ring(*rc::gen::inRange(1, 31));
        Position p{*rc::gen::arbitrary<std::uint64_t>()};
        RC_ASSERT(ring.index(p) == ring.index(p + ring.capacity()));
    });
}

TEST_CASE("prop: contiguous_space + index == capacity") {
    prop("contiguous space identity", [] {
        auto ring = make_ring(*rc::gen::inRange(1, 31));
        Position p{*rc::gen::arbitrary<std::uint64_t>()};
        RC_ASSERT(ring.contiguous_space_to_end(p) + ring.index(p) == ring.capacity());
    });
}

TEST_CASE("prop: alignment round-up is idempotent") {
    prop("alignment idempotency", [] {
        auto v = *rc::gen::inRange<std::uint64_t>(0, 1ULL << 20);
        auto log2 = *rc::gen::inRange<std::uint32_t>(0, 7);
        std::uint64_t align = 1ULL << log2;
        std::uint64_t rounded = (v + align - 1) & ~(align - 1);
        RC_ASSERT(rounded >= v);
        RC_ASSERT(rounded % align == 0);
        RC_ASSERT(((rounded + align - 1) & ~(align - 1)) == rounded);
    });
}

TEST_CASE("prop: distance is transitive mod 2^64") {
    prop("distance transitivity", [] {
        Position a{*rc::gen::arbitrary<std::uint64_t>()};
        Position b{*rc::gen::arbitrary<std::uint64_t>()};
        Position c{*rc::gen::arbitrary<std::uint64_t>()};
        RC_ASSERT(revenant::distance(a, b) + revenant::distance(b, c) == revenant::distance(a, c));
    });
}

TEST_CASE("prop: non-power-of-two capacity is rejected") {
    prop("non-pow2 rejected", [] {
        auto v = *rc::gen::inRange<std::uint64_t>(3, std::uint64_t{1} << 32);
        if ((v & (v - 1)) == 0)
            v |= (v >> 1);
        RC_ASSERT(!RingGeometry::make(v).ok());
    });
}

TEST_CASE("prop: power-of-two capacity is accepted") {
    prop("pow2 accepted",
         [] { RC_ASSERT(RingGeometry::make(1ULL << *rc::gen::inRange(0, 31)).ok()); });
}
