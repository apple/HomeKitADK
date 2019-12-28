// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#import <Foundation/Foundation.h>

#include "HAPPlatformKeyValueStore+Init.h"

#include <stdlib.h>

static const HAPLogObject kvs_log = { .subsystem = kHAPPlatform_LogSubsystem, .category = "KeyValueStore" };

static NSMutableDictionary* KeyValueStore = NULL;

void HAPPlatformKeyValueStoreCreate(
        HAPPlatformKeyValueStoreRef keyValueStore,
        const HAPPlatformKeyValueStoreOptions* options) {
    HAPPrecondition(options->rootDirectory);
    HAPPrecondition(keyValueStore);

    keyValueStore->rootDirectory = options->rootDirectory;
    HAPLog(&kvs_log, "Storage location: %s", keyValueStore->rootDirectory);

    NSError* error;
    NSData* data = [NSData dataWithContentsOfFile:@(keyValueStore->rootDirectory) options:0 error:&error];
    KeyValueStore = [NSKeyedUnarchiver unarchivedObjectOfClasses:[NSSet setWithArray:@[[NSMutableDictionary class]]]
                                                        fromData:data
                                                           error:&error];
    if (!KeyValueStore) {
        KeyValueStore = [[NSMutableDictionary alloc] init];
    }
}

static NSData* AsDictionaryKey(HAPPlatformKeyValueStoreDomain domain, HAPPlatformKeyValueStoreKey key) {
    uint8_t domainAndKey[] = { domain, key };
    return [NSData dataWithBytes:domainAndKey length:sizeof(domainAndKey)];
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreGet(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        void* _Nullable bytes,
        size_t maxBytes,
        size_t* _Nullable numBytes,
        bool* found) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(!maxBytes || bytes);
    HAPPrecondition((bytes == NULL) == (numBytes == NULL));
    HAPPrecondition(found);

    id value = KeyValueStore[AsDictionaryKey(domain, key)];
    if (!value || ![value isKindOfClass:[NSData class]]) {
        *found = false;
        return kHAPError_None;
    }
    NSData* data = (NSData*) value;
    size_t n = maxBytes;
    if (data.length < n)
        n = data.length;
    memcpy(bytes, data.bytes, n);
    *numBytes = n;
    *found = true;
    return kHAPError_None;
}

static HAPError Sync(const char* rootDirectory) {
    NSError* error;
    NSData* data = [NSKeyedArchiver archivedDataWithRootObject:KeyValueStore requiringSecureCoding:YES error:&error];
    if (!data) {
        HAPLogError(&kvs_log, "Serializing data with NSKeyedArchiver failed");
        return kHAPError_Unknown;
    }
    [data writeToFile:@(rootDirectory) atomically:YES];

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

    KeyValueStore[AsDictionaryKey(domain, key)] = [NSData dataWithBytes:bytes length:numBytes];

    return Sync(keyValueStore->rootDirectory);
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreRemove(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key) {
    HAPPrecondition(keyValueStore);

    [KeyValueStore removeObjectForKey:AsDictionaryKey(domain, key)];

    return Sync(keyValueStore->rootDirectory);
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreEnumerate(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreEnumerateCallback callback,
        void* _Nullable context) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(callback);

    HAPError err = kHAPError_None;
    for (NSData* key in KeyValueStore) {
        bool shouldContinue = true;
        HAPAssert(key.length == 2);
        const uint8_t* domainAndKey = (const uint8_t*) key.bytes;
        if (domainAndKey[0] != domain) {
            continue;
        }
        err = callback(context, keyValueStore, domainAndKey[0], domainAndKey[1], &shouldContinue);
        if (!shouldContinue) {
            break;
        }
    }

    return err;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStorePurgeDomain(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain) {
    HAPPrecondition(keyValueStore);

    NSMutableDictionary* newDict = [[NSMutableDictionary alloc] init];
    for (NSData* key in KeyValueStore) {
        HAPAssert(key.length == 2);
        const uint8_t* domainAndKey = (const uint8_t*) key.bytes;
        if (domainAndKey[0] != domain) {
            newDict[key] = KeyValueStore[key];
        }
    }
    KeyValueStore = newDict;

    return Sync(keyValueStore->rootDirectory);
}
