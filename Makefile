# EFL Boilerplate Makefile

# Program name (change for your project)
PROGRAM = efl_hello

# Source files (add under src/)
SOURCES = src/main.c

# Object files (auto-mapped from sources)
OBJECTS = $(SOURCES:.c=.o)

# Compiler
CC = gcc

# Elementary compile/link flags via pkg-config (minimal deps)
CFLAGS = `pkg-config --cflags elementary`
LDFLAGS = `pkg-config --libs elementary`

# Cross-platform and language standard
CFLAGS += -std=c99
CFLAGS += -D_GNU_SOURCE -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L

# Warning levels (mirrors main project style)
WARN_BASIC = -Wall
WARN_MEDIUM = $(WARN_BASIC) -Wextra -Wformat=2 -Wstrict-prototypes
WARN_STRICT = $(WARN_MEDIUM) -Wpedantic -Wcast-align -Wcast-qual -Wconversion -Wsign-conversion
WARN_PEDANTIC = $(WARN_STRICT) -Wshadow -Wredundant-decls -Wunreachable-code -Wwrite-strings -Wpointer-arith

# Default warning level
CFLAGS += $(WARN_BASIC)

# Default target
all: $(PROGRAM)

# Link the program
$(PROGRAM): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROGRAM) $(LDFLAGS)

# Compile source files (handles src/ paths)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(PROGRAM)

# Installation directories
PREFIX ?= /usr/local
# Allow using either PREFIX or the conventional 'prefix' variable
prefix ?= $(PREFIX)
DESTDIR ?=

# XDG-compliant paths
BINDIR   := $(prefix)/bin
APPDIR   := $(prefix)/share/applications
ICONDIR  := $(prefix)/share/icons/hicolor/scalable/apps

# Desktop entry and icon sources
DESKTOP_FILE := data/efl-hello.desktop
ICON_FILE    := data/efl-hello.svg

# Install binary
install: $(PROGRAM)
	# Create target directories
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(APPDIR)
	install -d $(DESTDIR)$(ICONDIR)
	# Install binary
	install -m 0755 $(PROGRAM) $(DESTDIR)$(BINDIR)/$(PROGRAM)
	# Install desktop file and icon
	install -m 0644 $(DESKTOP_FILE) $(DESTDIR)$(APPDIR)/efl-hello.desktop
	install -m 0644 $(ICON_FILE) $(DESTDIR)$(ICONDIR)/efl-hello.svg

# Uninstall binary
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(PROGRAM)
	rm -f $(DESTDIR)$(APPDIR)/efl-hello.desktop
	rm -f $(DESTDIR)$(ICONDIR)/efl-hello.svg

# Run the program
run: $(PROGRAM)
	./$(PROGRAM)

# Warning level targets
warn-basic: clean
	@echo "Building with basic warnings (-Wall)..."
	$(MAKE) CFLAGS="$(shell echo '$(CFLAGS)' | sed 's/$(WARN_BASIC)/$(WARN_BASIC)/')"

warn-medium: clean
	@echo "Building with medium warnings (-Wall -Wextra -Wformat=2 -Wstrict-prototypes)..."
	$(MAKE) CFLAGS="$(shell echo '$(CFLAGS)' | sed 's/$(WARN_BASIC)/$(WARN_MEDIUM)/')"

warn-strict: clean
	@echo "Building with strict warnings (includes -Wpedantic -Wcast-align -Wcast-qual -Wconversion -Wsign-conversion)..."
	$(MAKE) CFLAGS="$(shell echo '$(CFLAGS)' | sed 's/$(WARN_BASIC)/$(WARN_STRICT)/')"

warn-pedantic: clean
	@echo "Building with pedantic warnings (includes -Wshadow -Wredundant-decls -Wunreachable-code -Wwrite-strings -Wpointer-arith)..."
	$(MAKE) CFLAGS="$(shell echo '$(CFLAGS)' | sed 's/$(WARN_BASIC)/$(WARN_PEDANTIC)/')"

# Check dependencies
check-deps:
	@echo "Checking EFL dependencies..."
	@pkg-config --exists elementary && echo "✓ Elementary (EFL) found" || echo "✗ Elementary (EFL) not found"
	@pkg-config --modversion elementary 2>/dev/null && echo "EFL Version: `pkg-config --modversion elementary`" || echo "Could not determine EFL version"

# Help target
help:
	@echo "Available targets:"
	@echo "  all            - Build the program (default)"
	@echo "  clean          - Remove build artifacts"
	@echo "  run            - Build and run the program"
	@echo "  check-deps     - Check if EFL dependencies are installed"
	@echo "  install        - Install binary, desktop file, and icon (prefix=$(prefix))"
	@echo "  uninstall      - Remove installed binary, desktop file, and icon"
	@echo "Warning level targets:"
	@echo "  warn-basic     - Build with basic warnings (-Wall)"
	@echo "  warn-medium    - Build with medium warnings"
	@echo "  warn-strict    - Build with strict warnings"
	@echo "  warn-pedantic  - Build with pedantic warnings"

# Declare phony targets
.PHONY: all clean install uninstall run check-deps help warn-basic warn-medium warn-strict warn-pedantic