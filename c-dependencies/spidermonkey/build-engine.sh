#!/usr/bin/env bash

set -euo pipefail
set -x

working_dir="$(pwd)"
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Ensure apt-get is current, because otherwise bootstrapping might fail
sudo apt-get update -y

# Ensure the Rust version matches that used by Gecko, and can compile to WASI
rustup update 1.57.0
rustup override set 1.57.0
rustup target add wasm32-wasi

if [[ ! -a gecko-dev ]]
then

  # Clone Gecko repository at the required revision
  mkdir gecko-dev
  cd gecko-dev

  git init
  git remote add --no-tags -t wasi-embedding origin "$(cat "$script_dir/gecko-repository")"
  cd ..
fi

cd gecko-dev
git fetch --depth 1 origin "$(cat "$script_dir/gecko-revision")"
git checkout FETCH_HEAD

# Use Gecko's build system bootstrapping to ensure all dependencies are installed
./mach bootstrap --application-choice=js

# ... except, that doesn't install the wasi-sysroot, which we need, so we do that manually.
cd ~/.mozbuild
python3 \
  "${working_dir}/gecko-dev/mach" \
  artifact \
  toolchain \
  --bootstrap \
  --from-build \
  sysroot-wasm32-wasi

cd "${working_dir}"

mode="release"
flags=( --optimize )
if [[ "${1:-}" == 'debug' ]]; then
  mode="debug"
  flags+=(--debug)
else
  flags+=(--no-debug --build-only)
fi
rust_lib_dir="$mode"
objdir="obj-$mode"
outdir="$mode"

echo -- "${flags[@]}" "$rust_lib_dir"

# Build SpiderMonkey for WASI
MOZ_FETCHES_DIR=~/.mozbuild \
  CC=~/.mozbuild/clang/bin/clang \
  gecko-dev/js/src/devtools/automation/autospider.py \
  --objdir="$objdir" "${flags[@]}" wasi

# Copy header, object, and static lib files
rm -rf "$outdir"
mkdir -p "$outdir/lib"

cd "$objdir"
cp -Lr dist/include "../$outdir"

# We're relying on word splitting here to copy all the files mentioned in
# object-files.list
# shellcheck disable=SC2046
cp $(cat "$script_dir/object-files.list") ../$outdir/lib

cp js/src/build/libjs_static.a wasm32-wasi/${rust_lib_dir}/libjsrust.a ../$outdir/lib
