#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <test>"
  exit 1
fi

test="$1"

cd "$(dirname "${BASH_SOURCE[0]}")"
root="$(pwd)/../.."

npm ci -s

if [ -f "fixtures/$test/$test.ts" ]; then
  # TODO: we explicitly ignore errors from webpack, as the tests includes uses
  # of apis that are not yet typed by /sdk/js-compute/index.t.ts
  npm run webpack -- "./fixtures/$test/$test.ts" \
    --output-path "./fixtures/$test" \
    --output-filename "$test.js" || true
else
  echo "Skipping typescript conversion for fixtures/$test/$test.js"
fi

# NOTE: we run `js-compute-runtime` in the test directory, as there are some
# assumptions about project path that are derived from the cwd of the executable
# instead of the location of the js source.
(
  cd "fixtures/$test"
  "$root/target/release/js-compute-runtime" "$test.js" "$test.wasm"
)
