#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "types.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Game System Function Declarations
GameState create_game_state(const char* target_word);
GameState input_system(GameState state);
GameState word_editing_system(GameState state);
GameState word_validation_system(GameState state);
GameState level_progression_system(GameState state);
GameState result_display_system(GameState state);
GameState new_level_system(GameState state);
GameState view_switching_system(GameState state);
GameState crossword_input_system(GameState state);
GameState crossword_word_validation_system(GameState state);
GameState home_screen_input_system(GameState state);
GameState crossword_completion_input_system(GameState state);

// Crossword System Function Declarations
CrosswordLevel get_crossword_level(int level);

// Game Logic Helper Function Declarations
int is_letter_in_target_word(char letter, const char* target_word);
LetterState calculate_letter_state(char guess_letter, int position, const char* target_word);
int check_word_match(const char* word1, const char* word2);

// Crossword Helper Function Declarations
int find_first_editable_cell_in_word(GameState state, int word_index, int* out_x, int* out_y);
int find_next_incomplete_word(GameState state);
int find_previous_editable_cell_with_letter(GameState state, int current_x, int current_y, int* out_x, int* out_y);

// Animation System Function Declarations
GameState animation_update_system(GameState state);
float easeInOutQuad(float t);
float easeOutElastic(float t);
void trigger_letter_pop(GameState* state, int letter_index);
void trigger_word_celebration(GameState* state);
void trigger_level_celebration(GameState* state);
void trigger_letter_ease(GameState* state);
void spawn_particles(GameState* state, Vector2 position, Color color, int count);
void update_particles(GameState* state, float frame_time);
void complete_word_validation(GameState* state);

// Rendering System Function Declarations
LayoutConfig calculate_layout(GameState state);
Color get_color_for_letter_state(LetterState state);
void board_render_system(GameState state);
void ui_render_system(GameState state);
void render_system(GameState state);
void crossword_render_system(GameState state);
void home_screen_render_system(GameState state);
void crossword_completion_render_system(GameState state);

// Word System Function Declarations
void initialize_random_seed(void);
const char* get_random_word(void);
int get_total_words_count(void);
const char* get_word_by_index(int index);

// Crossword Generator Function Declarations
CrosswordLevel generate_crossword(int word_count, int grid_width, int grid_height);
int select_random_words(char selected_words[][WORD_LENGTH + 1], int count);
int find_intersections(const char* word1, const char* word2, IntersectionCandidate candidates[], int max_candidates);
int can_place_word(char grid[][9], int grid_width, int grid_height, const char* word, int start_x, int start_y, int direction);
void place_word_in_grid(char grid[][9], char word_mask[][9], int grid_width, int grid_height, const char* word, int start_x, int start_y, int direction);

#endif