SHELL:=/bin/bash
INIT_JS ?= test.js
WIZER ?= wizer
DESTDIR ?= .
WASI_SDK ?= /opt/wasi-sdk

CXX_OPT ?= -O2

DEBUG ?= false
ifneq ($(DEBUG),false)
  MODE := debug
  CARGO_FLAG :=
  CXX_OPT := $(CXX_OPT) -DDEBUG -DJS_DEBUG -g
  Q :=
else
  MODE := release
  CARGO_FLAG := --release
  Q := @
endif

ROOT_SRC ?= $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))/..
SM_SRC := $(ROOT_SRC)/spidermonkey/$(MODE)/
SM_OBJ := $(SM_SRC)lib/*.o
SM_OBJ += $(SM_SRC)lib/*.a
FSM_SRC := $(ROOT_SRC)/js-compute-runtime/

WASI_CXX ?= $(WASI_SDK)/bin/clang++
WASI_CC ?= $(WASI_SDK)/bin/clang

WIT_BINDGEN := $(shell which wit-bindgen)

ifndef WIT_BINDGEN
	WIT_BINDGEN = $(error No wit-bindgen in PATH, consider doing cargo install --git https://github.com/bytecodealliance/wit-bindgen wit-bindgen-cli)
endif

CXX_FLAGS := -std=gnu++17 -Wall -Werror -Qunused-arguments
CXX_FLAGS += -fno-sized-deallocation -fno-aligned-new -mthread-model single
CXX_FLAGS += -fPIC -fno-rtti -fno-exceptions -fno-math-errno -pipe
CXX_FLAGS += -fno-omit-frame-pointer -funwind-tables -I$(FSM_SRC)
CXX_FLAGS += --sysroot=$(WASI_SDK)/share/wasi-sysroot

CFLAGS := -Wall -Werror -Wno-unknown-attributes -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast

LD_FLAGS := -Wl,-z,stack-size=1048576 -Wl,--stack-first -lwasi-emulated-getpid

DEFINES ?=

# This is required when using spidermonkey headers, as it allows us to enable
# the streams library when setting up the js context.
DEFINES += -DMOZ_JS_STREAMS

.PHONY: all install clean distclean

all: js-compute-runtime.wasm

compiler_flags:
	echo '$(CXX_OPT) $(CXX_FLAGS)' | cmp -s - $@ || echo '$(CXX_OPT) $(CXX_FLAGS)' > $@

ifeq (,$(findstring g,$(CXX_OPT)))
ifneq (,$(shell which wasm-opt))
WASM_STRIP = wasm-opt --strip-debug -o $1 $1
endif
endif

OBJ_DIR := obj/
FSM_CPP := $(wildcard $(FSM_SRC)*.cpp) $(wildcard $(FSM_SRC)builtins/*.cpp)
FSM_DEP := $(patsubst $(FSM_SRC)%.cpp,$(OBJ_DIR)%.d,$(FSM_CPP)) 
FSM_OBJ := $(patsubst $(FSM_SRC)%.cpp,$(OBJ_DIR)%.o,$(FSM_CPP))
RUST_URL_SRC := $(FSM_SRC)rust-url
RUST_URL_RS_FILES := $(shell find $(RUST_URL_SRC)/src -name '*.rs')
RUST_URL_LIB := rusturl/wasm32-wasi/$(MODE)/librust_url.a

-include $(FSM_DEP)

$(RUST_URL_LIB): $(RUST_URL_RS_FILES) $(RUST_URL_SRC)/Cargo.toml $(RUST_URL_SRC)/cbindgen.toml $(FSM_SRC)Makefile compiler_flags
	cd $(RUST_URL_SRC) && cbindgen --output rust-url.h
	cargo build --manifest-path $(RUST_URL_SRC)/Cargo.toml --target-dir ./rusturl --target=wasm32-wasi $(CARGO_FLAG)

obj:
	$Q mkdir -p $(OBJ_DIR)builtins $(OBJ_DIR)xqd-world

$(OBJ_DIR)%.o: $(FSM_SRC)%.cpp $(FSM_SRC)Makefile $(RUST_URL_LIB) compiler_flags | obj
	$(WASI_CXX) $(CXX_FLAGS) $(CXX_OPT) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

$(OBJ_DIR)builtins/%.o: $(FSM_SRC)builtins/%.cpp $(FSM_SRC)Makefile $(RUST_URL_LIB) compiler_flags | obj
	$(WASI_CXX) $(CXX_FLAGS) $(CXX_OPT) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

$(OBJ_DIR)xqd-world/xqd_world.o: $(FSM_SRC)xqd-world/xqd_world.c $(FSM_SRC)Makefile compiler_flags
	$(WASI_CC) $(CFLAGS) $(CXX_OPT) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

$(OBJ_DIR)xqd-world/xqd_world_adapter.o: $(FSM_SRC)xqd-world/xqd_world_adapter.cpp $(FSM_SRC)Makefile compiler_flags
	$(WASI_CXX) $(CXX_FLAGS) $(CXX_OPT) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

$(OBJ_DIR)xqd-world/xqd_world_adapter_component.o: $(FSM_SRC)xqd-world/xqd_world_adapter.cpp $(FSM_SRC)Makefile compiler_flags
	$(WASI_CXX) $(CXX_FLAGS) $(CXX_OPT) $(DEFINES) -DCOMPONENT -I $(SM_SRC)include -MMD -MP -c -o $@ $<

js-compute-runtime.wasm: $(FSM_OBJ) $(SM_OBJ) $(OBJ_DIR)xqd-world/xqd_world.o $(OBJ_DIR)xqd-world/xqd_world_adapter.o $(RUST_URL_LIB)
	$(WASI_CXX) $(CXX_FLAGS) $(CXX_OPT) $(DEFINES) $(LD_FLAGS) -o $@ $^
	$(call WASM_STRIP,$@)

js-compute-runtime-component.wasm: $(FSM_OBJ) $(OBJ_DIR)xqd-world/xqd_world.o $(OBJ_DIR)xqd-world/xqd_world_adapter_component.o $(SM_OBJ) $(RUST_URL_LIB)
	$(WASI_CXX) $(CXX_FLAGS) $(CXX_OPT) $(DEFINES) $(LD_FLAGS) -o $@ $^
	$(call WASM_STRIP,$@)

install: js-compute-runtime.wasm
	install -m 444 -Dt $(DESTDIR)/dist js-compute-runtime.wasm

regenerate-world:
	$(WIT_BINDGEN) guest c xqd.wit --no-helpers --out-dir xqd-world

clean:
	$(RM) compile_commands.json $(FSM_OBJ) xqd-world/xqd_world_component_type.o

distclean: clean
	$(RM) $(FSM_DEP) $(FSM_DEP_COMPONENT) $(OBJ_DIR)xqd-world/*.o compiler_flags

.PHONY: compile_commands.json
compile_commands.json:
	$Q ( \
		sep="["; \
		for file in $(FSM_CPP); do \
			echo "$$sep"; \
			sep=","; \
			echo "{ \"directory\": \"$(FSM_SRC)\","; \
			echo "  \"command\": \"$(WASI_CXX) $(CXX_FLAGS) $(DEFINES) -I $(SM_SRC)include\","; \
			echo -n "  \"file\": \"$${file#$(FSM_SRC)}\"}"; \
		done; \
		echo; \
		echo ']' \
	) > "$@"

format: $(FSM_CPP)
	$(ROOT_SRC)/../ci/clang-format.sh --fix

# Useful for debugging, try `make print-FSM_CPP`
print-%:
	$Q echo "$($*)"
