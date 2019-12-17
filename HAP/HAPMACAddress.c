// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

/**
 * Checks whether a MAC address is valid.
 *
 * - The function may modify the given MAC address candidate.
 *
 * @param      macAddress           MAC address candidate. May be modified.
 *
 * @return true                     If the returned MAC address is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
typedef bool (*HAPMACAddressValidatorCallback)(HAPMACAddress* macAddress);

/**
 * Deterministically derives the MAC address for a given accessory server.
 *
 * @param      server_              Accessory server.
 * @param      networkInterface     Interface.
 * @param      macAddress           MAC address.
 * @param      validatorCallback    Validator callback to check whether a MAC address is valid.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPMACAddressGet(
        HAPAccessoryServerRef* server_,
        const char* _Nullable const networkInterface,
        HAPMACAddress* macAddress,
        HAPMACAddressValidatorCallback validatorCallback) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(macAddress);
    HAPPrecondition(validatorCallback);

    HAPError err;

    // Load Device ID.
    HAPDeviceID deviceID;
    err = HAPDeviceIDGet(server->platform.keyValueStore, &deviceID);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // Load firmware revision.
    // BLE: Accessories supporting random static Bluetooth LE device addresses must use a new
    //      Bluetooth LE device address after a firmware update.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.8 Firmware Update Requirements
    char salt[64 + 4]; // 64 byte FW rev + 4 bytes counter.
    HAPRawBufferZero(salt, sizeof salt);
    size_t numFirmwareVersionBytes = HAPStringGetNumBytes(server->primaryAccessory->firmwareVersion);
    HAPAssert(numFirmwareVersionBytes >= 1);
    HAPAssert(numFirmwareVersionBytes <= 64);
    HAPRawBufferCopyBytes(salt, server->primaryAccessory->firmwareVersion, numFirmwareVersionBytes);

    // Derive MAC addresses until a valid one is found.
    for (uint32_t i = 0;; i++) {
        HAPRawBufferCopyBytes(&salt[64], &i, sizeof i);

        HAP_hkdf_sha512(
                macAddress->bytes,
                sizeof macAddress->bytes,
                deviceID.bytes,
                sizeof deviceID.bytes,
                (const uint8_t*) salt,
                sizeof salt,
                (const uint8_t*) networkInterface,
                networkInterface ? HAPStringGetNumBytes(networkInterface) : 0);

        if (validatorCallback(macAddress)) {
            return kHAPError_None;
        }
    }
}

HAP_RESULT_USE_CHECK
static bool HAPMACAddressValidateRandomStaticBLEDeviceAddress(HAPMACAddress* macAddress) {
    HAPPrecondition(macAddress);

    // Make random static.
    HAPAssert(sizeof macAddress->bytes == 6);
    macAddress->bytes[0] |= 0xC0;

    // Check vs invalid BD_ADDR.
    // - The two most significant bits of the address shall be equal to 1.
    // - At least one bit of the random part of the address shall be 0.
    // - At least one bit of the random part of the address shall be 1.
    // See Bluetooth Core Specification Version 5
    // Vol 6 Part B Section 1.3.2.1 Static Device Address
    static const HAPMACAddress invalidMACAddresses[] = {
        { .bytes = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { .bytes = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } },
    };

    if (HAPRawBufferAreEqual(macAddress->bytes, invalidMACAddresses[0].bytes, sizeof(HAPMACAddress)) ||
        HAPRawBufferAreEqual(macAddress->bytes, invalidMACAddresses[1].bytes, sizeof(HAPMACAddress))) {
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPMACAddressGetRandomStaticBLEDeviceAddress(
        HAPAccessoryServerRef* server,
        const char* _Nullable bleInterface,
        HAPMACAddress* macAddress) {
    return HAPMACAddressGet(server, bleInterface, macAddress, HAPMACAddressValidateRandomStaticBLEDeviceAddress);
}
