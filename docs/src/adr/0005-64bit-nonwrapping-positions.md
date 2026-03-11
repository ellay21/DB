# ADR 0005 — 64-bit non-wrapping positions

**Status:** Accepted  
**Date:** 2026-03-11  
**Invariants:** I1, I2, I5

---

## Context

The ring buffer needs a position type to track producer and consumer progress. The key design questions are:

1. Width — 32-bit or 64-bit?
2. Wrapping or non-wrapping?
3. Raw integer or strong typedef?

## Decision

**64-bit, non-wrapping, strong typedef (`revenant::Position`).**

### Width: 64-bit

At a sustained 10 GB/s, 2^64 bytes of throughput requires approximately 58 years of continuous uptime. 32-bit positions wrap at 4 GiB — roughly 0.4 seconds at the same rate. A library whose positions wrap during a hot benchmark is not a serious contender.

The cost is one extra word in the consumer slot (`position` field in `ParticipantSlot`) and slightly wider arithmetic. This is trivially worthwhile.

### Non-wrapping (monotonic)

Positions are monotonically increasing byte counts that never intentionally reset. The ring index is derived by masking: `pos & (capacity - 1)`, which requires capacity to be a power of two. This is the standard technique (Aeron, rigtorp/SPSCQueue, LMAX Disruptor); there is no reason to deviate.

The type still provides mathematically correct wrap-around at 2^64 so that property tests can exercise the full range without invoking undefined behaviour. This is unsigned integer arithmetic — always well-defined in C++.

### Wrap-safe comparison

The comparison `a < b` on raw `uint64_t` is **not** wrap-safe. If `a = 2^64 - 1` and `b = 0` (b is one step ahead of a), `a < b` returns `false` — the wrong answer.

The correct idiom is:

```cpp
static_cast<int64_t>(a.value() - b.value()) < 0
```

This interprets the unsigned subtraction as a signed 64-bit distance. It is correct for any two positions within 2^63 of each other (i.e., for the entire practical operating range of this library).

This idiom is codified in the `operator<` of `revenant::Position` and is the **only** location where positions are compared. Using the raw `uint64_t` comparison anywhere else is a latent bug that takes years to manifest; the strong typedef makes it a compile error.

### Strong typedef

`revenant::Position` has no implicit conversions to or from `uint64_t`. This is not aesthetic — it is functional:

- You cannot accidentally pass a byte count where a position is expected.
- You cannot accidentally compare a position against a length or capacity.
- The compiler enforces the invariant at every call site with zero runtime cost.

## Alternatives considered

| Alternative | Reason rejected |
|-------------|----------------|
| 32-bit position | Wraps at 4 GiB — unacceptable at modern throughput rates |
| Wrapping 64-bit | The subtraction-comparison trick removes all practical need for wrapping semantics; non-wrapping is simpler to reason about |
| `using Position = uint64_t` (weak alias) | No protection against implicit conversion bugs; a strong typedef costs nothing |
| `std::strong_ordering` / `<=>` spaceship | C++20 spaceship is correct here and was considered; we define individual comparison operators explicitly so the ordering semantics are documented inline |

## Consequences

- All protocol positions are `revenant::Position`. No raw `uint64_t` appears as a position at any call site.
- Ring index computation is `p.value() & mask_` inside `RingGeometry`, encapsulated in one place.
- Property tests verify wrap-safe comparison near 2^64 with a committed seed set.
- Distance between two positions is `to.value() - from.value()` (unsigned, wraps correctly at 2^64).
