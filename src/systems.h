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

// Crossword System Function Declarations
CrosswordLevel get_crossword_level(int level);

// Game Logic Helper Function Declarations
int is_letter_in_target_word(char letter, const char* target_word);
LetterState calculate_letter_state(char guess_letter, int position, const char* target_word);
int check_word_match(const char* word1, const char* word2);

// Rendering System Function Declarations
LayoutConfig calculate_layout(GameState state);
Color get_color_for_letter_state(LetterState state);
void board_render_system(GameState state);
void ui_render_system(GameState state);
void render_system(GameState state);
void crossword_render_system(GameState state);

// Word System Function Declarations
void initialize_random_seed(void);
const char* get_random_word(void);

#endif