#!/usr/bin/env bash

set -euo pipefail
set -x

cd "$(dirname "${BASH_SOURCE[0]}")"
root="$(pwd)"

# Ensure the Rust version matches that used by Gecko, and can compile to WASI
rustup target add wasm32-wasi
# Use Gecko's build system bootstrapping to ensure all dependencies are
# installed
cd "$root/gecko-dev"
./mach --no-interactive bootstrap --application-choice=js --no-system-changes

# ... except, that doesn't install the wasi-sysroot, which we need, so we do
# that manually.
cd ~/.mozbuild
python3 \
  "${root}/gecko-dev/mach" \
  --no-interactive \
  artifact \
  toolchain \
  --bootstrap \
  --from-build \
  sysroot-wasm32-wasi

cd "$root"
