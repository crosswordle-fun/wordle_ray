#include "../systems.h"

GameState view_switching_system(GameState state) {
    if (state.system.tab_pressed) {
        // Only allow tab switching between Wordle and Crossword modes
        if (state.current_view == VIEW_WORDLE || state.current_view == VIEW_CROSSWORD) {
            // Only allow tab switching when no transition is in progress
            if (!state.ui.transitioning_view) {
                
                // Initialize transition state
                state.ui.transitioning_view = 1;
                state.ui.view_transition_timer = 0.0f;
                state.ui.previous_view = state.current_view;
                
                // Set transition direction and target view based on tab positions
                if (state.current_view == VIEW_WORDLE) {
                    state.current_view = VIEW_CROSSWORD;
                    state.ui.transition_direction = 1; // right-to-left (WORDLE→CROSS: Wordle slides right, Cross slides in from left)
                } else if (state.current_view == VIEW_CROSSWORD) {
                    state.current_view = VIEW_WORDLE;
                    state.ui.transition_direction = 0; // left-to-right (CROSS→WORDLE: Cross slides left, Wordle slides in from right)
                }
            }
        }
        // Home screen and completion screen handle their own navigation
    }
    return state;
}