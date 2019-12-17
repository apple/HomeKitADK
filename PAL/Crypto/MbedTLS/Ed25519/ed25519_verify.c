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
#include "ed25519_signature.h"

/*
 * Arithmetic on twisted Edwards curve y^2 - x^2 = 1 + dx^2y^2
 * with d = -(121665/121666) mod p
 *      d = 0x52036CEE2B6FFE738CC740797779E89800700A4D4141D8AB75EB4DCA135978A3
 *      p = 2**255 - 19
 *      p = 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED
 * Base point: y=4/5 mod p
 *      x = 0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A
 *      y = 0x6666666666666666666666666666666666666666666666666666666666666658
 * Base point order:
 *      l = 2**252 + 27742317777372353535851937790883648493
 *      l = 0x1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED
 */

typedef struct {
    unsigned char pk[32];
    PE_POINT q_table[16];
} EDP_SIGV_CTX;

extern const U_WORD _w_P[K_WORDS];
extern const U_WORD _w_di[K_WORDS];

extern const PA_POINT _w_base_folding8[256];
extern const U_WORD _w_NxBPO[16][K_WORDS];

#define _w_BPO _w_NxBPO[1]

#define _w_Zero _w_base_folding8[0].T2d
#define _w_One  _w_base_folding8[0].YpX

const U_WORD _w_I[K_WORDS] = /* sqrt(-1) */
        W256(0x4A0EA0B0, 0xC4EE1B27, 0xAD2FE478, 0x2F431806, 0x3DFBD7A7, 0x2B4D0099, 0x4FC1DF0B, 0x2B832480);

static const U_WORD _w_d[K_WORDS] =
        W256(0x135978A3, 0x75EB4DCA, 0x4141D8AB, 0x00700A4D, 0x7779E898, 0x8CC74079, 0x2B6FFE73, 0x52036CEE);

void ed25519_CalculateX(OUT U_WORD* X, IN const U_WORD* Y, U_WORD parity) {
    U_WORD u[K_WORDS], v[K_WORDS], a[K_WORDS], b[K_WORDS];

    /* Calculate sqrt((y^2 - 1)/(d*y^2 + 1)) */

    ecp_SqrReduce(u, Y);         /* u = y^2 */
    ecp_MulReduce(v, u, _w_d);   /* v = dy^2 */
    ecp_SubReduce(u, u, _w_One); /* u = y^2-1 */
    ecp_AddReduce(v, v, _w_One); /* v = dy^2+1 */

    /* Calculate:  sqrt(u/v) = u*v^3 * (u*v^7)^((p-5)/8) */

    ecp_SqrReduce(b, v);
    ecp_MulReduce(a, u, b);
    ecp_MulReduce(a, a, v); /* a = u*v^3 */
    ecp_SqrReduce(b, b);    /* b = v^4 */
    ecp_MulReduce(b, a, b); /* b = u*v^7 */
    ecp_ModExp2523(b, b);
    ecp_MulReduce(X, b, a);

    /* Check if we have correct sqrt, else, multiply by sqrt(-1) */

    ecp_SqrReduce(b, X);
    ecp_MulReduce(b, b, v);
    ecp_SubReduce(b, b, u);
    ecp_Mod(b);
    if (ecp_CmpNE(b, _w_Zero))
        ecp_MulReduce(X, X, _w_I);

    while (ecp_CmpLT(X, _w_P) == 0)
        ecp_Sub(X, X, _w_P);

    /* match parity */
    if (((X[0] ^ parity) & 1) != 0)
        ecp_Sub(X, _w_P, X);
}

void ed25519_UnpackPoint(Affine_POINT* r, const unsigned char* p) {
    U8 parity = ecp_DecodeInt(r->y, p);
    ed25519_CalculateX(r->x, r->y, parity);
}

void ecp_SrqMulReduce(U_WORD* Z, const U_WORD* X, int n, const U_WORD* Y) {
    U_WORD t[K_WORDS];
    ecp_SqrReduce(t, X);
    while (n-- > 1)
        ecp_SqrReduce(t, t);
    ecp_MulReduce(Z, t, Y);
}

void ecp_ModExp2523(U_WORD* Y, const U_WORD* X) {
    U_WORD x2[K_WORDS], x9[K_WORDS], x11[K_WORDS], x5[K_WORDS], x10[K_WORDS];
    U_WORD x20[K_WORDS], x50[K_WORDS], x100[K_WORDS], t[K_WORDS];

    ecp_SqrReduce(x2, X);                 /* 2 */
    ecp_SrqMulReduce(x9, x2, 2, X);       /* 9 */
    ecp_MulReduce(x11, x9, x2);           /* 11 */
    ecp_SqrReduce(t, x11);                /* 22 */
    ecp_MulReduce(x5, t, x9);             /* 31 = 2^5 - 2^0 */
    ecp_SrqMulReduce(x10, x5, 5, x5);     /* 2^10 - 2^0 */
    ecp_SrqMulReduce(x20, x10, 10, x10);  /* 2^20 - 2^0 */
    ecp_SrqMulReduce(t, x20, 20, x20);    /* 2^40 - 2^0 */
    ecp_SrqMulReduce(x50, t, 10, x10);    /* 2^50 - 2^0 */
    ecp_SrqMulReduce(x100, x50, 50, x50); /* 2^100 - 2^0 */
    ecp_SrqMulReduce(t, x100, 100, x100); /* 2^200 - 2^0 */
    ecp_SrqMulReduce(t, t, 50, x50);      /* 2^250 - 2^0 */
    ecp_SqrReduce(t, t);
    ecp_SqrReduce(t, t);    /* 2^252 - 2^2 */
    ecp_MulReduce(Y, t, X); /* 2^252 - 3 */
}

/*
    Assumptions: pre-computed q
    Cost: 8M + 6add
    Return: P = P + Q
*/
void edp_AddPoint(Ext_POINT* r, const Ext_POINT* p, const PE_POINT* q) {
    U_WORD a[K_WORDS], b[K_WORDS], c[K_WORDS], d[K_WORDS], e[K_WORDS];

    ecp_SubReduce(a, p->y, p->x); /* A = (Y1-X1)*(Y2-X2) */
    ecp_MulReduce(a, a, q->YmX);
    ecp_AddReduce(b, p->y, p->x); /* B = (Y1+X1)*(Y2+X2) */
    ecp_MulReduce(b, b, q->YpX);
    ecp_MulReduce(c, p->t, q->T2d); /* C = T1*2d*T2 */
    ecp_MulReduce(d, p->z, q->Z2);  /* D = Z1*2*Z2 */
    ecp_SubReduce(e, b, a);         /* E = B-A */
    ecp_AddReduce(b, b, a);         /* H = B+A */
    ecp_SubReduce(a, d, c);         /* F = D-C */
    ecp_AddReduce(d, d, c);         /* G = D+C */

    ecp_MulReduce(r->x, e, a); /* E*F */
    ecp_MulReduce(r->y, b, d); /* H*G */
    ecp_MulReduce(r->t, e, b); /* E*H */
    ecp_MulReduce(r->z, d, a); /* G*F */
}

int ed25519_VerifySignature(
        const unsigned char* signature, /* IN: signature (R,S) */
        const unsigned char* publicKey, /* IN: public key */
        const unsigned char* msg,
        size_t msg_size) /* IN: message to sign */
{
    EDP_SIGV_CTX ctx;

    ed25519_Verify_Init(&ctx, publicKey);

    return ed25519_Verify_Check(&ctx, signature, msg, msg_size);
}

#define QTABLE_SET(d, s) \
    edp_AddPoint(&T, &Q, &ctx->q_table[s]); \
    edp_ExtPoint2PE(&ctx->q_table[d], &T)

void* ed25519_Verify_Init(
        void* context,                  /* IO: null or context buffer to use */
        const unsigned char* publicKey) /* IN: [32 bytes] public key */
{
    int i;
    Ext_POINT Q, T;
    EDP_SIGV_CTX* ctx = (EDP_SIGV_CTX*) context;

    if (ctx == 0)
        ctx = (EDP_SIGV_CTX*) mem_alloc(sizeof(EDP_SIGV_CTX));

    if (ctx) {
        memcpy(ctx->pk, publicKey, 32);
        i = ecp_DecodeInt(Q.y, publicKey);
        ed25519_CalculateX(Q.x, Q.y, ~i); /* Invert parity for -Q */
        ecp_MulMod(Q.t, Q.x, Q.y);
        ecp_SetValue(Q.z, 1);

        /* pre-compute q-table */

        /* Calculate: Q0=Q, Q1=(2^64)*Q, Q2=(2^128)*Q, Q3=(2^192)*Q */

        ecp_SetValue(ctx->q_table[0].YpX, 1); /* -- -- -- -- */
        ecp_SetValue(ctx->q_table[0].YmX, 1);
        ecp_SetValue(ctx->q_table[0].T2d, 0);
        ecp_SetValue(ctx->q_table[0].Z2, 2);

        edp_ExtPoint2PE(&ctx->q_table[1], &Q); /* -- -- -- q0 */

        for (i = 0; i < 64; i++)
            edp_DoublePoint(&Q);

        edp_ExtPoint2PE(&ctx->q_table[2], &Q); /* -- -- q1 -- */
        QTABLE_SET(3, 1);                      /* -- -- q1 q0 */

        do
            edp_DoublePoint(&Q);
        while (++i < 128);

        edp_ExtPoint2PE(&ctx->q_table[4], &Q); /* -- q2 -- -- */
        QTABLE_SET(5, 1);                      /* -- q2 -- q0 */
        QTABLE_SET(6, 2);                      /* -- q2 q1 -- */
        QTABLE_SET(7, 3);                      /* -- q2 q1 q0 */

        do
            edp_DoublePoint(&Q);
        while (++i < 192);

        edp_ExtPoint2PE(&ctx->q_table[8], &Q); /* q3 -- -- -- */
        QTABLE_SET(9, 1);                      /* q3 -- -- q0 */
        QTABLE_SET(10, 2);                     /* q3 -- q1 -- */
        QTABLE_SET(11, 3);                     /* q3 -- q1 q0 */
        QTABLE_SET(12, 4);                     /* q3 q2 -- -- */
        QTABLE_SET(13, 5);                     /* q3 q2 -- q0 */
        QTABLE_SET(14, 6);                     /* q3 q2 q1 -- */
        QTABLE_SET(15, 7);                     /* q3 q2 q1 q0 */
    }
    return ctx;
}

void ed25519_Verify_Finish(void* ctx) {
    mem_free(ctx);
}

/*
    Assumptions: qtable = pre-computed Q
    Calculate: point R = a*P + b*Q  where P is base point
*/
static void edp_PolyPointMultiply(Affine_POINT* r, const U_WORD* a, const U_WORD* b, const PE_POINT* qtable) {
    int i = 1;
    Ext_POINT S;
    const PE_POINT* q0;
    U8 u[32], v[64];

    ecp_8Folds(u, a);
    ecp_4Folds(v, b);

    /* Set initial value of S */
    q0 = &qtable[v[0]];
    ecp_SubReduce(S.x, q0->YpX, q0->YmX); /* 2x */
    ecp_AddReduce(S.y, q0->YpX, q0->YmX); /* 2y */
    ecp_MulReduce(S.t, q0->T2d, _w_di);   /* 2xy */
    ecp_Copy(S.z, q0->Z2);                /* 2z */

    do { /* 31D + 31A */
        edp_DoublePoint(&S);
        edp_AddPoint(&S, &S, &qtable[v[i]]);
    } while (++i < 32);

    do { /* 32D + 64A */
        edp_DoublePoint(&S);
        edp_AddAffinePoint(&S, &_w_base_folding8[u[i - 32]]);
        edp_AddPoint(&S, &S, &qtable[v[i]]);
    } while (++i < 64);

    ecp_Inverse(S.z, S.z);
    ecp_MulMod(r->x, S.x, S.z);
    ecp_MulMod(r->y, S.y, S.z);
}

/*
    This function can be used for batch verification.
    Assumptions: context = ed25519_Verify_Init(pk)

*/
int ed25519_Verify_Check(
        const void* context,            /* IN: precomputes */
        const unsigned char* signature, /* IN: signature (R,S) */
        const unsigned char* msg,
        size_t msg_size) /* IN: message to sign */
{
    Affine_POINT T;
    U_WORD h[K_WORDS], s[K_WORDS];
    U8 md[SHA512_DIGEST_LENGTH];

    /* h = H(enc(R) + pk + m)  mod BPO */
    mbedtls_sha512_context ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, signature, 32); /* enc(R) */
    sha512_update(&ctx, ((EDP_SIGV_CTX*) context)->pk, 32);
    sha512_update(&ctx, msg, msg_size);
    sha512_final(&ctx, md);
    eco_DigestToWords(h, md);
    eco_Mod(h);

    /* T = s*P + h*(-Q) = (s - h*a)*P = r*P = R */

    ecp_BytesToWords(s, signature + 32);
    edp_PolyPointMultiply(&T, s, h, ((EDP_SIGV_CTX*) context)->q_table);
    ed25519_PackPoint(md, T.y, T.x[0]);

    return (memcmp(md, signature, 32) == 0) ? 1 : 0;
}
