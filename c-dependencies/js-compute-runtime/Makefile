SHELL:=/bin/bash
INIT_JS ?= test.js
WIZER ?= wizer
DESTDIR ?= .
WASI_SDK ?= /opt/wasi-sdk
OPENSSL_VERSION = 3.0.7

OPT_FLAGS ?= -O2

DEBUG ?= false
ifneq ($(DEBUG),false)
  MODE := debug
  CARGO_FLAG :=
  OPT_FLAGS += -DDEBUG -DJS_DEBUG -g
  Q :=
  quiet_flag =
else
  MODE := release
  CARGO_FLAG := --release
  Q := @
  quiet_flag = $1
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

CXX_FLAGS := -std=gnu++20 -Wall -Werror -Qunused-arguments
CXX_FLAGS += -fno-sized-deallocation -fno-aligned-new -mthread-model single
CXX_FLAGS += -fPIC -fno-rtti -fno-exceptions -fno-math-errno -pipe
CXX_FLAGS += -fno-omit-frame-pointer -funwind-tables -I$(FSM_SRC)
CXX_FLAGS += --sysroot=$(WASI_SDK)/share/wasi-sysroot

CFLAGS := -Wall -Werror -Wno-unknown-attributes -Wno-pointer-to-int-cast
CFLAGS += -Wno-int-to-pointer-cast --sysroot=$(WASI_SDK)/share/wasi-sysroot

LD_FLAGS := -Wl,-z,stack-size=1048576 -Wl,--stack-first
LD_FLAGS += -lwasi-emulated-signal
LD_FLAGS += -lwasi-emulated-process-clocks
LD_FLAGS += -lwasi-emulated-getpid

DEFINES ?=

# This is required when using spidermonkey headers, as it allows us to enable
# the streams library when setting up the js context.
DEFINES += -DMOZ_JS_STREAMS

.PHONY: all install clean distclean

all: js-compute-runtime.wasm js-compute-runtime-component.wasm | wasi_snapshot_preview1.wasm

compiler_flags:
	echo '$(OPT_FLAGS) $(CXX_FLAGS)' | cmp -s - $@ || echo '$(OPT_FLAGS) $(CXX_FLAGS)' > $@

ifeq (,$(findstring g,$(OPT_FLAGS)))
ifneq (,$(shell which wasm-opt))
WASM_STRIP = wasm-opt --strip-debug -o $1 $1
endif
endif

OBJ_DIR := obj
FSM_CPP := $(wildcard $(FSM_SRC)*.cpp) $(wildcard $(FSM_SRC)builtins/*.cpp)
FSM_DEP := $(patsubst $(FSM_SRC)%.cpp,$(OBJ_DIR)/%.d,$(FSM_CPP)) 
FSM_OBJ := $(patsubst $(FSM_SRC)%.cpp,$(OBJ_DIR)/%.o,$(FSM_CPP))
RUST_URL_SRC := $(FSM_SRC)rust-url
RUST_URL_RS_FILES := $(shell find $(RUST_URL_SRC)/src -name '*.rs')
RUST_URL_LIB := rusturl/wasm32-wasi/$(MODE)/librust_url.a

-include $(FSM_DEP)

$(RUST_URL_LIB): $(RUST_URL_RS_FILES) $(RUST_URL_SRC)/Cargo.toml $(RUST_URL_SRC)/cbindgen.toml $(FSM_SRC)Makefile compiler_flags
	cd $(RUST_URL_SRC) && cbindgen --output rust-url.h
	cargo build --manifest-path $(RUST_URL_SRC)/Cargo.toml --target-dir ./rusturl --target=wasm32-wasi $(CARGO_FLAG)

$(OBJ_DIR):
	$Q mkdir -p $(OBJ_DIR)/builtins $(OBJ_DIR)/xqd-world

$(OBJ_DIR)/openssl-$(OPENSSL_VERSION).tar.gz: | $(OBJ_DIR)
	$Q wget https://www.openssl.org/source/openssl-$(OPENSSL_VERSION).tar.gz \
		$(call quiet_flag,--quiet) -O $@

$(OBJ_DIR)/openssl-$(OPENSSL_VERSION)/token: $(OBJ_DIR)/openssl-$(OPENSSL_VERSION).tar.gz $(FSM_SRC)/getuid.patch
	$Q tar -C $(OBJ_DIR) -xf $<
	$Q patch -d $(OBJ_DIR)/openssl-$(OPENSSL_VERSION) -p1 < $(FSM_SRC)/getuid.patch
	$Q touch $@

OPENSSL_OPTS := -static -no-sock -no-asm -no-ui-console -no-egd
OPENSSL_OPTS += -no-afalgeng -no-tests -no-stdio -no-threads
OPENSSL_OPTS += -D_WASI_EMULATED_SIGNAL
OPENSSL_OPTS += -D_WASI_EMULATED_PROCESS_CLOCKS
OPENSSL_OPTS += -D_WASI_EMULATED_GETPID
OPENSSL_OPTS += -DHAVE_FORK=0
OPENSSL_OPTS += -DNO_SYSLOG
OPENSSL_OPTS += -DNO_CHMOD
OPENSSL_OPTS += -DOPENSSL_NO_SECURE_MEMORY
OPENSSL_OPTS += --with-rand-seed=getrandom
OPENSSL_OPTS += --prefix=$(FSM_SRC)/$(OBJ_DIR)/openssl
OPENSSL_OPTS += --cross-compile-prefix=$(WASI_SDK)/bin/
OPENSSL_OPTS += linux-x32

OPENSSL_DISABLED_WARNINGS := -Wno-unused-command-line-argument
OPENSSL_DISABLED_WARNINGS += -Wno-constant-conversion
OPENSSL_DISABLED_WARNINGS += -Wno-shift-count-overflow

$(OBJ_DIR)/openssl/token: $(OBJ_DIR)/openssl-$(OPENSSL_VERSION)/token
	export WASI_SDK_PATH=$(WASI_SDK) && \
		cd $(OBJ_DIR)/openssl-$(OPENSSL_VERSION) && \
		CC=clang \
		CFLAGS="--sysroot=$(WASI_SDK)/share/wasi-sysroot" \
		./Configure $(OPENSSL_OPTS) && \
		$(MAKE) -j8 && \
		$(MAKE) install_sw
	touch $@

OPENSSL_CFLAGS := -I$(OBJ_DIR)/openssl/include
OPENSSL_LIBS := -L$(OBJ_DIR)/openssl/libx32 -lcrypto

$(OBJ_DIR)/%.o: $(FSM_SRC)%.cpp $(OBJ_DIR)/openssl/token $(FSM_SRC)Makefile $(RUST_URL_LIB) compiler_flags | $(OBJ_DIR)
	$(WASI_CXX) $(CXX_FLAGS) $(OPENSSL_CFLAGS) $(OPT_FLAGS) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

$(OBJ_DIR)/%.o: $(FSM_SRC)%.c $(FSM_SRC)Makefile $(RUST_URL_LIB) compiler_flags | $(OBJ_DIR)
	$(WASI_CC) $(CFLAGS) $(OPT_FLAGS) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

$(OBJ_DIR)/xqd-world/xqd_world.o: $(FSM_SRC)xqd-world/xqd_world.c $(FSM_SRC)Makefile compiler_flags | $(OBJ_DIR)
	$(WASI_CC) $(CFLAGS) $(OPT_FLAGS) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

$(OBJ_DIR)/xqd-world/xqd_world_adapter.o: $(FSM_SRC)xqd-world/xqd_world_adapter.cpp $(FSM_SRC)Makefile compiler_flags | $(OBJ_DIR)
	$(WASI_CXX) $(CXX_FLAGS) $(OPT_FLAGS) $(DEFINES) -I $(SM_SRC)include -MMD -MP -c -o $@ $<

$(OBJ_DIR)/xqd-world/xqd_world_adapter_component.o: $(FSM_SRC)xqd-world/xqd_world_adapter.cpp $(FSM_SRC)Makefile compiler_flags | $(OBJ_DIR)
	$(WASI_CXX) $(CXX_FLAGS) $(OPT_FLAGS) $(DEFINES) -DCOMPONENT -I $(SM_SRC)include -MMD -MP -c -o $@ $<

# NOTE: we shadow wasm-opt by adding $(FSM_SRC)/scripts to the path, which
# includes a script called wasm-opt that immediately exits successfully. See
# that script for more information about why we do this.

js-compute-runtime.wasm: $(FSM_OBJ) $(SM_OBJ) $(RUST_URL_LIB)
js-compute-runtime.wasm: $(OBJ_DIR)/xqd-world/xqd_world.o
js-compute-runtime.wasm: $(OBJ_DIR)/xqd-world/xqd_world_adapter.o
	PATH="$(FSM_SRC)/scripts:$$PATH" $(WASI_CXX) $(CXX_FLAGS) $(OPT_FLAGS) \
	     $(DEFINES) $(LD_FLAGS) $(OPENSSL_LIBS) -o $@ $^
	$(call WASM_STRIP,$@)

js-compute-runtime-component.wasm: $(FSM_OBJ) $(SM_OBJ) $(RUST_URL_LIB)
js-compute-runtime-component.wasm: $(OBJ_DIR)/xqd-world/xqd_world.o
js-compute-runtime-component.wasm: $(OBJ_DIR)/xqd-world/xqd_world_adapter_component.o
	PATH="$(FSM_SRC)/scripts:$$PATH" $(WASI_CXX) $(CXX_FLAGS) $(OPT_FLAGS) \
	     $(DEFINES) $(LD_FLAGS) $(OPENSSL_LIBS) -o $@ $^
	$(call WASM_STRIP,$@)

install: js-compute-runtime.wasm
	install -m 444 -Dt $(DESTDIR)/dist js-compute-runtime.wasm

wasi_snapshot_preview1.wasm:
	curl -L https://github.com/bytecodealliance/preview2-prototyping/releases/download/latest/wasi_snapshot_preview1.wasm -o $@

regenerate-world:
	$(WIT_BINDGEN) guest c xqd.wit --no-helpers --out-dir xqd-world

clean:
	$(RM) compile_commands.json $(FSM_OBJ) xqd-world/xqd_world_component_type.o

distclean: clean
	$(RM) -r $(OBJ_DIR)
	$(RM) compiler_flags

.PHONY: compile_commands.json
compile_commands.json:
	$Q ( \
		sep="["; \
		for file in $(FSM_CPP); do \
			echo "$$sep"; \
			sep=","; \
			echo "{ \"directory\": \"$(FSM_SRC)\","; \
			echo "  \"command\": \"$(WASI_CXX) $(CXX_FLAGS) $(OPENSSL_CFLAGS) $(DEFINES) -I $(SM_SRC)include\","; \
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
