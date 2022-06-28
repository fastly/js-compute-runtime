#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <test>"
  exit 1
fi

test="$1"

cd "$(dirname "${BASH_SOURCE[0]}")"

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

../../target/release/js-compute-runtime "fixtures/$test/$test.js" "fixtures/$test/$test.wasm"
