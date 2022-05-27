#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

success=
for file in $(git ls-files | grep '\.sh$'); do
  if ! shellcheck "${file}"; then
    success=1
  fi
done

if [ -n "${success}" ]; then
  exit 1
fi
