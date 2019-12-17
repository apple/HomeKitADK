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

/* Trim private key   */
void ecp_TrimSecretKey(U8* X) {
    X[0] &= 0xf8;
    X[31] = (X[31] | 0x40) & 0x7f;
}

/* Convert big-endian byte array to little-endian byte array and vice versa */
U8* ecp_ReverseByteOrder(OUT U8* Y, IN const U8* X) {
    int i;
    for (i = 0; i < 32; i++)
        Y[i] = X[31 - i];
    return Y;
}

/* Convert little-endian byte array to little-endian word array */
U32* ecp_BytesToWords(OUT U32* Y, IN const U8* X) {
    int i;
    M32 m;

    for (i = 0; i < 8; i++) {
        m.u8.b0 = *X++;
        m.u8.b1 = *X++;
        m.u8.b2 = *X++;
        m.u8.b3 = *X++;

        Y[i] = m.u32;
    }
    return Y;
}

/* Convert little-endian word array to little-endian byte array */
U8* ecp_WordsToBytes(OUT U8* Y, IN const U32* X) {
    int i;
    M32 m;

    for (i = 0; i < 32;) {
        m.u32 = *X++;
        Y[i++] = m.u8.b0;
        Y[i++] = m.u8.b1;
        Y[i++] = m.u8.b2;
        Y[i++] = m.u8.b3;
    }
    return Y;
}

U8* ecp_EncodeInt(OUT U8* Y, IN const U32* X, IN U8 parity) {
    int i;
    M32 m;

    for (i = 0; i < 28;) {
        m.u32 = *X++;
        Y[i++] = m.u8.b0;
        Y[i++] = m.u8.b1;
        Y[i++] = m.u8.b2;
        Y[i++] = m.u8.b3;
    }

    m.u32 = *X;
    Y[28] = m.u8.b0;
    Y[29] = m.u8.b1;
    Y[30] = m.u8.b2;
    Y[31] = (U8)((m.u8.b3 & 0x7f) | (parity << 7));

    return Y;
}

U8 ecp_DecodeInt(OUT U32* Y, IN const U8* X) {
    int i;
    M32 m;

    for (i = 0; i < 7; i++) {
        m.u8.b0 = *X++;
        m.u8.b1 = *X++;
        m.u8.b2 = *X++;
        m.u8.b3 = *X++;

        Y[i] = m.u32;
    }

    m.u8.b0 = *X++;
    m.u8.b1 = *X++;
    m.u8.b2 = *X++;
    m.u8.b3 = *X & 0x7f;

    Y[7] = m.u32;

    return (U8)((*X >> 7) & 1);
}

void ecp_4Folds(U8* Y, const U32* X) {
    int i, j;
    U8 a, b;
    for (i = 32; i-- > 0; Y++) {
        a = 0;
        b = 0;
        for (j = 8; j > 1;) {
            j -= 2;
            a = (a << 1) + ((X[j + 1] >> i) & 1);
            b = (b << 1) + ((X[j] >> i) & 1);
        }
        Y[0] = a;
        Y[32] = b;
    }
}

void ecp_8Folds(U8* Y, const U32* X) {
    int i, j;
    U8 a = 0;
    for (i = 32; i-- > 0;) {
        for (j = 8; j-- > 0;)
            a = (a << 1) + ((X[j] >> i) & 1);
        *Y++ = a;
    }
}
