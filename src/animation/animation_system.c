#include "../systems.h"
#include <math.h>

GameState animation_update_system(GameState state) {
    if (!state.settings.animations_enabled) {
        return state;
    }
    
    float frame_time = (float)state.system.frame_time;
    
    // Update letter pop animations
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (state.ui.letter_pop_timers[i] > 0.0f) {
            state.ui.letter_pop_timers[i] -= frame_time;
            if (state.ui.letter_pop_timers[i] < 0.0f) {
                state.ui.letter_pop_timers[i] = 0.0f;
            }
        }
    }
    
    // Update cursor pulse
    state.ui.cursor_pulse_timer += frame_time * CURSOR_PULSE_SPEED;
    if (state.ui.cursor_pulse_timer > 6.28318530718f) { // 2 * PI
        state.ui.cursor_pulse_timer -= 6.28318530718f;
    }
    
    // Update word celebration
    if (state.ui.celebrating_word) {
        state.ui.word_complete_timer -= frame_time;
        if (state.ui.word_complete_timer <= 0.0f) {
            state.ui.celebrating_word = 0;
            state.ui.word_complete_timer = 0.0f;
        }
    }
    
    // Update level celebration
    if (state.ui.celebrating_level) {
        state.ui.level_complete_timer -= frame_time;
        if (state.ui.level_complete_timer <= 0.0f) {
            state.ui.celebrating_level = 0;
            state.ui.level_complete_timer = 0.0f;
        }
    }
    
    // Update letter ease animation
    if (state.ui.letter_easing) {
        state.ui.letter_ease_timer += frame_time;
        if (state.ui.letter_ease_timer >= LETTER_EASE_DURATION) {
            state.ui.letter_easing = 0;
            state.ui.letter_ease_timer = 0.0f;
        }
    }
    
    // Update letter reveal animation
    if (state.ui.letter_revealing) {
        state.ui.letter_reveal_timer += frame_time;
        if (state.ui.letter_reveal_timer >= LETTER_REVEAL_DURATION) {
            state.ui.letter_revealing = 0;
            state.ui.letter_reveal_timer = 0.0f;
            // Complete word validation after animation finishes
            complete_word_validation(&state);
        }
    }
    
    // Update particles
    update_particles(&state, frame_time);
    
    return state;
}

float easeInOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - 2.0f * (1.0f - t) * (1.0f - t);
}

float easeOutElastic(float t) {
    const float c4 = (2.0f * 3.14159265359f) / 3.0f;
    return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : 
           pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}