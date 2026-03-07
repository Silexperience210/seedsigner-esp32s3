/**
 * @file secp256k1_field.h
 * Field operations for secp256k1 (modulo p = 2^256 - 2^32 - 977)
 * Constant-time implementations
 */

#ifndef SECP256K1_FIELD_H
#define SECP256K1_FIELD_H

#include "secp256k1.h"

/* Field element - 32 bytes, big endian normalized */
typedef struct {
    uint32_t n[8];  /* Stored as 8 x 32-bit limbs, little endian internally */
} secp256k1_fe;

/* Field constants */
extern const secp256k1_fe secp256k1_fe_p;
extern const secp256k1_fe secp256k1_fe_one;
extern const secp256k1_fe secp256k1_fe_zero;

/* Field operations - constant time */
void secp256k1_fe_normalize(secp256k1_fe* r);
void secp256k1_fe_normalize_weak(secp256k1_fe* r);
void secp256k1_fe_normalize_var(secp256k1_fe* r);

int secp256k1_fe_is_zero(const secp256k1_fe* a);
int secp256k1_fe_is_odd(const secp256k1_fe* a);
int secp256k1_fe_equal(const secp256k1_fe* a, const secp256k1_fe* b);

void secp256k1_fe_set_int(secp256k1_fe* r, int a);
void secp256k1_fe_clear(secp256k1_fe* a);
void secp256k1_fe_copy(secp256k1_fe* r, const secp256k1_fe* a);

void secp256k1_fe_add(secp256k1_fe* r, const secp256k1_fe* a, const secp256k1_fe* b);
void secp256k1_fe_negate(secp256k1_fe* r, const secp256k1_fe* a, int m);
void secp256k1_fe_mul(secp256k1_fe* r, const secp256k1_fe* a, const secp256k1_fe* b);
void secp256k1_fe_sqr(secp256k1_fe* r, const secp256k1_fe* a);
void secp256k1_fe_inv(secp256k1_fe* r, const secp256k1_fe* a);
void secp256k1_fe_sqrt(secp256k1_fe* r, const secp256k1_fe* a);

/* Serialization */
void secp256k1_fe_get_b32(uint8_t* r, const secp256k1_fe* a);
int secp256k1_fe_set_b32(secp256k1_fe* r, const uint8_t* a);

/* Conditional operations (constant time) */
void secp256k1_fe_cmov(secp256k1_fe* r, const secp256k1_fe* a, int flag);

#endif /* SECP256K1_FIELD_H */
