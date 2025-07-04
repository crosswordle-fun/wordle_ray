# Crosswordle

An innovative hybrid word game that combines infinite Wordle gameplay with crossword puzzle mechanics, connected by a unique letter token system.

![Game Screenshot Placeholder]

## 🎮 What is Crosswordle?

Crosswordle features two interconnected game modes:

- **Infinite Wordle**: Unlimited attempts per level with infinite level progression
- **Crossword Puzzles**: Use letter tokens earned from Wordle to solve crossword grids
- **Letter Token System**: Your success in Wordle directly enables progress in Crossword mode

### Key Features

- 🔄 **Seamless mode switching** with Tab key
- 🎯 **Infinite Wordle levels** (no 6-guess limit)
- 🧩 **Crossword puzzles** with Wordle-style validation
- 🪙 **Letter token economy** connecting both modes
- ✨ **Smooth animations** and polished UI
- 🎨 **Authentic Wordle colors** and feedback system
- 🔧 **Debug mode** for testing and development

## 🚀 Quick Start

### Building the Game

#### Prerequisites
- CMake 3.11 or newer
- C compiler (GCC, Clang, or MSVC)

#### Build Instructions
```bash
# Clone the repository
git clone <repository-url>
cd crosswordle

# Create build directory and build
mkdir -p build
cd build
cmake ..
make

# Run the game
./crosswordle
```

### Dependencies
- **Raylib 5.5** (automatically downloaded and built)
- **Standard C libraries** (math.h, stdio.h, stdlib.h, string.h, time.h)

**Note**: On Linux systems, the math library (-lm) is automatically linked. On macOS, the required frameworks (IOKit, Cocoa, OpenGL) are automatically linked.

## 🎯 How to Play

### Wordle Mode

The infinite Wordle experience with a twist:

1. **Type a 5-letter word** using your keyboard
2. **Press Enter** to submit your guess
3. **Read the feedback**:
   - 🟩 **Green**: Correct letter in correct position
   - 🟨 **Yellow**: Correct letter in wrong position  
   - ⬜ **Gray**: Letter not in the target word
4. **Keep guessing** until you solve the word (no attempt limit!)
5. **Earn letter tokens** - get a random letter from each solved word
6. **Progress to the next level** and repeat

#### Wordle Controls
- **Letters**: Type your guess
- **Enter**: Submit word (must be exactly 5 letters)
- **Backspace**: Delete last letter
- **Mouse wheel / Up/Down arrows**: Scroll through guess history
- **Space**: Continue to next level (after solving)
- **Tab**: Switch to Crossword mode

### Crossword Mode

Use your earned letter tokens to solve crossword puzzles:

1. **Select a word** using Left/Right arrow keys
2. **Navigate within words** using Up/Down arrows (for vertical words)
3. **Place letters** by typing (consumes letter tokens)
4. **Validate words** by pressing Enter (requires exactly 5 letters)
5. **Get Wordle-style feedback** on your crossword entries
6. **Complete the puzzle** by filling all words correctly

#### Crossword Controls
- **Letters**: Place letters (uses tokens from inventory)
- **Left/Right arrows**: Select different words in the puzzle
- **Up/Down arrows**: Navigate within the current word
- **Enter**: Validate current word (needs 5 letters)
- **Shift**: Toggle between horizontal and vertical directions
- **Backspace**: Delete letter at cursor
- **Tab**: Switch to Wordle mode

### Shared Controls
- **Tab**: Switch between Wordle and Crossword modes
- **Number keys**: Access debug and settings features
  - **1**: Toggle debug mode (shows solutions)
  - **2**: Toggle letter bag display (show token inventory)
  - **3**: Award test tokens (development feature)
  - **4**: Toggle hard mode (placeholder)
  - **5**: Toggle color blind mode (placeholder)

## 🎲 Game Mechanics

### Letter Token System

The core innovation connecting both game modes:

- **Earning Tokens**: Each Wordle level completion awards one random letter from the solved word
- **Using Tokens**: Place letters in crossword grids by consuming tokens
- **Token Management**: Remove letters from crosswords to return tokens to inventory
- **Persistent Inventory**: Your letter collection carries across all gameplay sessions

### Game Progression

#### Wordle Mode
- **Infinite Levels**: No traditional 6-guess limit
- **Unlimited Attempts**: Take as many guesses as needed per level
- **Dynamic Grid**: Interface expands as you make more guesses
- **Auto-centering**: Camera smoothly follows your current input
- **Statistics Tracking**: Level completion, average guesses, best scores

#### Crossword Mode
- **Multiple Puzzles**: Various crossword layouts and difficulties
- **Smart Navigation**: Cursor only moves to valid, editable cells
- **Validation System**: Same Wordle logic applied to crossword words
- **Auto-completion**: Game detects when entire puzzle is solved
- **Token Efficiency**: Strategic letter placement and removal

### Visual Feedback

- **Authentic Colors**: Official Wordle color scheme throughout
- **Smooth Animations**: Letter reveals, typing feedback, celebrations
- **Clear UI**: Minimal interface focused on gameplay
- **Responsive Design**: Adapts to different window sizes
- **Accessibility**: Color blind mode and clear visual hierarchy

## 🛠️ Technical Details

### Architecture

Built with a clean, modular architecture:

- **ECS-Inspired Design**: Entity-Component-System patterns
- **Immutable State**: Pure functions operating on game state
- **Separation of Concerns**: Logic, rendering, and data clearly separated
- **Cross-platform**: Runs on Windows, macOS, and Linux

### File Structure
```
src/
├── main.c          # Entry point and main game loop
├── types.h         # Data structures and enums  
├── systems.h       # Function declarations
├── constants.h     # Game constants and colors
├── game.c          # Core game logic systems
├── render.c        # All rendering logic
└── words.c         # Word database (500+ words)
```

### Performance

- **60 FPS target** with smooth animations
- **Efficient rendering** with off-screen culling
- **Minimal memory usage** with stack-allocated data structures
- **No dynamic allocation** for consistent performance

## 🎨 Customization

### Debug Features

Enable debug mode (press `1`) to access:
- **Solution Display**: See target words and crossword solutions
- **Letter Bag**: View your current token inventory (press `2`)
- **Test Tokens**: Instantly award letters for testing (press `3`)

### Animation System

The game features a comprehensive animation system:
- **Letter Pop**: Typing feedback animation
- **Letter Reveal**: Staggered reveal of Wordle results  
- **Letter Ease**: Success feedback for validated words
- **Cursor Pulse**: Animated cursor indication
- **Particle Effects**: Celebration effects for achievements

Animations can be controlled via the settings system and respect the `animations_enabled` flag.

## 📊 Statistics

Track your progress with detailed statistics:
- **Levels Completed**: Total Wordle levels solved
- **Current Streak**: Consecutive levels completed
- **Average Guesses**: Efficiency metric across all levels
- **Best Score**: Minimum guesses for any single level
- **Letter Inventory**: Current token collection
- **Lifetime Stats**: Total guesses and completion rates

## 🤝 Contributing

This project welcomes contributions! Areas for enhancement:

- **New crossword layouts** and puzzle variety
- **Additional word databases** and categories  
- **Enhanced animation effects** and visual polish
- **Save system** for persistent progress
- **Multiplayer features** and social elements
- **Mobile platform** adaptations

## 📜 License

[Add your license information here]

## 🙏 Acknowledgments

- Built with [Raylib](https://www.raylib.com/) - A simple and easy-to-use library to enjoy videogames programming
- Inspired by [Wordle](https://www.nytimes.com/games/wordle/) by Josh Wardle
- Word list curated from common English vocabulary

---

**Enjoy playing Crosswordle!** 🎮✨

For technical details and development information, see [CLAUDE.md](CLAUDE.md).