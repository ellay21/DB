#pragma once

#include <cstddef>
#include <cstdint>
#include <new>

namespace revenant {

// kCacheLine is read once and stored in a constexpr so
// std::hardware_destructive_interference_size is never exposed at call sites
// (avoiding -Winterference-size on GCC and ABI fragility).
#if defined(__aarch64__) || defined(__arm64__)
inline constexpr std::size_t kCacheLine = 128;
#elif defined(__x86_64__) || defined(__i386__)
inline constexpr std::size_t kCacheLine = 64;
#else
#pragma message("Unknown architecture — defaulting cache line size to 64 bytes")
inline constexpr std::size_t kCacheLine = 64;
#endif

#define RVN_ALIGN_CACHE alignas(::revenant::kCacheLine)

// Wraps T in a struct padded to a full cache line to prevent false sharing.
template <typename T> struct alignas(kCacheLine) Padded {
    T value{};
    static_assert(sizeof(T) <= kCacheLine,
                  "Padded<T>: T is larger than a cache line — use a different layout");

  private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
    std::byte pad_[kCacheLine - sizeof(T)]{};
};

static_assert(sizeof(Padded<std::uint64_t>) == kCacheLine);
static_assert(alignof(Padded<std::uint64_t>) == kCacheLine);

} // namespace revenant
