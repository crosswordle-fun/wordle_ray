# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
This is a Raylib-based C game project called "Wordle" - an infinite level-based word guessing game. Unlike traditional Wordle, this game has unlimited attempts per level and infinite progression through levels. The project uses CMake for build configuration and automatically fetches Raylib 5.5 as a dependency.

## Build System
The project uses CMake with the following structure:
- `CMakeLists.txt`: Main build configuration
- `src/main.c`: Entry point with main function and game loop
- `src/systems.h`: Centralized header with all function declarations and structs
- `src/constants.h`: Game constants and color definitions
- `src/game.c`: Game logic system implementations
- `src/render.c`: Rendering system implementations
- `src/words.c`: Word database and selection implementations
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

**Note**: The user handles all builds/rebuilds manually - Claude Code should not run build commands automatically.

## Architecture
The game follows an Entity-Component-System (ECS) inspired architecture with distinct systems and proper C project structure:

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
- **Infinite Level Progression**: No 6-guess limit - unlimited attempts per level
- **Single-Row Display**: Clean interface showing only current input/result
- **Level-Based Statistics**: Track levels completed, average guesses, best scores
- **Smooth State Transitions**: Guess → Result display → Next input
- **Responsive layout** that adapts to window resizing
- **Full Wordle color scheme** (green, yellow, gray)
- **Debug mode** with number key controls:
  - Press 1: Toggle debug mode (shows target word)
  - Press 2: Toggle sound effects
  - Press 3: Toggle animations
  - Press 4: Toggle hard mode
  - Press 5: Toggle color blind mode

### Game Mechanics
- **Level Progression**: Complete a word → advance to next level
- **No Attempt Limits**: Take as many guesses as needed per level
- **Visual Flow**: Input → Submit → See colored result → Continue or advance
- **Statistics**: Levels completed, best performance, lifetime averages

## Key Dependencies
- Raylib 5.5 (automatically fetched if not found)
- CMake 3.11+ (for FetchContent support)