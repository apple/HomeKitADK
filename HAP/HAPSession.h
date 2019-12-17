// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_SESSION_H
#define HAP_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Session key.
 */
typedef struct {
    /** Value. */
    uint8_t bytes[CHACHA20_POLY1305_KEY_BYTES];
} HAPSessionKey;
HAP_STATIC_ASSERT(sizeof(HAPSessionKey) == CHACHA20_POLY1305_KEY_BYTES, HAPSessionKey);

/**
 * HAP session channel state.
 */
typedef struct {
    HAPSessionKey key; /**< Encryption key. */
    uint64_t nonce;    /**< Nonce. */
} HAPSessionChannelState;

/**
 * Bluetooth LE specific parameters. Part of #HAPSession structure.
 */
typedef struct {
    /**@cond */
    HAPAccessoryServerRef* server; /**< Accessory server. */
    HAPSessionRef* session;        /**< Session. */

    bool isTerminal;               /**< True if LE link must be disconnected. No more requests are accepted. */
    HAPPlatformTimerRef linkTimer; /**< On expiry, the LE link is disconnected. */
    HAPTime linkTimerDeadline;     /**< Timeout of link timer, if timer is active. */

    /**
     * On expiry, the current Pairing procedure times out.
     */
    HAPPlatformTimerRef pairingProcedureTimer;

    /**
     * Whether or not it is safe to disconnect.
     *
     * - After a BLE response packet has been sent, it may take a certain time until the packet is fully transmitted.
     *   If a disconnect is requested before that happens, certain BLE stacks may drop the packet.
     *   Therefore, a timer is used to delay pending disconnects until we assume that the packet has been sent.
     */
    bool isSafeToDisconnect;

    /**
     * On expiry, it is safe to disconnect.
     */
    HAPPlatformTimerRef safeToDisconnectTimer;
    /**@endcond */
} HAPBLESession;

/**
 * HAP session.
 */
typedef struct {
    /**@cond */
    HAPAccessoryServerRef* server;

    /**
     * HAP session state.
     */
    struct {
        /** Whether the security session is active. */
        bool active : 1;

        /** Whether the security session originated from a transient Pair Setup procedure (Software Authentication). */
        bool isTransient : 1;

        /**
         * Key-value store key of the pairing, if applicable.
         *
         * - For sessions from a transient Pair Setup procedure (Software Authentication), this is a value < 0.
         */
        int pairingID;

        /**
         * Shared secret, if applicable.
         *
         * - This is used to derive the BLE Broadcast Encryption Key.
         *
         * - For sessions from a transient Pair Setup procedure (Software Authentication), this is uninitialized.
         */
        uint8_t cv_KEY[X25519_BYTES];

        /** Accessory to controller state. */
        struct {
            /** Control channel encryption. */
            HAPSessionChannelState controlChannel;
        } accessoryToController;

        /** Controller to accessory state. */
        struct {
            /** Control channel encryption. */
            HAPSessionChannelState controlChannel;
        } controllerToAccessory;
    } hap;

    struct {
        /**
         * Pair Setup procedure state.
         */
        struct {
            uint8_t state;  /**< State. */
            uint8_t method; /**< Method. */
            uint8_t error;  /**< Error code. */
        } pairSetup;

        /**
         * Pair Verify procedure state.
         */
        struct {
            uint8_t state;  /**< State. */
            uint8_t method; /**< Method. */
            uint8_t error;  /**< Error code. */

            uint8_t SessionKey[CHACHA20_POLY1305_KEY_BYTES]; // Session Key for the Pair Verify procedure.
            uint8_t cv_PK[X25519_BYTES];                     // PK
            uint8_t cv_SK[X25519_SCALAR_BYTES];              // SK
            uint8_t cv_KEY[X25519_BYTES];                    // Key (SK, CTRL PK)
            int pairingID;
            uint8_t Controller_cv_PK[X25519_BYTES]; // CTRL PK
        } pairVerify;

        /**
         * Pairings state.
         */
        struct {
            uint8_t state;  /**< State. */
            uint8_t method; /**< Method. */
            uint8_t error;  /**< Error code. */

            // Remove pairing.
            HAPPairingID removedPairingID;
            size_t removedPairingIDLength;
        } pairings;
    } state;

    /**
     * Type of the underlying transport.
     */
    HAPTransportType transportType;

    /**
     * Transport-specific parameters, depending on #transport_type.
     */
    union {
        HAPBLESession ble; /**< Bluetooth LE specific parameters. */
    } _;

    /**@endcond */
} HAPSession;
HAP_STATIC_ASSERT(sizeof(HAPSessionRef) >= sizeof(HAPSession), HAPSession);

/**
 * Pairing procedure.
 */
HAP_ENUM_BEGIN(uint8_t, HAPPairingProcedureType) { /**
                                                    * Pair Verify.
                                                    */
                                                   kHAPPairingProcedureType_PairVerify,

                                                   /**
                                                    * Pairing Pairings.
                                                    */
                                                   kHAPPairingProcedureType_PairingPairings
} HAP_ENUM_END(uint8_t, HAPPairingProcedureType);

/**
 * Initializes a session.
 *
 * - The session must be destroyed using #hm_session_deinit once it is no
 *   longer needed to ensure that the accessory state is cleaned up.
 *
 * - While the session is in use, it must be retained in the same memory location.
 *
 * @param      server               Accessory server that the session belongs to.
 * @param[out] session              Session to initialize.
 * @param      transportType        Transport type.
 */
void HAPSessionCreate(HAPAccessoryServerRef* server, HAPSessionRef* session, HAPTransportType transportType);

/**
 * Destroys a session, cleaning up state in the accessory server.
 *
 * @param      server               Accessory server that the session belongs to.
 * @param      session              Session to destroy.
 */
void HAPSessionRelease(HAPAccessoryServerRef* server, HAPSessionRef* session);

/**
 * Invalidates a session so that all future requests are rejected until the session is destroyed.
 *
 * - Multiple invocations are okay and do nothing.
 *
 * @param      server               Accessory server that the session belongs to.
 * @param      session              Session to destroy.
 * @param      terminateLink        Whether or not the underlying connection should also be terminated.
 */
void HAPSessionInvalidate(HAPAccessoryServerRef* server, HAPSessionRef* session, bool terminateLink);

/**
 * Returns whether a secured HAP session has been established.
 *
 * @param      session              Session to query.
 *
 * @return true                     If a secured HAP session has been established.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPSessionIsSecured(const HAPSessionRef* session);

/**
 * Returns whether a secured HAP session is transient (Software Authentication).
 *
 * @param      session              Session to query.
 *
 * @return true                     If a secured HAP session is transient.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPSessionIsTransient(const HAPSessionRef* session);

/**
 * Returns whether the controller of a HAP session has administrator privileges.
 *
 * @param      session              Session to query.
 *
 * @return true                     If the controller of the HAP session has administrator privileges.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPSessionControllerIsAdmin(const HAPSessionRef* session);

/**
 * Processes a Pair Setup write request.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the request has been received.
 * @param      requestReader        TLV reader for parsing the value.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If request reader does not have enough free memory.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairSetupWrite(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPTLVReaderRef* requestReader);

/**
 * Processes a Pair Setup read request.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If communication with Apple Authentication Coprocessor failed.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairSetupRead(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPTLVWriterRef* responseWriter);

/**
 * Processes a Pair Verify write request.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the request has been received.
 * @param      requestReader        TLV reader for parsing the value.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If request reader does not have enough free memory.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairVerifyWrite(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPTLVReaderRef* requestReader);

/**
 * Processes a Pair Verify read request.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairVerifyRead(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPTLVWriterRef* responseWriter);

/**
 * Processes a Pairings write request.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the request has been received.
 * @param      requestReader        TLV reader for parsing the value.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairingsWrite(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPTLVReaderRef* requestReader);

/**
 * Processes a Pairings read request.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairingsRead(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPTLVWriterRef* responseWriter);

/**
 * Encrypt a control message to be sent over a HomeKit session.
 *
 * The length of the encrypted message is `<plaintext message length> + CHACHA20_POLY1305_TAG_BYTES` bytes.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the message will be sent.
 * @param[out] encryptedBytes       Encrypted message.
 * @param      plaintextBytes       Plaintext message.
 * @param      numPlaintextBytes    Plaintext message length.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the session is not encrypted.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionEncryptControlMessage(
        const HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        void* encryptedBytes,
        const void* plaintextBytes,
        size_t numPlaintextBytes);

/**
 * Encrypt a control message with additional authenticated data to be sent over a HomeKit session.
 *
 * The length of the encrypted message is `<plaintext message length> + CHACHA20_POLY1305_TAG_BYTES` bytes.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the message will be sent.
 * @param[out] encryptedBytes       Encrypted message.
 * @param      plaintextBytes       Plaintext message.
 * @param      numPlaintextBytes    Plaintext message length.
 * @param      aadBytes             Additional authenticated data.
 * @param      numAADBytes          Additional authenticated data length.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the session is not encrypted.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionEncryptControlMessageWithAAD(
        const HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        void* encryptedBytes,
        const void* plaintextBytes,
        size_t numPlaintextBytes,
        const void* aadBytes,
        size_t numAADBytes);

/**
 * Decrypts a control message received over a HomeKit session.
 *
 * The length of the decrypted message is `<encrypted message length> - CHACHA20_POLY1305_TAG_BYTES` bytes.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the message has been received.
 * @param[out] plaintextBytes       Plaintext message.
 * @param      encryptedBytes       Encrypted message.
 * @param      numEncryptedBytes    Encrypted message length.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the session is not encrypted.
 * @return kHAPError_InvalidData    If the controller sent a malformed request, or decryption failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionDecryptControlMessage(
        const HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        void* plaintextBytes,
        const void* encryptedBytes,
        size_t numEncryptedBytes);

/**
 * Decrypts a control message with additional authenticated data received over a HomeKit session.
 *
 * The length of the decrypted message is `<encrypted message length> - CHACHA20_POLY1305_TAG_BYTES` bytes.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the message has been received.
 * @param[out] plaintextBytes       Plaintext message.
 * @param      encryptedBytes       Encrypted message.
 * @param      numEncryptedBytes    Encrypted message length.
 * @param      aadBytes             Additional authenticated data.
 * @param      numAADBytes          Additional authenticated data length.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the session is not encrypted.
 * @return kHAPError_InvalidData    If the controller sent a malformed request, or decryption failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPSessionDecryptControlMessageWithAAD(
        const HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        void* plaintextBytes,
        const void* encryptedBytes,
        size_t numEncryptedBytes,
        const void* aadBytes,
        size_t numAADBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
