// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"
#include "HAPPlatformMFiHWAuth+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "MFiHWAuth" };

void HAPPlatformMFiHWAuthCreate(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);

    HAPRawBufferZero(mfiHWAuth, sizeof *mfiHWAuth);
}

void HAPPlatformMFiHWAuthRelease(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);
}

HAP_RESULT_USE_CHECK
bool HAPPlatformMFiHWAuthIsPoweredOn(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);

    return mfiHWAuth->poweredOn;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiHWAuthPowerOn(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);

    mfiHWAuth->poweredOn = true;
    return kHAPError_None;
}

void HAPPlatformMFiHWAuthPowerOff(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);

    mfiHWAuth->poweredOn = false;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiHWAuthWrite(HAPPlatformMFiHWAuthRef mfiHWAuth, const void* bytes, size_t numBytes) {
    HAPPrecondition(mfiHWAuth);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes >= 1 && numBytes <= 128);

    const uint8_t* b = bytes;
    HAPLogBufferDebug(&logObject, &b[1], numBytes - 1, "MFi > %02x", b[0]);
    switch (b[0]) {
        case kHAPMFiHWAuthRegister_SelfTestStatus: {
            if (b[1] & 1) {
                HAPLogInfo(&logObject, "Run X.509 certificate and private key tests.");
            }
            return kHAPError_None;
        }
        default: {
            HAPLog(&logObject, "Unknown register.");
            return kHAPError_Unknown;
        }
    }
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiHWAuthRead(
        HAPPlatformMFiHWAuthRef mfiHWAuth,
        uint8_t registerAddress,
        void* bytes,
        size_t numBytes) {
    HAPPrecondition(mfiHWAuth);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes >= 1 && numBytes <= 128);

    uint8_t* b = bytes;
    size_t o = 0;
    switch (registerAddress) {
        case kHAPMFiHWAuthRegister_DeviceVersion: {
            HAPPrecondition(numBytes == 1);
            b[o++] = kHAPMFiHWAuthDeviceVersion_3_0;
            HAPAssert(o == numBytes);
            HAPLogBufferDebug(&logObject, bytes, numBytes, "MFi < %02x", registerAddress);
            return kHAPError_None;
        }
        case kHAPMFiHWAuthRegister_AuthenticationRevision: {
            HAPPrecondition(numBytes == 1);
            b[o++] = 1;
            HAPAssert(o == numBytes);
            HAPLogBufferDebug(&logObject, bytes, numBytes, "MFi < %02x", registerAddress);
            return kHAPError_None;
        }
        case kHAPMFiHWAuthRegister_AuthenticationProtocolMajorVersion: {
            HAPPrecondition(numBytes == 1);
            b[o++] = 3;
            HAPAssert(o == numBytes);
            HAPLogBufferDebug(&logObject, bytes, numBytes, "MFi < %02x", registerAddress);
            return kHAPError_None;
        }
        case kHAPMFiHWAuthRegister_AuthenticationProtocolMinorVersion: {
            HAPPrecondition(numBytes == 1);
            b[o++] = 0;
            HAPAssert(o == numBytes);
            HAPLogBufferDebug(&logObject, bytes, numBytes, "MFi < %02x", registerAddress);
            return kHAPError_None;
        }
        case kHAPMFiHWAuthRegister_ErrorCode: {
            HAPPrecondition(numBytes == 1);
            b[o++] = kHAPMFiHWAuthError_NoError;
            HAPAssert(o == numBytes);
            HAPLogBufferDebug(&logObject, bytes, numBytes, "MFi < %02x", registerAddress);
            return kHAPError_None;
        }
        case kHAPMFiHWAuthRegister_SelfTestStatus: {
            HAPPrecondition(numBytes == 1);
            // Fake self-test results.
            b[o++] = (1 << 7) | (1 << 6);
            HAPAssert(o == numBytes);
            HAPLogBufferDebug(&logObject, bytes, numBytes, "MFi < %02x", registerAddress);
            return kHAPError_None;
        }
        default: {
            HAPLog(&logObject, "MFi < %02x (unexpected register)", registerAddress);
            return kHAPError_Unknown;
        }
    }
}
