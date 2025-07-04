#include "systems.h"
#include <math.h>

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
    
    // Initialize letter bag
    for (int i = 0; i < 26; i++) {
        state.stats.letter_counts[i] = 0;
    }
    state.stats.show_letter_bag = 0;
    
    state.settings.sound_enabled = 1;
    state.settings.animations_enabled = 1;
    state.settings.hard_mode = 0;
    state.settings.color_blind_mode = 0;
    
    state.ui.letter_reveal_timer = 0.0f;
    state.ui.game_over_timer = 0.0f;
    state.ui.show_statistics = 0;
    state.ui.show_help = 0;
    state.ui.animate_letters = 0;
    
    // Initialize new animation timers and states
    for (int i = 0; i < WORD_LENGTH; i++) {
        state.ui.letter_pop_timers[i] = 0.0f;
    }
    state.ui.cursor_pulse_timer = 0.0f;
    state.ui.word_complete_timer = 0.0f;
    state.ui.level_complete_timer = 0.0f;
    state.ui.view_transition_timer = 0.0f;
    state.ui.letter_ease_timer = 0.0f;
    
    state.ui.letter_revealing = 0;
    state.ui.celebrating_word = 0;
    state.ui.celebrating_level = 0;
    state.ui.transitioning_view = 0;
    state.ui.letter_easing = 0;
    
    state.ui.particle_count = 0;
    for (int i = 0; i < 50; i++) {
        state.ui.particles[i] = (Vector2){0, 0};
        state.ui.particle_velocities[i] = (Vector2){0, 0};
        state.ui.particle_lifetimes[i] = 0.0f;
        state.ui.particle_colors[i] = WHITE;
    }
    
    state.system.frame_time = 0.0;
    state.system.debug_mode = 0;
    state.system.camera_offset_y = 0.0f;
    state.system.target_camera_offset_y = 0.0f;
    state.system.user_has_scrolled = 0;
    state.system.auto_center_paused = 0;
    
    state.current_view = VIEW_HOME_SCREEN;
    
    // Initialize crossword grid to empty
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {
            state.crossword.grid[x][y] = '\0';
            state.crossword.letter_states[x][y] = LETTER_UNKNOWN;
            state.crossword.word_validated[x][y] = 0;
        }
    }
    state.crossword.current_level = get_crossword_level(1);
    
    // Initialize cursor to first word (Word 1)
    state.crossword.current_word_index = 0;  // Start with Word 1
    state.crossword.cursor_x = state.crossword.current_level.words[0].start_x;
    state.crossword.cursor_y = state.crossword.current_level.words[0].start_y;
    state.crossword.cursor_direction = state.crossword.current_level.words[0].direction;
    state.crossword.should_validate = 0;   // Initialize validation flag
    state.crossword.puzzle_completed = 0;  // Initialize completion flag
    
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
    state.system.tab_pressed = IsKeyPressed(KEY_TAB);
    state.system.shift_pressed = IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT);
    
    state.system.frame_time = GetFrameTime();
    
    // Handle scrolling (only in Wordle view)
    if ((state.system.scroll_wheel_move != 0 || state.system.up_arrow_pressed || state.system.down_arrow_pressed) && state.current_view == VIEW_WORDLE) {
        float scroll_amount = 0.0f;
        
        if (state.system.scroll_wheel_move > 0 || state.system.up_arrow_pressed) {
            scroll_amount = 60.0f;  // Scroll up (positive offset to see older guesses)
        } else if (state.system.scroll_wheel_move < 0 || state.system.down_arrow_pressed) {
            scroll_amount = -60.0f;  // Scroll down (negative offset toward current input)
        }
        
        state.system.camera_offset_y += scroll_amount;
        
        // Calculate dynamic scroll bounds based on current game state
        int total_rows = state.history.level_guess_count + 1;  // completed guesses + current input
        int screen_height = GetScreenHeight();
        
        // Use actual row height from layout calculation
        int available_width = GetScreenWidth() - 100;
        int cell_size = available_width / (WORD_LENGTH + (WORD_LENGTH - 1) * 0.12f);
        if (cell_size < 50) cell_size = 50;
        if (cell_size > 100) cell_size = 100;
        int cell_spacing = (int)(cell_size * 0.12f);
        int actual_row_height = cell_size + cell_spacing;
        
        // To center attempt 1 (row 0):
        // We want: board_start_y + 0 * row_height = screen_height/2 - cell_size/2
        // From calculate_layout: board_start_y = desired_input_y - input_row_y + camera_offset_y
        // Where: desired_input_y = screen_height/2 - cell_size/2 (center of screen)
        // And: input_row_y = current_input_row * row_height
        // So: camera_offset_y = (screen_height/2 - cell_size/2) - (desired_input_y - input_row_y)
        // Simplifying: camera_offset_y = input_row_y = level_guess_count * row_height
        float max_scroll_up = state.history.level_guess_count * actual_row_height;
        
        // Allow scrolling down until input row is centered on screen
        // Input row centered is the natural state (camera_offset_y = 0)
        float max_scroll_down = 0.0f;
        
        // Apply bounds
        if (state.system.camera_offset_y > max_scroll_up) {
            state.system.camera_offset_y = max_scroll_up;
        }
        if (state.system.camera_offset_y < max_scroll_down) {
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
        float camera_lerp_speed = 4.0f;  // Reduced for smoother movement
        state.system.camera_offset_y += (state.system.target_camera_offset_y - state.system.camera_offset_y) * camera_lerp_speed * state.system.frame_time;
    }
    
    if (state.system.number_key_pressed) {
        switch (state.system.pressed_number) {
            case 1:
                state.system.debug_mode = !state.system.debug_mode;
                break;
            case 2:
                state.stats.show_letter_bag = !state.stats.show_letter_bag;
                break;
            case 3:
                // Testing feature: Award 10 tokens of each letter
                for (int i = 0; i < 26; i++) {
                    state.stats.letter_counts[i] += 10;
                }
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

// Helper function to find first editable (non-green) cell in a specific word
int find_first_editable_cell_in_word(GameState state, int word_index, int* out_x, int* out_y) {
    if (word_index < 0 || word_index >= state.crossword.current_level.word_count) {
        return 0; // Invalid word index
    }
    
    CrosswordWord* word = &state.crossword.current_level.words[word_index];
    
    // Search through word positions
    for (int i = 0; i < word->length; i++) {
        int check_x = word->start_x + (word->direction == 0 ? i : 0);
        int check_y = word->start_y + (word->direction == 1 ? i : 0);
        
        // Check if this position is NOT green (available for editing)
        if (!(state.crossword.word_validated[check_x][check_y] && 
              state.crossword.letter_states[check_x][check_y] == LETTER_CORRECT)) {
            *out_x = check_x;
            *out_y = check_y;
            return 1; // Found editable cell
        }
    }
    
    return 0; // No editable cell found
}

// Helper function to find next incomplete word starting from current word
int find_next_incomplete_word(GameState state) {
    int current_word = state.crossword.current_word_index;
    
    // Start searching from next word, wrap around
    for (int i = 1; i <= state.crossword.current_level.word_count; i++) {
        int check_word_index = (current_word + i) % state.crossword.current_level.word_count;
        
        int dummy_x, dummy_y;
        if (find_first_editable_cell_in_word(state, check_word_index, &dummy_x, &dummy_y)) {
            return check_word_index; // Found incomplete word
        }
    }
    
    return current_word; // All words complete, stay on current word
}

// Helper function to find previous editable cell with a letter in current word
int find_previous_editable_cell_with_letter(GameState state, int current_x, int current_y, int* out_x, int* out_y) {
    CrosswordWord* word = &state.crossword.current_level.words[state.crossword.current_word_index];
    
    // Search backwards from current position
    for (int i = 1; i < word->length; i++) {
        int check_x, check_y;
        
        if (word->direction == 0) {
            // Horizontal: move left
            check_x = current_x - i;
            check_y = current_y;
        } else {
            // Vertical: move up
            check_x = current_x;
            check_y = current_y - i;
        }
        
        // Check if position is within word bounds
        if ((word->direction == 0 && check_x >= word->start_x) ||
            (word->direction == 1 && check_y >= word->start_y)) {
            
            // Check if position is valid and part of word
            if (check_x >= 0 && check_y >= 0 && check_x < 9 && check_y < 9 &&
                state.crossword.current_level.word_mask[check_x][check_y]) {
                
                // Check if this position has a letter and is NOT green (deletable)
                char existing_letter = state.crossword.grid[check_x][check_y];
                if (existing_letter != '\0' && 
                    !(state.crossword.word_validated[check_x][check_y] && 
                      state.crossword.letter_states[check_x][check_y] == LETTER_CORRECT)) {
                    *out_x = check_x;
                    *out_y = check_y;
                    return 1; // Found deletable letter
                }
            }
        } else {
            // Reached start of word
            break;
        }
    }
    
    return 0; // No deletable letter found
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

// Helper function to complete word validation (called after animation or immediately)
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
        // Reset camera to center on new input row (only if not paused)
        if (!state->system.auto_center_paused) {
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

GameState view_switching_system(GameState state) {
    if (state.system.tab_pressed) {
        // Only allow tab switching between Wordle and Crossword modes
        if (state.current_view == VIEW_WORDLE) {
            state.current_view = VIEW_CROSSWORD;
        } else if (state.current_view == VIEW_CROSSWORD) {
            state.current_view = VIEW_WORDLE;
        }
        // Home screen and completion screen handle their own navigation
    }
    return state;
}

GameState crossword_input_system(GameState state) {
    if (state.current_view != VIEW_CROSSWORD) {
        return state;
    }
    
    // Direction toggle with Shift key
    if (state.system.shift_pressed) {
        state.crossword.cursor_direction = !state.crossword.cursor_direction;
    }
    
    // Left/Right arrow keys cycle through words
    if (IsKeyPressed(KEY_LEFT)) {
        // Previous word (cycle backwards)
        state.crossword.current_word_index--;
        if (state.crossword.current_word_index < 0) {
            state.crossword.current_word_index = state.crossword.current_level.word_count - 1;
        }
        
        // Set cursor to first editable position in selected word
        CrosswordWord* current_word = &state.crossword.current_level.words[state.crossword.current_word_index];
        state.crossword.cursor_direction = current_word->direction;
        
        // Use helper function to find first editable cell
        int found_x, found_y;
        if (find_first_editable_cell_in_word(state, state.crossword.current_word_index, &found_x, &found_y)) {
            state.crossword.cursor_x = found_x;
            state.crossword.cursor_y = found_y;
        } else {
            // If no editable cell found, position at start anyway
            state.crossword.cursor_x = current_word->start_x;
            state.crossword.cursor_y = current_word->start_y;
        }
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        // Next word (cycle forwards)
        state.crossword.current_word_index++;
        if (state.crossword.current_word_index >= state.crossword.current_level.word_count) {
            state.crossword.current_word_index = 0;
        }
        
        // Set cursor to first editable position in selected word
        CrosswordWord* current_word = &state.crossword.current_level.words[state.crossword.current_word_index];
        state.crossword.cursor_direction = current_word->direction;
        
        // Use helper function to find first editable cell
        int found_x, found_y;
        if (find_first_editable_cell_in_word(state, state.crossword.current_word_index, &found_x, &found_y)) {
            state.crossword.cursor_x = found_x;
            state.crossword.cursor_y = found_y;
        } else {
            // If no editable cell found, position at start anyway
            state.crossword.cursor_x = current_word->start_x;
            state.crossword.cursor_y = current_word->start_y;
        }
    }
    
    // Up/Down arrow keys navigate within current word only, skipping green cells
    CrosswordWord* current_word = &state.crossword.current_level.words[state.crossword.current_word_index];
    if (state.system.up_arrow_pressed) {
        if (current_word->direction == 1) { // vertical word
            // Search backwards for next non-green cell
            for (int i = 1; i <= current_word->length; i++) {
                int new_y = state.crossword.cursor_y - i;
                if (new_y >= current_word->start_y && 
                    state.crossword.current_level.word_mask[state.crossword.cursor_x][new_y]) {
                    
                    // Check if this position is NOT green (available for editing)
                    if (!(state.crossword.word_validated[state.crossword.cursor_x][new_y] && 
                          state.crossword.letter_states[state.crossword.cursor_x][new_y] == LETTER_CORRECT)) {
                        state.crossword.cursor_y = new_y;
                        break;
                    }
                } else {
                    break; // Reached start of word
                }
            }
        }
    }
    if (state.system.down_arrow_pressed) {
        if (current_word->direction == 1) { // vertical word
            // Search forwards for next non-green cell
            for (int i = 1; i <= current_word->length; i++) {
                int new_y = state.crossword.cursor_y + i;
                if (new_y < current_word->start_y + current_word->length && 
                    state.crossword.current_level.word_mask[state.crossword.cursor_x][new_y]) {
                    
                    // Check if this position is NOT green (available for editing)
                    if (!(state.crossword.word_validated[state.crossword.cursor_x][new_y] && 
                          state.crossword.letter_states[state.crossword.cursor_x][new_y] == LETTER_CORRECT)) {
                        state.crossword.cursor_y = new_y;
                        break;
                    }
                } else {
                    break; // Reached end of word
                }
            }
        }
    }
    
    // Letter placement with auto-advance (only in word cells)
    if (state.system.letter_pressed) {
        // Check if current position is a word cell
        if (state.crossword.current_level.word_mask[state.crossword.cursor_x][state.crossword.cursor_y]) {
            // Check if current position is locked (green letter)
            if (state.crossword.word_validated[state.crossword.cursor_x][state.crossword.cursor_y] && 
                state.crossword.letter_states[state.crossword.cursor_x][state.crossword.cursor_y] == LETTER_CORRECT) {
                // This cell is locked, prevent modification
                return state;
            }
            
            char letter = state.system.pressed_letter;
            int letter_index = letter - 'A';
            
            // Check if player has this letter token
            if (state.stats.letter_counts[letter_index] > 0) {
                // Remove any existing letter at cursor position and return it to bag
                char existing_letter = state.crossword.grid[state.crossword.cursor_x][state.crossword.cursor_y];
                if (existing_letter != '\0') {
                    int existing_index = existing_letter - 'A';
                    state.stats.letter_counts[existing_index]++;
                }
                
                // Place new letter and remove from bag
                state.crossword.grid[state.crossword.cursor_x][state.crossword.cursor_y] = letter;
                state.stats.letter_counts[letter_index]--;
                
                // Auto-advance cursor within current word, skipping green cells
                CrosswordWord* word = &state.crossword.current_level.words[state.crossword.current_word_index];
                int next_x = state.crossword.cursor_x;
                int next_y = state.crossword.cursor_y;
                
                // Keep advancing until we find a non-green cell or reach the end
                for (int i = 1; i < word->length; i++) {
                    if (word->direction == 0) {
                        // Horizontal: move right
                        next_x = state.crossword.cursor_x + i;
                        next_y = state.crossword.cursor_y;
                    } else {
                        // Vertical: move down
                        next_x = state.crossword.cursor_x;
                        next_y = state.crossword.cursor_y + i;
                    }
                    
                    // Check if position is within current word bounds and valid
                    if ((word->direction == 0 && next_x < word->start_x + word->length) ||
                        (word->direction == 1 && next_y < word->start_y + word->length)) {
                        
                        if (next_x < 9 && next_y < 9 && 
                            state.crossword.current_level.word_mask[next_x][next_y]) {
                            
                            // Check if this position is NOT green (available for editing)
                            if (!(state.crossword.word_validated[next_x][next_y] && 
                                  state.crossword.letter_states[next_x][next_y] == LETTER_CORRECT)) {
                                // Found an available position, move cursor here
                                state.crossword.cursor_x = next_x;
                                state.crossword.cursor_y = next_y;
                                break;
                            }
                        }
                    } else {
                        // Reached end of word, stop searching
                        break;
                    }
                }
            }
        }
    }
    
    // Enhanced backspace with direction-aware deletion
    if (state.system.backspace_pressed) {
        // Check if current position is a word cell
        if (state.crossword.current_level.word_mask[state.crossword.cursor_x][state.crossword.cursor_y]) {
            char existing_letter = state.crossword.grid[state.crossword.cursor_x][state.crossword.cursor_y];
            if (existing_letter != '\0') {
                // Check if current position is locked (green letter)
                if (state.crossword.word_validated[state.crossword.cursor_x][state.crossword.cursor_y] && 
                    state.crossword.letter_states[state.crossword.cursor_x][state.crossword.cursor_y] == LETTER_CORRECT) {
                    // This cell is locked, prevent deletion
                    return state;
                }
                
                // Remove letter at current position
                int existing_index = existing_letter - 'A';
                state.stats.letter_counts[existing_index]++;
                state.crossword.grid[state.crossword.cursor_x][state.crossword.cursor_y] = '\0';
            } else {
                // No letter at current position, search backwards for deletable letter
                int prev_x, prev_y;
                if (find_previous_editable_cell_with_letter(state, state.crossword.cursor_x, state.crossword.cursor_y, &prev_x, &prev_y)) {
                    // Found a deletable letter, remove it and move cursor there
                    char prev_letter = state.crossword.grid[prev_x][prev_y];
                    int prev_index = prev_letter - 'A';
                    state.stats.letter_counts[prev_index]++;
                    state.crossword.grid[prev_x][prev_y] = '\0';
                    
                    // Move cursor to the deleted position
                    state.crossword.cursor_x = prev_x;
                    state.crossword.cursor_y = prev_y;
                }
                // If no deletable letter found, cursor stays at current position
            }
        }
    }
    
    // Enter key to validate current word
    if (state.system.enter_pressed) {
        state.crossword.should_validate = 1;
    }
    
    return state;
}

CrosswordLevel get_crossword_level(int level) {
    // Generate crossword based on level
    int word_count = 3;  // Start with 3 words for level 1
    
    // Increase word count for higher levels (up to 10 words max)
    if (level > 1) {
        word_count = 3 + (level - 1);
        if (word_count > 10) {
            word_count = 10;
        }
    }
    
    // Generate a crossword with the specified word count
    CrosswordLevel crossword_level = generate_crossword(word_count, 9, 9);
    crossword_level.level = level;
    
    return crossword_level;
}

GameState crossword_word_validation_system(GameState state) {
    if (state.current_view != VIEW_CROSSWORD || !state.crossword.should_validate) {
        return state;
    }
    
    // Reset validation flag
    state.crossword.should_validate = 0;
    
    // Extract current word based on cursor position and direction
    char current_word[WORD_LENGTH + 1] = {0};
    char solution_word[WORD_LENGTH + 1] = {0};
    int word_start_x = state.crossword.cursor_x;
    int word_start_y = state.crossword.cursor_y;
    
    // Find the start of the current word
    if (state.crossword.cursor_direction == 0) {
        // Horizontal: find leftmost cell of current word
        while (word_start_x > 0 && 
               state.crossword.current_level.word_mask[word_start_x - 1][word_start_y]) {
            word_start_x--;
        }
    } else {
        // Vertical: find topmost cell of current word
        while (word_start_y > 0 && 
               state.crossword.current_level.word_mask[word_start_x][word_start_y - 1]) {
            word_start_y--;
        }
    }
    
    // Extract the word and solution
    int word_length = 0;
    for (int i = 0; i < WORD_LENGTH; i++) {
        int check_x = word_start_x + (state.crossword.cursor_direction == 0 ? i : 0);
        int check_y = word_start_y + (state.crossword.cursor_direction == 1 ? i : 0);
        
        // Check if position is valid and part of a word
        if (check_x < 9 && check_y < 9 && 
            state.crossword.current_level.word_mask[check_x][check_y]) {
            current_word[i] = state.crossword.grid[check_x][check_y];
            solution_word[i] = state.crossword.current_level.solution[check_x][check_y];
            if (current_word[i] != '\0') {
                word_length++;
            }
        } else {
            break;
        }
    }
    
    // Only validate if word is complete (5 letters)
    if (word_length == WORD_LENGTH) {
        // Check if all letters are filled
        int all_filled = 1;
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (current_word[i] == '\0') {
                all_filled = 0;
                break;
            }
        }
        
        if (all_filled) {
            // Validate word using existing Wordle logic
            int word_correct = check_word_match(current_word, solution_word);
            
            // Calculate and store letter states for visual feedback
            for (int i = 0; i < WORD_LENGTH; i++) {
                int pos_x = word_start_x + (state.crossword.cursor_direction == 0 ? i : 0);
                int pos_y = word_start_y + (state.crossword.cursor_direction == 1 ? i : 0);
                
                if (pos_x < 9 && pos_y < 9) {
                    LetterState letter_state = calculate_letter_state(current_word[i], i, solution_word);
                    state.crossword.letter_states[pos_x][pos_y] = letter_state;
                    state.crossword.word_validated[pos_x][pos_y] = 1;
                }
            }
            
            // Trigger letter ease animation for all validated words
            trigger_letter_ease(&state);
            
            if (word_correct) {
                // Word is correct - mark completion and provide feedback
                printf("Word '%s' is correct!\n", current_word);
                
                // Check if entire crossword is completed
                int all_words_correct = 1;
                for (int x = 0; x < 9; x++) {
                    for (int y = 0; y < 9; y++) {
                        if (state.crossword.current_level.word_mask[x][y]) {
                            if (state.crossword.grid[x][y] != state.crossword.current_level.solution[x][y]) {
                                all_words_correct = 0;
                                break;
                            }
                        }
                    }
                    if (!all_words_correct) break;
                }
                
                if (all_words_correct) {
                    printf("Congratulations! Crossword puzzle completed!\n");
                    state.crossword.puzzle_completed = 1;
                    state.current_view = VIEW_CROSSWORD_COMPLETE;
                } else {
                    // Find next incomplete word and position cursor there
                    int next_word_index = find_next_incomplete_word(state);
                    int next_x, next_y;
                    
                    if (find_first_editable_cell_in_word(state, next_word_index, &next_x, &next_y)) {
                        state.crossword.current_word_index = next_word_index;
                        state.crossword.cursor_x = next_x;
                        state.crossword.cursor_y = next_y;
                        state.crossword.cursor_direction = state.crossword.current_level.words[next_word_index].direction;
                    }
                }
            } else {
                // Word is incorrect - provide letter-by-letter feedback
                printf("Word '%s' is incorrect. Expected '%s'\n", current_word, solution_word);
                
                // Position cursor at first editable cell of current word for retry
                int retry_x, retry_y;
                if (find_first_editable_cell_in_word(state, state.crossword.current_word_index, &retry_x, &retry_y)) {
                    state.crossword.cursor_x = retry_x;
                    state.crossword.cursor_y = retry_y;
                }
            }
        }
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

GameState home_screen_input_system(GameState state) {
    if (state.current_view != VIEW_HOME_SCREEN) {
        return state;
    }
    
    // Enter key starts the game
    if (state.system.enter_pressed) {
        // Initialize the first Wordle game
        strcpy(state.core.target_word, get_random_word());
        state.core.current_level = 1;
        state.core.guesses_this_level = 0;
        state.core.total_lifetime_guesses = 0;
        state.core.play_state = GAME_STATE_INPUT;
        state.core.level_complete = 0;
        
        // Reset input state
        memset(state.input.current_word, 0, sizeof(state.input.current_word));
        state.input.current_letter_pos = 0;
        state.input.word_complete = 0;
        state.input.should_submit = 0;
        
        // Reset history
        state.history.level_guess_count = 0;
        
        // Switch to Wordle view
        state.current_view = VIEW_WORDLE;
        
        // Reset camera
        state.system.camera_offset_y = 0.0f;
        state.system.target_camera_offset_y = 0.0f;
        state.system.user_has_scrolled = 0;
        state.system.auto_center_paused = 0;
    }
    
    return state;
}

GameState crossword_completion_input_system(GameState state) {
    if (state.current_view != VIEW_CROSSWORD_COMPLETE) {
        return state;
    }
    
    // Space key returns to home screen
    if (state.system.space_pressed) {
        // Reset crossword state for new game
        for (int x = 0; x < 9; x++) {
            for (int y = 0; y < 9; y++) {
                state.crossword.grid[x][y] = '\0';
                state.crossword.letter_states[x][y] = LETTER_UNKNOWN;
                state.crossword.word_validated[x][y] = 0;
            }
        }
        // Reset to first word
        state.crossword.current_word_index = 0;
        state.crossword.cursor_x = state.crossword.current_level.words[0].start_x;
        state.crossword.cursor_y = state.crossword.current_level.words[0].start_y;
        state.crossword.cursor_direction = state.crossword.current_level.words[0].direction;
        state.crossword.should_validate = 0;
        state.crossword.puzzle_completed = 0;
        
        // Return to home screen
        state.current_view = VIEW_HOME_SCREEN;
    }
    
    // Tab key returns to Wordle mode to continue playing
    if (state.system.tab_pressed) {
        state.current_view = VIEW_WORDLE;
    }
    
    return state;
}

// ============= ANIMATION SYSTEM FUNCTIONS =============

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

void spawn_particles(GameState* state, Vector2 position, Color color, int count) {
    if (!state->settings.animations_enabled) {
        return;
    }
    
    for (int i = 0; i < count && state->ui.particle_count < 50; i++) {
        int index = state->ui.particle_count;
        state->ui.particles[index] = position;
        
        // Random velocity
        float angle = (float)GetRandomValue(0, 359) * 3.14159265359f / 180.0f;
        float speed = 50.0f + (float)GetRandomValue(0, 99);
        state->ui.particle_velocities[index] = (Vector2){
            cos(angle) * speed,
            sin(angle) * speed - 100.0f // Initial upward bias
        };
        
        state->ui.particle_lifetimes[index] = PARTICLE_LIFETIME;
        state->ui.particle_colors[index] = color;
        state->ui.particle_count++;
    }
}

void update_particles(GameState* state, float frame_time) {
    for (int i = 0; i < state->ui.particle_count; i++) {
        state->ui.particle_lifetimes[i] -= frame_time;
        
        if (state->ui.particle_lifetimes[i] <= 0.0f) {
            // Remove particle by swapping with last particle
            state->ui.particle_count--;
            if (i < state->ui.particle_count) {
                state->ui.particles[i] = state->ui.particles[state->ui.particle_count];
                state->ui.particle_velocities[i] = state->ui.particle_velocities[state->ui.particle_count];
                state->ui.particle_lifetimes[i] = state->ui.particle_lifetimes[state->ui.particle_count];
                state->ui.particle_colors[i] = state->ui.particle_colors[state->ui.particle_count];
                i--; // Check this particle again
            }
            continue;
        }
        
        // Update position
        state->ui.particles[i].x += state->ui.particle_velocities[i].x * frame_time;
        state->ui.particles[i].y += state->ui.particle_velocities[i].y * frame_time;
        
        // Apply gravity
        state->ui.particle_velocities[i].y += PARTICLE_GRAVITY * frame_time;
        
        // Fade particle
        float life_ratio = state->ui.particle_lifetimes[i] / PARTICLE_LIFETIME;
        state->ui.particle_colors[i].a = (unsigned char)(255 * life_ratio);
    }
}