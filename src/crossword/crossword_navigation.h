#ifndef CROSSWORD_NAVIGATION_H
#define CROSSWORD_NAVIGATION_H

#include "../types.h"

// Crossword navigation helpers
int find_first_editable_cell_in_word(GameState state, int word_index, int* out_x, int* out_y);
int find_next_incomplete_word(GameState state);
int find_previous_editable_cell_with_letter(GameState state, int current_x, int current_y, int* out_x, int* out_y);

#endif