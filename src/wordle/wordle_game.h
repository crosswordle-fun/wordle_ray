#ifndef WORDLE_GAME_H
#define WORDLE_GAME_H

#include "../types.h"

// Wordle game systems
GameState word_editing_system(GameState state);
GameState word_validation_system(GameState state);
void complete_word_validation(GameState* state);
GameState result_display_system(GameState state);
GameState level_progression_system(GameState state);
GameState new_level_system(GameState state);

#endif