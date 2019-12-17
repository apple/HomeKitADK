// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_IP_SESSION_H
#define HAP_IP_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * IP session context.
 */
HAP_ENUM_BEGIN(uint8_t, HAPIPSessionContext) { /**
                                                * Session context is reading the accessory database.
                                                */
                                               kHAPIPSessionContext_GetAccessories,

                                               /**
                                                * Session context is reading characteristics.
                                                */
                                               kHAPIPSessionContext_GetCharacteristics,

                                               /**
                                                * Session context is handling event notifications.
                                                */
                                               kHAPIPSessionContext_EventNotification
} HAP_ENUM_END(uint8_t, HAPIPSessionContext);

/**
 * Read result.
 */
typedef struct {
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
} HAPIPSessionReadResult;

/**
 * Returns whether event notifications are enabled for a given characteristic in a given service provided by a given
 * accessory object on a given session.
 *
 * @param      session              IP session descriptor.
 * @param      characteristic       The characteristic whose event notification state is to be returned.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 *
 * @return Whether event notifications are enabled for the given characteristic.
 */
HAP_RESULT_USE_CHECK
bool HAPIPSessionAreEventNotificationsEnabled(
        HAPIPSessionDescriptorRef* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory);

/**
 * Handles a read request on a given characteristic in a given service provided by a given accessory object on a given
 * session.
 *
 * @param      session              IP session descriptor.
 * @param      sessionContext       IP session context of the read request.
 * @param      characteristic       The characteristic whose event notification state is to be returned.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      readResult           The result of the of the read request.
 * @param      dataBuffer           Buffer to store data blobs, strings, or a set of one or more TLV8's.
 */
void HAPIPSessionHandleReadRequest(
        HAPIPSessionDescriptorRef* session,
        HAPIPSessionContext sessionContext,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPIPSessionReadResult* readResult,
        HAPIPByteBuffer* dataBuffer);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
