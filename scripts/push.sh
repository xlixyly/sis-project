#!/usr/bin/env bash
set -euo pipefail

cleanup() {
  local exit_code=$?
  if [ "$exit_code" -ne 0 ]; then
    echo "Error in $(basename "$0") (exit code: $exit_code)" >&2
  fi
}
trap cleanup EXIT

echo "Skeleton for $(basename "$0")"
