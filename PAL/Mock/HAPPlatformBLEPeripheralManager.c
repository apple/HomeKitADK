// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatformBLEPeripheralManager+Init.h"
#include "HAPPlatformBLEPeripheralManager+Test.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "BLEPeripheralManager" };

void HAPPlatformBLEPeripheralManagerCreate(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerOptions* _Nonnull options) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(options);
    HAPPrecondition(options->attributes);
    HAPPrecondition(options->numAttributes <= SIZE_MAX / sizeof options->attributes[0]);

    HAPRawBufferZero(options->attributes, options->numAttributes * sizeof options->attributes[0]);

    HAPRawBufferZero(blePeripheralManager, sizeof *blePeripheralManager);
    blePeripheralManager->attributes = options->attributes;
    blePeripheralManager->numAttributes = options->numAttributes;
}

void HAPPlatformBLEPeripheralManagerSetDelegate(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerDelegate* _Nullable delegate) {
    HAPPrecondition(blePeripheralManager);

    if (delegate) {
        blePeripheralManager->delegate = *delegate;
    } else {
        HAPRawBufferZero(&blePeripheralManager->delegate, sizeof blePeripheralManager->delegate);
    }
}

void HAPPlatformBLEPeripheralManagerSetDeviceAddress(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerDeviceAddress* _Nonnull deviceAddress) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(!blePeripheralManager->isConnected);
    HAPPrecondition(deviceAddress);

    blePeripheralManager->deviceAddress = *deviceAddress;
    blePeripheralManager->isDeviceAddressSet = true;
}

void HAPPlatformBLEPeripheralManagerSetDeviceName(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        const char* _Nonnull deviceName) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(deviceName);

    size_t numDeviceNameBytes = HAPStringGetNumBytes(deviceName);
    HAPPrecondition(numDeviceNameBytes < sizeof blePeripheralManager->deviceName);

    HAPRawBufferZero(blePeripheralManager->deviceName, sizeof blePeripheralManager->deviceName);
    HAPRawBufferCopyBytes(blePeripheralManager->deviceName, deviceName, numDeviceNameBytes);
}

void HAPPlatformBLEPeripheralManagerRemoveAllServices(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(!blePeripheralManager->isConnected);

    HAPAssert(blePeripheralManager->numAttributes <= SIZE_MAX / sizeof blePeripheralManager->attributes[0]);
    HAPRawBufferZero(
            blePeripheralManager->attributes,
            blePeripheralManager->numAttributes * sizeof blePeripheralManager->attributes[0]);
    blePeripheralManager->didPublishAttributes = false;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddService(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* _Nonnull type,
        bool isPrimary) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(!blePeripheralManager->didPublishAttributes);
    HAPPrecondition(type);

    bool inService = false;
    bool inCharacteristic = false;
    HAPPlatformBLEPeripheralManagerAttributeHandle handle = 0;
    for (size_t i = 0; i < blePeripheralManager->numAttributes; i++) {
        HAPPlatformBLEPeripheralManagerAttribute* attribute = &blePeripheralManager->attributes[i];

        switch (attribute->type) {
            case kHAPPlatformBLEPeripheralManagerAttributeType_None: {
                HAPPlatformBLEPeripheralManagerAttributeHandle numNeededHandles = 1;
                if (handle >= (HAPPlatformBLEPeripheralManagerAttributeHandle)(-1 - numNeededHandles)) {
                    HAPLog(&logObject, "Not enough resources to add GATT service (GATT database is full).");
                    return kHAPError_OutOfResources;
                }

                HAPRawBufferZero(attribute, sizeof *attribute);
                attribute->type = kHAPPlatformBLEPeripheralManagerAttributeType_Service;
                attribute->_.service.type = *type;
                attribute->_.service.isPrimary = isPrimary;
                attribute->_.service.handle = ++handle;
            }
                return kHAPError_None;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Service: {
                inService = true;
                inCharacteristic = false;

                HAPAssert(attribute->_.service.handle == handle + 1);
                handle = attribute->_.service.handle;
            } break;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Characteristic: {
                HAPAssert(inService);
                inCharacteristic = true;

                HAPAssert(attribute->_.characteristic.handle == handle + 1);
                handle = attribute->_.characteristic.handle;
                HAPAssert(attribute->_.characteristic.valueHandle == handle + 1);
                handle = attribute->_.characteristic.valueHandle;

                if (attribute->_.characteristic.cccDescriptorHandle) {
                    HAPAssert(attribute->_.characteristic.cccDescriptorHandle == handle + 1);
                    handle = attribute->_.characteristic.cccDescriptorHandle;
                }
            } break;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Descriptor: {
                HAPAssert(inCharacteristic);

                HAPAssert(attribute->_.descriptor.handle == handle + 1);
                handle = attribute->_.descriptor.handle;
            } break;
        }
    }
    HAPLog(&logObject,
           "Not enough resources to add GATT service (have space for %zu GATT attributes).",
           blePeripheralManager->numAttributes);
    return kHAPError_OutOfResources;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddCharacteristic(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* _Nonnull type,
        HAPPlatformBLEPeripheralManagerCharacteristicProperties properties,
        const void* _Nullable constBytes,
        size_t constNumBytes,
        HAPPlatformBLEPeripheralManagerAttributeHandle* _Nonnull valueHandle,
        HAPPlatformBLEPeripheralManagerAttributeHandle* _Nullable cccDescriptorHandle) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(!blePeripheralManager->didPublishAttributes);
    HAPPrecondition(type);
    HAPPrecondition(valueHandle);
    if (properties.notify || properties.indicate) {
        HAPPrecondition(cccDescriptorHandle);
    } else {
        HAPPrecondition(!cccDescriptorHandle);
    }

    bool inService = false;
    bool inCharacteristic = false;
    HAPPlatformBLEPeripheralManagerAttributeHandle handle = 0;
    for (size_t i = 0; i < blePeripheralManager->numAttributes; i++) {
        HAPPlatformBLEPeripheralManagerAttribute* attribute = &blePeripheralManager->attributes[i];

        switch (attribute->type) {
            case kHAPPlatformBLEPeripheralManagerAttributeType_None: {
                HAPPrecondition(inService);

                HAPPlatformBLEPeripheralManagerAttributeHandle numNeededHandles = 2;
                if (properties.indicate || properties.notify) {
                    numNeededHandles++;
                }
                if (handle >= (HAPPlatformBLEPeripheralManagerAttributeHandle)(-1 - numNeededHandles)) {
                    HAPLog(&logObject, "Not enough resources to add GATT characteristic (GATT database is full).");
                    return kHAPError_OutOfResources;
                }

                HAPRawBufferZero(attribute, sizeof *attribute);
                attribute->type = kHAPPlatformBLEPeripheralManagerAttributeType_Characteristic;
                attribute->_.characteristic.type = *type;
                attribute->_.characteristic.properties = properties;
                attribute->_.characteristic.handle = ++handle;
                attribute->_.characteristic.valueHandle = ++handle;
                if (properties.indicate || properties.notify) {
                    attribute->_.characteristic.cccDescriptorHandle = ++handle;
                }

                *valueHandle = attribute->_.characteristic.valueHandle;
                if (cccDescriptorHandle) {
                    *cccDescriptorHandle = attribute->_.characteristic.cccDescriptorHandle;
                }
            }
                return kHAPError_None;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Service: {
                inService = true;
                inCharacteristic = false;

                HAPAssert(attribute->_.service.handle == handle + 1);
                handle = attribute->_.service.handle;
            } break;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Characteristic: {
                HAPAssert(inService);
                inCharacteristic = true;

                HAPAssert(attribute->_.characteristic.handle == handle + 1);
                handle = attribute->_.characteristic.handle;
                HAPAssert(attribute->_.characteristic.valueHandle == handle + 1);
                handle = attribute->_.characteristic.valueHandle;

                if (attribute->_.characteristic.cccDescriptorHandle) {
                    HAPAssert(attribute->_.characteristic.cccDescriptorHandle == handle + 1);
                    handle = attribute->_.characteristic.cccDescriptorHandle;
                }
            } break;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Descriptor: {
                HAPAssert(inCharacteristic);

                HAPAssert(attribute->_.descriptor.handle == handle + 1);
                handle = attribute->_.descriptor.handle;
            } break;
        }
    }
    HAPLog(&logObject,
           "Not enough resources to add GATT characteristic (have space for %zu GATT attributes).",
           blePeripheralManager->numAttributes);
    return kHAPError_OutOfResources;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddDescriptor(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* _Nonnull type,
        HAPPlatformBLEPeripheralManagerDescriptorProperties properties,
        const void* _Nullable constBytes,
        size_t constNumBytes,
        HAPPlatformBLEPeripheralManagerAttributeHandle* _Nonnull descriptorHandle) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(!blePeripheralManager->didPublishAttributes);
    HAPPrecondition(type);
    HAPPrecondition(descriptorHandle);

    bool inService = false;
    bool inCharacteristic = false;
    HAPPlatformBLEPeripheralManagerAttributeHandle handle = 0;
    for (size_t i = 0; i < blePeripheralManager->numAttributes; i++) {
        HAPPlatformBLEPeripheralManagerAttribute* attribute = &blePeripheralManager->attributes[i];

        switch (attribute->type) {
            case kHAPPlatformBLEPeripheralManagerAttributeType_None: {
                HAPPrecondition(inCharacteristic);

                HAPPlatformBLEPeripheralManagerAttributeHandle numNeededHandles = 1;
                if (handle >= (HAPPlatformBLEPeripheralManagerAttributeHandle)(-1 - numNeededHandles)) {
                    HAPLog(&logObject, "Not enough resources to add GATT descriptor (GATT database is full).");
                    return kHAPError_OutOfResources;
                }

                HAPRawBufferZero(attribute, sizeof *attribute);
                attribute->type = kHAPPlatformBLEPeripheralManagerAttributeType_Descriptor;
                attribute->_.descriptor.type = *type;
                attribute->_.descriptor.properties = properties;
                attribute->_.descriptor.handle = ++handle;

                *descriptorHandle = attribute->_.descriptor.handle;
            }
                return kHAPError_None;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Service: {
                inService = true;
                inCharacteristic = false;

                HAPAssert(attribute->_.service.handle == handle + 1);
                handle = attribute->_.service.handle;
            } break;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Characteristic: {
                HAPAssert(inService);
                inCharacteristic = true;

                HAPAssert(attribute->_.characteristic.handle == handle + 1);
                handle = attribute->_.characteristic.handle;
                HAPAssert(attribute->_.characteristic.valueHandle == handle + 1);
                handle = attribute->_.characteristic.valueHandle;

                if (attribute->_.characteristic.cccDescriptorHandle) {
                    HAPAssert(attribute->_.characteristic.cccDescriptorHandle == handle + 1);
                    handle = attribute->_.characteristic.cccDescriptorHandle;
                }
            } break;
            case kHAPPlatformBLEPeripheralManagerAttributeType_Descriptor: {
                HAPAssert(inCharacteristic);

                HAPAssert(attribute->_.descriptor.handle == handle + 1);
                handle = attribute->_.descriptor.handle;
            } break;
        }
    }
    HAPLog(&logObject,
           "Not enough resources to add GATT descriptor (have space for %zu GATT attributes).",
           blePeripheralManager->numAttributes);
    return kHAPError_OutOfResources;
}

void HAPPlatformBLEPeripheralManagerPublishServices(HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(blePeripheralManager->isDeviceAddressSet);
    HAPPrecondition(!blePeripheralManager->didPublishAttributes);

    blePeripheralManager->didPublishAttributes = true;
}

void HAPPlatformBLEPeripheralManagerStartAdvertising(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        HAPBLEAdvertisingInterval advertisingInterval,
        const void* _Nonnull advertisingBytes,
        size_t numAdvertisingBytes,
        const void* _Nullable scanResponseBytes,
        size_t numScanResponseBytes) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(blePeripheralManager->isDeviceAddressSet);
    HAPPrecondition(blePeripheralManager->didPublishAttributes);
    HAPPrecondition(advertisingInterval);
    HAPPrecondition(advertisingBytes);
    HAPPrecondition(numAdvertisingBytes);
    HAPPrecondition(numAdvertisingBytes <= sizeof blePeripheralManager->advertisingBytes);
    HAPPrecondition(!numScanResponseBytes || scanResponseBytes);
    HAPPrecondition(numScanResponseBytes <= sizeof blePeripheralManager->scanResponseBytes);

    HAPPlatformBLEPeripheralManagerStopAdvertising(blePeripheralManager);

    HAPRawBufferCopyBytes(blePeripheralManager->advertisingBytes, advertisingBytes, numAdvertisingBytes);
    blePeripheralManager->numAdvertisingBytes = (uint8_t) numAdvertisingBytes;
    if (scanResponseBytes) {
        HAPRawBufferCopyBytes(
                blePeripheralManager->scanResponseBytes, HAPNonnullVoid(scanResponseBytes), numScanResponseBytes);
    }
    blePeripheralManager->numScanResponseBytes = (uint8_t) numScanResponseBytes;
    blePeripheralManager->advertisingInterval = advertisingInterval;
}

void HAPPlatformBLEPeripheralManagerStopAdvertising(HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(blePeripheralManager->isDeviceAddressSet);
    HAPPrecondition(blePeripheralManager->didPublishAttributes);

    HAPRawBufferZero(blePeripheralManager->advertisingBytes, sizeof blePeripheralManager->advertisingBytes);
    blePeripheralManager->numAdvertisingBytes = 0;
    HAPRawBufferZero(blePeripheralManager->scanResponseBytes, sizeof blePeripheralManager->scanResponseBytes);
    blePeripheralManager->numScanResponseBytes = 0;
    blePeripheralManager->advertisingInterval = 0;
}

HAP_RESULT_USE_CHECK
bool HAPPlatformBLEPeripheralManagerIsAdvertising(HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager) {
    HAPPrecondition(blePeripheralManager);

    return blePeripheralManager->advertisingInterval != 0;
}

void HAPPlatformBLEPeripheralManagerGetDeviceAddress(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        HAPPlatformBLEPeripheralManagerDeviceAddress* _Nonnull deviceAddress) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(HAPPlatformBLEPeripheralManagerIsAdvertising(blePeripheralManager));
    HAPPrecondition(deviceAddress);

    *deviceAddress = blePeripheralManager->deviceAddress;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerGetAdvertisingData(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        void* _Nonnull advertisingBytes,
        size_t maxAdvertisingBytes,
        size_t* _Nonnull numAdvertisingBytes,
        void* _Nonnull scanResponseBytes,
        size_t maxScanResponseBytes,
        size_t* _Nonnull numScanResponseBytes) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(HAPPlatformBLEPeripheralManagerIsAdvertising(blePeripheralManager));
    HAPPrecondition(advertisingBytes);
    HAPPrecondition(numAdvertisingBytes);
    HAPPrecondition(scanResponseBytes);
    HAPPrecondition(numScanResponseBytes);

    if (maxAdvertisingBytes < blePeripheralManager->numAdvertisingBytes) {
        HAPLogError(
                &logObject,
                "Not enough space to store advertising data (%zu / %u bytes)",
                maxAdvertisingBytes,
                blePeripheralManager->numAdvertisingBytes);
        return kHAPError_OutOfResources;
    }
    if (maxScanResponseBytes < blePeripheralManager->numScanResponseBytes) {
        HAPLogError(
                &logObject,
                "Not enough space to store advertising data (%zu / %u bytes)",
                maxScanResponseBytes,
                blePeripheralManager->numScanResponseBytes);
        return kHAPError_OutOfResources;
    }

    HAPRawBufferCopyBytes(
            advertisingBytes, blePeripheralManager->advertisingBytes, blePeripheralManager->numAdvertisingBytes);
    *numAdvertisingBytes = blePeripheralManager->numAdvertisingBytes;

    HAPRawBufferCopyBytes(
            scanResponseBytes, blePeripheralManager->scanResponseBytes, blePeripheralManager->numScanResponseBytes);
    *numScanResponseBytes = blePeripheralManager->numScanResponseBytes;

    return kHAPError_None;
}

void HAPPlatformBLEPeripheralManagerCancelCentralConnection(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle) {
    HAPPrecondition(blePeripheralManager);

    (void) connectionHandle;
    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}

HAPError HAPPlatformBLEPeripheralManagerSendHandleValueIndication(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
        HAPPlatformBLEPeripheralManagerAttributeHandle valueHandle,
        const void* _Nullable bytes,
        size_t numBytes) HAP_DIAGNOSE_ERROR(!bytes && numBytes, "empty buffer cannot have a length") {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(valueHandle);
    HAPPrecondition(!numBytes || bytes);

    (void) connectionHandle;
    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}
