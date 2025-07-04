#include "systems.h"
#include <string.h>

// Maximum number of words we can handle in a crossword
#define MAX_CROSSWORD_WORDS 10
#define MAX_INTERSECTIONS 50

// Helper function to check if two words can intersect (share at least one letter)
int can_words_intersect(const char* word1, const char* word2) {
    for (int i = 0; i < WORD_LENGTH; i++) {
        for (int j = 0; j < WORD_LENGTH; j++) {
            if (word1[i] == word2[j]) {
                return 1; // Found at least one matching letter
            }
        }
    }
    return 0; // No matching letters found
}

// Helper function to select words with unique starting letters AND intersection compatibility
int select_words_with_unique_starting_letters(char selected_words[][WORD_LENGTH + 1], int count) {
    char used_letters[26] = {0}; // Track used starting letters (A-Z)
    int selected_count = 0;
    int total_words = get_total_words_count();
    const int max_attempts_per_word = 500; // Max attempts per word selection
    
    // Select first word randomly (no intersection constraints)
    if (count > 0) {
        int attempts = 0;
        while (selected_count == 0 && attempts < max_attempts_per_word) {
            char target_letter = 'A' + GetRandomValue(0, 25);
            attempts++;
            
            // Skip if we've already used this letter
            if (used_letters[target_letter - 'A']) {
                continue;
            }
            
            // Find words starting with target_letter
            int words_with_letter[50];
            int word_count_for_letter = 0;
            
            for (int i = 0; i < total_words && word_count_for_letter < 50; i++) {
                const char* word = get_word_by_index(i);
                if (word && word[0] == target_letter) {
                    words_with_letter[word_count_for_letter++] = i;
                }
            }
            
            // Select first word randomly from this letter's words
            if (word_count_for_letter > 0) {
                int random_word_index = GetRandomValue(0, word_count_for_letter - 1);
                int selected_word_index = words_with_letter[random_word_index];
                
                const char* word = get_word_by_index(selected_word_index);
                if (word) {
                    strcpy(selected_words[selected_count], word);
                    used_letters[target_letter - 'A'] = 1;
                    selected_count++;
                    printf("Selected first word: %s\n", word);
                }
            }
        }
    }
    
    // Select subsequent words (must intersect with at least one previous word)
    while (selected_count < count) {
        int word_found = 0;
        int attempts = 0;
        
        while (!word_found && attempts < max_attempts_per_word) {
            char target_letter = 'A' + GetRandomValue(0, 25);
            attempts++;
            
            // Skip if we've already used this letter
            if (used_letters[target_letter - 'A']) {
                continue;
            }
            
            // Find words starting with target_letter
            int words_with_letter[50];
            int word_count_for_letter = 0;
            
            for (int i = 0; i < total_words && word_count_for_letter < 50; i++) {
                const char* word = get_word_by_index(i);
                if (word && word[0] == target_letter) {
                    words_with_letter[word_count_for_letter++] = i;
                }
            }
            
            // Try each word starting with this letter
            for (int w = 0; w < word_count_for_letter && !word_found; w++) {
                const char* candidate_word = get_word_by_index(words_with_letter[w]);
                if (!candidate_word) continue;
                
                // Check if this candidate can intersect with ANY previously selected word
                int can_connect = 0;
                for (int prev = 0; prev < selected_count; prev++) {
                    if (can_words_intersect(candidate_word, selected_words[prev])) {
                        can_connect = 1;
                        printf("Found intersection: '%s' can connect to '%s'\n", candidate_word, selected_words[prev]);
                        break;
                    }
                }
                
                // If it can connect, select this word
                if (can_connect) {
                    strcpy(selected_words[selected_count], candidate_word);
                    used_letters[target_letter - 'A'] = 1;
                    selected_count++;
                    word_found = 1;
                    printf("Selected word %d: %s\n", selected_count, candidate_word);
                }
            }
        }
        
        // If we couldn't find a compatible word after many attempts, break
        if (!word_found) {
            printf("Could not find compatible word %d after %d attempts\n", selected_count + 1, max_attempts_per_word);
            break;
        }
    }
    
    return selected_count;
}

// Helper function to select random unique words from WORDLE_WORDS (fallback)
int select_random_words_fallback(char selected_words[][WORD_LENGTH + 1], int count) {
    if (count > MAX_CROSSWORD_WORDS) {
        count = MAX_CROSSWORD_WORDS;
    }
    
    // Get total word count from words.c
    int total_words = get_total_words_count();
    
    // Array to track which words we've already selected
    int selected_indices[MAX_CROSSWORD_WORDS];
    int selected_count = 0;
    
    // Select unique random words
    while (selected_count < count) {
        int random_index = GetRandomValue(0, total_words - 1);
        
        // Check if we've already selected this word
        int already_selected = 0;
        for (int i = 0; i < selected_count; i++) {
            if (selected_indices[i] == random_index) {
                already_selected = 1;
                break;
            }
        }
        
        if (!already_selected) {
            selected_indices[selected_count] = random_index;
            const char* word = get_word_by_index(random_index);
            strcpy(selected_words[selected_count], word);
            selected_count++;
        }
    }
    
    return selected_count;
}

// Main function to select random words with unique starting letters (with safe fallback)
int select_random_words(char selected_words[][WORD_LENGTH + 1], int count) {
    if (count > MAX_CROSSWORD_WORDS) {
        count = MAX_CROSSWORD_WORDS;
    }
    
    // Try to select words with unique starting letters and intersection compatibility
    int selected_count = select_words_with_unique_starting_letters(selected_words, count);
    
    // If we didn't get enough compatible words, try fallback WITH intersection checking
    if (selected_count < count && selected_count > 0) {
        printf("Trying fallback selection for remaining %d words...\n", count - selected_count);
        
        int total_words = get_total_words_count();
        int attempts = 0;
        const int max_fallback_attempts = 1000;
        
        while (selected_count < count && attempts < max_fallback_attempts) {
            int random_index = GetRandomValue(0, total_words - 1);
            const char* candidate_word = get_word_by_index(random_index);
            attempts++;
            
            if (!candidate_word) continue;
            
            // Check if already selected
            int already_selected = 0;
            for (int i = 0; i < selected_count; i++) {
                if (strcmp(selected_words[i], candidate_word) == 0) {
                    already_selected = 1;
                    break;
                }
            }
            
            if (already_selected) continue;
            
            // Check if it can intersect with any previously selected word
            int can_connect = 0;
            for (int prev = 0; prev < selected_count; prev++) {
                if (can_words_intersect(candidate_word, selected_words[prev])) {
                    can_connect = 1;
                    break;
                }
            }
            
            // Only add if it can connect
            if (can_connect) {
                strcpy(selected_words[selected_count], candidate_word);
                selected_count++;
                printf("Fallback selected word %d: %s\n", selected_count, candidate_word);
            }
        }
        
        if (attempts >= max_fallback_attempts) {
            printf("Fallback exhausted after %d attempts, proceeding with %d words\n", max_fallback_attempts, selected_count);
        }
    }
    
    return selected_count;
}

// Helper function to find all possible intersections between two words
int find_intersections(const char* word1, const char* word2, 
                      IntersectionCandidate candidates[], int max_candidates) {
    int candidate_count = 0;
    
    // Check each letter in word1 against each letter in word2
    for (int i = 0; i < WORD_LENGTH && candidate_count < max_candidates; i++) {
        for (int j = 0; j < WORD_LENGTH && candidate_count < max_candidates; j++) {
            if (word1[i] == word2[j]) {
                // Found a matching letter - this is a potential intersection
                IntersectionCandidate candidate;
                candidate.word1_pos = i;
                candidate.word2_pos = j;
                candidates[candidate_count] = candidate;
                candidate_count++;
            }
        }
    }
    
    return candidate_count;
}

// Helper function to check if a word can be placed at a given position
int can_place_word(char grid[][9], int grid_width, int grid_height,
                   const char* word, int start_x, int start_y, int direction) {
    int dx = (direction == 0) ? 1 : 0;  // horizontal: move right
    int dy = (direction == 1) ? 1 : 0;  // vertical: move down
    
    // Check if word fits within grid boundaries
    int end_x = start_x + dx * (WORD_LENGTH - 1);
    int end_y = start_y + dy * (WORD_LENGTH - 1);
    
    if (start_x < 0 || start_y < 0 || end_x >= grid_width || end_y >= grid_height) {
        return 0;  // Word doesn't fit
    }
    
    // Check each letter position
    for (int i = 0; i < WORD_LENGTH; i++) {
        int x = start_x + dx * i;
        int y = start_y + dy * i;
        
        // Additional bounds check (defensive programming)
        if (x < 0 || x >= grid_width || y < 0 || y >= grid_height) {
            return 0;  // Out of bounds
        }
        
        // If cell is not empty, check if it matches the word letter
        if (grid[x][y] != '\0') {
            if (grid[x][y] != word[i]) {
                return 0;  // Conflict with existing letter
            }
        }
    }
    
    return 1;  // Word can be placed
}

// Helper function to place a word in the grid
void place_word_in_grid(char grid[][9], char word_mask[][9], int grid_width, int grid_height,
                       const char* word, int start_x, int start_y, int direction) {
    int dx = (direction == 0) ? 1 : 0;  // horizontal: move right
    int dy = (direction == 1) ? 1 : 0;  // vertical: move down
    
    // Place each letter
    for (int i = 0; i < WORD_LENGTH; i++) {
        int x = start_x + dx * i;
        int y = start_y + dy * i;
        
        // Bounds check before placing
        if (x >= 0 && x < grid_width && y >= 0 && y < grid_height) {
            grid[x][y] = word[i];
            word_mask[x][y] = 1;  // Mark as word cell
        }
    }
}

// Main crossword generation function
CrosswordLevel generate_crossword(int word_count, int grid_width, int grid_height) {
    CrosswordLevel level = {0};
    
    // Limit to reasonable bounds
    if (word_count > MAX_CROSSWORD_WORDS) word_count = MAX_CROSSWORD_WORDS;
    if (grid_width > 9) grid_width = 9;
    if (grid_height > 9) grid_height = 9;
    
    // Reduce word count for smaller grids to prevent overcrowding
    if (grid_width == 9 && grid_height == 9) {
        if (word_count > 6) {
            word_count = 6; // Limit to 6 words max for 9x9 grid
            printf("Reduced word count to %d for 9x9 grid\n", word_count);
        }
    }
    
    // Initialize grid to empty
    for (int x = 0; x < grid_width; x++) {
        for (int y = 0; y < grid_height; y++) {
            level.solution[x][y] = '\0';
            level.word_mask[x][y] = 0;
        }
    }
    
    // Select random words
    char selected_words[MAX_CROSSWORD_WORDS][WORD_LENGTH + 1];
    int actual_word_count = select_random_words(selected_words, word_count);
    
    // Debug output
    printf("Generating crossword: requested %d words, selected %d words\n", word_count, actual_word_count);
    for (int i = 0; i < actual_word_count; i++) {
        printf("  Word %d: %s\n", i + 1, selected_words[i]);
    }
    
    // Array to track placed words
    PlacedWord placed_words[MAX_CROSSWORD_WORDS];
    int placed_count = 0;
    
    // Place first word horizontally in the center
    if (actual_word_count > 0) {
        int start_x = (grid_width - WORD_LENGTH) / 2;
        int start_y = grid_height / 2;
        
        if (can_place_word(level.solution, grid_width, grid_height, 
                          selected_words[0], start_x, start_y, 0)) {
            place_word_in_grid(level.solution, level.word_mask, grid_width, grid_height,
                              selected_words[0], start_x, start_y, 0);
            
            // Record the placed word
            strcpy(placed_words[placed_count].word, selected_words[0]);
            placed_words[placed_count].start_x = start_x;
            placed_words[placed_count].start_y = start_y;
            placed_words[placed_count].direction = 0;
            placed_words[placed_count].placed = 1;
            placed_count++;
            printf("Placed first word '%s' horizontally at (%d,%d)\n", selected_words[0], start_x, start_y);
        } else {
            printf("Failed to place first word '%s' at (%d,%d)\n", selected_words[0], start_x, start_y);
        }
    }
    
    // Try to place remaining words
    for (int word_idx = 1; word_idx < actual_word_count; word_idx++) {
        int word_placed = 0;
        
        // Special case for 2-word crosswords - try multiple simple placements
        if (word_count == 2 && placed_count == 1) {
            // Try different intersection points for the second word
            for (int pos1 = 0; pos1 < WORD_LENGTH && !word_placed; pos1++) {
                for (int pos2 = 0; pos2 < WORD_LENGTH && !word_placed; pos2++) {
                    // Check if letters match at this potential intersection
                    if (placed_words[0].word[pos1] == selected_words[word_idx][pos2]) {
                        // Calculate placement for vertical word through horizontal word
                        int intersection_x = placed_words[0].start_x + pos1;
                        int intersection_y = placed_words[0].start_y;
                        
                        int new_start_x = intersection_x;
                        int new_start_y = intersection_y - pos2;
                        
                        if (new_start_y >= 0 && new_start_y + WORD_LENGTH <= grid_height &&
                            can_place_word(level.solution, grid_width, grid_height,
                                          selected_words[word_idx], new_start_x, new_start_y, 1)) {
                            
                            place_word_in_grid(level.solution, level.word_mask, grid_width, grid_height,
                                              selected_words[word_idx], new_start_x, new_start_y, 1);
                            
                            strcpy(placed_words[placed_count].word, selected_words[word_idx]);
                            placed_words[placed_count].start_x = new_start_x;
                            placed_words[placed_count].start_y = new_start_y;
                            placed_words[placed_count].direction = 1;
                            placed_words[placed_count].placed = 1;
                            placed_count++;
                            word_placed = 1;
                            printf("Placed second word '%s' vertically at (%d,%d) intersecting at letter '%c'\n", 
                                   selected_words[word_idx], new_start_x, new_start_y, placed_words[0].word[pos1]);
                        }
                    }
                }
            }
            
            if (!word_placed) {
                printf("Failed to find any valid intersection for '%s' with '%s'\n", selected_words[word_idx], placed_words[0].word);
            }
        }
        
        // If simple placement didn't work, try intersection-based placement
        if (!word_placed) {
            // Try to intersect with each already-placed word
            for (int placed_idx = 0; placed_idx < placed_count && !word_placed; placed_idx++) {
                IntersectionCandidate candidates[MAX_INTERSECTIONS];
                int candidate_count = find_intersections(placed_words[placed_idx].word, 
                                                       selected_words[word_idx], 
                                                       candidates, MAX_INTERSECTIONS);
                
                // Try each intersection candidate
            for (int i = 0; i < candidate_count && !word_placed; i++) {
                IntersectionCandidate* candidate = &candidates[i];
                
                // Calculate where new word would be placed
                int new_direction = 1 - placed_words[placed_idx].direction;  // Opposite direction
                int intersection_x = placed_words[placed_idx].start_x;
                int intersection_y = placed_words[placed_idx].start_y;
                
                // Move to intersection point
                if (placed_words[placed_idx].direction == 0) {  // horizontal
                    intersection_x += candidate->word1_pos;
                } else {  // vertical
                    intersection_y += candidate->word1_pos;
                }
                
                // Calculate start position for new word
                int new_start_x = intersection_x;
                int new_start_y = intersection_y;
                
                if (new_direction == 0) {  // new word horizontal
                    new_start_x -= candidate->word2_pos;
                } else {  // new word vertical
                    new_start_y -= candidate->word2_pos;
                }
                
                // Check if we can place the word here
                if (new_start_x >= 0 && new_start_y >= 0 &&
                    can_place_word(level.solution, grid_width, grid_height,
                                  selected_words[word_idx], new_start_x, new_start_y, new_direction)) {
                    
                    // Place the word
                    place_word_in_grid(level.solution, level.word_mask, grid_width, grid_height,
                                      selected_words[word_idx], new_start_x, new_start_y, new_direction);
                    
                    // Record the placed word
                    strcpy(placed_words[placed_count].word, selected_words[word_idx]);
                    placed_words[placed_count].start_x = new_start_x;
                    placed_words[placed_count].start_y = new_start_y;
                    placed_words[placed_count].direction = new_direction;
                    placed_words[placed_count].placed = 1;
                    placed_count++;
                    word_placed = 1;
                }
            }
            }
        }
    }
    
    // Set up the CrosswordLevel structure
    level.level = 1;
    level.word_count = placed_count;
    
    printf("Final result: placed %d out of %d requested words\n", placed_count, word_count);
    
    // Copy placed words to CrosswordWord format
    for (int i = 0; i < placed_count; i++) {
        level.words[i].start_x = placed_words[i].start_x;
        level.words[i].start_y = placed_words[i].start_y;
        level.words[i].direction = placed_words[i].direction;
        level.words[i].length = WORD_LENGTH;
    }
    
    return level;
}