#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

failed=
for file in $(git ls-files | grep '\.sh$'); do
  if ! shellcheck "${file}"; then
    failed=1
  fi
done

if [ -n "${failed}" ]; then
  exit 1
fi
