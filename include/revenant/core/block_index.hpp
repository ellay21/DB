#pragma once

#include <atomic>
#include <cstdint>
#include <span>
#include <optional>

namespace rvn::core {

class BlockIndex {
public:
    static constexpr std::uint32_t kUnset = 0xFFFFFFFF;

    BlockIndex(std::span<std::atomic<std::uint32_t>> entries, std::uint32_t block_size_log2)
        : entries_(entries), block_size_log2_(block_size_log2) {}

    // Set the block index entry for the given position.
    // It encodes the offset within the block and the generation tag.
    void set(std::uint64_t position) noexcept {
        std::uint64_t block_number = position >> block_size_log2_;
        std::uint32_t block_idx = static_cast<std::uint32_t>(block_number % entries_.size());
        std::uint32_t generation = static_cast<std::uint32_t>(block_number / entries_.size());
        std::uint32_t offset = static_cast<std::uint32_t>(position & ((1ULL << block_size_log2_) - 1));

        std::uint32_t gen_shift = block_size_log2_;
        std::uint32_t gen_mask = (1U << (32 - gen_shift)) - 1;

        std::uint32_t packed = ((generation & gen_mask) << gen_shift) | offset;
        entries_[block_idx].store(packed, std::memory_order_release);
    }

    // Get the offset for a specific block and expected generation.
    // Returns nullopt if the entry is unset or the generation does not match.
    std::optional<std::uint32_t> get(std::uint32_t block_idx, std::uint32_t expected_generation) const noexcept {
        if (block_idx >= entries_.size()) return std::nullopt;

        std::uint32_t packed = entries_[block_idx].load(std::memory_order_acquire);
        if (packed == kUnset) return std::nullopt;

        std::uint32_t gen_shift = block_size_log2_;
        std::uint32_t gen_mask = (1U << (32 - gen_shift)) - 1;
        
        std::uint32_t generation = (packed >> gen_shift) & gen_mask;
        std::uint32_t offset = packed & ((1U << gen_shift) - 1);

        if (generation != (expected_generation & gen_mask)) {
            return std::nullopt;
        }

        return offset;
    }

    std::span<std::atomic<std::uint32_t>> entries() const noexcept { return entries_; }
    std::uint32_t block_size_log2() const noexcept { return block_size_log2_; }

private:
    std::span<std::atomic<std::uint32_t>> entries_;
    std::uint32_t block_size_log2_;
};

} // namespace rvn::core
