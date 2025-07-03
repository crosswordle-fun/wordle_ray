#include "raylib.h"
#include "words.h"
#include "game.h"
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define CELL_SIZE 70
#define CELL_SPACING 8
#define BOARD_WIDTH (WORD_LENGTH * CELL_SIZE + (WORD_LENGTH - 1) * CELL_SPACING)
#define BOARD_HEIGHT (MAX_ATTEMPTS * CELL_SIZE + (MAX_ATTEMPTS - 1) * CELL_SPACING)
#define BOARD_START_X ((SCREEN_WIDTH - BOARD_WIDTH) / 2)
#define BOARD_START_Y 120

#define WORDLE_GREEN (Color){106, 170, 100, 255}
#define WORDLE_YELLOW (Color){201, 180, 88, 255}
#define WORDLE_GRAY (Color){120, 124, 126, 255}
#define WORDLE_DARK_GRAY (Color){58, 58, 60, 255}
#define WORDLE_WHITE (Color){255, 255, 255, 255}
#define WORDLE_BLACK (Color){0, 0, 0, 255}
#define WORDLE_BG (Color){18, 18, 19, 255}
#define WORDLE_BORDER (Color){58, 58, 60, 255}
#define WORDLE_INPUT (Color){86, 87, 88, 255}

Color get_color_for_letter_state(LetterState state) {
    if (state == LETTER_CORRECT) return WORDLE_GREEN;
    if (state == LETTER_WRONG_POS) return WORDLE_YELLOW;
    if (state == LETTER_NOT_IN_WORD) return WORDLE_GRAY;
    return WORDLE_WHITE;
}

void board_render_system(GameState state) {
    for (int row = 0; row < MAX_ATTEMPTS; row++) {
        for (int col = 0; col < WORD_LENGTH; col++) {
            int cell_x = BOARD_START_X + col * (CELL_SIZE + CELL_SPACING);
            int cell_y = BOARD_START_Y + row * (CELL_SIZE + CELL_SPACING);
            
            Color cell_color = WORDLE_WHITE;
            Color border_color = WORDLE_BORDER;
            int border_width = 2;
            
            if (row < state.history.attempt_count) {
                cell_color = get_color_for_letter_state(state.history.letter_states[row][col]);
                border_color = cell_color;
                border_width = 0;
            } else if (row == state.history.attempt_count && col < state.input.current_letter_pos) {
                cell_color = WORDLE_INPUT;
                border_color = WORDLE_DARK_GRAY;
                border_width = 2;
            }
            
            DrawRectangle(cell_x, cell_y, CELL_SIZE, CELL_SIZE, cell_color);
            
            if (border_width > 0) {
                DrawRectangleLinesEx((Rectangle){cell_x, cell_y, CELL_SIZE, CELL_SIZE}, border_width, border_color);
            }
            
            char letter_to_display = '\0';
            if (row < state.history.attempt_count) {
                letter_to_display = state.history.all_guesses[row][col];
            } else if (row == state.history.attempt_count && col < state.input.current_letter_pos) {
                letter_to_display = state.input.current_word[col];
            }
            
            if (letter_to_display != '\0') {
                char letter_string[2] = {letter_to_display, '\0'};
                int font_size = 32;
                int text_width = MeasureText(letter_string, font_size);
                int text_x = cell_x + (CELL_SIZE - text_width) / 2;
                int text_y = cell_y + (CELL_SIZE - font_size) / 2;
                
                Color text_color = (row < state.history.attempt_count) ? WORDLE_WHITE : WORDLE_BLACK;
                DrawText(letter_string, text_x, text_y, font_size, text_color);
            }
        }
    }
}

void ui_render_system(GameState state) {
    const char* title = "WORDLE";
    int title_font_size = 48;
    int title_width = MeasureText(title, title_font_size);
    int title_x = (SCREEN_WIDTH - title_width) / 2;
    int title_y = 30;
    
    DrawText(title, title_x, title_y, title_font_size, WORDLE_WHITE);
    
    DrawLine(50, 90, SCREEN_WIDTH - 50, 90, WORDLE_BORDER);
    
    if (state.core.is_game_over) {
        const char* end_message = state.core.player_won ? "🎉 You Won! 🎉" : "😔 Game Over";
        int end_font_size = 36;
        int end_width = MeasureText(end_message, end_font_size);
        int end_x = (SCREEN_WIDTH - end_width) / 2;
        int end_y = BOARD_START_Y + BOARD_HEIGHT + 40;
        
        Color end_color = state.core.player_won ? WORDLE_GREEN : WORDLE_GRAY;
        DrawText(end_message, end_x, end_y, end_font_size, end_color);
        
        if (!state.core.player_won) {
            char target_message[50];
            sprintf(target_message, "The word was: %s", state.core.target_word);
            int target_font_size = 24;
            int target_width = MeasureText(target_message, target_font_size);
            int target_x = (SCREEN_WIDTH - target_width) / 2;
            int target_y = end_y + 50;
            
            DrawText(target_message, target_x, target_y, target_font_size, WORDLE_WHITE);
        }
        
        const char* restart_message = "Press SPACE to play again";
        int restart_font_size = 20;
        int restart_width = MeasureText(restart_message, restart_font_size);
        int restart_x = (SCREEN_WIDTH - restart_width) / 2;
        int restart_y = SCREEN_HEIGHT - 60;
        
        DrawText(restart_message, restart_x, restart_y, restart_font_size, WORDLE_BORDER);
        
        char stats_message[100];
        sprintf(stats_message, "Games: %d | Wins: %d | Streak: %d | Win Rate: %.0f%%", 
                state.stats.games_played, state.stats.games_won, 
                state.stats.current_streak, state.stats.win_percentage);
        int stats_font_size = 16;
        int stats_width = MeasureText(stats_message, stats_font_size);
        int stats_x = (SCREEN_WIDTH - stats_width) / 2;
        int stats_y = restart_y + 30;
        
        DrawText(stats_message, stats_x, stats_y, stats_font_size, WORDLE_BORDER);
        
    } else {
        const char* instruction = "Type a 5-letter word and press ENTER";
        int instruction_font_size = 18;
        int instruction_width = MeasureText(instruction, instruction_font_size);
        int instruction_x = (SCREEN_WIDTH - instruction_width) / 2;
        int instruction_y = SCREEN_HEIGHT - 80;
        
        DrawText(instruction, instruction_x, instruction_y, instruction_font_size, WORDLE_BORDER);
        
        char attempt_message[20];
        sprintf(attempt_message, "Attempt %d of %d", state.history.attempt_count + 1, MAX_ATTEMPTS);
        int attempt_font_size = 16;
        int attempt_width = MeasureText(attempt_message, attempt_font_size);
        int attempt_x = (SCREEN_WIDTH - attempt_width) / 2;
        int attempt_y = instruction_y + 25;
        
        DrawText(attempt_message, attempt_x, attempt_y, attempt_font_size, WORDLE_GRAY);
    }
}

void render_system(GameState state) {
    ClearBackground(WORDLE_BG);
    board_render_system(state);
    ui_render_system(state);
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wordle");
    SetTargetFPS(60);
    
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