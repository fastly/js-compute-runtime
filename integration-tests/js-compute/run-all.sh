#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"/../..

failed=

if ! command -v npm > /dev/null; then
  echo "Unable to find npm"
  failed=1
fi

if ! command -v cargo > /dev/null; then
  echo "Unable to find cargo"
  failed=1
fi

if ! command -v viceroy > /dev/null; then
  echo "Unable to find viceroy, please install it by running: cargo install viceroy"
  failed=1
fi

if [ -n "$failed" ]; then
  exit 1
fi

if [ ! -f "target/release/js-compute-runtime" ]; then
  cargo build --release
fi

# build the action
(
  cd .github/actions/compute-sdk-test
  npm ci
)

node ".github/actions/compute-sdk-test/main.js"
