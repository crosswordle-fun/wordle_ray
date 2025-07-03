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