#include "revenant/core/layout.hpp"
#include <cstdint>
#include <cstddef>
#include <span>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(rvn::core::ControlBlock)) return 0;
    
    const auto* cb = reinterpret_cast<const rvn::core::ControlBlock*>(data);
    
    // Validate control block (must not crash, handles hostile data)
    bool is_valid = rvn::core::is_control_block_valid(cb);
    (void)is_valid;
    
    return 0;
}
