#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include "../types.h"

// Animation system functions
GameState animation_update_system(GameState state);
float easeInOutQuad(float t);
float easeOutElastic(float t);

#endif