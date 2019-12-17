/* The MIT License (MIT)
 *
 * Copyright (c) 2015 mehdi sotoodeh
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "apple.h"
#include "curve25519_mehdi.h"

/*
    The curve used is y2 = x^3 + 486662x^2 + x, a Montgomery curve, over
    the prime field defined by the prime number 2^255 - 19, and it uses the
    base point x = 9.
    Protocol uses compressed elliptic point (only X coordinates), so it
    allows for efficient use of the Montgomery ladder for ECDH, using only
    XZ coordinates.

    The curve is birationally equivalent to Ed25519 (Twisted Edwards curve).

    b = 256
    p = 2**255 - 19
    l = 2**252 + 27742317777372353535851937790883648493

    This library is a constant-time implementation of field operations
*/

typedef struct {
    U32 X[8]; /* x = X/Z */
    U32 Z[8]; /*  */
} XZ_POINT;

const U32 _w_P[8] = { 0xFFFFFFED, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFFFFFF };

/* Maximum number of prime p that fits into 256-bits */
const U32 _w_maxP[8] = { /* 2*P < 2**256 */
                         0xFFFFFFDA, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};

void ecp_SetValue(U32* X, U32 value) {
    X[0] = value;
    X[1] = X[2] = X[3] = X[4] = X[5] = X[6] = X[7] = 0;
}

/* Y = X */
void ecp_Copy(U32* Y, const U32* X) {
    memcpy(Y, X, 8 * sizeof(U32));
}

int ecp_CmpNE(const U32* X, const U32* Y) {
    return ((X[0] ^ Y[0]) | (X[1] ^ Y[1]) | (X[2] ^ Y[2]) | (X[3] ^ Y[3]) | (X[4] ^ Y[4]) | (X[5] ^ Y[5]) |
            (X[6] ^ Y[6]) | (X[7] ^ Y[7]));
}

int ecp_CmpLT(const U32* X, const U32* Y) {
    U32 T[8];
    return ecp_Sub(T, X, Y);
}

#define ECP_ADD_C0(Y, X, V) \
    c.u64 = (U64)(X) + (V); \
    Y = c.u32.lo;
#define ECP_ADD_C1(Y, X) \
    c.u64 = (U64)(X) + c.u32.hi; \
    Y = c.u32.lo;

#define ECP_SUB_C0(Y, X, V) \
    c.s64 = (U64)(X) - (V); \
    Y = c.u32.lo;
#define ECP_SUB_C1(Y, X) \
    c.s64 = (U64)(X) + (S64) c.s32.hi; \
    Y = c.u32.lo;

#define ECP_MULSET_W0(Y, b, X) \
    c.u64 = (U64)(b) * (X); \
    Y = c.u32.lo;
#define ECP_MULSET_W1(Y, b, X) \
    c.u64 = (U64)(b) * (X) + c.u32.hi; \
    Y = c.u32.lo;

#define ECP_MULADD_W0(Z, Y, b, X) \
    c.u64 = (U64)(b) * (X) + (Y); \
    Z = c.u32.lo;
#define ECP_MULADD_W1(Z, Y, b, X) \
    c.u64 = (U64)(b) * (X) + (U64)(Y) + c.u32.hi; \
    Z = c.u32.lo;

#define ECP_ADD32(Z, X, Y) \
    c.u64 = (U64)(X) + (Y); \
    Z = c.u32.lo;
#define ECP_ADC32(Z, X, Y) \
    c.u64 = (U64)(X) + (U64)(Y) + c.u32.hi; \
    Z = c.u32.lo;
#define ECP_SUB32(Z, X, Y) \
    b.s64 = (S64)(X) - (Y); \
    Z = b.s32.lo;
#define ECP_SBC32(Z, X, Y) \
    b.s64 = (S64)(X) - (U64)(Y) + b.s32.hi; \
    Z = b.s32.lo;

/* Computes Z = X+Y */
U32 ecp_Add(U32* Z, const U32* X, const U32* Y) {
    M64 c;

    ECP_ADD32(Z[0], X[0], Y[0]);
    ECP_ADC32(Z[1], X[1], Y[1]);
    ECP_ADC32(Z[2], X[2], Y[2]);
    ECP_ADC32(Z[3], X[3], Y[3]);
    ECP_ADC32(Z[4], X[4], Y[4]);
    ECP_ADC32(Z[5], X[5], Y[5]);
    ECP_ADC32(Z[6], X[6], Y[6]);
    ECP_ADC32(Z[7], X[7], Y[7]);
    return c.u32.hi;
}

/* Computes Z = X-Y */
S32 ecp_Sub(U32* Z, const U32* X, const U32* Y) {
    M64 b;
    ECP_SUB32(Z[0], X[0], Y[0]);
    ECP_SBC32(Z[1], X[1], Y[1]);
    ECP_SBC32(Z[2], X[2], Y[2]);
    ECP_SBC32(Z[3], X[3], Y[3]);
    ECP_SBC32(Z[4], X[4], Y[4]);
    ECP_SBC32(Z[5], X[5], Y[5]);
    ECP_SBC32(Z[6], X[6], Y[6]);
    ECP_SBC32(Z[7], X[7], Y[7]);
    return b.s32.hi;
}

/* Computes Z = X+Y mod P */
void ecp_AddReduce(U32* Z, const U32* X, const U32* Y) {
    M64 c;
    c.u32.hi = ecp_Add(Z, X, Y) * 38;

    /* Z += c.u32.hi * 38 */
    ECP_ADD_C0(Z[0], Z[0], c.u32.hi);
    ECP_ADD_C1(Z[1], Z[1]);
    ECP_ADD_C1(Z[2], Z[2]);
    ECP_ADD_C1(Z[3], Z[3]);
    ECP_ADD_C1(Z[4], Z[4]);
    ECP_ADD_C1(Z[5], Z[5]);
    ECP_ADD_C1(Z[6], Z[6]);
    ECP_ADD_C1(Z[7], Z[7]);

    /* One more carry at most */
    ECP_ADD_C0(Z[0], Z[0], c.u32.hi * 38);
    ECP_ADD_C1(Z[1], Z[1]);
    ECP_ADD_C1(Z[2], Z[2]);
    ECP_ADD_C1(Z[3], Z[3]);
    ECP_ADD_C1(Z[4], Z[4]);
    ECP_ADD_C1(Z[5], Z[5]);
    ECP_ADD_C1(Z[6], Z[6]);
    ECP_ADD_C1(Z[7], Z[7]);
}

/* Computes Z = X-Y mod P */
void ecp_SubReduce(U32* Z, const U32* X, const U32* Y) {
    M64 c;
    c.u32.hi = ecp_Sub(Z, X, Y) & 38;

    ECP_SUB_C0(Z[0], Z[0], c.u32.hi);
    ECP_SUB_C1(Z[1], Z[1]);
    ECP_SUB_C1(Z[2], Z[2]);
    ECP_SUB_C1(Z[3], Z[3]);
    ECP_SUB_C1(Z[4], Z[4]);
    ECP_SUB_C1(Z[5], Z[5]);
    ECP_SUB_C1(Z[6], Z[6]);
    ECP_SUB_C1(Z[7], Z[7]);

    ECP_SUB_C0(Z[0], Z[0], c.u32.hi & 38);
    ECP_SUB_C1(Z[1], Z[1]);
    ECP_SUB_C1(Z[2], Z[2]);
    ECP_SUB_C1(Z[3], Z[3]);
    ECP_SUB_C1(Z[4], Z[4]);
    ECP_SUB_C1(Z[5], Z[5]);
    ECP_SUB_C1(Z[6], Z[6]);
    ECP_SUB_C1(Z[7], Z[7]);
}

void ecp_Mod(U32* X) {
    U32 T[8];
    U32 c = (U32) ecp_Sub(X, X, _w_P);

    /* set T = 0 if c=0, else T = P */

    T[0] = c & 0xFFFFFFED;
    T[1] = T[2] = T[3] = T[4] = T[5] = T[6] = c;
    T[7] = c >> 1;

    ecp_Add(X, X, T); /* X += 0 or P */

    /* In case there is another P there */

    c = (U32) ecp_Sub(X, X, _w_P);

    /* set T = 0 if c=0, else T = P */

    T[0] = c & 0xFFFFFFED;
    T[1] = T[2] = T[3] = T[4] = T[5] = T[6] = c;
    T[7] = c >> 1;

    ecp_Add(X, X, T); /* X += 0 or P */
}

/* Computes Y = b*X */
static void ecp_mul_set(U32* Y, U32 b, const U32* X) {
    M64 c;
    ECP_MULSET_W0(Y[0], b, X[0]);
    ECP_MULSET_W1(Y[1], b, X[1]);
    ECP_MULSET_W1(Y[2], b, X[2]);
    ECP_MULSET_W1(Y[3], b, X[3]);
    ECP_MULSET_W1(Y[4], b, X[4]);
    ECP_MULSET_W1(Y[5], b, X[5]);
    ECP_MULSET_W1(Y[6], b, X[6]);
    ECP_MULSET_W1(Y[7], b, X[7]);
    Y[8] = c.u32.hi;
}

/* Computes Y += b*X */
/* Addition is performed on lower 8-words of Y */
static void ecp_mul_add(U32* Y, U32 b, const U32* X) {
    M64 c;
    ECP_MULADD_W0(Y[0], Y[0], b, X[0]);
    ECP_MULADD_W1(Y[1], Y[1], b, X[1]);
    ECP_MULADD_W1(Y[2], Y[2], b, X[2]);
    ECP_MULADD_W1(Y[3], Y[3], b, X[3]);
    ECP_MULADD_W1(Y[4], Y[4], b, X[4]);
    ECP_MULADD_W1(Y[5], Y[5], b, X[5]);
    ECP_MULADD_W1(Y[6], Y[6], b, X[6]);
    ECP_MULADD_W1(Y[7], Y[7], b, X[7]);
    Y[8] = c.u32.hi;
}

/* Computes Z = Y + b*X and return carry */
void ecp_WordMulAddReduce(U32* Z, const U32* Y, U32 b, const U32* X) {
    M64 c;
    ECP_MULADD_W0(Z[0], Y[0], b, X[0]);
    ECP_MULADD_W1(Z[1], Y[1], b, X[1]);
    ECP_MULADD_W1(Z[2], Y[2], b, X[2]);
    ECP_MULADD_W1(Z[3], Y[3], b, X[3]);
    ECP_MULADD_W1(Z[4], Y[4], b, X[4]);
    ECP_MULADD_W1(Z[5], Y[5], b, X[5]);
    ECP_MULADD_W1(Z[6], Y[6], b, X[6]);
    ECP_MULADD_W1(Z[7], Y[7], b, X[7]);

    /* Z += c.u32.hi * 38 */
    ECP_MULADD_W0(Z[0], Z[0], c.u32.hi, 38);
    ECP_ADD_C1(Z[1], Z[1]);
    ECP_ADD_C1(Z[2], Z[2]);
    ECP_ADD_C1(Z[3], Z[3]);
    ECP_ADD_C1(Z[4], Z[4]);
    ECP_ADD_C1(Z[5], Z[5]);
    ECP_ADD_C1(Z[6], Z[6]);
    ECP_ADD_C1(Z[7], Z[7]);

    /* One more time at most */
    ECP_MULADD_W0(Z[0], Z[0], c.u32.hi, 38);
    ECP_ADD_C1(Z[1], Z[1]);
    ECP_ADD_C1(Z[2], Z[2]);
    ECP_ADD_C1(Z[3], Z[3]);
    ECP_ADD_C1(Z[4], Z[4]);
    ECP_ADD_C1(Z[5], Z[5]);
    ECP_ADD_C1(Z[6], Z[6]);
    ECP_ADD_C1(Z[7], Z[7]);
}

/* Computes Z = X*Y mod P. */
/* Output fits into 8 words but could be greater than P */
void ecp_MulReduce(U32* Z, const U32* X, const U32* Y) {
    U32 T[16];

    ecp_mul_set(T + 0, X[0], Y);
    ecp_mul_add(T + 1, X[1], Y);
    ecp_mul_add(T + 2, X[2], Y);
    ecp_mul_add(T + 3, X[3], Y);
    ecp_mul_add(T + 4, X[4], Y);
    ecp_mul_add(T + 5, X[5], Y);
    ecp_mul_add(T + 6, X[6], Y);
    ecp_mul_add(T + 7, X[7], Y);

    /* We have T = X*Y, now do the reduction in size */

    ecp_WordMulAddReduce(Z, T, 38, T + 8);
}

/* Computes Z = X*Y */
void ecp_Mul(U32* Z, const U32* X, const U32* Y) {
    ecp_mul_set(Z + 0, X[0], Y);
    ecp_mul_add(Z + 1, X[1], Y);
    ecp_mul_add(Z + 2, X[2], Y);
    ecp_mul_add(Z + 3, X[3], Y);
    ecp_mul_add(Z + 4, X[4], Y);
    ecp_mul_add(Z + 5, X[5], Y);
    ecp_mul_add(Z + 6, X[6], Y);
    ecp_mul_add(Z + 7, X[7], Y);
}

/* Computes Z = X*Y mod P. */
void ecp_SqrReduce(U32* Y, const U32* X) {
    /* TBD: Implementation is based on multiply */
    /*      Optimize for squaring */

    U32 T[16];

    ecp_mul_set(T + 0, X[0], X);
    ecp_mul_add(T + 1, X[1], X);
    ecp_mul_add(T + 2, X[2], X);
    ecp_mul_add(T + 3, X[3], X);
    ecp_mul_add(T + 4, X[4], X);
    ecp_mul_add(T + 5, X[5], X);
    ecp_mul_add(T + 6, X[6], X);
    ecp_mul_add(T + 7, X[7], X);

    /* We have T = X*X, now do the reduction in size */

    ecp_WordMulAddReduce(Y, T, 38, T + 8);
}

/* Computes Z = X*Y mod P. */
void ecp_MulMod(U32* Z, const U32* X, const U32* Y) {
    ecp_MulReduce(Z, X, Y);
    ecp_Mod(Z);
}

/* Courtesy of DJB */
/* Return out = 1/z mod P */
void ecp_Inverse(U32* out, const U32* z) {
    int i;
    U32 t0[8], t1[8], z2[8], z9[8], z11[8];
    U32 z2_5_0[8], z2_10_0[8], z2_20_0[8], z2_50_0[8], z2_100_0[8];

    /* 2 */ ecp_SqrReduce(z2, z);
    /* 4 */ ecp_SqrReduce(t1, z2);
    /* 8 */ ecp_SqrReduce(t0, t1);
    /* 9 */ ecp_MulReduce(z9, t0, z);
    /* 11 */ ecp_MulReduce(z11, z9, z2);
    /* 22 */ ecp_SqrReduce(t0, z11);
    /* 2^5 - 2^0 = 31 */ ecp_MulReduce(z2_5_0, t0, z9);

    /* 2^6 - 2^1 */ ecp_SqrReduce(t0, z2_5_0);
    /* 2^7 - 2^2 */ ecp_SqrReduce(t1, t0);
    /* 2^8 - 2^3 */ ecp_SqrReduce(t0, t1);
    /* 2^9 - 2^4 */ ecp_SqrReduce(t1, t0);
    /* 2^10 - 2^5 */ ecp_SqrReduce(t0, t1);
    /* 2^10 - 2^0 */ ecp_MulReduce(z2_10_0, t0, z2_5_0);

    /* 2^11 - 2^1 */ ecp_SqrReduce(t0, z2_10_0);
    /* 2^12 - 2^2 */ ecp_SqrReduce(t1, t0);
    /* 2^20 - 2^10 */ for (i = 2; i < 10; i += 2) {
        ecp_SqrReduce(t0, t1);
        ecp_SqrReduce(t1, t0);
    }
    /* 2^20 - 2^0 */ ecp_MulReduce(z2_20_0, t1, z2_10_0);

    /* 2^21 - 2^1 */ ecp_SqrReduce(t0, z2_20_0);
    /* 2^22 - 2^2 */ ecp_SqrReduce(t1, t0);
    /* 2^40 - 2^20 */ for (i = 2; i < 20; i += 2) {
        ecp_SqrReduce(t0, t1);
        ecp_SqrReduce(t1, t0);
    }
    /* 2^40 - 2^0 */ ecp_MulReduce(t0, t1, z2_20_0);

    /* 2^41 - 2^1 */ ecp_SqrReduce(t1, t0);
    /* 2^42 - 2^2 */ ecp_SqrReduce(t0, t1);
    /* 2^50 - 2^10 */ for (i = 2; i < 10; i += 2) {
        ecp_SqrReduce(t1, t0);
        ecp_SqrReduce(t0, t1);
    }
    /* 2^50 - 2^0 */ ecp_MulReduce(z2_50_0, t0, z2_10_0);

    /* 2^51 - 2^1 */ ecp_SqrReduce(t0, z2_50_0);
    /* 2^52 - 2^2 */ ecp_SqrReduce(t1, t0);
    /* 2^100 - 2^50 */ for (i = 2; i < 50; i += 2) {
        ecp_SqrReduce(t0, t1);
        ecp_SqrReduce(t1, t0);
    }
    /* 2^100 - 2^0 */ ecp_MulReduce(z2_100_0, t1, z2_50_0);

    /* 2^101 - 2^1 */ ecp_SqrReduce(t1, z2_100_0);
    /* 2^102 - 2^2 */ ecp_SqrReduce(t0, t1);
    /* 2^200 - 2^100 */ for (i = 2; i < 100; i += 2) {
        ecp_SqrReduce(t1, t0);
        ecp_SqrReduce(t0, t1);
    }
    /* 2^200 - 2^0 */ ecp_MulReduce(t1, t0, z2_100_0);

    /* 2^201 - 2^1 */ ecp_SqrReduce(t0, t1);
    /* 2^202 - 2^2 */ ecp_SqrReduce(t1, t0);
    /* 2^250 - 2^50 */ for (i = 2; i < 50; i += 2) {
        ecp_SqrReduce(t0, t1);
        ecp_SqrReduce(t1, t0);
    }
    /* 2^250 - 2^0 */ ecp_MulReduce(t0, t1, z2_50_0);

    /* 2^251 - 2^1 */ ecp_SqrReduce(t1, t0);
    /* 2^252 - 2^2 */ ecp_SqrReduce(t0, t1);
    /* 2^253 - 2^3 */ ecp_SqrReduce(t1, t0);
    /* 2^254 - 2^4 */ ecp_SqrReduce(t0, t1);
    /* 2^255 - 2^5 */ ecp_SqrReduce(t1, t0);
    /* 2^255 - 21 */ ecp_MulReduce(out, t1, z11);
}
