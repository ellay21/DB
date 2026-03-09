#pragma once

#include <cstdint>
#include <limits>

namespace revenant {

// A 64-bit monotonic byte-count position with wrap-safe comparison.
//
// Positions never reset. At 10 GB/s sustained, 2^64 bytes ≈ 58 years of
// uptime, so practical wrap is not a concern. The type still defines
// arithmetically correct wrap-around behaviour so property tests can
// exercise the full uint64_t range without undefined behaviour.
//
// Wrap-safe comparison: static_cast<int64_t>(a.value() - b.value()) < 0
// interprets the unsigned subtraction as a signed distance — correct for
// any two positions that are within 2^63 of each other.
class Position {
  public:
    constexpr Position() noexcept = default;
    constexpr explicit Position(std::uint64_t v) noexcept : v_(v) {}

    [[nodiscard]] constexpr std::uint64_t value() const noexcept { return v_; }

    // Arithmetic — wraps naturally at 2^64 (well-defined unsigned arithmetic).
    [[nodiscard]] constexpr Position operator+(std::uint64_t offset) const noexcept {
        return Position{v_ + offset};
    }
    constexpr Position& operator+=(std::uint64_t offset) noexcept {
        v_ += offset;
        return *this;
    }

    // Wrap-safe comparisons via signed reinterpretation of the difference.
    [[nodiscard]] constexpr bool operator==(Position o) const noexcept { return v_ == o.v_; }
    [[nodiscard]] constexpr bool operator!=(Position o) const noexcept { return v_ != o.v_; }

    [[nodiscard]] constexpr bool operator<(Position o) const noexcept {
        return static_cast<std::int64_t>(v_ - o.v_) < 0;
    }
    [[nodiscard]] constexpr bool operator>(Position o) const noexcept { return o < *this; }
    [[nodiscard]] constexpr bool operator<=(Position o) const noexcept { return !(o < *this); }
    [[nodiscard]] constexpr bool operator>=(Position o) const noexcept { return !(*this < o); }

  private:
    std::uint64_t v_{0};
};

// Returns the signed distance from `from` to `to` (wrap-safe).
// Positive means `to` is ahead of `from`.
[[nodiscard]] constexpr std::uint64_t distance(Position from, Position to) noexcept {
    return to.value() - from.value();
}

} // namespace revenant
