#include "revenant/core/frame.hpp"
#include "revenant/core/layout.hpp"
#include <doctest/doctest.h>
#include <vector>
#include <cstring>

using namespace rvn::core;
using revenant::StatusCode;

TEST_CASE("FrameCodec: encode/decode round-trip Standard") {
    std::vector<std::byte> buffer(1024);
    auto* hdr = new (buffer.data()) FrameHeaderStandard{};
    hdr->length = 42;
    hdr->type = 1337;
    hdr->flags = kFlagFragBegin | kFlagFragEnd;
    hdr->header_version = 1;
    hdr->sequence = 100;
    hdr->session = 5;
    hdr->checksum = 0xABCD;
    hdr->timestamp = 9999;

    auto res = decode_frame(buffer, FrameKind::Standard);
    REQUIRE(res.ok());
    auto view = res.value();
    
    CHECK(view.kind == FrameKind::Standard);
    CHECK(view.payload.size() == 42);
    CHECK(view.type == 1337);
    CHECK(view.flags == (kFlagFragBegin | kFlagFragEnd));
    CHECK(view.header_version == 1);
    CHECK(view.sequence == 100);
    CHECK(view.session == 5);
    CHECK(view.checksum == 0xABCD);
    CHECK(view.timestamp == 9999);
}

TEST_CASE("FrameCodec: encode/decode round-trip Minimal") {
    std::vector<std::byte> buffer(1024);
    auto* hdr = new (buffer.data()) FrameHeaderMinimal{};
    hdr->length = 42;
    hdr->type = 1337;
    hdr->flags = kFlagFragBegin;
    hdr->header_version = 1;

    auto res = decode_frame(buffer, FrameKind::Minimal);
    REQUIRE(res.ok());
    auto view = res.value();
    
    CHECK(view.kind == FrameKind::Minimal);
    CHECK(view.payload.size() == 42);
    CHECK(view.type == 1337);
    CHECK(view.flags == kFlagFragBegin);
    CHECK(view.header_version == 1);
}

TEST_CASE("FrameCodec: encode/decode round-trip Traced") {
    std::vector<std::byte> buffer(1024);
    auto* hdr = new (buffer.data()) FrameHeaderTraced{};
    hdr->length = 42;
    hdr->type = 1337;
    hdr->flags = kFlagFragBegin;
    hdr->header_version = 1;
    hdr->producer_ts = 8888;
    hdr->trace_id = 999999;

    auto res = decode_frame(buffer, FrameKind::Traced);
    REQUIRE(res.ok());
    auto view = res.value();
    
    CHECK(view.kind == FrameKind::Traced);
    CHECK(view.payload.size() == 42);
    CHECK(view.type == 1337);
    CHECK(view.flags == kFlagFragBegin);
    CHECK(view.header_version == 1);
    CHECK(view.producer_ts == 8888);
    CHECK(view.trace_id == 999999);
}

TEST_CASE("FrameCodec: claimed header (< 0) never decodes as readable") {
    std::vector<std::byte> buffer(1024);
    auto* hdr = new (buffer.data()) FrameHeaderMinimal{};
    hdr->length = -100;
    hdr->type = 1;

    auto res = decode_frame(buffer, FrameKind::Minimal);
    CHECK(!res.ok());
    CHECK(res.status().code() == StatusCode::WouldBlock);
}

TEST_CASE("FrameCodec: free frame (== 0) is rejected") {
    std::vector<std::byte> buffer(1024);
    auto* hdr = new (buffer.data()) FrameHeaderMinimal{};
    hdr->length = 0;
    
    auto res = decode_frame(buffer, FrameKind::Minimal);
    CHECK(!res.ok());
    CHECK(res.status().code() == StatusCode::WouldBlock);
}

TEST_CASE("FrameCodec: length == INT32_MIN is rejected, not negated") {
    std::vector<std::byte> buffer(1024);
    auto* hdr = new (buffer.data()) FrameHeaderMinimal{};
    hdr->length = -2147483648; // INT32_MIN
    
    auto res = decode_frame(buffer, FrameKind::Minimal);
    CHECK(!res.ok());
    CHECK(res.status().code() == StatusCode::Corrupt);
}

TEST_CASE("FrameCodec: padding frames decode and report their span") {
    std::vector<std::byte> buffer(1024);
    auto* hdr = new (buffer.data()) FrameHeaderStandard{};
    hdr->length = 100; // Total length of padding payload
    hdr->flags = kFlagPadding;
    
    auto res = decode_frame(buffer, FrameKind::Standard);
    REQUIRE(res.ok());
    CHECK((res.value().flags & kFlagPadding) != 0);
    CHECK(res.value().payload.size() == 100);
}

TEST_CASE("FrameCodec: buffer too small for header") {
    std::vector<std::byte> buffer(10);
    // Minimal is 8 bytes, fits. Standard is 32, does not fit.
    auto res = decode_frame(buffer, FrameKind::Standard);
    CHECK(!res.ok());
    CHECK(res.status().code() == StatusCode::Corrupt);
}

TEST_CASE("FrameCodec: buffer too small for payload") {
    std::vector<std::byte> buffer(40); // Standard header is 32, leaves 8 bytes for payload
    auto* hdr = new (buffer.data()) FrameHeaderStandard{};
    hdr->length = 20; // Needs 20, but only 8 available
    
    auto res = decode_frame(buffer, FrameKind::Standard);
    CHECK(!res.ok());
    CHECK(res.status().code() == StatusCode::Corrupt);
}
