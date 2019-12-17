// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PAIRING_H
#define HAP_PAIRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Pairing identifier.
 *
 * iOS and HAT based controllers have been observed to use 128-bit upper case UUIDs as their identifier.
 * Format: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
 */
typedef struct {
    uint8_t bytes[36]; /**< Value. */
} HAPPairingID;
HAP_STATIC_ASSERT(sizeof(HAPPairingID) == 36, HAPPairingID);

/**
 * Pairing public key.
 */
typedef struct {
    /** Value. */
    uint8_t value[ED25519_PUBLIC_KEY_BYTES];
} HAPPairingPublicKey;
HAP_STATIC_ASSERT(sizeof(HAPPairingPublicKey) == 32, HAPPairingPublicKey);

/**
 * Pairing.
 */
typedef struct {
    HAPPairingID identifier;       /**< Pairing identifier. */
    uint8_t numIdentifierBytes;    /**< Length of the pairing identifier. */
    HAPPairingPublicKey publicKey; /**< Public key. */
    uint8_t permissions;           /**< Permission flags. */
} HAPPairing;

/**
 * Methods.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 5-14 Methods
 */
HAP_ENUM_BEGIN(uint8_t, HAPPairingMethod) { /** Pair Setup. */
                                            kHAPPairingMethod_PairSetup = 0x00,

                                            /** Pair Setup with Auth. */
                                            kHAPPairingMethod_PairSetupWithAuth = 0x01,

                                            /** Pair Verify. */
                                            kHAPPairingMethod_PairVerify = 0x02,

                                            /** Add Pairing. */
                                            kHAPPairingMethod_AddPairing = 0x03,

                                            /** Remove Pairing. */
                                            kHAPPairingMethod_RemovePairing = 0x04,

                                            /** List Pairings. */
                                            kHAPPairingMethod_ListPairings = 0x05,

                                            /**
                                             * Pair Resume.
                                             *
                                             * @see HomeKit Accessory Protocol Specification R14
                                             *      Table 7-38 Defines Description
                                             */
                                            kHAPPairingMethod_PairResume = 0x06
} HAP_ENUM_END(uint8_t, HAPPairingMethod);

/**
 * Error Codes.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 5-16 Error Codes
 */
HAP_ENUM_BEGIN(uint8_t, HAPPairingError) {
    /** Generic error to handle unexpected errors. */
    kHAPPairingError_Unknown = 0x01,

    /** Setup code or signature verification failed. */
    kHAPPairingError_Authentication = 0x02,

    /**
     * Client must look at the retry delay TLV item and wait that many seconds before retrying.
     *
     * @remark Obsolete since R3.
     */
    kHAPPairingError_Backoff = 0x03,

    /** Server cannot accept any more pairings. */
    kHAPPairingError_MaxPeers = 0x04,

    /** Server reached its maximum number of authentication attempts. */
    kHAPPairingError_MaxTries = 0x05,

    /** Server pairing method is unavailable. */
    kHAPPairingError_Unavailable = 0x06,

    /** Server is busy and cannot accept a pairing request at this time. */
    kHAPPairingError_Busy = 0x07,
} HAP_ENUM_END(uint8_t, HAPPairingError);

/**
 * TLV Values.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 5-17 TLV Values
 */
HAP_ENUM_BEGIN(uint8_t, HAPPairingTLVType) { /**
                                              * Method to use for pairing.
                                              * integer.
                                              */
                                             kHAPPairingTLVType_Method = 0x00,

                                             /**
                                              * Identifier for authentication.
                                              * UTF-8.
                                              */
                                             kHAPPairingTLVType_Identifier = 0x01,

                                             /**
                                              * 16+ bytes of random salt.
                                              * bytes.
                                              */
                                             kHAPPairingTLVType_Salt = 0x02,

                                             /**
                                              * Curve25519, SRP public key, or signed Ed25519 key.
                                              * bytes.
                                              */
                                             kHAPPairingTLVType_PublicKey = 0x03,

                                             /**
                                              * Ed25519 or SRP proof.
                                              * bytes.
                                              */
                                             kHAPPairingTLVType_Proof = 0x04,

                                             /**
                                              * Encrypted data with auth tag at end.
                                              * bytes.
                                              */
                                             kHAPPairingTLVType_EncryptedData = 0x05,

                                             /**
                                              * State of the pairing process. 1=M1, 2=M2, etc.
                                              * integer.
                                              */
                                             kHAPPairingTLVType_State = 0x06,

                                             /**
                                              * Error code. Must only be present if error code is not 0.
                                              * integer.
                                              */
                                             kHAPPairingTLVType_Error = 0x07,

                                             /**
                                              * Seconds to delay until retrying a setup code.
                                              * integer.
                                              *
                                              * @remark Obsolete since R3.
                                              */
                                             kHAPPairingTLVType_RetryDelay = 0x08,

                                             /**
                                              * X.509 Certificate.
                                              * bytes.
                                              */
                                             kHAPPairingTLVType_Certificate = 0x09,

                                             /**
                                              * Ed25519 or Apple Authentication Coprocessor signature.
                                              * bytes.
                                              */
                                             kHAPPairingTLVType_Signature = 0x0A,

                                             /**
                                              * Bit value describing permissions of the controller being added.
                                              * None (0x00): Regular user
                                              * Bit 1 (0x01): Admin that is able to add and remove pairings against the
                                              * accessory. integer.
                                              */
                                             kHAPPairingTLVType_Permissions = 0x0B,

                                             /**
                                              * Non-last fragment of data. If length is 0, it's an ACK.
                                              * bytes.
                                              *
                                              * @remark Obsolete since R7.
                                              *
                                              * @see HomeKit Accessory Protocol Specification R6
                                              *      Section 3.8 Fragmentation and Reassembly
                                              */
                                             kHAPPairingTLVType_FragmentData = 0x0C,

                                             /**
                                              * Last fragment of data.
                                              * bytes.
                                              *
                                              * @remark Obsolete since R7.
                                              *
                                              * @see HomeKit Accessory Protocol Specification R6
                                              *      Section 3.8 Fragmentation and Reassembly
                                              */
                                             kHAPPairingTLVType_FragmentLast = 0x0D,

                                             /**
                                              * Identifier to resume a session.
                                              *
                                              * @see HomeKit Accessory Protocol Specification R14
                                              *      Table 7-38 Defines Description
                                              */
                                             kHAPPairingTLVType_SessionID = 0x0E,

                                             /**
                                              * Pairing Type Flags (32 bit unsigned integer).
                                              * integer.
                                              */
                                             kHAPPairingTLVType_Flags = 0x13,

                                             /**
                                              * Zero-length TLV that separates different TLVs in a list.
                                              * null.
                                              */
                                             kHAPPairingTLVType_Separator = 0xFF
} HAP_ENUM_END(uint8_t, HAPPairingTLVType);

/**
 * Pairing Type Flags.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 5-18 Pairing Type Flags
 */
HAP_OPTIONS_BEGIN(uint32_t, HAPPairingFlag) {
    /**
     * Transient Pair Setup.
     *
     * Pair Setup M1 - M4 without exchanging public keys.
     */
    kHAPPairingFlag_Transient = 1U << 4U,

    /**
     * Split Pair Setup.
     *
     * When set with kHAPPairingFlag_Transient save the SRP verifier used in this session,
     * and when only kHAPPairingFlag_Split is set, use the saved SRP verifier from previous session.
     */
    kHAPPairingFlag_Split = 1U << 24U,
} HAP_OPTIONS_END(uint32_t, HAPPairingFlag);

/**
 * Reads a flags value up to UInt32 in size from a Pairing protocol TLV
 * containing its corresponding little-endian representation.
 *
 * - Excess bytes are ignored.
 *
 * @param      tlv                  TLV containing numeric value.
 *
 * @return Numeric value.
 */
uint32_t HAPPairingReadFlags(const HAPTLV* tlv);

/**
 * Counts the number of bytes of a numeric value when serialized to a Pairing protocol TLV.
 *
 * @param      value                Numeric value.
 *
 * @return Number of bytes when serializing value to a Pairing protocol TLV.
 */
size_t HAPPairingGetNumBytes(uint32_t value);

/**
 * Looks for a pairing.
 *
 * @param      keyValueStore        Key-value store.
 * @param[in,out] pairing           On input, pairing identifier must be set. On output, if found, pairing is stored.
 * @param[out] key                  Key-value store key, if found.
 * @param[out] found                True if pairing has been found. False otherwise.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPairingFind(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPairing* pairing,
        HAPPlatformKeyValueStoreKey* key,
        bool* found);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
