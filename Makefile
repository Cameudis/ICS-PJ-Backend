# Usage:
#   1. `make`           executable              save to bin/
#   2. `make debug`     debug                   save to debug/
#   3. `make dll`       make dll in windows     save to bin/
#   4. `make clean`     clean all binary        /

# flag macros
CXXFLAGS := $(CXXFLAGS) -std=c++11
COBJFLAGS := $(CXXFLAGS) -c
NDBGFLAG := -DNDEBUG
DBGFLAGS := -g

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
OBJ_DLL := $(filter-out obj/main.o,$(OBJ))
OBJ := $(filter-out obj/api.o,$(OBJ))
OBJ_DEBUG := $(addprefix $(DBG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
CLEAN_LIST := $(TARGET) $(TARGET_DBG) $(TARGET_DLL) $(OBJ) $(OBJ_DEBUG)

# non-phony targets
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) $(NDBGFLAG) -o $@

$(TARGET_DBG): $(OBJ_DEBUG)
	$(CXX) $(OBJ_DEBUG) $(CXXFLAGS) $(DBGFLAGS) -o $@

$(TARGET_DLL): $(OBJ_DLL)
	$(CXX) $(OBJ_DLL) $(CXXFLAGS) $(NDBGFLAG) -shared -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(COBJFLAGS) $(NDBGFLAG) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(COBJFLAGS) $(DBGFLAGS) -o $@ $<

# default rule
default: makedir all


# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH)

.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: makedir $(TARGET_DBG)

.PHONY: dll
dll: makedir $(TARGET_DLL)
	@if [ "$(OS)" != "Windows_NT" ]; then \
		echo "Error: Please compile .dll in windows"; exit 2; \
	else true; fi

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)