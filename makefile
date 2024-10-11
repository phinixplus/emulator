CCOMPILER	= gcc
CFLAGS_DBG	= -Wall -Wextra -pedantic -std=c11 -D_POSIX_C_SOURCE=200809L -g -rdynamic
CFLAGS_REL	= -Werror -Wall -Wextra -pedantic -std=c11 -D_POSIX_C_SOURCE=200809L -O2
CLIBS		= -lpthread

BUILD_LOC	= build/
SOURCE_LOC	= source/
INCL_LOC	= include/
PROG_NAME	= pplusemu

INCLUDES = $(INCL_LOC)
SOURCES = $(shell find $(SOURCE_LOC) -type f -name "*.c")
OBJECTS = $(SOURCES:$(SOURCE_LOC)%.c=$(BUILD_LOC)%.o)

.PHONY: debug release

debug: CFLAGS = $(CFLAGS_DBG)
debug: $(BUILD_LOC)$(PROG_NAME)

release: CFLAGS = $(CFLAGS_REL)
release: $(BUILD_LOC)$(PROG_NAME)

$(OBJECTS): LOCAL_INCLUDE = $(patsubst $(BUILD_LOC)%,-I$(INCL_LOC)%,$(shell dirname $@)/)
$(OBJECTS): $(BUILD_LOC)%.o: $(SOURCE_LOC)%.c
	@dirname $@ | xargs mkdir -p
	@$(CCOMPILER) $(CFLAGS) -c -o $@ $^ $(INCLUDES:%=-I%) $(LOCAL_INCLUDE)
	@echo "$(shell echo $(CCOMPILER) | tr [:lower:] [:upper:])\t$@"

$(BUILD_LOC)$(PROG_NAME): $(OBJECTS)
	@$(CCOMPILER) $(CFLAGS) -o $@ $^ $(CLIBS)
	@echo "-> $@"
