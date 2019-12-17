// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "UUID" };

HAP_RESULT_USE_CHECK
bool HAPUUIDAreEqual(const HAPUUID* uuid, const HAPUUID* otherUUID) {
    HAPPrecondition(uuid);
    HAPPrecondition(otherUUID);

    return HAPRawBufferAreEqual(uuid->bytes, otherUUID->bytes, sizeof uuid->bytes);
}

HAP_RESULT_USE_CHECK
bool HAPUUIDIsAppleDefined(const HAPUUID* uuid) {
    HAPPrecondition(uuid);

    // See HomeKit Accessory Protocol Specification R14
    // Section 6.6.1 Service and Characteristic Types
    static const uint8_t hapBase[] = { 0x91, 0x52, 0x76, 0xBB, 0x26, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00 };
    return HAPRawBufferAreEqual(uuid->bytes, hapBase, sizeof hapBase) != 0;
}

HAP_RESULT_USE_CHECK
size_t HAPUUIDGetNumDescriptionBytes(const HAPUUID* uuid) {
    HAPPrecondition(uuid);
    HAPAssert(sizeof uuid->bytes == 16);

    size_t numBytes;
    if (HAPUUIDIsAppleDefined(uuid)) {
        numBytes = 8;
        if (uuid->bytes[15] == 0) {
            numBytes -= 2;
            if (uuid->bytes[14] == 0) {
                numBytes -= 2;
                if (uuid->bytes[13] == 0) {
                    numBytes -= 2;
                    if (uuid->bytes[12] < 0x10) {
                        numBytes -= 1;
                    }
                } else if (uuid->bytes[13] < 0x10) {
                    numBytes -= 1;
                }
            } else if (uuid->bytes[14] < 0x10) {
                numBytes -= 1;
            }
        } else if (uuid->bytes[15] < 0x10) {
            numBytes -= 1;
        }
    } else {
        numBytes = 36;
    }
    return numBytes;
}

HAP_RESULT_USE_CHECK
HAPError HAPUUIDGetDescription(const HAPUUID* uuid, char* bytes, size_t maxBytes) {
    HAPPrecondition(uuid);
    HAPPrecondition(bytes);
    HAPAssert(sizeof uuid->bytes == 16);

    HAPError err;

    if (HAPUUIDIsAppleDefined(uuid)) {
        if (uuid->bytes[15] != 0) {
            err = HAPStringWithFormat(
                    bytes,
                    maxBytes,
                    "%X%02X%02X%02X",
                    uuid->bytes[15],
                    uuid->bytes[14],
                    uuid->bytes[13],
                    uuid->bytes[12]);
        } else if (uuid->bytes[14] != 0) {
            err = HAPStringWithFormat(bytes, maxBytes, "%X%02X%02X", uuid->bytes[14], uuid->bytes[13], uuid->bytes[12]);
        } else if (uuid->bytes[13] != 0) {
            err = HAPStringWithFormat(bytes, maxBytes, "%X%02X", uuid->bytes[13], uuid->bytes[12]);
        } else {
            err = HAPStringWithFormat(bytes, maxBytes, "%X", uuid->bytes[12]);
        }
    } else {
        err = HAPStringWithFormat(
                bytes,
                maxBytes,
                "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                uuid->bytes[15],
                uuid->bytes[14],
                uuid->bytes[13],
                uuid->bytes[12],
                uuid->bytes[11],
                uuid->bytes[10],
                uuid->bytes[9],
                uuid->bytes[8],
                uuid->bytes[7],
                uuid->bytes[6],
                uuid->bytes[5],
                uuid->bytes[4],
                uuid->bytes[3],
                uuid->bytes[2],
                uuid->bytes[1],
                uuid->bytes[0]);
    }
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPUUIDGetShortFormBytes(const HAPUUID* uuid, void* bytes_, size_t maxBytes, size_t* numBytes) {
    HAPPrecondition(uuid);
    HAPPrecondition(bytes_);
    uint8_t* bytes = bytes_;
    HAPPrecondition(numBytes);
    HAPAssert(sizeof uuid->bytes == 16);

    if (HAPUUIDIsAppleDefined(uuid)) {
        if (uuid->bytes[15] != 0) {
            *numBytes = 4;
        } else if (uuid->bytes[14] != 0) {
            *numBytes = 3;
        } else if (uuid->bytes[13] != 0) {
            *numBytes = 2;
        } else if (uuid->bytes[12] != 0) {
            *numBytes = 1;
        } else {
            *numBytes = 0;
        }

        if (maxBytes < *numBytes) {
            HAPLog(&logObject, "Not enough resources to serialize compact UUID (%zu bytes needed).", *numBytes);
            return kHAPError_OutOfResources;
        }
        HAPAssert(*numBytes <= sizeof uuid->bytes - 12);
        HAPRawBufferCopyBytes(bytes, &uuid->bytes[12], *numBytes);
    } else {
        *numBytes = sizeof uuid->bytes;

        if (maxBytes < *numBytes) {
            HAPLog(&logObject, "Not enough resources to serialize compact UUID (%zu bytes needed).", *numBytes);
            return kHAPError_OutOfResources;
        }
        HAPRawBufferCopyBytes(bytes, uuid->bytes, *numBytes);
    }
    return kHAPError_None;
}
