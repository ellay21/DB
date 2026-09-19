# ADR 0001: Byte Ring vs. Fixed Slots

## Context

A shared-memory lock-free queue must organise its payload space. Broadly, there are two approaches:
1. **Fixed slots (term buffers)**: The memory is divided into fixed-size chunks (e.g. Aeron). Claims always take exactly one slot.
2. **Byte ring**: The memory is a contiguous byte array wrapped at a power-of-two bound (e.g. LMAX Disruptor, standard `kfifo`). Frames are variable length and placed sequentially.

We need to decide which layout Revenant will use.

## Decision

We will use a **Byte Ring** with a **Block Index**.

## Rationale

1. **Space efficiency for diverse payloads**: If the channel handles 10-byte control messages and 1 MB data payloads, fixed slots force either massive internal fragmentation (slots are 1 MB) or require application-level fragmentation logic (splitting data across slots).
2. **Zero-copy usability**: A contiguous byte ring allows the producer to claim `N` bytes and write them as a single `std::span` without worrying about chunk boundaries, provided the claim does not straddle the physical ring wrap (in which case a padding frame is emitted).

### The Overrun Problem

The primary argument *against* a byte ring is consumer overrun recovery. In fixed slots, if a consumer is lapped (its position is now > `capacity` bytes behind the head), it simply jumps to `published - capacity` and aligns to a slot boundary trivially.

In a byte ring, `published - capacity` lands at an arbitrary byte offset, likely in the middle of a frame. The consumer cannot parse forward because there is no magic byte or synchronization marker to reliably find the start of the next frame.

### The Solution: Block Index

To solve the overrun problem while retaining the benefits of a byte ring, we divide the ring into logical blocks (default 64 KiB). We introduce an out-of-band **Block Index** array.
- For block `i`, `block_index[i]` records the exact byte offset of the *first* frame that starts in that block, plus a generation tag to prevent ABA.
- When an overrun occurs, a consumer jumps to the oldest safely-readable block, reads its index entry, validates the generation, and starts parsing from that exact frame boundary.
- If a frame naturally straddles the boundary such that no frame starts exactly *near* the boundary, the producer emits a padding frame to ensure every block has at least one valid starting point.

## Consequences

**Positive:**
- Zero-copy variable-length messages.
- No wasted memory from slot padding.

**Negative:**
- Producer logic is more complex (must maintain the block index, emit padding frames on wraps and block boundaries).
- Consumers have a slightly more complex resync path.
