# Wire Format Specification

This document specifies the on-disk and in-memory layout of a Revenant ring buffer.

## Control Block (256 bytes)

Located at offset 0. Read-mostly metadata, hot-path fields, and reaper fields separated by cache lines.

| Offset | Size | Type | Name | Description |
|---|---|---|---|---|
| 0 | 4 | u32 | `magic` | `0x52564E54` |
| 4 | 2 | u16 | `version` | `1` |
| 6 | 2 | u16 | `header_kind` | `0`=Minimal, `1`=Standard, `2`=Traced |
| 8 | 4 | u32 | `capacity_log2` | 2^N bytes |
| 12 | 4 | u32 | `block_size_log2` | 2^N bytes |
| 16 | 48 | u8[48] | `pad0` | Padding |
| 64 | 8 | u64 | `published` | Hot consumer read |
| 72 | 4 | u32 | `epoch` | Failover epoch |
| 76 | 52 | u8[52] | `pad1` | Padding |
| 128 | 8 | u64 | `reserved_pos` | Hot producer read |
| 136 | 56 | u8[56] | `pad2` | Padding |
| 192 | 64 | u8[64] | `pad3` | Ensure 256B total size |

## Participant Table

Immediately follows the control block. Each slot is 256 bytes.

| Offset | Size | Type | Name | Description |
|---|---|---|---|---|
| 0 | 8 | u64 | `position` | Consumer read position |
| 8 | 8 | u64 | `heartbeat_ns` | Monotonic heartbeat |
| 16 | 112 | u8[112] | `pad0` | Padding |
| 128 | 8 | u64 | `slot_word` | Gen(40) | State(8) | Role(8) | Flags(8) |
| 136 | 32 | u8[32] | `identity` | Process identity |
| 168 | 8 | u64 | `lease_expiry_ns` | Lease timeout |
| 176 | 8 | u64 | `registered_ns` | Registration time |
| 184 | 24 | char[24] | `name` | Human label |
| 208 | 48 | u8[48] | `pad1` | Fill |

## Frame Headers

All frames begin with an identical 8-byte prefix to allow uniform CAS.

### Minimal Header (8 bytes)

| Offset | Size | Type | Name | Description |
|---|---|---|---|---|
| 0 | 4 | i32 | `length` | Payload length |
| 4 | 2 | u16 | `type` | App type |
| 6 | 1 | u8 | `flags` | Bitmask |
| 7 | 1 | u8 | `header_version` | Version |

### Standard Header (32 bytes)

| Offset | Size | Type | Name | Description |
|---|---|---|---|---|
| 0 | 4 | i32 | `length` | Payload length |
| 4 | 2 | u16 | `type` | App type |
| 6 | 1 | u8 | `flags` | Bitmask |
| 7 | 1 | u8 | `header_version` | Version |
| 8 | 8 | u64 | `sequence` | Monotonic counter |
| 16 | 4 | u32 | `session` | Epoch |
| 20 | 4 | u32 | `checksum` | CRC32C |
| 24 | 8 | u64 | `timestamp` | Producer TSC |

### Traced Header (48 bytes)

| Offset | Size | Type | Name | Description |
|---|---|---|---|---|
| 0 | 4 | i32 | `length` | Payload length |
| 4 | 2 | u16 | `type` | App type |
| 6 | 1 | u8 | `flags` | Bitmask |
| 7 | 1 | u8 | `header_version` | Version |
| 8 | 8 | u64 | `sequence` | Monotonic counter |
| 16 | 4 | u32 | `session` | Epoch |
| 20 | 4 | u32 | `checksum` | CRC32C |
| 24 | 8 | u64 | `timestamp` | Producer TSC |
| 32 | 8 | u64 | `producer_ts` | Extra timestamp |
| 40 | 8 | u64 | `trace_id` | Trace ID |
