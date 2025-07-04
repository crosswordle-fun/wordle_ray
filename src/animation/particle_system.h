#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "../types.h"

// Particle system functions
void spawn_particles(GameState* state, Vector2 position, Color color, int count);
void update_particles(GameState* state, float frame_time);

#endif