#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <test>"
  exit 1
fi

test="$1"

# NOTE: we run `js-compute-runtime` in the test directory, as there are some
# assumptions about project path that are derived from the cwd of the executable
# instead of the location of the js source.
(
  cd "$(dirname "${BASH_SOURCE[0]}")/fixtures/$test"
  fastly compute build -i
)
