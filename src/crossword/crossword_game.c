#include "../systems.h"

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
    // Generate crossword based on level - start with 2 words, then 3, 4, 5, etc.
    int word_count = 1 + level;  // Level 1 = 2 words, Level 2 = 3 words, etc.
    
    // Cap at 10 words maximum
    if (word_count > 10) {
        word_count = 10;
    }
    
    // Cap at 3 words for now (matches available levels)
    if (word_count > 3) {
        word_count = 3;
    }
    
    // Use the generate_crossword function to create the level
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
                    printf("Congratulations! Crossword level %d completed!\n", state.crossword.current_level.level);
                    
                    // Advance to next level
                    int next_level = state.crossword.current_level.level + 1;
                    state.crossword.current_level = get_crossword_level(next_level);
                    
                    // Reset grid for new level
                    for (int x = 0; x < 9; x++) {
                        for (int y = 0; y < 9; y++) {
                            state.crossword.grid[x][y] = '\0';
                            state.crossword.letter_states[x][y] = LETTER_UNKNOWN;
                            state.crossword.word_validated[x][y] = 0;
                        }
                    }
                    
                    // Reset cursor to first word of new level
                    state.crossword.current_word_index = 0;
                    state.crossword.cursor_x = state.crossword.current_level.words[0].start_x;
                    state.crossword.cursor_y = state.crossword.current_level.words[0].start_y;
                    state.crossword.cursor_direction = state.crossword.current_level.words[0].direction;
                    state.crossword.should_validate = 0;
                    state.crossword.puzzle_completed = 0;
                    
                    printf("Starting crossword level %d with %d words!\n", next_level, state.crossword.current_level.word_count);
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