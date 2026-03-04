#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

log()  { echo "[bootstrap] $*"; }
warn() { echo "[bootstrap] WARN: $*" >&2; }
die()  { echo "[bootstrap] ERROR: $*" >&2; exit 1; }

# ---- Tool version checks ----------------------------------------------------
check_version() {
    local cmd="$1" want_major="$2"
    if ! command -v "${cmd}" &>/dev/null; then
        die "'${cmd}' not found. Please install it."
    fi
}

check_version cmake 3
check_version git   2
check_version python3 3

CMAKE_VERSION="$(cmake --version | head -1 | awk '{print $3}')"
CMAKE_MAJOR="$(echo "${CMAKE_VERSION}" | cut -d. -f1)"
CMAKE_MINOR="$(echo "${CMAKE_VERSION}" | cut -d. -f2)"
if (( CMAKE_MAJOR < 3 || (CMAKE_MAJOR == 3 && CMAKE_MINOR < 25) )); then
    die "CMake >= 3.25 required, found ${CMAKE_VERSION}"
fi
log "cmake ${CMAKE_VERSION} OK"

# ---- Git hooks --------------------------------------------------------------
log "Installing git hooks..."
git -C "${ROOT}" config core.hooksPath .githooks
chmod +x "${ROOT}/.githooks/"*
log "Git hooks installed."

# ---- Python tools -----------------------------------------------------------
log "Checking python tools..."
if ! command -v pre-commit &>/dev/null; then
    warn "pre-commit not found. Run: pip install pre-commit"
else
    pre-commit install --install-hooks --quiet
    log "pre-commit hooks installed."
fi

# ---- /dev/shm check ---------------------------------------------------------
SHM_AVAIL_KB=$(df -k /dev/shm 2>/dev/null | tail -1 | awk '{print $4}' || echo 0)
SHM_AVAIL_MB=$(( SHM_AVAIL_KB / 1024 ))
if (( SHM_AVAIL_MB < 512 )); then
    warn "/dev/shm has only ${SHM_AVAIL_MB} MiB available (need >= 512 MiB)."
    warn "In Docker, pass --shm-size=2g to docker run."
fi

# ---- RLIMIT_MEMLOCK ---------------------------------------------------------
MEMLOCK="$(ulimit -l 2>/dev/null || echo 0)"
if [[ "${MEMLOCK}" != "unlimited" && "${MEMLOCK}" -lt 65536 ]]; then
    warn "RLIMIT_MEMLOCK is ${MEMLOCK} KB. mlock() may fail."
    warn "In Docker, pass --ulimit memlock=-1:-1."
fi

# ---- Capability detection ---------------------------------------------------
log "Detecting optional kernel features..."
detect() {
    local name="$1" file="$2"
    if [[ -e "${file}" ]]; then
        log "  ${name}: available"
    else
        warn "  ${name}: not available (fallback path will be used)"
    fi
}

[[ -f /proc/sys/kernel/random/boot_id ]] && log "  boot_id: available" || warn "  boot_id: not available"
python3 -c "import ctypes; libc=ctypes.CDLL(None); libc.pidfd_open(0,0)" 2>/dev/null \
    && log "  pidfd: available" || warn "  pidfd: not available (fallback to /proc)"
grep -q "memfd_create" /proc/kallsyms 2>/dev/null \
    && log "  memfd: likely available" || warn "  memfd: status unknown"

# ---- Build ------------------------------------------------------------------
log "Configuring dev preset..."
cmake --preset dev -S "${ROOT}" > /dev/null

log "Building..."
cmake --build --preset dev --parallel

log "Running unit tests..."
ctest --preset unit --test-dir "${ROOT}/build/dev"

log ""
log "Bootstrap complete. You are ready to develop."
log "  cmake --preset dev && cmake --build --preset dev && ctest --preset dev"
