#include "../systems.h"
#include <math.h>

void spawn_particles(GameState* state, Vector2 position, Color color, int count) {
    if (!state->settings.animations_enabled) {
        return;
    }
    
    for (int i = 0; i < count && state->ui.particle_count < 50; i++) {
        int index = state->ui.particle_count;
        state->ui.particles[index] = position;
        
        // Random velocity
        float angle = (float)GetRandomValue(0, 359) * 3.14159265359f / 180.0f;
        float speed = 50.0f + (float)GetRandomValue(0, 99);
        state->ui.particle_velocities[index] = (Vector2){
            cos(angle) * speed,
            sin(angle) * speed - 100.0f // Initial upward bias
        };
        
        state->ui.particle_lifetimes[index] = PARTICLE_LIFETIME;
        state->ui.particle_colors[index] = color;
        state->ui.particle_count++;
    }
}

void update_particles(GameState* state, float frame_time) {
    for (int i = 0; i < state->ui.particle_count; i++) {
        state->ui.particle_lifetimes[i] -= frame_time;
        
        if (state->ui.particle_lifetimes[i] <= 0.0f) {
            // Remove particle by swapping with last particle
            state->ui.particle_count--;
            if (i < state->ui.particle_count) {
                state->ui.particles[i] = state->ui.particles[state->ui.particle_count];
                state->ui.particle_velocities[i] = state->ui.particle_velocities[state->ui.particle_count];
                state->ui.particle_lifetimes[i] = state->ui.particle_lifetimes[state->ui.particle_count];
                state->ui.particle_colors[i] = state->ui.particle_colors[state->ui.particle_count];
                i--; // Check this particle again
            }
            continue;
        }
        
        // Update position
        state->ui.particles[i].x += state->ui.particle_velocities[i].x * frame_time;
        state->ui.particles[i].y += state->ui.particle_velocities[i].y * frame_time;
        
        // Apply gravity
        state->ui.particle_velocities[i].y += PARTICLE_GRAVITY * frame_time;
        
        // Fade particle
        float life_ratio = state->ui.particle_lifetimes[i] / PARTICLE_LIFETIME;
        state->ui.particle_colors[i].a = (unsigned char)(255 * life_ratio);
    }
}