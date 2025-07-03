# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
This is a Raylib-based C game project called "Wordle" - a complete implementation of the popular word guessing game. The project uses CMake for build configuration and automatically fetches Raylib 5.5 as a dependency.

## Build System
The project uses CMake with the following structure:
- `CMakeLists.txt`: Main build configuration
- `src/main.c`: Main game file containing rendering and game loop
- `src/game.h`: Game state management and core logic
- `src/words.h`: Word database and random selection
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
The game follows an Entity-Component-System (ECS) inspired architecture with distinct systems:

### Core Systems
- **Input System**: Handles keyboard input (letters, backspace, enter, F11)
- **Word Editing System**: Manages current word input and validation
- **Word Validation System**: Processes guesses and calculates letter states
- **Game State System**: Manages win/loss conditions and game progression
- **Game Reset System**: Handles game restart and statistics tracking
- **Render System**: Manages all drawing operations

### Game State Structure
The game uses a comprehensive `GameState` struct containing:
- **CoreGameState**: Target word, attempts, game over status
- **PlayerInputState**: Current word input and completion status
- **GameHistoryState**: All previous guesses and their letter states
- **GameStatsState**: Win/loss statistics and streaks
- **GameSettingsState**: Game options and preferences
- **UIState**: Animation timers and display flags
- **SystemState**: Input handling and frame timing

### Key Features
- Responsive layout that adapts to window resizing
- Full Wordle color scheme (green, yellow, gray)
- Game statistics tracking (games played, win rate, streaks)
- Fullscreen support (F11 key)
- Proper letter state calculation with position-aware logic
- Debug mode with number key controls:
  - Press 1: Toggle debug mode (shows target word)
  - Press 2: Toggle sound effects
  - Press 3: Toggle animations
  - Press 4: Toggle hard mode
  - Press 5: Toggle color blind mode

## Key Dependencies
- Raylib 5.5 (automatically fetched if not found)
- CMake 3.11+ (for FetchContent support)