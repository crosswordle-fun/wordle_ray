#include "../systems.h"

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
    
    return current_word; // No incomplete word found, return current
}

// Helper function to find previous editable cell with a letter (for smart backspace)
int find_previous_editable_cell_with_letter(GameState state, int current_x, int current_y, int* out_x, int* out_y) {
    CrosswordWord* word = &state.crossword.current_level.words[state.crossword.current_word_index];
    
    // Search backwards through the current word
    for (int i = 1; i < word->length; i++) {
        int prev_x, prev_y;
        
        if (word->direction == 0) {
            // Horizontal: move left
            prev_x = current_x - i;
            prev_y = current_y;
        } else {
            // Vertical: move up
            prev_x = current_x;
            prev_y = current_y - i;
        }
        
        // Check if position is within word bounds
        if ((word->direction == 0 && prev_x >= word->start_x) ||
            (word->direction == 1 && prev_y >= word->start_y)) {
            
            // Check if position has a letter and is not green (locked)
            if (state.crossword.current_level.word_mask[prev_x][prev_y] &&
                state.crossword.grid[prev_x][prev_y] != '\0') {
                
                // Check if this position is NOT green (deletable)
                if (!(state.crossword.word_validated[prev_x][prev_y] && 
                      state.crossword.letter_states[prev_x][prev_y] == LETTER_CORRECT)) {
                    *out_x = prev_x;
                    *out_y = prev_y;
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