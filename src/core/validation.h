#ifndef CORE_VALIDATION_H
#define CORE_VALIDATION_H

#include "../types.h"

// Validation functions
int is_letter_in_target_word(char letter, const char* target_word);
LetterState calculate_letter_state(char guess_letter, int position, const char* target_word);
int check_word_match(const char* word1, const char* word2);

#endif