// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"
#include "HAPPlatformKeyValueStore+SDKDomains.h"
#include "HAPPlatformMFiTokenAuth+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "MFiTokenAuth" };

void HAPPlatformMFiTokenAuthCreate(
        HAPPlatformMFiTokenAuthRef mfiTokenAuth,
        const HAPPlatformMFiTokenAuthOptions* options) {
    HAPPrecondition(mfiTokenAuth);
    HAPPrecondition(options);
    HAPPrecondition(options->keyValueStore);

    mfiTokenAuth->keyValueStore = options->keyValueStore;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiTokenAuthLoad(
        HAPPlatformMFiTokenAuthRef mfiTokenAuth,
        bool* valid,
        HAPPlatformMFiTokenAuthUUID* _Nullable mfiTokenUUID,
        void* _Nullable mfiTokenBytes,
        size_t maxMFiTokenBytes,
        size_t* _Nullable numMFiTokenBytes) {
    HAPPrecondition(mfiTokenAuth);
    HAPPrecondition(valid);
    HAPPrecondition((mfiTokenUUID == NULL) == (mfiTokenBytes == NULL));
    HAPPrecondition(!maxMFiTokenBytes || mfiTokenBytes);
    HAPPrecondition((mfiTokenBytes == NULL) == (numMFiTokenBytes == NULL));

    HAPError err;

    bool foundMFiTokenUUID;
    size_t numMFiTokenUUIDBytes = 0;
    err = HAPPlatformKeyValueStoreGet(
            mfiTokenAuth->keyValueStore,
            kSDKKeyValueStoreDomain_Provisioning,
            kSDKKeyValueStoreKey_Provisioning_MFiTokenUUID,
            mfiTokenUUID,
            mfiTokenUUID ? sizeof *mfiTokenUUID : 0,
            mfiTokenUUID ? &numMFiTokenUUIDBytes : NULL,
            &foundMFiTokenUUID);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    bool foundMFiToken;
    size_t numMFiTokenBytes_ = 0;
    err = HAPPlatformKeyValueStoreGet(
            mfiTokenAuth->keyValueStore,
            kSDKKeyValueStoreDomain_Provisioning,
            kSDKKeyValueStoreKey_Provisioning_MFiToken,
            mfiTokenBytes,
            maxMFiTokenBytes,
            mfiTokenBytes ? &numMFiTokenBytes_ : NULL,
            &foundMFiToken);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (numMFiTokenBytes) {
        *numMFiTokenBytes = numMFiTokenBytes_;
    }

    *valid = foundMFiTokenUUID && foundMFiToken;
    if (!*valid) {
        return kHAPError_None;
    }
    if (numMFiTokenBytes && *numMFiTokenBytes == maxMFiTokenBytes) {
        HAPLog(&logObject,
               "Software Token does not fit into buffer: available = %lu bytes.",
               (unsigned long) maxMFiTokenBytes);
        return kHAPError_OutOfResources;
    }

    return kHAPError_None;
}

bool HAPPlatformMFiTokenAuthIsProvisioned(HAPPlatformMFiTokenAuthRef mfiTokenAuth) {
    HAPPrecondition(mfiTokenAuth);

    HAPError err;

    bool valid;
    err = HAPPlatformMFiTokenAuthLoad(mfiTokenAuth, &valid, NULL, NULL, 0, NULL);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return false;
    }

    return valid;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiTokenAuthUpdate(
        HAPPlatformMFiTokenAuthRef mfiTokenAuth,
        const void* mfiTokenBytes,
        size_t numMFiTokenBytes) {
    HAPPrecondition(mfiTokenAuth);
    HAPPrecondition(mfiTokenBytes);
    HAPPrecondition(numMFiTokenBytes <= kHAPPlatformMFiTokenAuth_MaxMFiTokenBytes);

    HAPError err;

    // Try to find old Software Token.
    bool found;
    err = HAPPlatformKeyValueStoreGet(
            mfiTokenAuth->keyValueStore,
            kSDKKeyValueStoreDomain_Provisioning,
            kSDKKeyValueStoreKey_Provisioning_MFiToken,
            NULL,
            0,
            NULL,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        HAPLogInfo(&logObject, "Trying to update Software Token but no Software Token is present in key-value store.");
        return kHAPError_Unknown;
    }

    // Update Software Token.
    err = HAPPlatformKeyValueStoreSet(
            mfiTokenAuth->keyValueStore,
            kSDKKeyValueStoreDomain_Provisioning,
            kSDKKeyValueStoreKey_Provisioning_MFiToken,
            mfiTokenBytes,
            numMFiTokenBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}
