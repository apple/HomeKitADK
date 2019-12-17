// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatformServiceDiscovery+Init.h"
#include "HAPPlatformServiceDiscovery+Test.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "ServiceDiscovery" };

void HAPPlatformServiceDiscoveryCreate(HAPPlatformServiceDiscoveryRef serviceDiscovery) {
    HAPPrecondition(serviceDiscovery);

    HAPRawBufferZero(serviceDiscovery, sizeof *serviceDiscovery);
}

void HAPPlatformServiceDiscoveryRegister(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        const char* name,
        const char* protocol,
        HAPNetworkPort port,
        HAPPlatformServiceDiscoveryTXTRecord* txtRecords,
        size_t numTXTRecords) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(!HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));
    HAPPrecondition(name);
    HAPPrecondition(protocol);
    HAPPrecondition(port);
    HAPPrecondition(txtRecords);

    HAPLog(&logObject, "%s - %s.%s @ %u.", __func__, name, protocol, port);

    // Copy name.
    HAPPrecondition(HAPStringGetNumBytes(name) < sizeof serviceDiscovery->name);
    HAPRawBufferCopyBytes(serviceDiscovery->name, name, HAPStringGetNumBytes(name) + 1);

    // Copy protocol.
    HAPPrecondition(HAPStringGetNumBytes(protocol) < sizeof serviceDiscovery->protocol);
    HAPRawBufferCopyBytes(serviceDiscovery->protocol, protocol, HAPStringGetNumBytes(protocol) + 1);

    // Copy port.
    serviceDiscovery->port = port;

    // Copy TXT records.
    HAPPrecondition(numTXTRecords < HAPArrayCount(serviceDiscovery->txtRecords));
    for (size_t i = 0; i < numTXTRecords; i++) {
        HAPLogBuffer(&logObject, txtRecords[i].value.bytes, txtRecords[i].value.numBytes, "%s", txtRecords[i].key);

        // Copy key.
        HAPPrecondition(HAPStringGetNumBytes(txtRecords[i].key) < sizeof serviceDiscovery->txtRecords[i].key);
        HAPRawBufferCopyBytes(
                serviceDiscovery->txtRecords[i].key, txtRecords[i].key, HAPStringGetNumBytes(txtRecords[i].key) + 1);

        // Copy value.
        HAPPrecondition(txtRecords[i].value.numBytes < sizeof serviceDiscovery->txtRecords[i].value.bytes);
        HAPRawBufferCopyBytes(
                serviceDiscovery->txtRecords[i].value.bytes,
                HAPNonnullVoid(txtRecords[i].value.bytes),
                txtRecords[i].value.numBytes);
        serviceDiscovery->txtRecords[i].value.bytes[txtRecords[i].value.numBytes] = '\0';
        HAPAssert(txtRecords[i].value.numBytes <= UINT8_MAX);
        serviceDiscovery->txtRecords[i].value.numBytes = (uint8_t) txtRecords[i].value.numBytes;
    }

    HAPAssert(HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));
}

void HAPPlatformServiceDiscoveryUpdateTXTRecords(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        HAPPlatformServiceDiscoveryTXTRecord* txtRecords,
        size_t numTXTRecords) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));
    HAPPrecondition(txtRecords);

    HAPLog(&logObject, "%s.", __func__);

    // Reset TXT records.
    HAPRawBufferZero(serviceDiscovery->txtRecords, sizeof serviceDiscovery->txtRecords);

    // Copy TXT records.
    HAPPrecondition(numTXTRecords < HAPArrayCount(serviceDiscovery->txtRecords));
    for (size_t i = 0; i < numTXTRecords; i++) {
        HAPLogBuffer(&logObject, txtRecords[i].value.bytes, txtRecords[i].value.numBytes, "%s", txtRecords[i].key);

        // Copy key.
        HAPPrecondition(HAPStringGetNumBytes(txtRecords[i].key) < sizeof serviceDiscovery->txtRecords[i].key);
        HAPRawBufferCopyBytes(
                serviceDiscovery->txtRecords[i].key, txtRecords[i].key, HAPStringGetNumBytes(txtRecords[i].key) + 1);

        // Copy value.
        HAPPrecondition(txtRecords[i].value.numBytes <= sizeof serviceDiscovery->txtRecords[i].value.bytes);
        HAPRawBufferCopyBytes(
                serviceDiscovery->txtRecords[i].value.bytes,
                HAPNonnullVoid(txtRecords[i].value.bytes),
                txtRecords[i].value.numBytes);
        HAPAssert(txtRecords[i].value.numBytes <= UINT8_MAX);
        serviceDiscovery->txtRecords[i].value.numBytes = (uint8_t) txtRecords[i].value.numBytes;
    }

    HAPAssert(HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));
}

void HAPPlatformServiceDiscoveryStop(HAPPlatformServiceDiscoveryRef serviceDiscovery) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));

    HAPLog(&logObject, "%s.", __func__);

    // Reset service discovery.
    HAPRawBufferZero(serviceDiscovery, sizeof *serviceDiscovery);

    HAPAssert(!HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));
}

HAP_RESULT_USE_CHECK
bool HAPPlatformServiceDiscoveryIsAdvertising(HAPPlatformServiceDiscoveryRef serviceDiscovery) {
    HAPPrecondition(serviceDiscovery);

    return serviceDiscovery->port != 0;
}

HAP_RESULT_USE_CHECK
const char* HAPPlatformServiceDiscoveryGetName(HAPPlatformServiceDiscoveryRef serviceDiscovery) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));

    return serviceDiscovery->name;
}

HAP_RESULT_USE_CHECK
const char* HAPPlatformServiceDiscoveryGetProtocol(HAPPlatformServiceDiscoveryRef serviceDiscovery) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));

    return serviceDiscovery->protocol;
}

HAP_RESULT_USE_CHECK
HAPNetworkPort HAPPlatformServiceDiscoveryGetPort(HAPPlatformServiceDiscoveryRef serviceDiscovery) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));

    return serviceDiscovery->port;
}

void HAPPlatformServiceDiscoveryEnumerateTXTRecords(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        HAPPlatformServiceDiscoveryEnumerateTXTRecordsCallback callback,
        void* _Nullable context) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery));
    HAPPrecondition(callback);

    bool shouldContinue = true;
    for (size_t i = 0; shouldContinue && i < HAPArrayCount(serviceDiscovery->txtRecords); i++) {
        if (!HAPStringGetNumBytes(serviceDiscovery->txtRecords[i].key)) {
            break;
        }

        callback(
                context,
                serviceDiscovery,
                serviceDiscovery->txtRecords[i].key,
                serviceDiscovery->txtRecords[i].value.bytes,
                serviceDiscovery->txtRecords[i].value.numBytes,
                &shouldContinue);
    }
}
