#include "systems.h"

static const char* WORDLE_WORDS[] = {
    "ABOUT", "ABOVE", "ABUSE", "ACTOR", "ACUTE", "ADMIT", "ADOPT", "ADULT", "AFTER", "AGAIN",
    "AGENT", "AGREE", "AHEAD", "ALARM", "ALBUM", "ALERT", "ALIEN", "ALIGN", "ALIKE", "ALIVE",
    "ALLOW", "ALONE", "ALONG", "ALTER", "AMBER", "AMEND", "AMONG", "ANGER", "ANGLE", "ANGRY",
    "ANKLE", "APART", "APPLE", "APPLY", "ARENA", "ARGUE", "ARISE", "ARMED", "ARMOR", "ARRAY",
    "ASIDE", "ASSET", "ATLAS", "AUDIO", "AUDIT", "AVOID", "AWAKE", "AWARD", "AWARE", "BADLY",
    "BADGE", "BASIN", "BASIC", "BATCH", "BEACH", "BEGAN", "BEGIN", "BEING", "BELOW", "BENCH",
    "BILLY", "BIRTH", "BLACK", "BLAME", "BLANK", "BLAST", "BLIND", "BLOCK", "BLOOD", "BLOOM",
    "BLOWN", "BLUES", "BLUNT", "BLUSH", "BOARD", "BOAST", "BOBBY", "BOOST", "BOOTH", "BOUND",
    "BRAIN", "BRAKE", "BRAND", "BRASS", "BRAVE", "BREAD", "BREAK", "BREED", "BRICK", "BRIDE",
    "BRIEF", "BRING", "BROAD", "BROKE", "BROWN", "BRUSH", "BUILD", "BUILT", "BUNCH", "BURNS",
    "BURST", "CABIN", "CABLE", "CACHE", "CAMEL", "CANAL", "CANDY", "CANON", "CARRY", "CARVE",
    "CATCH", "CAUSE", "CHAIN", "CHAIR", "CHAMP", "CHAOS", "CHARM", "CHART", "CHASE", "CHEAP",
    "CHECK", "CHESS", "CHEST", "CHIEF", "CHILD", "CHINA", "CHOIR", "CHORD", "CHOSE", "CIVIL",
    "CLAIM", "CLASS", "CLEAN", "CLEAR", "CLICK", "CLIFF", "CLIMB", "CLOCK", "CLOSE", "CLOTH",
    "CLOUD", "CLOWN", "CLUBS", "COACH", "COAST", "CODED", "CODES", "COINS", "COLOR", "COMET",
    "CORAL", "COSTS", "COUCH", "COUGH", "COULD", "COUNT", "COURT", "COVER", "CRACK", "CRAFT",
    "CRASH", "CRAZY", "CREAM", "CRIME", "CRISP", "CROSS", "CROWD", "CROWN", "CRUDE", "CRUSH",
    "CURVE", "CYCLE", "DAILY", "DAIRY", "DANCE", "DATED", "DEALS", "DEATH", "DEBUT", "DELAY",
    "DENSE", "DEPTH", "DOING", "DOLOR", "DOORS", "DOUBT", "DOZEN", "DRAFT", "DRAMA", "DRANK",
    "DRAWN", "DREAM", "DRESS", "DRILL", "DRINK", "DRIVE", "DRONE", "DROVE", "DRUNK", "DRYING"
};

#define TOTAL_WORDS (sizeof(WORDLE_WORDS) / sizeof(WORDLE_WORDS[0]))

const char* get_random_word(void) {
    int random_index = GetRandomValue(0, TOTAL_WORDS - 1);
    return WORDLE_WORDS[random_index];
}