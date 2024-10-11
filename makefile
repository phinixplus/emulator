# C Definitions
C_COMPILER		= gcc
C_FLAGS_DBG		= -Wall -Wextra -pedantic -std=c11 -D_POSIX_C_SOURCE=200809L -g -rdynamic
C_FLAGS_REL		= -Werror -Wall -Wextra -pedantic -std=c11 -D_POSIX_C_SOURCE=200809L -O2
C_LIBRARIES		= -lpthread

C_BUILD_LOC		?= build/emu/
C_SOURCE_LOC	?= source/
C_INCLUDE_LOC	?= include/
C_EXE_NAME		?= build/pplusemu

C_INCLUDES	= $(C_INCLUDE_LOC)
C_SOURCES	= $(shell find $(C_SOURCE_LOC) -type f -name "*.c")
C_OBJECTS	= $(C_SOURCES:$(C_SOURCE_LOC)%.c=$(C_BUILD_LOC)%.o)

# C Rules
.PHONY: debug release

debug: C_FLAGS = $(C_FLAGS_DBG)
debug: $(C_EXE_NAME)

release: C_FLAGS = $(C_FLAGS_REL)
release: $(C_EXE_NAME)

$(C_OBJECTS): LOCAL_INCLUDE = $(patsubst $(C_BUILD_LOC)%,-I$(C_INCLUDE_LOC)%,$(shell dirname $@)/)
$(C_OBJECTS): $(C_BUILD_LOC)%.o: $(C_SOURCE_LOC)%.c
	@dirname $@ | xargs mkdir -p
	@$(C_COMPILER) $(C_FLAGS) -c $^ $(C_INCLUDES:%=-I%) $(LOCAL_INCLUDE) -o $@
	@echo "$(shell echo $(C_COMPILER) | tr [:lower:] [:upper:])\t$@"

$(C_EXE_NAME): $(C_OBJECTS)
	@$(C_COMPILER) $(CFLAGS) $^ $(C_LIBRARIES) -o $@
	@echo "-> $@"

# Assembly Definitions
ASSEMBLER		= customasm
ASM_FLAGS		= -q -flogisim8 archdef/archdef_emu.asm

ASM_BUILD_LOC	?= build/asm/
ASM_SOURCE_LOC	?= test/

ASM_SOURCES	= $(shell find $(ASM_SOURCE_LOC) -maxdepth 1 -type f -name "*.asm")
ASM_OBJECTS	= $(ASM_SOURCES:$(ASM_SOURCE_LOC)%.asm=$(ASM_BUILD_LOC)%.hex)

# Assembly Rules
.PHONY: assemble

assemble: $(ASM_OBJECTS)

$(ASM_OBJECTS): $(ASM_BUILD_LOC)%.hex: $(ASM_SOURCE_LOC)%.asm
	@mkdir -p $(ASM_BUILD_LOC)
	@$(ASSEMBLER) $(ASM_FLAGS) $^ -o $@
	@echo "$(shell echo $(ASSEMBLER) | tr [:lower:] [:upper:])\t$@"

# Utility Rules
.PHONY: clean purge

clean:
	-@find $(C_BUILD_LOC) -type f -name "*.o" -delete
	-@find $(C_BUILD_LOC) -type d -empty -delete

purge:
	-@rm $(C_EXE_NAME)
	-@rm -r $(C_BUILD_LOC)
	-@rm -r $(ASM_BUILD_LOC)

	-@echo '$(C_BUILD_LOC)\n${ASM_BUILD_LOC}' |\
	cut -d'/' -f1 | xargs -I! find ! -type d |\
	xargs rmdir -p
