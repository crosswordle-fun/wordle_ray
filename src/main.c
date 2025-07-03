#include "raylib.h"
#include "words.h"
#include "game.h"
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define CELL_SIZE 60
#define CELL_SPACING 10
#define BOARD_START_X 200
#define BOARD_START_Y 100

Color get_color_for_letter_state(LetterState state) {
    if (state == LETTER_CORRECT) return GREEN;
    if (state == LETTER_WRONG_POS) return YELLOW;
    if (state == LETTER_NOT_IN_WORD) return GRAY;
    return WHITE;
}

void board_render_system(GameWorld world) {
    for (int row = 0; row < MAX_ATTEMPTS; row++) {
        for (int col = 0; col < WORD_LENGTH; col++) {
            int cell_x = BOARD_START_X + col * (CELL_SIZE + CELL_SPACING);
            int cell_y = BOARD_START_Y + row * (CELL_SIZE + CELL_SPACING);
            
            Color cell_color = WHITE;
            if (row < world.game_progress.attempt_count) {
                cell_color = get_color_for_letter_state(world.game_progress.letter_states[row][col]);
            } else if (row == world.game_progress.attempt_count && col < world.word_editing.current_letter_pos) {
                cell_color = LIGHTGRAY;
            }
            
            DrawRectangle(cell_x, cell_y, CELL_SIZE, CELL_SIZE, cell_color);
            DrawRectangleLines(cell_x, cell_y, CELL_SIZE, CELL_SIZE, BLACK);
            
            char letter_to_display = '\0';
            if (row < world.game_progress.attempt_count) {
                letter_to_display = world.game_progress.all_guesses[row][col];
            } else if (row == world.game_progress.attempt_count && col < world.word_editing.current_letter_pos) {
                letter_to_display = world.word_editing.current_word[col];
            }
            
            if (letter_to_display != '\0') {
                char letter_string[2] = {letter_to_display, '\0'};
                int text_x = cell_x + CELL_SIZE/2 - 10;
                int text_y = cell_y + CELL_SIZE/2 - 10;
                DrawText(letter_string, text_x, text_y, 20, BLACK);
            }
        }
    }
}

void ui_render_system(GameWorld world) {
    DrawText("WORDLE", SCREEN_WIDTH/2 - 60, 20, 40, WHITE);
    
    if (world.game_control.is_game_over) {
        const char* end_message = world.game_control.player_won ? "You Won!" : "Game Over!";
        DrawText(end_message, SCREEN_WIDTH/2 - 80, SCREEN_HEIGHT - 150, 30, WHITE);
        
        if (!world.game_control.player_won) {
            char target_message[50];
            sprintf(target_message, "The word was: %s", world.game_progress.target_word);
            DrawText(target_message, SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT - 100, 20, WHITE);
        }
        
        DrawText("Press SPACE to restart", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT - 50, 20, WHITE);
    } else {
        DrawText("Enter a 5-letter word and press ENTER", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT - 50, 16, WHITE);
    }
}

void render_system(GameWorld world) {
    ClearBackground(DARKGRAY);
    board_render_system(world);
    ui_render_system(world);
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wordle");
    SetTargetFPS(60);
    
    GameWorld world = create_game_world(get_random_word());
    
    while (!WindowShouldClose()) {
        world = input_system(world);
        world = word_editing_system(world);
        world = word_validation_system(world);
        world = game_state_system(world);
        world = game_reset_system(world, get_random_word());
        
        BeginDrawing();
        render_system(world);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}