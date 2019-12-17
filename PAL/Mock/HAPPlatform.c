// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"
#include "HAPPlatform+Init.h"
#include "HAPPlatformAccessorySetup+Init.h"
#include "HAPPlatformBLEPeripheralManager+Init.h"
#include "HAPPlatformKeyValueStore+Init.h"
#include "HAPPlatformMFiHWAuth+Init.h"
#include "HAPPlatformServiceDiscovery+Init.h"
#include "HAPPlatformTCPStreamManager+Init.h"

HAP_RESULT_USE_CHECK
uint32_t HAPPlatformGetCompatibilityVersion(void) {
    return HAP_PLATFORM_COMPATIBILITY_VERSION;
}

HAP_RESULT_USE_CHECK
const char* HAPPlatformGetIdentification(void) {
    return "Test";
}

HAP_RESULT_USE_CHECK
const char* HAPPlatformGetVersion(void) {
    return "Internal";
}

HAP_RESULT_USE_CHECK
const char* HAPPlatformGetBuild(void) {
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdate-time")
    const char* build = __DATE__ " " __TIME__;
    HAP_DIAGNOSTIC_POP

    return build;
}

/**
 * Number of attributes to allow BLE peripheral manager to use.
 */
#define kHAPPlatform_NumBLEPeripheralManagerAttributes ((size_t) 100)

static HAPPlatformKeyValueStore keyValueStore;
static HAPPlatformAccessorySetup accessorySetup;
static HAPPlatformServiceDiscovery serviceDiscovery;
static HAPPlatformTCPStreamManager tcpStreamManager;
static HAPPlatformBLEPeripheralManager blePeripheralManager;
static HAPPlatformMFiHWAuth mfiHWAuth;
HAPPlatform platform = {
    .keyValueStore = &keyValueStore,
    .accessorySetup = &accessorySetup,
    .ip = {
        .tcpStreamManager = &tcpStreamManager,
        .serviceDiscovery = &serviceDiscovery,
    },
    .ble = {
        .blePeripheralManager = &blePeripheralManager
    },
    .authentication = {
        .mfiHWAuth = &mfiHWAuth
    }
};

void HAPPlatformCreate(void) {
    static bool initialized = false;
    HAPPrecondition(!initialized);
    initialized = true;

    // Key-value store.
    static HAPPlatformKeyValueStoreItem keyValueStoreItems[32];
    HAPPlatformKeyValueStoreCreate(
            platform.keyValueStore,
            &(const HAPPlatformKeyValueStoreOptions) { .items = keyValueStoreItems,
                                                       .numItems = HAPArrayCount(keyValueStoreItems) });

    // Accessory setup manager. Does not require initialization.

    // TCP stream manager.
    static HAPPlatformTCPStream tcpStreams[kHAPIPSessionStorage_DefaultNumElements];
    HAPPlatformTCPStreamManagerCreate(
            HAPNonnull(platform.ip.tcpStreamManager),
            &(const HAPPlatformTCPStreamManagerOptions) { .tcpStreams = tcpStreams,
                                                          .numTCPStreams = HAPArrayCount(tcpStreams) });

    // Service discovery.
    HAPPlatformServiceDiscoveryCreate(HAPNonnull(platform.ip.serviceDiscovery));

    // BLE peripheral manager.
    static HAPPlatformBLEPeripheralManagerAttribute attributes[kHAPPlatform_NumBLEPeripheralManagerAttributes];
    HAPPlatformBLEPeripheralManagerCreate(
            HAPNonnull(platform.ble.blePeripheralManager),
            &(const HAPPlatformBLEPeripheralManagerOptions) { .attributes = attributes,
                                                              .numAttributes = HAPArrayCount(attributes) });

    // Apple Authentication Coprocessor provider.
    HAPPlatformMFiHWAuthCreate(HAPNonnull(platform.authentication.mfiHWAuth));
}
