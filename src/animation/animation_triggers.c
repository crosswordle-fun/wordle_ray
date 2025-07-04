#include "../systems.h"

void trigger_letter_pop(GameState* state, int letter_index) {
    if (!state->settings.animations_enabled || letter_index < 0 || letter_index >= WORD_LENGTH) {
        return;
    }
    state->ui.letter_pop_timers[letter_index] = LETTER_POP_DURATION;
}

void trigger_word_celebration(GameState* state) {
    if (!state->settings.animations_enabled) {
        return;
    }
    state->ui.celebrating_word = 1;
    state->ui.word_complete_timer = WORD_COMPLETE_DURATION;
    
    // Spawn celebration particles at center of screen
    Vector2 center = {(float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f};
    spawn_particles(state, center, WORDLE_GREEN, 20);
}

void trigger_level_celebration(GameState* state) {
    if (!state->settings.animations_enabled) {
        return;
    }
    state->ui.celebrating_level = 1;
    state->ui.level_complete_timer = LEVEL_COMPLETE_DURATION;
    
    // Spawn more particles for level completion
    Vector2 center = {(float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f};
    spawn_particles(state, center, WORDLE_YELLOW, 30);
    spawn_particles(state, center, WORDLE_GREEN, 20);
}

void trigger_letter_ease(GameState* state) {
    if (!state->settings.animations_enabled) {
        return;
    }
    state->ui.letter_easing = 1;
    state->ui.letter_ease_timer = 0.0f;
}