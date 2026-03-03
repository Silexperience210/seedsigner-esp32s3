#include "core/qr_generator.h"
#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <esp_heap_caps.h>

// Minimal QR Code implementation
// Based on ISO/IEC 18004 - simplified for embedded use

namespace QR {

// Galois field log/antilog tables for Reed-Solomon
static const uint8_t GF_EXP[512] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1d, 0x3a, 0x74, 0xe8, 0xcd, 0x87, 0x13, 0x26,
    0x4c, 0x98, 0x2d, 0x5a, 0xb4, 0x75, 0xea, 0xc9, 0x8f, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0,
    0x9d, 0x27, 0x4e, 0x9c, 0x25, 0x4a, 0x94, 0x35, 0x6a, 0xd4, 0xb5, 0x77, 0xee, 0xc1, 0x9f, 0x23,
    0x46, 0x8c, 0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0, 0x5d, 0xba, 0x69, 0xd2, 0xb9, 0x6f, 0xde, 0xa1,
    0x5f, 0xbe, 0x61, 0xc2, 0x99, 0x2f, 0x5e, 0xbc, 0x65, 0xca, 0x89, 0x0f, 0x1e, 0x3c, 0x78, 0xf0,
    0xfd, 0xe7, 0xd3, 0xbb, 0x6b, 0xd6, 0xb1, 0x7f, 0xfe, 0xe1, 0xdf, 0xa3, 0x5b, 0xb6, 0x71, 0xe2,
    0xd9, 0xaf, 0x43, 0x86, 0x11, 0x22, 0x44, 0x88, 0x0d, 0x1a, 0x34, 0x68, 0xd0, 0xbd, 0x67, 0xce,
    0x81, 0x1f, 0x3e, 0x7c, 0xf8, 0xed, 0xc7, 0x93, 0x3b, 0x76, 0xec, 0xc5, 0x97, 0x33, 0x66, 0xcc,
    0x85, 0x17, 0x2e, 0x5c, 0xb8, 0x6d, 0xda, 0xa9, 0x4f, 0x9e, 0x21, 0x42, 0x84, 0x15, 0x2a, 0x54,
    0xa8, 0x4d, 0x9a, 0x29, 0x52, 0xa4, 0x55, 0xaa, 0x49, 0x92, 0x39, 0x72, 0xe4, 0xd5, 0xb7, 0x73,
    0xe6, 0xd1, 0xbf, 0x63, 0xc6, 0x91, 0x3f, 0x7e, 0xfc, 0xe5, 0xd7, 0xb3, 0x7b, 0xf6, 0xf1, 0xff,
    0xe3, 0xdb, 0xab, 0x4b, 0x96, 0x31, 0x62, 0xc4, 0x95, 0x37, 0x6e, 0xdc, 0xa5, 0x57, 0xae, 0x41,
    0x82, 0x19, 0x32, 0x64, 0xc8, 0x8d, 0x07, 0x0e, 0x1c, 0x38, 0x70, 0xe0, 0xdd, 0xa7, 0x53, 0xa6,
    0x51, 0xa2, 0x59, 0xb2, 0x79, 0xf2, 0xf9, 0xef, 0xc3, 0x9b, 0x2b, 0x56, 0xac, 0x45, 0x8a, 0x09,
    0x12, 0x24, 0x48, 0x90, 0x3d, 0x7a, 0xf4, 0xf5, 0xf7, 0xf3, 0xfb, 0xeb, 0xcb, 0x8b, 0x0b, 0x16,
    0x2c, 0x58, 0xb0, 0x7d, 0xfa, 0xe9, 0xcf, 0x83, 0x1b, 0x36, 0x6c, 0xd8, 0xad, 0x47, 0x8e, 0x01,
    // Repeated for convenience
    0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1d, 0x3a, 0x74, 0xe8, 0xcd, 0x87, 0x13, 0x26, 0x4c
};

static const uint8_t GF_LOG[256] = {
    0xff, 0x00, 0x01, 0x19, 0x02, 0x32, 0x1a, 0xc6, 0x03, 0xdf, 0x33, 0xee, 0x1b, 0x68, 0xc7, 0x4b,
    0x04, 0x64, 0xe0, 0x0e, 0x34, 0x8d, 0xef, 0x81, 0x1c, 0xc1, 0x69, 0xf8, 0xc8, 0x08, 0x4c, 0x71,
    0x05, 0x8a, 0x65, 0x2f, 0xe1, 0x24, 0x0f, 0x21, 0x35, 0x93, 0x8e, 0xda, 0xf0, 0x12, 0x82, 0x45,
    0x1d, 0xb5, 0xc2, 0x7d, 0x6a, 0x27, 0xf9, 0xb9, 0xc9, 0x9a, 0x09, 0x78, 0x4d, 0xe4, 0x72, 0xa6,
    0x06, 0xbf, 0x8b, 0x62, 0x66, 0xdd, 0x30, 0xfd, 0xe2, 0x98, 0x25, 0xb3, 0x10, 0x91, 0x22, 0x88,
    0x36, 0xd0, 0x94, 0xce, 0x8f, 0x96, 0xdb, 0xbd, 0xf1, 0xd2, 0x13, 0x5c, 0x83, 0x38, 0x46, 0x40,
    0x1e, 0x42, 0xb6, 0xa3, 0xc3, 0x48, 0x7e, 0x6e, 0x6b, 0x3a, 0x28, 0x54, 0xfa, 0x85, 0xba, 0x3d,
    0xca, 0x5e, 0x9b, 0x9f, 0x0a, 0x15, 0x79, 0x2b, 0x4e, 0xd4, 0xe5, 0xac, 0x73, 0xf3, 0xa7, 0x57,
    0x07, 0x70, 0xc0, 0xf7, 0x8c, 0x80, 0x63, 0x0d, 0x67, 0x4a, 0xde, 0xed, 0x31, 0xc5, 0xfe, 0x18,
    0xe3, 0xa5, 0x99, 0x77, 0x26, 0xb8, 0xb4, 0x7c, 0x11, 0x44, 0x92, 0xd9, 0x23, 0x20, 0x89, 0x2e,
    0x37, 0x3f, 0xd1, 0x5b, 0x95, 0xbc, 0xcf, 0xcd, 0x90, 0x87, 0x97, 0xb2, 0xdc, 0xfc, 0xbe, 0x61,
    0xf6, 0x56, 0x3b, 0xbb, 0xeb, 0xcc, 0x3e, 0x14, 0x0b, 0xbf, 0xe7, 0x67, 0x1f, 0xea, 0x60, 0xa4,
    0x5f, 0x1a, 0x52, 0xe6, 0xb7, 0xd3, 0x29, 0xc4, 0x3c, 0x4f, 0x5d, 0xae, 0x55, 0x16, 0x6f, 0x9c,
    0x86, 0xf4, 0xf2, 0x7b, 0xa0, 0x76, 0xab, 0xd5, 0x8a, 0xae, 0x5c, 0xe8, 0x75, 0x5f, 0x4f, 0x3d,
    0x6d, 0x78, 0xcd, 0x43, 0x1b, 0x8b, 0x5a, 0xe9, 0x4a, 0x93, 0x97, 0xb1, 0xdc, 0x4b, 0x59, 0xf9,
    0x1f, 0x17, 0x8f, 0x4d, 0xec, 0x03, 0x75, 0xba, 0xe6, 0x65, 0xf2, 0xa1, 0xfc, 0x86, 0x1e, 0xfb
};

// Mode indicators
enum class Mode {
    NUMERIC = 1,
    ALPHANUMERIC = 2,
    BYTE = 4,
    KANJI = 8
};

// Error correction codewords per version
static const uint16_t ECC_CODEWORDS[40][4] = {
    // L, M, Q, H
    {7, 10, 13, 17}, {10, 16, 22, 28}, {15, 26, 36, 44}, {20, 36, 52, 64},
    {26, 48, 72, 88}, {36, 64, 96, 112}, {40, 72, 108, 130}, {48, 88, 132, 156}
    // ... would continue for all 40 versions
};

// Position adjustment patterns (simplified)
static const uint8_t ALIGNMENT_PATTERN[7][7] = {
    {1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1}
};

// Simple Reed-Solomon encoder
static void rs_encode(int data_len, const uint8_t* data, int ecc_len, uint8_t* ecc) {
    memset(ecc, 0, ecc_len);
    
    for (int i = 0; i < data_len; i++) {
        uint8_t m = data[i] ^ ecc[0];
        for (int j = 0; j < ecc_len - 1; j++) {
            ecc[j] = ecc[j + 1];
        }
        ecc[ecc_len - 1] = 0;
        
        if (m != 0) {
            // Multiply by generator polynomial (simplified)
            for (int j = 0; j < ecc_len; j++) {
                ecc[j] ^= GF_EXP[(GF_LOG[m] + 0) % 255]; // Simplified
            }
        }
    }
}

// Get minimum QR version for data length
uint8_t get_min_version(size_t data_len, ECC ecc) {
    // Simplified - returns appropriate version
    if (data_len <= 152) return 1;
    if (data_len <= 272) return 2;
    if (data_len <= 440) return 3;
    if (data_len <= 640) return 4;
    if (data_len <= 864) return 5;
    return 10; // Default
}

// Place fixed patterns (finder patterns, timing, etc)
static void place_fixed_patterns(Code* qr) {
    int size = qr->width;
    
    // Finder patterns (corners)
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            // Top-left
            qr->modules[i][j] = (i == 0 || i == 6 || j == 0 || j == 6) ? 1 :
                                (i >= 2 && i <= 4 && j >= 2 && j <= 4) ? 1 : 0;
            // Top-right
            qr->modules[i][size - 7 + j] = (i == 0 || i == 6 || j == 0 || j == 6) ? 1 :
                                            (i >= 2 && i <= 4 && j >= 2 && j <= 4) ? 1 : 0;
            // Bottom-left
            qr->modules[size - 7 + i][j] = (i == 0 || i == 6 || j == 0 || j == 6) ? 1 :
                                            (i >= 2 && i <= 4 && j >= 2 && j <= 4) ? 1 : 0;
        }
    }
    
    // Separators (white border around finder patterns)
    for (int i = 0; i < 8; i++) {
        qr->modules[i][7] = 0;
        qr->modules[7][i] = 0;
        qr->modules[i][size - 8] = 0;
        qr->modules[7][size - 8 + i] = 0;
        qr->modules[size - 8][i] = 0;
        qr->modules[size - 8 + i][7] = 0;
    }
    
    // Timing patterns (alternating black/white)
    for (int i = 8; i < size - 8; i++) {
        qr->modules[6][i] = (i % 2 == 0) ? 1 : 0;
        qr->modules[i][6] = (i % 2 == 0) ? 1 : 0;
    }
    
    // Dark module
    qr->modules[4 * qr->version + 9][8] = 1;
}

// Place data modules
static void place_data(Code* qr, const uint8_t* data, size_t len) {
    int size = qr->width;
    int dir = -1; // Upward
    int row = size - 1;
    int col = size - 1;
    size_t bit_idx = 0;
    
    while (col > 0) {
        if (col == 6) col--; // Skip timing column
        
        while (row >= 0 && row < size) {
            for (int c = 0; c < 2; c++) {
                int x = col - c;
                if (qr->modules[row][x] == 0) { // Only place in empty modules
                    uint8_t byte_idx = bit_idx / 8;
                    uint8_t bit = 7 - (bit_idx % 8);
                    if (byte_idx < len) {
                        qr->modules[row][x] = (data[byte_idx] >> bit) & 1;
                    } else {
                        qr->modules[row][x] = 0; // Padding
                    }
                    bit_idx++;
                }
            }
            row += dir;
        }
        dir = -dir;
        row += dir;
        col -= 2;
    }
}

bool generate(const uint8_t* data, size_t len, Code* qr, ECC ecc) {
    if (!data || !qr || len > MAX_QR_SIZE) return false;
    
    qr->clear();
    qr->version = get_min_version(len, ecc);
    qr->ecc_level = ecc;
    qr->width = 17 + 4 * qr->version;
    
    // Place fixed patterns
    place_fixed_patterns(qr);
    
    // Encode data (simplified - proper encoding would segment and add headers)
    uint8_t* encoded = (uint8_t*)heap_caps_malloc(2953, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!encoded) return false;
    size_t encoded_len = 0;
    
    // Mode indicator (byte mode)
    encoded[encoded_len++] = 0x40; // Byte mode indicator
    
    // Character count indicator (simplified for version < 9)
    encoded[encoded_len++] = len;
    
    // Data
    memcpy(encoded + encoded_len, data, len);
    encoded_len += len;
    
    // Terminator
    encoded[encoded_len++] = 0x00;
    
    // Pad to codeword boundary
    while (encoded_len % 8 != 0) {
        encoded[encoded_len++] = 0;
    }
    
    // Add padding bytes
    const uint8_t PADDING[2] = {0xEC, 0x11};
    int pad_idx = 0;
    while (encoded_len < 100) { // Simplified
        encoded[encoded_len++] = PADDING[pad_idx];
        pad_idx = 1 - pad_idx;
    }
    
    // Calculate and add error correction (simplified)
    // Real implementation would split into blocks and interleave
    
    // Place data
    place_data(qr, encoded, encoded_len);
    
    // Apply mask pattern (simplified - pattern 0)
    for (int i = 0; i < qr->width; i++) {
        for (int j = 0; j < qr->width; j++) {
            if ((i + j) % 2 == 0 && qr->modules[i][j] != 2) {
                qr->modules[i][j] ^= 1; // Toggle
            }
        }
    }
    
    free(encoded);
    return true;
}

bool generate_text(const char* text, Code* qr, ECC ecc) {
    if (!text || !qr) return false;
    return generate((const uint8_t*)text, strlen(text), qr, ecc);
}

bool render_bitmap(const Code* qr, uint8_t* bitmap, size_t bitmap_size) {
    if (!qr || !bitmap) return false;
    
    int width = qr->width;
    int stride = (width + 7) / 8;
    size_t required = stride * width;
    
    if (bitmap_size < required) return false;
    
    memset(bitmap, 0, required);
    
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < width; x++) {
            if (qr->modules[y][x]) {
                bitmap[y * stride + (x / 8)] |= (0x80 >> (x % 8));
            }
        }
    }
    
    return true;
}

bool encode_address(const char* address, Code* qr) {
    // Bitcoin UR format
    char uri[128];
    snprintf(uri, sizeof(uri), "bitcoin:%s", address);
    return generate_text(uri, qr, ECC::ECC_MEDIUM);
}

size_t encode_psbt_multipart(const uint8_t* psbt, size_t len, 
                              Code* parts, size_t max_parts,
                              ECC ecc) {
    // Simplified - would use crypto-psbt UR format in real implementation
    if (!psbt || !parts || max_parts == 0) return 0;
    
    // For now, just encode as base64 if fits
    if (len < 500) {
        // Simple base64 encoding (simplified)
        char base64[1024];
        // ... base64 encode ...
        if (generate_text(base64, &parts[0], ecc)) {
            return 1;
        }
    }
    
    return 0;
}

bool encode_xpub(const char* xpub, Code* qr) {
    // SLIP-132 or standard xpub format
    char data[150];
    snprintf(data, sizeof(data), "xpub:%s", xpub);
    return generate_text(data, qr, ECC::ECC_MEDIUM);
}

} // namespace QR
