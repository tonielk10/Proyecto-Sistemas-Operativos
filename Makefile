# Makefile del simulador de kernel (C++17, pthreads).
#
# Uso: make [target] [CXX=clang++]
# Cada variante compila en su propio directorio: build/<variante>/
#   release (por defecto) -O2 | debug -g -O0 | tsan ThreadSanitizer | asan Address+UBSan
# Los targets de variante se relanzan a si mismos con VARIANT=<v>.

SHELL := /bin/bash
CXX ?= g++
BASE_FLAGS := -std=c++17 -Wall -Wextra -pthread -I. -Iinclude

VARIANT ?= release
ifeq ($(VARIANT),release)
  VFLAGS := -O2
else ifeq ($(VARIANT),debug)
  VFLAGS := -g -O0
else ifeq ($(VARIANT),tsan)
  VFLAGS := -g -O1 -fsanitize=thread
else ifeq ($(VARIANT),asan)
  VFLAGS := -g -O1 -fsanitize=address,undefined
else
  $(error VARIANT desconocida: $(VARIANT))
endif

CXXFLAGS += $(BASE_FLAGS) $(VFLAGS)
LDFLAGS  += $(VFLAGS)

BUILD     := build/$(VARIANT)
SRCS      := $(wildcard src/*.cpp)
OBJS      := $(SRCS:src/%.cpp=$(BUILD)/%.o)
LIB_OBJS  := $(filter-out $(BUILD)/main.o,$(OBJS))
TESTS     := $(sort $(wildcard tests/test_*.cpp))
TEST_BINS := $(TESTS:tests/%.cpp=$(BUILD)/tests/%)

.PHONY: all release debug tsan asan test test-tsan run clean help \
        _bin _tests-run

all: release ## Compila la variante release (por defecto)

release: ## Compila build/release/simulador (-O2)
	@$(MAKE) --no-print-directory VARIANT=release _bin
debug: ## Compila build/debug/simulador (-g -O0)
	@$(MAKE) --no-print-directory VARIANT=debug _bin
tsan: ## Compila build/tsan/simulador con ThreadSanitizer
	@$(MAKE) --no-print-directory VARIANT=tsan _bin
asan: ## Compila build/asan/simulador con Address+UndefinedBehaviorSanitizer
	@$(MAKE) --no-print-directory VARIANT=asan _bin
test: ## Compila y ejecuta todas las pruebas (release)
	@$(MAKE) --no-print-directory VARIANT=release _tests-run
test-tsan: ## Igual que test, pero con ThreadSanitizer
	@$(MAKE) --no-print-directory VARIANT=tsan _tests-run
run: release ## Ejecuta build/release/simulador
	@build/release/simulador
clean: ## Borra build/
	rm -rf build
help: ## Muestra esta ayuda
	@grep -hE '^[a-z-]+:.*## ' $(MAKEFILE_LIST) | sed -E 's/^([a-z-]+):.*## (.*)/  \1\t\2/'

# --- Reglas internas (se ejecutan con VARIANT definido) ---
_bin: $(BUILD)/simulador

$(BUILD)/simulador: $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/tests/%: tests/%.cpp $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(LIB_OBJS) $(LDFLAGS) -o $@

# Ejecuta todas las pruebas sin detenerse en el primer fallo.
_tests-run: $(TEST_BINS)
	@ok=0; fallo=0; \
	for t in $(TEST_BINS); do \
	  n=$$(basename $$t); \
	  if timeout 120 $$t; then echo "OK $$n"; ok=$$((ok+1)); \
	  else echo "FALLO $$n"; fallo=$$((fallo+1)); fi; \
	done; \
	if [ -f tests/prueba_consola.sh ]; then \
	  if timeout 120 bash tests/prueba_consola.sh; then echo "OK prueba_consola.sh"; ok=$$((ok+1)); \
	  else echo "FALLO prueba_consola.sh"; fallo=$$((fallo+1)); fi; \
	fi; \
	echo "$$ok pruebas OK, $$fallo fallidas"; \
	[ $$fallo -eq 0 ]

-include $(OBJS:.o=.d) $(TEST_BINS:=.d)
