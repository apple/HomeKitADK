// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "Pairing" };

uint32_t HAPPairingReadFlags(const HAPTLV* tlv) {
    HAPPrecondition(tlv);

    uint32_t value = 0;
    for (size_t i = 0; i < tlv->value.numBytes; i++) {
        const uint8_t* b = tlv->value.bytes;

        if (i < sizeof value) {
            value |= (uint32_t)(b[i] << (i * 8));
        } else if (b[i]) {
            HAPLogBuffer(
                    &logObject,
                    tlv->value.bytes,
                    tlv->value.numBytes,
                    "Ignoring flags after 32 bits in TLV type 0x%02X.",
                    tlv->type);
            break;
        }
    }
    return value;
}

size_t HAPPairingGetNumBytes(uint32_t value) {
    if (value > 0x00FFFFFF) {
        return 4;
    }
    if (value > 0x0000FFFF) {
        return 3;
    }
    if (value > 0x000000FF) {
        return 2;
    }
    if (value > 0x00000000) {
        return 1;
    }
    return 0;
}

typedef struct {
    HAPPairing* pairing;
    HAPPlatformKeyValueStoreKey* key;
    bool* found;
} FindPairingEnumerateContext;

HAP_RESULT_USE_CHECK
static HAPError FindPairingEnumerateCallback(
        void* _Nullable context,
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        bool* shouldContinue) {
    HAPPrecondition(context);
    FindPairingEnumerateContext* arguments = context;
    HAPPrecondition(arguments->pairing);
    HAPPrecondition(arguments->key);
    HAPPrecondition(arguments->found);
    HAPPrecondition(!*arguments->found);
    HAPPrecondition(keyValueStore);
    HAPPrecondition(domain == kHAPKeyValueStoreDomain_Pairings);
    HAPPrecondition(shouldContinue);

    HAPError err;

    // Load pairing.
    bool found;
    size_t numBytes;
    uint8_t pairingBytes[sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
    err = HAPPlatformKeyValueStoreGet(keyValueStore, domain, key, pairingBytes, sizeof pairingBytes, &numBytes, &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPAssert(found);
    if (numBytes != sizeof pairingBytes) {
        HAPLog(&logObject, "Invalid pairing 0x%02X size %lu.", key, (unsigned long) numBytes);
        return kHAPError_Unknown;
    }
    HAPPairing pairing;
    HAPRawBufferZero(&pairing, sizeof pairing);
    HAPAssert(sizeof pairing.identifier.bytes == 36);
    HAPRawBufferCopyBytes(pairing.identifier.bytes, &pairingBytes[0], 36);
    pairing.numIdentifierBytes = pairingBytes[36];
    HAPAssert(sizeof pairing.publicKey.value == 32);
    HAPRawBufferCopyBytes(pairing.publicKey.value, &pairingBytes[37], 32);
    pairing.permissions = pairingBytes[69];

    // Check if pairing found.
    if (pairing.numIdentifierBytes != arguments->pairing->numIdentifierBytes) {
        return kHAPError_None;
    }
    if (!HAPRawBufferAreEqual(
                pairing.identifier.bytes, arguments->pairing->identifier.bytes, pairing.numIdentifierBytes)) {
        return kHAPError_None;
    }

    // Pairing found.
    HAPRawBufferCopyBytes(arguments->pairing, &pairing, sizeof pairing);
    *arguments->key = key;
    *arguments->found = true;
    *shouldContinue = false;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPairingFind(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPairing* pairing,
        HAPPlatformKeyValueStoreKey* key,
        bool* found) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(pairing);
    HAPPrecondition(pairing->numIdentifierBytes <= sizeof pairing->identifier.bytes);
    HAPPrecondition(key);
    HAPPrecondition(found);

    HAPError err;

    *found = false;
    FindPairingEnumerateContext context = { .pairing = pairing, .key = key, .found = found };
    err = HAPPlatformKeyValueStoreEnumerate(
            keyValueStore, kHAPKeyValueStoreDomain_Pairings, FindPairingEnumerateCallback, &context);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    return kHAPError_None;
}
