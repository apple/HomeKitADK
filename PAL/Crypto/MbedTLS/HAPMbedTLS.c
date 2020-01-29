// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"
#include "HAPCrypto.h"
#include "HAPPlatform.h"

#include <string.h>
#include <stdlib.h>

#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#include "mbedtls/md.h"
#include "mbedtls/hkdf.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/chachapoly.h"
#include "mbedtls/aes.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/bignum.h"

static void sha512_init(mbedtls_sha512_context* ctx) {
    mbedtls_sha512_init(ctx);
    int ret = mbedtls_sha512_starts_ret(ctx, 0);
    HAPAssert(ret == 0);
}

static void sha512_update(mbedtls_sha512_context* ctx, const uint8_t* data, size_t size) {
    int ret = mbedtls_sha512_update_ret(ctx, data, size);
    HAPAssert(ret == 0);
}

static void sha512_final(mbedtls_sha512_context* ctx, uint8_t md[SHA512_BYTES]) {
    int ret = mbedtls_sha512_finish_ret(ctx, md);
    HAPAssert(ret == 0);
    mbedtls_sha512_free(ctx);
}

#ifndef HAVE_CUSTOM_ED25519

#include "Ed25519/curve25519_mehdi.c"
#include "Ed25519/curve25519_utils.c"
#include "Ed25519/curve25519_order.c"
#include "Ed25519/ed25519_sign.c"
#include "Ed25519/ed25519_verify.c"

#define USE_AND_CLEAR(name, size, X) \
    do { \
        uint8_t name[size]; \
        X; \
        memset(name, 0, sizeof name); \
    } while (0)

#define WITH_BLINDING(X) \
    do { \
        uint8_t seed[64]; \
        HAPPlatformRandomNumberFill(seed, sizeof seed); \
        EDP_BLINDING_CTX ctx; \
        ed25519_Blinding_Init(&ctx, seed, sizeof seed); \
        X; \
        ed25519_Blinding_Finish(&ctx); \
    } while (0)

void HAP_ed25519_public_key(uint8_t pk[ED25519_PUBLIC_KEY_BYTES], const uint8_t sk[ED25519_SECRET_KEY_BYTES]) {
    WITH_BLINDING(
            { USE_AND_CLEAR(privKey, ed25519_private_key_size, { ed25519_CreateKeyPair(pk, privKey, &ctx, sk); }); });
}

void HAP_ed25519_sign(
        uint8_t sig[ED25519_BYTES],
        const uint8_t* m,
        size_t m_len,
        const uint8_t sk[ED25519_SECRET_KEY_BYTES],
        const uint8_t pk[ED25519_PUBLIC_KEY_BYTES]) {
    WITH_BLINDING({
        USE_AND_CLEAR(privKey, ed25519_private_key_size, {
            memcpy(privKey, sk, ED25519_SECRET_KEY_BYTES);
            memcpy(privKey + ED25519_SECRET_KEY_BYTES, pk, ED25519_PUBLIC_KEY_BYTES);
            ed25519_SignMessage(sig, privKey, &ctx, m, m_len);
        });
    });
}

int HAP_ed25519_verify(
        const uint8_t sig[ED25519_BYTES],
        const uint8_t* m,
        size_t m_len,
        const uint8_t pk[ED25519_PUBLIC_KEY_BYTES]) {
    int ret = ed25519_VerifySignature(sig, pk, m, m_len);
    return (ret == 1) ? 0 : -1;
}

#endif

static int blinding_rng(void* context, uint8_t* buffer, size_t n) {
    HAPPlatformRandomNumberFill(buffer, n);
    return 0;
}

#define WITH(type, name, init, free, X) \
    do { \
        type name; \
        init(&name); \
        X; \
        free(&name); \
    } while (0)

#define WITH_ECP_KEYPAIR(name, X) WITH(mbedtls_ecp_keypair, name, mbedtls_ecp_keypair_init, mbedtls_ecp_keypair_free, X)
#define WITH_ECDH(name, X)        WITH(mbedtls_ecdh_context, name, mbedtls_ecdh_init, mbedtls_ecdh_free, X)

void HAP_X25519_scalarmult(
        uint8_t r[X25519_BYTES],
        const uint8_t n[X25519_SCALAR_BYTES],
        const uint8_t p[X25519_BYTES]) {
    WITH_ECP_KEYPAIR(our_key, {
        int ret = mbedtls_ecp_read_key(MBEDTLS_ECP_DP_CURVE25519, &our_key, n, X25519_SCALAR_BYTES);
        HAPAssert(ret == 0);
        WITH_ECP_KEYPAIR(their_key, {
            ret = mbedtls_ecp_group_load(&their_key.grp, MBEDTLS_ECP_DP_CURVE25519);
            HAPAssert(ret == 0);
            ret = mbedtls_ecp_point_read_binary(&their_key.grp, &their_key.Q, p, X25519_BYTES);
            HAPAssert(ret == 0);
            WITH_ECDH(ecdh, {
                ret = mbedtls_ecdh_get_params(&ecdh, &their_key, MBEDTLS_ECDH_THEIRS);
                HAPAssert(ret == 0);
                ret = mbedtls_ecdh_get_params(&ecdh, &our_key, MBEDTLS_ECDH_OURS);
                HAPAssert(ret == 0);
                size_t out_len;
                ret = mbedtls_ecdh_calc_secret(&ecdh, &out_len, r, X25519_BYTES, blinding_rng, NULL);
                HAPAssert(ret == 0);
                HAPAssert(out_len == X25519_BYTES);
            });
        });
    });
}

void HAP_X25519_scalarmult_base(uint8_t r[X25519_BYTES], const uint8_t n[X25519_SCALAR_BYTES]) {
    WITH_ECP_KEYPAIR(key, {
        int ret = mbedtls_ecp_read_key(MBEDTLS_ECP_DP_CURVE25519, &key, n, X25519_SCALAR_BYTES);
        HAPAssert(ret == 0);
        ret = mbedtls_ecp_mul(&key.grp, &key.Q, &key.d, &key.grp.G, blinding_rng, NULL);
        HAPAssert(ret == 0);
        size_t out_len;
        ret = mbedtls_ecp_point_write_binary(&key.grp, &key.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &out_len, r, X25519_BYTES);
        HAPAssert(ret == 0);
        HAPAssert(out_len == X25519_BYTES);
    });
}

#ifndef CUSTOM_SRP

// MbedTLS doesn't support SRP6a with SHA512 so we have to do this on foot

static void
        Calc_x(uint8_t x[SHA512_BYTES],
               const uint8_t salt[SRP_SALT_BYTES],
               const uint8_t* user,
               size_t user_len,
               const uint8_t* pass,
               size_t pass_len) {
    mbedtls_sha512_context ctx;

    sha512_init(&ctx);
    sha512_update(&ctx, user, user_len);
    sha512_update(&ctx, (const uint8_t*) ":", 1);
    sha512_update(&ctx, pass, pass_len);
    sha512_final(&ctx, x);

    sha512_init(&ctx);
    sha512_update(&ctx, salt, SRP_SALT_BYTES);
    sha512_update(&ctx, x, SHA512_BYTES);
    sha512_final(&ctx, x);
}

// TODO: cache mbedtls_mpi of N and G? and maybe cache temporary p for mbedtls_mpi_exp_mod?

// SRP 3072-bit prime number
static const uint8_t N_3072[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9, 0x0f, 0xda, 0xa2, 0x21, 0x68, 0xc2, 0x34, 0xc4, 0xc6, 0x62,
    0x8b, 0x80, 0xdc, 0x1c, 0xd1, 0x29, 0x02, 0x4e, 0x08, 0x8a, 0x67, 0xcc, 0x74, 0x02, 0x0b, 0xbe, 0xa6, 0x3b, 0x13,
    0x9b, 0x22, 0x51, 0x4a, 0x08, 0x79, 0x8e, 0x34, 0x04, 0xdd, 0xef, 0x95, 0x19, 0xb3, 0xcd, 0x3a, 0x43, 0x1b, 0x30,
    0x2b, 0x0a, 0x6d, 0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d, 0x6d, 0x51, 0xc2, 0x45, 0xe4, 0x85, 0xb5, 0x76,
    0x62, 0x5e, 0x7e, 0xc6, 0xf4, 0x4c, 0x42, 0xe9, 0xa6, 0x37, 0xed, 0x6b, 0x0b, 0xff, 0x5c, 0xb6, 0xf4, 0x06, 0xb7,
    0xed, 0xee, 0x38, 0x6b, 0xfb, 0x5a, 0x89, 0x9f, 0xa5, 0xae, 0x9f, 0x24, 0x11, 0x7c, 0x4b, 0x1f, 0xe6, 0x49, 0x28,
    0x66, 0x51, 0xec, 0xe4, 0x5b, 0x3d, 0xc2, 0x00, 0x7c, 0xb8, 0xa1, 0x63, 0xbf, 0x05, 0x98, 0xda, 0x48, 0x36, 0x1c,
    0x55, 0xd3, 0x9a, 0x69, 0x16, 0x3f, 0xa8, 0xfd, 0x24, 0xcf, 0x5f, 0x83, 0x65, 0x5d, 0x23, 0xdc, 0xa3, 0xad, 0x96,
    0x1c, 0x62, 0xf3, 0x56, 0x20, 0x85, 0x52, 0xbb, 0x9e, 0xd5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6d, 0x67, 0x0c, 0x35,
    0x4e, 0x4a, 0xbc, 0x98, 0x04, 0xf1, 0x74, 0x6c, 0x08, 0xca, 0x18, 0x21, 0x7c, 0x32, 0x90, 0x5e, 0x46, 0x2e, 0x36,
    0xce, 0x3b, 0xe3, 0x9e, 0x77, 0x2c, 0x18, 0x0e, 0x86, 0x03, 0x9b, 0x27, 0x83, 0xa2, 0xec, 0x07, 0xa2, 0x8f, 0xb5,
    0xc5, 0x5d, 0xf0, 0x6f, 0x4c, 0x52, 0xc9, 0xde, 0x2b, 0xcb, 0xf6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7c,
    0xea, 0x95, 0x6a, 0xe5, 0x15, 0xd2, 0x26, 0x18, 0x98, 0xfa, 0x05, 0x10, 0x15, 0x72, 0x8e, 0x5a, 0x8a, 0xaa, 0xc4,
    0x2d, 0xad, 0x33, 0x17, 0x0d, 0x04, 0x50, 0x7a, 0x33, 0xa8, 0x55, 0x21, 0xab, 0xdf, 0x1c, 0xba, 0x64, 0xec, 0xfb,
    0x85, 0x04, 0x58, 0xdb, 0xef, 0x0a, 0x8a, 0xea, 0x71, 0x57, 0x5d, 0x06, 0x0c, 0x7d, 0xb3, 0x97, 0x0f, 0x85, 0xa6,
    0xe1, 0xe4, 0xc7, 0xab, 0xf5, 0xae, 0x8c, 0xdb, 0x09, 0x33, 0xd7, 0x1e, 0x8c, 0x94, 0xe0, 0x4a, 0x25, 0x61, 0x9d,
    0xce, 0xe3, 0xd2, 0x26, 0x1a, 0xd2, 0xee, 0x6b, 0xf1, 0x2f, 0xfa, 0x06, 0xd9, 0x8a, 0x08, 0x64, 0xd8, 0x76, 0x02,
    0x73, 0x3e, 0xc8, 0x6a, 0x64, 0x52, 0x1f, 0x2b, 0x18, 0x17, 0x7b, 0x20, 0x0c, 0xbb, 0xe1, 0x17, 0x57, 0x7a, 0x61,
    0x5d, 0x6c, 0x77, 0x09, 0x88, 0xc0, 0xba, 0xd9, 0x46, 0xe2, 0x08, 0xe2, 0x4f, 0xa0, 0x74, 0xe5, 0xab, 0x31, 0x43,
    0xdb, 0x5b, 0xfc, 0xe0, 0xfd, 0x10, 0x8e, 0x4b, 0x82, 0xd1, 0x20, 0xa9, 0x3a, 0xd2, 0xca, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
};

static const uint8_t g_3072[] = {
    0x05,
};

#define MPI_READ_BINARY(m, d, n) \
    do { \
        int ret = mbedtls_mpi_read_binary(&m, d, n); \
        HAPAssert(ret == 0); \
    } while (0)

#define MPI_WRITE_BINARY(m, d, n) \
    do { \
        int ret = mbedtls_mpi_write_binary(&m, d, n); \
        HAPAssert(ret == 0); \
    } while (0)

#define WITH_BN(name, X) \
    do { \
        mbedtls_mpi name; \
        mbedtls_mpi_init(&name); \
        X; \
        mbedtls_mpi_free(&name); \
    } while (0)

#define BN_FROM_BYTES(name, b, bl, X) \
    WITH_BN(name, { \
        MPI_READ_BINARY(name, b, bl); \
        X; \
    })

#define WRAP_BN_BYTES(name, b, bl, X) \
    WITH_BN(name, { \
        X; \
        MPI_WRITE_BINARY(name, b, bl); \
    })

#define WITH_gN(X) \
    BN_FROM_BYTES(g, g_3072, sizeof g_3072, { BN_FROM_BYTES(N, (const uint8_t*) N_3072, sizeof N_3072, { X; }); })

void HAP_srp_verifier(
        uint8_t v[SRP_VERIFIER_BYTES],
        const uint8_t salt[SRP_SALT_BYTES],
        const uint8_t* user,
        size_t user_len,
        const uint8_t* pass,
        size_t pass_len) {
    uint8_t h[SHA512_BYTES];
    Calc_x(h, salt, user, user_len, pass, pass_len);
    BN_FROM_BYTES(x, h, SHA512_BYTES, {
        WRAP_BN_BYTES(verifier, v, SRP_VERIFIER_BYTES, {
            WITH_gN({
                int ret = mbedtls_mpi_exp_mod(&verifier, &g, &x, &N, NULL);
                HAPAssert(ret == 0);
            });
        });
    });
}

static void Calc_k(mbedtls_mpi* result) {
    uint8_t g[SRP_PRIME_BYTES];
    memset(g, 0, sizeof g);
    g[sizeof g - 1] = g_3072[0];
    uint8_t k[SHA512_BYTES];
    mbedtls_sha512_context ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, (const uint8_t*) N_3072, sizeof N_3072);
    sha512_update(&ctx, g, SRP_PRIME_BYTES);
    sha512_final(&ctx, k);
    int ret = mbedtls_mpi_read_binary(result, k, sizeof k);
    HAPAssert(ret == 0);
}

static void Calc_B(mbedtls_mpi* B, const mbedtls_mpi* b, const mbedtls_mpi* v) {
    WITH_gN({
        WITH_BN(gb, {
            int ret = mbedtls_mpi_exp_mod(&gb, &g, b, &N, NULL);
            HAPAssert(ret == 0);
            WITH_BN(k, {
                Calc_k(&k);
                WITH_BN(kv, {
                    ret = mbedtls_mpi_mul_mpi(&kv, v, &k);
                    HAPAssert(ret == 0);
                    ret = mbedtls_mpi_mod_mpi(&kv, &kv, &N);
                    HAPAssert(ret == 0);
                    ret = mbedtls_mpi_add_mpi(B, &gb, &kv);
                    HAPAssert(ret == 0);
                    ret = mbedtls_mpi_mod_mpi(B, B, &N);
                    HAPAssert(ret == 0);
                });
            });
        });
    });
}

void HAP_srp_public_key(
        uint8_t pub_b[SRP_PUBLIC_KEY_BYTES],
        const uint8_t priv_b[SRP_SECRET_KEY_BYTES],
        const uint8_t v[SRP_VERIFIER_BYTES]) {
    BN_FROM_BYTES(b, priv_b, SRP_SECRET_KEY_BYTES, {
        BN_FROM_BYTES(verifier, v, SRP_VERIFIER_BYTES, {
            WRAP_BN_BYTES(B, pub_b, SRP_PUBLIC_KEY_BYTES, { Calc_B(&B, &b, &verifier); });
        });
    });
}

void HAP_srp_scrambling_parameter(
        uint8_t u[SRP_SCRAMBLING_PARAMETER_BYTES],
        const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
        const uint8_t pub_b[SRP_PUBLIC_KEY_BYTES]) {
    mbedtls_sha512_context ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, pub_a, SRP_PUBLIC_KEY_BYTES);
    sha512_update(&ctx, pub_b, SRP_PUBLIC_KEY_BYTES);
    sha512_final(&ctx, u);
}

int HAP_srp_premaster_secret(
        uint8_t s[SRP_PREMASTER_SECRET_BYTES],
        const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
        const uint8_t priv_b[SRP_SECRET_KEY_BYTES],
        const uint8_t u[SRP_SCRAMBLING_PARAMETER_BYTES],
        const uint8_t v[SRP_VERIFIER_BYTES]) {
    bool isAValid = false;
    BN_FROM_BYTES(A, pub_a, SRP_PUBLIC_KEY_BYTES, {
        // Refer RFC 5054: https://tools.ietf.org/html/rfc5054
        // Section 2.5.4
        // Fail if A%N == 0
        WITH_gN({
            WITH_BN(rem, {
                int ret = mbedtls_mpi_mod_mpi(&rem, &A, &N);
                HAPAssert(ret == 0);
                if (mbedtls_mpi_cmp_int(&rem, 0) != 0) {
                    isAValid = true;
                }
            });
        });

        BN_FROM_BYTES(b, priv_b, SRP_SECRET_KEY_BYTES, {
            BN_FROM_BYTES(u_, u, SRP_SCRAMBLING_PARAMETER_BYTES, {
                BN_FROM_BYTES(v_, v, SRP_VERIFIER_BYTES, {
                    WRAP_BN_BYTES(s_, s, SRP_PREMASTER_SECRET_BYTES, {
                        WITH_gN({
                            int ret = mbedtls_mpi_exp_mod(&s_, &v_, &u_, &N, NULL);
                            HAPAssert(ret == 0);
                            ret = mbedtls_mpi_mul_mpi(&s_, &A, &s_);
                            HAPAssert(ret == 0);
                            ret = mbedtls_mpi_mod_mpi(&s_, &s_, &N);
                            HAPAssert(ret == 0);
                            ret = mbedtls_mpi_exp_mod(&s_, &s_, &b, &N, NULL);
                            HAPAssert(ret == 0);
                        });
                    });
                });
            });
        });
    });
    return (isAValid) ? 0 : 1;
}

static size_t Count_Leading_Zeroes(const uint8_t* start, size_t n) {
    const uint8_t* p = start;
    const uint8_t* stop = start + n;
    while (p < stop && !*p) {
        p++;
    }
    return p - start;
}

void HAP_srp_session_key(uint8_t k[SRP_SESSION_KEY_BYTES], const uint8_t s[SRP_PREMASTER_SECRET_BYTES]) {
    size_t z = Count_Leading_Zeroes(s, SRP_PREMASTER_SECRET_BYTES);
    HAP_sha512(k, s + z, SRP_PREMASTER_SECRET_BYTES - z);
}

static void Xor(int* x, const int* a, const int* b, size_t n) {
    while (n-- > 0) {
        *x++ = *a++ ^ *b++;
    }
}

void HAP_srp_proof_m1(
        uint8_t m1[SRP_PROOF_BYTES],
        const uint8_t* user,
        size_t user_len,
        const uint8_t salt[SRP_SALT_BYTES],
        const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
        const uint8_t pub_b[SRP_PUBLIC_KEY_BYTES],
        const uint8_t k[SRP_SESSION_KEY_BYTES]) {
    uint8_t H_N[SHA512_BYTES];
    HAP_sha512(H_N, (const uint8_t*) N_3072, sizeof N_3072);
    uint8_t H_g[SHA512_BYTES];
    HAP_sha512(H_g, g_3072, sizeof g_3072);
    uint8_t H_Ng[SHA512_BYTES];
    Xor((int*) H_Ng, (const int*) H_N, (const int*) H_g, SHA512_BYTES / sizeof(int));
    uint8_t H_U[SHA512_BYTES];
    HAP_sha512(H_U, user, user_len);
    size_t z_A = Count_Leading_Zeroes(pub_a, SRP_PUBLIC_KEY_BYTES);
    size_t z_B = Count_Leading_Zeroes(pub_b, SRP_PUBLIC_KEY_BYTES);
    mbedtls_sha512_context ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, H_Ng, sizeof H_Ng);
    sha512_update(&ctx, H_U, sizeof H_U);
    sha512_update(&ctx, salt, SRP_SALT_BYTES);
    sha512_update(&ctx, pub_a + z_A, SRP_PUBLIC_KEY_BYTES - z_A);
    sha512_update(&ctx, pub_b + z_B, SRP_PUBLIC_KEY_BYTES - z_B);
    sha512_update(&ctx, k, SRP_SESSION_KEY_BYTES);
    sha512_final(&ctx, m1);
}

void HAP_srp_proof_m2(
        uint8_t m2[SRP_PROOF_BYTES],
        const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
        const uint8_t m1[SRP_PROOF_BYTES],
        const uint8_t k[SRP_SESSION_KEY_BYTES]) {
    mbedtls_sha512_context ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, pub_a, SRP_PUBLIC_KEY_BYTES);
    sha512_update(&ctx, m1, SRP_PROOF_BYTES);
    sha512_update(&ctx, k, SRP_SESSION_KEY_BYTES);
    sha512_final(&ctx, m2);
}

#endif

void HAP_sha1(uint8_t md[SHA1_BYTES], const uint8_t* data, size_t size) {
    mbedtls_sha1_context ctx;
    mbedtls_sha1_init(&ctx);
    int ret = mbedtls_sha1_starts_ret(&ctx);
    HAPAssert(ret == 0);
    ret = mbedtls_sha1_update_ret(&ctx, data, size);
    HAPAssert(ret == 0);
    ret = mbedtls_sha1_finish_ret(&ctx, md);
    HAPAssert(ret == 0);
    mbedtls_sha1_free(&ctx);
}

void HAP_sha256(uint8_t md[SHA256_BYTES], const uint8_t* data, size_t size) {
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    int ret = mbedtls_sha256_starts_ret(&ctx, 0);
    HAPAssert(ret == 0);
    ret = mbedtls_sha256_update_ret(&ctx, data, size);
    HAPAssert(ret == 0);
    ret = mbedtls_sha256_finish_ret(&ctx, md);
    HAPAssert(ret == 0);
    mbedtls_sha256_free(&ctx);
}

void HAP_sha512(uint8_t md[SHA512_BYTES], const uint8_t* data, size_t size) {
    mbedtls_sha512_context ctx;
    sha512_init(&ctx);
    sha512_update(&ctx, data, size);
    sha512_final(&ctx, md);
}

void HAP_hmac_sha1_aad(
        uint8_t r[HMAC_SHA1_BYTES],
        const uint8_t* key,
        size_t key_len,
        const uint8_t* in,
        size_t in_len,
        const uint8_t* aad,
        size_t aad_len) {
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    int ret = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), 1);
    HAPAssert(ret == 0);
    ret = mbedtls_md_hmac_starts(&ctx, key, key_len);
    HAPAssert(ret == 0);
    ret = mbedtls_md_hmac_update(&ctx, in, in_len);
    HAPAssert(ret == 0);
    ret = mbedtls_md_hmac_update(&ctx, aad, aad_len);
    HAPAssert(ret == 0);
    ret = mbedtls_md_hmac_finish(&ctx, r);
    HAPAssert(ret == 0);
    mbedtls_md_free(&ctx);
}

void HAP_hkdf_sha512(
        uint8_t* r,
        size_t r_len,
        const uint8_t* key,
        size_t key_len,
        const uint8_t* salt,
        size_t salt_len,
        const uint8_t* info,
        size_t info_len) {
    int ret = mbedtls_hkdf(
            mbedtls_md_info_from_type(MBEDTLS_MD_SHA512), salt, salt_len, key, key_len, info, info_len, r, r_len);
    HAPAssert(ret == 0);
}

void HAP_pbkdf2_hmac_sha1(
        uint8_t* key,
        size_t key_len,
        const uint8_t* password,
        size_t password_len,
        const uint8_t* salt,
        size_t salt_len,
        uint32_t count) {
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    int ret = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), 1);
    HAPAssert(ret == 0);
    ret = mbedtls_md_starts(&ctx);
    HAPAssert(ret == 0);
    ret = mbedtls_pkcs5_pbkdf2_hmac(&ctx, password, password_len, salt, salt_len, count, key_len, key);
    HAPAssert(ret == 0);
    mbedtls_md_free(&ctx);
}

typedef struct {
    mbedtls_chachapoly_context* ctx;
} mbedtls_chachapoly_context_Handle;

HAP_STATIC_ASSERT(
        sizeof(HAP_chacha20_poly1305_ctx) >= sizeof(mbedtls_chachapoly_context_Handle),
        HAP_chacha20_poly1305_ctx);

static void chacha20_poly1305_update(
        HAP_chacha20_poly1305_ctx* ctx,
        mbedtls_chachapoly_mode_t mode,
        uint8_t* output,
        const uint8_t* input,
        size_t input_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    mbedtls_chachapoly_context_Handle* handle = (mbedtls_chachapoly_context_Handle*) ctx;
    int ret;
    if (!handle->ctx) {
        handle->ctx = malloc(sizeof(mbedtls_chachapoly_context));
        mbedtls_chachapoly_init(handle->ctx);
        ret = mbedtls_chachapoly_setkey(handle->ctx, k);
        HAPAssert(ret == 0);
        if (n_len >= CHACHA20_POLY1305_NONCE_BYTES_MAX) {
            n_len = CHACHA20_POLY1305_NONCE_BYTES_MAX;
        }
        // pad nonce
        uint8_t nonce[CHACHA20_POLY1305_NONCE_BYTES_MAX];
        memset(nonce, 0, sizeof nonce);
        memcpy(nonce + sizeof nonce - n_len, n, n_len);
        ret = mbedtls_chachapoly_starts(handle->ctx, nonce, mode);
        HAPAssert(ret == 0);
    }
    if (input_len > 0) {
        ret = mbedtls_chachapoly_update(handle->ctx, input_len, input, output);
        HAPAssert(ret == 0);
    }
}

void chacha20_poly1305_update_aad(
        HAP_chacha20_poly1305_ctx* ctx,
        mbedtls_chachapoly_mode_t mode,
        const uint8_t* a,
        size_t a_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    chacha20_poly1305_update(ctx, mode, NULL, NULL, 0, n, n_len, k);
    mbedtls_chachapoly_context_Handle* handle = (mbedtls_chachapoly_context_Handle*) ctx;
    int ret = mbedtls_chachapoly_update_aad(handle->ctx, a, a_len);
    HAPAssert(ret == 0);
}

void chacha20_poly1305_final(HAP_chacha20_poly1305_ctx* ctx, uint8_t tag[CHACHA20_POLY1305_TAG_BYTES]) {
    mbedtls_chachapoly_context_Handle* handle = (mbedtls_chachapoly_context_Handle*) ctx;
    int ret = mbedtls_chachapoly_finish(handle->ctx, tag);
    HAPAssert(ret == 0);
    mbedtls_chachapoly_free(handle->ctx);
    free(handle->ctx);
    handle->ctx = NULL;
}

void HAP_chacha20_poly1305_init(
        HAP_chacha20_poly1305_ctx* ctx,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    mbedtls_chachapoly_context_Handle* handle = (mbedtls_chachapoly_context_Handle*) ctx;
    handle->ctx = NULL;
}

void HAP_chacha20_poly1305_update_enc(
        HAP_chacha20_poly1305_ctx* ctx,
        uint8_t* c,
        const uint8_t* m,
        size_t m_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    chacha20_poly1305_update(ctx, MBEDTLS_CHACHAPOLY_ENCRYPT, c, m, m_len, n, n_len, k);
}

void HAP_chacha20_poly1305_update_enc_aad(
        HAP_chacha20_poly1305_ctx* ctx,
        const uint8_t* a,
        size_t a_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    chacha20_poly1305_update_aad(ctx, MBEDTLS_CHACHAPOLY_ENCRYPT, a, a_len, n, n_len, k);
}

void HAP_chacha20_poly1305_final_enc(HAP_chacha20_poly1305_ctx* ctx, uint8_t tag[CHACHA20_POLY1305_TAG_BYTES]) {
    chacha20_poly1305_final(ctx, tag);
}

void HAP_chacha20_poly1305_update_dec(
        HAP_chacha20_poly1305_ctx* ctx,
        uint8_t* m,
        const uint8_t* c,
        size_t c_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    chacha20_poly1305_update(ctx, MBEDTLS_CHACHAPOLY_DECRYPT, m, c, c_len, n, n_len, k);
}

void HAP_chacha20_poly1305_update_dec_aad(
        HAP_chacha20_poly1305_ctx* ctx,
        const uint8_t* a,
        size_t a_len,
        const uint8_t* n,
        size_t n_len,
        const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]) {
    chacha20_poly1305_update_aad(ctx, MBEDTLS_CHACHAPOLY_DECRYPT, a, a_len, n, n_len, k);
}

int HAP_chacha20_poly1305_final_dec(HAP_chacha20_poly1305_ctx* ctx, const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES]) {
    uint8_t tag2[CHACHA20_POLY1305_TAG_BYTES];
    chacha20_poly1305_final(ctx, tag2);
    return HAP_constant_time_equal(tag, tag2, CHACHA20_POLY1305_TAG_BYTES) ? 0 : -1;
}

typedef struct {
    mbedtls_aes_context ctx;
    size_t nc_off;
    unsigned char nonce_counter[16];
    unsigned char stream_block[16];
} AES_CTR_CTX;

typedef struct {
    AES_CTR_CTX* ctx;
} AES_CTR_Handle;

HAP_STATIC_ASSERT(sizeof(HAP_aes_ctr_ctx) >= sizeof(AES_CTR_Handle), HAP_aes_ctr_ctx);

void HAP_aes_ctr_init(HAP_aes_ctr_ctx* ctx, const uint8_t* key, int size, const uint8_t iv[16]) {
    AES_CTR_Handle* handle = (AES_CTR_Handle*) ctx;
    HAPAssert(size == 16 || size == 32);
    handle->ctx = malloc(sizeof(AES_CTR_CTX));
    AES_CTR_CTX* ctr_ctx = handle->ctx;
    mbedtls_aes_context* mbedtls_ctx = &(ctr_ctx->ctx);
    mbedtls_aes_init(mbedtls_ctx);
    // always use an encryption key, for both encrypt and decrypt since this is CTR
    int ret = mbedtls_aes_setkey_enc(mbedtls_ctx, key, size * 8);
    HAPAssert(ret == 0);
    ctr_ctx->nc_off = 0;
    memcpy(ctr_ctx->nonce_counter, iv, 16);
    memset(ctr_ctx->stream_block, 0, sizeof ctr_ctx->stream_block);
}

void HAP_aes_ctr_encrypt(HAP_aes_ctr_ctx* ctx, uint8_t* ct, const uint8_t* pt, size_t pt_len) {
    AES_CTR_Handle* handle = (AES_CTR_Handle*) ctx;
    AES_CTR_CTX* ctr_ctx = handle->ctx;
    mbedtls_aes_context* mbedtls_ctx = &(ctr_ctx->ctx);
    int ret = mbedtls_aes_crypt_ctr(
            mbedtls_ctx, pt_len, &ctr_ctx->nc_off, ctr_ctx->nonce_counter, ctr_ctx->stream_block, pt, ct);
    HAPAssert(ret == 0);
}

void HAP_aes_ctr_decrypt(HAP_aes_ctr_ctx* ctx, uint8_t* pt, const uint8_t* ct, size_t ct_len) {
    HAP_aes_ctr_encrypt(ctx, pt, ct, ct_len);
}

void HAP_aes_ctr_done(HAP_aes_ctr_ctx* ctx) {
    AES_CTR_Handle* handle = (AES_CTR_Handle*) ctx;
    AES_CTR_CTX* ctr_ctx = handle->ctx;
    mbedtls_aes_context* mbedtls_ctx = &(ctr_ctx->ctx);
    mbedtls_aes_free(mbedtls_ctx);
    memset(ctr_ctx, 0, sizeof *ctr_ctx);
    free(ctr_ctx);
    handle->ctx = NULL;
}
