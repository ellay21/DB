#include "revenant/core/layout.hpp"
#include <doctest/doctest.h>
#include <cstddef>
#include <type_traits>

TEST_CASE("I14: ControlBlock hot fields are on distinct cache lines") {
    using rvn::core::ControlBlock;
    using rvn::core::kCacheLine;

    static_assert(sizeof(ControlBlock) == 3 * kCacheLine);
    static_assert(alignof(ControlBlock) == kCacheLine);

    static_assert(offsetof(ControlBlock, magic) == 0);
    static_assert(offsetof(ControlBlock, published) == kCacheLine);
    static_assert(offsetof(ControlBlock, reserved_pos) == 2 * kCacheLine);
}

TEST_CASE("FrameHeader first 8 bytes are byte-identical across all kinds") {
    using rvn::core::FrameHeaderMinimal;
    using rvn::core::FrameHeaderStandard;
    using rvn::core::FrameHeaderTraced;

    // Check sizes
    static_assert(sizeof(FrameHeaderMinimal) == 8);
    static_assert(sizeof(FrameHeaderStandard) == 32);
    static_assert(sizeof(FrameHeaderTraced) == 48);

    // Check alignments
    static_assert(alignof(FrameHeaderMinimal) >= 4);
    static_assert(alignof(FrameHeaderStandard) >= 8);
    static_assert(alignof(FrameHeaderTraced) >= 8);

    // Offsets of the first 4 fields must be identical and 8 bytes total
    auto check_common_header = [](auto dummy) {
        using T = decltype(dummy);
        static_assert(offsetof(T, length) == 0);
        static_assert(offsetof(T, type) == 4);
        static_assert(offsetof(T, flags) == 6);
        static_assert(offsetof(T, header_version) == 7);
    };

    check_common_header(FrameHeaderMinimal{});
    check_common_header(FrameHeaderStandard{});
    check_common_header(FrameHeaderTraced{});
}

TEST_CASE("ParticipantSlot layout") {
    using rvn::core::ParticipantSlot;
    using rvn::core::kCacheLine;

    static_assert(sizeof(ParticipantSlot) == 4 * kCacheLine);
    static_assert(alignof(ParticipantSlot) == 2 * kCacheLine);
    static_assert(offsetof(ParticipantSlot, position) == 0);
    static_assert(offsetof(ParticipantSlot, slot_word) == 2 * kCacheLine);
}
