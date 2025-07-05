#ifndef RENDER_H
#define RENDER_H

// Dependencies provided by systems.h

static inline LayoutConfig calculate_layout(GameState state) {
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

static inline Color get_color_for_letter_state(LetterState state) {
    if (state == LETTER_CORRECT) return WORDLE_GREEN;
    if (state == LETTER_WRONG_POS) return WORDLE_YELLOW;
    if (state == LETTER_NOT_IN_WORD) return WORDLE_GRAY;
    return WORDLE_WHITE;
}

static inline void render_animated_tab(GameState state) {
    // Render animated tab highlighting during view transitions
    if (!state.ui.transitioning_view || !state.settings.animations_enabled) return;
    
    float progress = state.ui.view_transition_timer / VIEW_TRANSITION_DURATION;
    progress = easeInOutQuad(progress);
    
    int screen_width = GetScreenWidth();
    int title_font_size = (int)(screen_width * 0.05f);
    if (title_font_size < 24) title_font_size = 24;
    if (title_font_size > 48) title_font_size = 48;
    
    // Calculate tab positions and dimensions
    int cross_width = MeasureText("CROSS", title_font_size);
    int space_width = MeasureText(" ", title_font_size);
    int wordle_width = MeasureText("WORDLE", title_font_size);
    
    int title_total_width = cross_width + space_width + wordle_width;
    int title_x = (screen_width - title_total_width) / 2;
    int title_y = 20;
    
    int tab_padding = 8;
    
    // Calculate start and end positions for tab highlighting
    int cross_tab_x = title_x - tab_padding;
    int cross_tab_width = cross_width + 2 * tab_padding;
    
    int wordle_tab_x = title_x + cross_width + space_width - tab_padding;
    int wordle_tab_width = wordle_width + 2 * tab_padding;
    
    // Interpolate tab position and size based on transition direction and progress
    int animated_tab_x, animated_tab_width;
    Color animated_tab_color;
    Color cross_text_color, wordle_text_color;
    
    if (state.current_view == VIEW_CROSSWORD) {
        // Transitioning TO crossword (CROSS tab becomes active)
        animated_tab_x = (int)(wordle_tab_x + (cross_tab_x - wordle_tab_x) * progress);
        animated_tab_width = (int)(wordle_tab_width + (cross_tab_width - wordle_tab_width) * progress);
        
        // Interpolate color from green to yellow
        float inv_progress = 1.0f - progress;
        animated_tab_color = (Color){
            (unsigned char)(WORDLE_GREEN.r * inv_progress + WORDLE_YELLOW.r * progress),
            (unsigned char)(WORDLE_GREEN.g * inv_progress + WORDLE_YELLOW.g * progress), 
            (unsigned char)(WORDLE_GREEN.b * inv_progress + WORDLE_YELLOW.b * progress),
            255
        };
        
        // Text colors transition
        cross_text_color = (Color){
            (unsigned char)(WORDLE_GRAY.r * inv_progress + WORDLE_WHITE.r * progress),
            (unsigned char)(WORDLE_GRAY.g * inv_progress + WORDLE_WHITE.g * progress),
            (unsigned char)(WORDLE_GRAY.b * inv_progress + WORDLE_WHITE.b * progress),
            255
        };
        wordle_text_color = (Color){
            (unsigned char)(WORDLE_WHITE.r * inv_progress + WORDLE_GRAY.r * progress),
            (unsigned char)(WORDLE_WHITE.g * inv_progress + WORDLE_GRAY.g * progress),
            (unsigned char)(WORDLE_WHITE.b * inv_progress + WORDLE_GRAY.b * progress),
            255
        };
        
    } else {
        // Transitioning TO wordle (WORDLE tab becomes active)
        animated_tab_x = (int)(cross_tab_x + (wordle_tab_x - cross_tab_x) * progress);
        animated_tab_width = (int)(cross_tab_width + (wordle_tab_width - cross_tab_width) * progress);
        
        // Interpolate color from yellow to green
        float inv_progress = 1.0f - progress;
        animated_tab_color = (Color){
            (unsigned char)(WORDLE_YELLOW.r * inv_progress + WORDLE_GREEN.r * progress),
            (unsigned char)(WORDLE_YELLOW.g * inv_progress + WORDLE_GREEN.g * progress),
            (unsigned char)(WORDLE_YELLOW.b * inv_progress + WORDLE_GREEN.b * progress),
            255
        };
        
        // Text colors transition
        cross_text_color = (Color){
            (unsigned char)(WORDLE_WHITE.r * inv_progress + WORDLE_GRAY.r * progress),
            (unsigned char)(WORDLE_WHITE.g * inv_progress + WORDLE_GRAY.g * progress),
            (unsigned char)(WORDLE_WHITE.b * inv_progress + WORDLE_GRAY.b * progress),
            255
        };
        wordle_text_color = (Color){
            (unsigned char)(WORDLE_GRAY.r * inv_progress + WORDLE_WHITE.r * progress),
            (unsigned char)(WORDLE_GRAY.g * inv_progress + WORDLE_WHITE.g * progress),
            (unsigned char)(WORDLE_GRAY.b * inv_progress + WORDLE_WHITE.b * progress),
            255
        };
    }
    
    // Draw the full top bar background during transitions
    int top_bar_height = title_y + title_font_size + 20;  // 20px bottom padding
    Rectangle top_bar = {0, 0, screen_width, top_bar_height};
    DrawRectangleRec(top_bar, WORDLE_BLACK);
    DrawRectangleLinesEx(top_bar, 2, WORDLE_WHITE);
    
    // Draw the animated tab background
    Rectangle animated_tab_bg = {
        animated_tab_x,
        title_y - tab_padding,
        animated_tab_width,
        title_font_size + 2 * tab_padding
    };
    DrawRectangleRounded(animated_tab_bg, 0.3f, 6, animated_tab_color);
    
    // Draw the tab text with animated colors
    DrawText("CROSS", title_x, title_y, title_font_size, cross_text_color);
    DrawText(" ", title_x + cross_width, title_y, title_font_size, WORDLE_WHITE);
    DrawText("WORDLE", title_x + cross_width + space_width, title_y, title_font_size, wordle_text_color);
}

static inline void board_render_system(GameState state) {
    LayoutConfig layout = calculate_layout(state);
    
    // Render all rows (completed guesses + current input row)
    for (int row = 0; row < layout.total_rows; row++) {
        int row_y = layout.board_start_y + row * layout.row_height;
        
        // Skip rendering rows that are off-screen (simple culling)
        if (row_y + layout.cell_size < 0 || row_y > layout.screen_height) {
            continue;
        }
        
        // Skip rendering input row when level is complete
        if (row == layout.current_input_row && state.core.level_complete) {
            continue;
        }
        
        for (int col = 0; col < WORD_LENGTH; col++) {
            int cell_x = layout.board_start_x + col * (layout.cell_size + layout.cell_spacing);
            int cell_y = row_y;
            
            Color cell_color = WORDLE_BLACK;  // Default to black background for empty cells
            Color border_color = WORDLE_WHITE;  // White border for empty cells
            int border_width = 2;
            char letter_to_display = '\0';
            Color text_color = WORDLE_WHITE;  // White text for visibility on black background
            
            // Determine what to display based on row type
            if (row < state.history.level_guess_count) {
                // Completed guess row
                cell_color = get_color_for_letter_state(state.history.level_letter_states[row][col]);
                border_color = cell_color;
                border_width = 0;
                letter_to_display = state.history.level_guesses[row][col];
                text_color = WORDLE_WHITE;
                
            } else if (row == layout.current_input_row && state.ui.letter_revealing) {
                // Letter reveal animation in progress - show current guess being revealed
                float elapsed = state.ui.letter_reveal_timer;
                float letter_reveal_time = col * LETTER_REVEAL_STAGGER;
                
                if (elapsed >= letter_reveal_time) {
                    // This letter should be revealed
                    float letter_progress = (elapsed - letter_reveal_time) / LETTER_REVEAL_STAGGER;
                    if (letter_progress > 1.0f) letter_progress = 1.0f;
                    
                    // Apply flip animation effect
                    float flip_scale = 1.0f;
                    if (letter_progress < 0.5f) {
                        // First half: scale down (hiding)
                        flip_scale = 1.0f - (letter_progress * 2.0f);
                        cell_color = WORDLE_INPUT;
                        letter_to_display = state.history.current_guess[col];
                        text_color = WORDLE_WHITE;
                    } else {
                        // Second half: scale up (revealing)
                        flip_scale = (letter_progress - 0.5f) * 2.0f;
                        cell_color = get_color_for_letter_state(state.history.current_guess_states[col]);
                        letter_to_display = state.history.current_guess[col];
                        text_color = WORDLE_WHITE;
                    }
                    
                    border_color = cell_color;
                    border_width = 0;
                } else {
                    // Letter not yet revealed - show as input
                    cell_color = WORDLE_INPUT;
                    border_color = WORDLE_DARK_GRAY;
                    letter_to_display = state.history.current_guess[col];
                    text_color = WORDLE_WHITE;
                }
                
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
            
            // Draw the cell (check if we're animating this specific cell)
            int draw_cell_x = cell_x;
            int draw_cell_y = cell_y; 
            int draw_cell_size = layout.cell_size;
            
            // Check for letter pop animation (typing feedback)
            if (row == layout.current_input_row && 
                state.core.play_state == GAME_STATE_INPUT && 
                col < state.input.current_letter_pos &&
                state.ui.letter_pop_timers[col] > 0.0f) {
                
                float pop_progress = 1.0f - (state.ui.letter_pop_timers[col] / LETTER_POP_DURATION);
                float pop_scale = 1.0f + (easeInOutQuad(pop_progress) * (LETTER_POP_SCALE - 1.0f));
                
                draw_cell_size = (int)(layout.cell_size * pop_scale);
                int offset = (layout.cell_size - draw_cell_size) / 2;
                draw_cell_x = cell_x + offset;
                draw_cell_y = cell_y + offset;
            }
            
            // Check for letter ease animation (success feedback)
            if (row < state.history.level_guess_count && state.ui.letter_easing) {
                float ease_progress = state.ui.letter_ease_timer / LETTER_EASE_DURATION;
                if (ease_progress <= 1.0f) {
                    // Ease in-out animation: scale up then back down
                    float ease_scale = 1.0f;
                    if (ease_progress < 0.5f) {
                        // First half: ease in (scale up)
                        ease_scale = 1.0f + (easeInOutQuad(ease_progress * 2.0f) * (LETTER_EASE_SCALE - 1.0f));
                    } else {
                        // Second half: ease out (scale down)
                        ease_scale = LETTER_EASE_SCALE - (easeInOutQuad((ease_progress - 0.5f) * 2.0f) * (LETTER_EASE_SCALE - 1.0f));
                    }
                    
                    draw_cell_size = (int)(layout.cell_size * ease_scale);
                    int offset = (layout.cell_size - draw_cell_size) / 2;
                    draw_cell_x = cell_x + offset;
                    draw_cell_y = cell_y + offset;
                }
            }
            
            // Check for cursor pulse animation
            if (row == layout.current_input_row && 
                state.core.play_state == GAME_STATE_INPUT && 
                col == state.input.current_letter_pos &&
                state.input.current_letter_pos < WORD_LENGTH) {
                
                // Add pulsing border effect for cursor
                float pulse = (sin(state.ui.cursor_pulse_timer) + 1.0f) / 2.0f; // 0 to 1
                int pulse_border_width = (int)(3 + pulse * 2); // 3 to 5 pixels
                border_color = (Color){0, 150, 255, 255};  // Bright blue border (same as crossword)
                border_width = pulse_border_width;
            }
            
            // Check if this cell is being animated during letter reveal
            if (row == layout.current_input_row && state.ui.letter_revealing) {
                float elapsed = state.ui.letter_reveal_timer;
                float letter_reveal_time = col * LETTER_REVEAL_STAGGER;
                
                if (elapsed >= letter_reveal_time) {
                    float letter_progress = (elapsed - letter_reveal_time) / LETTER_REVEAL_STAGGER;
                    if (letter_progress > 1.0f) letter_progress = 1.0f;
                    
                    if (letter_progress <= 1.0f) {
                        float flip_scale = 1.0f;
                        if (letter_progress < 0.5f) {
                            flip_scale = 1.0f - (letter_progress * 2.0f);
                        } else {
                            flip_scale = (letter_progress - 0.5f) * 2.0f;
                        }
                        
                        draw_cell_size = (int)(layout.cell_size * flip_scale);
                        int offset = (layout.cell_size - draw_cell_size) / 2;
                        draw_cell_x = cell_x + offset;
                        draw_cell_y = cell_y + offset;
                    }
                }
            }
            
            DrawRectangle(draw_cell_x, draw_cell_y, draw_cell_size, draw_cell_size, cell_color);
            
            if (border_width > 0) {
                DrawRectangleLinesEx((Rectangle){draw_cell_x, draw_cell_y, draw_cell_size, draw_cell_size}, border_width, border_color);
            }
            
            // Draw the letter
            if (letter_to_display != '\0') {
                char letter_string[2] = {letter_to_display, '\0'};
                int font_size = (int)(draw_cell_size * 0.45f);
                int text_width = MeasureText(letter_string, font_size);
                int text_x = draw_cell_x + (draw_cell_size - text_width) / 2;
                int text_y = draw_cell_y + (draw_cell_size - font_size) / 2;
                
                DrawText(letter_string, text_x, text_y, font_size, text_color);
            }
        }
    }
}

static inline void ui_render_system(GameState state) {
    LayoutConfig layout = calculate_layout(state);
    
    // Calculate main title dimensions
    int title_font_size = (int)(layout.screen_width * 0.05f);
    if (title_font_size < 24) title_font_size = 24;
    if (title_font_size > 48) title_font_size = 48;
    
    // Calculate tab-style title dimensions (just "CROSS WORDLE")
    int cross_width = MeasureText("CROSS", title_font_size);
    int space_width = MeasureText(" ", title_font_size);
    int wordle_width = MeasureText("WORDLE", title_font_size);
    
    int title_total_width = cross_width + space_width + wordle_width;
    int title_x = (layout.screen_width - title_total_width) / 2;
    int title_y = 20;  // Top margin
    
    // Calculate level info for separate row
    char level_info[50];
    sprintf(level_info, "LEVEL %d", state.core.current_level);
    int level_font_size = (int)(layout.screen_width * 0.035f);
    if (level_font_size < 20) level_font_size = 20;
    if (level_font_size > 32) level_font_size = 32;
    
    int level_width = MeasureText(level_info, level_font_size);
    int level_x = (layout.screen_width - level_width) / 2;
    int level_y = title_y + title_font_size + 10;
    
    // Calculate debug info if present
    int debug_font_size = 0;
    int debug_y = 0;
    char debug_message[50] = "";
    if (state.system.debug_mode) {
        sprintf(debug_message, "DEBUG: Answer is %s", state.core.target_word);
        debug_font_size = (int)(layout.screen_width * 0.025f);
        if (debug_font_size < 16) debug_font_size = 16;
        if (debug_font_size > 24) debug_font_size = 24;
        debug_y = level_y + level_font_size + 8;
    }
    
    // Calculate level stats
    char level_stats[100];
    sprintf(level_stats, "Guess %d this level | %d total guesses", 
            state.core.guesses_this_level + 1, state.core.total_lifetime_guesses);
    int stats_font_size = (int)(layout.screen_width * 0.025f);
    if (stats_font_size < 16) stats_font_size = 16;
    if (stats_font_size > 22) stats_font_size = 22;
    
    int stats_width = MeasureText(level_stats, stats_font_size);
    int stats_x = (layout.screen_width - stats_width) / 2;
    int stats_y = (state.system.debug_mode) ? debug_y + debug_font_size + 8 : level_y + level_font_size + 8;
    
    // Calculate top bar height (only title)
    int top_bar_height = title_y + title_font_size + 20;  // 20px bottom padding
    
    // Draw full-width black top bar with white border (skip during transitions)
    if (!state.ui.transitioning_view) {
        Rectangle top_bar = {0, 0, layout.screen_width, top_bar_height};
        DrawRectangleRec(top_bar, WORDLE_BLACK);
        DrawRectangleLinesEx(top_bar, 2, WORDLE_WHITE);
    }
    
    // Draw tab-style title with active highlighting (skip during transitions)
    if (!state.ui.transitioning_view) {
        int tab_padding = 8;
        
        // Draw CROSS tab (inactive)
        Color cross_color = WORDLE_GRAY;
        DrawText("CROSS", title_x, title_y, title_font_size, cross_color);
        
        // Draw space
        DrawText(" ", title_x + cross_width, title_y, title_font_size, WORDLE_WHITE);
        
        // Draw WORDLE tab (active - with green background)
        Rectangle wordle_tab_bg = {
            title_x + cross_width + space_width - tab_padding,
            title_y - tab_padding,
            wordle_width + 2 * tab_padding,
            title_font_size + 2 * tab_padding
        };
        DrawRectangleRounded(wordle_tab_bg, 0.3f, 6, WORDLE_GREEN);
        DrawText("WORDLE", title_x + cross_width + space_width, title_y, title_font_size, WORDLE_WHITE);
    }
    
    // Draw level info (not part of the black top bar)
    DrawText(level_info, level_x, level_y, level_font_size, WORDLE_DARK_GRAY);
    
    // Draw debug mode indicator if active
    if (state.system.debug_mode) {
        DrawText(debug_message, (layout.screen_width - MeasureText(debug_message, debug_font_size)) / 2, 
                debug_y, debug_font_size, RED);
    }
    
    // Draw level stats
    DrawText(level_stats, stats_x, stats_y, stats_font_size, WORDLE_GRAY);
    
    // Show level complete message instead of board when appropriate
    if (state.core.level_complete && state.ui.show_level_complete) {
        char message[50];
        sprintf(message, "NICE! WORD WAS: %s", state.core.target_word);
        int message_font_size = (int)(layout.screen_width * 0.04f);
        if (message_font_size < 20) message_font_size = 20;
        if (message_font_size > 40) message_font_size = 40;
        
        int message_width = MeasureText(message, message_font_size);
        int message_x = (layout.screen_width - message_width) / 2;
        int message_y = layout.screen_height / 2 - message_font_size / 2;
        
        DrawText(message, message_x, message_y, message_font_size, WORDLE_GREEN);
        
        // Add prompt to continue
        const char* prompt = "Press SPACE to continue";
        int prompt_font_size = (int)(layout.screen_width * 0.025f);
        if (prompt_font_size < 16) prompt_font_size = 16;
        if (prompt_font_size > 24) prompt_font_size = 24;
        
        int prompt_width = MeasureText(prompt, prompt_font_size);
        int prompt_x = (layout.screen_width - prompt_width) / 2;
        int prompt_y = message_y + message_font_size + 20;
        
        DrawText(prompt, prompt_x, prompt_y, prompt_font_size, WORDLE_GRAY);
    }
    
    // Show game over message when game is over
    if (state.core.play_state == GAME_STATE_LEVEL_COMPLETE) {
        const char* message = "GAME OVER!";
        int message_font_size = (int)(layout.screen_width * 0.06f);
        if (message_font_size < 30) message_font_size = 30;
        if (message_font_size > 60) message_font_size = 60;
        
        int message_width = MeasureText(message, message_font_size);
        int message_x = (layout.screen_width - message_width) / 2;
        int message_y = layout.screen_height / 2 - message_font_size / 2;
        
        DrawText(message, message_x, message_y, message_font_size, RED);
    }
    
    // Draw scrolling hint
    if (state.history.level_guess_count > 5 && !state.core.level_complete) {
        const char* scroll_hint = "Use UP/DOWN arrows or mouse wheel to scroll";
        int hint_font_size = (int)(layout.screen_width * 0.02f);
        if (hint_font_size < 14) hint_font_size = 14;
        if (hint_font_size > 20) hint_font_size = 20;
        
        int hint_width = MeasureText(scroll_hint, hint_font_size);
        int hint_x = (layout.screen_width - hint_width) / 2;
        int hint_y = layout.screen_height - 30;
        
        DrawText(scroll_hint, hint_x, hint_y, hint_font_size, WORDLE_GRAY);
    }
    
    // Draw letter bag debug display if enabled
    if (state.stats.show_letter_bag) {
        // Background for letter bag
        int bag_x = 20;
        int bag_y = layout.screen_height - 200;
        int bag_width = 300;
        int bag_height = 180;
        
        DrawRectangle(bag_x, bag_y, bag_width, bag_height, (Color){0, 0, 0, 200});
        DrawRectangleLinesEx((Rectangle){bag_x, bag_y, bag_width, bag_height}, 2, WORDLE_WHITE);
        
        // Title
        DrawText("Letter Tokens", bag_x + 10, bag_y + 10, 16, WORDLE_WHITE);
        
        // Display letters in a grid
        int letter_size = 20;
        int letter_spacing = 5;
        int grid_start_x = bag_x + 10;
        int grid_start_y = bag_y + 35;
        
        for (int i = 0; i < 26; i++) {
            int count = state.stats.letter_counts[i];
            if (count > 0) {
                int row = i / 8;  // 8 letters per row
                int col = i % 8;
                
                int x = grid_start_x + col * (letter_size + letter_spacing);
                int y = grid_start_y + row * (letter_size + letter_spacing);
                
                // Draw letter background
                DrawRectangle(x, y, letter_size, letter_size, WORDLE_GREEN);
                
                // Draw letter
                char letter[2] = {'A' + i, '\0'};
                int text_width = MeasureText(letter, 12);
                DrawText(letter, x + (letter_size - text_width) / 2, y + 2, 12, WORDLE_WHITE);
                
                // Draw count
                char count_str[10];
                sprintf(count_str, "%d", count);
                int count_width = MeasureText(count_str, 10);
                DrawText(count_str, x + (letter_size - count_width) / 2, y + 10, 10, WORDLE_BLACK);
            }
        }
        
        // Total count
        int total = 0;
        for (int i = 0; i < 26; i++) {
            total += state.stats.letter_counts[i];
        }
        char total_str[50];
        sprintf(total_str, "Total: %d tokens", total);
        DrawText(total_str, bag_x + 10, bag_y + bag_height - 25, 14, WORDLE_WHITE);
    }
}

static inline void crossword_render_system(GameState state) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    
    // Calculate layout dimensions
    CrosswordLayoutConfig layout = {0};
    layout.grid_width = 9;
    layout.grid_height = 9;
    
    // Calculate cell size based on screen dimensions
    int max_grid_width = screen_width - 100;
    int max_grid_height = screen_height - 200;  // Leave room for UI
    
    layout.cell_size = max_grid_width / layout.grid_width;
    int cell_size_by_height = max_grid_height / layout.grid_height;
    if (cell_size_by_height < layout.cell_size) {
        layout.cell_size = cell_size_by_height;
    }
    
    // Clamp cell size
    if (layout.cell_size < 40) layout.cell_size = 40;
    if (layout.cell_size > 80) layout.cell_size = 80;
    
    layout.grid_pixel_width = layout.grid_width * layout.cell_size;
    layout.grid_pixel_height = layout.grid_height * layout.cell_size;
    
    layout.grid_start_x = (screen_width - layout.grid_pixel_width) / 2;
    layout.grid_start_y = 120;  // Below the title and UI
    
    // Draw title with tab highlighting
    int title_font_size = (int)(screen_width * 0.05f);
    if (title_font_size < 24) title_font_size = 24;
    if (title_font_size > 48) title_font_size = 48;
    
    // Calculate tab-style title dimensions (just "CROSS WORDLE")
    int cross_width = MeasureText("CROSS", title_font_size);
    int space_width = MeasureText(" ", title_font_size);
    int wordle_width = MeasureText("WORDLE", title_font_size);
    
    int title_total_width = cross_width + space_width + wordle_width;
    int title_x = (screen_width - title_total_width) / 2;
    int title_y = 20;  // Top margin
    
    // Calculate top bar height (only title)
    int top_bar_height = title_y + title_font_size + 20;  // 20px bottom padding
    
    // Draw full-width black top bar with white border (skip during transitions)
    if (!state.ui.transitioning_view) {
        Rectangle top_bar = {0, 0, screen_width, top_bar_height};
        DrawRectangleRec(top_bar, WORDLE_BLACK);
        DrawRectangleLinesEx(top_bar, 2, WORDLE_WHITE);
    }
    
    // Draw tab-style title with active highlighting (skip during transitions)
    if (!state.ui.transitioning_view) {
        int tab_padding = 8;
        
        // Draw CROSS tab (active - with yellow background)
        Rectangle cross_tab_bg = {
            title_x - tab_padding,
            title_y - tab_padding,
            cross_width + 2 * tab_padding,
            title_font_size + 2 * tab_padding
        };
        DrawRectangleRounded(cross_tab_bg, 0.3f, 6, WORDLE_YELLOW);
        DrawText("CROSS", title_x, title_y, title_font_size, WORDLE_WHITE);
        
        // Draw space
        DrawText(" ", title_x + cross_width, title_y, title_font_size, WORDLE_WHITE);
        
        // Draw WORDLE tab (inactive)
        Color wordle_color = WORDLE_GRAY;
        DrawText("WORDLE", title_x + cross_width + space_width, title_y, title_font_size, wordle_color);
    }
    
    // Draw subtitle
    const char* subtitle = "Use letter tokens to solve the crossword!";
    int subtitle_font_size = (int)(screen_width * 0.025f);
    if (subtitle_font_size < 16) subtitle_font_size = 16;
    if (subtitle_font_size > 24) subtitle_font_size = 24;
    
    int subtitle_width = MeasureText(subtitle, subtitle_font_size);
    DrawText(subtitle, (screen_width - subtitle_width) / 2, title_y + title_font_size + 25, subtitle_font_size, WORDLE_GRAY);
    
    // Draw debug info if enabled
    if (state.system.debug_mode) {
        // Draw solution grid on the right side
        int debug_x = layout.grid_start_x + layout.grid_pixel_width + 40;
        int debug_y = layout.grid_start_y;
        
        DrawText("SOLUTION:", debug_x, debug_y - 25, 16, RED);
        
        for (int y = 0; y < layout.grid_height; y++) {
            for (int x = 0; x < layout.grid_width; x++) {
                char solution_letter = state.crossword.level.solution[x][y];
                if (solution_letter != '\0') {
                    int cell_x = debug_x + x * 20;
                    int cell_y = debug_y + y * 20;
                    
                    DrawRectangle(cell_x, cell_y, 18, 18, WORDLE_GRAY);
                    
                    char letter_str[2] = {solution_letter, '\0'};
                    DrawText(letter_str, cell_x + 4, cell_y + 2, 14, WORDLE_WHITE);
                }
            }
        }
    }
    
    // Draw the crossword grid
    for (int y = 0; y < layout.grid_height; y++) {
        for (int x = 0; x < layout.grid_width; x++) {
            int cell_x = layout.grid_start_x + x * layout.cell_size;
            int cell_y = layout.grid_start_y + y * layout.cell_size;
            
            // Check if this is a word cell
            if (state.crossword.level.word_mask[x][y]) {
                char placed_letter = state.crossword.grid[x][y];
                Color cell_color = WORDLE_WHITE;
                Color text_color = WORDLE_BLACK;
                int border_width = 2;
                Color border_color = WORDLE_GRAY;
                
                // Determine cell color based on validation state
                if (placed_letter != '\0' && state.crossword.word_validated[x][y]) {
                    // Cell has been validated - use Wordle colors
                    LetterState letter_state = state.crossword.letter_states[x][y];
                    cell_color = get_color_for_letter_state(letter_state);
                    text_color = WORDLE_WHITE;
                    border_width = 0;
                } else if (placed_letter != '\0') {
                    // Cell has a letter but not validated yet
                    cell_color = GRAY;
                    text_color = WORDLE_BLACK;
                }
                
                // Highlight cursor position
                if (x == state.crossword.cursor_x && y == state.crossword.cursor_y) {
                    border_color = (Color){0, 150, 255, 255};  // Bright blue
                    border_width = 4;
                }
                
                // Draw cell background
                DrawRectangle(cell_x, cell_y, layout.cell_size, layout.cell_size, cell_color);
                
                // Draw border
                DrawRectangleLinesEx((Rectangle){cell_x, cell_y, layout.cell_size, layout.cell_size}, border_width, border_color);
                
                // Draw letter if present
                if (placed_letter != '\0') {
                    char letter_str[2] = {placed_letter, '\0'};
                    int font_size = (int)(layout.cell_size * 0.6f);
                    int text_width = MeasureText(letter_str, font_size);
                    int text_x = cell_x + (layout.cell_size - text_width) / 2;
                    int text_y = cell_y + (layout.cell_size - font_size) / 2;
                    
                    DrawText(letter_str, text_x, text_y, font_size, text_color);
                }
            } else {
                // Empty cell (not part of any word)
                DrawRectangle(cell_x, cell_y, layout.cell_size, layout.cell_size, WORDLE_BLACK);
            }
        }
    }
    
    // Draw direction indicator
    const char* direction_text = state.crossword.direction == 0 ? "HORIZONTAL" : "VERTICAL";
    int direction_font_size = 20;
    int direction_y = layout.grid_start_y + layout.grid_pixel_height + 20;
    
    DrawText("Direction: ", (screen_width - 200) / 2, direction_y, direction_font_size, WORDLE_GRAY);
    DrawText(direction_text, (screen_width - 200) / 2 + 100, direction_y, direction_font_size, WORDLE_GREEN);
    
    // Draw controls hint
    const char* controls = "Arrows: Move | Shift: Change Direction | Enter: Validate Word";
    int controls_font_size = 16;
    int controls_width = MeasureText(controls, controls_font_size);
    DrawText(controls, (screen_width - controls_width) / 2, direction_y + 30, controls_font_size, WORDLE_GRAY);
    
    // Draw error message if present
    if (state.crossword.show_error && state.crossword.error_message[0] != '\0') {
        int error_font_size = 20;
        int error_width = MeasureText(state.crossword.error_message, error_font_size);
        DrawText(state.crossword.error_message, (screen_width - error_width) / 2, 
                direction_y + 60, error_font_size, RED);
    }
    
    // Draw letter inventory on the left side
    int inventory_x = 20;
    int inventory_y = layout.grid_start_y;
    int inventory_width = 150;
    
    DrawText("Letter Tokens", inventory_x, inventory_y - 25, 18, WORDLE_WHITE);
    DrawRectangle(inventory_x, inventory_y, inventory_width, 400, (Color){0, 0, 0, 180});
    DrawRectangleLinesEx((Rectangle){inventory_x, inventory_y, inventory_width, 400}, 2, WORDLE_WHITE);
    
    // Display letters with counts
    int letter_y = inventory_y + 10;
    int letters_displayed = 0;
    
    for (int i = 0; i < 26; i++) {
        if (state.stats.letter_counts[i] > 0) {
            char letter = 'A' + i;
            char display[20];
            sprintf(display, "%c: %d", letter, state.stats.letter_counts[i]);
            
            Color letter_color = WORDLE_WHITE;
            if (state.stats.letter_counts[i] < 3) {
                letter_color = WORDLE_YELLOW;  // Low inventory warning
            }
            
            DrawText(display, inventory_x + 10, letter_y, 16, letter_color);
            letter_y += 20;
            letters_displayed++;
            
            // Limit display to prevent overflow
            if (letters_displayed >= 18) {
                DrawText("...", inventory_x + 10, letter_y, 16, WORDLE_GRAY);
                break;
            }
        }
    }
    
    // Show total tokens
    int total_tokens = 0;
    for (int i = 0; i < 26; i++) {
        total_tokens += state.stats.letter_counts[i];
    }
    
    char total_str[50];
    sprintf(total_str, "Total: %d", total_tokens);
    DrawText(total_str, inventory_x + 10, inventory_y + 370, 16, WORDLE_GREEN);
}

static inline void home_screen_render_system(GameState state) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    
    // Clear background
    ClearBackground(WORDLE_BLACK);
    
    // Draw main title
    const char* title = "CROSSWORDLE";
    int title_font_size = (int)(screen_width * 0.08f);
    if (title_font_size < 40) title_font_size = 40;
    if (title_font_size > 80) title_font_size = 80;
    
    int title_width = MeasureText(title, title_font_size);
    int title_x = (screen_width - title_width) / 2;
    int title_y = screen_height / 4;
    
    DrawText(title, title_x, title_y, title_font_size, WORDLE_WHITE);
    
    // Draw subtitle
    const char* subtitle = "A Wordle + Crossword Adventure";
    int subtitle_font_size = (int)(screen_width * 0.03f);
    if (subtitle_font_size < 18) subtitle_font_size = 18;
    if (subtitle_font_size > 30) subtitle_font_size = 30;
    
    int subtitle_width = MeasureText(subtitle, subtitle_font_size);
    DrawText(subtitle, (screen_width - subtitle_width) / 2, title_y + title_font_size + 10, subtitle_font_size, WORDLE_GRAY);
    
    // Draw menu options
    int menu_y = screen_height / 2;
    int option_spacing = 60;
    int option_font_size = (int)(screen_width * 0.04f);
    if (option_font_size < 24) option_font_size = 24;
    if (option_font_size > 40) option_font_size = 40;
    
    const char* options[] = {
        "Press ENTER to Start",
        "Press TAB to Switch Modes",
        "Press ESC to Exit"
    };
    
    Color option_colors[] = {
        WORDLE_GREEN,
        WORDLE_YELLOW,
        WORDLE_GRAY
    };
    
    for (int i = 0; i < 3; i++) {
        int option_width = MeasureText(options[i], option_font_size);
        int option_x = (screen_width - option_width) / 2;
        DrawText(options[i], option_x, menu_y + i * option_spacing, option_font_size, option_colors[i]);
    }
    
    // Draw bottom credits
    const char* credits = "Made with Raylib";
    int credits_font_size = 16;
    int credits_width = MeasureText(credits, credits_font_size);
    DrawText(credits, (screen_width - credits_width) / 2, screen_height - 40, credits_font_size, WORDLE_DARK_GRAY);
}

static inline void crossword_completion_render_system(GameState state) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    
    // Clear background with a slightly lighter color for celebration
    ClearBackground((Color){20, 20, 20, 255});
    
    // Draw particles
    GameState mutable_state = state;
    update_particles(&mutable_state, GetFrameTime());
    
    // Enable blending for particles
    rlSetBlendMode(RL_BLEND_ALPHA);
    
    for (int i = 0; i < state.ui.particle_count; i++) {
        if (state.ui.particle_lifetimes[i] > 0) {
            DrawCircle((int)state.ui.particles[i].x, (int)state.ui.particles[i].y, 5, state.ui.particle_colors[i]);
        }
    }
    
    // Draw completion message
    const char* congrats = "CROSSWORD COMPLETE!";
    int congrats_font_size = (int)(screen_width * 0.06f);
    if (congrats_font_size < 36) congrats_font_size = 36;
    if (congrats_font_size > 72) congrats_font_size = 72;
    
    int congrats_width = MeasureText(congrats, congrats_font_size);
    int congrats_x = (screen_width - congrats_width) / 2;
    int congrats_y = screen_height / 3;
    
    // Add pulsing effect to the text
    float pulse = (sin(state.ui.completion_animation_timer * 3.0f) + 1.0f) / 2.0f;
    Color text_color = (Color){
        (unsigned char)(WORDLE_GREEN.r * (0.7f + 0.3f * pulse)),
        (unsigned char)(WORDLE_GREEN.g * (0.7f + 0.3f * pulse)),
        (unsigned char)(WORDLE_GREEN.b * (0.7f + 0.3f * pulse)),
        255
    };
    
    DrawText(congrats, congrats_x, congrats_y, congrats_font_size, text_color);
    
    // Draw stats
    char stats[100];
    sprintf(stats, "Words Solved: %d", state.crossword.level.word_count);
    int stats_font_size = (int)(screen_width * 0.03f);
    if (stats_font_size < 20) stats_font_size = 20;
    if (stats_font_size > 32) stats_font_size = 32;
    
    int stats_width = MeasureText(stats, stats_font_size);
    DrawText(stats, (screen_width - stats_width) / 2, congrats_y + congrats_font_size + 40, stats_font_size, WORDLE_WHITE);
    
    // Draw continue prompt
    const char* prompt = "Press SPACE to continue playing Wordle";
    int prompt_font_size = (int)(screen_width * 0.025f);
    if (prompt_font_size < 18) prompt_font_size = 18;
    if (prompt_font_size > 28) prompt_font_size = 28;
    
    int prompt_width = MeasureText(prompt, prompt_font_size);
    DrawText(prompt, (screen_width - prompt_width) / 2, screen_height * 2 / 3, prompt_font_size, WORDLE_YELLOW);
    
    // Draw decorative elements
    int decoration_y = congrats_y - 40;
    int decoration_width = congrats_width + 100;
    int decoration_x = (screen_width - decoration_width) / 2;
    
    // Top decorative line
    DrawRectangle(decoration_x, decoration_y, decoration_width, 3, WORDLE_YELLOW);
    
    // Bottom decorative line
    DrawRectangle(decoration_x, congrats_y + congrats_font_size + 20, decoration_width, 3, WORDLE_YELLOW);
}

static inline void render_system(GameState state) {
    ClearBackground(WORDLE_BLACK);
    
    // Render based on current view
    switch (state.current_view) {
        case VIEW_HOME_SCREEN:
            home_screen_render_system(state);
            break;
            
        case VIEW_WORDLE:
            board_render_system(state);
            ui_render_system(state);
            break;
            
        case VIEW_CROSSWORD:
            crossword_render_system(state);
            break;
            
        case VIEW_CROSSWORD_COMPLETE:
            crossword_completion_render_system(state);
            break;
    }
    
    // Render view transition animation overlay
    render_animated_tab(state);
}

#endif // RENDER_H