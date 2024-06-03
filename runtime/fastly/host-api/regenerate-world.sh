#!/bin/bash

# cargo install --git https://github.com/bytecodealliance/wit-bindgen wit-bindgen-cli --no-default-features --features c
wit-bindgen c --no-helpers --out-dir component --world fastly-world wit
