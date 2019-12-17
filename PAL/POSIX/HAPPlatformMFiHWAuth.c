// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "HAPPlatformMFiHWAuth+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "MFiHWAuth" };

// MFi I2C Driver for Raspberry Pi

// -------------------------------------------------
// To enable I2C on the Raspberry Pi:
// enable I2C in raspi-config
// or
// add "i2c-dev" to /etc/modules
// add "dtparam=i2c_arm=on" to /boot/config.txt
// -------------------------------------------------

// See Accessory Interface Specification R30
// Section 64.5.3 Addressing
#define I2C_ADDRESS ((uint8_t) 0x10) // 7 bit address

// Coprocessor 2.0C Address Selection
//
// RST State | I2C write address | I2C read address
// ------------------------------------------------
// 0         | 0x20              | 0x21
// 1         | 0x22              | 0x23
// -------------------------------------------------

// Raspberry-Pi I2C Port
#define kHAPPlatformMFiHWAuth_I2CPort "/dev/i2c-1"

void HAPPlatformMFiHWAuthCreate(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);

    HAPLogDebug(&logObject, "%s", __func__);

    HAPLogDebug(&logObject, "Storage configuration: mfiHWAuth = %lu", (unsigned long) sizeof *mfiHWAuth);

    do {
        mfiHWAuth->i2cFile = open(kHAPPlatformMFiHWAuth_I2CPort, O_RDWR);
    } while (mfiHWAuth->i2cFile == -1 && errno == EINTR);
    if (mfiHWAuth->i2cFile < 0) {
        int _errno = errno;
        HAPAssert(mfiHWAuth->i2cFile == -1);
        HAPLogError(
                &logObject,
                "open %s failed: %d - i2c-dev installed and enabled?",
                kHAPPlatformMFiHWAuth_I2CPort,
                _errno);
        HAPFatalError();
    }

    int e = ioctl(mfiHWAuth->i2cFile, I2C_SLAVE, I2C_ADDRESS);
    if (e < 0) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPLogError(&logObject, "i2c address set failed on %s: %d.", kHAPPlatformMFiHWAuth_I2CPort, _errno);
        HAPFatalError();
    }
}

void HAPPlatformMFiHWAuthRelease(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);
    HAPPrecondition(mfiHWAuth->i2cFile > -1);

    HAPLogDebug(&logObject, "%s", __func__);

    (void) close(mfiHWAuth->i2cFile);
    mfiHWAuth->i2cFile = 0;
}

HAP_RESULT_USE_CHECK
bool HAPPlatformMFiHWAuthIsPoweredOn(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);

    return mfiHWAuth->enabled;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiHWAuthPowerOn(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);

    mfiHWAuth->enabled = true;
    return kHAPError_None;
}

void HAPPlatformMFiHWAuthPowerOff(HAPPlatformMFiHWAuthRef mfiHWAuth) {
    HAPPrecondition(mfiHWAuth);
    HAPPrecondition(mfiHWAuth->enabled);

    mfiHWAuth->enabled = false;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiHWAuthWrite(HAPPlatformMFiHWAuthRef mfiHWAuth, const void* bytes, size_t numBytes) {
    HAPPrecondition(mfiHWAuth);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    HAPLogBufferDebug(&logObject, bytes, numBytes, "MFi >");
    int repeat = 1000;
    while (--repeat >= 0) {
        ssize_t n = write(mfiHWAuth->i2cFile, bytes, numBytes);
        if (n == (ssize_t) numBytes) {
            HAPLogDebug(&logObject, "MFi write complete.");
            return kHAPError_None;
        }
        (void) usleep(500);
    }
    HAPLog(&logObject, "I2C write timed out.");
    return kHAPError_Unknown;
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

    HAPLogDebug(&logObject, "MFi read 0x%02x.", registerAddress);

    int repeat = 1000;

    // Send register ID to read.
    while (--repeat >= 0) {
        ssize_t n = write(mfiHWAuth->i2cFile, &registerAddress, 1);
        if (n == 1) {
            break;
        }
        (void) usleep(500);
    }

    // Send read request.
    while (--repeat >= 0) {
        ssize_t n = read(mfiHWAuth->i2cFile, bytes, numBytes);
        if (n == (ssize_t) numBytes) {
            HAPLogBufferDebug(&logObject, bytes, numBytes, "MFi < %02x", registerAddress);
            return kHAPError_None;
        }
        (void) usleep(500);
    }

    HAPLog(&logObject, "I2C read timed out.");
    return kHAPError_Unknown;
}
