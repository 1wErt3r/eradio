# eradio

A simple internet radio player built with the Enlightenment Foundation Libraries (EFL).

## Features

- Search for radio stations using the [radio-browser.info](http://radio-browser.info/) API
- List stations with their favicons
- Play, pause, and stop radio streams
- Search by station name, country, language, or tag

## Prerequisites

- `EFL / Elementary`
- `gcc` with C99 support
- `pkg-config`
- `libxml2`

## Project Structure

- `configure.ac` — Autoconf script for project configuration and dependency checks.
- `Makefile.am` — Top-level Automake file.
- `src/main.c` — Entry point and all application logic.
- `src/Makefile.am` — Automake file for the source directory.
- `data/` — Contains data files, such as the `.desktop` file and icon.
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
./src/eradio
```

To clean the build artifacts:

```bash
make clean
```

## Code Overview

The application initializes Elementary, creates a window with a search bar, a results list, and playback controls. It uses `Ecore_Con` to fetch station data from the `radio-browser.info` API, parses the XML response with `libxml2`, and populates a list. The `Emotion` library is used to handle media playback.

