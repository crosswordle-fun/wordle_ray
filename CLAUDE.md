# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
This is a Raylib-based C game project called "crosswordle" (currently a basic Raylib template). The project uses CMake for build configuration and automatically fetches Raylib 5.5 as a dependency.

## Build System
The project uses CMake with the following structure:
- `CMakeLists.txt`: Main build configuration
- `main.c`: Single source file containing the game logic
- `build/`: Generated build directory

## Common Commands

### Build the project
```bash
mkdir -p build
cd build
cmake ..
make
```

### Run the executable
```bash
./build/crosswordle
```

### Clean build
```bash
rm -rf build
```

## Architecture
- Single-file C application using Raylib for graphics and window management
- CMake automatically handles Raylib dependency via FetchContent
- Platform-specific linking for macOS (IOKit, Cocoa, OpenGL frameworks)
- Generates `compile_commands.json` for IDE integration

## Key Dependencies
- Raylib 5.5 (automatically fetched if not found)
- CMake 3.11+ (for FetchContent support)