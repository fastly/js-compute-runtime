#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -lt 2 ]; then
  cat <<EOF
Usage: $0 <test-fixture> <override_host>

Replace the host part of each backend's url with another one. This will only
replace instances of "JS_COMPUTE_TEST_BACKEND", as other urls might be
meaningful for an individual test.
EOF
  exit 1
fi

fixtures_dir="$(dirname "${BASH_SOURCE[0]}")/fixtures"

fastly_toml="$fixtures_dir/$1/fastly.toml"
override_host="$2"

sed -i'' -e "s|JS_COMPUTE_TEST_BACKEND|$override_host|" "$fastly_toml"
