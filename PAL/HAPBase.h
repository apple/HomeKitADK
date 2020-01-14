// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BASE_H
#define HAP_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

// C library header files that are also available in freestanding environments (-ffreestanding).
#include <float.h>
#include <iso646.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HAP_HAS_EXTENSION(x) (x == 1)

#include "HAPBase+CompilerAbstraction.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Gets the number of elements in an array.
 *
 * - This only works before the array is passed to other C functions!
 *
 * @param      array                Array.
 *
 * @return Number of elements in the array.
 */
#define HAPArrayCount(array) ((size_t)(sizeof(array) / sizeof((array)[0])))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns the lesser of two comparable values.
 *
 * @param      x                    A value to compare.
 * @param      y                    Another value to compare.
 *
 * @return The lesser of @p x and @p y. If @p x is equal to @p y, returns @p x.
 */
#define HAPMin(x, y) (((x) <= (y)) ? (x) : (y))

/**
 * Returns the greater of two comparable values.
 *
 * @param      x                    A value to compare.
 * @param      y                    Another value to compare.
 *
 * @return The greater of @p x and @p y. If @p x is equal to @p y, returns @p y.
 */
#define HAPMax(x, y) (((x) <= (y)) ? (y) : (x))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Error type.
 */
HAP_ENUM_BEGIN(uint8_t, HAPError) {
    kHAPError_None,           /**< No error occurred. */
    kHAPError_Unknown,        /**< Unknown error. */
    kHAPError_InvalidState,   /**< Operation is not supported in current state. */
    kHAPError_InvalidData,    /**< Data has unexpected format. */
    kHAPError_OutOfResources, /**< Out of resources. */
    kHAPError_NotAuthorized,  /**< Insufficient authorization. */
    kHAPError_Busy            /**< Operation failed temporarily, retry later. */
} HAP_ENUM_END(uint8_t, HAPError);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * System time expressed as milliseconds relative to an implementation-defined time in the past.
 */
typedef uint64_t HAPTime;

/**
 * Time interval in nanoseconds.
 */
typedef uint64_t HAPTimeNS;

/**
 * 1 millisecond in milliseconds.
 */
#define HAPMillisecond ((HAPTime) 1)

/**
 * 1 second in milliseconds.
 */
#define HAPSecond ((HAPTime)(1000 * HAPMillisecond))

/**
 * 1 minute in milliseconds.
 */
#define HAPMinute ((HAPTime)(60 * HAPSecond))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Length of a MAC address. */
#define kHAPMACAddress_NumBytes ((size_t) 6)

/**
 * MAC address.
 */
typedef struct {
    /** Network byte order (big-endian). */
    uint8_t bytes[kHAPMACAddress_NumBytes];
} HAPMACAddress;
HAP_STATIC_ASSERT(sizeof(HAPMACAddress) == kHAPMACAddress_NumBytes, HAPMACAddress);

/**
 * IP address version.
 */
HAP_ENUM_BEGIN(uint8_t, HAPIPAddressVersion) { /** IPv4. */
                                               kHAPIPAddressVersion_IPv4 = 1,

                                               /** IPv6. */
                                               kHAPIPAddressVersion_IPv6
} HAP_ENUM_END(uint8_t, HAPIPAddressVersion);

/**
 * Determines whether a value represents a valid IP address version.
 *
 * @param      value                Value.
 *
 * @return true                     If the provided value is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPIPAddressVersionIsValid(HAPIPAddressVersion value);

/**
 * Network port.
 */
typedef uint16_t HAPNetworkPort;

/**
 * Any network port.
 */
#define kHAPNetworkPort_Any ((HAPNetworkPort) 0)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Length of a WPA/WPA2 Personal PSK.
 */
#define kHAPWiFiWPAPSK_NumBytes ((size_t) 32)

/**
 * Computes the WPA PSK from an ASCII passphrase for a SSID.
 *
 * - Passphrase must be valid.
 *
 * @param[out] psk                  WPA/WPA2 personal PSK.
 * @param      ssid                 SSID of the WPA/WPA2 personal Wi-Fi network.
 * @param      passphrase           The network's passphrase credential: 8-63 printable ASCII characters.
 */
void HAPWiFiGetWPAPSKForPassphrase(
        uint8_t psk[_Nonnull kHAPWiFiWPAPSK_NumBytes],
        const char* ssid,
        const char* passphrase);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Setup info.
 */
typedef struct {
    uint8_t salt[16];      /**< SRP salt. */
    uint8_t verifier[384]; /**< SRP verifier. */
} HAPSetupInfo;
HAP_STATIC_ASSERT(sizeof(HAPSetupInfo) == 400, HAPSetupInfo);

/**
 * NULL-terminated setup ID string (format: XXXX).
 */
typedef struct {
    char stringValue[4 + 1]; /**< NULL-terminated. */
} HAPSetupID;
HAP_STATIC_ASSERT(sizeof(HAPSetupID) == 5, HAPSetupID);

/**
 * NULL-terminated setup code string (format: XXX-XX-XXX).
 */
typedef struct {
    char stringValue[10 + 1]; /**< NULL-terminated. */
} HAPSetupCode;
HAP_STATIC_ASSERT(sizeof(HAPSetupCode) == 11, HAPSetupCode);

/**
 * NULL-terminated setup payload string.
 */
typedef struct {
    char stringValue[20 + 1]; /**< NULL-terminated. */
} HAPSetupPayload;
HAP_STATIC_ASSERT(sizeof(HAPSetupPayload) == 21, HAPSetupPayload);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Advertising interval for Bluetooth LE.
 *
 * Unit: 0.625 ms
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 2 Part E Section 7.8.5 LE Set Advertising Parameters Command
 */
typedef uint16_t HAPBLEAdvertisingInterval;

/**
 * Converts an advertising interval in milliseconds to an advertising interval for Bluetooth LE.
 *
 * @param      milliseconds         Advertising interval in milliseconds.
 *
 * @return Advertising interval for Bluetooth LE.
 */
#define HAPBLEAdvertisingIntervalCreateFromMilliseconds(milliseconds) \
    ((HAPBLEAdvertisingInterval)((milliseconds) / (0.625f)))

/**
 * Converts an advertising interval for Bluetooth LE to an advertising interval in milliseconds.
 *
 * @param      advertisingInterval  Advertising interval for Bluetooth LE.
 *
 * @return Advertising interval in milliseconds.
 */
#define HAPBLEAdvertisingIntervalGetMilliseconds(advertisingInterval) \
    ((advertisingInterval) * (0.625f * HAPMillisecond))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Fills a buffer with zeros.
 *
 * @param[out] bytes                Buffer to fill with zeros.
 * @param      numBytes             Number of bytes to fill.
 */
void HAPRawBufferZero(void* bytes, size_t numBytes);

/**
 * Copies bytes from a source buffer to a destination buffer.
 *
 * @param[out] destinationBytes     Destination buffer.
 * @param      sourceBytes          Source buffer.
 * @param      numBytes             Number of bytes to copy.
 */
void HAPRawBufferCopyBytes(void* destinationBytes, const void* sourceBytes, size_t numBytes);

/**
 * Determines equality of two buffers in constant time.
 *
 * @param      bytes                Buffer to compare.
 * @param      otherBytes           Buffer to compare with.
 * @param      numBytes             Number of bytes to compare.
 *
 * @return true                     If the contents of both buffers are equal.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPRawBufferAreEqual(const void* bytes, const void* otherBytes, size_t numBytes);

/**
 * Determines if a buffer contains only zeros in constant time.
 *
 * @param      bytes                Buffer to check for zeros.
 * @param      numBytes             Length of buffer.
 *
 * @return true                     If a buffer contains only zeros
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPRawBufferIsZero(const void* bytes, size_t numBytes);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_STATIC_ASSERT(CHAR_BIT == 8, CHAR_BIT);
HAP_STATIC_ASSERT(sizeof(bool) == 1, bool);
HAP_STATIC_ASSERT(sizeof(uint8_t) == 1, uint8_t);
HAP_STATIC_ASSERT(sizeof(uint16_t) == 2, uint16_t);
HAP_STATIC_ASSERT(sizeof(uint32_t) == 4, uint32_t);
HAP_STATIC_ASSERT(sizeof(uint64_t) == 8, uint64_t);
HAP_STATIC_ASSERT(sizeof(int8_t) == 1, int8_t);
HAP_STATIC_ASSERT(sizeof(int16_t) == 2, int16_t);
HAP_STATIC_ASSERT(sizeof(int32_t) == 4, int32_t);
HAP_STATIC_ASSERT(sizeof(int64_t) == 8, int64_t);
HAP_STATIC_ASSERT(sizeof(float) == sizeof(uint32_t), float);
HAP_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t), double);
HAP_STATIC_ASSERT(sizeof(size_t) <= sizeof(uint64_t), size_t);

HAP_STATIC_ASSERT('\b' == 0x8, Backspace);
HAP_STATIC_ASSERT('\f' == 0xc, FormFeed);
HAP_STATIC_ASSERT('\n' == 0xa, NewLine);
HAP_STATIC_ASSERT('\r' == 0xd, CarriageReturn);
HAP_STATIC_ASSERT('\t' == 0x9, HorizontalTab);

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt8 value from a buffer.
 *
 * @param      bytes                Buffer to read from. Must contain at least 1 byte.
 *
 * @return Value that has been read.
 */
#define HAPReadUInt8(bytes) (((const uint8_t*) (bytes))[0])

/**
 * Writes a UInt8 value to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 1 byte.
 * @param      value                Value to write.
 */
#define HAPWriteUInt8(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (value); \
    } while (0)

/**
 * Expands a UInt8 value to a sequence of bytes.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandUInt8(value) (value),

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int8 value from a buffer.
 *
 * @param      bytes                Buffer to read from. Must contain at least 1 byte.
 *
 * @return Value that has been read.
 */
#define HAPReadInt8(bytes) (int8_t)(((const uint8_t*) (bytes))[0])

/**
 * Writes an Int8 value to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 1 byte.
 * @param      value                Value to write.
 */
#define HAPWriteInt8(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = ((uint8_t)(value)); \
    } while (0)

/**
 * Expands an Int8 value to a sequence of bytes.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandInt8(value) (uint8_t)(value),

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt16 value from a buffer containing its corresponding little-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 2 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadLittleUInt16(bytes) \
    (uint16_t)( \
            (uint16_t)((uint16_t)((const uint8_t*) (bytes))[0] << 0x00U) | \
            (uint16_t)((uint16_t)((const uint8_t*) (bytes))[1] << 0x08U))

/**
 * Writes a UInt16 value's little endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 2 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteLittleUInt16(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint16_t)((value) >> 0x00U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint16_t)((value) >> 0x08U) & 0xFFU; \
    } while (0)

/**
 * Expands a UInt16 value to a sequence of bytes containing its corresponding little-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandLittleUInt16(value) \
    (uint8_t)((uint16_t)((value) >> 0x00U) & 0xFFU), (uint8_t)((uint16_t)((value) >> 0x08U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int16 value from a buffer containing its corresponding little-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 2 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadLittleInt16(bytes) \
    (int16_t)( \
            (uint16_t)((uint16_t)((const uint8_t*) (bytes))[0] << 0x00U) | \
            (uint16_t)((uint16_t)((const uint8_t*) (bytes))[1] << 0x08U))

/**
 * Writes an Int16 value's little endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 2 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteLittleInt16(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint16_t)((uint16_t)(value) >> 0x00U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint16_t)((uint16_t)(value) >> 0x08U) & 0xFFU; \
    } while (0)

/**
 * Expands an Int16 value to a sequence of bytes containing its corresponding little-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandLittleInt16(value) \
    (uint8_t)((uint16_t)((uint16_t)(value) >> 0x00U) & 0xFFU), (uint8_t)((uint16_t)((uint16_t)(value) >> 0x08U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt24 value from a buffer containing its corresponding little-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 3 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadLittleUInt24(bytes) \
    (uint32_t)( \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[0] << 0x00U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[1] << 0x08U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[2] << 0x10U))

/**
 * Writes a UInt24 value's little endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 3 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteLittleUInt24(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint32_t)((value) >> 0x00U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint32_t)((value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint32_t)((value) >> 0x10U) & 0xFFU; \
    } while (0)

/**
 * Expands a UInt24 value to a sequence of bytes containing its corresponding little-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandLittleUInt24(value) \
    (uint8_t)((uint32_t)((value) >> 0x00U) & 0xFFU), (uint8_t)((uint32_t)((value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint32_t)((value) >> 0x10U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int24 value from a buffer containing its corresponding little-endian representation.
 *
 * - Int24 is represented as an Int32 value.
 *
 * @param      bytes                Buffer to read from. Must contain at least 3 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadLittleInt24(bytes) \
    (int32_t)( \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[0] << 0x00U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[1] << 0x08U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[2] << 0x10U))

/**
 * Writes an Int24 value's little endian representation to a buffer.
 *
 * - Int24 is represented as an Int32 value.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 3 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteLittleInt24(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint32_t)((uint32_t)(value) >> 0x00U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint32_t)((uint32_t)(value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint32_t)((uint32_t)(value) >> 0x10U) & 0xFFU; \
    } while (0)

/**
 * Expands an Int24 value to a sequence of bytes containing its corresponding little-endian representation.
 *
 * - Int24 is represented as an Int32 value.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandLittleInt24(value) \
    (uint8_t)((uint32_t)((uint32_t)(value) >> 0x00U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x10U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt32 value from a buffer containing its corresponding little-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 4 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadLittleUInt32(bytes) \
    (uint32_t)( \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[0] << 0x00U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[1] << 0x08U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[2] << 0x10U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[3] << 0x18U))

/**
 * Writes a UInt32 value's little endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 4 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteLittleUInt32(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint32_t)((value) >> 0x00U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint32_t)((value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint32_t)((value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[3] = (uint32_t)((value) >> 0x18U) & 0xFFU; \
    } while (0)

/**
 * Expands a UInt32 value to a sequence of bytes containing its corresponding little-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandLittleUInt32(value) \
    (uint8_t)((uint32_t)((value) >> 0x00U) & 0xFFU), (uint8_t)((uint32_t)((value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint32_t)((value) >> 0x10U) & 0xFFU), (uint8_t)((uint32_t)((value) >> 0x18U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int32 value from a buffer containing its corresponding little-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 4 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadLittleInt32(bytes) \
    (int32_t)( \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[0] << 0x00U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[1] << 0x08U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[2] << 0x10U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[3] << 0x18U))

/**
 * Writes an Int32 value's little endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 4 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteLittleInt32(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint32_t)((uint32_t)(value) >> 0x00U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint32_t)((uint32_t)(value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint32_t)((uint32_t)(value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[3] = (uint32_t)((uint32_t)(value) >> 0x18U) & 0xFFU; \
    } while (0)

/**
 * Expands an Int32 value to a sequence of bytes containing its corresponding little-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandLittleInt32(value) \
    (uint8_t)((uint32_t)((uint32_t)(value) >> 0x00U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x10U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x18U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt64 value from a buffer containing its corresponding little-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 8 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadLittleUInt64(bytes) \
    (uint64_t)( \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[0] << 0x00U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[1] << 0x08U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[2] << 0x10U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[3] << 0x18U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[4] << 0x20U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[5] << 0x28U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[6] << 0x30U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[7] << 0x38U))

/**
 * Writes a UInt64 value's little endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 8 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteLittleUInt64(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint64_t)((value) >> 0x00U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint64_t)((value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint64_t)((value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[3] = (uint64_t)((value) >> 0x18U) & 0xFFU; \
        ((uint8_t*) (bytes))[4] = (uint64_t)((value) >> 0x20U) & 0xFFU; \
        ((uint8_t*) (bytes))[5] = (uint64_t)((value) >> 0x28U) & 0xFFU; \
        ((uint8_t*) (bytes))[6] = (uint64_t)((value) >> 0x30U) & 0xFFU; \
        ((uint8_t*) (bytes))[7] = (uint64_t)((value) >> 0x38U) & 0xFFU; \
    } while (0)

/**
 * Expands a UInt64 value to a sequence of bytes containing its corresponding little-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandLittleUInt64(value) \
    (uint8_t)((uint64_t)((value) >> 0x00U) & 0xFFU), (uint8_t)((uint64_t)((value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint64_t)((value) >> 0x10U) & 0xFFU), (uint8_t)((uint64_t)((value) >> 0x18U) & 0xFFU), \
            (uint8_t)((uint64_t)((value) >> 0x20U) & 0xFFU), (uint8_t)((uint64_t)((value) >> 0x28U) & 0xFFU), \
            (uint8_t)((uint64_t)((value) >> 0x30U) & 0xFFU), (uint8_t)((uint64_t)((value) >> 0x38U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int64 value from a buffer containing its corresponding little-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 8 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadLittleInt64(bytes) \
    (int64_t)( \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[0] << 0x00U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[1] << 0x08U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[2] << 0x10U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[3] << 0x18U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[4] << 0x20U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[5] << 0x28U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[6] << 0x30U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[7] << 0x38U))

/**
 * Writes an Int64 value's little endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 8 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteLittleInt64(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint64_t)((uint64_t)(value) >> 0x00U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint64_t)((uint64_t)(value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint64_t)((uint64_t)(value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[3] = (uint64_t)((uint64_t)(value) >> 0x18U) & 0xFFU; \
        ((uint8_t*) (bytes))[4] = (uint64_t)((uint64_t)(value) >> 0x20U) & 0xFFU; \
        ((uint8_t*) (bytes))[5] = (uint64_t)((uint64_t)(value) >> 0x28U) & 0xFFU; \
        ((uint8_t*) (bytes))[6] = (uint64_t)((uint64_t)(value) >> 0x30U) & 0xFFU; \
        ((uint8_t*) (bytes))[7] = (uint64_t)((uint64_t)(value) >> 0x38U) & 0xFFU; \
    } while (0)

/**
 * Expands an Int64 value to a sequence of bytes containing its corresponding little-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandLittleInt64(value) \
    (uint8_t)((uint64_t)((uint64_t)(value) >> 0x00U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x10U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x18U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x20U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x28U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x30U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x38U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt16 value from a buffer containing its corresponding big-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 2 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadBigUInt16(bytes) \
    (uint16_t)( \
            (uint16_t)((uint16_t)((const uint8_t*) (bytes))[0] << 0x08U) | \
            (uint16_t)((uint16_t)((const uint8_t*) (bytes))[1] << 0x00U))

/**
 * Writes a UInt16 value's big endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 2 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteBigUInt16(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint16_t)((value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint16_t)((value) >> 0x00U) & 0xFFU; \
    } while (0)

/**
 * Expands a UInt16 value to a sequence of bytes containing its corresponding big-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandBigUInt16(value) \
    (uint8_t)((uint16_t)((value) >> 0x08U) & 0xFFU), (uint8_t)((uint16_t)((value) >> 0x00U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int16 value from a buffer containing its corresponding big-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 2 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadBigInt16(bytes) \
    (uint16_t)( \
            (uint16_t)((uint16_t)((const uint8_t*) (bytes))[0] << 0x08U) | \
            (uint16_t)((uint16_t)((const uint8_t*) (bytes))[1] << 0x00U))

/**
 * Writes an Int16 value's big endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 2 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteBigInt16(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint16_t)((uint16_t)(value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint16_t)((uint16_t)(value) >> 0x00U) & 0xFFU; \
    } while (0)

/**
 * Expands an Int16 value to a sequence of bytes containing its corresponding big-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandBigInt16(value) \
    (uint8_t)((uint16_t)((uint16_t)(value) >> 0x08U) & 0xFFU), (uint8_t)((uint16_t)((uint16_t)(value) >> 0x00U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt24 value from a buffer containing its corresponding big-endian representation.
 *
 * - UInt24 is represented as a UInt32 value.
 *
 * @param      bytes                Buffer to read from. Must contain at least 3 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadBigUInt24(bytes) \
    (uint32_t)( \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[0] << 0x10U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[1] << 0x08U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[2] << 0x00U))

/**
 * Writes a UInt24 value's big endian representation to a buffer.
 *
 * - UInt24 is represented as a UInt32 value.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 3 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteBigUInt24(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint32_t)((value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint32_t)((value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint32_t)((value) >> 0x00U) & 0xFFU; \
    } while (0)

/**
 * Expands a UInt24 value to a sequence of bytes containing its corresponding big-endian representation.
 *
 * - UInt24 is represented as a UInt32 value.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandBigUInt24(value) \
    (uint8_t)((uint32_t)((value) >> 0x10U) & 0xFFU), (uint8_t)((uint32_t)((value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint32_t)((value) >> 0x00U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int24 value from a buffer containing its corresponding big-endian representation.
 *
 * - Int24 is represented as an Int32 value.
 *
 * @param      bytes                Buffer to read from. Must contain at least 3 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadBigInt24(bytes) \
    (int32_t)( \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[0] << 0x10U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[1] << 0x08U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[2] << 0x00U))

/**
 * Writes an Int24 value's big endian representation to a buffer.
 *
 * - Int24 is represented as an Int32 value.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 3 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteBigInt24(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint32_t)((uint32_t)(value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint32_t)((uint32_t)(value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint32_t)((uint32_t)(value) >> 0x00U) & 0xFFU; \
    } while (0)

/**
 * Expands an Int24 value to a sequence of bytes containing its corresponding big-endian representation.
 *
 * - Int24 is represented as an Int32 value.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandBigInt24(value) \
    (uint8_t)((uint32_t)((uint32_t)(value) >> 0x10U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x00U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt32 value from a buffer containing its corresponding big-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 4 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadBigUInt32(bytes) \
    (uint32_t)( \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[0] << 0x18U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[1] << 0x10U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[2] << 0x08U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[3] << 0x00U))

/**
 * Writes a UInt32 value's big endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 4 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteBigUInt32(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint32_t)((value) >> 0x18U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint32_t)((value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint32_t)((value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[3] = (uint32_t)((value) >> 0x00U) & 0xFFU; \
    } while (0)

/**
 * Expands a UInt32 value to a sequence of bytes containing its corresponding big-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandBigUInt32(value) \
    (uint8_t)((uint32_t)((value) >> 0x18U) & 0xFFU), (uint8_t)((uint32_t)((value) >> 0x10U) & 0xFFU), \
            (uint8_t)((uint32_t)((value) >> 0x08U) & 0xFFU), (uint8_t)((uint32_t)((value) >> 0x00U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int32 value from a buffer containing its corresponding big-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 4 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadBigInt32(bytes) \
    (int32_t)( \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[0] << 0x18U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[1] << 0x10U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[2] << 0x08U) | \
            (uint32_t)((uint32_t)((const uint8_t*) (bytes))[3] << 0x00U))

/**
 * Writes an Int32 value's big endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 4 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteBigInt32(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint32_t)((uint32_t)(value) >> 0x18U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint32_t)((uint32_t)(value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint32_t)((uint32_t)(value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[3] = (uint32_t)((uint32_t)(value) >> 0x00U) & 0xFFU; \
    } while (0)

/**
 * Expands an Int32 value to a sequence of bytes containing its corresponding big-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandBigInt32(value) \
    (uint8_t)((uint32_t)((uint32_t)(value) >> 0x18U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x10U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint32_t)((uint32_t)(value) >> 0x00U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a UInt64 value from a buffer containing its corresponding big-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 8 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadBigUInt64(bytes) \
    (uint64_t)( \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[0] << 0x38U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[1] << 0x30U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[2] << 0x28U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[3] << 0x20U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[4] << 0x18U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[5] << 0x10U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[6] << 0x08U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[7] << 0x00U))

/**
 * Writes a UInt64 value's big endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 8 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteBigUInt64(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint64_t)((value) >> 0x38U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint64_t)((value) >> 0x30U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint64_t)((value) >> 0x28U) & 0xFFU; \
        ((uint8_t*) (bytes))[3] = (uint64_t)((value) >> 0x20U) & 0xFFU; \
        ((uint8_t*) (bytes))[4] = (uint64_t)((value) >> 0x18U) & 0xFFU; \
        ((uint8_t*) (bytes))[5] = (uint64_t)((value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[6] = (uint64_t)((value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[7] = (uint64_t)((value) >> 0x00U) & 0xFFU; \
    } while (0)

/**
 * Expands a UInt64 value to a sequence of bytes containing its corresponding big-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandBigUInt64(value) \
    (uint8_t)((uint64_t)((value) >> 0x38U) & 0xFFU), (uint8_t)((uint64_t)((value) >> 0x30U) & 0xFFU), \
            (uint8_t)((uint64_t)((value) >> 0x28U) & 0xFFU), (uint8_t)((uint64_t)((value) >> 0x20U) & 0xFFU), \
            (uint8_t)((uint64_t)((value) >> 0x18U) & 0xFFU), (uint8_t)((uint64_t)((value) >> 0x10U) & 0xFFU), \
            (uint8_t)((uint64_t)((value) >> 0x08U) & 0xFFU), (uint8_t)((uint64_t)((value) >> 0x00U) & 0xFFU)

//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads an Int64 value from a buffer containing its corresponding big-endian representation.
 *
 * @param      bytes                Buffer to read from. Must contain at least 8 bytes.
 *
 * @return Value that has been read.
 */
#define HAPReadBigInt64(bytes) \
    (int64_t)( \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[0] << 0x38U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[1] << 0x30U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[2] << 0x28U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[3] << 0x20U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[4] << 0x18U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[5] << 0x10U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[6] << 0x08U) | \
            (uint64_t)((uint64_t)((const uint8_t*) (bytes))[7] << 0x00U))

/**
 * Writes an Int64 value's big endian representation to a buffer.
 *
 * @param[out] bytes                Buffer to write to. Must have space for at least 8 bytes.
 * @param      value                Value to write.
 */
#define HAPWriteBigInt64(bytes, value) \
    do { \
        ((uint8_t*) (bytes))[0] = (uint64_t)((uint64_t)(value) >> 0x38U) & 0xFFU; \
        ((uint8_t*) (bytes))[1] = (uint64_t)((uint64_t)(value) >> 0x30U) & 0xFFU; \
        ((uint8_t*) (bytes))[2] = (uint64_t)((uint64_t)(value) >> 0x28U) & 0xFFU; \
        ((uint8_t*) (bytes))[3] = (uint64_t)((uint64_t)(value) >> 0x20U) & 0xFFU; \
        ((uint8_t*) (bytes))[4] = (uint64_t)((uint64_t)(value) >> 0x18U) & 0xFFU; \
        ((uint8_t*) (bytes))[5] = (uint64_t)((uint64_t)(value) >> 0x10U) & 0xFFU; \
        ((uint8_t*) (bytes))[6] = (uint64_t)((uint64_t)(value) >> 0x08U) & 0xFFU; \
        ((uint8_t*) (bytes))[7] = (uint64_t)((uint64_t)(value) >> 0x00U) & 0xFFU; \
    } while (0)

/**
 * Expands an Int64 value to a sequence of bytes containing its corresponding big-endian representation.
 *
 * @param      value                Value to expand.
 *
 * @return Sequence of bytes representing the value.
 */
#define HAPExpandBigInt64(value) \
    (uint8_t)((uint64_t)((uint64_t)(value) >> 0x38U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x30U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x28U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x20U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x18U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x10U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x08U) & 0xFFU), \
            (uint8_t)((uint64_t)((uint64_t)(value) >> 0x00U) & 0xFFU)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Maximum number of bytes needed by the string representation of a UInt8 in decimal format.
 *
 * - UINT8_MAX = 0xFF = 255.
 */
#define kHAPUInt8_MaxDescriptionBytes (sizeof "255")

/**
 * Maximum number of bytes needed by the string representation of a UInt16 in decimal format.
 *
 * - UINT16_MAX = 0xFFFF = 65535.
 */
#define kHAPUInt16_MaxDescriptionBytes (sizeof "65535")

/**
 * Maximum number of bytes needed by the string representation of a UInt32 in decimal format.
 *
 * - UINT32_MAX = 0xFFFFFFFF = 4294967295.
 */
#define kHAPUInt32_MaxDescriptionBytes (sizeof "4294967295")

/**
 * Maximum number of bytes needed by the string representation of a float in decimal format.
 *
 * - Maximum is 9 significant digits + decimal point + sign + two digit exponent.
 */
#define kHAPFloat_MaxDescriptionBytes (sizeof "-1.23456789e-33")

/**
 * Determines the space needed by the string representation of the given integer value in decimal format.
 *
 * @param      value                Numeric value.
 *
 * @return Number of bytes that the value's string representation needs (excluding NULL-terminator).
 */
HAP_RESULT_USE_CHECK
size_t HAPInt32GetNumDescriptionBytes(int32_t value);

/**
 * Determines the space needed by the string representation of the given integer value in decimal format.
 *
 * @param      value                Numeric value.
 *
 * @return Number of bytes that the value's string representation needs (excluding NULL-terminator).
 */
HAP_RESULT_USE_CHECK
size_t HAPUInt64GetNumDescriptionBytes(uint64_t value);

/**
 * Gets the string representation of the given integer value in decimal format.
 *
 * @param      value                Numeric value.
 * @param[out] bytes                Buffer to fill with the value's string representation. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt64GetDescription(uint64_t value, char* bytes, size_t maxBytes);

/**
 * Letter case.
 */
HAP_ENUM_BEGIN(uint8_t, HAPLetterCase) { /**
                                          * Lowercase.
                                          */
                                         kHAPLetterCase_Lowercase = 'a',

                                         /**
                                          * Uppercase.
                                          */
                                         kHAPLetterCase_Uppercase = 'A'
} HAP_ENUM_END(uint8_t, HAPLetterCase);

/**
 * Gets the string representation of the given integer value in hexadecimal format.
 *
 * @param      value                Numeric value.
 * @param[out] bytes                Buffer to fill with the value's string representation. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 * @param      letterCase           Lower case or upper case characters.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt64GetHexDescription(uint64_t value, char* bytes, size_t maxBytes, HAPLetterCase letterCase);

/**
 * Creates a new integer value from the given string.
 *
 * - The string may begin with a plus or minus sign character (+ or -), followed by one or more numeric digits (0-9).
 *
 * @param      description          The ASCII representation of a number. NULL-terminated.
 * @param[out] value                Numeric value.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the string is in an invalid format,
 *                                  or if the value it denotes in base 10 is not representable.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt64FromString(const char* description, uint64_t* value);

/**
 * Creates a new integer value from the given string.
 *
 * - The string may begin with a plus or minus sign character (+ or -), followed by one or more numeric digits (0-9).
 *
 * @param      description          The ASCII representation of a number. NULL-terminated.
 * @param[out] value                Numeric value.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the string is in an invalid format,
 *                                  or if the value it denotes in base 10 is not representable.
 */
HAP_RESULT_USE_CHECK
HAPError HAPInt64FromString(const char* description, int64_t* value);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Creates a new float value with the given bit pattern.
 *
 * - The bit pattern is interpreted in the binary interchange format defined by the IEEE 754 specification.
 *
 * @param      bitPattern           The integer encoding of the float value.
 *
 * @return Float value with the given bit pattern.
 */
HAP_RESULT_USE_CHECK
float HAPFloatFromBitPattern(uint32_t bitPattern);

/**
 * Returns the bit pattern of a float value.
 *
 * - The bit pattern matches the binary interchange format defined by the IEEE 754 specification.
 *
 * @param      value                Value.
 *
 * @return The bit pattern of the value's encoding.
 */
HAP_RESULT_USE_CHECK
uint32_t HAPFloatGetBitPattern(float value);

/**
 * Creates a new float value from the given string.
 *
 * - The string can represent a real number in decimal format.
 *
 * - The given string may begin with a plus or minus sign character (+ or -).
 *
 * - A decimal value contains the significand, a sequence of decimal digits that may include a decimal point.
 *   A decimal value may also include an exponent following the significand,
 *   indicating the power of 10 by which the significand should be multiplied.
 *   If included, the exponent is separated by a single character, e or E,
 *   and consists of an optional plus or minus sign character and a sequence of decimal digits.
 *
 * - Hexadecimal format is not supported at this time.
 *
 * - Special floating-point values for infinity and NaN ("not a number") are not supported at this time.
 *
 * @param      description          The ASCII representation of a number. NULL-terminated.
 * @param[out] value                Numeric value.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the string is in an invalid format.
 */
HAP_RESULT_USE_CHECK
HAPError HAPFloatFromString(const char* description, float* value);

/**
 * Creates a string representation of a float value.
 *
 * The string will represent the float in decimal format.
 * The string will use at most kHAPFloat_MaxDescriptionBytes bytes excluding NULL termination.
 * The output will contain:
 * - "nan", if the float represents a NAN.
 * - "inf" or "-inf" if the float represents plus or minus infinity.
 * - A decimal integer if the float represents as an integer in the range 0 to 999999.
 * - A decimal fixpoint number if the float is in the range 10^-4 to 10^6.
 * - A decimal float in scientific notation otherwise (x.xxxxxe-xx).
 * In any case the number of digits used is chosen such that reading the string with either
 * HAPFloatFromString() or the standard function strtof() will retrieve the original float.
 *
 * @param[out] bytes                Buffer to fill with the string. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 * @param      value                The float value.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPFloatGetDescription(char* bytes, size_t maxBytes, float value);

/**
 * Absolute value of the supplied floating point value.
 *
 * @param      value                Value.
 *
 * @return |value|.
 */
float HAPFloatGetAbsoluteValue(float value);

/**
 * Fractional part of the supplied floating point value.
 *
 * @param      value                Value.
 *
 * @return fractional part of value.
 */
float HAPFloatGetFraction(float value);

/**
 * Determines whether the supplied floating point value is zero.
 *
 * - This returns true for either -0.0 or +0.0.
 *
 * @param      value                Value to check.
 *
 * @return true                     If the supplied value is zero.
 * @return false                    Otherwise.
 */
bool HAPFloatIsZero(float value);

/**
 * Determines whether the supplied floating point value is finite.
 *
 * - All values other than NaN and infinity are considered finite, whether normal or subnormal.
 *
 * @param      value                Value to check.
 *
 * @return true                     If the supplied value is finite.
 * @return false                    Otherwise.
 */
bool HAPFloatIsFinite(float value);

/**
 * Determines whether the supplied floating point value is infinite.
 *
 * @param      value                Value to check.
 *
 * @return true                     If the supplied value is infinite.
 * @return false                    Otherwise.
 */
bool HAPFloatIsInfinite(float value);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Creates a new double value with the given bit pattern.
 *
 * - The bit pattern is interpreted in the binary interchange format defined by the IEEE 754 specification.
 *
 * @param      bitPattern           The integer encoding of the float value.
 *
 * @return Double value with the given bit pattern.
 */
HAP_RESULT_USE_CHECK
double HAPDoubleFromBitPattern(uint64_t bitPattern);

/**
 * Returns the bit pattern of a double value.
 *
 * - The bit pattern matches the binary interchange format defined by the IEEE 754 specification.
 *
 * @param      value                Value.
 *
 * @return The bit pattern of the value's encoding.
 */
HAP_RESULT_USE_CHECK
uint64_t HAPDoubleGetBitPattern(double value);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Creates a string from a formatted string and a variable number of arguments.
 *
 * - The supported conversion specifiers follow the IEEE printf specification.
 *   See http://pubs.opengroup.org/onlinepubs/009695399/functions/printf.html
 *
 * @param[out] bytes                Buffer to fill with the formatted string. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 * @param      format               A format string.
 * @param      ...                  Arguments for the format string.
 *
 * Currently the following options are supported:
 * flags:  0, +, ' '
 * width:  number
 * length: l, ll, z
 * types:  %, d, i, u, x, X, g, c, s, p
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_PRINTFLIKE(3, 4)
HAP_RESULT_USE_CHECK
HAPError HAPStringWithFormat(char* bytes, size_t maxBytes, const char* format, ...);

/**
 * Creates a string from a formatted string and a variable number of arguments.
 *
 * - The supported conversion specifiers follow the IEEE printf specification.
 *   See http://pubs.opengroup.org/onlinepubs/009695399/functions/printf.html
 *
 * Currently the following options are supported:
 * flags:  0, +, ' '
 * width:  number
 * length: l, ll, z
 * types:  %, d, i, u, x, X, g, c, s, p
 *
 * @param[out] bytes                Buffer to fill with the formatted string. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 * @param      format               A format string.
 * @param      arguments            Arguments for the format string.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_PRINTFLIKE(3, 0)
HAP_RESULT_USE_CHECK
HAPError HAPStringWithFormatAndArguments(char* bytes, size_t maxBytes, const char* format, va_list arguments);

/**
 * Returns the number of bytes of a string, excluding the NULL-terminator.
 *
 * @param      string               String. NULL-terminated.
 *
 * @return Number of bytes of the string.
 */
HAP_RESULT_USE_CHECK
size_t HAPStringGetNumBytes(const char* string);

/**
 * Determines equality of two strings.
 *
 * @param      string               String to compare. NULL-terminated.
 * @param      otherString          String to compare with. NULL-terminated.
 *
 * @return true                     If the contents of both strings are equal.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPStringAreEqual(const char* string, const char* otherString);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Determines whether the supplied data is a valid UTF-8 byte sequence according to
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - Table 3-7, page 94.
 *
 * @param      bytes                Input data.
 * @param      numBytes             Length of input data.
 *
 * @return true                     If the supplied data is a valid UTF-8 byte sequence.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPUTF8IsValidData(const void* bytes, size_t numBytes);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Length of SHA-1 hash.
 */
#define kHAPSHA1Checksum_NumBytes ((size_t) 20)

/**
 * The SHA-1 based checksum of a given input block is computed and put into @p checksum.
 *
 * @param[out] checksum             Generated checksum.
 * @param      bytes                Input data.
 * @param      numBytes             Length of @p data.
 */
void HAPSHA1Checksum(uint8_t checksum[_Nonnull kHAPSHA1Checksum_NumBytes], const void* bytes, size_t numBytes);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#include "HAPAssert.h"
#include "HAPLog.h"

// Functions to convert a _Nullable type to its _Nonnull variant.
#if __has_feature(nullability) && __has_attribute(overloadable)
/**
 * Generates support functions to convert from _Nullable to _Nonnull.
 *
 * @param      type                 Type for which to generate support functions.
 */
#define HAP_NONNULL_SUPPORT(type) \
    HAP_DIAGNOSTIC_PUSH \
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wunused-function") \
    __attribute__((always_inline)) __attribute__(( \
            overloadable)) static type* _Nonnull HAPNonnull(/* NOLINT(bugprone-macro-parentheses) */ \
                                                            type* _Nullable const value) /* NOLINT(bugprone-macro-parentheses) \
                                                                                          */ \
    { \
        HAPAssert(value); \
        return value; \
    } \
\
    __attribute__((always_inline)) __attribute__(( \
            overloadable)) static const type* _Nonnull HAPNonnull(/* NOLINT(bugprone-macro-parentheses) */ \
                                                                  const type* _Nullable const value) /* NOLINT(bugprone-macro-parentheses) \
                                                                                                      */ \
    { \
        HAPAssert(value); \
        return value; \
    } \
    HAP_DIAGNOSTIC_POP

HAP_DIAGNOSTIC_PUSH
HAP_DIAGNOSTIC_IGNORED_CLANG("-Wunused-function")
__attribute__((always_inline))
__attribute__((overloadable)) static void* _Nonnull HAPNonnullVoid(void* _Nullable const value) {
    HAPAssert(value);
    return value;
}

__attribute__((always_inline))
__attribute__((overloadable)) static const void* _Nonnull HAPNonnullVoid(const void* _Nullable const value) {
    HAPAssert(value);
    return value;
}
HAP_DIAGNOSTIC_POP
#else
/**
 * Generates support functions to convert from _Nullable to _Nonnull.
 *
 * @param      type                 Type for which to generate support functions.
 */
#define HAP_NONNULL_SUPPORT(type)

static inline void HAPCheckNonnull(const void* value) {
    HAPAssert(value);
}

#define HAP_PP_COMMA          ,
#define HAPNonnullVoid(value) (HAPCheckNonnull(value) HAP_PP_COMMA value)
#define HAPNonnull(value)     (HAPNonnullVoid(value))
#endif
HAP_NONNULL_SUPPORT(char)
HAP_NONNULL_SUPPORT(uint8_t)
HAP_NONNULL_SUPPORT(int64_t)
HAP_NONNULL_SUPPORT(double)
HAP_NONNULL_SUPPORT(HAPSetupCode)
HAP_NONNULL_SUPPORT(HAPSetupPayload)

#ifdef __cplusplus
}
#endif

#endif
