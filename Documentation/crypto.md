# Supported Cryptographic Libraries
* OpenSSL (1.1.1c or later)
* MbedTLS (2.18.0 or later)

Additional platforms can be supported by providing a custom PAL implementation.

## Design considerations

The PAL abstracts all relevant aspects of the underlying platform. Currently the PAL attempts to avoid
memory allocations. This is not necessarily a design goal we'll maintain going forward. The
PAL has 3 build types: Test, Debug, and Release. Test uses the Mock PAL and is used for unit
tests. Debug and Release use platform-specific backends.

### Configuration and weak symbols

Past versions of the ADK used platform-specific constructors to pass options to PAL
constructors. The current version of the ADK instead uses a consistent constructor
across all platforms. Our goal is for applications to be portable across platforms and
across BLE and IP.

## Crypto PAL

### Ed25519

EdDSA (Edwards-curve Digital Signature Algorithm) signature scheme using SHA-512 and Curve25519.

*MbedTLS does not natively support Ed25519. We ship a performant software-only implementation
along with our MbedTLS implementation but we strongly encourage silicon vendors to substitute
their own production quality implementation (or hardware accelerated implementation).*

To disable our implementation we recommend including the C file of our bindings in a new C file and
defining *HAVE_CUSTOM_ED25519*, which disables our implementation.

```
#define ED25519_PUBLIC_KEY_BYTES 32
#define ED25519_SECRET_KEY_BYTES 32
#define ED25519_BYTES 64

void HAP_ed25519_public_key(uint8_t pk[ED25519_PUBLIC_KEY_BYTES],
                            const uint8_t sk[ED25519_SECRET_KEY_BYTES]);
void HAP_ed25519_sign(uint8_t sig[ED25519_BYTES],
                      const uint8_t *m, size_t m_len,
                      const uint8_t sk[ED25519_SECRET_KEY_BYTES],
                      const uint8_t pk[ED25519_PUBLIC_KEY_BYTES]);
int HAP_ed25519_verify(const uint8_t sig[ED25519_BYTES],
                       const uint8_t *m, size_t m_len,
                       const uint8_t pk[ED25519_PUBLIC_KEY_BYTES]);
```

### X25519

X25519 is an elliptic curve Diffie-Hellman key exchange scheme using Curve25519.

```
#define X25519_SCALAR_BYTES 32
#define X25519_BYTES 32

void HAP_X25519_scalarmult_base(uint8_t r[X25519_BYTES],
                                const uint8_t n[X25519_SCALAR_BYTES]);
void HAP_X25519_scalarmult(uint8_t r[X25519_BYTES],
                           const uint8_t n[X25519_SCALAR_BYTES],
                           const uint8_t p[X25519_BYTES]);
```

### ChaCha20 Poly1305

Authenticated Encryption using ChaCha20 stream cipher and Poly1305 authenticator.

```
#define CHACHA20_POLY1305_KEY_BYTES 32
#define CHACHA20_POLY1305_NONCE_BYTES_MAX 12
#define CHACHA20_POLY1305_TAG_BYTES 16

void HAP_chacha20_poly1305_init(HAP_chacha20_poly1305_ctx *ctx,
                                const uint8_t *n, size_t n_len,
                                const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void HAP_chacha20_poly1305_update_enc(HAP_chacha20_poly1305_ctx *ctx,
                                      uint8_t *c,
                                      const uint8_t *m, size_t m_len,
                                      const uint8_t *n, size_t n_len,
                                      const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void HAP_chacha20_poly1305_update_enc_aad(HAP_chacha20_poly1305_ctx *ctx,
                                          const uint8_t *a, size_t a_len,
                                          const uint8_t *n, size_t n_len,
                                          const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void HAP_chacha20_poly1305_final_enc(HAP_chacha20_poly1305_ctx *ctx,
                                     uint8_t tag[CHACHA20_POLY1305_TAG_BYTES]);
void HAP_chacha20_poly1305_update_dec(HAP_chacha20_poly1305_ctx *ctx,
                                      uint8_t *m,
                                      const uint8_t *c, size_t c_len,
                                      const uint8_t *n, size_t n_len,
                                      const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
void HAP_chacha20_poly1305_update_dec_aad(HAP_chacha20_poly1305_ctx *ctx,
                                          const uint8_t *a, size_t a_len,
                                          const uint8_t *n, size_t n_len,
                                          const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
int HAP_chacha20_poly1305_final_dec(HAP_chacha20_poly1305_ctx *ctx,
                                    const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES]);
```

Note: *The implementation must support overlapping buffers (m and c).*

If HAVE_CUSTOM_SINGLE_SHOT_CHACHA20_POLY1305 is not set, we synthesize a single shot API from
the streaming API above. Otherwise the backend must provide the following API. This is
recommended for BLE-only crypto backends. *The streaming API is needed by IP accessories only.*

```
void HAP_chacha20_poly1305_encrypt_aad(uint8_t tag[CHACHA20_POLY1305_TAG_BYTES],
                                       uint8_t *c,
                                       const uint8_t *m, size_t m_len,
                                       const uint8_t *a, size_t a_len,
                                       const uint8_t *n, size_t n_len,
                                       const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);

int HAP_chacha20_poly1305_decrypt_aad(const uint8_t tag[CHACHA20_POLY1305_TAG_BYTES],
                                      uint8_t *m,
                                      const uint8_t *c, size_t c_len,
                                      const uint8_t *a, size_t a_len,
                                      const uint8_t *n, size_t n_len,
                                      const uint8_t k[CHACHA20_POLY1305_KEY_BYTES]);
```

Note: a_len might be NULL.

### SRP6a

Secure Remote Password protocol (SRP6a), an augmented password-authenticated key agreement (PAKE) protocol.

*Both OpenSSL and MbedTLS do not natively support SRP6a. We are shipping an SRP6a implementation with
our code that uses the BIGNUM support in OpenSSL and MbedTLS respectively. We strongly recommend
substituting an optimized implementation for MCU-class silicon.*

To replace our implementation we recommend including the C file of our bindings in a new C file and
defining *CUSTOM_SRP*, which disables our implementation.

```
#define SRP_PRIME_BYTES 384
#define SRP_SALT_BYTES 16
#define SRP_VERIFIER_BYTES 384
#define SRP_SECRET_KEY_BYTES 32
#define SRP_PUBLIC_KEY_BYTES 384
#define SRP_SCRAMBLING_PARAMETER_BYTES 64
#define SRP_PREMASTER_SECRET_BYTES 384
#define SRP_SESSION_KEY_BYTES 64
#define SRP_PROOF_BYTES 64

void HAP_srp_verifier(uint8_t v[SRP_VERIFIER_BYTES],
                      const uint8_t salt[SRP_SALT_BYTES],
                      const uint8_t *user, size_t user_len,
                      const uint8_t *pass, size_t pass_len);
void HAP_srp_public_key(uint8_t pub_b[SRP_PUBLIC_KEY_BYTES],
                        const uint8_t priv_b[SRP_SECRET_KEY_BYTES],
                        const uint8_t v[SRP_VERIFIER_BYTES]);
void HAP_srp_scrambling_parameter(uint8_t u[SRP_SCRAMBLING_PARAMETER_BYTES],
                                  const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
                                  const uint8_t pub_b[SRP_PUBLIC_KEY_BYTES]);
int HAP_srp_premaster_secret(uint8_t s[SRP_PREMASTER_SECRET_BYTES],
                             const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
                             const uint8_t priv_b[SRP_SECRET_KEY_BYTES],
                             const uint8_t u[SRP_SCRAMBLING_PARAMETER_BYTES],
                             const uint8_t v[SRP_VERIFIER_BYTES]);
void HAP_srp_session_key(uint8_t k[SRP_SESSION_KEY_BYTES],
                         const uint8_t s[SRP_PREMASTER_SECRET_BYTES]);
void HAP_srp_proof_m1(uint8_t m1[SRP_PROOF_BYTES],
                      const uint8_t *user, size_t user_len,
                      const uint8_t salt[SRP_SALT_BYTES],
                      const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
                      const uint8_t pub_b[SRP_PUBLIC_KEY_BYTES],
                      const uint8_t k[SRP_SESSION_KEY_BYTES]);
void HAP_srp_proof_m2(uint8_t m2[SRP_PROOF_BYTES],
                      const uint8_t pub_a[SRP_PUBLIC_KEY_BYTES],
                      const uint8_t m1[SRP_PROOF_BYTES],
                      const uint8_t k[SRP_SESSION_KEY_BYTES]);
```

### SHA

Secure Hash Algorithms (SHA1, SHA-256, and SHA-512)

```
#define SHA1_BYTES 20

void HAP_sha1(uint8_t md[SHA1_BYTES], const uint8_t *data, size_t size);

#define SHA256_BYTES 32

void HAP_sha256(uint8_t md[SHA256_BYTES], const uint8_t *data, size_t size);

#define SHA512_BYTES 64

void HAP_sha512(uint8_t md[SHA512_BYTES], const uint8_t *data, size_t size);
```

### HMAC SHA1

Hash-based message authentication code using SHA1. *Only needed for IP Accessories.*

```
#define HMAC_SHA1_BYTES SHA1_BYTES

void HAP_hmac_sha1_aad(uint8_t r[HMAC_SHA1_BYTES],
                       const uint8_t* key, size_t key_len,
                       const uint8_t* in, size_t in_len,
                       const uint8_t* aad, size_t aad_len);
```

### HKDF-SHA512

HMAC-based key derivation function using SHA-512.

```
void HAP_hkdf_sha512(uint8_t* r, size_t r_len,
                     const uint8_t* key, size_t key_len,
                     const uint8_t* salt, size_t salt_len,
                     const uint8_t* info, size_t info_len);
```

Brute-force resistant password-based key derivation function.

### PBKDF2-SHA1

```
void HAP_pbkdf2_hmac_sha1(uint8_t *key, size_t key_len,
                          const uint8_t *password, size_t password_len,
                          const uint8_t *salt, size_t salt_len,
                          uint32_t count);
```

### AES-CTR

AES block cipher in CTR mode. *Only needed for IP Accessories.*

```
void HAP_aes_ctr_init(HAP_aes_ctr_ctx *ctx, const uint8_t *key, int size, const uint8_t iv[16]);
void HAP_aes_ctr_encrypt(HAP_aes_ctr_ctx *ctx, uint8_t* ct, const uint8_t* pt, size_t pt_len);
void HAP_aes_ctr_decrypt(HAP_aes_ctr_ctx *ctx, uint8_t* pt, const uint8_t* ct, size_t ct_len);
void HAP_aes_ctr_done(HAP_aes_ctr_ctx *ctx);
```
