#!/usr/bin/env bash

set -euo pipefail

usage() {
  cat <<EOF
Usage: $0 [-h] [--fix]

  Format c/c++ source that's tracked by git. Exits with a non-zero return code
  when formatting is applied.

  -h     Print this message
  --fix  Format files in-place
EOF
}

fix=
while [ "$#" -gt 0 ] ; do
  case $1 in

    -h)
      usage
      exit 1
      ;;

    --fix)
      fix=1
      ;;

    *)
      echo "Unrecognized option: $1"
      echo
      usage
      exit 1
      ;;

  esac

  shift
done

cd "$(dirname "${BASH_SOURCE[0]}")/.."

failure=
for file in $(git ls-files | grep '\.\(cpp\|h\)$'); do
  if grep -F -x "$file" ci/clang-format.ignore > /dev/null; then
    continue
  fi

  formatted="${file}.formatted"
  /opt/wasi-sdk/bin/clang-format "$file" > "$formatted"
  if ! cmp -s "$file" "$formatted"; then
    if [ -z "$fix" ]; then
      rm "$formatted"
      echo "${file} needs formatting"
      failure=1
    else
      echo "${file} formatted"
      mv "$formatted" "$file"
    fi
  fi

  rm -f "$formatted"
done

if [ -n "$failure" ]; then
  exit 1
fi
