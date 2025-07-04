#include "../systems.h"

GameState word_editing_system(GameState state) {
    // Only allow input when in INPUT state
    if (state.core.play_state != GAME_STATE_INPUT) {
        return state;
    }
    
    if (state.system.letter_pressed && state.input.current_letter_pos < WORD_LENGTH) {
        int letter_index = state.input.current_letter_pos;
        state.input.current_word[state.input.current_letter_pos] = toupper(state.system.pressed_letter);
        state.input.current_letter_pos++;
        state.input.current_word[state.input.current_letter_pos] = '\0';
        
        // Trigger letter pop animation
        trigger_letter_pop(&state, letter_index);
        
        // User started typing - reactivate auto-centering
        if (state.system.user_has_scrolled) {
            state.system.auto_center_paused = 0;
            state.system.user_has_scrolled = 0;
            state.system.target_camera_offset_y = 0.0f;  // Center on current input
        }
    }
    
    if (state.system.backspace_pressed && state.input.current_letter_pos > 0) {
        state.input.current_letter_pos--;
        state.input.current_word[state.input.current_letter_pos] = '\0';
        
        // User started typing (backspace counts as typing) - reactivate auto-centering
        if (state.system.user_has_scrolled) {
            state.system.auto_center_paused = 0;
            state.system.user_has_scrolled = 0;
            state.system.target_camera_offset_y = 0.0f;  // Center on current input
        }
    }
    
    state.input.word_complete = (state.input.current_letter_pos == WORD_LENGTH);
    state.input.should_submit = (state.system.enter_pressed && state.input.word_complete);
    
    return state;
}

GameState word_validation_system(GameState state) {
    if (!state.input.should_submit || state.core.play_state != GAME_STATE_INPUT) {
        return state;
    }
    
    // Store the current guess for result display
    strcpy(state.history.current_guess, state.input.current_word);
    
    // Calculate letter states for the current guess
    for (int i = 0; i < WORD_LENGTH; i++) {
        state.history.current_guess_states[i] = 
            calculate_letter_state(
                state.input.current_word[i], 
                i, 
                state.core.target_word
            );
    }
    
    // Start letter reveal animation if animations are enabled
    if (state.settings.animations_enabled) {
        state.ui.letter_revealing = 1;
        state.ui.letter_reveal_timer = 0.0f;
        state.core.play_state = GAME_STATE_SHOWING_RESULT;
    } else {
        // No animation - complete validation immediately
        complete_word_validation(&state);
    }
    
    // Clear input for next guess
    memset(state.input.current_word, 0, sizeof(state.input.current_word));
    state.input.current_letter_pos = 0;
    state.input.word_complete = 0;
    state.input.should_submit = 0;
    
    return state;
}

void complete_word_validation(GameState* state) {
    // Update counters
    state->core.guesses_this_level++;
    state->core.total_lifetime_guesses++;
    state->stats.total_guesses++;
    
    // Add this guess to level history
    if (state->history.level_guess_count < MAX_RECENT_GUESSES) {
        strcpy(state->history.level_guesses[state->history.level_guess_count], state->history.current_guess);
        for (int i = 0; i < WORD_LENGTH; i++) {
            state->history.level_letter_states[state->history.level_guess_count][i] = state->history.current_guess_states[i];
        }
        state->history.level_guess_count++;
    }
    
    // Check if level is complete
    if (check_word_match(state->history.current_guess, state->core.target_word)) {
        // Award a random letter token from the solved word (only happens once per level)
        int random_letter_index = GetRandomValue(0, WORD_LENGTH - 1);
        char awarded_letter = state->core.target_word[random_letter_index];
        int letter_array_index = awarded_letter - 'A';  // Convert A-Z to 0-25
        state->stats.letter_counts[letter_array_index]++;
        
        // Update statistics (only happens once per level completion)
        state->stats.levels_completed++;
        state->stats.current_level_streak++;
        if (state->stats.current_level_streak > state->stats.max_level_streak) {
            state->stats.max_level_streak = state->stats.current_level_streak;
        }
        
        // Update best score if this level was solved with fewer guesses
        if (state->core.guesses_this_level < state->stats.best_level_score) {
            state->stats.best_level_score = state->core.guesses_this_level;
        }
        
        // Calculate average guesses per level
        if (state->stats.levels_completed > 0) {
            state->stats.average_guesses_per_level = 
                (float)state->stats.total_guesses / (float)state->stats.levels_completed;
        }
        
        state->core.level_complete = 1;
        state->core.play_state = GAME_STATE_LEVEL_COMPLETE;
        
        // Trigger celebration animations
        trigger_word_celebration(state);
        trigger_level_celebration(state);
        trigger_letter_ease(state);
    } else {
        // Return to input state for next guess
        state->core.play_state = GAME_STATE_INPUT;
        // Reset camera to focus on new input row
        if (!state->system.user_has_scrolled) {
            state->system.auto_center_paused = 0;
            state->system.target_camera_offset_y = 0.0f;
        }
    }
}

GameState result_display_system(GameState state) {
    // This system is no longer needed since we process guesses instantly
    // Keeping the function for compatibility but it does nothing
    return state;
}

GameState level_progression_system(GameState state) {
    if (state.core.play_state != GAME_STATE_LEVEL_COMPLETE) {
        return state;
    }
    
    // Wait for space to continue to next level
    if (state.system.space_pressed) {
        state.core.play_state = GAME_STATE_INPUT_READY;
    }
    
    return state;
}

GameState new_level_system(GameState state) {
    if (state.core.play_state != GAME_STATE_INPUT_READY) {
        return state;
    }
    
    // Start new level
    state.core.current_level++;
    state.core.guesses_this_level = 0;
    state.core.level_complete = 0;
    state.core.play_state = GAME_STATE_INPUT;
    
    // Clear level history for new level
    state.history.level_guess_count = 0;
    
    // Reset camera to center on the first (and only) row
    state.system.camera_offset_y = 0.0f;
    state.system.target_camera_offset_y = 0.0f;
    
    return state;
}