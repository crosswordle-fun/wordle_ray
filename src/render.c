#include "systems.h"

LayoutConfig calculate_layout(void) {
    LayoutConfig layout = {0};
    
    layout.screen_width = GetScreenWidth();
    layout.screen_height = GetScreenHeight();
    
    int available_width = layout.screen_width - 100;
    
    // For single row, we can use more generous cell sizing
    layout.cell_size = available_width / (WORD_LENGTH + (WORD_LENGTH - 1) * CELL_SPACING_RATIO);
    
    if (layout.cell_size < MIN_CELL_SIZE) layout.cell_size = MIN_CELL_SIZE;
    if (layout.cell_size > MAX_CELL_SIZE) layout.cell_size = MAX_CELL_SIZE;
    
    layout.cell_spacing = (int)(layout.cell_size * CELL_SPACING_RATIO);
    
    layout.board_width = WORD_LENGTH * layout.cell_size + (WORD_LENGTH - 1) * layout.cell_spacing;
    layout.board_height = layout.cell_size;  // Single row only
    
    layout.board_start_x = (layout.screen_width - layout.board_width) / 2;
    layout.board_start_y = (layout.screen_height - layout.cell_size) / 2;  // Center vertically
    
    return layout;
}

Color get_color_for_letter_state(LetterState state) {
    if (state == LETTER_CORRECT) return WORDLE_GREEN;
    if (state == LETTER_WRONG_POS) return WORDLE_YELLOW;
    if (state == LETTER_NOT_IN_WORD) return WORDLE_GRAY;
    return WORDLE_WHITE;
}

void board_render_system(GameState state) {
    LayoutConfig layout = calculate_layout();
    
    // Single row display - shows either input or result
    for (int col = 0; col < WORD_LENGTH; col++) {
        int cell_x = layout.board_start_x + col * (layout.cell_size + layout.cell_spacing);
        int cell_y = layout.board_start_y;
        
        Color cell_color = WORDLE_WHITE;
        Color border_color = WORDLE_BORDER;
        int border_width = 2;
        char letter_to_display = '\0';
        Color text_color = WORDLE_BLACK;
        
        if (state.core.play_state == GAME_STATE_SHOWING_RESULT) {
            // Show the result of the last guess with colors
            if (col < WORD_LENGTH) {
                cell_color = get_color_for_letter_state(state.history.current_guess_states[col]);
                border_color = cell_color;
                border_width = 0;
                letter_to_display = state.history.current_guess[col];
                text_color = WORDLE_WHITE;
            }
        } else if (state.core.play_state == GAME_STATE_INPUT) {
            // Show current input
            if (col < state.input.current_letter_pos) {
                cell_color = WORDLE_INPUT;
                border_color = WORDLE_DARK_GRAY;
                letter_to_display = state.input.current_word[col];
                text_color = WORDLE_WHITE;
            }
        }
        
        // Draw the cell
        DrawRectangle(cell_x, cell_y, layout.cell_size, layout.cell_size, cell_color);
        
        if (border_width > 0) {
            DrawRectangleLinesEx((Rectangle){cell_x, cell_y, layout.cell_size, layout.cell_size}, border_width, border_color);
        }
        
        // Draw the letter
        if (letter_to_display != '\0') {
            char letter_string[2] = {letter_to_display, '\0'};
            int font_size = (int)(layout.cell_size * 0.45f);
            int text_width = MeasureText(letter_string, font_size);
            int text_x = cell_x + (layout.cell_size - text_width) / 2;
            int text_y = cell_y + (layout.cell_size - font_size) / 2;
            
            DrawText(letter_string, text_x, text_y, font_size, text_color);
        }
    }
}

void ui_render_system(GameState state) {
    LayoutConfig layout = calculate_layout();
    
    // Level title at top
    char level_title[50];
    sprintf(level_title, "WORDLE - LEVEL %d", state.core.current_level);
    int title_font_size = (int)(layout.screen_width * 0.05f);
    if (title_font_size < 24) title_font_size = 24;
    if (title_font_size > 48) title_font_size = 48;
    
    int title_width = MeasureText(level_title, title_font_size);
    int title_x = (layout.screen_width - title_width) / 2;
    int title_y = 40;
    
    DrawText(level_title, title_x, title_y, title_font_size, WORDLE_WHITE);
    
    // Debug info
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
    
    // Current level stats above the word
    char level_stats[100];
    sprintf(level_stats, "Guess %d this level | %d total guesses", 
            state.core.guesses_this_level + 1, state.core.total_lifetime_guesses);
    int stats_font_size = (int)(layout.screen_width * 0.02f);
    if (stats_font_size < 14) stats_font_size = 14;
    if (stats_font_size > 18) stats_font_size = 18;
    
    int stats_width = MeasureText(level_stats, stats_font_size);
    int stats_x = (layout.screen_width - stats_width) / 2;
    int stats_y = layout.board_start_y - 60;
    
    DrawText(level_stats, stats_x, stats_y, stats_font_size, WORDLE_GRAY);
    
    // Game state specific messages
    if (state.core.play_state == GAME_STATE_LEVEL_COMPLETE) {
        const char* success_message = "LEVEL COMPLETE!";
        int success_font_size = (int)(layout.screen_width * 0.04f);
        if (success_font_size < 20) success_font_size = 20;
        if (success_font_size > 36) success_font_size = 36;
        
        int success_width = MeasureText(success_message, success_font_size);
        int success_x = (layout.screen_width - success_width) / 2;
        int success_y = layout.board_start_y + layout.board_height + 40;
        
        DrawText(success_message, success_x, success_y, success_font_size, WORDLE_GREEN);
        
        char level_score[100];
        sprintf(level_score, "Solved in %d guesses! Press SPACE for Level %d", 
                state.core.guesses_this_level, state.core.current_level + 1);
        int score_font_size = (int)(layout.screen_width * 0.025f);
        if (score_font_size < 16) score_font_size = 16;
        if (score_font_size > 24) score_font_size = 24;
        
        int score_width = MeasureText(level_score, score_font_size);
        int score_x = (layout.screen_width - score_width) / 2;
        int score_y = success_y + 50;
        
        DrawText(level_score, score_x, score_y, score_font_size, WORDLE_WHITE);
        
    } else {
        // Normal gameplay instructions
        const char* instruction = "Type a 5-letter word and press ENTER";
        int instruction_font_size = (int)(layout.screen_width * 0.022f);
        if (instruction_font_size < 16) instruction_font_size = 16;
        if (instruction_font_size > 22) instruction_font_size = 22;
        
        int instruction_width = MeasureText(instruction, instruction_font_size);
        int instruction_x = (layout.screen_width - instruction_width) / 2;
        int instruction_y = layout.board_start_y + layout.board_height + 60;
        
        DrawText(instruction, instruction_x, instruction_y, instruction_font_size, WORDLE_BORDER);
        
        const char* debug_instruction = "Press 1 for debug mode | 2-5 for settings";
        int debug_instruction_font_size = (int)(layout.screen_width * 0.018f);
        if (debug_instruction_font_size < 12) debug_instruction_font_size = 12;
        if (debug_instruction_font_size > 16) debug_instruction_font_size = 16;
        
        int debug_instruction_width = MeasureText(debug_instruction, debug_instruction_font_size);
        int debug_instruction_x = (layout.screen_width - debug_instruction_width) / 2;
        int debug_instruction_y = instruction_y + 25;
        
        DrawText(debug_instruction, debug_instruction_x, debug_instruction_y, debug_instruction_font_size, WORDLE_GRAY);
    }
    
    // Lifetime stats at bottom
    char lifetime_stats[150];
    sprintf(lifetime_stats, "Levels completed: %d | Best: %d guesses | Average: %.1f guesses", 
            state.stats.levels_completed, 
            (state.stats.best_level_score < 999) ? state.stats.best_level_score : 0,
            state.stats.average_guesses_per_level);
    int lifetime_font_size = (int)(layout.screen_width * 0.018f);
    if (lifetime_font_size < 12) lifetime_font_size = 12;
    if (lifetime_font_size > 16) lifetime_font_size = 16;
    
    int lifetime_width = MeasureText(lifetime_stats, lifetime_font_size);
    int lifetime_x = (layout.screen_width - lifetime_width) / 2;
    int lifetime_y = layout.screen_height - 30;
    
    DrawText(lifetime_stats, lifetime_x, lifetime_y, lifetime_font_size, WORDLE_BORDER);
}

void render_system(GameState state) {
    ClearBackground(WORDLE_BG);
    board_render_system(state);
    ui_render_system(state);
}