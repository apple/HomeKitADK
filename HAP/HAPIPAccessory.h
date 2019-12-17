// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_IP_ACCESSORY_H
#define HAP_IP_ACCESSORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Serialization context for incremental attribute database serialization.
 */
typedef struct {
    /**
     * Serialization state
     */
    uint8_t state;

    /**
     * Generic array enumeration index.
     */
    uint8_t index;

    /**
     * Accessory index. 0 => primaryAccessory, > 0 => bridgedAccessories[accessoryIndex - 1]
     */
    uint8_t accessoryIndex;

    /**
     * Service index.
     */
    uint8_t serviceIndex;

    /**
     * Characteristic index.
     */
    uint8_t characteristicIndex;
} HAPIPAccessorySerializationContext;

/**
 * Creates a new serialization context.
 *
 * @param      context              An serialization context.
 */
void HAPIPAccessoryCreateSerializationContext(HAPIPAccessorySerializationContext* context);

/**
 * Returns whether the incremental response serialization for the given serialization context is complete.
 *
 * @param      context              Serialization context.
 *
 * @return Whether the serialization is complete.
 */
HAP_RESULT_USE_CHECK
bool HAPIPAccessorySerializationIsComplete(HAPIPAccessorySerializationContext* context);

/**
 * Incrementally serializes a GET /accessories response.
 *
 * @param      context              Serialization context to incrementally serialize the response.
 * @param      server               Accessory server.
 * @param      session              IP session descriptor.
 * @param[out] bytes                Buffer to fill.
 * @param      minBytes             Minimum number of bytes to serialize, until the response is complete.
 * @param      maxBytes             Maximum number of bytes to serialize in a single invocation of this function.
 * @param      numBytes             Number of bytes serialized.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPIPAccessorySerializeReadResponse(
        HAPIPAccessorySerializationContext* context,
        HAPAccessoryServerRef* server,
        HAPIPSessionDescriptorRef* session,
        char* bytes,
        size_t minBytes,
        size_t maxBytes,
        size_t* numBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
