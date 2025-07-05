#include "../systems.h"

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
    
    state.system.frame_time = GetFrameTime();
    
    // Handle debug key inputs
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