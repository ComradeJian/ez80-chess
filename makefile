NAME = Chess
ICON = icon.png
DESCRIPTION = "Chess for the CE!"
COMPRESSED = YES

OBJDIR = obj
BINDIR = bin

CFLAGS = -Wall -Wextra -Oz -Isrc
CXXFLAGS = -Wall -Wextra -Oz -Isrc

TEST_INCLUDES = -Itests/framework -Itests/unit
TEST_SOURCES = $(filter-out src/main.c,$(wildcard src/*.c))

.PHONY: all tests

all: $(BINDIR)/$(NAME).8xp

tests: 
	@echo "Building test version..."
	$(MAKE) NAME=ChessTst \
			DESCRIPTION="Chess Project Unit Tests" \
			COMPRESSED=YES \
			DEBUGMODE=DEBUG \
			SRCDIR=tests \
			CFLAGS="$(CFLAGS) $(TEST_INCLUDES)" \
			EXTRA_C_SOURCES="$(TEST_SOURCES)"

# External Configuration
include $(shell cedev-config --makefile)