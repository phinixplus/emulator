SHELL=/usr/bin/env bash

# ---------------------------------------------------------------------------- #

### Layout Configuration ###
BUILD_DIR		= build
C_SOURCE_DIR	= source
C_INCLUDE_DIR	= include
ASM_SOURCE_DIR	= test

PROGRAM_NAME	= $(BUILD_DIR)/pplusemu
C_BUILD_DIR		= $(BUILD_DIR)/emu
ASM_BUILD_DIR	= $(BUILD_DIR)/asm

### Compiler Configuration ###
LIBS_EXISTING = pthread

C_COMPILER = gcc
override C_FLAGS_COM += -Wall -Wextra -pedantic
override C_FLAGS_DBG += ${C_FLAGS_COM} -g -std=c11 -D_POSIX_C_SOURCE=200809L
override C_FLAGS_REL += ${C_FLAGS_COM} -Werror -O2 -std=c11 -D_POSIX_C_SOURCE=200809L
override C_FLAGS_LNK += ${C_FLAGS_COM}

ASSEMBLER = customasm
override ASM_FLAGS += -q -flogisim8 archdef/archdef_emu.asm

# ---------------------------------------------------------------------------- #

#### Derived Variables ###
C_INCLUDES	= $(addprefix -I, $(C_INCLUDE_DIR))
C_SOURCES	= $(shell find $(C_SOURCE_DIR) -type f -name "*.c")
C_OBJECTS	= $(C_SOURCES:$(C_SOURCE_DIR)%.c=$(C_BUILD_DIR)%.o)
C_LIBRARIES	= $(addprefix -l, $(LIBS_EXISTING))

ASM_SOURCES	= $(shell find $(ASM_SOURCE_DIR) -type f -name "*.asm")
ASM_OBJECTS	= $(ASM_SOURCES:$(ASM_SOURCE_DIR)%.asm=$(ASM_BUILD_DIR)%.hex)

### User Rules ###
.PHONY: debug release assemble clean purge

debug: C_FLAGS = $(C_FLAGS_DBG)
debug: $(LIBS_TO_MAKE) $(PROGRAM_NAME)

release: C_FLAGS = $(C_FLAGS_REL)
release: $(LIBS_TO_MAKE) $(PROGRAM_NAME)

assemble: $(ASM_OBJECTS)

clean: # Remove build artifacts only (Keep executables and libraries)
	@rm -rf ./$(C_BUILD_DIR)

purge: # Return project to a state just after cloning
	@rm -rf ./$(BUILD_DIR)

### C Source Build Rule ###
$(C_OBJECTS): LOCAL_INCLUDE = $(patsubst $(C_BUILD_DIR)%,-I$(C_INCLUDE_DIR)%,$(shell dirname $@))
$(C_OBJECTS): $(C_BUILD_DIR)%.o: $(C_SOURCE_DIR)%.c
	@dirname $@ | xargs mkdir -p
	@echo -e '$(shell echo $(C_COMPILER) | tr [:lower:] [:upper:])\t$@'
	@$(C_COMPILER) $(C_FLAGS) -c $^ $(C_INCLUDES) $(LOCAL_INCLUDE) -o $@

$(PROGRAM_NAME): $(C_OBJECTS)
	@echo '-> $@'
	@$(C_COMPILER) $(C_FLAGS_LNK) $^ $(C_LIBRARIES) -o $@

### Assembly Build Rule ###
$(ASM_OBJECTS): $(ASM_BUILD_DIR)%.hex: $(ASM_SOURCE_DIR)%.asm
	@dirname $@ | xargs mkdir -p
	@$(ASSEMBLER) $(ASM_FLAGS) $^ -o $@
	@echo -e '$(shell echo $(ASSEMBLER) | tr [:lower:] [:upper:])\t$@'

# ---------------------------------------------------------------------------- #
