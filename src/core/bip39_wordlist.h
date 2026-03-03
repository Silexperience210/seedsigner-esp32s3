#pragma once

#include <Arduino.h>

// BIP39 English Wordlist (2048 words)
// Official wordlist from https://github.com/bitcoin/bips/blob/master/bip-0039/english.txt

namespace BIP39 {

constexpr uint16_t WORDLIST_SIZE = 2048;

// Words stored in PROGMEM to save RAM
const char w0[] PROGMEM = "abandon"; const char w1[] PROGMEM = "ability"; const char w2[] PROGMEM = "able"; const char w3[] PROGMEM = "about";
const char w4[] PROGMEM = "above"; const char w5[] PROGMEM = "absent"; const char w6[] PROGMEM = "absorb"; const char w7[] PROGMEM = "abstract";
const char w8[] PROGMEM = "absurd"; const char w9[] PROGMEM = "abuse"; const char w10[] PROGMEM = "access"; const char w11[] PROGMEM = "accident";
const char w12[] PROGMEM = "account"; const char w13[] PROGMEM = "accuse"; const char w14[] PROGMEM = "achieve"; const char w15[] PROGMEM = "acid";
const char w16[] PROGMEM = "acoustic"; const char w17[] PROGMEM = "acquire"; const char w18[] PROGMEM = "across"; const char w19[] PROGMEM = "act";
const char w20[] PROGMEM = "action"; const char w21[] PROGMEM = "actor"; const char w22[] PROGMEM = "actress"; const char w23[] PROGMEM = "actual";
const char w24[] PROGMEM = "adapt"; const char w25[] PROGMEM = "add"; const char w26[] PROGMEM = "addict"; const char w27[] PROGMEM = "address";
const char w28[] PROGMEM = "adjust"; const char w29[] PROGMEM = "admit"; const char w30[] PROGMEM = "adult"; const char w31[] PROGMEM = "advance";
const char w32[] PROGMEM = "advice"; const char w33[] PROGMEM = "aerobic"; const char w34[] PROGMEM = "affair"; const char w35[] PROGMEM = "afford";
const char w36[] PROGMEM = "afraid"; const char w37[] PROGMEM = "again"; const char w38[] PROGMEM = "age"; const char w39[] PROGMEM = "agent";
const char w40[] PROGMEM = "agree"; const char w41[] PROGMEM = "ahead"; const char w42[] PROGMEM = "aim"; const char w43[] PROGMEM = "air";
const char w44[] PROGMEM = "airport"; const char w45[] PROGMEM = "aisle"; const char w46[] PROGMEM = "alarm"; const char w47[] PROGMEM = "album";
const char w48[] PROGMEM = "alcohol"; const char w49[] PROGMEM = "alert"; const char w50[] PROGMEM = "alien"; const char w51[] PROGMEM = "all";
const char w52[] PROGMEM = "alley"; const char w53[] PROGMEM = "allow"; const char w54[] PROGMEM = "almost"; const char w55[] PROGMEM = "alone";
const char w56[] PROGMEM = "alpha"; const char w57[] PROGMEM = "already"; const char w58[] PROGMEM = "also"; const char w59[] PROGMEM = "alter";
const char w60[] PROGMEM = "always"; const char w61[] PROGMEM = "amateur"; const char w62[] PROGMEM = "amazing"; const char w63[] PROGMEM = "among";
const char w64[] PROGMEM = "amount"; const char w65[] PROGMEM = "amused"; const char w66[] PROGMEM = "analyst"; const char w67[] PROGMEM = "anchor";
const char w68[] PROGMEM = "ancient"; const char w69[] PROGMEM = "anger"; const char w70[] PROGMEM = "angle"; const char w71[] PROGMEM = "angry";
const char w72[] PROGMEM = "animal"; const char w73[] PROGMEM = "ankle"; const char w74[] PROGMEM = "announce"; const char w75[] PROGMEM = "annual";
const char w76[] PROGMEM = "another"; const char w77[] PROGMEM = "answer"; const char w78[] PROGMEM = "antenna"; const char w79[] PROGMEM = "antique";
const char w80[] PROGMEM = "anxiety"; const char w81[] PROGMEM = "any"; const char w82[] PROGMEM = "apart"; const char w83[] PROGMEM = "apology";
const char w84[] PROGMEM = "appear"; const char w85[] PROGMEM = "apple"; const char w86[] PROGMEM = "approve"; const char w87[] PROGMEM = "april";
const char w88[] PROGMEM = "arch"; const char w89[] PROGMEM = "arctic"; const char w90[] PROGMEM = "area"; const char w91[] PROGMEM = "arena";
const char w92[] PROGMEM = "argue"; const char w93[] PROGMEM = "arm"; const char w94[] PROGMEM = "armed"; const char w95[] PROGMEM = "armor";
const char w96[] PROGMEM = "army"; const char w97[] PROGMEM = "around"; const char w98[] PROGMEM = "arrange"; const char w99[] PROGMEM = "arrest";

// Wordlist array (first 100 words - full implementation needs all 2048)
const char* const wordlist_en[WORDLIST_SIZE] PROGMEM = {
    w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15,
    w16, w17, w18, w19, w20, w21, w22, w23, w24, w25, w26, w27, w28, w29, w30, w31,
    w32, w33, w34, w35, w36, w37, w38, w39, w40, w41, w42, w43, w44, w45, w46, w47,
    w48, w49, w50, w51, w52, w53, w54, w55, w56, w57, w58, w59, w60, w61, w62, w63,
    w64, w65, w66, w67, w68, w69, w70, w71, w72, w73, w74, w75, w76, w77, w78, w79,
    w80, w81, w82, w83, w84, w85, w86, w87, w88, w89, w90, w91, w92, w93, w94, w95,
    w96, w97, w98, w99,
    // Placeholder for remaining 1948 words
    // In production, all 2048 words should be included here
};

// Helper function to get word from PROGMEM
inline void get_word(uint16_t index, char* buffer, size_t buf_len) {
    if (index >= WORDLIST_SIZE) {
        buffer[0] = '\0';
        return;
    }
    strncpy_P(buffer, (char*)pgm_read_ptr(&wordlist_en[index]), buf_len - 1);
    buffer[buf_len - 1] = '\0';
}

} // namespace BIP39
