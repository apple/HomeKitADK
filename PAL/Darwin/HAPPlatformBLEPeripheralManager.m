// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatformBLEPeripheralManager+Init.h"

#import <CoreBluetooth/CoreBluetooth.h>

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "BLEPeripheralManager" };

extern NSString* const CBAdvertisementDataAppleMfgData;
extern NSString* const CBManagerIsPrivilegedDaemonKey;
extern NSString* const CBCentralManagerScanOptionAllowDuplicatesKey;

static HAPPlatformBLEPeripheralManagerRef blePeripheralManager = NULL;
static HAPPlatformBLEPeripheralManagerDelegate delegate;

static id peek(NSMutableArray* array) {
    return array.firstObject;
}

static id pop(NSMutableArray* array) {
    id obj = peek(array);
    if (obj) {
        [array removeObjectAtIndex:0];
    }
    return obj;
}

@interface HAPBLEPeripheralDarwin : NSObject<CBPeripheralManagerDelegate> {
}
@end

@implementation HAPBLEPeripheralDarwin {
    CBPeripheralManager* _peripheralManager;
    NSMutableArray<id>* _pending;
    __weak CBMutableService* _pendingService;
    __weak CBMutableCharacteristic* _pendingCharacteristic;
    NSMutableDictionary<NSString*, NSNumber*>* _handles;
    NSMutableDictionary<NSString*, CBMutableCharacteristic*>* _characteristics;
    HAPBLEAdvertisingInterval _advertisingInterval;
    NSData* _advertisingData;
    NSData* _scanResponseData;
    uint16_t _nextHandle;
    CBCentral* _connectedCentral;
    struct {
        uint8_t bytes[kHAPPlatformBLEPeripheralManager_MaxAttributeBytes];
        uint16_t handle;
        size_t offset;
        size_t numBytes;
    } _gatt;
}

- (instancetype)init {
    if (self = [super init]) {
        _peripheralManager = nil;
        _advertisingInterval = 0;
        _advertisingData = nil;
        ;
        _scanResponseData = nil;
        _connectedCentral = nil;
        [self removeAllServices];
    }
    return self;
}

- (void)addService:(CBMutableService*)service {
    [_pending addObject:(_pendingService = service)];
    _pendingCharacteristic = nil;
}

- (uint16_t)addCharacteristic:(CBMutableCharacteristic*)characteristic {
    HAPAssert(_pendingService);

    [_pending addObject:(_pendingCharacteristic = characteristic)];
    return [self assignHandleToCharacteristic:characteristic service:_pendingService];
}

- (uint16_t)addDescriptor:(CBMutableDescriptor*)descriptor {
    HAPAssert(_pendingService);
    HAPAssert(_pendingCharacteristic);

    [_pending addObject:descriptor];
    return [self assignHandleToDescriptor:descriptor characteristic:_pendingCharacteristic service:_pendingService];
}

- (void)publishServices {
    _peripheralManager = [[CBPeripheralManager alloc] initWithDelegate:self
                                                                 queue:nil
                                                               options:@{
                                                                   CBManagerIsPrivilegedDaemonKey: @(YES),
                                                                   CBCentralManagerScanOptionAllowDuplicatesKey: @(YES),
                                                               }];
}

- (void)removeAllServices {
    _pending = [[NSMutableArray alloc] init];
    _pendingService = nil;
    _pendingCharacteristic = nil;
    _nextHandle = 1;
    _handles = [[NSMutableDictionary alloc] init];
    _characteristics = [[NSMutableDictionary alloc] init];

    if (_peripheralManager) {
        [_peripheralManager stopAdvertising];
        [_peripheralManager removeAllServices];
        _peripheralManager = nil;
    }
}

- (void)startAdvertising:(HAPBLEAdvertisingInterval)advertisingInterval
         advertisingData:(NSData*)advertisingData
            scanResponse:(NSData*)scanResponseData {
    _advertisingInterval = advertisingInterval;
    _advertisingData = advertisingData;
    _scanResponseData = scanResponseData;

    [self updateState];
}

- (void)stopAdvertising {
    _advertisingInterval = 0;
    _advertisingData = nil;
    _scanResponseData = nil;

    [self updateState];
}

- (void)updateState {
    if (_peripheralManager.state != CBManagerStatePoweredOn) {
        HAPLog(&logObject, "Not powered on.");
        return;
    }

    while (peek(_pending)) {
        HAPAssert([peek(_pending) isKindOfClass:CBMutableService.class]);
        CBMutableService* service = pop(_pending);
        NSMutableArray* characteristics = [[NSMutableArray alloc] init];
        while ([peek(_pending) isKindOfClass:CBMutableCharacteristic.class]) {
            CBMutableCharacteristic* characteristic = pop(_pending);
            NSMutableArray* descriptors = [[NSMutableArray alloc] init];
            while ([peek(_pending) isKindOfClass:CBMutableDescriptor.class]) {
                [descriptors addObject:pop(_pending)];
            }
            characteristic.descriptors = descriptors.count ? descriptors : nil;
            [characteristics addObject:characteristic];
        }
        service.characteristics = characteristics;
        [_peripheralManager addService:service];
    }
    _pendingService = nil;
    _pendingCharacteristic = nil;

    if (!_advertisingInterval) {
        if (_peripheralManager.isAdvertising) {
            [_peripheralManager stopAdvertising];
        }

        HAPLog(&logObject, "Not advertising.");
        return;
    }

    HAPLog(&logObject, "Starting to advertise.");

    [_peripheralManager startAdvertising:@{
        CBAdvertisementDataAppleMfgData: _advertisingData,
        CBAdvertisementDataLocalNameKey: _scanResponseData,
    }];
}

- (void)peripheralManagerDidUpdateState:(CBPeripheralManager*)peripheral {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, "peripheralManagerDidUpdateState");

        [self updateState];
    });
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral willRestoreState:(NSDictionary<NSString*, id>*)dict {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, "willRestoreState");
    });
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral didAddService:(CBService*)service error:(NSError*)error {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, "didAddService");
    });
}

- (void)peripheralManagerDidStartAdvertising:(CBPeripheralManager*)peripheral error:(NSError*)error {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, "peripheralManagerDidStartAdvertising");
    });
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral
                             central:(CBCentral*)central
        didSubscribeToCharacteristic:(CBCharacteristic*)characteristic {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, "didSubscribeFromCharacteristic");

        [self updateCentralConnection:central];
    });
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral
                                 central:(CBCentral*)central
        didUnsubscribeFromCharacteristic:(CBCharacteristic*)characteristic {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, "didUnsubscribeFromCharacteristic");

        [self updateCentralConnection:nil];
    });
}

- (void)peripheralManagerIsReadyToUpdateSubscribers:(CBPeripheralManager*)peripheral {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, "peripheralManagerIsReadyToUpdateSubscribers");
    });
}

- (void)updateCentralConnection:(CBCentral*)central {
    if (central != _connectedCentral) {
        if (_connectedCentral) {
            if (delegate.handleDisconnectedCentral) {
                delegate.handleDisconnectedCentral(
                        blePeripheralManager, [self getHandleForCentral:_connectedCentral], delegate.context);
            }
            _connectedCentral = nil;
        }
        if (central) {
            uint16_t connectionHandle = [self assignHandleToCentral:(_connectedCentral = central)];
            HAPRawBufferZero(&_gatt, sizeof _gatt);
            if (delegate.handleConnectedCentral) {
                delegate.handleConnectedCentral(blePeripheralManager, connectionHandle, delegate.context);
            }
        }
    }
}

- (void)respondToRequest:(CBATTRequest*)request withResult:(CBATTError)error {
    [_peripheralManager.self respondToRequest:request withResult:error];
    if (error) {
        // Abort the connection if there is an error
        [self updateCentralConnection:nil];
    }
}

- (uint16_t)makeHandle {
    return _nextHandle++;
}

- (void)setHandle:(uint16_t)handle forString:(NSString*)str {
    [_handles setValue:@(handle) forKey:str];
}

- (uint16_t)assignHandleToString:(NSString*)str {
    uint16_t handle = [self makeHandle];
    [self setHandle:handle forString:str];
    return handle;
}

- (uint16_t)assignHandleToCentral:(CBCentral*)central {
    return [self assignHandleToString:central.identifier.UUIDString];
}

- (uint16_t)assignHandleToCharacteristic:(CBMutableCharacteristic*)characteristic service:(CBMutableService*)service {
    uint16_t handle = [self assignHandleToString:[@[service.UUID.UUIDString, characteristic.UUID.UUIDString]
                                                         componentsJoinedByString:@""]];
    [_characteristics setValue:characteristic forKey:@(handle).stringValue];
    return handle;
}

- (uint16_t)assignHandleToDescriptor:(CBMutableDescriptor*)descriptor
                      characteristic:(CBMutableCharacteristic*)characteristic
                             service:(CBMutableService*)service {
    return [self
            assignHandleToString:[@[service.UUID.UUIDString, characteristic.UUID.UUIDString, descriptor.UUID.UUIDString]
                                         componentsJoinedByString:@""]];
}

- (uint16_t)getHandleForString:(NSString*)str {
    uint32_t handle = [[_handles objectForKey:str] shortValue];
    HAPAssert(handle);
    return handle;
}

- (uint16_t)getHandleForCentral:(CBCentral*)central {
    return [self getHandleForString:_connectedCentral.identifier.UUIDString];
}

- (uint16_t)getHandleForCharacteristic:(CBCharacteristic*)characteristic service:(CBService*)service {
    return [self getHandleForString:[@[service.UUID.UUIDString, characteristic.UUID.UUIDString]
                                            componentsJoinedByString:@""]];
}

- (CBMutableCharacteristic*)getCharacteristicForHandle:(uint16_t)handle {
    return [_characteristics valueForKey:@(handle).stringValue];
}

- (void)updateCharacteristic:(uint16_t)valueHandle withValue:(NSData*)value forConnection:(uint16_t)connectionHandle {
    HAPPrecondition(_connectedCentral);
    HAPPrecondition([self getHandleForCentral:_connectedCentral] == connectionHandle);

    CBMutableCharacteristic* characteristic = [self getCharacteristicForHandle:valueHandle];
    HAPAssert(characteristic);

    BOOL ok = [_peripheralManager updateValue:value
                            forCharacteristic:characteristic
                         onSubscribedCentrals:@[_connectedCentral]];
    HAPAssert(ok);
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral didReceiveReadRequest:(CBATTRequest*)request {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, __func__);

        [self updateCentralConnection:request.central];

        uint16_t valueHandle = [self getHandleForCharacteristic:request.characteristic
                                                        service:request.characteristic.service];

        if (!request.offset) {
            // Start new read transaction.
            if (!delegate.handleReadRequest) {
                HAPLog(&logObject, "No read request handler plugged in. Sending error response.");
                [self respondToRequest:request withResult:CBATTErrorReadNotPermitted];
                return;
            }

            HAPLogDebug(&logObject, "(0x%04x) ATT Read Request.", valueHandle);

            size_t len;
            HAPError err;
            err = delegate.handleReadRequest(
                    blePeripheralManager,
                    [self getHandleForCentral:_connectedCentral],
                    valueHandle,
                    _gatt.bytes,
                    sizeof _gatt.bytes,
                    &len,
                    delegate.context);
            if (err) {
                HAPAssert(err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
                [self respondToRequest:request withResult:CBATTErrorInsufficientResources];
                return;
            }

            _gatt.handle = valueHandle;
            _gatt.offset = 0;
            HAPAssert(len <= sizeof _gatt.bytes);
            _gatt.numBytes = len;
        } else {
            // Read Blob Request.
            // See Bluetooth Core Specification Version 5
            // Vol 3 Part F Section 3.4.4.5 Read Blob Request
            HAPLogDebug(&logObject, "(0x%04x) ATT Read Blob Request.", valueHandle);

            if (_gatt.handle != valueHandle) {
                HAPLog(&logObject,
                       "Received Read Blob Request for a different characteristic than prior Read Request.");
                [self respondToRequest:request withResult:CBATTErrorRequestNotSupported];
                return;
            }

            if (_gatt.offset != request.offset) {
                HAPLog(&logObject,
                       "Received Read Blob Request with non-sequential offset (expected: %u, actual: %u).",
                       (unsigned) _gatt.offset,
                       (unsigned) request.offset);
                [self respondToRequest:request withResult:CBATTErrorInvalidOffset];
                return;
            }
        }

        HAPAssert(request.offset <= _gatt.numBytes);
        HAPAssert(request.offset >= 0);

        // Send response.
        size_t mtu = _connectedCentral.maximumUpdateValueLength;
        size_t len = _gatt.numBytes - request.offset;
        if (len > mtu) {
            len = mtu;
        }
        request.value = [NSData dataWithBytes:(_gatt.bytes + request.offset) length:len];
        [self respondToRequest:request withResult:CBATTErrorSuccess];

        _gatt.offset += len;
    });
}

- (void)peripheralManager:(CBPeripheralManager*)peripheral didReceiveWriteRequests:(NSArray<CBATTRequest*>*)requests {
    HAPAssert(peripheral == _peripheralManager);

    dispatch_async(dispatch_get_main_queue(), ^{
        HAPLog(&logObject, __func__);

        for (CBATTRequest* request in requests) {
            [self updateCentralConnection:request.central];

            uint16_t valueHandle = [self getHandleForCharacteristic:request.characteristic
                                                            service:request.characteristic.service];

            HAPError err = delegate.handleWriteRequest(
                    blePeripheralManager,
                    [self getHandleForCentral:_connectedCentral],
                    valueHandle,
                    (void*) request.value.bytes,
                    request.value.length,
                    delegate.context);
            if (err) {
                HAPAssert(err == kHAPError_InvalidState || err == kHAPError_InvalidData);
                [self respondToRequest:request withResult:CBATTErrorInvalidPdu];
                return;
            }
        }

        [self respondToRequest:requests[0] withResult:CBATTErrorSuccess];
    });
}

@end

static HAPBLEPeripheralDarwin* peripheral = nil;

void HAPPlatformBLEPeripheralManagerCreate(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerOptions* options) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(options);
    HAPPrecondition(options->keyValueStore);

    HAPRawBufferZero(&delegate, sizeof delegate);
}

void HAPPlatformBLEPeripheralManagerSetDelegate(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager_,
        const HAPPlatformBLEPeripheralManagerDelegate* _Nullable delegate_) {
    HAPPrecondition(blePeripheralManager_);

    if (delegate_) {
        delegate = *delegate_;
        blePeripheralManager = blePeripheralManager_;
    } else {
        HAPRawBufferZero(&delegate, sizeof delegate);
    }
}

void HAPPlatformBLEPeripheralManagerSetDeviceAddress(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerDeviceAddress* deviceAddress) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(deviceAddress);

    HAPLog(&logObject, __func__);
}

void HAPPlatformBLEPeripheralManagerSetDeviceName(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const char* deviceName) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(deviceName);

    HAPLog(&logObject, __func__);
}

static void memrev(uint8_t* dst, const uint8_t* src, size_t n) {
    src += n;
    while (n--) {
        *dst++ = *--src;
    }
}

static CBUUID* uuid(const HAPPlatformBLEPeripheralManagerUUID* uuid) {
    NSMutableData* data = [NSMutableData dataWithLength:sizeof(*uuid)];
    memrev((uint8_t*) data.bytes, (const uint8_t*) uuid, sizeof(*uuid));
    return [CBUUID UUIDWithData:data];
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddService(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* type,
        bool isPrimary) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(type);

    HAPLog(&logObject, __func__);

    if (!peripheral) {
        peripheral = [[HAPBLEPeripheralDarwin alloc] init];
    }

    [peripheral addService:[[CBMutableService alloc] initWithType:uuid(type) primary:isPrimary]];

    return kHAPError_None;
}

void HAPPlatformBLEPeripheralManagerRemoveAllServices(HAPPlatformBLEPeripheralManagerRef blePeripheralManager) {
    HAPPrecondition(blePeripheralManager);

    HAPLog(&logObject, __func__);

    [peripheral removeAllServices];
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddCharacteristic(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* type,
        HAPPlatformBLEPeripheralManagerCharacteristicProperties properties,
        const void* _Nullable constBytes,
        size_t constNumBytes,
        HAPPlatformBLEPeripheralManagerAttributeHandle* valueHandle,
        HAPPlatformBLEPeripheralManagerAttributeHandle* _Nullable cccDescriptorHandle) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(type);
    HAPPrecondition((!constBytes && !constNumBytes) || (constBytes && constNumBytes));
    HAPPrecondition(valueHandle);
    if (properties.notify || properties.indicate) {
        HAPPrecondition(cccDescriptorHandle);
    } else {
        HAPPrecondition(!cccDescriptorHandle);
    }

    HAPLog(&logObject, __func__);

    CBCharacteristicProperties prop = 0;
    if (properties.read)
        prop |= CBCharacteristicPropertyRead;
    if (properties.write)
        prop |= CBCharacteristicPropertyWrite;
    if (properties.writeWithoutResponse)
        prop |= CBCharacteristicPropertyWriteWithoutResponse;
    if (properties.notify)
        prop |= CBCharacteristicPropertyNotify;
    if (properties.indicate)
        prop |= CBCharacteristicPropertyIndicate;
    CBAttributePermissions perm = 0;
    if (properties.read || properties.notify || properties.indicate)
        perm |= CBAttributePermissionsReadable;
    if (properties.write || properties.writeWithoutResponse)
        perm |= CBAttributePermissionsWriteable;

    NSData* value = constBytes ? [NSData dataWithBytes:constBytes length:constNumBytes] : nil;
    CBMutableCharacteristic* characteristic = [[CBMutableCharacteristic alloc] initWithType:uuid(type)
                                                                                 properties:prop
                                                                                      value:value
                                                                                permissions:perm];
    if (properties.notify || properties.indicate) {
        *cccDescriptorHandle = [peripheral makeHandle];
    }

    *valueHandle = [peripheral addCharacteristic:characteristic];

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddDescriptor(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* type,
        HAPPlatformBLEPeripheralManagerDescriptorProperties properties,
        const void* _Nullable constBytes,
        size_t constNumBytes,
        HAPPlatformBLEPeripheralManagerAttributeHandle* descriptorHandle) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(type);
    HAPPrecondition(constBytes && constNumBytes);
    HAPPrecondition(descriptorHandle);

    HAPLog(&logObject, __func__);

    NSData* value = constBytes ? [NSData dataWithBytes:constBytes length:constNumBytes] : nil;
    CBMutableDescriptor* descriptor = [[CBMutableDescriptor alloc] initWithType:uuid(type) value:value];
    *descriptorHandle = [peripheral addDescriptor:descriptor];

    return kHAPError_None;
}

void HAPPlatformBLEPeripheralManagerPublishServices(HAPPlatformBLEPeripheralManagerRef blePeripheralManager) {
    HAPPrecondition(blePeripheralManager);

    HAPLog(&logObject, __func__);

    if (!peripheral) {
        return;
    }

    [peripheral publishServices];
}

void HAPPlatformBLEPeripheralManagerStartAdvertising(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPBLEAdvertisingInterval advertisingInterval,
        const void* advertisingBytes,
        size_t numAdvertisingBytes,
        const void* _Nullable scanResponseBytes,
        size_t numScanResponseBytes) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(advertisingInterval);
    HAPPrecondition(advertisingBytes);
    HAPPrecondition(numAdvertisingBytes);
    HAPPrecondition(!numScanResponseBytes || scanResponseBytes);

    HAPLog(&logObject, __func__);

    // CoreBluetooth automatically prepends 3 bytes for Flags to our advertisement data
    // (It adds flag 0x06: LE General Discoverable Mode bit + BR/EDR Not Supported bit)
    HAPAssert(numAdvertisingBytes >= 3);
    advertisingBytes += 3;
    numAdvertisingBytes -= 3;
    HAPAssert(numScanResponseBytes >= 2);
    scanResponseBytes += 2;
    numScanResponseBytes -= 2;
    NSData* advertisingData = [NSData dataWithBytes:advertisingBytes length:numAdvertisingBytes];
    NSData* scanResponse = [NSData dataWithBytes:scanResponseBytes length:numScanResponseBytes];
    [peripheral startAdvertising:advertisingInterval advertisingData:advertisingData scanResponse:scanResponse];
}

void HAPPlatformBLEPeripheralManagerStopAdvertising(HAPPlatformBLEPeripheralManagerRef blePeripheralManager) {
    HAPPrecondition(blePeripheralManager);

    HAPLog(&logObject, __func__);

    [peripheral stopAdvertising];
}

void HAPPlatformBLEPeripheralManagerCancelCentralConnection(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle) {
    HAPPrecondition(blePeripheralManager);

    HAPLog(&logObject, __func__);

    [peripheral updateCentralConnection:nil];
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerSendHandleValueIndication(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
        HAPPlatformBLEPeripheralManagerAttributeHandle valueHandle,
        const void* _Nullable bytes,
        size_t numBytes) {
    HAPPrecondition(blePeripheralManager);

    HAPLog(&logObject, __func__);

    [peripheral updateCharacteristic:valueHandle
                           withValue:[NSData dataWithBytes:bytes length:numBytes]
                       forConnection:connectionHandle];

    return kHAPError_None;
}
