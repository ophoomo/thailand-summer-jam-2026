CC := clang
CXX := clang++
BUILD_DIR := build
BUILD_TYPE ?= Debug
CMAKE_GENERATOR ?= Ninja
CMAKE_OPTIONS := -G $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_C_COMPILER=$(CC) -DCMAKE_CXX_COMPILER=$(CXX)

all: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR)

$(BUILD_DIR)/Makefile:
	@echo "Configuring project ($(BUILD_TYPE))..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_OPTIONS) ..

cmake-configure: $(BUILD_DIR)/Makefile

debug:
	$(MAKE) BUILD_TYPE=Debug all

release:
	$(MAKE) BUILD_TYPE=Release all

clean:
	@echo "Cleaning project..."
	@rm -rf $(BUILD_DIR)

distclean: clean
	@echo "Removing CMake cache..."
	@rm -f CMakeCache.txt

.PHONY: all debug release clean distclean cmake-configure
