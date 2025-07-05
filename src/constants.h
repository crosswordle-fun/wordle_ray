#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600
#define MIN_CELL_SIZE 50
#define MAX_CELL_SIZE 100
#define CELL_SPACING_RATIO 0.12f
#define BOARD_MARGIN_TOP 120
#define BOARD_MARGIN_BOTTOM 150

// Level system constants
#define MAX_RECENT_GUESSES 20
#define RESULT_DISPLAY_TIME 1.5f

// Animation constants
#define LETTER_REVEAL_DURATION 0.6f          // Total time for letter reveal animation
#define LETTER_REVEAL_STAGGER 0.1f           // Delay between each letter reveal
#define LETTER_POP_DURATION 0.25f            // Duration of letter typing pop animation
#define LETTER_POP_SCALE 1.05f               // Scale factor for letter pop (subtle)
#define CURSOR_PULSE_SPEED 2.0f              // Speed of cursor pulse animation
#define WORD_COMPLETE_DURATION 1.0f          // Duration of word completion celebration
#define LEVEL_COMPLETE_DURATION 2.0f         // Duration of level completion celebration
#define VIEW_TRANSITION_DURATION 0.5f        // Duration of view switching animation
#define LETTER_EASE_DURATION 0.4f            // Duration of letter ease in/out animation
#define LETTER_EASE_SCALE 1.1f               // Scale factor for letter ease animation
#define HOME_TAB_ANIMATION_DURATION 1.0f     // Duration of home screen ping-pong tab animation (1 second)
#define HOME_TAB_PAUSE_DURATION 1.0f         // Pause duration in each mode before animating (1 second)
#define PARTICLE_LIFETIME 1.5f               // Lifetime of celebration particles
#define PARTICLE_GRAVITY 200.0f              // Gravity applied to particles

#define WORDLE_GREEN (Color){106, 170, 100, 255}
#define WORDLE_YELLOW (Color){201, 180, 88, 255}
#define WORDLE_GRAY (Color){120, 124, 126, 255}
#define WORDLE_DARK_GRAY (Color){58, 58, 60, 255}
#define WORDLE_WHITE (Color){255, 255, 255, 255}
#define WORDLE_BLACK (Color){0, 0, 0, 255}
#define WORDLE_BG (Color){18, 18, 19, 255}
#define WORDLE_BORDER (Color){58, 58, 60, 255}
#define WORDLE_INPUT (Color){86, 87, 88, 255}

#endif