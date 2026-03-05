#pragma once

// Compile-time feature gates. Defaults can be overridden via -D on the command line.

#ifndef REVENANT_CACHE_LINE_SIZE
#if defined(__aarch64__) || defined(__arm64__)
#define REVENANT_CACHE_LINE_SIZE 128
#else
#define REVENANT_CACHE_LINE_SIZE 64
#endif
#endif

// Disable kernel features for testing fallback paths.
// These are checked at runtime, not compile time.
// Set via environment variable, e.g. REVENANT_DISABLE_PIDFD=1.

#if defined(__linux__)
#define REVENANT_PLATFORM_LINUX 1
#elif defined(__APPLE__)
#define REVENANT_PLATFORM_DARWIN 1
#else
#define REVENANT_PLATFORM_POSIX 1
#endif

#define REVENANT_HOTPATH __attribute__((noinline)) // placeholder; real attr TBD in P3

#define RVN_LIKELY(x) __builtin_expect(!!(x), 1)
#define RVN_UNLIKELY(x) __builtin_expect(!!(x), 0)
