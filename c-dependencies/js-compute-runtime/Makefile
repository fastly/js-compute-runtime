# DEFINES := -DDEBUG -DJS_DEBUG

INIT_JS ?= test.js

ROOT_SRC?=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))/..
SM_SRC=$(ROOT_SRC)/spidermonkey/
SM_OBJ=$(SM_SRC)lib/*
FSM_SRC=$(ROOT_SRC)/js-compute-runtime/

WASI_CC ?= /opt/wasi-sdk/bin/clang
WASI_CXX ?= /opt/wasi-sdk/bin/clang++
WIZER ?= wizer

CXX_FLAGS := -std=gnu++17 -Wall -Werror
CXX_OPT ?= O2

ifeq (,$(findstring g,$(CXX_OPT)))
ifneq (,$(shell which wasm-opt))
WASM_STRIP = wasm-opt --strip-debug -o $@ $@
endif
endif

FSM_CPP := $(wildcard $(FSM_SRC)*.cpp)
FSM_DEP := $(patsubst $(FSM_SRC)%.cpp,$(OBJDIR)%.d,$(FSM_CPP))
FSM_OBJ := $(patsubst $(FSM_SRC)%.cpp,$(OBJDIR)%.o,$(FSM_CPP))
RUST_URL_SRC := $(FSM_SRC)rust-url
RUST_URL_RS_FILES := $(shell find $(RUST_URL_SRC)/src -name '*.rs')
RUST_URL_LIB := $(RUST_URL_SRC)/target/wasm32-wasi/release/librust_url.a

SM_RUST_LIB = $(wildcard libjsrust/*.o)

.PHONY: all clean

all: js-compute-runtime.wasm

-include $(FSM_DEP)

$(RUST_URL_LIB): $(RUST_URL_RS_FILES) $(RUST_URL_SRC)/Cargo.toml $(RUST_URL_SRC)/cbindgen.toml $(FSM_SRC)Makefile
	cd $(RUST_URL_SRC) && cbindgen --output rust-url.h
	cd $(RUST_URL_SRC) && cargo build --target=wasm32-wasi --release

%.o: $(FSM_SRC)%.cpp $(FSM_SRC)Makefile $(RUST_URL_LIB)
	$(WASI_CXX) $(CXX_FLAGS) $(DEFINES) -I $(SM_SRC)include -$(CXX_OPT) -MMD -MP -c -o $@ $<

js-compute-runtime.wasm: $(FSM_OBJ) $(SM_OBJ) $(RUST_URL_LIB)
	$(WASI_CXX) $(CXX_FLAGS) $(DEFINES) -fvisibility=hidden -Wl,--allow-undefined -Wl,--stack-first -Wl,-z,stack-size=1048576 -mexec-model=command -$(CXX_OPT) -o $@ $^
	$(WASM_STRIP)

initialized-js-compute-runtime.wasm: js-compute-runtime.wasm $(INIT_JS)
	cat $(INIT_JS) | $(WIZER) --allow-wasi -r _start=wizer.resume -o $@ $<
