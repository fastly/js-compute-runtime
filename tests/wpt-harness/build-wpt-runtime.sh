#!/usr/bin/env bash

set -euo pipefail
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

inputs=(
  "${script_dir}/pre-harness.js"
  "${script_dir}/wpt/resources/testharness.js"
  "${script_dir}/post-harness.js"
)

cat "${inputs[@]}" > "${script_dir}/wpt-test-runner.js"
node "${script_dir}/../../js-compute-runtime-cli.js" "${script_dir}/wpt-test-runner.js" wpt-runtime.wasm
