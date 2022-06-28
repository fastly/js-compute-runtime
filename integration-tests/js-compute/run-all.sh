#!/usr/bin/env bash

set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")"/../.. && pwd)"
action="${root}/.github/actions/compute-sdk-test"

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

if [ ! -f "${root}/target/release/js-compute-runtime" ]; then
  cd "${root}"
  cargo build --release
fi

# build the action
cd "${action}"
npm ci

cd "$root"

node "${action}/main.js"
