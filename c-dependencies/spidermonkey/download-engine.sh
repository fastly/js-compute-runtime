#!/usr/bin/env bash

set -euo pipefail

display_help() {
  cat <<EOF
Usage $0: [options]

debug        Fetch a debug build of spidermonkey
-h, --help   Display this message
EOF
}

mode="release"
while [ "$#" -gt 0 ]; do
  case $1 in

    debug)
      mode="debug"
      ;;

    *)
      display_help
      exit 1
      ;;

  esac
  shift 1
done

cd "$(dirname "${BASH_SOURCE[0]}")"

# The release sha for the build we currently depend on from
# `tschneidereit/spidermonkey-wasi-embedding`
RELEASE="cd81b695e31ec65e4dbbd2a0a6255fda69ab7802"

base_url="https://github.com/tschneidereit/spidermonkey-wasi-embedding/"

file="spidermonkey-wasm-static-lib_${mode}.tar.gz"

bundle_url="${base_url}/releases/download/rev_${RELEASE}/${file}"

curl --fail -L -O "$bundle_url"
trap 'rm $file' EXIT

tar xf "$file"
