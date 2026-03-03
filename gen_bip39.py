with open('bip39_words.txt', 'r') as f:
    words = f.read().strip().split('\n')

# Generate C++ file
lines = ['#pragma once', '#include <Arduino.h>', '', '// BIP39 English Wordlist (2048 words) - Complete', 'namespace BIP39 {', 'constexpr uint16_t WORDLIST_SIZE = 2048;', '', 'const char* const wordlist_en[WORDLIST_SIZE] PROGMEM = {']

# Add words in groups of 8 for readability
for i in range(0, len(words), 8):
    chunk = words[i:i+8]
    lines.append('    ' + ', '.join([f'"{w}"' for w in chunk]) + ',')

lines.extend([
'};',
'',
'// Helper to get word from index',
'inline const char* get_word(uint16_t index) {',
'    if (index >= WORDLIST_SIZE) return nullptr;',
'    return (const char*)pgm_read_ptr(&wordlist_en[index]);',
'}',
'',
'// Copy word to buffer',
'inline void get_word_copy(uint16_t index, char* buffer, size_t buf_len) {',
'    const char* word = get_word(index);',
'    if (word) {',
'        strncpy_P(buffer, word, buf_len - 1);',
'        buffer[buf_len - 1] = \'\\0\';',
'    } else {',
'        buffer[0] = \'\\0\';',
'    }',
'}',
'',
'} // namespace BIP39'
])

with open('src/core/bip39_wordlist_full.h', 'w') as f:
    f.write('\n'.join(lines))

print(f'Generated file with {len(words)} words')
