// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_KEY_VALUE_STORE_H
#define HAP_PLATFORM_KEY_VALUE_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Key-value store.
 */
typedef struct HAPPlatformKeyValueStore HAPPlatformKeyValueStore;
typedef struct HAPPlatformKeyValueStore* HAPPlatformKeyValueStoreRef;
HAP_NONNULL_SUPPORT(HAPPlatformKeyValueStore)

/**
 * Domain.
 *
 * Domain ownership:
 * - 0x00-0x3F - Accessory manufacturer.
 * - 0x40-0x7F - SDK developer.
 * - 0x80-0xFF - Reserved for core implementation.
 */
typedef uint8_t HAPPlatformKeyValueStoreDomain;

/**
 * Key. Semantics depend on domain.
 */
typedef uint8_t HAPPlatformKeyValueStoreKey;

/**
 * Fetches the value of a key in a domain.
 *
 * @param      keyValueStore        Key-value store.
 * @param      domain               Domain to search.
 * @param      key                  Key to fetch value of.
 * @param[out] bytes                On output, value of key, if found, truncated up to maxBytes bytes.
 * @param      maxBytes             Capacity of bytes buffer.
 * @param[out] numBytes             Effective length of bytes buffer, if found.
 * @param[out] found                Whether or not a key with a value has been found.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreGet(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        void* _Nullable bytes,
        size_t maxBytes,
        size_t* _Nullable numBytes,
        bool* found);

/**
 * Sets the value of a key in a domain to the contents of a buffer.
 *
 * @param      keyValueStore        Key-value store.
 * @param      domain               Domain to modify.
 * @param      key                  Key to modify.
 * @param      bytes                Buffer that contains the value to set.
 * @param      numBytes             Length of bytes buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreSet(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        const void* bytes,
        size_t numBytes);

/**
 * Removes the value of a key in a domain.
 *
 * @param      keyValueStore        Key-value store.
 * @param      domain               Domain to modify.
 * @param      key                  Key to remove.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreRemove(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key);

/**
 * Callback that should be invoked for each key-value store entry.
 *
 * @param      context              Context.
 * @param      keyValueStore        Key-value store.
 * @param      domain               Domain of key-value store entry.
 * @param      key                  Key of key-value store entry.
 * @param[in,out] shouldContinue    True if enumeration shall continue, False otherwise. Is set to true on input.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
typedef HAPError (*HAPPlatformKeyValueStoreEnumerateCallback)(
        void* _Nullable context,
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        bool* shouldContinue);

/**
 * Enumerates keys in a domain.
 *
 * @param      keyValueStore        Key-value store.
 * @param      domain               Domain to enumerate.
 * @param      callback             Function to call on each key-value store entry.
 * @param      context              Context that is passed to the callback.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreEnumerate(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreEnumerateCallback callback,
        void* _Nullable context);

/**
 * Removes values of all keys in a domain.
 *
 * @param      keyValueStore        Key-value store.
 * @param      domain               Domain to purge.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStorePurgeDomain(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
