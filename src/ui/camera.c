#include "../systems.h"

GameState camera_scrolling_system(GameState state) {
    // Handle scrolling (only in Wordle view)
    if ((state.system.scroll_wheel_move != 0 || state.system.up_arrow_pressed || state.system.down_arrow_pressed) && state.current_view == VIEW_WORDLE) {
        float scroll_amount = 0.0f;
        
        if (state.system.scroll_wheel_move > 0 || state.system.up_arrow_pressed) {
            scroll_amount = 60.0f;  // Scroll up (positive offset to see older guesses)
        } else if (state.system.scroll_wheel_move < 0 || state.system.down_arrow_pressed) {
            scroll_amount = -60.0f;  // Scroll down (negative offset toward current input)
        }
        
        state.system.camera_offset_y += scroll_amount;
        
        // Calculate dynamic scroll bounds based on current game state
        int total_rows = state.history.level_guess_count + 1;  // completed guesses + current input
        int screen_height = GetScreenHeight();
        
        // Use actual row height from layout calculation
        int available_width = GetScreenWidth() - 100;
        int cell_size = available_width / (WORD_LENGTH + (WORD_LENGTH - 1) * 0.12f);
        if (cell_size < 50) cell_size = 50;
        if (cell_size > 100) cell_size = 100;
        int cell_spacing = (int)(cell_size * 0.12f);
        int actual_row_height = cell_size + cell_spacing;
        
        // To center attempt 1 (row 0):
        // We want: board_start_y + 0 * row_height = screen_height/2 - cell_size/2
        // From calculate_layout: board_start_y = desired_input_y - input_row_y + camera_offset_y
        // Where: desired_input_y = screen_height/2 - cell_size/2 (center of screen)
        // And: input_row_y = current_input_row * row_height
        // So: camera_offset_y = (screen_height/2 - cell_size/2) - (desired_input_y - input_row_y)
        // Simplifying: camera_offset_y = input_row_y = level_guess_count * row_height
        float max_scroll_up = state.history.level_guess_count * actual_row_height;
        
        // Allow scrolling down until input row is centered on screen
        // Input row centered is the natural state (camera_offset_y = 0)
        float max_scroll_down = 0.0f;
        
        // Apply bounds
        if (state.system.camera_offset_y > max_scroll_up) {
            state.system.camera_offset_y = max_scroll_up;
        }
        if (state.system.camera_offset_y < max_scroll_down) {
            state.system.camera_offset_y = max_scroll_down;
        }
        
        // User has manually scrolled - pause auto-centering
        state.system.user_has_scrolled = 1;
        state.system.auto_center_paused = 1;
        // Set target to current position to prevent camera interpolation from fighting
        state.system.target_camera_offset_y = state.system.camera_offset_y;
    }
    
    // Smooth camera interpolation toward target (only when not paused)
    if (!state.system.auto_center_paused) {
        float camera_lerp_speed = 4.0f;  // Reduced for smoother movement
        state.system.camera_offset_y += (state.system.target_camera_offset_y - state.system.camera_offset_y) * camera_lerp_speed * state.system.frame_time;
    }
    
    return state;
}