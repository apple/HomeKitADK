// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatformMFiTokenAuth+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "MFiTokenAuth" };

void HAPPlatformMFiTokenAuthCreate(
        HAPPlatformMFiTokenAuthRef _Nonnull mfiTokenAuth,
        const HAPPlatformMFiTokenAuthOptions* _Nonnull options) {
    HAPPrecondition(mfiTokenAuth);
    HAPPrecondition(options);
    HAPPrecondition(options->keyValueStore);
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiTokenAuthLoad(
        HAPPlatformMFiTokenAuthRef _Nonnull mfiTokenAuth,
        bool* _Nonnull valid,
        HAPPlatformMFiTokenAuthUUID* _Nullable mfiTokenUUID,
        void* _Nullable mfiTokenBytes,
        size_t maxMFiTokenBytes,
        size_t* _Nullable numMFiTokenBytes) {
    HAPPrecondition(mfiTokenAuth);
    HAPPrecondition(valid);
    HAPPrecondition((mfiTokenUUID == NULL) == (mfiTokenBytes == NULL));
    HAPPrecondition(!maxMFiTokenBytes || mfiTokenBytes);
    HAPPrecondition((mfiTokenBytes == NULL) == (numMFiTokenBytes == NULL));

    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiTokenAuthUpdate(
        HAPPlatformMFiTokenAuthRef _Nonnull mfiTokenAuth,
        const void* _Nonnull mfiTokenBytes,
        size_t numMFiTokenBytes) {
    HAPPrecondition(mfiTokenAuth);
    HAPPrecondition(mfiTokenBytes);
    HAPPrecondition(numMFiTokenBytes <= kHAPPlatformMFiTokenAuth_MaxMFiTokenBytes);

    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}

bool HAPPlatformMFiTokenAuthIsProvisioned(HAPPlatformMFiTokenAuthRef _Nonnull mfiTokenAuth) {
    HAPPrecondition(mfiTokenAuth);

    return false;
}
