#pragma once

#include <cstddef>
#include <cstdint>

namespace rvn::core {

// Portable CRC32C table-based fallback
std::uint32_t crc32c_portable(const void* data, std::size_t size, std::uint32_t crc = 0) noexcept;

// Compute CRC32C using hardware acceleration if available (SSE4.2/ARMv8),
// falling back to portable implementation otherwise.
std::uint32_t crc32c(const void* data, std::size_t size, std::uint32_t crc = 0) noexcept;

} // namespace rvn::core
