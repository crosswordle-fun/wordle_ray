#ifndef CROSSWORD_GAME_H
#define CROSSWORD_GAME_H

#include "../types.h"

// Crossword game systems
GameState crossword_input_system(GameState state);
CrosswordLevel get_crossword_level(int level);
GameState crossword_word_validation_system(GameState state);

#endif