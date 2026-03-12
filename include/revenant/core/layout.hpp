#pragma once

#include <atomic>
#include <cstdint>

namespace rvn::core {

constexpr std::size_t kCacheLine = 64;

struct alignas(kCacheLine) ControlBlock {
    // line 0 — read-mostly metadata (cached by everyone)
    std::uint32_t magic;         // 0x52564E54 ('RVNT')
    std::uint16_t version;       // 1
    std::uint16_t header_kind;   // 0=Minimal, 1=Standard, 2=Traced
    std::uint32_t capacity_log2; // e.g. 24 for 16MB
    std::uint32_t block_size_log2; // e.g. 16 for 64KB
    std::uint8_t pad0[48];

    // line 1 — hot path: producer write, consumer read
    std::atomic<std::uint64_t> published;
    std::atomic<std::uint32_t> epoch;
    std::uint8_t pad1[52];

    // line 2 — hot path: producer write, reaper read
    std::atomic<std::uint64_t> reserved_pos;
    std::uint8_t pad2[56];
};
static_assert(sizeof(ControlBlock) == 3 * kCacheLine);

struct ProcessIdentity {            // 32 bytes, compared as a whole
    std::uint64_t boot_id_hash;       // from /proc/sys/kernel/random/boot_id
    std::uint64_t pid;
    std::uint64_t start_time_ticks;   // /proc/<pid>/stat field 22 — defeats PID reuse
    std::uint64_t reserved;
};

enum class Role : std::uint8_t { None, Producer, Consumer, Observer };
enum class SlotState : std::uint8_t { Free, Claiming, Live, Suspect, Dead, Reaped };

struct alignas(2 * kCacheLine) ParticipantSlot {
    // line 0 — written by the owner only
    std::atomic<std::uint64_t> position;      // consumer read position
    std::atomic<std::uint64_t> heartbeat_ns;  // CLOCK_MONOTONIC_COARSE
    std::uint8_t pad0[2 * kCacheLine - 16];

    // line 1 — written on registration / eviction, read often
    std::atomic<std::uint64_t> slot_word;     // packed: generation:40 | state:8 | role:8 | flags:8
    ProcessIdentity identity;
    std::atomic<std::uint64_t> lease_expiry_ns;
    std::uint64_t registered_ns;
    char name[24];                   // human label for revenantctl
    std::uint8_t pad1[4 * kCacheLine - 208];
};
static_assert(sizeof(ParticipantSlot) == 4 * kCacheLine);

struct alignas(4) FrameHeaderMinimal {
    std::int32_t length;
    std::uint16_t type;
    std::uint8_t flags;
    std::uint8_t header_version;
};

struct alignas(8) FrameHeaderStandard {
    std::int32_t length;
    std::uint16_t type;
    std::uint8_t flags;
    std::uint8_t header_version;
    
    std::uint64_t sequence;
    std::uint32_t session;
    std::uint32_t checksum;
    std::uint64_t timestamp;
};

struct alignas(8) FrameHeaderTraced {
    std::int32_t length;
    std::uint16_t type;
    std::uint8_t flags;
    std::uint8_t header_version;
    
    std::uint64_t sequence;
    std::uint32_t session;
    std::uint32_t checksum;
    std::uint64_t timestamp;
    
    std::uint64_t producer_ts;
    std::uint64_t trace_id;
};

} // namespace rvn::core
