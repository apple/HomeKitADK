// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_REQUEST_HANDLERS_HAP_PROTOCOL_INFORMATION_H
#define HAP_REQUEST_HANDLERS_HAP_PROTOCOL_INFORMATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Handle read request to the 'Version' characteristic of the HAP Protocol Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleHAPProtocolInformationVersionRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
