#ifndef GAME_H
#define GAME_H

#include <string.h>
#include <ctype.h>
#include "raylib.h"

#define WORD_LENGTH 5
#define MAX_ATTEMPTS 6

typedef enum {
    LETTER_UNKNOWN = 0,
    LETTER_CORRECT = 1,
    LETTER_WRONG_POS = 2,
    LETTER_NOT_IN_WORD = 3
} LetterState;

typedef struct {
    char target_word[WORD_LENGTH + 1];
    int current_attempt;
    int is_game_over;
    int player_won;
    int should_restart;
} CoreGameState;

typedef struct {
    char current_word[WORD_LENGTH + 1];
    int current_letter_pos;
    int word_complete;
    int should_submit;
} PlayerInputState;

typedef struct {
    LetterState letter_states[MAX_ATTEMPTS][WORD_LENGTH];
    char all_guesses[MAX_ATTEMPTS][WORD_LENGTH + 1];
    int attempt_count;
} GameHistoryState;

typedef struct {
    int games_played;
    int games_won;
    int current_streak;
    int max_streak;
    int win_distribution[MAX_ATTEMPTS];
    float win_percentage;
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

GameState create_game_state(const char* target_word) {
    GameState state = {0};
    
    strcpy(state.core.target_word, target_word);
    state.core.current_attempt = 0;
    state.core.is_game_over = 0;
    state.core.player_won = 0;
    state.core.should_restart = 0;
    
    state.input.current_letter_pos = 0;
    state.input.word_complete = 0;
    state.input.should_submit = 0;
    
    state.history.attempt_count = 0;
    
    state.stats.games_played = 0;
    state.stats.games_won = 0;
    state.stats.current_streak = 0;
    state.stats.max_streak = 0;
    state.stats.win_percentage = 0.0f;
    
    state.settings.sound_enabled = 1;
    state.settings.animations_enabled = 1;
    state.settings.hard_mode = 0;
    state.settings.color_blind_mode = 0;
    
    state.ui.letter_reveal_timer = 0.0f;
    state.ui.game_over_timer = 0.0f;
    state.ui.show_statistics = 0;
    state.ui.show_help = 0;
    state.ui.animate_letters = 0;
    
    state.system.frame_time = 0.0;
    state.system.debug_mode = 0;
    
    return state;
}

GameState input_system(GameState state) {
    state.system.last_key_pressed = GetKeyPressed();
    state.system.space_pressed = IsKeyPressed(KEY_SPACE);
    state.system.enter_pressed = (state.system.last_key_pressed == KEY_ENTER);
    state.system.backspace_pressed = (state.system.last_key_pressed == KEY_BACKSPACE);
    state.system.letter_pressed = (state.system.last_key_pressed >= KEY_A && state.system.last_key_pressed <= KEY_Z);
    state.system.pressed_letter = state.system.letter_pressed ? (char)state.system.last_key_pressed : 0;
    
    state.system.frame_time = GetFrameTime();
    
    return state;
}

GameState word_editing_system(GameState state) {
    if (state.core.is_game_over) {
        return state;
    }
    
    if (state.system.letter_pressed && state.input.current_letter_pos < WORD_LENGTH) {
        state.input.current_word[state.input.current_letter_pos] = toupper(state.system.pressed_letter);
        state.input.current_letter_pos++;
        state.input.current_word[state.input.current_letter_pos] = '\0';
    }
    
    if (state.system.backspace_pressed && state.input.current_letter_pos > 0) {
        state.input.current_letter_pos--;
        state.input.current_word[state.input.current_letter_pos] = '\0';
    }
    
    state.input.word_complete = (state.input.current_letter_pos == WORD_LENGTH);
    state.input.should_submit = (state.system.enter_pressed && state.input.word_complete);
    
    return state;
}

int is_letter_in_target_word(char letter, const char* target_word) {
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (target_word[i] == letter) {
            return 1;
        }
    }
    return 0;
}

LetterState calculate_letter_state(char guess_letter, int position, const char* target_word) {
    if (guess_letter == target_word[position]) {
        return LETTER_CORRECT;
    } else if (is_letter_in_target_word(guess_letter, target_word)) {
        return LETTER_WRONG_POS;
    } else {
        return LETTER_NOT_IN_WORD;
    }
}

int check_word_match(const char* word1, const char* word2) {
    return strcmp(word1, word2) == 0;
}

GameState word_validation_system(GameState state) {
    if (!state.input.should_submit) {
        return state;
    }
    
    int current_attempt = state.history.attempt_count;
    
    strcpy(state.history.all_guesses[current_attempt], state.input.current_word);
    
    for (int i = 0; i < WORD_LENGTH; i++) {
        state.history.letter_states[current_attempt][i] = 
            calculate_letter_state(
                state.input.current_word[i], 
                i, 
                state.core.target_word
            );
    }
    
    state.history.attempt_count++;
    state.core.current_attempt = state.history.attempt_count;
    
    memset(state.input.current_word, 0, sizeof(state.input.current_word));
    state.input.current_letter_pos = 0;
    state.input.word_complete = 0;
    state.input.should_submit = 0;
    
    return state;
}

GameState game_state_system(GameState state) {
    if (state.core.is_game_over) {
        return state;
    }
    
    if (state.history.attempt_count > 0) {
        int last_attempt = state.history.attempt_count - 1;
        if (check_word_match(state.history.all_guesses[last_attempt], state.core.target_word)) {
            state.core.player_won = 1;
            state.core.is_game_over = 1;
        }
    }
    
    if (state.history.attempt_count >= MAX_ATTEMPTS) {
        state.core.is_game_over = 1;
    }
    
    return state;
}

GameState game_reset_system(GameState state, const char* new_target_word) {
    if (state.core.is_game_over && state.system.space_pressed) {
        state.core.should_restart = 1;
        
        GameStatsState preserved_stats = state.stats;
        GameSettingsState preserved_settings = state.settings;
        
        state.stats.games_played++;
        if (state.core.player_won) {
            state.stats.games_won++;
            state.stats.current_streak++;
            if (state.stats.current_streak > state.stats.max_streak) {
                state.stats.max_streak = state.stats.current_streak;
            }
            state.stats.win_distribution[state.history.attempt_count - 1]++;
        } else {
            state.stats.current_streak = 0;
        }
        
        state.stats.win_percentage = (state.stats.games_played > 0) ? 
            (float)state.stats.games_won / (float)state.stats.games_played * 100.0f : 0.0f;
        
        state = create_game_state(new_target_word);
        state.stats = preserved_stats;
        state.settings = preserved_settings;
    }
    
    return state;
}

#endif