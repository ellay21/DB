#pragma once

#include <revenant/core/position.hpp>
#include <revenant/errors.hpp>

#include <bit>
#include <cstdint>

namespace revenant {

// Immutable ring geometry derived from a validated power-of-two capacity.
// All methods are constexpr and noexcept — zero overhead at the call site.
class RingGeometry {
  public:
    // Factory — returns an error if capacity is not a power of two or is zero.
    // block_size_log2: log2 of the block size for block-boundary straddle detection.
    //   0 = disabled (no block boundary checking). Defaults to 0.
    //   If non-zero, must satisfy (1 << block_size_log2) <= capacity.
    [[nodiscard]] static Result<RingGeometry> make(std::uint64_t capacity,
                                                   std::uint32_t block_size_log2 = 0) noexcept {
        if (capacity == 0 || !std::has_single_bit(capacity)) {
            return Status{StatusCode::InvalidArgument,
                          "Ring capacity must be a non-zero power of two"};
        }
        if (block_size_log2 != 0 && (1ULL << block_size_log2) > capacity) {
            return Status{StatusCode::InvalidArgument,
                          "block_size_log2 out of range for this capacity"};
        }
        return RingGeometry{capacity, block_size_log2};
    }

    [[nodiscard]] constexpr std::uint64_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] constexpr std::uint64_t mask() const noexcept { return mask_; }
    [[nodiscard]] constexpr std::uint32_t block_size_log2() const noexcept {
        return block_size_log2_;
    }
    [[nodiscard]] constexpr std::uint64_t block_size() const noexcept {
        return 1ULL << block_size_log2_;
    }
    [[nodiscard]] constexpr std::uint64_t block_count() const noexcept {
        return capacity_ >> block_size_log2_;
    }

    // Maps a monotonic position to a ring index via bitmask.
    // Invariant: index(p + capacity) == index(p)
    [[nodiscard]] constexpr std::uint64_t index(Position p) const noexcept {
        return p.value() & mask_;
    }

    // Contiguous bytes available from p's ring offset to the end of the ring.
    // Satisfies: contiguous_space_to_end(p) + index(p) == capacity
    [[nodiscard]] constexpr std::uint64_t contiguous_space_to_end(Position p) const noexcept {
        return capacity_ - index(p);
    }

    // True if a frame of `len` bytes starting at position `p` would cross
    // the ring end (i.e. it would need to be split at the wrap boundary).
    [[nodiscard]] constexpr bool straddles_ring_end(Position p, std::uint64_t len) const noexcept {
        return index(p) + len > capacity_;
    }

    // True if a frame of `len` bytes starting at position `p` would cross
    // a block-index boundary (producer must insert a padding frame instead).
    [[nodiscard]] constexpr bool straddles_block_boundary(Position p,
                                                          std::uint64_t len) const noexcept {
        const std::uint64_t offset = index(p);
        const std::uint64_t block_mask = block_size() - 1;
        const std::uint64_t offset_in_block = offset & block_mask;
        // Crosses iff the frame does not fit entirely within the current block.
        return (offset_in_block + len) > block_size();
    }

  private:
    constexpr RingGeometry() noexcept : capacity_(0), mask_(0), block_size_log2_(0) {}
    constexpr RingGeometry(std::uint64_t capacity, std::uint32_t block_size_log2) noexcept
        : capacity_(capacity), mask_(capacity - 1), block_size_log2_(block_size_log2) {}

    std::uint64_t capacity_;
    std::uint64_t mask_;
    std::uint32_t block_size_log2_;
};

} // namespace revenant
