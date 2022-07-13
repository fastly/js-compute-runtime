#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

failed=0
for manifest in $(git ls-files | grep Cargo.toml); do
  if ! cargo fmt --manifest-path="$manifest" -- --check; then
    failed=1
  fi
done

exit "$failed"
