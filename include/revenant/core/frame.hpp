#pragma once

#include <cstdint>
#include <cstring>
#include <span>

#include "revenant/core/layout.hpp"
#include "revenant/errors.hpp"

namespace rvn::core {

enum class FrameKind : std::uint8_t { Minimal = 0, Standard = 1, Traced = 2 };

struct FrameView {
    FrameKind kind;
    std::span<const std::byte> payload;

    // Extracted fields
    std::uint16_t type;
    std::uint8_t flags;
    std::uint8_t header_version;

    // Only valid if kind >= Standard
    std::uint64_t sequence = 0;
    std::uint32_t session = 0;
    std::uint32_t checksum = 0;
    std::uint64_t timestamp = 0;

    // Only valid if kind == Traced
    std::uint64_t producer_ts = 0;
    std::uint64_t trace_id = 0;
};

// Flags bitmask
constexpr std::uint8_t kFlagFragBegin = 1 << 0;
constexpr std::uint8_t kFlagFragEnd = 1 << 1;
constexpr std::uint8_t kFlagPadding = 1 << 2;
constexpr std::uint8_t kFlagReaped = 1 << 3;
constexpr std::uint8_t kFlagChecksummed = 1 << 4;

inline revenant::Result<FrameView> decode_frame(std::span<const std::byte> memory,
                                                FrameKind expected_kind) noexcept {
    std::size_t header_size = 0;
    switch (expected_kind) {
    case FrameKind::Minimal:
        header_size = sizeof(FrameHeaderMinimal);
        break;
    case FrameKind::Standard:
        header_size = sizeof(FrameHeaderStandard);
        break;
    case FrameKind::Traced:
        header_size = sizeof(FrameHeaderTraced);
        break;
    default:
        return revenant::Status(revenant::StatusCode::InvalidArgument, "Invalid FrameKind");
    }

    if (memory.size() < header_size) {
        return revenant::Status(revenant::StatusCode::Corrupt, "Buffer too small for header");
    }

    // Read the common 8 bytes to get the length.
    // They are identical across all kinds.
    FrameHeaderMinimal common;
    std::memcpy(&common, memory.data(), sizeof(common));

    if (common.length == -2147483648) { // INT32_MIN
        return revenant::Status(revenant::StatusCode::Corrupt, "INT32_MIN length rejected");
    }

    if (common.length < 0 || common.length == 0) {
        return revenant::Status(revenant::StatusCode::WouldBlock,
                                "Frame claimed but not readable, or free");
    }

    std::size_t payload_len = static_cast<std::size_t>(common.length);

    if (memory.size() - header_size < payload_len) {
        return revenant::Status(revenant::StatusCode::Corrupt, "Buffer too small for payload");
    }

    FrameView view{};
    view.kind = expected_kind;
    view.payload = std::span<const std::byte>(memory.data() + header_size, payload_len);
    view.type = common.type;
    view.flags = common.flags;
    view.header_version = common.header_version;

    if (expected_kind == FrameKind::Standard || expected_kind == FrameKind::Traced) {
        FrameHeaderStandard std_hdr;
        std::memcpy(&std_hdr, memory.data(), sizeof(std_hdr));
        view.sequence = std_hdr.sequence;
        view.session = std_hdr.session;
        view.checksum = std_hdr.checksum;
        view.timestamp = std_hdr.timestamp;
    }

    if (expected_kind == FrameKind::Traced) {
        FrameHeaderTraced trc_hdr;
        std::memcpy(&trc_hdr, memory.data(), sizeof(trc_hdr));
        view.producer_ts = trc_hdr.producer_ts;
        view.trace_id = trc_hdr.trace_id;
    }

    return view;
}

} // namespace rvn::core
