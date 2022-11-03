# Usage:
#   1. `make`           executable              save to bin/
#   2. `make debug`     debug (add -g)          save to debug/
#   3. `make dll`       make dll in windows     save to bin/
#   4. `make clean`     clean all binary        /

# flag
CXXFLAGS := $(CXXFLAGS)
COBJFLAGS := $(CXXFLAGS) -c

# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug

# compile macros
TARGET_NAME := y86_sim
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DBG := $(DBG_PATH)/$(TARGET_NAME)
ifeq ($(OS),Windows_NT)
    TARGET := $(addsuffix .exe,$(TARGET))
    TARGET_DBG := $(addsuffix .exe,$(TARGET_DBG))
endif
TARGET_DLL := $(addsuffix .dll,$(BIN_PATH)/$(TARGET_NAME))

# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.cpp)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
CLEAN_LIST := $(TARGET) $(TARGET_DBG) $(TARGET_DLL) $(OBJ)

# default rule
default: makedir all

# non-phony targets
$(TARGET): $(OBJ)
    $(CXX) $(OBJ) $(CXXFLAGS) -o $@

$(TARGET_DBG): $(OBJ)
    $(CXX) $(OBJ) $(CXXFLAGS) -g -o $@

$(TARGET_DLL): $(OBJ)
    $(CXX) $(OBJ) $(CXXFLAGS) -shared -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
    $(CXX) $(COBJFLAGS) -o $@ $<

# phony rules
.PHONY: makedir
makedir:
    @mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET_DBG)

.PHONY: dll
dll: $(TARGET_DLL)
    @if [ "$(OS)" != "Windows_NT" ]; then \
        echo "Error: Variables not set correctly"; exit 2; \
    else true; fi

.PHONY: clean
clean:
    @echo CLEAN $(CLEAN_LIST)
    @rm -f $(CLEAN_LIST)