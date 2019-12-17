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

#include "curve25519_mehdi.h"
#include "ed25519_signature.h"
#include "apple.h"

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

extern const U_WORD _w_maxP[K_WORDS];
extern const U_WORD _w_NxBPO[16][K_WORDS];

#define _w_BPO _w_NxBPO[1]

static EDP_BLINDING_CTX edp_custom_blinding = {
    W256(0xD1DFA242, 0xAB91A857, 0xE9F62749, 0xE314C485, 0x48FE8FD3, 0xF00E7295, 0xD29CF9EF, 0x06A83629),
    W256(0xC724BEF6, 0x59D19EB7, 0x1A7ECF15, 0x5C439216, 0xFCBB0F20, 0xA02E4E62, 0xA41D8396, 0x2D8FD635),
    { W256(0xDA38075E, 0x33285265, 0x7C4AF98A, 0x1329C8E1, 0xA1D64651, 0x05761C7A, 0x22D98600, 0x0028E8FE),
      W256(0x333BA706, 0x842E7E42, 0x50F16F1D, 0x11FC488E, 0x28BCF020, 0x078534D6, 0x1A0870D7, 0xB9CD265C),
      W256(0x1D6F86C0, 0xA6D7476F, 0xC3BD3FF6, 0xF18C0B79, 0x512BF0EA, 0x6823C74C, 0xEA0B036A, 0x26708E65),
      W256(0x860B528A, 0x5C7CD5E5, 0xBFBDA927, 0x9834D9F4, 0xF696EA66, 0xED15167A, 0x375453BC, 0x5DA1B958) }
};

const U_WORD _w_2d[K_WORDS] = /* 2*d */
        W256(0x26B2F159, 0xEBD69B94, 0x8283B156, 0x00E0149A, 0xEEF3D130, 0x198E80F2, 0x56DFFCE7, 0x2406D9DC);
const U_WORD _w_di[K_WORDS] = /* 1/d */
        W256(0xCDC9F843, 0x25E0F276, 0x4279542E, 0x0B5DD698, 0xCDB9CF66, 0x2B162114, 0x14D5CE43, 0x40907ED2);

#include "base_folding8.h"

/*
    Reference: http://eprint.iacr.org/2008/522
    Cost: 7M + 7add
    Return: R = P + BasePoint
*/
void edp_AddBasePoint(Ext_POINT* p) {
    U_WORD a[K_WORDS], b[K_WORDS], c[K_WORDS], d[K_WORDS], e[K_WORDS];

    ecp_SubReduce(a, p->y, p->x); /* A = (Y1-X1)*(Y2-X2) */
    ecp_MulReduce(a, a, _w_base_folding8[1].YmX);
    ecp_AddReduce(b, p->y, p->x); /* B = (Y1+X1)*(Y2+X2) */
    ecp_MulReduce(b, b, _w_base_folding8[1].YpX);
    ecp_MulReduce(c, p->t, _w_base_folding8[1].T2d); /* C = T1*2d*T2 */
    ecp_AddReduce(d, p->z, p->z);                    /* D = 2*Z1 */
    ecp_SubReduce(e, b, a);                          /* E = B-A */
    ecp_AddReduce(b, b, a);                          /* H = B+A */
    ecp_SubReduce(a, d, c);                          /* F = D-C */
    ecp_AddReduce(d, d, c);                          /* G = D+C */

    ecp_MulReduce(p->x, e, a); /* E*F */
    ecp_MulReduce(p->y, b, d); /* H*G */
    ecp_MulReduce(p->t, e, b); /* E*H */
    ecp_MulReduce(p->z, d, a); /* G*F */
}

/*
    Assumptions: pre-computed q, q->Z=1
    Cost: 7M + 7add
    Return: P = P + Q
*/
void edp_AddAffinePoint(Ext_POINT* p, const PA_POINT* q) {
    U_WORD a[K_WORDS], b[K_WORDS], c[K_WORDS], d[K_WORDS], e[K_WORDS];
    ecp_SubReduce(a, p->y, p->x); /* A = (Y1-X1)*(Y2-X2) */
    ecp_MulReduce(a, a, q->YmX);
    ecp_AddReduce(b, p->y, p->x); /* B = (Y1+X1)*(Y2+X2) */
    ecp_MulReduce(b, b, q->YpX);
    ecp_MulReduce(c, p->t, q->T2d); /* C = T1*2d*T2 */
    ecp_AddReduce(d, p->z, p->z);   /* D = Z1*2*Z2 (Z2=1)*/
    ecp_SubReduce(e, b, a);         /* E = B-A */
    ecp_AddReduce(b, b, a);         /* H = B+A */
    ecp_SubReduce(a, d, c);         /* F = D-C */
    ecp_AddReduce(d, d, c);         /* G = D+C */

    ecp_MulReduce(p->x, e, a); /* E*F */
    ecp_MulReduce(p->y, b, d); /* H*G */
    ecp_MulReduce(p->t, e, b); /* E*H */
    ecp_MulReduce(p->z, d, a); /* G*F */
}

/*
    Reference: http://eprint.iacr.org/2008/522
    Cost: 4M + 4S + 7add
    Return: P = 2*P
*/
void edp_DoublePoint(Ext_POINT* p) {
    U_WORD a[K_WORDS], b[K_WORDS], c[K_WORDS], d[K_WORDS], e[K_WORDS];

    ecp_SqrReduce(a, p->x); /* A = X1^2 */
    ecp_SqrReduce(b, p->y); /* B = Y1^2 */
    ecp_SqrReduce(c, p->z); /* C = 2*Z1^2 */
    ecp_AddReduce(c, c, c);
    ecp_SubReduce(d, _w_maxP, a); /* D = -A */

    ecp_SubReduce(a, d, b);       /* H = D-B */
    ecp_AddReduce(d, d, b);       /* G = D+B */
    ecp_SubReduce(b, d, c);       /* F = G-C */
    ecp_AddReduce(e, p->x, p->y); /* E = (X1+Y1)^2-A-B = (X1+Y1)^2+H */
    ecp_SqrReduce(e, e);
    ecp_AddReduce(e, e, a);

    ecp_MulReduce(p->x, e, b); /* E*F */
    ecp_MulReduce(p->y, a, d); /* H*G */
    ecp_MulReduce(p->z, d, b); /* G*F */
    ecp_MulReduce(p->t, e, a); /* E*H */
}

/* -- FOLDING ---------------------------------------------------------------
//
//    The performance boost is achieved by a process that I call it FOLDING.
//    Folding can be viewed as an extension of Shamir's trick but it is based
//    on break down of the scalar multiplier of a*P into a polynomial of the
//    form:
//
//        a*P = SUM(a_i*2^(i*w))*P    for i = 0,1,2,...n-1
//
//        a*P = SUM(a_i*P_i)
//
//        where P_i = (2^(i*w))*P
//              n = number of folds
//              w = bit-length of a_i
//
//    For folding of 8, 256-bit multiplier 'a' is chopped into 8 limbs of
//    32-bits each (a_0, a_1,...a_7). P_0 - P_7 can be pre-calculated and
//    their 256-different permutations can be cached or hard-coded
//    directly into the code.
//    This arrangement combined with double-and-add approach reduces the
//    number of EC point calculations by a factor of 8. We only need 31
//    double & add operations.
//
//       +---+---+---+---+---+---+- .... -+---+---+---+---+---+---+
//  a = (|255|254|253|252|251|250|        | 5 | 4 | 3 | 2 | 1 | 0 |)
//       +---+---+---+---+---+---+- .... -+---+---+---+---+---+---+
//
//                     a_i                       P_i
//       +---+---+---+ .... -+---+---+---+    ----------
// a7 = (|255|254|253|       |226|225|224|) * (2**224)*P
//       +---+---+---+ .... -+---+---+---+
// a6 = (|225|224|223|       |194|193|192|) * (2**192)*P
//       +---+---+---+ .... -+---+---+---+
// a5 = (|191|190|189|       |162|161|160|) * (2**160)*P
//       +---+---+---+ .... -+---+---+---+
// a4 = (|159|158|157|       |130|129|128|) * (2**128)*P
//       +---+---+---+ .... -+---+---+---+
// a3 = (|127|126|125|       | 98| 97| 96|) * (2**96)*P
//       +---+---+---+ .... -+---+---+---+
// a2 = (| 95| 94| 93|       | 66| 65| 64|) * (2**64)*P
//       +---+---+---+ .... -+---+---+---+
// a1 = (| 63| 62| 61|       | 34| 33| 32|) * (2**32)*P
//       +---+---+---+ .... -+---+---+---+
// a0 = (| 31| 30| 29|       | 2 | 1 | 0 |) * (2**0)*P
//       +---+---+---+ .... -+---+---+---+
//         |   |                   |   |
//         |   +--+                |   +--+
//         |      |                |      |
//         V      V     slices     V      V
//       +---+  +---+    ....    +---+  +---+
//       |255|  |254|            |225|  |224|   P7
//       +---+  +---+    ....    +---+  +---+
//       |225|  |224|            |193|  |192|   P6
//       +---+  +---+    ....    +---+  +---+
//       |191|  |190|            |161|  |160|   P5
//       +---+  +---+    ....    +---+  +---+
//       |159|  |158|            |129|  |128|   P4
//       +---+  +---+    ....    +---+  +---+
//       |127|  |126|            | 97|  | 96|   P3
//       +---+  +---+    ....    +---+  +---+
//       | 95|  | 94|            | 65|  | 64|   P2
//       +---+  +---+    ....    +---+  +---+
//       | 63|  | 62|            | 33|  | 32|   P1
//       +---+  +---+    ....    +---+  +---+
//       | 31|  | 30|            | 1 |  | 0 |   P0
//       +---+  +---+    ....    +---+  +---+
// cut[]:  0      1      ....      30     31
// --------------------------------------------------------------------------
// Return S = a*P where P is ed25519 base point and R is random
*/
void edp_BasePointMult(OUT Ext_POINT* S, IN const U_WORD* sk, IN const U_WORD* R) {
    int i = 1;
    U8 cut[32];
    const PA_POINT* p0;

    ecp_8Folds(cut, sk);

    p0 = &_w_base_folding8[cut[0]];

    ecp_SubReduce(S->x, p0->YpX, p0->YmX); /* 2x */
    ecp_AddReduce(S->y, p0->YpX, p0->YmX); /* 2y */
    ecp_MulReduce(S->t, p0->T2d, _w_di);   /* 2xy */

    /* Randomize starting point */

    ecp_AddReduce(S->z, R, R);    /* Z = 2R */
    ecp_MulReduce(S->x, S->x, R); /* X = 2xR */
    ecp_MulReduce(S->t, S->t, R); /* T = 2xyR */
    ecp_MulReduce(S->y, S->y, R); /* Y = 2yR */

    do {
        edp_DoublePoint(S);
        edp_AddAffinePoint(S, &_w_base_folding8[cut[i]]);
    } while (i++ < 31);
}

void edp_BasePointMultiply(OUT Affine_POINT* R, IN const U_WORD* sk, IN const void* blinding) {
    Ext_POINT S;
    U_WORD t[K_WORDS];

    if (blinding) {
        eco_AddReduce(t, sk, ((EDP_BLINDING_CTX*) blinding)->bl);
        edp_BasePointMult(&S, t, ((EDP_BLINDING_CTX*) blinding)->zr);
        edp_AddPoint(&S, &S, &((EDP_BLINDING_CTX*) blinding)->BP);
    } else {
        edp_BasePointMult(&S, sk, edp_custom_blinding.zr);
    }

    ecp_Inverse(S.z, S.z);
    ecp_MulMod(R->x, S.x, S.z);
    ecp_MulMod(R->y, S.y, S.z);
}

void edp_ExtPoint2PE(PE_POINT* r, const Ext_POINT* p) {
    ecp_AddReduce(r->YpX, p->y, p->x);
    ecp_SubReduce(r->YmX, p->y, p->x);
    ecp_MulReduce(r->T2d, p->t, _w_2d);
    ecp_AddReduce(r->Z2, p->z, p->z);
}

/* -- Blinding -------------------------------------------------------------
//
//  Blinding is a measure to protect against side channel attacks.
//  Blinding randomizes the scalar multiplier.
//
//  Instead of calculating a*P, calculate (a+b mod BPO)*P + B
//
//  Where b = random blinding and B = -b*P
//
// -------------------------------------------------------------------------
*/
void ed25519_Blinding_Init(
        EDP_BLINDING_CTX* ctx,     /* IO: blinding context */
        const unsigned char* seed, /* IN: [size bytes] random blinding seed */
        size_t size)               /* IN: size of blinding seed */
{
    struct {
        Ext_POINT T;
        U_WORD t[K_WORDS];
        U8 digest[SHA512_DIGEST_LENGTH];
    } d;

    /* Use edp_custom_blinding to protect generation of the new blinder */

    mbedtls_sha512_context sha_ctx;
    sha512_init(&sha_ctx);
    sha512_update(&sha_ctx, (const uint8_t*) edp_custom_blinding.zr, 32);
    sha512_update(&sha_ctx, seed, size);
    sha512_final(&sha_ctx, d.digest);

    ecp_BytesToWords(ctx->zr, d.digest + 32);
    ecp_BytesToWords(d.t, d.digest);
    eco_Mod(d.t);
    ecp_Sub(ctx->bl, _w_BPO, d.t);

    eco_AddReduce(d.t, d.t, edp_custom_blinding.bl);
    edp_BasePointMult(&d.T, d.t, edp_custom_blinding.zr);
    edp_AddPoint(&d.T, &d.T, &edp_custom_blinding.BP);

    edp_ExtPoint2PE(&ctx->BP, &d.T);

    /* clear potentially sensitive data */
    mem_clear(&d, sizeof(d));
}

void ed25519_Blinding_Finish(EDP_BLINDING_CTX* ctx) /* IN: blinding context */
{
    mem_clear(ctx, sizeof(EDP_BLINDING_CTX));
}

/* Generate public and private key pair associated with the secret key */
void ed25519_CreateKeyPair(
        unsigned char* pubKey,   /* OUT: public key */
        unsigned char* privKey,  /* OUT: private key */
        const void* blinding,    /* IN: [optional] null or blinding context */
        const unsigned char* sk) /* IN: secret key (32 bytes) */
{
    U8 md[SHA512_DIGEST_LENGTH];
    U_WORD t[K_WORDS];
    Affine_POINT Q;

    /* [a:b] = H(sk) */
    HAP_sha512(md, sk, 32);
    ecp_TrimSecretKey(md);

    ecp_BytesToWords(t, md);
    edp_BasePointMultiply(&Q, t, blinding);
    ed25519_PackPoint(pubKey, Q.y, Q.x[0]);

    memcpy(privKey, sk, 32);
    memcpy(privKey + 32, pubKey, 32);
}

/*
 * Generate message signature
 */
void ed25519_SignMessage(
        unsigned char* signature,     /* OUT: [64 bytes] signature (R,S) */
        const unsigned char* privKey, /*  IN: [64 bytes] private key (sk,pk) */
        const void* blinding,         /*  IN: [optional] null or blinding context */
        const unsigned char* msg,     /*  IN: [msg_size bytes] message to sign */
        size_t msg_size) {
    Affine_POINT R;
    U_WORD a[K_WORDS], t[K_WORDS], r[K_WORDS];
    U8 md[SHA512_DIGEST_LENGTH];

    /* [a:b] = H(sk) */
    HAP_sha512(md, privKey, 32);
    ecp_TrimSecretKey(md); /* a = first 32 bytes */
    ecp_BytesToWords(a, md);

    /* r = H(b + m) mod BPO */
    mbedtls_sha512_context ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, md + 32, 32);
    sha512_update(&ctx, msg, msg_size);
    sha512_final(&ctx, md);
    eco_DigestToWords(r, md);
    eco_Mod(r); /* r mod BPO */

    /* R = r*P */
    edp_BasePointMultiply(&R, r, blinding);
    ed25519_PackPoint(signature, R.y, R.x[0]); /* R part of signature */

    /* S = r + H(encoded(R) + pk + m) * a  mod BPO */
    sha512_init(&ctx);
    sha512_update(&ctx, signature, 32);    /* encoded(R) */
    sha512_update(&ctx, privKey + 32, 32); /* pk */
    sha512_update(&ctx, msg, msg_size);    /* m */
    sha512_final(&ctx, md);
    eco_DigestToWords(t, md);

    eco_MulReduce(t, t, a); /* h()*a */
    eco_AddReduce(t, t, r);
    eco_Mod(t);
    ecp_WordsToBytes(signature + 32, t); /* S part of signature */

    /* Clear sensitive data */
    ecp_SetValue(a, 0);
    ecp_SetValue(r, 0);
}
