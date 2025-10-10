# EFL Boilerplate Template

A minimal, well-structured starter template for building desktop applications with Enlightenment Foundation Libraries (EFL). It provides a clean "Hello, World!" example, a consistent Makefile, and a conventional project layout that you can copy and adapt for new projects.

## Features

- Simple EFL (Elementary) window with centered "Hello, World!" label
- Consistent Makefile with build, run, install, and warning-level targets
- Conventional layout (`src/`) for easy expansion
- Minimal dependencies (only `elementary` via `pkg-config`)

## Prerequisites

- `EFL / Elementary`
- `gcc` with C99 support
- `pkg-config`

## Project Structure

- `configure.ac` — Autoconf script for project configuration and dependency checks.
- `Makefile.am` — Top-level Automake file.
- `src/main.c` — Entry point with a minimal EFL window.
- `src/Makefile.am` — Automake file for the source directory.
- `README.md` — This guide.

## Build & Run

First, generate the build system:

```bash
autoreconf --install
```

Then, configure, build, and run the application:

```bash
./configure
make
./src/efl_hello
```

To clean the build artifacts:

```bash
make clean
```

## Customizing for New Projects

- **Project Name:** Edit `AC_INIT` in `configure.ac` to set your project's name, version, and contact email.
- **Program Name:** In `src/Makefile.am`, change `bin_PROGRAMS` and the associated `_SOURCES` variable if you rename the executable.
- **Source Files:** Add new `.c` files to the `efl_hello_SOURCES` variable in `src/Makefile.am`.

## Code Overview

The template initializes Elementary, creates a standard window, adds a background, and centers a label.
It demonstrates core EFL concepts: window creation, object sizing, alignment, and the main event loop.
