#ifndef WORDLE_GAME_H
#define WORDLE_GAME_H

// Dependencies provided by systems.h

// Wordle game systems
static inline GameState word_editing_system(GameState state) {
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

static inline GameState word_validation_system(GameState state) {
    if (!state.input.should_submit || state.core.play_state != GAME_STATE_INPUT) {
        return state;
    }
    
    // Store the current guess for result display
    strcpy(state.history.current_guess, state.input.current_word);
    
    // Calculate letter states for this guess
    for (int i = 0; i < WORD_LENGTH; i++) {
        state.history.current_guess_states[i] = calculate_letter_state(
            state.input.current_word[i], i, state.core.target_word);
    }
    
    // Check if word matches target
    int word_correct = check_word_match(state.input.current_word, state.core.target_word);
    
    if (word_correct) {
        state.core.level_complete = 1;
        state.core.play_state = GAME_STATE_LEVEL_COMPLETE;
        
        // Award letter tokens from completed word
        for (int i = 0; i < WORD_LENGTH; i++) {
            char letter = state.core.target_word[i];
            int letter_index = letter - 'A';
            if (letter_index >= 0 && letter_index < 26) {
                state.stats.letter_counts[letter_index]++;
            }
        }
        
        // Update stats
        state.stats.levels_completed++;
        state.core.guesses_this_level++;
        state.core.total_lifetime_guesses++;
        
        trigger_level_celebration(&state);
    } else {
        // Wrong guess - transition to result display
        state.core.play_state = GAME_STATE_SHOWING_RESULT;
        state.core.result_display_timer = RESULT_DISPLAY_DURATION;
        state.ui.letter_revealing = 1;
        state.ui.letter_reveal_timer = 0.0f;
        
        state.core.guesses_this_level++;
        state.core.total_lifetime_guesses++;
    }
    
    // Add current guess to history
    if (state.history.level_guess_count < MAX_RECENT_GUESSES) {
        strcpy(state.history.level_guesses[state.history.level_guess_count], 
               state.input.current_word);
        for (int i = 0; i < WORD_LENGTH; i++) {
            state.history.level_letter_states[state.history.level_guess_count][i] = 
                state.history.current_guess_states[i];
        }
        state.history.level_guess_count++;
    }
    
    // Clear input for next guess
    memset(state.input.current_word, 0, sizeof(state.input.current_word));
    state.input.current_letter_pos = 0;
    state.input.word_complete = 0;
    state.input.should_submit = 0;
    
    return state;
}

static inline void complete_word_validation(GameState* state) {
    // This function is called after letter reveal animation completes
    if (state->core.play_state == GAME_STATE_SHOWING_RESULT) {
        state->core.play_state = GAME_STATE_INPUT;
        state->core.result_display_timer = 0.0f;
    }
}

static inline GameState result_display_system(GameState state) {
    if (state.core.play_state != GAME_STATE_SHOWING_RESULT) {
        return state;
    }
    
    if (!state.ui.letter_revealing) {
        state.core.result_display_timer -= state.system.frame_time;
        if (state.core.result_display_timer <= 0.0f) {
            state.core.play_state = GAME_STATE_INPUT;
            state.core.result_display_timer = 0.0f;
        }
    }
    
    return state;
}

static inline GameState level_progression_system(GameState state) {
    if (state.core.level_complete && state.system.space_pressed) {
        state.ui.show_level_complete = 0;
        state.core.level_complete = 0;
        state.core.play_state = GAME_STATE_INPUT_READY;
    }
    
    return state;
}

static inline GameState new_level_system(GameState state) {
    if (state.core.play_state == GAME_STATE_INPUT_READY) {
        // Generate new target word
        strcpy(state.core.target_word, get_random_word());
        
        // Reset level state
        state.core.current_level++;
        state.core.guesses_this_level = 0;
        state.core.play_state = GAME_STATE_INPUT;
        
        // Clear history for new level
        state.history.level_guess_count = 0;
        memset(state.history.level_guesses, 0, sizeof(state.history.level_guesses));
        memset(state.history.level_letter_states, 0, sizeof(state.history.level_letter_states));
        
        // Clear input
        memset(state.input.current_word, 0, sizeof(state.input.current_word));
        state.input.current_letter_pos = 0;
        state.input.word_complete = 0;
        state.input.should_submit = 0;
        
        // Reset camera
        state.system.camera_offset_y = 0.0f;
        state.system.target_camera_offset_y = 0.0f;
        state.system.user_has_scrolled = 0;
        state.system.auto_center_paused = 0;
    }
    
    return state;
}

#endif