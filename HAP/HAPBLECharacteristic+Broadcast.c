// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLECharacteristic" };

HAP_RESULT_USE_CHECK
bool HAPBLECharacteristicIsValidBroadcastInterval(uint8_t value) {
    switch (value) {
        case kHAPBLECharacteristicBroadcastInterval_20Ms:
        case kHAPBLECharacteristicBroadcastInterval_1280Ms:
        case kHAPBLECharacteristicBroadcastInterval_2560Ms: {
            return true;
        }
        default: {
            return false;
        }
    }
}

typedef struct {
    uint16_t aid;
    bool* found;
    void* bytes;
    size_t maxBytes;
    size_t* numBytes;
    HAPPlatformKeyValueStoreKey* key;
} GetBroadcastParametersEnumerateContext;

HAP_RESULT_USE_CHECK
static HAPError GetBroadcastConfigurationEnumerateCallback(
        void* _Nullable context,
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        bool* shouldContinue) {
    HAPPrecondition(context);
    GetBroadcastParametersEnumerateContext* arguments = context;
    HAPPrecondition(arguments->aid);
    HAPPrecondition(arguments->aid == 1);
    HAPPrecondition(arguments->found);
    HAPPrecondition(!*arguments->found);
    HAPPrecondition(arguments->bytes);
    HAPPrecondition(arguments->maxBytes >= 2);
    HAPPrecondition(arguments->numBytes);
    HAPPrecondition(arguments->key);
    HAPPrecondition(domain == kHAPKeyValueStoreDomain_CharacteristicConfiguration);
    HAPPrecondition(shouldContinue);
    HAPPrecondition(*shouldContinue);

    HAPError err;

    // Load.
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore, domain, key, arguments->bytes, arguments->maxBytes, arguments->numBytes, arguments->found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPAssert(arguments->found);
    if (*arguments->numBytes < 2 || *arguments->numBytes == arguments->maxBytes || (*arguments->numBytes - 2) % 3) {
        HAPLog(&logObject,
               "Invalid characteristic configuration 0x%02X size %lu.",
               key,
               (unsigned long) *arguments->numBytes);
        return kHAPError_Unknown;
    }

    // Check for match.
    if (HAPReadLittleUInt16(arguments->bytes) != arguments->aid) {
        *arguments->found = false;
        return kHAPError_None;
    }

    // Match found.
    *arguments->key = key;
    *shouldContinue = false;
    return kHAPError_None;
}

/**
 * Fetches the characteristic configuration for an accessory.
 *
 * @param      aid                  Accessory ID.
 * @param[out] found                Whether the characteristic configuration has been found.
 * @param[out] bytes                Buffer to store characteristic configuration, if found.
 * @param      maxBytes             Capacity of buffer. Must be at least 2 + 3 * #<concurrent active broadcasts> + 1.
 * @param[out] numBytes             Effective length of buffer, if found.
 * @param[out] key                  Key, if found.
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 */
HAP_RESULT_USE_CHECK
static HAPError GetBroadcastConfiguration(
        uint16_t aid,
        bool* found,
        void* bytes,
        size_t maxBytes,
        size_t* numBytes,
        HAPPlatformKeyValueStoreKey* key,
        HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(aid);
    HAPPrecondition(found);
    HAPPrecondition(bytes);
    HAPPrecondition(maxBytes >= 3);
    HAPPrecondition(numBytes);
    HAPPrecondition(key);
    HAPPrecondition(keyValueStore);

    HAPError err;

    *found = false;
    GetBroadcastParametersEnumerateContext context = {
        .aid = aid, .found = found, .bytes = bytes, .maxBytes = maxBytes, .numBytes = numBytes, .key = key
    };
    err = HAPPlatformKeyValueStoreEnumerate(
            keyValueStore,
            kHAPKeyValueStoreDomain_CharacteristicConfiguration,
            GetBroadcastConfigurationEnumerateCallback,
            &context);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicGetBroadcastConfiguration(
        const HAPCharacteristic* characteristic_,
        const HAPService* service,
        const HAPAccessory* accessory,
        bool* broadcastsEnabled,
        HAPBLECharacteristicBroadcastInterval* broadcastInterval,
        HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(characteristic->properties.ble.supportsBroadcastNotification);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(broadcastsEnabled);
    HAPPrecondition(broadcastInterval);
    HAPPrecondition(keyValueStore);

    HAPError err;

    HAPAssert(accessory->aid == 1);
    uint16_t aid = (uint16_t) accessory->aid;
    HAPAssert(characteristic->iid <= UINT16_MAX);
    uint16_t cid = (uint16_t) characteristic->iid;

    // Get configuration.
    HAPPlatformKeyValueStoreKey key;
    size_t numBytes;
    uint8_t bytes[2 + 3 * 42 + 1]; // 128 + 1, allows for 42 concurrent broadcasts on a single KVS key.
    bool found;
    err = GetBroadcastConfiguration(aid, &found, bytes, sizeof bytes, &numBytes, &key, keyValueStore);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        *broadcastsEnabled = false;
        return kHAPError_None;
    }
    HAPAssert(numBytes >= 2 && !((numBytes - 2) % 3));
    HAPAssert(HAPReadLittleUInt16(bytes) == aid);

    // Find characteristic.
    for (size_t i = 2; i < numBytes; i += 3) {
        uint16_t itemCID = HAPReadLittleUInt16(&bytes[i]);
        if (itemCID < cid) {
            continue;
        }
        if (itemCID > cid) {
            break;
        }

        // Found. Extract configuration.
        uint8_t broadcastConfiguration = bytes[i + 2];
        if (!HAPBLECharacteristicIsValidBroadcastInterval(broadcastConfiguration)) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Invalid stored broadcast interval: 0x%02x.",
                    broadcastConfiguration);
            return kHAPError_Unknown;
        }
        *broadcastsEnabled = true;
        *broadcastInterval = (HAPBLECharacteristicBroadcastInterval) broadcastConfiguration;
        return kHAPError_None;
    }

    // Not found.
    *broadcastsEnabled = false;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicEnableBroadcastNotifications(
        const HAPCharacteristic* characteristic_,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPBLECharacteristicBroadcastInterval broadcastInterval,
        HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(characteristic->properties.ble.supportsBroadcastNotification);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(HAPBLECharacteristicIsValidBroadcastInterval(broadcastInterval));
    HAPPrecondition(keyValueStore);

    HAPError err;

    HAPLogCharacteristicInfo(
            &logObject,
            characteristic,
            service,
            accessory,
            "Enabling broadcasts (interval = 0x%02x).",
            broadcastInterval);

    HAPAssert(accessory->aid == 1);
    uint16_t aid = (uint16_t) accessory->aid;
    HAPAssert(characteristic->iid <= UINT16_MAX);
    uint16_t cid = (uint16_t) characteristic->iid;

    // Get configuration.
    HAPPlatformKeyValueStoreKey key;
    size_t numBytes;
    uint8_t bytes[2 + 3 * 42 + 1]; // 128 + 1, allows for 42 concurrent broadcasts on a single KVS key.
    bool found;
    err = GetBroadcastConfiguration(aid, &found, bytes, sizeof bytes, &numBytes, &key, keyValueStore);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        key = 0;
        HAPWriteLittleUInt16(bytes, aid);
        numBytes = 2;
    }
    HAPAssert(numBytes >= 2 && !((numBytes - 2) % 3));
    HAPAssert(HAPReadLittleUInt16(bytes) == aid);

    // Find characteristic.
    size_t i;
    for (i = 2; i < numBytes; i += 3) {
        uint16_t itemCID = HAPReadLittleUInt16(&bytes[i]);
        if (itemCID < cid) {
            continue;
        }
        if (itemCID > cid) {
            break;
        }

        // Found. Extract configuration.
        uint8_t broadcastConfiguration = bytes[i + 2];
        if (!HAPBLECharacteristicIsValidBroadcastInterval(broadcastConfiguration)) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Invalid stored broadcast interval: 0x%02x.",
                    broadcastConfiguration);
            return kHAPError_Unknown;
        }

        // Update configuration.
        if ((HAPBLECharacteristicBroadcastInterval) broadcastConfiguration == broadcastInterval) {
            return kHAPError_None;
        }
        bytes[i + 2] = broadcastInterval;
        err = HAPPlatformKeyValueStoreSet(
                keyValueStore, kHAPKeyValueStoreDomain_CharacteristicConfiguration, key, bytes, numBytes);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
        return kHAPError_None;
    }

    // Add configuration.
    if (numBytes >= sizeof bytes - 1 - 3) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Not enough space to store characteristic configuration.");
        return kHAPError_Unknown;
    }
    HAPRawBufferCopyBytes(&bytes[i + 3], &bytes[i], numBytes - i);
    HAPWriteLittleUInt16(&bytes[i], cid);
    bytes[i + 2] = broadcastInterval;
    numBytes += 3;
    err = HAPPlatformKeyValueStoreSet(
            keyValueStore, kHAPKeyValueStoreDomain_CharacteristicConfiguration, key, bytes, numBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicDisableBroadcastNotifications(
        const HAPCharacteristic* characteristic_,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(characteristic->properties.ble.supportsBroadcastNotification);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(keyValueStore);

    HAPError err;

    HAPLogCharacteristicInfo(&logObject, characteristic, service, accessory, "Disabling broadcasts.");

    HAPAssert(accessory->aid == 1);
    uint16_t aid = (uint16_t) accessory->aid;
    HAPAssert(characteristic->iid <= UINT16_MAX);
    uint16_t cid = (uint16_t) characteristic->iid;

    // Get configuration.
    HAPPlatformKeyValueStoreKey key;
    size_t numBytes;
    uint8_t bytes[2 + 3 * 42 + 1]; // 128 + 1, allows for 42 concurrent broadcasts on a single KVS key.
    bool found;
    err = GetBroadcastConfiguration(aid, &found, bytes, sizeof bytes, &numBytes, &key, keyValueStore);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        return kHAPError_None;
    }
    HAPAssert(numBytes >= 2 && !((numBytes - 2) % 3));
    HAPAssert(HAPReadLittleUInt16(bytes) == aid);

    // Find characteristic.
    size_t i;
    for (i = 2; i < numBytes; i += 3) {
        uint16_t itemCID = HAPReadLittleUInt16(&bytes[i]);
        if (itemCID < cid) {
            continue;
        }
        if (itemCID > cid) {
            break;
        }

        // Found. Extract configuration.
        uint8_t broadcastConfiguration = bytes[i + 2];
        if (!HAPBLECharacteristicIsValidBroadcastInterval(broadcastConfiguration)) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Invalid stored broadcast interval: 0x%02x.",
                    broadcastConfiguration);
            return kHAPError_Unknown;
        }

        // Remove configuration.
        numBytes -= 3;
        HAPRawBufferCopyBytes(&bytes[i], &bytes[i + 3], numBytes - i);
        if (numBytes == 2) {
            err = HAPPlatformKeyValueStoreRemove(
                    keyValueStore, kHAPKeyValueStoreDomain_CharacteristicConfiguration, key);
        } else {
            err = HAPPlatformKeyValueStoreSet(
                    keyValueStore, kHAPKeyValueStoreDomain_CharacteristicConfiguration, key, bytes, numBytes);
        }
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
        return kHAPError_None;
    }
    return kHAPError_None;
}
