// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_IP_ACCESSORY_PROTOCOL_H
#define HAP_IP_ACCESSORY_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

#define kHAPIPAccessoryProtocolAID_PrimaryAccessory ((uint64_t) 1)

#define kHAPIPAccessoryProtocolIID_AccessoryInformation ((uint64_t) 1)

typedef struct {
    uint64_t aid;
    uint64_t iid;
    int32_t status;
    union {
        int32_t intValue;
        uint64_t unsignedIntValue;
        float floatValue;
        struct {
            char* _Nullable bytes;
            size_t numBytes;
        } stringValue;
    } value;
    bool ev;
} HAPIPReadContext;
HAP_STATIC_ASSERT(sizeof(HAPIPReadContextRef) >= sizeof(HAPIPReadContext), HAPIPReadContext);

typedef struct {
    bool meta;
    bool perms;
    bool type;
    bool ev;
} HAPIPReadRequestParameters;

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicReadRequests(
        char* bytes,
        size_t numBytes,
        HAPIPReadContextRef* readContexts,
        size_t maxReadContexts,
        size_t* numReadContexts,
        HAPIPReadRequestParameters* parameters);

HAP_RESULT_USE_CHECK
size_t HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
        HAPAccessoryServerRef* server,
        HAPIPReadContextRef* readContexts,
        size_t numReadContexts,
        HAPIPReadRequestParameters* parameters);

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
        HAPAccessoryServerRef* server,
        HAPIPReadContextRef* readContexts,
        size_t numReadContexts,
        HAPIPReadRequestParameters* parameters,
        HAPIPByteBuffer* buffer);

HAP_ENUM_BEGIN(uint8_t, HAPIPWriteValueType) { kHAPIPWriteValueType_None,
                                               kHAPIPWriteValueType_Int,
                                               kHAPIPWriteValueType_UInt,
                                               kHAPIPWriteValueType_Float,
                                               kHAPIPWriteValueType_String } HAP_ENUM_END(uint8_t, HAPIPWriteValueType);

HAP_ENUM_BEGIN(uint8_t, HAPIPEventNotificationState) {
    kHAPIPEventNotificationState_Undefined,
    kHAPIPEventNotificationState_Disabled,
    kHAPIPEventNotificationState_Enabled,
} HAP_ENUM_END(uint8_t, HAPIPEventNotificationState);

typedef struct {
    uint64_t aid;
    uint64_t iid;
    int32_t status;
    HAPIPWriteValueType type;
    union {
        int32_t intValue;
        uint64_t unsignedIntValue;
        float floatValue;
        struct {
            char* _Nullable bytes;
            size_t numBytes;
        } stringValue;
    } value;
    struct {
        char* _Nullable bytes;
        size_t numBytes;
    } authorizationData;
    bool remote;
    HAPIPEventNotificationState ev;
    bool response;
} HAPIPWriteContext;
HAP_STATIC_ASSERT(sizeof(HAPIPWriteContextRef) >= sizeof(HAPIPWriteContext), HAPIPWriteContext);

/**
 * Parses a PUT /characteristic request.
 *
 * @param      bytes                Bytes
 * @param      numBytes             Length of @p bytes.
 * @param[out] writeContexts        Contexts to store data about the received write requests.
 * @param      maxWriteContexts     Capacity of @p writeContexts.
 * @param[out] numWriteContexts     Number of valid contexts.
 * @param[out] hasPID               True if a PID was specified. False otherwise.
 * @param[out] pid                  PID, if a PID was specified.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If request malformed.
 * @return kHAPError_OutOfResources If not enough contexts were available.
 */
HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicWriteRequests(
        char* bytes,
        size_t numBytes,
        HAPIPWriteContextRef* writeContexts,
        size_t maxWriteContexts,
        size_t* numWriteContexts,
        bool* hasPID,
        uint64_t* pid);

HAP_RESULT_USE_CHECK
size_t HAPIPAccessoryProtocolGetNumCharacteristicWriteResponseBytes(
        HAPAccessoryServerRef* server,
        HAPIPWriteContextRef* writeContexts,
        size_t numWriteContexts);

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicWriteResponseBytes(
        HAPAccessoryServerRef* server,
        HAPIPWriteContextRef* writeContexts,
        size_t numWriteContexts,
        HAPIPByteBuffer* buffer);

HAP_RESULT_USE_CHECK
size_t HAPIPAccessoryProtocolGetNumEventNotificationBytes(
        HAPAccessoryServerRef* server,
        HAPIPReadContextRef* readContexts,
        size_t numReadContexts);

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetEventNotificationBytes(
        HAPAccessoryServerRef* server,
        HAPIPReadContextRef* readContexts,
        size_t numReadContexts,
        HAPIPByteBuffer* buffer);

/**
 * Parses a PUT /prepare request.
 *
 * @param      bytes                Buffer
 * @param      numBytes             Length of @p bytes.
 * @param[out] ttl                  TTL.
 * @param[out] pid                  PID.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If request malformed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicWritePreparation(
        const char* bytes,
        size_t numBytes,
        uint64_t* ttl,
        uint64_t* pid);
#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
