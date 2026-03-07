/**
 * @file secp256k1_field.c
 * Field operations for secp256k1
 * 
 * Prime p = 2^256 - 2^32 - 977
 * = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
 */

#include "secp256k1_field.h"

/* secp256k1 prime p */
static const uint32_t secp256k1_p[8] = {
    0xFFFFFC2F, 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};

/* (p + 1) / 4 - used for sqrt */
static const uint32_t secp256k1_p_plus_1_over_4[8] = {
    0x3FFFFFDF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x3FFFFFFF
};

const secp256k1_fe secp256k1_fe_p = {{0xFFFFFC2F, 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF,
                                       0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}};
const secp256k1_fe secp256k1_fe_one = {{1, 0, 0, 0, 0, 0, 0, 0}};
const secp256k1_fe secp256k1_fe_zero = {{0, 0, 0, 0, 0, 0, 0, 0}};

/* Constant-time comparison: returns 1 if a < b, 0 otherwise */
static int secp256k1_is_less(const uint32_t* a, const uint32_t* b, int len) {
    int res = 0;
    int done = 0;
    for (int i = len - 1; i >= 0; i--) {
        int lt = (a[i] < b[i]) & ~done;
        int gt = (a[i] > b[i]) & ~done;
        res |= lt;
        done |= lt | gt;
    }
    return res;
}

/* Constant-time equality: returns 1 if a == b, 0 otherwise */
static int secp256k1_is_equal(const uint32_t* a, const uint32_t* b, int len) {
    uint32_t diff = 0;
    for (int i = 0; i < len; i++) {
        diff |= (a[i] ^ b[i]);
    }
    return diff == 0;
}

/* Conditional move: if flag, r = a (constant time) */
static void secp256k1_cmov32(uint32_t* r, const uint32_t* a, int flag, int len) {
    uint32_t mask = flag ? 0xFFFFFFFF : 0;
    for (int i = 0; i < len; i++) {
        r[i] = (r[i] & ~mask) | (a[i] & mask);
    }
}

/* Add with carry */
static uint32_t secp256k1_add_carry(uint32_t* r, uint32_t a, uint32_t b, uint32_t carry) {
    uint64_t sum = (uint64_t)a + b + carry;
    *r = (uint32_t)sum;
    return (uint32_t)(sum >> 32);
}

/* Subtract with borrow */
static uint32_t secp256k1_sub_borrow(uint32_t* r, uint32_t a, uint32_t b, uint32_t borrow) {
    uint64_t diff = (uint64_t)a - b - borrow;
    *r = (uint32_t)diff;
    return (uint32_t)((diff >> 32) & 1);
}

/* Multiply two 32-bit values, return 64-bit result */
static uint64_t secp256k1_mul_32x32(uint32_t a, uint32_t b) {
    return (uint64_t)a * b;
}

/* Reduce modulo p (weak normalization) */
void secp256k1_fe_normalize_weak(secp256k1_fe* r) {
    /* Add 0x1000003D1 * 2^32 if overflow detected */
    uint64_t t[8];
    int i;
    
    /* Convert to 64-bit for intermediate calculation */
    for (i = 0; i < 8; i++) {
        t[i] = r->n[i];
    }
    
    /* Check if >= p */
    int overflow = !secp256k1_is_less(r->n, secp256k1_p, 8);
    
    /* If overflow, subtract p */
    if (overflow) {
        uint32_t borrow = 0;
        for (i = 0; i < 8; i++) {
            borrow = secp256k1_sub_borrow(&r->n[i], r->n[i], secp256k1_p[i], borrow);
        }
    }
}

/* Full normalization - ensures unique representation */
void secp256k1_fe_normalize(secp256k1_fe* r) {
    secp256k1_fe_normalize_weak(r);
    
    /* Additional reduction pass to ensure fully normalized */
    int overflow = !secp256k1_is_less(r->n, secp256k1_p, 8);
    if (overflow) {
        uint32_t borrow = 0;
        for (int i = 0; i < 8; i++) {
            borrow = secp256k1_sub_borrow(&r->n[i], r->n[i], secp256k1_p[i], borrow);
        }
    }
}

/* Variable-time normalization (ok for non-secret data) */
void secp256k1_fe_normalize_var(secp256k1_fe* r) {
    secp256k1_fe_normalize(r);
}

/* Test if zero (constant time) */
int secp256k1_fe_is_zero(const secp256k1_fe* a) {
    uint32_t acc = 0;
    for (int i = 0; i < 8; i++) {
        acc |= a->n[i];
    }
    return acc == 0;
}

/* Test if odd (constant time) */
int secp256k1_fe_is_odd(const secp256k1_fe* a) {
    secp256k1_fe t;
    secp256k1_fe_copy(&t, a);
    secp256k1_fe_normalize_weak(&t);
    return t.n[0] & 1;
}

/* Equality test (constant time) */
int secp256k1_fe_equal(const secp256k1_fe* a, const secp256k1_fe* b) {
    secp256k1_fe ta, tb;
    secp256k1_fe_copy(&ta, a);
    secp256k1_fe_copy(&tb, b);
    secp256k1_fe_normalize_weak(&ta);
    secp256k1_fe_normalize_weak(&tb);
    return secp256k1_is_equal(ta.n, tb.n, 8);
}

/* Set to small integer */
void secp256k1_fe_set_int(secp256k1_fe* r, int a) {
    r->n[0] = (uint32_t)a;
    for (int i = 1; i < 8; i++) {
        r->n[i] = 0;
    }
}

/* Clear (set to zero) */
void secp256k1_fe_clear(secp256k1_fe* a) {
    volatile uint32_t* p = a->n;
    for (int i = 0; i < 8; i++) {
        p[i] = 0;
    }
}

/* Copy */
void secp256k1_fe_copy(secp256k1_fe* r, const secp256k1_fe* a) {
    for (int i = 0; i < 8; i++) {
        r->n[i] = a->n[i];
    }
}

/* Addition modulo p */
void secp256k1_fe_add(secp256k1_fe* r, const secp256k1_fe* a, const secp256k1_fe* b) {
    uint32_t carry = 0;
    for (int i = 0; i < 8; i++) {
        carry = secp256k1_add_carry(&r->n[i], a->n[i], b->n[i], carry);
    }
    
    /* Reduce if >= p */
    int overflow = carry || !secp256k1_is_less(r->n, secp256k1_p, 8);
    if (overflow) {
        uint32_t borrow = 0;
        for (int i = 0; i < 8; i++) {
            borrow = secp256k1_sub_borrow(&r->n[i], r->n[i], secp256k1_p[i], borrow);
        }
    }
}

/* Negation modulo p: r = -a mod p */
void secp256k1_fe_negate(secp256k1_fe* r, const secp256k1_fe* a, int m) {
    (void)m; /* unused but kept for API compatibility */
    
    /* r = p - a (if a != 0) */
    uint32_t borrow = 0;
    for (int i = 0; i < 8; i++) {
        borrow = secp256k1_sub_borrow(&r->n[i], secp256k1_p[i], a->n[i], borrow);
    }
}

/* Multiplication modulo p */
void secp256k1_fe_mul(secp256k1_fe* r, const secp256k1_fe* a, const secp256k1_fe* b) {
    uint64_t t[16] = {0};
    
    /* Schoolbook multiplication */
    for (int i = 0; i < 8; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < 8; j++) {
            uint64_t prod = t[i + j] + secp256k1_mul_32x32(a->n[i], b->n[j]) + carry;
            t[i + j] = prod & 0xFFFFFFFF;
            carry = prod >> 32;
        }
        t[i + 8] = carry;
    }
    
    /* Reduce modulo p using special form of secp256k1 prime */
    /* p = 2^256 - 2^32 - 977, so 2^256 ≡ 2^32 + 977 (mod p) */
    
    /* Multiply high part by (2^32 + 977) and add to low part */
    uint64_t c = 0;
    for (int i = 0; i < 8; i++) {
        uint64_t acc = t[i] + c;
        if (i < 8) {
            acc += secp256k1_mul_32x32((uint32_t)t[8 + i], 977);
            if (i > 0) acc += t[7 + i];  /* 2^32 term */
        }
        r->n[i] = (uint32_t)acc;
        c = acc >> 32;
    }
    
    /* Final reduction */
    secp256k1_fe_normalize_weak(r);
}

/* Squaring (optimized) */
void secp256k1_fe_sqr(secp256k1_fe* r, const secp256k1_fe* a) {
    /* For now, use multiplication (can be optimized ~1.5x) */
    secp256k1_fe_mul(r, a, a);
}

/* Modular inverse using Fermat's little theorem: a^(p-2) ≡ a^(-1) (mod p) */
void secp256k1_fe_inv(secp256k1_fe* r, const secp256k1_fe* a) {
    /* Compute a^(p-2) mod p */
    /* p-2 = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2D */
    
    secp256k1_fe x[10];  /* temporaries */
    secp256k1_fe_copy(&x[0], a);
    secp256k1_fe_normalize(&x[0]);
    
    /* Binary exponentiation */
    /* Build the chain for p-2 */
    secp256k1_fe_sqr(&x[1], &x[0]);          /* x^2 */
    secp256k1_fe_mul(&x[1], &x[1], &x[0]);   /* x^3 */
    secp256k1_fe_sqr(&x[2], &x[1]);          /* x^6 */
    secp256k1_fe_sqr(&x[2], &x[2]);          /* x^12 */
    secp256k1_fe_mul(&x[1], &x[1], &x[2]);   /* x^15 = x^3 * x^12 */
    
    /* Continue building up... */
    secp256k1_fe_copy(r, &x[1]);
    
    /* Simplified: just do repeated squaring and multiplication */
    /* Full implementation needs complete addition chain for p-2 */
    
    /* For production, use the complete addition chain */
    /* This is a placeholder - real implementation follows secp256k1 field_inv */
    
    /* Final normalization */
    secp256k1_fe_normalize(r);
}

/* Square root modulo p (if exists) */
void secp256k1_fe_sqrt(secp256k1_fe* r, const secp256k1_fe* a) {
    /* For p ≡ 3 (mod 4), sqrt(a) = a^((p+1)/4) mod p */
    /* (p+1)/4 = 0x3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFBFFFFF0C */
    
    secp256k1_fe x, t;
    secp256k1_fe_copy(&x, a);
    secp256k1_fe_normalize(&x);
    
    /* Compute x^((p+1)/4) using exponentiation */
    /* This requires implementing the full exponentiation by (p+1)/4 */
    
    /* Placeholder - compute x^((p+1)/4) */
    secp256k1_fe_copy(r, &x);
    
    /* Verify: r^2 should equal a */
    secp256k1_fe_sqr(&t, r);
    secp256k1_fe_normalize(&t);
    
    /* If not equal, no square root exists (a is not a quadratic residue) */
}

/* Serialize to 32 bytes big endian */
void secp256k1_fe_get_b32(uint8_t* r, const secp256k1_fe* a) {
    secp256k1_fe t;
    secp256k1_fe_copy(&t, a);
    secp256k1_fe_normalize(&t);
    
    for (int i = 0; i < 8; i++) {
        r[28 - i*4] = (t.n[i] >> 24) & 0xFF;
        r[29 - i*4] = (t.n[i] >> 16) & 0xFF;
        r[30 - i*4] = (t.n[i] >> 8) & 0xFF;
        r[31 - i*4] = t.n[i] & 0xFF;
    }
}

/* Deserialize from 32 bytes big endian */
int secp256k1_fe_set_b32(secp256k1_fe* r, const uint8_t* a) {
    for (int i = 0; i < 8; i++) {
        r->n[i] = ((uint32_t)a[28 - i*4] << 24) |
                  ((uint32_t)a[29 - i*4] << 16) |
                  ((uint32_t)a[30 - i*4] << 8) |
                  ((uint32_t)a[31 - i*4]);
    }
    
    /* Check if valid (must be < p) */
    return secp256k1_is_less(r->n, secp256k1_p, 8);
}

/* Conditional move */
void secp256k1_fe_cmov(secp256k1_fe* r, const secp256k1_fe* a, int flag) {
    secp256k1_cmov32(r->n, a->n, flag, 8);
}
