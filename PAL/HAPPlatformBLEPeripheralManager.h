// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_BLE_PERIPHERAL_MANAGER_H
#define HAP_PLATFORM_BLE_PERIPHERAL_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * BLE peripheral manager.
 */
typedef struct HAPPlatformBLEPeripheralManager HAPPlatformBLEPeripheralManager;
typedef struct HAPPlatformBLEPeripheralManager* HAPPlatformBLEPeripheralManagerRef;
HAP_NONNULL_SUPPORT(HAPPlatformBLEPeripheralManager)

/**
 * Bluetooth Connection Handle.
 *
 * - Range: 0x0000-0x0EFF.
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 2 Part E Section 5.3.1 Primary Controller Handles
 */
typedef uint16_t HAPPlatformBLEPeripheralManagerConnectionHandle;

/**
 * Bluetooth Attribute Handle.
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 3 Part F Section 3.2.2 Attribute Handle
 */
typedef uint16_t HAPPlatformBLEPeripheralManagerAttributeHandle;

/**
 * Maximum length of an attribute value.
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 3 Part F Section 3.2.9 Long Attribute Values
 */
#define kHAPPlatformBLEPeripheralManager_MaxAttributeBytes ((size_t) 512)

/**
 * Delegate that is used to monitor read, write, and subscription requests from remote central devices.
 */
typedef struct {
    /**
     * Client context pointer. Will be passed to callbacks.
     */
    void* _Nullable context;

    /**
     * Invoked when a connection has been established in response to the advertising data that has been set
     * through HAPPlatformBLEPeripheralManagerStartAdvertising.
     *
     * - If a connection is established through other means, it is not considered a HomeKit connection
     *   and must not lead to the invocation of this callback.
     *
     * @param      blePeripheralManager BLE peripheral manager.
     * @param      connectionHandle     Connection handle of the connected central.
     * @param      context              The context pointer of the BLE peripheral manager delegate structure.
     */
    void (*_Nullable handleConnectedCentral)(
            HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
            HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
            void* _Nullable context);

    /**
     * Invoked when a connection that was reported to the handleConnectedCentral callback has been terminated.
     *
     * @param      blePeripheralManager BLE peripheral manager.
     * @param      connectionHandle     Connection handle of the disconnected central.
     * @param      context              The context pointer of the BLE peripheral manager delegate structure.
     */
    void (*_Nullable handleDisconnectedCentral)(
            HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
            HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
            void* _Nullable context);

    /**
     * Invoked when a read request is received on an attribute that has been registered through
     * HAPPlatformBLEPeripheralManagerAddCharacteristic or HAPPlatformBLEPeripheralManagerAddDescriptor.
     *
     * - The supplied buffer should have space for kHAPPlatformBLEPeripheralManager_MaxAttributeBytes bytes.
     *   It is left to the BLE peripheral manager implementation to then transfer the full buffer over a sequence of
     *   central-initiated "Read Request" and "Read Blob Request" operations to the central.
     *   This callback should only be invoked again once the full data has been transmitted.
     *
     * @param      blePeripheralManager BLE peripheral manager.
     * @param      connectionHandle     Connection handle of the central that sent the request.
     * @param      attributeHandle      Attribute handle that is being read.
     * @param[out] bytes                Buffer to fill read response into.
     * @param      maxBytes             Capacity of buffer.
     * @param[out] numBytes             Length of data that was filled into buffer.
     * @param      context              The context pointer of the BLE peripheral manager delegate structure.
     *
     * @return kHAPError_None           If successful.
     * @return kHAPError_InvalidState   If a read on that characteristic is not allowed in the current state.
     * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
     */
    HAP_RESULT_USE_CHECK
    HAPError (*_Nullable handleReadRequest)(
            HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
            HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
            HAPPlatformBLEPeripheralManagerAttributeHandle attributeHandle,
            void* bytes,
            size_t maxBytes,
            size_t* numBytes,
            void* _Nullable context);

    /**
     * Invoked when a write request is received on an attribute that has been registered through
     * HAPPlatformBLEPeripheralManagerAddCharacteristic or HAPPlatformBLEPeripheralManagerAddDescriptor.
     *
     * - The supplied buffer must support writes up to kHAPPlatformBLEPeripheralManager_MaxAttributeBytes bytes.
     *   It is left to the BLE peripheral manager implementation to assemble fragments of potential
     *   "Prepare Write Request" and "Execute Write Request" operations.
     *   This callback should only be invoked once the full data has been received.
     *
     * @param      blePeripheralManager BLE peripheral manager.
     * @param      connectionHandle     Connection handle of the central that sent the request.
     * @param      attributeHandle      Attribute handle that is being read.
     * @param      bytes                Buffer that contains the request data.
     * @param      numBytes             Length of data in buffer.
     * @param      context              The context pointer of the BLE peripheral manager delegate structure.
     *
     * @return kHAPError_None           If successful.
     * @return kHAPError_InvalidState   If a write on that characteristic is not allowed in the current state.
     * @return kHAPError_InvalidData    If the request data has an invalid format.
     * @return kHAPError_OutOfResources If there are not enough resources to handle the request.
     */
    HAP_RESULT_USE_CHECK
    HAPError (*_Nullable handleWriteRequest)(
            HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
            HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
            HAPPlatformBLEPeripheralManagerAttributeHandle attributeHandle,
            void* bytes,
            size_t numBytes,
            void* _Nullable context);

    /**
     * Invoked when the BLE peripheral manager is again ready to send characteristic value updates through
     * HAPPlatformBLEPeripheralManagerSendHandleValueIndication.
     *
     * @param      blePeripheralManager BLE peripheral manager.
     * @param      connectionHandle     Connection handle of the central that is ready to receive events.
     * @param      context              The context pointer of the BLE peripheral manager delegate structure.
     */
    void (*_Nullable handleReadyToUpdateSubscribers)(
            HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
            HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
            void* _Nullable context);
} HAPPlatformBLEPeripheralManagerDelegate;

/**
 * Specifies or clears the delegate for receiving peripheral events.
 *
 * - The delegate structure is copied and does not need to remain valid.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      delegate             The delegate to receive the peripheral role events. NULL to clear.
 */
void HAPPlatformBLEPeripheralManagerSetDelegate(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerDelegate* _Nullable delegate);

/**
 * Bluetooth device address (BD_ADDR).
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 2 Part B Section 1.2 Bluetooth Device Addressing
 */
typedef struct {
    uint8_t bytes[6]; /**< Little-endian. */
} HAPPlatformBLEPeripheralManagerDeviceAddress;
HAP_STATIC_ASSERT(
        sizeof(HAPPlatformBLEPeripheralManagerDeviceAddress) == 6,
        HAPPlatformBLEPeripheralManagerDeviceAddress);

/**
 * Sets the Bluetooth device address (BD_ADDR).
 *
 * - The address is a random (static) MAC address.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      deviceAddress        Bluetooth device address.
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 6 Part B Section 1.3.2.1 Static Device Address
 */
void HAPPlatformBLEPeripheralManagerSetDeviceAddress(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerDeviceAddress* deviceAddress);

/**
 * Sets the Bluetooth GAP Device Name.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      deviceName           Bluetooth device name.
 */
void HAPPlatformBLEPeripheralManagerSetDeviceName(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const char* deviceName);

/**
 * 128-bit Bluetooth UUID.
 */
typedef struct {
    uint8_t bytes[16]; /**< Little-endian. */
} HAPPlatformBLEPeripheralManagerUUID;
HAP_STATIC_ASSERT(sizeof(HAPPlatformBLEPeripheralManagerUUID) == 16, HAPPlatformBLEPeripheralManagerUUID);

/**
 * Removes all published services from the local GATT database.
 *
 * - Only services that were added through HAPPlatformBLEPeripheralManager methods are affected.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 */
void HAPPlatformBLEPeripheralManagerRemoveAllServices(HAPPlatformBLEPeripheralManagerRef blePeripheralManager);

/**
 * Publishes a service to the local GATT database.
 *
 * - Separate AddCharacteristic calls are used to publish the associated characteristics.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      type                 The Bluetooth-specific 128-bit UUID that identifies the service.
 * @param      isPrimary            Whether the type of service is primary or secondary.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If there are not enough resources to publish the service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddService(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* type,
        bool isPrimary);

/**
 * Possible properties of a characteristic.
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 3 Part G Section 3.3.1.1 Characteristic Properties
 */
typedef struct {
    /** If set, permits reads of the Characteristic Value. */
    bool read : 1;

    /** If set, permit writes of the Characteristic Value without response. */
    bool writeWithoutResponse : 1;

    /** If set, permits writes of the Characteristic Value with response. */
    bool write : 1;

    /**
     * If set, permits notifications of a Characteristic Value without acknowledgment.
     * If set, the Client Characteristic Configuration Descriptor must be published as well.
     */
    bool notify : 1;

    /**
     * If set, permits indications of a Characteristic Value with acknowledgment.
     * If set, the Client Characteristic Configuration Descriptor must be published as well.
     */
    bool indicate : 1;
} HAPPlatformBLEPeripheralManagerCharacteristicProperties;

/**
 * Publishes a characteristic to the local GATT database. It is associated with the most recently added service.
 *
 * - Separate AddDescriptor calls are used to publish the associated descriptors.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      type                 The Bluetooth-specific 128-bit UUID that identifies the characteristic.
 * @param      properties           The properties of the characteristic.
 * @param      constBytes           Value if constant, otherwise NULL
 * @param      constNumBytes        Size if constant, otherwise 0
 * @param[out] valueHandle          Attribute handle of the added Characteristic Value declaration.
 * @param[out] cccDescriptorHandle  Attribute handle of the added Client Characteristic Configuration descriptor.
 *                                  This parameter must only be filled if notify or indicate properties are set.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If there are not enough resources to publish the characteristic.
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 3 Part G Section 3.3.3.3 Client Characteristic Configuration
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddCharacteristic(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* type,
        HAPPlatformBLEPeripheralManagerCharacteristicProperties properties,
        const void* _Nullable constBytes,
        size_t constNumBytes,
        HAPPlatformBLEPeripheralManagerAttributeHandle* valueHandle,
        HAPPlatformBLEPeripheralManagerAttributeHandle* _Nullable cccDescriptorHandle);

/**
 * Possible properties of a descriptor.
 */
typedef struct {
    /** If set, permits reads of the descriptor. */
    bool read : 1;

    /** If set, permits writes of the descriptor. */
    bool write : 1;
} HAPPlatformBLEPeripheralManagerDescriptorProperties;

/**
 * Publishes a descriptor to the local GATT database. It is associated with the most recently added characteristic.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      type                 The Bluetooth-specific 128-bit UUID that identifies the descriptor.
 * @param      properties           The properties of the descriptor.
 * @param      constBytes           Value if constant, otherwise NULL
 * @param      constNumBytes        Size if constant, otherwise 0*
 * @param[out] descriptorHandle     Attribute handle of the added descriptor.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If there are not enough resources to publish the descriptor.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerAddDescriptor(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerUUID* type,
        HAPPlatformBLEPeripheralManagerDescriptorProperties properties,
        const void* _Nullable constBytes,
        size_t constNumBytes,
        HAPPlatformBLEPeripheralManagerAttributeHandle* descriptorHandle);

/**
 * This function is called after all services have been added.
 *
 * - Before new services are added again, HAPPlatformBLEPeripheralManagerRemoveAllServices is called.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 */
void HAPPlatformBLEPeripheralManagerPublishServices(HAPPlatformBLEPeripheralManagerRef blePeripheralManager);

/**
 * Advertises BLE peripheral manager data or updates advertised data.
 *
 * - Advertisements must be undirected and connectable (ADV_IND).
 *
 * - When a central connects in response to the advertisements,
 *   the delegate's handleConnectedCentral method shall be called.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      advertisingInterval  Advertisement interval.
 * @param      advertisingBytes     Buffer containing advertising data.
 * @param      numAdvertisingBytes  Length of advertising data buffer.
 * @param      scanResponseBytes    Buffer containing scan response data, if applicable.
 * @param      numScanResponseBytes Length of scan response data buffer.
 */
void HAPPlatformBLEPeripheralManagerStartAdvertising(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPBLEAdvertisingInterval advertisingInterval,
        const void* advertisingBytes,
        size_t numAdvertisingBytes,
        const void* _Nullable scanResponseBytes,
        size_t numScanResponseBytes);

/**
 * Stops advertising BLE peripheral manager data.
 *
 * - Once this function returns, the delegate's handleConnectedCentral method must not be called anymore
 *   unless advertisements are started again.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 */
void HAPPlatformBLEPeripheralManagerStopAdvertising(HAPPlatformBLEPeripheralManagerRef blePeripheralManager);

/**
 * Cancels an active connection to a central.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      connectionHandle     Connection handle of the central to disconnect.
 */
void HAPPlatformBLEPeripheralManagerCancelCentralConnection(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle);

/**
 * Sends an indication to a subscribed central to update a characteristic value.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param      connectionHandle     Connection handle of the central to update.
 * @param      valueHandle          Handle of the Characteristic Value declaration whose value changed.
 * @param      bytes                Buffer containing the value that is sent to the central.
 * @param      numBytes             Length of buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the event could not be sent at this time, or the central is not subscribed.
 * @return kHAPError_OutOfResources If the buffer is too large to send.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerSendHandleValueIndication(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
        HAPPlatformBLEPeripheralManagerAttributeHandle valueHandle,
        const void* _Nullable bytes,
        size_t numBytes) HAP_DIAGNOSE_ERROR(!bytes && numBytes, "empty buffer cannot have a length");

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
