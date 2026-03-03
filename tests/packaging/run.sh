#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
STAGE_DIR="${SOURCE_DIR}/build/staging"

log() { echo "[packaging] $*"; }
die() { echo "[packaging] ERROR: $*" >&2; exit 1; }

# Stage the install
log "Building and staging install..."
cmake -S "${SOURCE_DIR}" -B "${SOURCE_DIR}/build/pkg-release" \
    -DCMAKE_BUILD_TYPE=Release \
    -DREVENANT_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX="${STAGE_DIR}" > /dev/null
cmake --build "${SOURCE_DIR}/build/pkg-release" --parallel > /dev/null
cmake --install "${SOURCE_DIR}/build/pkg-release" > /dev/null

# Test 1: find_package
log "Testing find_package..."
BUILD="${SOURCE_DIR}/build/pkg-find_package"
cmake -S "${SCRIPT_DIR}/find_package" -B "${BUILD}" \
    -DCMAKE_PREFIX_PATH="${STAGE_DIR}" \
    -DCMAKE_CXX_STANDARD=20 > /dev/null
cmake --build "${BUILD}" --parallel > /dev/null
"${BUILD}/consumer" | grep -q "." || die "find_package consumer produced no output"
log "  find_package OK"

# Test 2: FetchContent (pass source dir, avoids network)
log "Testing FetchContent..."
BUILD="${SOURCE_DIR}/build/pkg-fetch_content"
cmake -S "${SCRIPT_DIR}/fetch_content" -B "${BUILD}" \
    -DREVENANT_SOURCE_DIR="${SOURCE_DIR}" \
    -DCMAKE_CXX_STANDARD=20 > /dev/null
cmake --build "${BUILD}" --parallel > /dev/null
"${BUILD}/consumer" | grep -q "." || die "FetchContent consumer produced no output"
log "  FetchContent OK"

# Test 3: add_subdirectory
log "Testing add_subdirectory..."
BUILD="${SOURCE_DIR}/build/pkg-add_subdirectory"
cmake -S "${SCRIPT_DIR}/add_subdirectory" -B "${BUILD}" \
    -DREVENANT_SOURCE_DIR="${SOURCE_DIR}" \
    -DCMAKE_CXX_STANDARD=20 > /dev/null
cmake --build "${BUILD}" --parallel > /dev/null
"${BUILD}/consumer" | grep -q "." || die "add_subdirectory consumer produced no output"
log "  add_subdirectory OK"

log "All packaging tests passed."
