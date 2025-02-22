NAME = Chess
ICON = icon.png
DESCRIPTION = "Chess for the CE!"
COMPRESSED = YES

OBJDIR = obj
BINDIR = bin

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

TEST_INCLUDES = -Itests -Itests/framework
TEST_SOURCES = $(wildcard tests/framework/*.c) \
               tests/test_main.c

.PHONY: all tests

all: $(BINDIR)/$(NAME).8xp

tests: 
	@echo "Building test version..."
	$(MAKE) NAME=ChessTst \
			DESCRIPTION="BytCHESS Tests" \
			COMPRESSED=YES \
			LTO=NO \
			DEBUGMODE=DEBUG \
			SRCDIR=tests \
			CFLAGS="$(CFLAGS) $(TEST_INCLUDES)" \
			EXTRA_C_SOURCES="$(TEST_SOURCES)"

# External Configuration
include $(shell cedev-config --makefile)