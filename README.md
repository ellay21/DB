# Revenant

> **Status: pre-alpha** — not yet suitable for production use.

A single-machine, lock-free, zero-copy shared-memory transport for C++20 with **defined, tested and formally specified behaviour when a participant process dies mid-operation**.

---

## The problem

Every lock-free ring buffer published for C++ is written under an unstated assumption: threads do not vanish. Inside one process that is nearly true. Across processes it is false:

- A process is `SIGKILL`ed by an operator or supervisor
- The OOM killer selects it
- It segfaults in unrelated code while holding a claimed ring slot
- A container is evicted or hits a cgroup memory limit

When that happens to a producer that has *reserved* ring space but not *committed* it, every existing library is either permanently wedged or silently corrupt.

Revenant makes four guarantees:

| # | Guarantee |
|---|-----------|
| G1 | **No torn reads.** A consumer never observes a frame whose payload was not fully written before the producer died. |
| G2 | **Bounded unwedging.** Death of any participant is detected and its effect removed within a configured bound `T_detect`. |
| G3 | **No silent loss.** Data loss is always reported as a typed event with a byte/sequence range. |
| G4 | **Single-writer recovery.** After producer death, exactly one standby may take over; consumers observe an explicit discontinuity marker. |

---

## Honest comparison

| System | Crash semantics | Why Revenant differs |
|--------|-----------------|----------------------|
| **Aeron** | Genuinely good: term buffers, media driver | Requires a separate driver process. Revenant is driver-less and formally specified. Cite Aeron; do not trash it. |
| **iceoryx / iceoryx2** | RouDi daemon owns cleanup | Central daemon dependency. Revenant has no daemon. |
| **boost::interprocess** | None — a death while holding the mutex wedges permanently | This is the headline repro (see `examples/`). |
| **rigtorp/SPSCQueue** | N/A — in-process only | Different problem; the reference for the uncontended fast path. |
| **Unix domain sockets** | Perfect (kernel owns the buffer) | 5–20× the latency. If 3 µs is acceptable, use a socket. |

**If you need cross-machine transport, use Aeron. If you are in ROS, use iceoryx. If 5 µs is fine, use a Unix socket.** Revenant is for sub-microsecond single-machine IPC where you cannot tolerate a wedged channel.

---

## Non-goals (v1)

- Cross-machine transport
- Persistence / durability (the ring is volatile by design)
- Windows (platform layer is designed for it; shipping it is v1.2)
- RPC / serialisation / schema (Revenant moves opaque bytes)
- Dynamic ring resizing
- Priority / QoS ordering

---

## Building

```bash
git clone https://github.com/you/revenant && cd revenant
./scripts/bootstrap.sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Requires: CMake ≥ 3.25, GCC ≥ 11 or Clang ≥ 14, C++20.

---

## License

Apache-2.0. See [LICENSE](LICENSE).
