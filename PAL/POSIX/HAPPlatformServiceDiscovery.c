// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <arpa/inet.h>
#include <unistd.h>

#include "HAPPlatform+Init.h"
#include "HAPPlatformServiceDiscovery+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "ServiceDiscovery" };

// TODO Add support for re-registering service discovery in case of error while app is running.

static void HandleFileHandleCallback(
        HAPPlatformFileHandleRef fileHandle,
        HAPPlatformFileHandleEvent fileHandleEvents,
        void* _Nullable context) {
    HAPAssert(fileHandle);
    HAPAssert(fileHandleEvents.isReadyForReading);
    HAPAssert(context);

    HAPPlatformServiceDiscoveryRef serviceDiscovery = context;

    HAPAssert(serviceDiscovery->fileHandle == fileHandle);

    DNSServiceErrorType errorCode = DNSServiceProcessResult(serviceDiscovery->dnsService);
    if (errorCode != kDNSServiceErr_NoError) {
        HAPLogError(&logObject, "%s: Service discovery results processing failed: %ld.", __func__, (long) errorCode);
        HAPFatalError();
    }
}

static void HandleServiceRegisterReply(
        DNSServiceRef service HAP_UNUSED,
        DNSServiceFlags flags HAP_UNUSED,
        DNSServiceErrorType errorCode,
        const char* name HAP_UNUSED,
        const char* regtype HAP_UNUSED,
        const char* domain HAP_UNUSED,
        void* context_ HAP_UNUSED) {
    if (errorCode != kDNSServiceErr_NoError) {
        HAPLogError(&logObject, "%s: Service discovery registration failed: %ld.", __func__, (long) errorCode);
        HAPFatalError();
    }
}

void HAPPlatformServiceDiscoveryCreate(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        const HAPPlatformServiceDiscoveryOptions* options) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(options);

    HAPLogDebug(&logObject, "Storage configuration: serviceDiscovery = %lu", (unsigned long) sizeof *serviceDiscovery);

    HAPRawBufferZero(serviceDiscovery, sizeof *serviceDiscovery);

    if (options->interfaceName) {
        size_t numInterfaceNameBytes = HAPStringGetNumBytes(HAPNonnull(options->interfaceName));
        if ((numInterfaceNameBytes == 0) || (numInterfaceNameBytes >= sizeof serviceDiscovery->interfaceName)) {
            HAPLogError(&logObject, "Invalid local network interface name.");
            HAPFatalError();
        }
        HAPRawBufferCopyBytes(
                serviceDiscovery->interfaceName, HAPNonnull(options->interfaceName), numInterfaceNameBytes);
    }

    serviceDiscovery->dnsService = NULL;
}

void HAPPlatformServiceDiscoveryRegister(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        const char* name,
        const char* protocol,
        HAPNetworkPort port,
        HAPPlatformServiceDiscoveryTXTRecord* txtRecords,
        size_t numTXTRecords) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(!serviceDiscovery->dnsService);
    HAPPrecondition(name);
    HAPPrecondition(protocol);
    HAPPrecondition(txtRecords);

    HAPError err;
    DNSServiceErrorType errorCode;

    uint32_t interfaceIndex;
    if (serviceDiscovery->interfaceName[0]) {
        unsigned int i = if_nametoindex(serviceDiscovery->interfaceName);
        if ((i == 0) || (i > UINT32_MAX)) {
            HAPLogError(&logObject, "Mapping the local network interface name to its corresponding index failed.");
            HAPFatalError();
        }
        interfaceIndex = (uint32_t) i;
    } else {
        interfaceIndex = 0;
    }

    HAPLogDebug(&logObject, "interfaceIndex: %lu", (unsigned long) interfaceIndex);
    HAPLogDebug(&logObject, "name: \"%s\"", name);
    HAPLogDebug(&logObject, "protocol: \"%s\"", protocol);
    HAPLogDebug(&logObject, "port: %u", port);

    TXTRecordCreate(
            &serviceDiscovery->txtRecord,
            sizeof serviceDiscovery->txtRecordBuffer,
            &serviceDiscovery->txtRecordBuffer[0]);
    for (size_t i = 0; i < numTXTRecords; i++) {
        HAPPrecondition(!txtRecords[i].value.numBytes || txtRecords[i].value.bytes);
        HAPPrecondition(txtRecords[i].value.numBytes <= UINT8_MAX);
        if (txtRecords[i].value.bytes) {
            HAPLogBufferDebug(
                    &logObject,
                    txtRecords[i].value.bytes,
                    txtRecords[i].value.numBytes,
                    "txtRecord[%lu]: \"%s\"",
                    (unsigned long) i,
                    txtRecords[i].key);
        } else {
            HAPLogDebug(&logObject, "txtRecord[%lu]: \"%s\"", (unsigned long) i, txtRecords[i].key);
        }
        errorCode = TXTRecordSetValue(
                &serviceDiscovery->txtRecord,
                txtRecords[i].key,
                (uint8_t) txtRecords[i].value.numBytes,
                txtRecords[i].value.bytes);
        if (errorCode != kDNSServiceErr_NoError) {
            HAPLogError(&logObject, "%s: TXTRecordSetValue failed: %ld.", __func__, (long) errorCode);
            HAPFatalError();
        }
    }

    errorCode = DNSServiceRegister(
            &serviceDiscovery->dnsService,
            /* flags: */ 0,
            interfaceIndex,
            name,
            protocol,
            /* domain: */ NULL,
            /* host: */ NULL,
            htons(port),
            TXTRecordGetLength(&serviceDiscovery->txtRecord),
            TXTRecordGetBytesPtr(&serviceDiscovery->txtRecord),
            HandleServiceRegisterReply,
            serviceDiscovery);
    if (errorCode != kDNSServiceErr_NoError) {
        HAPLogError(&logObject, "%s: DNSServiceRegister failed: %ld.", __func__, (long) errorCode);
        HAPFatalError();
    }

    err = HAPPlatformFileHandleRegister(
            &serviceDiscovery->fileHandle,
            DNSServiceRefSockFD(serviceDiscovery->dnsService),
            (HAPPlatformFileHandleEvent) {
                    .isReadyForReading = true, .isReadyForWriting = false, .hasErrorConditionPending = false },
            HandleFileHandleCallback,
            serviceDiscovery);
    if (err) {
        HAPLogError(&logObject, "%s: HAPPlatformFileHandleRegister failed: %u.", __func__, err);
        HAPFatalError();
    }
    HAPAssert(serviceDiscovery->fileHandle);
}

void HAPPlatformServiceDiscoveryUpdateTXTRecords(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        HAPPlatformServiceDiscoveryTXTRecord* txtRecords,
        size_t numTXTRecords) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(serviceDiscovery->dnsService);
    HAPPrecondition(txtRecords);

    DNSServiceErrorType errorCode;

    TXTRecordDeallocate(&serviceDiscovery->txtRecord);
    TXTRecordCreate(
            &serviceDiscovery->txtRecord,
            sizeof serviceDiscovery->txtRecordBuffer,
            &serviceDiscovery->txtRecordBuffer[0]);
    for (size_t i = 0; i < numTXTRecords; i++) {
        HAPPrecondition(!txtRecords[i].value.numBytes || txtRecords[i].value.bytes);
        HAPPrecondition(txtRecords[i].value.numBytes <= UINT8_MAX);
        if (txtRecords[i].value.bytes) {
            HAPLogBufferDebug(
                    &logObject,
                    txtRecords[i].value.bytes,
                    txtRecords[i].value.numBytes,
                    "txtRecord[%lu]: \"%s\"",
                    (unsigned long) i,
                    txtRecords[i].key);
        } else {
            HAPLogDebug(&logObject, "txtRecord[%lu]: \"%s\"", (unsigned long) i, txtRecords[i].key);
        }
        errorCode = TXTRecordSetValue(
                &serviceDiscovery->txtRecord,
                txtRecords[i].key,
                (uint8_t) txtRecords[i].value.numBytes,
                txtRecords[i].value.bytes);
        if (errorCode != kDNSServiceErr_NoError) {
            HAPLogError(&logObject, "%s: TXTRecordSetValue failed: %ld.", __func__, (long) errorCode);
            HAPFatalError();
        }
    }

    errorCode = DNSServiceUpdateRecord(
            serviceDiscovery->dnsService,
            /* recordRef: */ NULL,
            /* flags: */ 0,
            TXTRecordGetLength(&serviceDiscovery->txtRecord),
            TXTRecordGetBytesPtr(&serviceDiscovery->txtRecord),
            0);
    if (errorCode != kDNSServiceErr_NoError) {
        HAPLogError(&logObject, "%s: DNSServiceUpdateRecord failed: %ld.", __func__, (long) errorCode);
        HAPFatalError();
    }
}

void HAPPlatformServiceDiscoveryStop(HAPPlatformServiceDiscoveryRef serviceDiscovery) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(serviceDiscovery->dnsService);

    HAPPlatformFileHandleDeregister(serviceDiscovery->fileHandle);

    DNSServiceRefDeallocate(serviceDiscovery->dnsService);
    serviceDiscovery->dnsService = NULL;

    TXTRecordDeallocate(&serviceDiscovery->txtRecord);
}
