# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
This is a Raylib-based C game project called "Crosswordle" - featuring both an infinite level-based Wordle game and a crossword puzzle mode. The Wordle mode has unlimited attempts per level and infinite progression. The crossword mode uses letter tokens earned from Wordle to solve crossword puzzles with proper Wordle-style validation. The project uses CMake for build configuration and automatically fetches Raylib 5.5 as a dependency.

## Build System
The project uses CMake with the following structure:
- `CMakeLists.txt`: Main build configuration
- `main.c`: Legacy entry point (use `src/main.c` instead)
- `src/main.c`: Current entry point with main function and game loop
- `src/types.h`: All data structures, enums, and type definitions
- `src/systems.h`: Centralized header with all function declarations
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
The game follows an Entity-Component-System (ECS) inspired architecture with distinct systems and proper C project structure. It features a dual-view architecture with separate game modes:

### Dual-View System
- **VIEW_WORDLE**: Traditional infinite Wordle gameplay with letter token rewards
- **VIEW_CROSSWORD**: Crossword puzzle mode using earned letter tokens
- **Tab Key**: Switches between views seamlessly
- **Shared State**: Both views use the same `GameState` struct with mode-specific substates

### Core Systems
- **Input System**: Universal keyboard input handling across both views
- **View Switching System**: Manages transitions between Wordle and Crossword modes
- **Word Editing System**: Manages word input in Wordle mode
- **Word Validation System**: Processes Wordle guesses and calculates letter states
- **Crossword Input System**: Handles letter placement, cursor movement, and direction switching in crossword mode
- **Crossword Word Validation System**: Validates crossword words using Wordle logic (Enter-triggered, 5-letter validation)
- **Render System**: Mode-aware rendering with separate render functions for each view

### Game State Structure
The game uses a comprehensive `GameState` struct containing:
- **CoreGameState**: Target word, attempts, game over status (Wordle mode)
- **PlayerInputState**: Current word input and completion status (Wordle mode)
- **GameHistoryState**: All previous guesses and their letter states (Wordle mode)
- **GameStatsState**: Win/loss statistics, streaks, and letter token inventory (shared)
- **GameSettingsState**: Game options and preferences (shared)
- **UIState**: Animation timers and display flags (shared)
- **SystemState**: Input handling and frame timing (shared)
- **CrosswordState**: Grid state, cursor position, validation states (Crossword mode)
- **GameViewState**: Current view (VIEW_WORDLE or VIEW_CROSSWORD)

### Key Features

#### Wordle Mode Features
- **Infinite Level Progression**: No 6-guess limit - unlimited attempts per level
- **Single-Row Display**: Clean interface showing only current input/result
- **Level-Based Statistics**: Track levels completed, average guesses, best scores
- **Letter Token Rewards**: Earn random letter tokens from each completed word
- **Smooth State Transitions**: Guess → Result display → Next input
- **Responsive layout** that adapts to window resizing

#### Crossword Mode Features
- **Letter Token System**: Use earned letters to fill crossword grids
- **Smart Cursor Navigation**: Arrow keys move only to valid word cells
- **Bidirectional Input**: Horizontal/vertical word entry with Shift to toggle
- **Enter-Key Validation**: Only validates complete 5-letter words when Enter is pressed
- **Wordle-Style Feedback**: Green (correct), Yellow (wrong position), Grey (not in word), Light grey (unvalidated)
- **Word Intersection Handling**: Proper crossword logic with shared letters
- **Auto-advance Cursor**: Moves to next cell after letter placement

#### Shared Features
- **Full Wordle color scheme** (green, yellow, gray) across both modes
- **Debug mode** with number key controls:
  - Press 1: Toggle debug mode (shows target word/solution)
  - Press 2: Toggle letter bag display
  - Press 3: Award test letter tokens (development feature)
  - Press 4: Toggle hard mode
  - Press 5: Toggle color blind mode

### Game Mechanics

#### Wordle Mode Mechanics
- **Level Progression**: Complete a word → advance to next level
- **No Attempt Limits**: Take as many guesses as needed per level
- **Dynamic Expanding Grid**: Starts with 1 row, expands with each guess
- **Camera Centering**: Always centers on current input row
- **Scrollable History**: Use mouse wheel or arrow keys to scroll up/down
- **Visual Flow**: Input → Submit → See colored result → Add to history → Continue
- **Letter Token Earning**: Completing a word awards a random letter from that word

#### Crossword Mode Mechanics
- **Letter Token Consumption**: Placing letters consumes tokens from inventory
- **Letter Token Return**: Removing letters returns tokens to inventory
- **Word Validation Flow**: Place 5 letters → Press Enter → See Wordle validation colors
- **Completion Detection**: Automatically detects when entire crossword is solved
- **Direction-Aware Navigation**: Cursor movement and deletion follow current direction (horizontal/vertical)

### Controls

#### Wordle Mode Controls
- **Letter keys**: Type your guess
- **Enter**: Submit completed 5-letter word
- **Backspace**: Delete last letter
- **Mouse wheel / Up/Down arrows**: Scroll through guess history
- **Space**: Continue to next level (when level is complete)

#### Crossword Mode Controls
- **Letter keys**: Place letters (consumes tokens)
- **Arrow keys**: Move cursor to valid word cells only
- **Enter**: Validate current word (5 letters required)
- **Shift**: Toggle direction (horizontal ↔ vertical)
- **Backspace**: Delete letter at cursor or move back and delete

#### Shared Controls
- **Tab**: Switch between Wordle and Crossword views
- **Number keys 1-5**: Toggle debug mode and settings

## Critical Architecture Details

### Validation System Design
- **Shared Validation Logic**: Both Wordle and Crossword modes use the same `calculate_letter_state()` and `check_word_match()` functions
- **Crossword Validation State**: Uses `letter_states[9][9]` and `word_validated[9][9]` arrays to track validation status per cell
- **Validation Triggering**: Crossword validation only occurs on Enter key when exactly 5 letters are placed in current direction
- **Visual State Management**: Unvalidated letters show light grey, validated letters show proper Wordle colors

### File Structure and Separation of Concerns
- **types.h**: All data structures, enums, and type definitions (no logic)
- **constants.h**: Game constants, colors, and configuration values
- **systems.h**: Function declarations for all game systems (header only)
- **game.c**: All game logic systems and state management
- **render.c**: All rendering logic separated by view (board_render_system, crossword_render_system)
- **words.c**: Word database and random word selection
- **main.c**: Minimal main loop with system orchestration

### System Processing Order
The main game loop processes systems in a specific order for each view:
1. **Shared**: `input_system()` → `view_switching_system()`
2. **Wordle View**: `word_editing_system()` → `word_validation_system()` → `result_display_system()` → `level_progression_system()`
3. **Crossword View**: `crossword_input_system()` → `crossword_word_validation_system()`
4. **Rendering**: `render_system()` (delegates to view-specific renderers)

## Key Dependencies
- Raylib 5.5 (automatically fetched if not found)
- CMake 3.11+ (for FetchContent support)
- Standard C library (stdio.h, stdlib.h, string.h, ctype.h, time.h)

## Development Notes
- The project includes a `docs/roadmap.md` with comprehensive feature enhancement ideas
- There's a legacy `main.c` in the root directory - use `src/main.c` instead
- Debug mode provides useful development tools accessible via number keys 1-5
- The architecture separates concerns cleanly: types, constants, systems, game logic, rendering, and word management