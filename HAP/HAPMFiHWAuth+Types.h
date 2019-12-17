// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_MFI_HW_AUTH_TYPES_H
#define HAP_MFI_HW_AUTH_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Coprocessor register map.
 *
 * @see Accessory Interface Specification R30
 *      Section 64.5.7 Registers
 *
 * @see Accessory Interface Specification R29
 *      Section 69.8.1 Register Addresses
 */
HAP_ENUM_BEGIN(uint8_t, HAPMFiHWAuthRegister) {
    /**
     * Device Version.
     *
     * - Block: 0
     * - Length: 1 byte
     * - Power-Up Value: See HAPMFiHWAuthDeviceVersion
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_DeviceVersion = 0x00,

    /**
     * Authentication Revision / Firmware Version (2.0C).
     *
     *  - Block: 0
     *  - Length: 1 byte
     *  - Power-Up Value: 0x00 / 0x01 (2.0C)
     *  - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AuthenticationRevision = 0x01,

    /**
     * Authentication Protocol Major Version.
     *
     * - Block: 0
     * - Length: 1 byte
     * - Power-Up Value: 0x03 / 0x02 (2.0C)
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AuthenticationProtocolMajorVersion = 0x02,

    /**
     * Authentication Protocol Minor Version.
     *
     * - Block: 0
     * - Length: 1 byte
     * - Power-Up Value: 0x00
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AuthenticationProtocolMinorVersion = 0x03,

    /**
     * Device ID.
     *
     * - Block: 0
     * - Length: 4 bytes
     * - Power-Up Value: 0x00000300 / 0x00000200 (2.0C)
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_DeviceID = 0x04,

    /**
     * Error Code.
     *
     * - Block: 0
     * - Length: 1 byte
     * - Power-Up Value: 0x00
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_ErrorCode = 0x05,

    /**
     * Authentication Control and Status.
     *
     * - Block: 1
     * - Length: 1 byte
     * - Power-Up Value: 0x00
     * - Access: Read/write
     */
    kHAPMFiHWAuthRegister_AuthenticationControlAndStatus = 0x10,

    /**
     * Challenge Response Data Length.
     *
     * - Block: 1
     * - Length: 2 bytes
     * - Power-Up Value: 0 / 128 (2.0C)
     * - Access: Read-only / Read/write (2.0C)
     */
    kHAPMFiHWAuthRegister_ChallengeResponseDataLength = 0x11,

    /**
     * Challenge Response Data.
     *
     * - Block: 1
     * - Length: 64 bytes / 128 bytes (2.0C)
     * - Power-Up Value: Undefined
     * - Access: Read-only / Read/write (2.0C)
     */
    kHAPMFiHWAuthRegister_ChallengeResponseData = 0x12,

    /**
     * Challenge Data Length.
     *
     * - Block: 2
     * - Length: 2 bytes
     * - Power-Up Value: 0 / 20 (2.0C)
     * - Access: Read-only / Read/write (2.0C)
     */
    kHAPMFiHWAuthRegister_ChallengeDataLength = 0x20,

    /**
     * Challenge Data.
     *
     * - Block: 2
     * - Length: 32 bytes / 128 bytes (2.0C)
     * - Power-Up Value: Undefined
     * - Access: Read/write
     */
    kHAPMFiHWAuthRegister_ChallengeData = 0x21,

    /**
     * Accessory Certificate Data Length.
     *
     * - Block: 3
     * - Length: 2 bytes
     * - Power-Up Value: 607-609 / <= 1280 (2.0C)
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataLength = 0x30,

    /**
     * Accessory Certificate Data (Part 1).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart1 = 0x31,

    /**
     * Accessory Certificate Data (Part 2).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart2 = 0x32,

    /**
     * Accessory Certificate Data (Part 3).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart3 = 0x33,

    /**
     * Accessory Certificate Data (Part 4).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart4 = 0x34,

    /**
     * Accessory Certificate Data (Part 5).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart5 = 0x35,

    /**
     * Accessory Certificate Data (Part 6) (2.0C).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart6 = 0x36,

    /**
     * Accessory Certificate Data (Part 7) (2.0C).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart7 = 0x37,

    /**
     * Accessory Certificate Data (Part 8) (2.0C).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart8 = 0x38,

    /**
     * Accessory Certificate Data (Part 9) (2.0C).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart9 = 0x39,

    /**
     * Accessory Certificate Data (Part 10) (2.0C).
     *
     * - Block: 3
     * - Length: 128 bytes
     * - Power-Up Value: Certificate
     * - Access: Read-only
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateDataPart10 = 0x3A,

    /**
     * Self-Test Status / Self-Test Control and Status (2.0C).
     *
     * - Block: 4
     * - Length: 1 byte
     * - Power-Up Value: 0x00
     * - Access: Read-only / Read/write (2.0C)
     */
    kHAPMFiHWAuthRegister_SelfTestStatus = 0x40,

    /**
     * System Event Counter (SEC) (2.0C).
     *
     * - Block: 4
     * - Length: 1 byte
     * - Power-Up Value: Undefined
     * - Access: Read-only
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_SystemEventCounter = 0x4D,

    /**
     * Device Certificate Serial Number / Accessory Certificate Serial Number (2.0C).
     *
     * - Block: 4
     * - Length: 32 bytes / 31 bytes (2.0C)
     * - Power-Up Value: Certificate / Null-terminated string (2.0C)
     * - Access: Read-only
     */
    kHAPMFiHWAuthRegister_AccessoryCertificateSerialNumber = 0x4E,

    /**
     * Apple Device Certificate Data Length (2.0C).
     *
     * - Block: 5
     * - Length: 2 bytes
     * - Power-Up Value: 0x0000
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataLength = 0x50,

    /**
     * Apple Device Certificate Data (Part 1) (2.0C).
     *
     * - Block: 5
     * - Length: 128 bytes
     * - Power-Up Value: Undefined
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataPart1 = 0x51,

    /**
     * Apple Device Certificate Data (Part 2) (2.0C).
     *
     * - Block: 5
     * - Length: 128 bytes
     * - Power-Up Value: Undefined
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataPart2 = 0x52,

    /**
     * Apple Device Certificate Data (Part 3) (2.0C).
     *
     * - Block: 5
     * - Length: 128 bytes
     * - Power-Up Value: Undefined
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataPart3 = 0x53,

    /**
     * Apple Device Certificate Data (Part 4) (2.0C).
     *
     * - Block: 5
     * - Length: 128 bytes
     * - Power-Up Value: Undefined
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataPart4 = 0x54,

    /**
     * Apple Device Certificate Data (Part 5) (2.0C).
     *
     * - Block: 5
     * - Length: 128 bytes
     * - Power-Up Value: Undefined
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataPart5 = 0x55,

    /**
     * Apple Device Certificate Data (Part 6) (2.0C).
     *
     * - Block: 5
     * - Length: 128 bytes
     * - Power-Up Value: Undefined
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataPart6 = 0x56,

    /**
     * Apple Device Certificate Data (Part 7) (2.0C).
     *
     * - Block: 5
     * - Length: 128 bytes
     * - Power-Up Value: Undefined
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataPart7 = 0x57,

    /**
     * Apple Device Certificate Data (Part 8) (2.0C).
     *
     * - Block: 5
     * - Length: 128 bytes
     * - Power-Up Value: Undefined
     * - Access: Read/write
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthRegister_AppleDeviceCertificateDataPart8 = 0x58,

    /**
     * Sleep.
     *
     * - Block: 1
     * - Length: 1 byte
     * - Power-Up Value: Undefined
     * - Access: Write-only
     */
    kHAPMFiHWAuthRegister_Sleep = 0x60
} HAP_ENUM_END(uint8_t, HAPMFiHWAuthRegister);

/**
 * Coprocessor device versions.
 */
HAP_ENUM_BEGIN(uint8_t, HAPMFiHWAuthDeviceVersion) { /**
                                                      * 2.0C.
                                                      *
                                                      * @see Accessory Interface Specification R29
                                                      *      Section 69.8.1 Register Addresses
                                                      *
                                                      * @remark Obsolete since R30.
                                                      */
                                                     kHAPMFiHWAuthDeviceVersion_2_0C = 0x05,

                                                     /**
                                                      * 3.0.
                                                      *
                                                      * @see Accessory Interface Specification 30
                                                      *      Section 64.5.7.1 Device Version
                                                      */
                                                     kHAPMFiHWAuthDeviceVersion_3_0 = 0x07
} HAP_ENUM_END(uint8_t, HAPMFiHWAuthDeviceVersion);

/**
 * Coprocessor error codes.
 *
 * @see Accessory Interface Specification R30
 *      Section 64.5.7.6 Error Code
 */
HAP_ENUM_BEGIN(uint8_t, HAPMFiHWAuthError) {
    /** No error. */
    kHAPMFiHWAuthError_NoError = 0x00,

    /**
     * Invalid register for read (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthError_InvalidRegisterForRead = 0x01,

    /**
     * Invalid register specified or register is read-only / Invalid register for write (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     */
    kHAPMFiHWAuthError_InvalidRegister = 0x02,

    /**
     * Invalid challenge response length (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthError_InvalidChallengeResponseLength = 0x03,

    /**
     * Invalid challenge length (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthError_InvalidChallengeLength = 0x04,

    /**
     * Sequence error, command out of sequence / Invalid certificate length (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     */
    kHAPMFiHWAuthError_InvalidCommandSequence = 0x05,

    /**
     * Internal process error during challenge response generation.
     */
    kHAPMFiHWAuthError_InternalChallengeResponseGenerationError = 0x06,

    /**
     * Internal process error during challenge generation (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthError_InternalChallengeGenerationError = 0x07,

    /**
     * Internal process error during challenge response verification (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthError_InternalChallengeResponseVerificationError = 0x08,

    /**
     * Internal process error during certificate validation (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthError_InternalCertificateValidationError = 0x09,

    /**
     * Invalid process control (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthError_InvalidProcessControl = 0x0A,

    /**
     * Process control out of sequence (2.0C).
     *
     * @see Accessory Interface Specification R29
     *      Section 69.8.2.5 Error Code
     *
     * @remark Obsolete since R30.
     */
    kHAPMFiHWAuthError_InvalidProcessControlSequence = 0x0B
} HAP_ENUM_END(uint8_t, HAPMFiHWAuthError);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
