#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CHANGED_ONLY=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --changed-only) CHANGED_ONLY=true; shift ;;
        *) echo "Unknown argument: $1" >&2; exit 1 ;;
    esac
done

if ${CHANGED_ONLY}; then
    FILES="$(git -C "${ROOT}" diff --name-only --cached HEAD -- '*.cpp' '*.hpp' '*.h' '*.cc')"
else
    FILES="$(find "${ROOT}/include" "${ROOT}/src" "${ROOT}/tests" -name '*.cpp' -o -name '*.hpp' -o -name '*.h' 2>/dev/null || true)"
fi

if [[ -z "${FILES}" ]]; then
    echo "[format] No files to format."
    exit 0
fi

echo "[format] Running clang-format..."
echo "${FILES}" | xargs clang-format -i --style=file
echo "[format] Done."
