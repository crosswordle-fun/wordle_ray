#include "systems.h"

GameState create_game_state(const char* target_word) {
    GameState state = {0};
    
    strcpy(state.core.target_word, target_word);
    state.core.current_level = 1;
    state.core.guesses_this_level = 0;
    state.core.total_lifetime_guesses = 0;
    state.core.play_state = GAME_STATE_INPUT;
    state.core.result_display_timer = 0.0f;
    state.core.level_complete = 0;
    
    state.input.current_letter_pos = 0;
    state.input.word_complete = 0;
    state.input.should_submit = 0;
    
    state.history.level_guess_count = 0;
    
    state.stats.levels_completed = 0;
    state.stats.current_level_streak = 1;
    state.stats.max_level_streak = 1;
    state.stats.total_guesses = 0;
    state.stats.average_guesses_per_level = 0.0f;
    state.stats.best_level_score = 999;  // Initialize to high number
    
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
    state.system.camera_offset_y = 0.0f;
    state.system.target_camera_offset_y = 0.0f;
    state.system.user_has_scrolled = 0;
    state.system.auto_center_paused = 0;
    
    return state;
}

GameState input_system(GameState state) {
    state.system.last_key_pressed = GetKeyPressed();
    state.system.space_pressed = IsKeyPressed(KEY_SPACE);
    state.system.enter_pressed = (state.system.last_key_pressed == KEY_ENTER);
    state.system.backspace_pressed = (state.system.last_key_pressed == KEY_BACKSPACE);
    state.system.letter_pressed = (state.system.last_key_pressed >= KEY_A && state.system.last_key_pressed <= KEY_Z);
    state.system.pressed_letter = state.system.letter_pressed ? (char)state.system.last_key_pressed : 0;
    
    state.system.number_key_pressed = (state.system.last_key_pressed >= KEY_ONE && state.system.last_key_pressed <= KEY_NINE);
    state.system.pressed_number = state.system.number_key_pressed ? (state.system.last_key_pressed - KEY_ONE + 1) : 0;
    
    // Scroll input
    state.system.scroll_wheel_move = (int)GetMouseWheelMove();
    state.system.up_arrow_pressed = IsKeyPressed(KEY_UP);
    state.system.down_arrow_pressed = IsKeyPressed(KEY_DOWN);
    
    state.system.frame_time = GetFrameTime();
    
    // Handle scrolling
    if (state.system.scroll_wheel_move != 0 || state.system.up_arrow_pressed || state.system.down_arrow_pressed) {
        float scroll_amount = 0.0f;
        
        if (state.system.scroll_wheel_move > 0 || state.system.up_arrow_pressed) {
            scroll_amount = -60.0f;  // Scroll up (negative offset to see older guesses)
        } else if (state.system.scroll_wheel_move < 0 || state.system.down_arrow_pressed) {
            scroll_amount = 60.0f;  // Scroll down (positive offset toward current input)
        }
        
        state.system.camera_offset_y += scroll_amount;
        
        // Calculate dynamic scroll bounds based on current game state
        int total_rows = state.history.level_guess_count + 1;  // completed guesses + current input
        float row_height = 70.0f;  // Approximate row height
        int screen_height = GetScreenHeight();
        
        // Allow scrolling up until attempt 1 (row 0) is centered on screen
        // When row 0 is centered: board_start_y + 0 * row_height = screen_height/2 - cell_size/2
        // Solving for camera_offset_y: camera_offset_y = (screen_height/2 - cell_size/2) - (desired_input_y - input_row_y)
        float max_scroll_up = -(state.history.level_guess_count * row_height);  // Centers first attempt
        
        // Allow scrolling down until input row is centered on screen
        // When input row is centered, camera_offset_y should be 0 (this is the natural center position)
        float max_scroll_down = 0.0f;  // Input row centered is the natural state
        
        // Apply bounds
        if (state.system.camera_offset_y < max_scroll_up) {
            state.system.camera_offset_y = max_scroll_up;
        }
        if (state.system.camera_offset_y > max_scroll_down) {
            state.system.camera_offset_y = max_scroll_down;
        }
        
        // User has manually scrolled - pause auto-centering
        state.system.user_has_scrolled = 1;
        state.system.auto_center_paused = 1;
        // Set target to current position to prevent camera interpolation from fighting
        state.system.target_camera_offset_y = state.system.camera_offset_y;
    }
    
    // Smooth camera interpolation toward target (only when not paused)
    if (!state.system.auto_center_paused) {
        float camera_lerp_speed = 8.0f;
        state.system.camera_offset_y += (state.system.target_camera_offset_y - state.system.camera_offset_y) * camera_lerp_speed * state.system.frame_time;
    }
    
    if (state.system.number_key_pressed) {
        switch (state.system.pressed_number) {
            case 1:
                state.system.debug_mode = !state.system.debug_mode;
                break;
            case 2:
                state.settings.sound_enabled = !state.settings.sound_enabled;
                break;
            case 3:
                state.settings.animations_enabled = !state.settings.animations_enabled;
                break;
            case 4:
                state.settings.hard_mode = !state.settings.hard_mode;
                break;
            case 5:
                state.settings.color_blind_mode = !state.settings.color_blind_mode;
                break;
        }
    }
    
    return state;
}

GameState word_editing_system(GameState state) {
    // Only allow input when in INPUT state
    if (state.core.play_state != GAME_STATE_INPUT) {
        return state;
    }
    
    if (state.system.letter_pressed && state.input.current_letter_pos < WORD_LENGTH) {
        state.input.current_word[state.input.current_letter_pos] = toupper(state.system.pressed_letter);
        state.input.current_letter_pos++;
        state.input.current_word[state.input.current_letter_pos] = '\0';
        
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
    
    // Update counters
    state.core.guesses_this_level++;
    state.core.total_lifetime_guesses++;
    state.stats.total_guesses++;
    
    // Add this guess to level history immediately
    if (state.history.level_guess_count < MAX_RECENT_GUESSES) {
        strcpy(state.history.level_guesses[state.history.level_guess_count], state.input.current_word);
        for (int i = 0; i < WORD_LENGTH; i++) {
            state.history.level_letter_states[state.history.level_guess_count][i] = state.history.current_guess_states[i];
        }
        state.history.level_guess_count++;
    }
    
    // Check if level is complete
    if (check_word_match(state.input.current_word, state.core.target_word)) {
        state.core.level_complete = 1;
        state.core.play_state = GAME_STATE_LEVEL_COMPLETE;
    } else {
        // Return to input state immediately for next guess
        state.core.play_state = GAME_STATE_INPUT;
        // Reset camera to center on new input row (only if not paused)
        if (!state.system.auto_center_paused) {
            state.system.target_camera_offset_y = 0.0f;
        }
    }
    
    // Clear input for next guess
    memset(state.input.current_word, 0, sizeof(state.input.current_word));
    state.input.current_letter_pos = 0;
    state.input.word_complete = 0;
    state.input.should_submit = 0;
    
    return state;
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
    
    // Update statistics
    state.stats.levels_completed++;
    state.stats.current_level_streak++;
    if (state.stats.current_level_streak > state.stats.max_level_streak) {
        state.stats.max_level_streak = state.stats.current_level_streak;
    }
    
    // Update best score if this level was solved with fewer guesses
    if (state.core.guesses_this_level < state.stats.best_level_score) {
        state.stats.best_level_score = state.core.guesses_this_level;
    }
    
    // Calculate average guesses per level
    if (state.stats.levels_completed > 0) {
        state.stats.average_guesses_per_level = 
            (float)state.stats.total_guesses / (float)state.stats.levels_completed;
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