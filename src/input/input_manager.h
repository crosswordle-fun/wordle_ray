#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "../types.h"

// Input system functions
GameState input_system(GameState state);
GameState home_screen_input_system(GameState state);
GameState crossword_completion_input_system(GameState state);

#endif