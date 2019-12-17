// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatformKeyValueStore+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "KeyValueStore" };

void HAPPlatformKeyValueStoreCreate(
        HAPPlatformKeyValueStoreRef keyValueStore,
        const HAPPlatformKeyValueStoreOptions* options) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(options);
    HAPPrecondition(options->items);

    keyValueStore->bytes = options->items;
    keyValueStore->maxBytes = options->numItems;
    HAPRawBufferZero(keyValueStore->bytes, sizeof keyValueStore->bytes[0] * keyValueStore->maxBytes);
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreGet(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        void* _Nullable const bytes,
        size_t maxBytes,
        size_t* _Nullable numBytes,
        bool* found) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(!maxBytes || bytes);
    HAPPrecondition((bytes == NULL) == (numBytes == NULL));
    HAPPrecondition(found);

    *found = false;
    for (size_t i = 0; i < keyValueStore->maxBytes; i++) {
        if (!keyValueStore->bytes[i].active) {
            continue;
        }
        if (keyValueStore->bytes[i].domain != domain || keyValueStore->bytes[i].key != key) {
            continue;
        }

        *found = true;
        if (bytes) {
            HAPAssert(keyValueStore->bytes[i].numBytes <= sizeof keyValueStore->bytes[i].bytes);
            *numBytes = keyValueStore->bytes[i].numBytes < maxBytes ? keyValueStore->bytes[i].numBytes : maxBytes;
            HAPRawBufferCopyBytes(bytes, keyValueStore->bytes[i].bytes, *numBytes);
            HAPLogBufferDebug(&logObject, bytes, *numBytes, "Read %02X.%02X", domain, key);
        } else {
            HAPLogDebug(&logObject, "Found %02X.%02X", domain, key);
        }

        return kHAPError_None;
    }
    HAPLogDebug(&logObject, "Read %02X.%02X (not found)", domain, key);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreSet(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        const void* bytes,
        size_t numBytes) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(bytes);

    HAPLogBufferDebug(&logObject, bytes, numBytes, "Write %02X.%02X", domain, key);

    size_t index = 0;
    bool found = false;
    size_t free_index = 0;
    bool free_found = false;
    for (size_t i = 0; i < keyValueStore->maxBytes; i++) {
        if (keyValueStore->bytes[i].active && keyValueStore->bytes[i].domain == domain &&
            keyValueStore->bytes[i].key == key) {
            index = i;
            found = true;
            break;
        }
        if (!keyValueStore->bytes[i].active) {
            free_index = i;
            free_found = true;
        }
    }
    if (!found && free_found) {
        index = free_index;
        found = true;
    }

    // Update.
    if (!found) {
        HAPLog(&logObject, "No free entry to store value for domain 0x%X / key 0x%04X.", domain, key);
        return kHAPError_Unknown;
    }

    if (numBytes > sizeof keyValueStore->bytes[index].bytes) {
        HAPLog(&logObject,
               "Not enough memory to store value with length %lu for domain 0x%X / key 0x%04X.",
               (unsigned long) numBytes,
               domain,
               key);
        return kHAPError_Unknown;
    }
    keyValueStore->bytes[index].active = true;
    keyValueStore->bytes[index].domain = domain;
    keyValueStore->bytes[index].key = key;
    keyValueStore->bytes[index].numBytes = numBytes;
    if (numBytes) {
        HAPRawBufferCopyBytes(keyValueStore->bytes[index].bytes, bytes, numBytes);
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreRemove(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key) {
    HAPPrecondition(keyValueStore);

    for (size_t i = 0; i < keyValueStore->maxBytes; i++) {
        if (!keyValueStore->bytes[i].active) {
            continue;
        }
        if (keyValueStore->bytes[i].domain == domain && keyValueStore->bytes[i].key == key) {
            keyValueStore->bytes[i].active = false;
            return kHAPError_None;
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreEnumerate(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreEnumerateCallback callback,
        void* _Nullable context) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(callback);

    HAPError err;

    bool cont = true;
    for (size_t i = 0; cont && i < keyValueStore->maxBytes; i++) {
        if (!keyValueStore->bytes[i].active) {
            continue;
        }
        if (keyValueStore->bytes[i].domain != domain) {
            continue;
        }

        HAPAssert(keyValueStore->bytes[i].numBytes <= sizeof keyValueStore->bytes[i].bytes);
        err = callback(context, keyValueStore, keyValueStore->bytes[i].domain, keyValueStore->bytes[i].key, &cont);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStorePurgeDomain(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain) {
    HAPPrecondition(keyValueStore);

    for (size_t i = 0; i < keyValueStore->maxBytes; i++) {
        if (!keyValueStore->bytes[i].active) {
            continue;
        }
        if (keyValueStore->bytes[i].domain != domain) {
            continue;
        }
        keyValueStore->bytes[i].active = false;
    }
    return kHAPError_None;
}
