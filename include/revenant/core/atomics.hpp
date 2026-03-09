#pragma once

// Ordering-documented wrappers over std::atomic_ref.
//
// No raw std::memory_order appears outside this file anywhere in the codebase.
// Each wrapper name encodes the ordering, making the memory model readable at
// call sites and reviewable without looking up the enum value.
//
// Ordering justification for each ordering used in the protocol lives in
// docs/src/memory-model.md — one paragraph per label.

#include <atomic>
#include <cstdint>

namespace revenant::atomics {

// --- Loads -------------------------------------------------------------------

template <typename T> [[nodiscard]] T load_relaxed(const std::atomic<T>& a) noexcept {
    return a.load(std::memory_order_relaxed);
}

template <typename T> [[nodiscard]] T load_acquire(const std::atomic<T>& a) noexcept {
    return a.load(std::memory_order_acquire);
}

template <typename T> [[nodiscard]] T load_seq_cst(const std::atomic<T>& a) noexcept {
    return a.load(std::memory_order_seq_cst);
}

// --- Stores ------------------------------------------------------------------

template <typename T> void store_relaxed(std::atomic<T>& a, T val) noexcept {
    a.store(val, std::memory_order_relaxed);
}

template <typename T> void store_release(std::atomic<T>& a, T val) noexcept {
    a.store(val, std::memory_order_release);
}

// --- Fetch-add ---------------------------------------------------------------

template <typename T> T fetch_add_relaxed(std::atomic<T>& a, T delta) noexcept {
    return a.fetch_add(delta, std::memory_order_relaxed);
}

template <typename T> T fetch_add_acq_rel(std::atomic<T>& a, T delta) noexcept {
    return a.fetch_add(delta, std::memory_order_acq_rel);
}

// --- Compare-exchange --------------------------------------------------------

// Returns true if the exchange succeeded.
template <typename T> bool cas_weak_acq_rel(std::atomic<T>& a, T& expected, T desired) noexcept {
    return a.compare_exchange_weak(expected, desired, std::memory_order_acq_rel,
                                   std::memory_order_acquire);
}

template <typename T> bool cas_strong_acq_rel(std::atomic<T>& a, T& expected, T desired) noexcept {
    return a.compare_exchange_strong(expected, desired, std::memory_order_acq_rel,
                                     std::memory_order_acquire);
}

// --- atomic_ref variants (for atomics over mapped memory) --------------------

template <typename T> [[nodiscard]] T ref_load_acquire(const T& obj) noexcept {
    return std::atomic_ref<const T>{obj}.load(std::memory_order_acquire);
}

template <typename T> void ref_store_release(T& obj, T val) noexcept {
    std::atomic_ref<T>{obj}.store(val, std::memory_order_release);
}

template <typename T> bool ref_cas_strong_acq_rel(T& obj, T& expected, T desired) noexcept {
    return std::atomic_ref<T>{obj}.compare_exchange_strong(
        expected, desired, std::memory_order_acq_rel, std::memory_order_acquire);
}

} // namespace revenant::atomics
