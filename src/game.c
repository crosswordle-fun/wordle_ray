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
        float camera_lerp_speed = 8.0f;
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
        // Award a random letter token from the solved word (only happens once per level)
        int random_letter_index = rand() % WORD_LENGTH;
        char awarded_letter = state.core.target_word[random_letter_index];
        int letter_array_index = awarded_letter - 'A';  // Convert A-Z to 0-25
        state.stats.letter_counts[letter_array_index]++;
        
        // Update statistics (only happens once per level completion)
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
        
        // Set cursor to start of selected word
        CrosswordWord* current_word = &state.crossword.current_level.words[state.crossword.current_word_index];
        state.crossword.cursor_x = current_word->start_x;
        state.crossword.cursor_y = current_word->start_y;
        state.crossword.cursor_direction = current_word->direction;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        // Next word (cycle forwards)
        state.crossword.current_word_index++;
        if (state.crossword.current_word_index >= state.crossword.current_level.word_count) {
            state.crossword.current_word_index = 0;
        }
        
        // Set cursor to start of selected word
        CrosswordWord* current_word = &state.crossword.current_level.words[state.crossword.current_word_index];
        state.crossword.cursor_x = current_word->start_x;
        state.crossword.cursor_y = current_word->start_y;
        state.crossword.cursor_direction = current_word->direction;
    }
    
    // Up/Down arrow keys navigate within current word only
    CrosswordWord* current_word = &state.crossword.current_level.words[state.crossword.current_word_index];
    if (state.system.up_arrow_pressed) {
        if (current_word->direction == 1) { // vertical word
            int new_y = state.crossword.cursor_y - 1;
            if (new_y >= current_word->start_y && 
                state.crossword.current_level.word_mask[state.crossword.cursor_x][new_y]) {
                state.crossword.cursor_y = new_y;
            }
        }
    }
    if (state.system.down_arrow_pressed) {
        if (current_word->direction == 1) { // vertical word
            int new_y = state.crossword.cursor_y + 1;
            if (new_y < current_word->start_y + current_word->length && 
                state.crossword.current_level.word_mask[state.crossword.cursor_x][new_y]) {
                state.crossword.cursor_y = new_y;
            }
        }
    }
    
    // Letter placement with auto-advance (only in word cells)
    if (state.system.letter_pressed) {
        // Check if current position is a word cell
        if (state.crossword.current_level.word_mask[state.crossword.cursor_x][state.crossword.cursor_y]) {
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
                
                // Auto-advance cursor within current word
                CrosswordWord* word = &state.crossword.current_level.words[state.crossword.current_word_index];
                int next_x = state.crossword.cursor_x;
                int next_y = state.crossword.cursor_y;
                
                if (word->direction == 0) {
                    // Horizontal: move right
                    next_x++;
                } else {
                    // Vertical: move down
                    next_y++;
                }
                
                // Check if next position is within current word bounds
                if ((word->direction == 0 && next_x < word->start_x + word->length) ||
                    (word->direction == 1 && next_y < word->start_y + word->length)) {
                    
                    if (next_x < 9 && next_y < 9 && 
                        state.crossword.current_level.word_mask[next_x][next_y]) {
                        state.crossword.cursor_x = next_x;
                        state.crossword.cursor_y = next_y;
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
                // Remove letter at current position
                int existing_index = existing_letter - 'A';
                state.stats.letter_counts[existing_index]++;
                state.crossword.grid[state.crossword.cursor_x][state.crossword.cursor_y] = '\0';
            } else {
                // No letter at current position, move backwards within current word and delete
                CrosswordWord* word = &state.crossword.current_level.words[state.crossword.current_word_index];
                int prev_x = state.crossword.cursor_x;
                int prev_y = state.crossword.cursor_y;
                
                if (word->direction == 0) {
                    // Horizontal: move left
                    prev_x--;
                } else {
                    // Vertical: move up
                    prev_y--;
                }
                
                // Check if previous position is within current word bounds
                if ((word->direction == 0 && prev_x >= word->start_x) ||
                    (word->direction == 1 && prev_y >= word->start_y)) {
                    
                    if (prev_x >= 0 && prev_y >= 0 && 
                        state.crossword.current_level.word_mask[prev_x][prev_y]) {
                        
                        char prev_letter = state.crossword.grid[prev_x][prev_y];
                        if (prev_letter != '\0') {
                            // Remove letter at previous position
                            int prev_index = prev_letter - 'A';
                            state.stats.letter_counts[prev_index]++;
                            state.crossword.grid[prev_x][prev_y] = '\0';
                            
                            // Move cursor back
                            state.crossword.cursor_x = prev_x;
                            state.crossword.cursor_y = prev_y;
                        }
                    }
                }
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
    CrosswordLevel crossword_level = {0};
    crossword_level.level = level;
    
    // Initialize all cells as blocked (0) and empty
    for (int x = 0; x < 9; x++) {
        for (int y = 0; y < 9; y++) {
            crossword_level.word_mask[x][y] = 0;
            crossword_level.solution[x][y] = '\0';
        }
    }
    
    if (level == 1) {
        // Level 1: SWORD (horizontal row 0, cols 1-5), STARK (vertical col 1, rows 0-4), CROSS (horizontal row 3, cols 0-4)
        
        // Define words
        crossword_level.word_count = 3;
        
        // Word 1: SWORD (horizontal, row 0, cols 1-5)
        crossword_level.words[0].start_x = 1;
        crossword_level.words[0].start_y = 0;
        crossword_level.words[0].direction = 0; // horizontal
        crossword_level.words[0].length = 5;
        
        const char* sword = "SWORD";
        for (int i = 0; i < 5; i++) {
            crossword_level.solution[1 + i][0] = sword[i];
            crossword_level.word_mask[1 + i][0] = 1;
        }
        
        // Word 2: STARK (vertical, col 1, rows 0-4) 
        crossword_level.words[1].start_x = 1;
        crossword_level.words[1].start_y = 0;
        crossword_level.words[1].direction = 1; // vertical
        crossword_level.words[1].length = 5;
        
        const char* stark = "STARK";
        for (int i = 0; i < 5; i++) {
            crossword_level.solution[1][i] = stark[i];
            crossword_level.word_mask[1][i] = 1;
        }
        
        // Word 3: CROSS (horizontal, row 3, cols 0-4)
        crossword_level.words[2].start_x = 0;
        crossword_level.words[2].start_y = 3;
        crossword_level.words[2].direction = 0; // horizontal
        crossword_level.words[2].length = 5;
        
        const char* cross = "CROSS";
        for (int i = 0; i < 5; i++) {
            crossword_level.solution[i][3] = cross[i];
            crossword_level.word_mask[i][3] = 1;
        }
    }
    
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
                }
            } else {
                // Word is incorrect - provide letter-by-letter feedback
                printf("Word '%s' is incorrect. Expected '%s'\n", current_word, solution_word);
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