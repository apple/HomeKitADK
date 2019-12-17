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

#ifndef __ed25519_signature_h__
#define __ed25519_signature_h__

#ifdef __cplusplus
extern "C" {
#endif

/* -- ed25519-sign ------------------------------------------------------------- */

#define ed25519_public_key_size  32
#define ed25519_secret_key_size  32
#define ed25519_private_key_size 64
#define ed25519_signature_size   64

/* Generate public key associated with the secret key */
void ed25519_CreateKeyPair(
        unsigned char* pubKey,    /* OUT: public key */
        unsigned char* privKey,   /* OUT: private key */
        const void* blinding,     /* IN: [optional] null or blinding context */
        const unsigned char* sk); /* IN: secret key (32 bytes) */

/* Generate message signature */
void ed25519_SignMessage(
        unsigned char* signature,     /* OUT:[64 bytes] signature (R,S) */
        const unsigned char* privKey, /* IN: [64 bytes] private key (sk,pk) */
        const void* blinding,         /* IN: [optional] null or blinding context */
        const unsigned char* msg,     /* IN: [msg_size bytes] message to sign */
        size_t msg_size);             /* IN: size of message */

void ed25519_Blinding_Init(
        EDP_BLINDING_CTX* ctx,     /* IO: blinding context */
        const unsigned char* seed, /* IN: [size bytes] random blinding seed */
        size_t size);              /* IN: size of blinding seed */

void ed25519_Blinding_Finish(EDP_BLINDING_CTX* ctx); /* IN: blinding context */

/* -- ed25519-verify ----------------------------------------------------------- */

/*  Single-phased signature validation.
    Returns 1 for SUCCESS and 0 for FAILURE
*/
int ed25519_VerifySignature(
        const unsigned char* signature, /* IN: [64 bytes] signature (R,S) */
        const unsigned char* publicKey, /* IN: [32 bytes] public key */
        const unsigned char* msg,       /* IN: [msg_size bytes] message to sign */
        size_t msg_size);               /* IN: size of message */

/*  First part of two-phase signature validation.
    This function creates context specifc to a given public key.
    Needs to be called once per public key
*/
void* ed25519_Verify_Init(
        void* context,                   /* IO: null or verify context to use */
        const unsigned char* publicKey); /* IN: [32 bytes] public key */

/*  Second part of two-phase signature validation.
    Input context is output of ed25519_Verify_Init() for associated public key.
    Call it once for each message/signature pairs
    Returns 1 for SUCCESS and 0 for FAILURE
*/
int ed25519_Verify_Check(
        const void* context,            /* IN: created by ed25519_Verify_Init */
        const unsigned char* signature, /* IN: signature (R,S) */
        const unsigned char* msg,       /* IN: message to sign */
        size_t msg_size);               /* IN: size of message */

/* Free up context memory */
void ed25519_Verify_Finish(void* ctx);

#ifdef __cplusplus
}
#endif
#endif /* __ed25519_signature_h__ */
