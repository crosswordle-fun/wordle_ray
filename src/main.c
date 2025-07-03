#include "raylib.h"
#include "words.h"
#include "game.h"
#include <stdio.h>

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600
#define MIN_CELL_SIZE 50
#define MAX_CELL_SIZE 100
#define CELL_SPACING_RATIO 0.12f
#define BOARD_MARGIN_TOP 120
#define BOARD_MARGIN_BOTTOM 150

typedef struct {
    int screen_width;
    int screen_height;
    int cell_size;
    int cell_spacing;
    int board_width;
    int board_height;
    int board_start_x;
    int board_start_y;
} LayoutConfig;

LayoutConfig calculate_layout() {
    LayoutConfig layout = {0};
    
    layout.screen_width = GetScreenWidth();
    layout.screen_height = GetScreenHeight();
    
    int available_width = layout.screen_width - 100;
    int available_height = layout.screen_height - BOARD_MARGIN_TOP - BOARD_MARGIN_BOTTOM;
    
    int max_cell_width = available_width / (WORD_LENGTH + (WORD_LENGTH - 1) * CELL_SPACING_RATIO);
    int max_cell_height = available_height / (MAX_ATTEMPTS + (MAX_ATTEMPTS - 1) * CELL_SPACING_RATIO);
    
    layout.cell_size = (max_cell_width < max_cell_height) ? max_cell_width : max_cell_height;
    
    if (layout.cell_size < MIN_CELL_SIZE) layout.cell_size = MIN_CELL_SIZE;
    if (layout.cell_size > MAX_CELL_SIZE) layout.cell_size = MAX_CELL_SIZE;
    
    layout.cell_spacing = (int)(layout.cell_size * CELL_SPACING_RATIO);
    
    layout.board_width = WORD_LENGTH * layout.cell_size + (WORD_LENGTH - 1) * layout.cell_spacing;
    layout.board_height = MAX_ATTEMPTS * layout.cell_size + (MAX_ATTEMPTS - 1) * layout.cell_spacing;
    
    layout.board_start_x = (layout.screen_width - layout.board_width) / 2;
    layout.board_start_y = BOARD_MARGIN_TOP;
    
    return layout;
}

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
    LayoutConfig layout = calculate_layout();
    
    for (int row = 0; row < MAX_ATTEMPTS; row++) {
        for (int col = 0; col < WORD_LENGTH; col++) {
            int cell_x = layout.board_start_x + col * (layout.cell_size + layout.cell_spacing);
            int cell_y = layout.board_start_y + row * (layout.cell_size + layout.cell_spacing);
            
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
            
            DrawRectangle(cell_x, cell_y, layout.cell_size, layout.cell_size, cell_color);
            
            if (border_width > 0) {
                DrawRectangleLinesEx((Rectangle){cell_x, cell_y, layout.cell_size, layout.cell_size}, border_width, border_color);
            }
            
            char letter_to_display = '\0';
            if (row < state.history.attempt_count) {
                letter_to_display = state.history.all_guesses[row][col];
            } else if (row == state.history.attempt_count && col < state.input.current_letter_pos) {
                letter_to_display = state.input.current_word[col];
            }
            
            if (letter_to_display != '\0') {
                char letter_string[2] = {letter_to_display, '\0'};
                int font_size = (int)(layout.cell_size * 0.45f);
                int text_width = MeasureText(letter_string, font_size);
                int text_x = cell_x + (layout.cell_size - text_width) / 2;
                int text_y = cell_y + (layout.cell_size - font_size) / 2;
                
                Color text_color = (row < state.history.attempt_count) ? WORDLE_WHITE : WORDLE_BLACK;
                DrawText(letter_string, text_x, text_y, font_size, text_color);
            }
        }
    }
}

void ui_render_system(GameState state) {
    LayoutConfig layout = calculate_layout();
    
    const char* title = "WORDLE";
    int title_font_size = (int)(layout.screen_width * 0.06f);
    if (title_font_size < 30) title_font_size = 30;
    if (title_font_size > 60) title_font_size = 60;
    
    int title_width = MeasureText(title, title_font_size);
    int title_x = (layout.screen_width - title_width) / 2;
    int title_y = 30;
    
    DrawText(title, title_x, title_y, title_font_size, WORDLE_WHITE);
    
    if (state.system.debug_mode) {
        char debug_message[50];
        sprintf(debug_message, "DEBUG: Answer is %s", state.core.target_word);
        int debug_font_size = (int)(layout.screen_width * 0.025f);
        if (debug_font_size < 16) debug_font_size = 16;
        if (debug_font_size > 22) debug_font_size = 22;
        
        int debug_width = MeasureText(debug_message, debug_font_size);
        int debug_x = (layout.screen_width - debug_width) / 2;
        int debug_y = title_y + title_font_size + 10;
        
        DrawText(debug_message, debug_x, debug_y, debug_font_size, WORDLE_YELLOW);
    }
    
    int line_margin = (int)(layout.screen_width * 0.06f);
    DrawLine(line_margin, 90, layout.screen_width - line_margin, 90, WORDLE_BORDER);
    
    if (state.core.is_game_over) {
        const char* end_message = state.core.player_won ? "🎉 You Won! 🎉" : "😔 Game Over";
        int end_font_size = (int)(layout.screen_width * 0.045f);
        if (end_font_size < 24) end_font_size = 24;
        if (end_font_size > 48) end_font_size = 48;
        
        int end_width = MeasureText(end_message, end_font_size);
        int end_x = (layout.screen_width - end_width) / 2;
        int end_y = layout.board_start_y + layout.board_height + 40;
        
        Color end_color = state.core.player_won ? WORDLE_GREEN : WORDLE_GRAY;
        DrawText(end_message, end_x, end_y, end_font_size, end_color);
        
        if (!state.core.player_won) {
            char target_message[50];
            sprintf(target_message, "The word was: %s", state.core.target_word);
            int target_font_size = (int)(layout.screen_width * 0.03f);
            if (target_font_size < 18) target_font_size = 18;
            if (target_font_size > 30) target_font_size = 30;
            
            int target_width = MeasureText(target_message, target_font_size);
            int target_x = (layout.screen_width - target_width) / 2;
            int target_y = end_y + 50;
            
            DrawText(target_message, target_x, target_y, target_font_size, WORDLE_WHITE);
        }
        
        const char* restart_message = "Press SPACE to play again | F11 for fullscreen";
        int restart_font_size = (int)(layout.screen_width * 0.025f);
        if (restart_font_size < 16) restart_font_size = 16;
        if (restart_font_size > 24) restart_font_size = 24;
        
        int restart_width = MeasureText(restart_message, restart_font_size);
        int restart_x = (layout.screen_width - restart_width) / 2;
        int restart_y = layout.screen_height - 60;
        
        DrawText(restart_message, restart_x, restart_y, restart_font_size, WORDLE_BORDER);
        
        char stats_message[100];
        sprintf(stats_message, "Games: %d | Wins: %d | Streak: %d | Win Rate: %.0f%%", 
                state.stats.games_played, state.stats.games_won, 
                state.stats.current_streak, state.stats.win_percentage);
        int stats_font_size = (int)(layout.screen_width * 0.02f);
        if (stats_font_size < 14) stats_font_size = 14;
        if (stats_font_size > 20) stats_font_size = 20;
        
        int stats_width = MeasureText(stats_message, stats_font_size);
        int stats_x = (layout.screen_width - stats_width) / 2;
        int stats_y = restart_y + 30;
        
        DrawText(stats_message, stats_x, stats_y, stats_font_size, WORDLE_BORDER);
        
    } else {
        const char* instruction = "Type a 5-letter word and press ENTER | F11 for fullscreen";
        int instruction_font_size = (int)(layout.screen_width * 0.022f);
        if (instruction_font_size < 16) instruction_font_size = 16;
        if (instruction_font_size > 22) instruction_font_size = 22;
        
        int instruction_width = MeasureText(instruction, instruction_font_size);
        int instruction_x = (layout.screen_width - instruction_width) / 2;
        int instruction_y = layout.screen_height - 80;
        
        DrawText(instruction, instruction_x, instruction_y, instruction_font_size, WORDLE_BORDER);
        
        const char* debug_instruction = "Press 1 for debug mode | 2-5 for settings";
        int debug_instruction_font_size = (int)(layout.screen_width * 0.018f);
        if (debug_instruction_font_size < 12) debug_instruction_font_size = 12;
        if (debug_instruction_font_size > 16) debug_instruction_font_size = 16;
        
        int debug_instruction_width = MeasureText(debug_instruction, debug_instruction_font_size);
        int debug_instruction_x = (layout.screen_width - debug_instruction_width) / 2;
        int debug_instruction_y = instruction_y + 25;
        
        DrawText(debug_instruction, debug_instruction_x, debug_instruction_y, debug_instruction_font_size, WORDLE_GRAY);
        
        char attempt_message[20];
        sprintf(attempt_message, "Attempt %d of %d", state.history.attempt_count + 1, MAX_ATTEMPTS);
        int attempt_font_size = (int)(layout.screen_width * 0.02f);
        if (attempt_font_size < 14) attempt_font_size = 14;
        if (attempt_font_size > 18) attempt_font_size = 18;
        
        int attempt_width = MeasureText(attempt_message, attempt_font_size);
        int attempt_x = (layout.screen_width - attempt_width) / 2;
        int attempt_y = debug_instruction_y + 20;
        
        DrawText(attempt_message, attempt_x, attempt_y, attempt_font_size, WORDLE_GRAY);
    }
}

void render_system(GameState state) {
    ClearBackground(WORDLE_BG);
    board_render_system(state);
    ui_render_system(state);
}

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