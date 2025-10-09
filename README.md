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

- `src/main.c` — Entry point with a minimal EFL window
- `Makefile` — Build rules mirroring the main app’s style
- `README.md` — This guide

## Build & Run

```bash
make            # build the template
make run        # build and run ./efl_hello
make clean      # remove build artifacts
make check-deps # verify Elementary is available
```

## Customizing for New Projects

- Change the program name by editing `PROGRAM` in `Makefile` (default: `efl_hello`).
- Add new source files under `src/` and update `SOURCES` accordingly, e.g. `SOURCES = src/main.c src/app.c`.
- Use the provided pattern rule to compile files in `src/` automatically.

## Code Overview

The template initializes Elementary, creates a standard window, adds a background, and centers a label.
It demonstrates core EFL concepts: window creation, object sizing, alignment, and the main event loop.

## License

You may reuse and modify this template freely for your projects. Consider preserving attribution to EFL.