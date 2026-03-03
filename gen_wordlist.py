with open('bip39_words.txt', 'r') as f:
    words = f.read().strip().split('\n')

# Generate compact wordlist
lines = ['#pragma once', '#include <Arduino.h>', '', '// BIP39 English Wordlist (2048 words) - Memory optimized', '// Words stored in flash only, no RAM usage', 'namespace BIP39 {', 'constexpr uint16_t WORDLIST_SIZE = 2048;']

# Create concatenated string with null separators
word_data = []
for w in words:
    word_data.append(w)

# Write as raw bytes to avoid string issues
lines.append('')
lines.append('// Concatenated word data')
lines.append('static const uint8_t WORDLIST_BYTES[] PROGMEM = {')

# Convert to hex bytes
data_str = ''
for w in word_data:
    for c in w:
        data_str += f'0x{ord(c):02x}, '
    data_str += '0x00, '  # null terminator

# Wrap at 16 bytes per line
byte_list = data_str.strip(', ').split(', ')
for i in range(0, len(byte_list), 16):
    chunk = byte_list[i:i+16]
    lines.append('    ' + ', '.join(chunk) + ',')

lines.append('};')
lines.append('')
lines.append('// Inline helper - copies word to buffer')
lines.append('inline void get_word_copy(uint16_t index, char* buffer, size_t buf_len) {')
lines.append('    if (index >= WORDLIST_SIZE || buffer == nullptr || buf_len == 0) return;')
lines.append('    // Simple linear search for now (can be optimized with offset table)')
lines.append('    uint16_t count = 0;')
lines.append('    uint16_t pos = 0;')
lines.append('    while (count < index) {')
lines.append('        if (pgm_read_byte(&WORDLIST_BYTES[pos]) == 0) count++;')
lines.append('        pos++;')
lines.append('    }')
lines.append('    // Copy word')
lines.append('    uint8_t i = 0;')
lines.append('    uint8_t c;')
lines.append('    while ((c = pgm_read_byte(&WORDLIST_BYTES[pos + i])) != 0 && i < buf_len - 1) {')
lines.append('        buffer[i] = c;')
lines.append('        i++;')
lines.append('    }')
lines.append("    buffer[i] = '\\0';")
lines.append('}')
lines.append('')
lines.append('} // namespace BIP39')

with open('src/core/bip39_wordlist_full.h', 'w') as f:
    f.write('\n'.join(lines))

print(f'Generated compact wordlist with {len(words)} words, {len(byte_list)} bytes')
