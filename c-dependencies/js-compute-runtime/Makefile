INIT_JS ?= test.js
WIZER ?= wizer

CXX_OPT ?= -O2

MODE=release
CARGO_FLAG=--release
ifdef DEBUG
  MODE=debug
  CARGO_FLAG=
  CXX_OPT := $(CXX_OPT) -DDEBUG -DJS_DEBUG
endif

ROOT_SRC?=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))/..
SM_SRC=$(ROOT_SRC)/spidermonkey/$(MODE)/
SM_OBJ=$(SM_SRC)lib/*.o
SM_OBJ+=$(SM_SRC)lib/*.a
FSM_SRC=$(ROOT_SRC)/js-compute-runtime/

WASI_CXX ?= /opt/wasi-sdk/bin/clang++


CXX_FLAGS := -std=gnu++17 -Wall -Werror -Qunused-arguments -fno-sized-deallocation -fno-aligned-new -mthread-model single -fPIC -fno-rtti -fno-exceptions -fno-math-errno -pipe -fno-omit-frame-pointer -funwind-tables
LD_FLAGS := -Wl,-z,noexecstack -Wl,-z,text -Wl,-z,relro -Wl,-z,nocopyreloc -Wl,-z,stack-size=1048576 -Wl,--stack-first

.PHONY: all

all: js-compute-runtime.wasm

compiler_flags:
	echo '$(CXX_OPT)' | cmp -s - $@ || echo '$(CXX_OPT)' > $@

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
RUST_URL_LIB := rusturl/wasm32-wasi/$(MODE)/librust_url.a

-include $(FSM_DEP)

$(RUST_URL_LIB): $(RUST_URL_RS_FILES) $(RUST_URL_SRC)/Cargo.toml $(RUST_URL_SRC)/cbindgen.toml $(FSM_SRC)Makefile compiler_flags
	cd $(RUST_URL_SRC) && cbindgen --output rust-url.h
	cargo build --manifest-path $(RUST_URL_SRC)/Cargo.toml --target-dir ./rusturl --target=wasm32-wasi $(CARGO_FLAG)

%.o: $(FSM_SRC)%.cpp $(FSM_SRC)Makefile $(RUST_URL_LIB) compiler_flags
	$(WASI_CXX) $(CXX_FLAGS) $(CXX_OPT) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

js-compute-runtime.wasm: $(FSM_OBJ) $(SM_OBJ) $(RUST_URL_LIB)
	$(WASI_CXX) $(CXX_FLAGS) $(CXX_OPT) $(DEFINES) $(LD_FLAGS) -o $@ $^
	$(WASM_STRIP)

initialized-js-compute-runtime.wasm: js-compute-runtime.wasm $(INIT_JS)
	cat $(INIT_JS) | $(WIZER) --allow-wasi --dir=. -r _start=wizer.resume -o $@ $<
