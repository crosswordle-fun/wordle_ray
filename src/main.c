#include "systems.h"

int main(void) {
    InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "Wordle");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(400, 300);
    
    // Set random seed for consistent word selection across runs
    SetRandomSeed(12345);
    
    GameState state = create_game_state("");
    
    while (!WindowShouldClose()) {
        state = input_system(state);
        state = camera_scrolling_system(state);
        state = view_switching_system(state);
        
        if (state.current_view == VIEW_HOME_SCREEN) {
            state = home_screen_input_system(state);
        } else if (state.current_view == VIEW_WORDLE) {
            state = word_editing_system(state);
            state = word_validation_system(state);
            state = result_display_system(state);
            state = level_progression_system(state);
            
            // Handle new level setup
            if (state.core.play_state == GAME_STATE_INPUT_READY) {
                strcpy(state.core.target_word, get_random_word());
                state = new_level_system(state);
            }
        } else if (state.current_view == VIEW_CROSSWORD) {
            state = crossword_input_system(state);
            state = crossword_word_validation_system(state);
        } else if (state.current_view == VIEW_CROSSWORD_COMPLETE) {
            state = crossword_completion_input_system(state);
        }
        
        // Update animations for all views
        state = animation_update_system(state);
        
        BeginDrawing();
        render_system(state);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}