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
    char current_word[WORD_LENGTH + 1];
    int current_letter_pos;
    int word_complete;
    int should_submit;
} WordEditingState;

typedef struct {
    char target_word[WORD_LENGTH + 1];
    int attempt_count;
    LetterState letter_states[MAX_ATTEMPTS][WORD_LENGTH];
    char all_guesses[MAX_ATTEMPTS][WORD_LENGTH + 1];
} GameProgressState;

typedef struct {
    int is_game_over;
    int player_won;
    int should_restart;
} GameControlState;

typedef struct {
    int last_key_pressed;
    int space_pressed;
    int enter_pressed;
    int backspace_pressed;
    int letter_pressed;
    char pressed_letter;
} InputState;

typedef struct {
    WordEditingState word_editing;
    GameProgressState game_progress;
    GameControlState game_control;
    InputState input;
} GameWorld;

GameWorld create_game_world(const char* target_word) {
    GameWorld world = {0};
    
    strcpy(world.game_progress.target_word, target_word);
    world.game_progress.attempt_count = 0;
    
    world.word_editing.current_letter_pos = 0;
    world.word_editing.word_complete = 0;
    world.word_editing.should_submit = 0;
    
    world.game_control.is_game_over = 0;
    world.game_control.player_won = 0;
    world.game_control.should_restart = 0;
    
    return world;
}

GameWorld input_system(GameWorld world) {
    world.input.last_key_pressed = GetKeyPressed();
    world.input.space_pressed = IsKeyPressed(KEY_SPACE);
    world.input.enter_pressed = (world.input.last_key_pressed == KEY_ENTER);
    world.input.backspace_pressed = (world.input.last_key_pressed == KEY_BACKSPACE);
    world.input.letter_pressed = (world.input.last_key_pressed >= KEY_A && world.input.last_key_pressed <= KEY_Z);
    world.input.pressed_letter = world.input.letter_pressed ? (char)world.input.last_key_pressed : 0;
    
    return world;
}

GameWorld word_editing_system(GameWorld world) {
    if (world.game_control.is_game_over) {
        return world;
    }
    
    if (world.input.letter_pressed && world.word_editing.current_letter_pos < WORD_LENGTH) {
        world.word_editing.current_word[world.word_editing.current_letter_pos] = toupper(world.input.pressed_letter);
        world.word_editing.current_letter_pos++;
        world.word_editing.current_word[world.word_editing.current_letter_pos] = '\0';
    }
    
    if (world.input.backspace_pressed && world.word_editing.current_letter_pos > 0) {
        world.word_editing.current_letter_pos--;
        world.word_editing.current_word[world.word_editing.current_letter_pos] = '\0';
    }
    
    world.word_editing.word_complete = (world.word_editing.current_letter_pos == WORD_LENGTH);
    world.word_editing.should_submit = (world.input.enter_pressed && world.word_editing.word_complete);
    
    return world;
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

GameWorld word_validation_system(GameWorld world) {
    if (!world.word_editing.should_submit) {
        return world;
    }
    
    int current_attempt = world.game_progress.attempt_count;
    
    strcpy(world.game_progress.all_guesses[current_attempt], world.word_editing.current_word);
    
    for (int i = 0; i < WORD_LENGTH; i++) {
        world.game_progress.letter_states[current_attempt][i] = 
            calculate_letter_state(
                world.word_editing.current_word[i], 
                i, 
                world.game_progress.target_word
            );
    }
    
    world.game_progress.attempt_count++;
    
    memset(world.word_editing.current_word, 0, sizeof(world.word_editing.current_word));
    world.word_editing.current_letter_pos = 0;
    world.word_editing.word_complete = 0;
    world.word_editing.should_submit = 0;
    
    return world;
}

GameWorld game_state_system(GameWorld world) {
    if (world.game_control.is_game_over) {
        return world;
    }
    
    if (world.game_progress.attempt_count > 0) {
        int last_attempt = world.game_progress.attempt_count - 1;
        if (check_word_match(world.game_progress.all_guesses[last_attempt], world.game_progress.target_word)) {
            world.game_control.player_won = 1;
            world.game_control.is_game_over = 1;
        }
    }
    
    if (world.game_progress.attempt_count >= MAX_ATTEMPTS) {
        world.game_control.is_game_over = 1;
    }
    
    return world;
}

GameWorld game_reset_system(GameWorld world, const char* new_target_word) {
    if (world.game_control.is_game_over && world.input.space_pressed) {
        world.game_control.should_restart = 1;
        return create_game_world(new_target_word);
    }
    
    return world;
}

#endif