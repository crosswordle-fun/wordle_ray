#ifndef SYSTEMS_H
#define SYSTEMS_H

// Standard library headers
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Raylib headers
#include "raylib.h"
#include "rlgl.h"

// Project headers in dependency order
#include "constants.h"
#include "types.h"

// Core headers
#include "core/validation.h"
#include "core/state.h"

// UI headers
#include "ui/camera.h"
#include "ui/view_manager.h"

// Animation headers
#include "animation/particle_system.h"
#include "animation/animation_triggers.h"
#include "animation/animation_system.h"

// Data headers (must come first - provide functions used by other modules)
#include "words.h"
#include "crossword_generator.h"

// Game mode headers
#include "crossword/crossword_navigation.h"
#include "crossword/crossword_game.h"
#include "wordle/wordle_game.h"

// Input headers
#include "input/input_manager.h"

// Rendering headers
#include "render.h"

#endif