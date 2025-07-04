#include "systems.h"
#include <math.h>

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
    
    // Calculate top bar content and dimensions
    char level_title[50];
    sprintf(level_title, "WORDLE - LEVEL %d", state.core.current_level);
    int title_font_size = (int)(layout.screen_width * 0.05f);
    if (title_font_size < 24) title_font_size = 24;
    if (title_font_size > 48) title_font_size = 48;
    
    int title_width = MeasureText(level_title, title_font_size);
    int title_x = (layout.screen_width - title_width) / 2;
    int title_y = 20;  // Top margin
    
    // Calculate debug info if present
    int debug_font_size = 0;
    int debug_y = 0;
    char debug_message[50] = "";
    if (state.system.debug_mode) {
        sprintf(debug_message, "DEBUG: Answer is %s", state.core.target_word);
        debug_font_size = (int)(layout.screen_width * 0.025f);
        if (debug_font_size < 16) debug_font_size = 16;
        if (debug_font_size > 22) debug_font_size = 22;
        debug_y = title_y + title_font_size + 10;
    }
    
    // Calculate level stats
    char level_stats[100];
    sprintf(level_stats, "Guess %d this level | %d total guesses", 
            state.core.guesses_this_level + 1, state.core.total_lifetime_guesses);
    int stats_font_size = (int)(layout.screen_width * 0.02f);
    if (stats_font_size < 14) stats_font_size = 14;
    if (stats_font_size > 18) stats_font_size = 18;
    
    int stats_width = MeasureText(level_stats, stats_font_size);
    int stats_x = (layout.screen_width - stats_width) / 2;
    int stats_y = (state.system.debug_mode) ? debug_y + debug_font_size + 10 : title_y + title_font_size + 10;
    
    // Calculate top bar height
    int top_bar_height = stats_y + stats_font_size + 20;  // 20px bottom padding
    
    // Draw full-width white top bar
    Rectangle top_bar = {0, 0, layout.screen_width, top_bar_height};
    DrawRectangleRec(top_bar, WORDLE_WHITE);
    
    // Draw top bar content with dark text
    DrawText(level_title, title_x, title_y, title_font_size, WORDLE_BLACK);
    
    if (state.system.debug_mode) {
        int debug_width = MeasureText(debug_message, debug_font_size);
        int debug_x = (layout.screen_width - debug_width) / 2;
        DrawText(debug_message, debug_x, debug_y, debug_font_size, (Color){200, 140, 0, 255});  // Dark yellow
    }
    
    DrawText(level_stats, stats_x, stats_y, stats_font_size, WORDLE_DARK_GRAY);
    
    // Game state specific messages
    if (state.core.play_state == GAME_STATE_LEVEL_COMPLETE) {
        const char* success_message = "LEVEL COMPLETE!";
        int success_font_size = (int)(layout.screen_width * 0.04f);
        if (success_font_size < 20) success_font_size = 20;
        if (success_font_size > 36) success_font_size = 36;
        
        char level_score[100];
        sprintf(level_score, "Solved in %d guesses! Press SPACE for Level %d", 
                state.core.guesses_this_level, state.core.current_level + 1);
        int score_font_size = (int)(layout.screen_width * 0.025f);
        if (score_font_size < 16) score_font_size = 16;
        if (score_font_size > 24) score_font_size = 24;
        
        // Calculate completion message bar
        int completion_bar_height = success_font_size + score_font_size + 40;  // 40px padding
        int completion_bar_y = layout.board_start_y + layout.board_height + 20;
        
        // Draw completion message bar
        Rectangle completion_bar = {0, completion_bar_y, layout.screen_width, completion_bar_height};
        DrawRectangleRec(completion_bar, WORDLE_WHITE);
        
        // Draw completion content
        int success_width = MeasureText(success_message, success_font_size);
        int success_x = (layout.screen_width - success_width) / 2;
        int success_y = completion_bar_y + 15;
        DrawText(success_message, success_x, success_y, success_font_size, WORDLE_GREEN);
        
        int score_width = MeasureText(level_score, score_font_size);
        int score_x = (layout.screen_width - score_width) / 2;
        int score_y = success_y + success_font_size + 10;
        DrawText(level_score, score_x, score_y, score_font_size, WORDLE_BLACK);
        
    } else {
        // Calculate bottom bar content and dimensions
        const char* instruction = "Type a 5-letter word and press ENTER";
        int instruction_font_size = (int)(layout.screen_width * 0.022f);
        if (instruction_font_size < 16) instruction_font_size = 16;
        if (instruction_font_size > 22) instruction_font_size = 22;
        
        const char* debug_instruction = "Press 1 for debug mode | 2 for letter bag | 3 for test tokens | 4-5 for settings";
        int debug_instruction_font_size = (int)(layout.screen_width * 0.018f);
        if (debug_instruction_font_size < 12) debug_instruction_font_size = 12;
        if (debug_instruction_font_size > 16) debug_instruction_font_size = 16;
        
        // Lifetime stats
        char lifetime_stats[150];
        sprintf(lifetime_stats, "Levels completed: %d | Best: %d guesses | Average: %.1f guesses", 
                state.stats.levels_completed, 
                (state.stats.best_level_score < 999) ? state.stats.best_level_score : 0,
                state.stats.average_guesses_per_level);
        int lifetime_font_size = (int)(layout.screen_width * 0.018f);
        if (lifetime_font_size < 12) lifetime_font_size = 12;
        if (lifetime_font_size > 16) lifetime_font_size = 16;
        
        // Letter bag text
        char letter_bag_text[300] = "";
        int bag_font_size = 0;
        if (state.stats.show_letter_bag) {
            strcpy(letter_bag_text, "Letter Bag: ");
            int bag_start_len = strlen(letter_bag_text);
            
            for (int i = 0; i < 26; i++) {
                if (state.stats.letter_counts[i] > 0) {
                    char letter_entry[10];
                    sprintf(letter_entry, "%c:%d ", 'A' + i, state.stats.letter_counts[i]);
                    strcat(letter_bag_text, letter_entry);
                }
            }
            
            if (strlen(letter_bag_text) == bag_start_len) {
                strcat(letter_bag_text, "No letters collected yet");
            }
            
            bag_font_size = (int)(layout.screen_width * 0.016f);
            if (bag_font_size < 12) bag_font_size = 12;
            if (bag_font_size > 16) bag_font_size = 16;
        }
        
        // Calculate bottom bar height and position
        int total_lines = 3 + (state.stats.show_letter_bag ? 1 : 0);  // instruction + debug + lifetime + bag
        int line_spacing = 5;
        int bottom_bar_height = instruction_font_size + debug_instruction_font_size + lifetime_font_size + 
                               (state.stats.show_letter_bag ? bag_font_size : 0) + 
                               (total_lines - 1) * line_spacing + 40;  // 40px total padding
        
        int bottom_bar_y = layout.screen_height - bottom_bar_height;
        
        // Draw full-width white bottom bar
        Rectangle bottom_bar = {0, bottom_bar_y, layout.screen_width, bottom_bar_height};
        DrawRectangleRec(bottom_bar, WORDLE_WHITE);
        
        // Draw bottom bar content with dark text
        int current_y = bottom_bar_y + 20;  // Top padding
        
        // Letter bag (if shown) - at top of bottom bar
        if (state.stats.show_letter_bag) {
            int bag_width = MeasureText(letter_bag_text, bag_font_size);
            int bag_x = (layout.screen_width - bag_width) / 2;
            DrawText(letter_bag_text, bag_x, current_y, bag_font_size, (Color){200, 140, 0, 255});  // Dark yellow
            current_y += bag_font_size + line_spacing;
        }
        
        // Lifetime stats
        int lifetime_width = MeasureText(lifetime_stats, lifetime_font_size);
        int lifetime_x = (layout.screen_width - lifetime_width) / 2;
        DrawText(lifetime_stats, lifetime_x, current_y, lifetime_font_size, WORDLE_DARK_GRAY);
        current_y += lifetime_font_size + line_spacing;
        
        // Debug instruction
        int debug_instruction_width = MeasureText(debug_instruction, debug_instruction_font_size);
        int debug_instruction_x = (layout.screen_width - debug_instruction_width) / 2;
        DrawText(debug_instruction, debug_instruction_x, current_y, debug_instruction_font_size, WORDLE_DARK_GRAY);
        current_y += debug_instruction_font_size + line_spacing;
        
        // Main instruction
        int instruction_width = MeasureText(instruction, instruction_font_size);
        int instruction_x = (layout.screen_width - instruction_width) / 2;
        DrawText(instruction, instruction_x, current_y, instruction_font_size, WORDLE_BLACK);
    }
}

void crossword_render_system(GameState state) {
    int screen_width = GetScreenWidth();
    
    // Calculate top bar for crossword title
    const char* title = "CROSSWORD";
    int title_font_size = 48;
    int title_width = MeasureText(title, title_font_size);
    int title_x = (screen_width - title_width) / 2;
    int title_y = 20;  // Top margin
    
    // Calculate top bar height
    int top_bar_height = title_font_size + 40;  // 40px total padding
    
    // Draw full-width white top bar
    Rectangle top_bar = {0, 0, screen_width, top_bar_height};
    DrawRectangleRec(top_bar, WORDLE_WHITE);
    
    // Draw title with dark text
    DrawText(title, title_x, title_y, title_font_size, WORDLE_BLACK);
    
    // Calculate grid layout
    int grid_size = 9;
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
                // Word cell - default to white background
                cell_color = WORDLE_WHITE;
                
                char placed_letter = state.crossword.grid[x][y];
                
                if (placed_letter != '\0') {
                    if (state.crossword.word_validated[x][y]) {
                        // Use proper Wordle validation colors for validated letters
                        cell_color = get_color_for_letter_state(state.crossword.letter_states[x][y]);
                    } else {
                        // Light grey for unvalidated placed letters
                        cell_color = (Color){220, 220, 220, 255};  // Light grey
                    }
                }
                
                // Highlight current word cells with subtle background
                CrosswordWord* current_word = &state.crossword.current_level.words[state.crossword.current_word_index];
                int is_current_word_cell = 0;
                if (current_word->direction == 0) { // horizontal
                    if (y == current_word->start_y && 
                        x >= current_word->start_x && 
                        x < current_word->start_x + current_word->length) {
                        is_current_word_cell = 1;
                    }
                } else { // vertical
                    if (x == current_word->start_x && 
                        y >= current_word->start_y && 
                        y < current_word->start_y + current_word->length) {
                        is_current_word_cell = 1;
                    }
                }
                
                if (is_current_word_cell && placed_letter == '\0' && !state.crossword.word_validated[x][y]) {
                    // Subtle highlight for current word cells
                    cell_color = (Color){240, 240, 240, 255};  // Very light gray
                }
                
                // Highlight cursor position (only in word cells, not on green letters)
                if (x == state.crossword.cursor_x && y == state.crossword.cursor_y) {
                    // Only show yellow cursor on non-green cells
                    if ((placed_letter == '\0' || !state.crossword.word_validated[x][y]) ||
                        (state.crossword.word_validated[x][y] && 
                         state.crossword.letter_states[x][y] != LETTER_CORRECT)) {
                        cell_color = WORDLE_YELLOW;  // Cursor highlight
                    }
                    // Always show white border on cursor position (even on green)
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
                    
                    // Text color based on validation state
                    if (state.crossword.word_validated[x][y]) {
                        // Validated letters use white text for all colored backgrounds
                        text_color = WORDLE_WHITE;
                    } else {
                        // Unvalidated letters use black text on light grey background
                        text_color = WORDLE_BLACK;
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
    
    
    // Word and direction indicator at top-left of grid
    const char* direction_text = (state.crossword.cursor_direction == 0) ? "ACROSS" : "DOWN";
    char word_indicator[50];
    sprintf(word_indicator, "Word %d - %s", 
            state.crossword.current_word_index + 1, 
            direction_text);
    
    int word_font_size = 18;
    int word_x = grid_start_x;  // Top-left of grid
    int word_y = grid_start_y - 40;  // Above the grid
    DrawText(word_indicator, word_x, word_y, word_font_size, (Color){200, 140, 0, 255});  // Dark yellow
    
    // Calculate bottom bar content and dimensions
    const char* instructions = "Left/Right: select word | Up/Down: navigate within word | Letters: place letters | Enter: validate word | Tab: return to Wordle";
    int inst_font_size = 14;
    
    // Letter bag text
    char letter_bag_text[300] = "";
    int bag_font_size = 0;
    if (state.stats.show_letter_bag) {
        strcpy(letter_bag_text, "Available Letters: ");
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
        
        bag_font_size = 14;
    }
    
    // Calculate bottom bar height and position
    int total_lines = 1 + (state.stats.show_letter_bag ? 1 : 0);  // instructions + bag
    int line_spacing = 5;
    int bottom_bar_height = inst_font_size + (state.stats.show_letter_bag ? bag_font_size : 0) + 
                           (total_lines - 1) * line_spacing + 40;  // 40px total padding
    
    int bottom_bar_y = screen_height - bottom_bar_height;
    
    // Draw full-width white bottom bar
    Rectangle bottom_bar = {0, bottom_bar_y, screen_width, bottom_bar_height};
    DrawRectangleRec(bottom_bar, WORDLE_WHITE);
    
    // Draw bottom bar content with dark text
    int current_y = bottom_bar_y + 20;  // Top padding
    
    // Letter bag (if shown) - at top of bottom bar
    if (state.stats.show_letter_bag) {
        int bag_width = MeasureText(letter_bag_text, bag_font_size);
        int bag_x = (screen_width - bag_width) / 2;
        DrawText(letter_bag_text, bag_x, current_y, bag_font_size, (Color){200, 140, 0, 255});  // Dark yellow
        current_y += bag_font_size + line_spacing;
    }
    
    // Instructions
    int inst_width = MeasureText(instructions, inst_font_size);
    int inst_x = (screen_width - inst_width) / 2;
    DrawText(instructions, inst_x, current_y, inst_font_size, WORDLE_BLACK);
}

void home_screen_render_system(GameState state) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    
    // Game title
    const char* title = "CROSSWORDLE";
    int title_font_size = (int)(screen_width * 0.08f);
    if (title_font_size < 48) title_font_size = 48;
    if (title_font_size > 96) title_font_size = 96;
    
    int title_width = MeasureText(title, title_font_size);
    int title_x = (screen_width - title_width) / 2;
    int title_y = screen_height / 3;
    
    // Draw background rectangle for home title
    int title_bg_padding = 12;
    Rectangle title_bg = {
        title_x - title_bg_padding,
        title_y - title_bg_padding,
        title_width + 2 * title_bg_padding,
        title_font_size + 2 * title_bg_padding
    };
    DrawRectangleRec(title_bg, (Color){0, 0, 0, 180});  // Semi-transparent black
    
    DrawText(title, title_x, title_y, title_font_size, WORDLE_WHITE);
    
    // Subtitle
    const char* subtitle = "Infinite Wordle + Crossword Puzzles";
    int subtitle_font_size = (int)(screen_width * 0.025f);
    if (subtitle_font_size < 18) subtitle_font_size = 18;
    if (subtitle_font_size > 32) subtitle_font_size = 32;
    
    int subtitle_width = MeasureText(subtitle, subtitle_font_size);
    int subtitle_x = (screen_width - subtitle_width) / 2;
    int subtitle_y = title_y + title_font_size + 20;
    
    DrawText(subtitle, subtitle_x, subtitle_y, subtitle_font_size, WORDLE_GRAY);
    
    // Start instruction
    const char* start_text = "Press ENTER to start";
    int start_font_size = (int)(screen_width * 0.04f);
    if (start_font_size < 24) start_font_size = 24;
    if (start_font_size > 48) start_font_size = 48;
    
    int start_width = MeasureText(start_text, start_font_size);
    int start_x = (screen_width - start_width) / 2;
    int start_y = screen_height / 2 + 80;
    
    // Add a subtle pulsing effect
    float pulse = (sin(GetTime() * 2.0f) + 1.0f) * 0.5f;
    Color start_color = (Color){
        (int)(WORDLE_YELLOW.r * (0.7f + 0.3f * pulse)),
        (int)(WORDLE_YELLOW.g * (0.7f + 0.3f * pulse)),
        (int)(WORDLE_YELLOW.b * (0.7f + 0.3f * pulse)),
        255
    };
    
    DrawText(start_text, start_x, start_y, start_font_size, start_color);
    
    // Version or credits at bottom
    const char* credits = "Built with Raylib";
    int credits_font_size = 14;
    int credits_width = MeasureText(credits, credits_font_size);
    int credits_x = (screen_width - credits_width) / 2;
    int credits_y = screen_height - 40;
    
    DrawText(credits, credits_x, credits_y, credits_font_size, WORDLE_BORDER);
}

void crossword_completion_render_system(GameState state) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    
    // Victory title
    const char* victory_title = "CROSSWORD COMPLETED!";
    int title_font_size = (int)(screen_width * 0.06f);
    if (title_font_size < 36) title_font_size = 36;
    if (title_font_size > 72) title_font_size = 72;
    
    int title_width = MeasureText(victory_title, title_font_size);
    int title_x = (screen_width - title_width) / 2;
    int title_y = screen_height / 3;
    
    DrawText(victory_title, title_x, title_y, title_font_size, WORDLE_GREEN);
    
    // Congratulations message
    const char* congrats = "Well done! You solved the puzzle!";
    int congrats_font_size = (int)(screen_width * 0.03f);
    if (congrats_font_size < 20) congrats_font_size = 20;
    if (congrats_font_size > 36) congrats_font_size = 36;
    
    int congrats_width = MeasureText(congrats, congrats_font_size);
    int congrats_x = (screen_width - congrats_width) / 2;
    int congrats_y = title_y + title_font_size + 40;
    
    DrawText(congrats, congrats_x, congrats_y, congrats_font_size, WORDLE_WHITE);
    
    // Statistics (letters used, etc.)
    int total_letters_used = 0;
    for (int i = 0; i < 26; i++) {
        total_letters_used += state.stats.letter_counts[i];
    }
    
    char stats_text[100];
    sprintf(stats_text, "Letter tokens remaining: %d", total_letters_used);
    int stats_font_size = (int)(screen_width * 0.025f);
    if (stats_font_size < 16) stats_font_size = 16;
    if (stats_font_size > 24) stats_font_size = 24;
    
    int stats_width = MeasureText(stats_text, stats_font_size);
    int stats_x = (screen_width - stats_width) / 2;
    int stats_y = congrats_y + congrats_font_size + 40;
    
    DrawText(stats_text, stats_x, stats_y, stats_font_size, WORDLE_YELLOW);
    
    // Navigation instructions
    const char* space_instruction = "Press SPACE to return to home screen";
    int space_font_size = (int)(screen_width * 0.03f);
    if (space_font_size < 18) space_font_size = 18;
    if (space_font_size > 32) space_font_size = 32;
    
    int space_width = MeasureText(space_instruction, space_font_size);
    int space_x = (screen_width - space_width) / 2;
    int space_y = screen_height / 2 + 80;
    
    DrawText(space_instruction, space_x, space_y, space_font_size, WORDLE_WHITE);
    
    const char* tab_instruction = "Press TAB to continue playing Wordle";
    int tab_width = MeasureText(tab_instruction, space_font_size);
    int tab_x = (screen_width - tab_width) / 2;
    int tab_y = space_y + space_font_size + 20;
    
    DrawText(tab_instruction, tab_x, tab_y, space_font_size, WORDLE_GRAY);
    
    // Add some celebratory visual effects (optional sparkles)
    // Simple animated stars/sparkles
    for (int i = 0; i < 8; i++) {
        float angle = (GetTime() + i * 0.5f) * 2.0f;
        int star_x = (int)(title_x + title_width / 2 + cos(angle) * (100 + i * 10));
        int star_y = (int)(title_y + title_font_size / 2 + sin(angle * 1.3f) * (50 + i * 5));
        
        char star_char = (i % 2 == 0) ? '*' : '+';
        Color star_color = (i % 3 == 0) ? WORDLE_YELLOW : 
                          (i % 3 == 1) ? WORDLE_GREEN : WORDLE_WHITE;
        
        DrawText((char[]){star_char, '\0'}, star_x, star_y, 20, star_color);
    }
}

void render_system(GameState state) {
    ClearBackground(WORDLE_BG);
    
    if (state.current_view == VIEW_HOME_SCREEN) {
        home_screen_render_system(state);
    } else if (state.current_view == VIEW_WORDLE) {
        board_render_system(state);
        ui_render_system(state);
    } else if (state.current_view == VIEW_CROSSWORD) {
        crossword_render_system(state);
    } else if (state.current_view == VIEW_CROSSWORD_COMPLETE) {
        crossword_completion_render_system(state);
    }
}