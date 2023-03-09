
# Build constants ##############################################################

# The path to the directory containing this Makefile, the
# //c-dependencies/js-compute-runtime directory.
FSM_SRC := $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")

# The path to the //c-dependencies directory.
ROOT_SRC := $(shell dirname "$(FSM_SRC)")

# Environmentally derived config ###############################################

# Build verbosity, useful when debugging build failures. Setting it to anything
# will cause the quiet output to be disabled.
V ?=

# The destination directory when installing the resulting wasm binaries.
DESTDIR ?= .

# The path to the wasi-sdk installation.
WASI_SDK ?= /opt/wasi-sdk

# The version of OpenSSL to build with.
OPENSSL_VERSION = 3.0.7

# Whether or not this will be a debug build. When set to anything other than
# `false` the build will be treated as a debug build.
DEBUG ?= false

# The path to the wit-bindgen executable
WIT_BINDGEN ?= $(shell which wit-bindgen)

# Default optimization flgs for clang/clang++.
OPT_FLAGS ?= -O2

# Command helpers for making nice build output.
include mk/commands.mk

# Derived configuration ########################################################

# The wasi-sdk provided c++ compiler wrapper.
WASI_CXX ?= $(WASI_SDK)/bin/clang++

# The wasi-sdk provided c compiler wrapper.
WASI_CC ?= $(WASI_SDK)/bin/clang

# The wasi-sdk provided ar wrapper.
WASI_AR ?= $(WASI_SDK)/bin/ar

ifneq ($(DEBUG),false)
  MODE := debug
  CARGO_FLAG :=
  OPT_FLAGS += -DDEBUG -DJS_DEBUG -g

  # Define an empty WASM_STRIP macro when making a debug build
  WASM_STRIP =
else
  MODE := release
  CARGO_FLAG := --release

  # Strip binaries when making a non-debug build.
  WASM_STRIP = wasm-opt --strip-debug -o $1 $1
endif

# The base build directory, where all our build artifacts go.
BUILD := $(FSM_SRC)/build

# The output directory for the current build mode (relase/debug).
OBJ_DIR := $(BUILD)/$(MODE)

# The path to the //c-dependencies/spidermonkey/$(MODE) directory.
SM_SRC := $(ROOT_SRC)/spidermonkey/$(MODE)

# The objects we link in from spidermonkey
SM_OBJ := $(wildcard $(SM_SRC)/lib/*.o)
SM_OBJ += $(wildcard $(SM_SRC)/lib/*.a)

# This is required when using spidermonkey headers, as it allows us to enable
# the streams library when setting up the js context.
DEFINES := -DMOZ_JS_STREAMS

# Flags for c++ compilation
CXX_FLAGS := -std=gnu++20 -Wall -Werror -Qunused-arguments
CXX_FLAGS += -fno-sized-deallocation -fno-aligned-new -mthread-model single
CXX_FLAGS += -fPIC -fno-rtti -fno-exceptions -fno-math-errno -pipe
CXX_FLAGS += -fno-omit-frame-pointer -funwind-tables
CXX_FLAGS += --sysroot=$(WASI_SDK)/share/wasi-sysroot

# Flags for C compilation
CFLAGS := -Wall -Werror -Wno-unknown-attributes -Wno-pointer-to-int-cast
CFLAGS += -Wno-int-to-pointer-cast
CFLAGS += --sysroot=$(WASI_SDK)/share/wasi-sysroot

# Includes for compiling c++
INCLUDES := -I$(FSM_SRC)
INCLUDES += -I$(SM_SRC)/include
INCLUDES += -I$(BUILD)/openssl/include

# Linker flags.
LD_FLAGS := -Wl,-z,stack-size=1048576 -Wl,--stack-first
LD_FLAGS += -lwasi-emulated-signal
LD_FLAGS += -lwasi-emulated-process-clocks
LD_FLAGS += -lwasi-emulated-getpid
LD_FLAGS += -L$(BUILD)/openssl/libx32 -lcrypto


# Default targets ##############################################################

.PHONY: all
all: $(FSM_SRC)/js-compute-runtime.wasm
all: $(FSM_SRC)/js-compute-runtime-component.wasm
all: wasi_snapshot_preview1.wasm

# Remove just the build artifacts for the current runtime build.
.PHONY: clean
clean:
	$(call cmd,rm,$(FSM_SRC)/js-compute-runtime.wasm)
	$(call cmd,rm,$(FSM_SRC)/js-compute-runtime-component.wasm)
	$(call cmd,rmdir,$(BUILD)/release)
	$(call cmd,rmdir,$(BUILD)/debug)

# Remove all build artifacts.
.PHONY: distclean
distclean: clean
	$(call cmd,rmdir,$(BUILD))

# Run clang-format over the codebase.
.PHONY: format
format: $(FSM_CPP)
	$(ROOT_SRC)/../ci/clang-format.sh --fix


# Build directories ############################################################

$(BUILD):
	$(call cmd,mkdir,$@)

$(OBJ_DIR):
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/c-at-e-world:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/builtins:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/builtins/shared:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/core:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/host_interface:
	$(call cmd,mkdir,$@)

shared:
	$(call cmd,mkdir,$@)

# Downloaded dependencies ######################################################

$(BUILD)/openssl-$(OPENSSL_VERSION).tar.gz: URL=https://www.openssl.org/source/openssl-$(OPENSSL_VERSION).tar.gz
$(BUILD)/openssl-$(OPENSSL_VERSION).tar.gz: | $(BUILD)
	$(call cmd,wget,$@)

wasi_snapshot_preview1.wasm: URL=https://github.com/bytecodealliance/preview2-prototyping/releases/download/latest/wasi_snapshot_preview1.wasm
wasi_snapshot_preview1.wasm:
	$(call cmd,wget,$@)

# OpenSSL build ################################################################

# Convenience target for building openssl.
.PHONY: openssl
openssl: $(BUILD)/openssl/token

# Extract and prepare the openssl build directory.
$(BUILD)/openssl-$(OPENSSL_VERSION)/token: $(BUILD)/openssl-$(OPENSSL_VERSION).tar.gz $(FSM_SRC)/third_party/getuid.patch
	$Q tar -C $(BUILD) -xf $<
	$Q patch -d $(BUILD)/openssl-$(OPENSSL_VERSION) -p1 < $(FSM_SRC)/third_party/getuid.patch
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
OPENSSL_OPTS += --prefix=$(BUILD)/openssl
OPENSSL_OPTS += --cross-compile-prefix=$(WASI_SDK)/bin/
OPENSSL_OPTS += linux-x32

OPENSSL_DISABLED_WARNINGS := -Wno-unused-command-line-argument
OPENSSL_DISABLED_WARNINGS += -Wno-constant-conversion
OPENSSL_DISABLED_WARNINGS += -Wno-shift-count-overflow

# Configure and build openssl.
$(BUILD)/openssl/token: $(BUILD)/openssl-$(OPENSSL_VERSION)/token
	$Q export WASI_SDK_PATH=$(WASI_SDK) && \
		cd $(BUILD)/openssl-$(OPENSSL_VERSION) && \
		CC=clang \
		CFLAGS="--sysroot=$(WASI_SDK)/share/wasi-sysroot" \
		./Configure $(OPENSSL_OPTS) && \
		$(MAKE) -j8 && \
		$(MAKE) install_sw
	$Q touch $@


# rusturl build ################################################################

RUST_URL_SRC := $(FSM_SRC)/rust-url

RUST_URL_RS_FILES := $(shell find $(RUST_URL_SRC)/src -name '*.rs')

RUST_URL_LIB := $(BUILD)/rusturl/wasm32-wasi/$(MODE)/librust_url.a

rusturl: $(RUST_URL_LIB)

$(RUST_URL_LIB): $(RUST_URL_RS_FILES)
$(RUST_URL_LIB): $(RUST_URL_SRC)/Cargo.toml
$(RUST_URL_LIB): $(RUST_URL_SRC)/cbindgen.toml
$(RUST_URL_LIB): | $(BUILD)
	$(call cmd_format,CARGO,$@) \
	cd $(RUST_URL_SRC) && cbindgen --output rust-url.h && \
	cargo build $(call quiet_flag,--quiet) \
		--manifest-path $(RUST_URL_SRC)/Cargo.toml \
		--target-dir $(BUILD)/rusturl \
		--target=wasm32-wasi $(CARGO_FLAG)


# wit-bindgen integration ######################################################

.PHONY: regenerate-world
ifeq ($(WIT_BINDGEN),)
regenerate-world:
	@echo ""
	@echo "No wit-bindgen found in PATH, consider running"
	@echo ""
	@echo "  cargo install --git https://github.com/bytecodealliance/wit-bindgen wit-bindgen-cli"
	@echo ""
	@exit 1
else
regenerate-world:
	$(WIT_BINDGEN) c c-at-e.wit --no-helpers --out-dir c-at-e-world
endif


# Compute runtime shared build #################################################

FSM_CPP := $(wildcard $(FSM_SRC)/*.cpp)
FSM_CPP += $(wildcard $(FSM_SRC)/builtins/*.cpp)
FSM_CPP += $(wildcard $(FSM_SRC)/builtins/shared/*.cpp)
FSM_CPP += $(wildcard $(FSM_SRC)/core/*.cpp)
FSM_CPP += $(wildcard $(FSM_SRC)/host_interface/*.cpp)
FSM_OBJ := $(patsubst $(FSM_SRC)/%.cpp,$(OBJ_DIR)/%.o,$(FSM_CPP))

# Build all the above object files
$(foreach source,$(FSM_CPP),$(eval $(call compile_cxx,$(source))))

# Compute runtime build ########################################################

$(eval $(call compile_cxx,$(FSM_SRC)/c-at-e-world/c_at_e_world_adapter.cpp))

# This version of the runtime uses the world adapter in place of the world
# generated by wit-bindgen, adapting to the currently available host calls on
# c@e.
#
# NOTE: we shadow wasm-opt by adding $(FSM_SRC)/scripts to the path, which
# includes a script called wasm-opt that immediately exits successfully. See
# that script for more information about why we do this.
$(OBJ_DIR)/js-compute-runtime.wasm: $(FSM_OBJ) $(SM_OBJ) $(RUST_URL_LIB)
$(OBJ_DIR)/js-compute-runtime.wasm: $(OBJ_DIR)/c-at-e-world/c_at_e_world_adapter.o
	$(call cmd_format,WASI_LD,$@) PATH="$(FSM_SRC)/scripts:$$PATH" \
	$(WASI_CXX) $(LD_FLAGS) $(OPENSSL_LIBS) -o $@ $^
	$(call cmd_format,WASM_STRIP,$@) $(call WASM_STRIP,$@)


# Compute runtime component build ##############################################

$(eval $(call compile_c,$(FSM_SRC)/c-at-e-world/c_at_e_world.c))

# This version of the runtime uses the code generated by wit-bindgen, to
# ultimately be run as a component on c@e.
#
# NOTE: we shadow wasm-opt by adding $(FSM_SRC)/scripts to the path, which
# includes a script called wasm-opt that immediately exits successfully. See
# that script for more information about why we do this.
$(OBJ_DIR)/js-compute-runtime-component.wasm: $(FSM_OBJ) $(SM_OBJ) $(RUST_URL_LIB)
$(OBJ_DIR)/js-compute-runtime-component.wasm: $(OBJ_DIR)/c-at-e-world/c_at_e_world.o
	$(call cmd_format,WASI_LD,$@) PATH="$(FSM_SRC)/scripts:$$PATH" \
	$(WASI_CXX) $(LD_FLAGS) $(OPENSSL_LIBS) -o $@ $^
	$(call cmd_format,WASM_STRIP,$@) $(call WASM_STRIP,$@)


# Shared builtins build ########################################################

shared-builtins: shared/builtins.a shared/librust_url.a

shared/builtins.a: $(OBJ_DIR)/builtins/shared/*.o $(OBJ_DIR)/builtin.o | shared
	$(call cmd,wasi_ar,$^)

shared/librust_url.a: $(RUST_URL_LIB) | shared
	$(call cmd,cp,$@)

# These two rules copy the built artifacts into the $(FSM_SRC) directory, and
# are both marked phony as we need to do the right thing when running the
# following sequence:
#
# make; DEBUG=1 make; make
#
# Without marking them phony, the wasm won't be copied in the last invocation of
# make, as it will look up-to-date.

.PHONY: $(FSM_SRC)/js-compute-runtime.wasm
$(FSM_SRC)/js-compute-runtime.wasm: $(OBJ_DIR)/js-compute-runtime.wasm
	$(call cmd,cp,$@)

.PHONY: $(FSM_SRC)/js-compute-runtime-component.wasm
$(FSM_SRC)/js-compute-runtime-component.wasm: $(OBJ_DIR)/js-compute-runtime-component.wasm
	$(call cmd,cp,$@)


# Debugging rules ##############################################################

# Useful for debugging, try `make print-FSM_CPP`
print-%:
	$Q echo "$* = '$($*)'"


# Development rules ############################################################

# Generate a compile_commands.json for powering clangd.
.PHONY: compile_commands.json
compile_commands.json:
	$Q ( \
		sep="["; \
		for file in $(FSM_CPP); do \
			echo "$$sep"; \
			sep=","; \
			echo "{ \"directory\": \"$(FSM_SRC)\","; \
			echo "  \"command\": \"$(WASI_CXX) $(CXX_FLAGS) $(INCLUDES) $(DEFINES)\","; \
			echo -n "  \"file\": \"$${file#$(FSM_SRC)}\"}"; \
		done; \
		echo; \
		echo ']' \
	) > "$@"

