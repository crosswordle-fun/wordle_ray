#include "../systems.h"

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