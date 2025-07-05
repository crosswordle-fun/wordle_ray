#ifndef CORE_VALIDATION_H
#define CORE_VALIDATION_H

// Dependencies provided by systems.h

// Validation functions
static inline int is_letter_in_target_word(char letter, const char* target_word) {
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (target_word[i] == letter) {
            return 1;
        }
    }
    return 0;
}

static inline LetterState calculate_letter_state(char guess_letter, int position, const char* target_word) {
    if (guess_letter == target_word[position]) {
        return LETTER_CORRECT;
    } else if (is_letter_in_target_word(guess_letter, target_word)) {
        return LETTER_WRONG_POS;
    } else {
        return LETTER_NOT_IN_WORD;
    }
}

static inline int check_word_match(const char* word1, const char* word2) {
    return strcmp(word1, word2) == 0;
}

#endif