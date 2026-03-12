#include "revenant/core/checksum.hpp"
#include <doctest/doctest.h>
#include <string_view>

TEST_CASE("CRC32C RFC 3720 vectors") {
    using rvn::core::crc32c;
    using rvn::core::crc32c_portable;

    // 32 bytes of zeros
    std::uint8_t zeros[32] = {0};
    CHECK(crc32c_portable(zeros, 32) == 0x8a9136aa);
    CHECK(crc32c(zeros, 32) == 0x8a9136aa);

    // 32 bytes of ones
    std::uint8_t ones[32];
    for(int i=0; i<32; ++i) ones[i] = 0xff;
    CHECK(crc32c_portable(ones, 32) == 0x62a8ab43);
    CHECK(crc32c(ones, 32) == 0x62a8ab43);

    // 32 bytes of 0..31
    std::uint8_t inc[32];
    for(int i=0; i<32; ++i) inc[i] = static_cast<std::uint8_t>(i);
    CHECK(crc32c_portable(inc, 32) == 0x46dd794e);
    CHECK(crc32c(inc, 32) == 0x46dd794e);

    // 32 bytes of 31..0
    std::uint8_t dec[32];
    for(int i=0; i<32; ++i) dec[i] = static_cast<std::uint8_t>(31 - i);
    CHECK(crc32c_portable(dec, 32) == 0x113fdb5c);
    CHECK(crc32c(dec, 32) == 0x113fdb5c);
}
