# SSAAS · cross-platform Makefile
# Detects host OS and links the right socket libraries.
#
#   make            -> build/ssaas_server[.exe]
#   make run        -> build then run with sample data
#   make clean      -> remove build artefacts
#   make smoke      -> compile and run the headless smoke test

CXX      ?= g++
CXXSTD    = -std=c++14
WARN      = -Wall -Wextra -Wpedantic -Wno-unused-parameter
INC       = -Iinclude -Iinclude/third_party
OPT      ?= -O2
CXXFLAGS  = $(CXXSTD) $(WARN) $(INC) $(OPT)

# OS detection
ifeq ($(OS),Windows_NT)
    LDLIBS = -lws2_32 -lwsock32
    EXE    = .exe
else
    LDLIBS =
    EXE    =
endif

SRC = $(wildcard src/core/*.cpp) \
      $(wildcard src/analytics/*.cpp) \
      $(wildcard src/alerts/*.cpp) \
      $(wildcard src/storage/*.cpp) \
      $(wildcard src/server/*.cpp)

MAIN = main.cpp
OUT  = build/ssaas_server$(EXE)

SMOKE_SRC = test_core.cpp $(SRC)
SMOKE_OUT = build/smoke_test$(EXE)

all: $(OUT)

$(OUT): $(MAIN) $(SRC)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(MAIN) $(SRC) -o $(OUT) $(LDLIBS)

run: $(OUT)
	cd $(dir $(OUT)) && ./$(notdir $(OUT)) --data ../data/sample_data.json --static ../../frontend

smoke: $(SMOKE_OUT)
	./$(SMOKE_OUT)

$(SMOKE_OUT): $(SMOKE_SRC)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SMOKE_SRC) -o $(SMOKE_OUT) $(LDLIBS)

clean:
	rm -rf build

.PHONY: all run smoke clean
