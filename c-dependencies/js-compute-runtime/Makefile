# DEFINES := -DDEBUG -DJS_DEBUG

INIT_JS ?= test.js

ifneq (, $(shell which wasm-opt))
WASM_STRIP = wasm-opt --strip-debug -o $@ $@
endif

ROOT_SRC?=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))/..
SM_SRC=$(ROOT_SRC)/spidermonkey/
SM_OBJ=$(SM_SRC)lib/*
FSM_SRC=$(ROOT_SRC)/js-compute-runtime/

WASI_CC ?= /opt/wasi-sdk/bin/clang
WASI_CXX ?= /opt/wasi-sdk/bin/clang++
WIZER ?= wizer

CXX_FLAGS := -std=gnu++17 -Wall -Werror

FSM_CPP := $(wildcard $(FSM_SRC)*.cpp)
FSM_DEP := $(patsubst $(FSM_SRC)%.cpp,$(OBJDIR)%.d,$(FSM_CPP))
FSM_OBJ := $(patsubst $(FSM_SRC)%.cpp,$(OBJDIR)%.o,$(FSM_CPP))

.PHONY: all clean

all: js-compute-runtime.wasm

-include $(FSM_DEP)

%.o: $(FSM_SRC)%.cpp $(FSM_SRC)Makefile
	$(WASI_CXX) $(CXX_FLAGS) $(DEFINES) -I $(SM_SRC)include -O3 -MMD -MP -c -o $@ $<

js-compute-runtime.wasm: $(FSM_OBJ) $(SM_OBJ)
	$(WASI_CXX) $(CXX_FLAGS) $(DEFINES) -fvisibility=hidden -Wl,--allow-undefined -Wl,--stack-first -Wl,-z,stack-size=1048576 -mexec-model=command -O3 -o $@ $^
	$(WASM_STRIP)

initialized-js-compute-runtime.wasm: js-compute-runtime.wasm
	cat $(INIT_JS) | $(WIZER) --allow-wasi -r _start=wizer.resume -o $@ $^
