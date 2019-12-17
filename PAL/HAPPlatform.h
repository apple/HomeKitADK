// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_H
#define HAP_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPBase.h"

#include "HAPPlatformAbort.h"
#include "HAPPlatformAccessorySetup.h"
#include "HAPPlatformAccessorySetupDisplay.h"
#include "HAPPlatformAccessorySetupNFC.h"
#include "HAPPlatformBLEPeripheralManager.h"
#include "HAPPlatformClock.h"
#include "HAPPlatformKeyValueStore.h"
#include "HAPPlatformLog.h"
#include "HAPPlatformMFiHWAuth.h"
#include "HAPPlatformMFiTokenAuth.h"
#include "HAPPlatformRandomNumber.h"
#include "HAPPlatformRunLoop.h"
#include "HAPPlatformServiceDiscovery.h"
#include "HAPPlatformTCPStreamManager.h"
#include "HAPPlatformTimer.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Log subsystem used by the HAP platform implementation.
 */
#define kHAPPlatform_LogSubsystem "com.apple.mfi.HomeKit.Platform"

/**
 * Compatibility version of the HAP platform interface.
 *
 * - If this version differs from the one returned by HAPPlatformGetCompatibilityVersion,
 *   the HAP platform implementation is incompatible and must not be used.
 */
#define HAP_PLATFORM_COMPATIBILITY_VERSION (7)

/**
 * Gets the compatibility version of the HAP platform implementation.
 *
 * - If the compatibility version differs from HAP_PLATFORM_COMPATIBILITY_VERSION,
 *   the HAP platform implementation is incompatible and may not be used.
 *
 * @return Compatibility version of the HAP library.
 */
HAP_RESULT_USE_CHECK
uint32_t HAPPlatformGetCompatibilityVersion(void);

/**
 * Gets the identification of the HAP platform implementation.
 *
 * @return HAP platform implementation identification string.
 */
HAP_RESULT_USE_CHECK
const char* HAPPlatformGetIdentification(void);

/**
 * Gets the version string of the HAP platform implementation.
 *
 * @return Version string of the HAP platform implementation.
 */
HAP_RESULT_USE_CHECK
const char* HAPPlatformGetVersion(void);

/**
 * Gets the build version string of the HAP platform implementation.
 *
 * @return Build version string of the HAP platform implementation.
 */
HAP_RESULT_USE_CHECK
const char* HAPPlatformGetBuild(void);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
