#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "raylib.h"
#include "constants.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    LetterState current_guess_states[WORD_LENGTH];
    char current_guess[WORD_LENGTH + 1];
    LetterState recent_letter_states[MAX_RECENT_GUESSES][WORD_LENGTH];
    char recent_guesses[MAX_RECENT_GUESSES][WORD_LENGTH + 1];
    int recent_guess_count;
} GameHistoryState;

typedef struct {
    int levels_completed;
    int current_level_streak;
    int max_level_streak;
    int total_guesses;
    float average_guesses_per_level;
    int best_level_score;  // minimum guesses for any level
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
} UIState;

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
} SystemState;

typedef struct {
    CoreGameState core;
    PlayerInputState input;
    GameHistoryState history;
    GameStatsState stats;
    GameSettingsState settings;
    UIState ui;
    SystemState system;
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
} LayoutConfig;

// Game System Function Declarations
GameState create_game_state(const char* target_word);
GameState input_system(GameState state);
GameState word_editing_system(GameState state);
GameState word_validation_system(GameState state);
GameState level_progression_system(GameState state);
GameState result_display_system(GameState state);
GameState new_level_system(GameState state);

// Game Logic Helper Function Declarations
int is_letter_in_target_word(char letter, const char* target_word);
LetterState calculate_letter_state(char guess_letter, int position, const char* target_word);
int check_word_match(const char* word1, const char* word2);

// Rendering System Function Declarations
LayoutConfig calculate_layout(void);
Color get_color_for_letter_state(LetterState state);
void board_render_system(GameState state);
void ui_render_system(GameState state);
void render_system(GameState state);

// Word System Function Declarations
void initialize_random_seed(void);
const char* get_random_word(void);

#endif