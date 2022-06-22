#!/usr/bin/env bash

set -euo pipefail

if [ "$#" -lt 2 ]; then
  cat <<EOF
Usage: $0 <test-fixture> <override_host_and_protocol>

Replace the host and protocol part of each backend's url with another one. This
will only replace instances of "JS_COMPUTE_TEST_BACKEND", as other urls might be
meaningful for an individual test.
EOF
  exit 1
fi

fixtures_dir="$(dirname "${BASH_SOURCE[0]}")/fixtures"

fastly_toml="$fixtures_dir/$1/fastly.toml"
fastly_toml_in="$fixtures_dir/$1/fastly.toml.in"
override_host="$2"

sed -e "s|JS_COMPUTE_TEST_BACKEND|$override_host|" "$fastly_toml_in" > "$fastly_toml"
