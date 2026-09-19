#include <revenant/core/position.hpp>

#include <cstdint>
#include <doctest/doctest.h>
#include <limits>
#include <type_traits>

using revenant::Position;

// RED: these tests drive the Position type design

TEST_CASE("Position is a strong typedef — no implicit conversion from uint64_t") {
    // Must not compile: Position p = uint64_t{42};
    // Verified by static_assert below:
    static_assert(!std::is_convertible_v<std::uint64_t, Position>,
                  "Position must not be implicitly constructible from uint64_t");
    static_assert(!std::is_convertible_v<Position, std::uint64_t>,
                  "Position must not be implicitly convertible to uint64_t");
}

TEST_CASE("Position default-constructs to zero") {
    Position p{};
    CHECK(p.value() == 0ULL);
}

TEST_CASE("Position explicit construction and value round-trip") {
    Position p{42ULL};
    CHECK(p.value() == 42ULL);
}

TEST_CASE("Position equality and inequality") {
    Position a{10}, b{10}, c{20};
    CHECK(a == b);
    CHECK(a != c);
}

TEST_CASE("Position wrap-safe comparison near 2^64") {
    // Monotonic positions must compare correctly even when the raw uint64_t
    // difference would overflow. The canonical wrap-safe comparison is:
    //   static_cast<int64_t>(a.value() - b.value()) < 0  ↔  a < b
    constexpr auto max64 = std::numeric_limits<std::uint64_t>::max();

    Position big{max64};
    Position small{max64 - 10};

    CHECK(small < big);
    CHECK(big > small);
    CHECK(!(big < small));

    // Near-wrap: 0 is "ahead of" max64 by one step (position has advanced)
    // i.e. distance(max64, 0) == 1
    Position zero{0};
    Position one{1};
    CHECK(zero > big); // wrap-safe: zero is 1 step ahead of max64
    CHECK(one > zero);
}

TEST_CASE("Position distance() is wrap-safe") {
    using revenant::distance;
    constexpr auto max64 = std::numeric_limits<std::uint64_t>::max();

    CHECK(distance(Position{0}, Position{10}) == 10);
    CHECK(distance(Position{100}, Position{150}) == 50);

    // Wrap-around case: later - earlier = correct positive delta
    CHECK(distance(Position{max64}, Position{0}) == 1);
    CHECK(distance(Position{max64 - 4}, Position{1}) == 6);
}

TEST_CASE("Position arithmetic — advance by offset") {
    Position p{100};
    Position q = p + 50;
    CHECK(q.value() == 150);

    // Wrap through zero
    constexpr auto max64 = std::numeric_limits<std::uint64_t>::max();
    Position near_max{max64};
    Position wrapped = near_max + 1;
    CHECK(wrapped.value() == 0);
}

TEST_CASE("Position is constexpr and noexcept") {
    static_assert(noexcept(Position{}));
    static_assert(noexcept(Position{42ULL}));

    constexpr Position a{10}, b{20};
    static_assert(a < b);
    static_assert(b > a);
    static_assert(a != b);
}
