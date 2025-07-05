#ifndef CROSSWORD_GAME_H
#define CROSSWORD_GAME_H

// Dependencies provided by systems.h

// Crossword game systems
static inline GameState crossword_input_system(GameState state) {
    // Basic stub - just return state unchanged for now
    return state;
}

static inline CrosswordLevel get_crossword_level(int level) {
    // Generate a simple crossword for the given level
    return generate_crossword(2, 9, 9);
}

static inline GameState crossword_word_validation_system(GameState state) {
    // Basic stub - just return state unchanged for now
    return state;
}

#endif