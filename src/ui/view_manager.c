#include "../systems.h"

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