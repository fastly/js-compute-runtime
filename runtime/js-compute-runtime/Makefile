
# Build constants ##############################################################


# The path to the directory containing this Makefile, the
# /runtime/js-compute-runtime directory.
FSM_SRC := $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")

# The path to the /runtime directory.
ROOT_SRC := $(shell dirname "$(FSM_SRC)")

# The name of the project
PROJECT_NAME := $(shell npm info "$(ROOT_SRC)/.." --json name)
PROJECT_VERSION := $(shell npm info "$(ROOT_SRC)/.." --json version)

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

# The path to the wasm-tools executable
WASM_TOOLS ?= $(shell which wasm-tools)

ifeq ($(WASM_TOOLS),)
$(error ERROR: "No wasm-tools found in PATH, consider running 'cargo install wasm-tools'")
else
WASM_METADATA = $(WASM_TOOLS) metadata add --sdk $(PROJECT_NAME)=$(PROJECT_VERSION) --output $1 $1
endif

# The base build directory, where all our build artifacts go.
BUILD := $(FSM_SRC)/build

# The output directory for the current build mode (release/debug).
OBJ_DIR := $(BUILD)/$(MODE)

MOZ_OBJDIR = $(BUILD)/obj-gecko-dev-$(MODE)
MOZ_OUTDIR = $(BUILD)/gecko-dev-$(MODE)

# The objects we link in from spidermonkey
GECKO_OBJ := "$(MOZ_OUTDIR)/lib/Unified_cpp_memory_build0.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/Unified_cpp_memory_mozalloc0.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/AutoProfilerLabel.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/ConditionVariable_noop.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/MmapFaultHandler.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/Mutex_noop.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/Printf.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/StackWalk.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/TimeStamp.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/TimeStamp_posix.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/Uptime.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/Decimal.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/lz4.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/lz4frame.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/lz4hc.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/xxhash.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/Unified_cpp_mfbt0.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/Unified_cpp_mfbt1.o"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/libjs_static.a"
GECKO_OBJ += "$(MOZ_OUTDIR)/lib/libjsrust.a"

# This is required when using gecko-dev headers, as it allows us to enable
# the streams library when setting up the js context.
DEFINES := -DMOZ_JS_STREAMS

# Flags for c++ compilation
CXX_FLAGS := -std=gnu++20 -Wall -Werror -Qunused-arguments
CXX_FLAGS += -fno-sized-deallocation -fno-aligned-new -mthread-model single
CXX_FLAGS += -fPIC -fno-rtti -fno-exceptions -fno-math-errno -pipe
CXX_FLAGS += -fno-omit-frame-pointer -funwind-tables -m32
CXX_FLAGS += --sysroot=$(WASI_SDK)/share/wasi-sysroot

# Flags for C compilation
CFLAGS := -Wall -Werror -Wno-unknown-attributes -Wno-pointer-to-int-cast
CFLAGS += -Wno-int-to-pointer-cast -m32
CFLAGS += --sysroot=$(WASI_SDK)/share/wasi-sysroot

# Includes for compiling c++
INCLUDES := -I$(FSM_SRC)
INCLUDES += -I$(MOZ_OUTDIR)/include
INCLUDES += -I$(BUILD)/openssl/include

# Linker flags.
LD_FLAGS := -Wl,-z,stack-size=1048576 -Wl,--stack-first
LD_FLAGS += -lwasi-emulated-signal
LD_FLAGS += -lwasi-emulated-process-clocks
LD_FLAGS += -lwasi-emulated-getpid
LD_FLAGS += -L$(BUILD)/openssl/libx32 -lcrypto

# JS Compute Runtime cpp files
FSM_CPP := $(wildcard $(FSM_SRC)/*.cpp)
FSM_CPP += $(wildcard $(FSM_SRC)/builtins/*.cpp)
FSM_CPP += $(wildcard $(FSM_SRC)/builtins/shared/*.cpp)
FSM_CPP += $(wildcard $(FSM_SRC)/core/*.cpp)
FSM_CPP += $(wildcard $(FSM_SRC)/host_interface/*.cpp)


# Default targets ##############################################################

.PHONY: all
all: $(FSM_SRC)/js-compute-runtime.wasm
all: $(FSM_SRC)/js-compute-runtime-component.wasm

# Remove just the build artifacts for the current runtime build.
.PHONY: clean
clean:
	$(call cmd,rm,$(FSM_SRC)/js-compute-runtime.wasm)
	$(call cmd,rm,$(FSM_SRC)/js-compute-runtime-component.wasm)
	$(call cmd,rmdir,$(BUILD)/release)
	$(call cmd,rmdir,$(BUILD)/debug)
	$(call cmd,rmdir,$(BUILD)/gecko-dev-release)
	$(call cmd,rmdir,$(BUILD)/gecko-dev-debug)
	$(call cmd,rmdir,$(BUILD)/obj-gecko-dev-release)
	$(call cmd,rmdir,$(BUILD)/obj-gecko-dev-debug)

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

$(OBJ_DIR): $(BUILD)
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/impl:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/builtins:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/builtins/shared:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/core:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/host_interface:
	$(call cmd,mkdir,$@)

$(OBJ_DIR)/host_interface/component:
	$(call cmd,mkdir,$@)

$(BUILD)/gecko-dev-$(MODE):
	$(call cmd,mkdir,$@)

shared:
	$(call cmd,mkdir,$@)

# Downloaded dependencies ######################################################

$(BUILD)/openssl-$(OPENSSL_VERSION).tar.gz: URL=https://www.openssl.org/source/openssl-$(OPENSSL_VERSION).tar.gz
$(BUILD)/openssl-$(OPENSSL_VERSION).tar.gz: | $(BUILD)
	$(call cmd,wget,$@)

# OpenSSL build ################################################################

# Convenience target for building openssl.
.PHONY: openssl
openssl: $(BUILD)/openssl/token

# Extract and prepare the openssl build directory.
$(BUILD)/openssl-$(OPENSSL_VERSION)/token: $(BUILD)/openssl-$(OPENSSL_VERSION).tar.gz $(FSM_SRC)/third_party/getuid.patch $(FSM_SRC)/third_party/rand.patch
	$Q tar -C $(BUILD) -xf $<
	$Q patch -d $(BUILD)/openssl-$(OPENSSL_VERSION) -p1 < $(FSM_SRC)/third_party/getuid.patch
	$Q patch -d $(BUILD)/openssl-$(OPENSSL_VERSION) -p1 < $(FSM_SRC)/third_party/rand.patch
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

# gecko-dev build ##############################################################

$(FSM_SRC)/mozconfig-$(MODE):
	echo "ac_add_options --enable-project=js" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --enable-application=js" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --target=wasm32-unknown-wasi" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --without-system-zlib" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --without-intl-api" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --disable-jit" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --disable-shared-js" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --disable-shared-memory" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --disable-tests" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --disable-clang-plugin" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --enable-jitspew" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --enable-optimize=-O3" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --enable-js-streams" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --enable-portable-baseline-interp" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	echo "ac_add_options --prefix=$(MOZ_OBJDIR)/dist" >> "$(FSM_SRC)/mozconfig-$(MODE)"
	if [ "$(MODE)" == "debug" ]; then \
		echo "ac_add_options --enable-debug" >> "$(FSM_SRC)/mozconfig-$(MODE)"; \
	else \
		echo "ac_add_options --disable-debug" >> "$(FSM_SRC)/mozconfig-$(MODE)"; \
	fi
	if [ "$(uname)" == "Linux" ]; then \
		echo "ac_add_options --disable-stdcxx-compat" >> "$(FSM_SRC)/mozconfig-$(MODE)"; \
	else \
		echo "ac_add_options --host=aarch64-apple-darwin" >> "$(FSM_SRC)/mozconfig-$(MODE)"; \
	fi

gecko: $(FSM_SRC)/mozconfig-$(MODE)
	export MOZCONFIG=$(FSM_SRC)/mozconfig-$(MODE) && \
		MOZ_OBJDIR=$(MOZ_OBJDIR) \
		AUTOCLOBBER=1 \
		MOZ_FETCHES_DIR=~/.mozbuild \
		CC=~/.mozbuild/clang/bin/clang \
		CXX=~/.mozbuild/clang/bin/clang++ \
		AR=~/.mozbuild/clang/bin/llvm-ar \
		python3 "${FSM_SRC}/gecko-dev/mach" \
		--no-interactive \
		build
	rm -rf "$(MOZ_OUTDIR)"
	mkdir -p "$(MOZ_OUTDIR)/lib"
	cp -Lr "$(MOZ_OBJDIR)/dist/include" "$(MOZ_OUTDIR)"
	cp $(MOZ_OBJDIR)/memory/build/Unified_cpp_memory_build0.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/memory/mozalloc/Unified_cpp_memory_mozalloc0.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/AutoProfilerLabel.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/ConditionVariable_noop.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/MmapFaultHandler.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/Mutex_noop.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/Printf.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/StackWalk.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/TimeStamp.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/TimeStamp_posix.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/Uptime.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mozglue/misc/Decimal.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mfbt/lz4.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mfbt/lz4frame.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mfbt/lz4hc.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mfbt/xxhash.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mfbt/Unified_cpp_mfbt0.o "$(MOZ_OUTDIR)/lib"
	cp $(MOZ_OBJDIR)/mfbt/Unified_cpp_mfbt1.o "$(MOZ_OUTDIR)/lib"
	cp "$(MOZ_OBJDIR)/js/src/build/libjs_static.a" "$(MOZ_OUTDIR)/lib"
	cp "$(MOZ_OBJDIR)/wasm32-wasi/${MODE}/libjsrust.a" "$(MOZ_OUTDIR)/lib"

$(GECKO_OBJ): gecko

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

# rustencoding build ################################################################

RUST_ENCODING_SRC := $(FSM_SRC)/rust-encoding

RUST_ENCODING_RS_FILES := $(shell find $(RUST_ENCODING_SRC)/src -name '*.rs')

RUST_ENCODING_LIB := $(BUILD)/rustencoding/wasm32-wasi/$(MODE)/librust_encoding.a

rustencoding: $(RUST_ENCODING_LIB)

$(RUST_ENCODING_LIB): $(RUST_ENCODING_RS_FILES)
$(RUST_ENCODING_LIB): $(RUST_ENCODING_SRC)/Cargo.toml
$(RUST_ENCODING_LIB): $(RUST_ENCODING_SRC)/cbindgen.toml
$(RUST_ENCODING_LIB): | $(BUILD)
	$(call cmd_format,CARGO,$@) \
	cd $(RUST_ENCODING_SRC) && cbindgen --output rust-encoding.h && \
	cargo build $(call quiet_flag,--quiet) \
		--manifest-path $(RUST_ENCODING_SRC)/Cargo.toml \
		--target-dir $(BUILD)/rustencoding \
		--target=wasm32-wasi $(CARGO_FLAG)


# wit-bindgen integration ######################################################

.PHONY: regenerate-world
ifeq ($(WIT_BINDGEN),)
regenerate-world:
	@echo ""
	@echo "No wit-bindgen found in PATH, consider running"
	@echo ""
	@echo "  cargo install --git https://github.com/bytecodealliance/wit-bindgen wit-bindgen-cli --no-default-features --features c"
	@echo ""
	@exit 1
else
regenerate-world:
	$(WIT_BINDGEN) c --no-helpers \
	  --out-dir host_interface/component \
	  --world fastly-world \
	  host_interface/wit
endif


# Compute runtime shared build #################################################

FSM_OBJ := $(patsubst $(FSM_SRC)/%.cpp,$(OBJ_DIR)/%.o,$(FSM_CPP))

# Build all the above object files
$(foreach source,$(FSM_CPP),$(eval $(call compile_cxx,$(source))))

# Compute runtime build ########################################################

$(eval $(call compile_cxx,$(FSM_SRC)/host_interface/component/fastly_world_adapter.cpp))
$(eval $(call compile_cxx,$(FSM_SRC)/impl/main.cpp))

$(OBJ_DIR)/host_interface/component/fastly_world_adapter.o: $(GECKO_OBJ)
$(OBJ_DIR)/impl/main.o: $(GECKO_OBJ)
$(FSM_OBJ): $(GECKO_OBJ)

# This version of the runtime uses the world adapter in place of the world
# generated by wit-bindgen, adapting to the currently available host calls on
# c@e.
#
# NOTE: we shadow wasm-opt by adding $(FSM_SRC)/scripts to the path, which
# includes a script called wasm-opt that immediately exits successfully. See
# that script for more information about why we do this.
$(OBJ_DIR)/js-compute-runtime.wasm: $(FSM_OBJ) $(RUST_URL_LIB) $(RUST_ENCODING_LIB)
$(OBJ_DIR)/js-compute-runtime.wasm: $(OBJ_DIR)/impl/main.o
$(OBJ_DIR)/js-compute-runtime.wasm: $(OBJ_DIR)/host_interface/component/fastly_world_adapter.o
	$(call cmd_format,WASI_LD,$@) PATH="$(FSM_SRC)/scripts:$$PATH" \
	$(WASI_CXX) $(LD_FLAGS) $(OPENSSL_LIBS) -o $@ $^
	$(call cmd_format,WASM_STRIP,$@) $(call WASM_STRIP,$@)
	$(call cmd_format,WASM_METADATA,$@) $(call WASM_METADATA,$@)

# Compute runtime component build ##############################################

$(eval $(call compile_c,$(FSM_SRC)/host_interface/component/fastly_world.c))
$(eval $(call compile_cxx,$(FSM_SRC)/impl/main_component.cpp))

$(OBJ_DIR)/impl/main_component.o: $(GECKO_OBJ)
$(FSM_SRC)/host_interface/component/fastly_world_component_type.o: $(GECKO_OBJ)
$(OBJ_DIR)/host_interface/component/fastly_world.o: $(GECKO_OBJ)

# This version of the runtime uses the code generated by wit-bindgen, to
# ultimately be run as a component on c@e.
#
# NOTE: we shadow wasm-opt by adding $(FSM_SRC)/scripts to the path, which
# includes a script called wasm-opt that immediately exits successfully. See
# that script for more information about why we do this.
$(OBJ_DIR)/js-compute-runtime-component.wasm: $(FSM_OBJ) $(RUST_URL_LIB) $(RUST_ENCODING_LIB)
$(OBJ_DIR)/js-compute-runtime-component.wasm: $(OBJ_DIR)/impl/main_component.o
$(OBJ_DIR)/js-compute-runtime-component.wasm: $(FSM_SRC)/host_interface/component/fastly_world_component_type.o
$(OBJ_DIR)/js-compute-runtime-component.wasm: $(OBJ_DIR)/host_interface/component/fastly_world.o
	$(call cmd_format,WASI_LD,$@) PATH="$(FSM_SRC)/scripts:$$PATH" \
	$(WASI_CXX) $(LD_FLAGS) $(OPENSSL_LIBS) -o $@ $^
	$(call cmd_format,WASM_STRIP,$@) $(call WASM_STRIP,$@)
	$(call cmd_format,WASM_METADATA,$@) $(call WASM_METADATA,$@)

# Shared builtins build ########################################################

shared-builtins: shared/builtins.a shared/librust_url.a shared/librust_encoding.a

shared/builtins.a: $(OBJ_DIR)/builtins/shared/*.o
shared/builtins.a: $(OBJ_DIR)/builtin.o
shared/builtins.a: $(OBJ_DIR)/core/encode.o
shared/builtins.a: | shared
	$(call cmd,wasi_ar,$^)

shared/librust_url.a: $(RUST_URL_LIB) | shared
	$(call cmd,cp,$@)

shared/librust_encoding.a: $(RUST_ENCODING_LIB) | shared
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
			echo -n "  \"file\": \"$${file}\"}"; \
		done; \
		echo; \
		echo ']' \
	) > "$@"

