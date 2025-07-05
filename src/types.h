#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include "constants.h"

// Game Constants
#define WORD_LENGTH 5

// Enums
typedef enum {
    LETTER_UNKNOWN = 0,
    LETTER_CORRECT = 1,
    LETTER_WRONG_POS = 2,
    LETTER_NOT_IN_WORD = 3
} LetterState;

typedef enum {
    GAME_STATE_INPUT = 0,
    GAME_STATE_SHOWING_RESULT = 1,
    GAME_STATE_LEVEL_COMPLETE = 2,
    GAME_STATE_INPUT_READY = 3
} GamePlayState;

typedef enum {
    VIEW_HOME_SCREEN = 0,
    VIEW_WORDLE = 1,
    VIEW_CROSSWORD = 2,
    VIEW_CROSSWORD_COMPLETE = 3
} GameViewState;

// Game State Structs
typedef struct {
    char target_word[WORD_LENGTH + 1];
    int current_level;
    int guesses_this_level;
    int total_lifetime_guesses;
    GamePlayState play_state;
    float result_display_timer;
    int level_complete;
} CoreGameState;

typedef struct {
    char current_word[WORD_LENGTH + 1];
    int current_letter_pos;
    int word_complete;
    int should_submit;
} PlayerInputState;

typedef struct {
    LetterState level_letter_states[MAX_RECENT_GUESSES][WORD_LENGTH];
    char level_guesses[MAX_RECENT_GUESSES][WORD_LENGTH + 1];
    int level_guess_count;
    LetterState current_guess_states[WORD_LENGTH];  // For result display state
    char current_guess[WORD_LENGTH + 1];           // For result display state
} GameHistoryState;

typedef struct {
    int levels_completed;
    int current_level_streak;
    int max_level_streak;
    int total_guesses;
    float average_guesses_per_level;
    int best_level_score;  // minimum guesses for any level
    int letter_counts[26]; // A=0, B=1, ..., Z=25 - collected letter tokens
    int show_letter_bag;   // Toggle for letter bag display
} GameStatsState;

typedef struct {
    int sound_enabled;
    int animations_enabled;
    int hard_mode;
    int color_blind_mode;
} GameSettingsState;

typedef struct {
    float letter_reveal_timer;
    float game_over_timer;
    int show_statistics;
    int show_help;
    int animate_letters;
    
    // Animation timers and states
    float letter_pop_timers[WORD_LENGTH];     // Per-letter typing animation timers
    float cursor_pulse_timer;                 // Cursor blinking animation
    float word_complete_timer;                // Word completion celebration timer
    float level_complete_timer;               // Level completion celebration timer
    float view_transition_timer;              // View switching animation timer
    float letter_ease_timer;                  // Letter ease in/out animation timer
    
    // Animation states
    int letter_revealing;                     // Flag for letter reveal animation in progress
    int celebrating_word;                     // Flag for word completion celebration
    int celebrating_level;                    // Flag for level completion celebration
    int transitioning_view;                   // Flag for view transition in progress
    int letter_easing;                        // Flag for letter ease animation in progress
    
    // View transition state
    GameViewState previous_view;              // View we're transitioning from
    int transition_direction;                 // 0=left-to-right, 1=right-to-left
    
    // Particle effects
    int particle_count;                       // Number of active particles
    Vector2 particles[50];                    // Particle positions (max 50)
    Vector2 particle_velocities[50];          // Particle velocities
    float particle_lifetimes[50];             // Particle remaining lifetimes
    Color particle_colors[50];                // Particle colors
} UIState;

typedef struct {
    int start_x;            // Starting X position
    int start_y;            // Starting Y position
    int direction;          // 0=horizontal, 1=vertical
    int length;             // Word length (always 5 for now)
} CrosswordWord;

typedef struct {
    char word[WORD_LENGTH + 1];     // The word itself
    int start_x, start_y;           // Starting position in grid
    int direction;                  // 0=horizontal, 1=vertical
    int placed;                     // 1 if successfully placed, 0 if not
} PlacedWord;

typedef struct {
    int word1_index, word2_index;   // Indices of the two words
    int word1_pos, word2_pos;       // Letter positions within words
    int grid_x, grid_y;             // Grid coordinates of intersection
    int word2_direction;            // Direction word2 would be placed
} IntersectionCandidate;

typedef struct {
    char solution[9][9];    // Correct letters for the puzzle
    char word_mask[9][9];   // 1 = word cell, 0 = blocked cell
    int level;              // Current crossword level
    CrosswordWord words[10]; // Array of word definitions (max 10 words)
    int word_count;         // Number of words in this level
} CrosswordLevel;

typedef struct {
    char grid[9][9];        // 9x9 grid of placed letters ('\0' = empty)
    int cursor_x;           // Current cursor position X (0-8)
    int cursor_y;           // Current cursor position Y (0-8)
    int cursor_direction;   // 0=horizontal (right), 1=vertical (down)
    CrosswordLevel current_level; // Current puzzle data
    int should_validate;    // Set to 1 when Enter is pressed to validate current word
    LetterState letter_states[9][9]; // Validation states for each cell
    int word_validated[9][9]; // Track which words have been validated
    int puzzle_completed;   // Set to 1 when entire crossword is solved
    int current_word_index; // Currently selected word (0-based)
} CrosswordState;

typedef struct {
    int last_key_pressed;
    int space_pressed;
    int enter_pressed;
    int backspace_pressed;
    int letter_pressed;
    char pressed_letter;
    double frame_time;
    int debug_mode;
    int number_key_pressed;
    int pressed_number;
    float camera_offset_y;
    float target_camera_offset_y;
    int scroll_wheel_move;
    int up_arrow_pressed;
    int down_arrow_pressed;
    int user_has_scrolled;
    int auto_center_paused;
    int tab_pressed;
} SystemState;

typedef struct {
    CoreGameState core;
    PlayerInputState input;
    GameHistoryState history;
    GameStatsState stats;
    GameSettingsState settings;
    UIState ui;
    SystemState system;
    GameViewState current_view;
    CrosswordState crossword;
} GameState;

// Rendering Structs
typedef struct {
    int screen_width;
    int screen_height;
    int cell_size;
    int cell_spacing;
    int board_width;
    int board_height;
    int board_start_x;
    int board_start_y;
    int total_rows;
    int current_input_row;
    int row_height;
} LayoutConfig;

#endif