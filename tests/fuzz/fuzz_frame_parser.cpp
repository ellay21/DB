#include "revenant/core/frame.hpp"
#include <cstdint>
#include <cstddef>
#include <span>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) return 0;
    
    // The first byte determines which FrameKind to use.
    uint8_t kind_byte = data[0] % 3;
    rvn::core::FrameKind kind = static_cast<rvn::core::FrameKind>(kind_byte);
    
    std::span<const std::byte> memory(reinterpret_cast<const std::byte*>(data + 1), size - 1);
    
    // We just want to ensure it doesn't crash, infinite loop, or UB.
    auto res = rvn::core::decode_frame(memory, kind);
    (void)res; // Ignore output
    
    return 0;
}
