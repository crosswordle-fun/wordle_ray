#include "raylib.h"
#include "words.h"
#include "game.h"
#include "constants.h"
#include "render.h"

int main(void) {
    InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "Wordle");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(400, 300);
    
    GameState state = create_game_state(get_random_word());
    
    while (!WindowShouldClose()) {
        state = input_system(state);
        state = word_editing_system(state);
        state = word_validation_system(state);
        state = game_state_system(state);
        state = game_reset_system(state, get_random_word());
        
        BeginDrawing();
        render_system(state);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}