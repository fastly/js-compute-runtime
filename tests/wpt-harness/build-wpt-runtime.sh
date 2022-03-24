#!/bin/bash
set -e

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

cat $script_dir/pre-harness.js $script_dir/wpt/resources/testharness.js $script_dir/post-harness.js | wizer --allow-wasi --dir=. -r _start=wizer.resume -o wpt-runtime.wasm js-compute-runtime.wasm
