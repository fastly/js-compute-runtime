#!/usr/bin/env bash

# Use this script to run the wpt test harness locally. You'll need to have the
# following executables in your path:
#
# * wizer
# * npm
# * node
#
# From any directory, run this script and it will do the following:
#
# 1. build js-compute-runtime.wasm and move it into the current directory
# 2. build the wpt runner
# 3. run the test suite and forward any arguments to it
#
# If you'd like to run a debug build, set the `DEBUG` environment variable when
# running this script:
#
# > mkdir my_test
# > cd my_test
# > DEBUG=true WEVAL=true ../tests/wpt-harness/run-wpt.sh
#
# For this to work, you'll need to have run the following command in advance:
#
# > cd runtime/spidermonkey
# > ./download-engine.sh debug
#
# If you get an error about missing "jsapi.h" while building the runtime,
# something's gone wrong with the engine download.

set -euo pipefail

working_dir="$(pwd)"
root="$(dirname "${BASH_SOURCE[0]}")/../.."

output="$(mktemp)"
trap 'rm $output' EXIT

echo "Building the runtime..."
cd "$root"
if ! npm run build > "$output" 2>&1; then
  cat "$output"
  exit 1
fi

cd "$working_dir"

echo "Building the wpt runtime..."
bash "$root/tests/wpt-harness/build-wpt-runtime.sh"

echo "Running the wpt tests..."
node "$root/tests/wpt-harness/run-wpt.mjs" "$@"
