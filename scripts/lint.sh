#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CHANGED_ONLY=false
FAILED=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --changed-only) CHANGED_ONLY=true; shift ;;
        *) echo "Unknown argument: $1" >&2; exit 1 ;;
    esac
done

if ${CHANGED_ONLY}; then
    FILES="$(git -C "${ROOT}" diff --name-only --cached HEAD -- '*.cpp' '*.hpp' '*.h' '*.cc' 2>/dev/null || true)"
else
    FILES="$(find "${ROOT}/include" "${ROOT}/src" "${ROOT}/tests" -name '*.cpp' -o -name '*.hpp' -o -name '*.h' 2>/dev/null || true)"
fi

if [[ -n "${FILES}" ]] && command -v clang-format &>/dev/null; then
    echo "[lint] clang-format check..."
    if ! echo "${FILES}" | xargs clang-format --dry-run --Werror --style=file 2>/dev/null; then
        echo "[lint] clang-format violations found. Run scripts/format.sh" >&2
        FAILED=1
    fi
fi

if command -v codespell &>/dev/null; then
    echo "[lint] codespell..."
    codespell "${ROOT}/include" "${ROOT}/src" "${ROOT}/docs" \
        --skip="*.pc.in,*.pc" --quiet-level=2 || FAILED=1
fi

exit ${FAILED}
