// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_KEY_VALUE_STORE_SDK_DOMAINS_H
#define HAP_PLATFORM_KEY_VALUE_STORE_SDK_DOMAINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

/**
 * Statically provisioned data.
 *
 * Purged: Never.
 */
#define kSDKKeyValueStoreDomain_Provisioning ((HAPPlatformKeyValueStoreDomain) 0x40)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Static setup code SRP salt & verifier.
 *
 * Format: HAPSetupInfo.
 */
#define kSDKKeyValueStoreKey_Provisioning_SetupInfo ((HAPPlatformKeyValueStoreKey) 0x10)

/**
 * Setup ID.
 *
 * Format: HAPSetupID.
 */
#define kSDKKeyValueStoreKey_Provisioning_SetupID ((HAPPlatformKeyValueStoreKey) 0x11)

/**
 * Setup code for NFC.
 *
 * Format: HAPSetupCode.
 */
#define kSDKKeyValueStoreKey_Provisioning_SetupCode ((HAPPlatformKeyValueStoreKey) 0x12)

/**
 * Software Token UUID.
 *
 * Format: HAPPlatformMFiTokenAuthUUID.
 */
#define kSDKKeyValueStoreKey_Provisioning_MFiTokenUUID ((HAPPlatformKeyValueStoreKey) 0x20)

/**
 * Software Token.
 *
 * Format: Opaque blob, up to kHAPPlatformMFiTokenAuth_MaxMFiTokenBytes bytes.
 */
#define kSDKKeyValueStoreKey_Provisioning_MFiToken ((HAPPlatformKeyValueStoreKey) 0x21)

#ifdef __cplusplus
}
#endif

#endif
