#ifndef ANIMATION_TRIGGERS_H
#define ANIMATION_TRIGGERS_H

#include "../types.h"

// Animation trigger functions
void trigger_letter_pop(GameState* state, int letter_index);
void trigger_word_celebration(GameState* state);
void trigger_level_celebration(GameState* state);
void trigger_letter_ease(GameState* state);

#endif