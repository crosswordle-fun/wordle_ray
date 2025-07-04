#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "types.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Include module headers
#include "core/state.h"
#include "core/validation.h"
#include "input/input_manager.h"
#include "ui/camera.h"
#include "ui/view_manager.h"
#include "wordle/wordle_game.h"
#include "crossword/crossword_game.h"
#include "crossword/crossword_navigation.h"
#include "animation/animation_system.h"
#include "animation/animation_triggers.h"
#include "animation/particle_system.h"

// Camera system (not in original declarations but needed)
GameState camera_scrolling_system(GameState state);

// Rendering System Function Declarations (still in render.c)
LayoutConfig calculate_layout(GameState state);
Color get_color_for_letter_state(LetterState state);
void board_render_system(GameState state);
void ui_render_system(GameState state);
void render_system(GameState state);
void crossword_render_system(GameState state);
void home_screen_render_system(GameState state);
void crossword_completion_render_system(GameState state);

// Word System Function Declarations (still in words.c)
void initialize_random_seed(void);
const char* get_random_word(void);
int get_total_words_count(void);
const char* get_word_by_index(int index);

// Crossword Generator Function Declarations (still in crossword_generator.c)
CrosswordLevel generate_crossword(int word_count, int grid_width, int grid_height);
int select_random_words(char selected_words[][WORD_LENGTH + 1], int count);
int find_intersections(const char* word1, const char* word2, IntersectionCandidate candidates[], int max_candidates);
int can_place_word(char grid[][9], int grid_width, int grid_height, const char* word, int start_x, int start_y, int direction);
void place_word_in_grid(char grid[][9], char word_mask[][9], int grid_width, int grid_height, const char* word, int start_x, int start_y, int direction);

#endif