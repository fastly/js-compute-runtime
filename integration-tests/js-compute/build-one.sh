#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <test>"
  exit 1
fi

test="$1"

cd "$(dirname "${BASH_SOURCE[0]}")"

npm install -s
npm run -s build:test --test="$test"

cd fixtures/"$test"
fastly compute pack --verbose --wasm-binary "./${test}.wasm"
