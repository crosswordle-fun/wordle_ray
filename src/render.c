#include "systems.h"

LayoutConfig calculate_layout(GameState state) {
    LayoutConfig layout = {0};
    
    layout.screen_width = GetScreenWidth();
    layout.screen_height = GetScreenHeight();
    
    int available_width = layout.screen_width - 100;
    
    // Calculate cell size based on available width
    layout.cell_size = available_width / (WORD_LENGTH + (WORD_LENGTH - 1) * CELL_SPACING_RATIO);
    
    if (layout.cell_size < MIN_CELL_SIZE) layout.cell_size = MIN_CELL_SIZE;
    if (layout.cell_size > MAX_CELL_SIZE) layout.cell_size = MAX_CELL_SIZE;
    
    layout.cell_spacing = (int)(layout.cell_size * CELL_SPACING_RATIO);
    layout.row_height = layout.cell_size + layout.cell_spacing;
    
    // Calculate dynamic row count: completed guesses + 1 input row
    layout.total_rows = state.history.level_guess_count + 1;
    layout.current_input_row = state.history.level_guess_count;  // 0-indexed
    
    layout.board_width = WORD_LENGTH * layout.cell_size + (WORD_LENGTH - 1) * layout.cell_spacing;
    layout.board_height = layout.total_rows * layout.row_height - layout.cell_spacing;  // Remove spacing after last row
    
    layout.board_start_x = (layout.screen_width - layout.board_width) / 2;
    
    // Calculate board_start_y with camera offset to center current input row
    int input_row_y = layout.current_input_row * layout.row_height;
    int desired_input_y = (layout.screen_height - layout.cell_size) / 2;  // Center of screen
    layout.board_start_y = desired_input_y - input_row_y + (int)state.system.camera_offset_y;
    
    return layout;
}

Color get_color_for_letter_state(LetterState state) {
    if (state == LETTER_CORRECT) return WORDLE_GREEN;
    if (state == LETTER_WRONG_POS) return WORDLE_YELLOW;
    if (state == LETTER_NOT_IN_WORD) return WORDLE_GRAY;
    return WORDLE_WHITE;
}

void board_render_system(GameState state) {
    LayoutConfig layout = calculate_layout(state);
    
    // Render all rows (completed guesses + current input row)
    for (int row = 0; row < layout.total_rows; row++) {
        int row_y = layout.board_start_y + row * layout.row_height;
        
        // Skip rendering rows that are off-screen (simple culling)
        if (row_y + layout.cell_size < 0 || row_y > layout.screen_height) {
            continue;
        }
        
        for (int col = 0; col < WORD_LENGTH; col++) {
            int cell_x = layout.board_start_x + col * (layout.cell_size + layout.cell_spacing);
            int cell_y = row_y;
            
            Color cell_color = WORDLE_WHITE;
            Color border_color = WORDLE_BORDER;
            int border_width = 2;
            char letter_to_display = '\0';
            Color text_color = WORDLE_BLACK;
            
            // Determine what to display based on row type
            if (row < state.history.level_guess_count) {
                // Completed guess row
                cell_color = get_color_for_letter_state(state.history.level_letter_states[row][col]);
                border_color = cell_color;
                border_width = 0;
                letter_to_display = state.history.level_guesses[row][col];
                text_color = WORDLE_WHITE;
                
            } else if (row == layout.current_input_row) {
                // Current input row - only show current input since guesses are added to history immediately
                if (state.core.play_state == GAME_STATE_INPUT && col < state.input.current_letter_pos) {
                    // Show current input
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
}

void ui_render_system(GameState state) {
    LayoutConfig layout = calculate_layout(state);
    
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
        
        const char* debug_instruction = "Press 1 for debug mode | 2 for letter bag | 3 for test tokens | 4-5 for settings";
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
    
    // Letter bag display
    if (state.stats.show_letter_bag) {
        char letter_bag_text[300] = "Letter Bag: ";
        int bag_start_len = strlen(letter_bag_text);
        
        // Build letter bag string showing only letters with count > 0
        for (int i = 0; i < 26; i++) {
            if (state.stats.letter_counts[i] > 0) {
                char letter_entry[10];
                sprintf(letter_entry, "%c:%d ", 'A' + i, state.stats.letter_counts[i]);
                strcat(letter_bag_text, letter_entry);
            }
        }
        
        // If no letters collected yet, show a message
        if (strlen(letter_bag_text) == bag_start_len) {
            strcat(letter_bag_text, "No letters collected yet");
        }
        
        int bag_font_size = (int)(layout.screen_width * 0.016f);
        if (bag_font_size < 12) bag_font_size = 12;
        if (bag_font_size > 16) bag_font_size = 16;
        
        int bag_width = MeasureText(letter_bag_text, bag_font_size);
        int bag_x = (layout.screen_width - bag_width) / 2;
        int bag_y = lifetime_y - 25;  // Above lifetime stats
        
        DrawText(letter_bag_text, bag_x, bag_y, bag_font_size, WORDLE_YELLOW);
    }
}

void crossword_render_system(GameState state) {
    // Title
    const char* title = "CROSSWORD";
    int title_font_size = 48;
    int title_width = MeasureText(title, title_font_size);
    int title_x = (GetScreenWidth() - title_width) / 2;
    int title_y = 40;
    DrawText(title, title_x, title_y, title_font_size, WORDLE_WHITE);
    
    // Calculate grid layout
    int grid_size = 9;
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    int available_size = (screen_width < screen_height - 200) ? screen_width - 100 : screen_height - 300;
    int cell_size = available_size / grid_size;
    if (cell_size > 60) cell_size = 60;
    if (cell_size < 30) cell_size = 30;
    
    int grid_width = grid_size * cell_size;
    int grid_height = grid_size * cell_size;
    int grid_start_x = (screen_width - grid_width) / 2;
    int grid_start_y = title_y + 80;
    
    // Draw grid
    for (int x = 0; x < grid_size; x++) {
        for (int y = 0; y < grid_size; y++) {
            int cell_x = grid_start_x + x * cell_size;
            int cell_y = grid_start_y + y * cell_size;
            
            // Check if this is a word cell or blocked cell
            int is_word_cell = state.crossword.current_level.word_mask[x][y];
            
            Color cell_color;
            Color border_color = WORDLE_BORDER;
            int border_width = 2;
            
            if (!is_word_cell) {
                // Blocked cell - dark gray
                cell_color = WORDLE_DARK_GRAY;
                border_color = WORDLE_BLACK;
            } else {
                // Word cell - white background
                cell_color = WORDLE_WHITE;
                
                // Check if letter matches solution for validation feedback
                char placed_letter = state.crossword.grid[x][y];
                char solution_letter = state.crossword.current_level.solution[x][y];
                
                if (placed_letter != '\0') {
                    if (placed_letter == solution_letter) {
                        cell_color = WORDLE_GREEN;  // Correct letter
                    } else {
                        cell_color = (Color){255, 200, 200, 255};  // Light red for incorrect
                    }
                }
                
                // Highlight cursor position (only in word cells)
                if (x == state.crossword.cursor_x && y == state.crossword.cursor_y) {
                    if (placed_letter == '\0' || placed_letter != solution_letter) {
                        cell_color = WORDLE_YELLOW;  // Cursor highlight
                    }
                    border_color = WORDLE_WHITE;
                    border_width = 3;
                }
            }
            
            // Draw cell
            DrawRectangle(cell_x, cell_y, cell_size, cell_size, cell_color);
            DrawRectangleLinesEx((Rectangle){cell_x, cell_y, cell_size, cell_size}, border_width, border_color);
            
            // Draw letter if present (only in word cells)
            if (is_word_cell) {
                char player_letter = state.crossword.grid[x][y];
                char solution_letter = state.crossword.current_level.solution[x][y];
                
                char letter_to_display = '\0';
                Color text_color = WORDLE_BLACK;
                
                if (player_letter != '\0') {
                    // Priority 1: Show player-placed letter with validation colors
                    letter_to_display = player_letter;
                    
                    // Text color based on cell background (green/red validation)
                    if (cell_color.r == WORDLE_GREEN.r && cell_color.g == WORDLE_GREEN.g) {
                        text_color = WORDLE_WHITE;  // White text on green background
                    } else {
                        text_color = WORDLE_BLACK;  // Black text on white/red background
                    }
                    
                } else if (state.system.debug_mode && solution_letter != '\0') {
                    // Priority 2: Show debug solution letter in light gray (only when no player letter)
                    letter_to_display = solution_letter;
                    text_color = (Color){180, 180, 180, 255};  // Light gray for debug
                }
                
                // Draw the letter if we have one to display
                if (letter_to_display != '\0') {
                    char letter_string[2] = {letter_to_display, '\0'};
                    int font_size = (int)(cell_size * 0.6f);
                    int text_width = MeasureText(letter_string, font_size);
                    int text_x = cell_x + (cell_size - text_width) / 2;
                    int text_y = cell_y + (cell_size - font_size) / 2;
                    
                    DrawText(letter_string, text_x, text_y, font_size, text_color);
                }
            }
        }
    }
    
    // Instructions
    const char* instructions = "Use arrows to move cursor | Letter keys to place tokens | Backspace to remove | Tab to return to Wordle";
    int inst_font_size = 16;
    int inst_width = MeasureText(instructions, inst_font_size);
    int inst_x = (screen_width - inst_width) / 2;
    int inst_y = grid_start_y + grid_height + 30;
    DrawText(instructions, inst_x, inst_y, inst_font_size, WORDLE_GRAY);
    
    // Letter bag display
    if (state.stats.show_letter_bag) {
        char letter_bag_text[300] = "Available Letters: ";
        int bag_start_len = strlen(letter_bag_text);
        
        for (int i = 0; i < 26; i++) {
            if (state.stats.letter_counts[i] > 0) {
                char letter_entry[10];
                sprintf(letter_entry, "%c:%d ", 'A' + i, state.stats.letter_counts[i]);
                strcat(letter_bag_text, letter_entry);
            }
        }
        
        if (strlen(letter_bag_text) == bag_start_len) {
            strcat(letter_bag_text, "None - play Wordle to earn letters!");
        }
        
        int bag_font_size = 14;
        int bag_width = MeasureText(letter_bag_text, bag_font_size);
        int bag_x = (screen_width - bag_width) / 2;
        int bag_y = inst_y + 30;
        
        DrawText(letter_bag_text, bag_x, bag_y, bag_font_size, WORDLE_YELLOW);
    }
}

void render_system(GameState state) {
    ClearBackground(WORDLE_BG);
    
    if (state.current_view == VIEW_WORDLE) {
        board_render_system(state);
        ui_render_system(state);
    } else if (state.current_view == VIEW_CROSSWORD) {
        crossword_render_system(state);
    }
}