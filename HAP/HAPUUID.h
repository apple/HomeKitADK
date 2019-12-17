// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_UUID_H
#define HAP_UUID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Creates a HAPUUID structure from a short UUID that is based on the Apple-defined HAP Base UUID.
 *
 * - Full UUIDs have the form XXXXXXXX-0000-1000-8000-0026BB765291.
 *   The short form consists of just the front part, e.g. 0x43 for the HomeKit Light Bulb service.
 *   UUID strings use hexadecimal digits - remember to use the 0x prefix.
 *
 * - This function may only be used for Apple-defined types.
 *   For vendor-specific UUIDs, a different base UUID must be used.
 *
 * @param      uuid                 Short UUID, e.g. 0x43 for the HomeKit Light Bulb service.
 *
 * @return Initialized HAPUUID structure.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 6.6.1 Service and Characteristic Types
 */
#define HAPUUIDCreateAppleDefined(uuid) \
    { \
        { 0x91, 0x52, 0x76, 0xBB, 0x26, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, HAPExpandLittleUInt32(uuid) } \
    }

/**
 * Returns whether a HAP UUID is Apple defined.
 *
 * @param      uuid                 UUID.
 *
 * @return true                     If the UUID is Apple defined.
 * @return false                    Otherwise.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 6.6.1 Service and Characteristic Types
 */
HAP_RESULT_USE_CHECK
bool HAPUUIDIsAppleDefined(const HAPUUID* uuid);

/**
 * Determines the space needed by the string representation of a HAP UUID.
 *
 * @param      uuid                 UUID.
 *
 * @return Number of bytes that the UUID's string representation needs (excluding NULL-terminator).
 */
HAP_RESULT_USE_CHECK
size_t HAPUUIDGetNumDescriptionBytes(const HAPUUID* uuid);

/**
 * Gets the string representation of a HAP UUID.
 *
 * @param      uuid                 UUID.
 * @param[out] bytes                Buffer to fill with the UUID's string representation. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUUIDGetDescription(const HAPUUID* uuid, char* bytes, size_t maxBytes);

/**
 * Gets the short form of a HAP UUID.
 *
 * - When Apple-defined UUIDs based on the HAP Base UUID 00000000-0000-1000-8000-0026BB765291 are encoded
 *   in short form, the -0000-1000-8000-0026BB765291 suffix is omitted and leading zero bytes are removed.
 *   The remaining bytes are sent in the same order as when sending a full UUID.
 *   To convert back to a full UUID, the process is reversed.
 *
 * - Custom types do not use the HAP Base UUID and are encoded in the same format as the full UUID.
 *
 * - Examples:
 *   00000000-0000-1000-8000-0026BB765291 -> []
 *   0000003E-0000-1000-8000-0026BB765291 -> [0x3E]
 *   00000001-0000-1000-8000-0026BB765291 -> [0x01]
 *   00000F25-0000-1000-8000-0026BB765291 -> [0x25, 0x0F]
 *   0000BBAB-0000-1000-8000-0026BB765291 -> [0xAB, 0xBB]
 *   00112233-0000-1000-8000-0026BB765291 -> [0x33, 0x22, 0x11]
 *   010004FF-0000-1000-8000-0026BB765291 -> [0xFF, 0x04, 0x00, 0x01]
 *   FF000000-0000-1000-8000-0026BB765291 -> [0x00, 0x00, 0x00, 0xFF]
 *
 * @param      uuid                 UUID.
 * @param[out] bytes                Buffer to fill with the UUID's compact representation.
 * @param      maxBytes             Capacity of buffer.
 * @param[out] numBytes             Effective length of buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 6.6.1 Service and Characteristic Types
 */
HAP_RESULT_USE_CHECK
HAPError HAPUUIDGetShortFormBytes(const HAPUUID* uuid, void* bytes, size_t maxBytes, size_t* numBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
