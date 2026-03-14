#include "revenant/core/checksum.hpp"

namespace rvn::core {

static std::uint32_t crc32c_table[256];
static bool crc32c_table_init = false;

static void init_crc32c_table() {
    if (crc32c_table_init)
        return;
    for (std::uint32_t i = 0; i < 256; i++) {
        std::uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            c = (c >> 1) ^ ((c & 1) ? 0x82F63B78 : 0);
        }
        crc32c_table[i] = c;
    }
    crc32c_table_init = true;
}

std::uint32_t crc32c_portable(const void* data, std::size_t size, std::uint32_t crc) noexcept {
    init_crc32c_table();
    const std::uint8_t* p = static_cast<const std::uint8_t*>(data);
    crc = ~crc;
    for (std::size_t i = 0; i < size; ++i) {
        crc = crc32c_table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    }
    return ~crc;
}

// Check for HW accelerated crc32c at runtime (simplified for this implementation)
std::uint32_t crc32c(const void* data, std::size_t size, std::uint32_t crc) noexcept {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    // HW dispatch omitted for brevity; using portable fallback.
    // In production, we'd use __builtin_ia32_crc32qi, etc.
    return crc32c_portable(data, size, crc);
#else
    return crc32c_portable(data, size, crc);
#endif
}

} // namespace rvn::core
