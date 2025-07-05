#include "../systems.h"

GameState view_switching_system(GameState state) {
    if (state.system.tab_pressed) {
        // Only allow tab switching between Wordle and Crossword modes
        if (state.current_view == VIEW_WORDLE || state.current_view == VIEW_CROSSWORD) {
            // Allow interrupting transitions with new transitions (handle rapid tab presses)
            if (!state.ui.transitioning_view || 
                (state.ui.transitioning_view && state.ui.view_transition_timer > 0.1f)) {
                
                // Initialize transition state
                state.ui.transitioning_view = 1;
                state.ui.view_transition_timer = 0.0f;
                state.ui.previous_view = state.current_view;
                
                // Set transition direction and target view
                if (state.current_view == VIEW_WORDLE) {
                    state.current_view = VIEW_CROSSWORD;
                    state.ui.transition_direction = 0; // left-to-right (Wordle slides left, Crossword slides in from right)
                } else if (state.current_view == VIEW_CROSSWORD) {
                    state.current_view = VIEW_WORDLE;
                    state.ui.transition_direction = 1; // right-to-left (Crossword slides left, Wordle slides in from right)
                }
            }
        }
        // Home screen and completion screen handle their own navigation
    }
    return state;
}