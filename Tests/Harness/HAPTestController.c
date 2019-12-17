// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP.h"
#include "HAPPlatformBLEPeripheralManager+Test.h"
#include "HAPPlatformServiceDiscovery+Test.h"
#include "HAPTestController.h"

#include "util_base64.h"

static const HAPLogObject logObject = { .subsystem = "com.apple.mfi.HomeKit.Core.Test", .category = "TestController" };

typedef struct {
    HAPAccessoryServerInfo* _Nonnull serverInfo;
    bool invalidData : 1;
    bool foundCN : 1;
    bool foundFF : 1;
    bool foundID : 1;
    bool foundMD : 1;
    bool foundPV : 1;
    bool foundSN : 1;
    bool foundSF : 1;
    bool foundCI : 1;
    bool foundSH : 1;
} EnumerateHAPTXTRecordsContext;

static void EnumerateHAPTXTRecordsCallback(
        void* _Nullable context,
        HAPPlatformServiceDiscoveryRef _Nonnull serviceDiscovery,
        const char* _Nonnull key,
        const void* _Nonnull valueBytes,
        size_t numValueBytes,
        bool* _Nonnull shouldContinue) {
    HAPPrecondition(context);
    EnumerateHAPTXTRecordsContext* arguments = context;
    HAPPrecondition(arguments->serverInfo);
    HAPPrecondition(!arguments->invalidData);
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(key);
    HAPPrecondition(valueBytes);
    HAPPrecondition(shouldContinue);
    HAPPrecondition(*shouldContinue);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 6.4 Discovery

#define RETURN_ERROR() \
    do { \
        arguments->invalidData = true; \
        *shouldContinue = false; \
        return; \
    } while (0)

#define PARSE_INT_OR_RETURN_ERROR(value, minValue, maxValue) \
    do { \
        err = HAPUInt64FromString(valueBytes, (value)); \
        if (err) { \
            HAPAssert(err == kHAPError_InvalidData); \
            HAPLogError(&logObject, "Found malformed %s value (not a number).", key); \
            RETURN_ERROR(); \
        } \
        if (*(value) < (minValue) || *(value) > (maxValue)) { \
            HAPLogError(&logObject, "Found out of range %s value (%llu).", key, (unsigned long long) *(value)); \
            RETURN_ERROR(); \
        } \
    } while (0)

    if (HAPStringGetNumBytes(valueBytes) != numValueBytes) {
        HAPLogError(&logObject, "Found malformed %s value (not a string).", key);
        RETURN_ERROR();
    }

    if (HAPStringAreEqual(key, "c#")) {
        if (arguments->foundCN) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundCN = true;

        uint64_t value;
        PARSE_INT_OR_RETURN_ERROR(&value, 1, 65535);
        arguments->serverInfo->configurationNumber = (uint32_t) value;
    } else if (HAPStringAreEqual(key, "ff")) {
        if (arguments->foundFF) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundFF = true;

        uint64_t value;
        PARSE_INT_OR_RETURN_ERROR(&value, 0x00, 0xff);
        if ((value & 0x01u) == 0x01u) {
            arguments->serverInfo->pairingFeatureFlags.supportsMFiHWAuth = true;
            value &= ~0x01u;
        }
        if ((value & 0x02u) == 0x02u) {
            arguments->serverInfo->pairingFeatureFlags.supportsMFiTokenAuth = true;
            value &= ~0x02u;
        }
        if (value) {
            HAPLog(&logObject, "Ignoring unknown %s flags: 0x%02x.", key, (uint8_t) value);
        }
    } else if (HAPStringAreEqual(key, "id")) {
        if (arguments->foundID) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundID = true;

        // XX:XX:XX:XX:XX:XX
        const char* stringValue = valueBytes;
        if (numValueBytes != 3 * 6 - 1) {
            HAPLogError(&logObject, "Found malformed %s value (unexpected length).", key);
            RETURN_ERROR();
        }
        for (size_t i = 0; i < 5; i++) {
            if (stringValue[3 * i + 2] != ':') {
                HAPLogError(&logObject, "Found malformed %s value (separator missing).", key);
                RETURN_ERROR();
            }
        }
        for (size_t i = 0; i < 6; i++) {
            uint8_t n = 0;
            char c = stringValue[3 * i + 0];
            switch (c) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    n += (uint8_t)(0 + c - '0');
                } break;
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f': {
                    n += (uint8_t)(10 + c - 'a');
                } break;
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F': {
                    n += (uint8_t)(10 + c - 'A');
                } break;
                default: {
                    HAPLogError(&logObject, "Found malformed %s value (unexpected character).", key);
                }
                    RETURN_ERROR();
            }
            n *= 16;
            c = stringValue[3 * i + 1];
            switch (c) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    n += (uint8_t)(0 + c - '0');
                } break;
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f': {
                    n += (uint8_t)(10 + c - 'a');
                } break;
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F': {
                    n += (uint8_t)(10 + c - 'A');
                } break;
                default: {
                    HAPLogError(&logObject, "Found malformed %s value (unexpected character).", key);
                }
                    RETURN_ERROR();
            }
            arguments->serverInfo->deviceID.bytes[i] = n;
        }
    } else if (HAPStringAreEqual(key, "md")) {
        if (arguments->foundMD) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundMD = true;

        if (numValueBytes >= sizeof arguments->serverInfo->model) {
            HAPLogError(&logObject, "Found too long %s value (%zu bytes).", key, numValueBytes);
            RETURN_ERROR();
        }
        HAPRawBufferCopyBytes(arguments->serverInfo->model, valueBytes, numValueBytes + 1);
    } else if (HAPStringAreEqual(key, "pv")) {
        if (arguments->foundPV) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundPV = true;

        // X.X
        const char* stringValue = valueBytes;
        bool separatorAllowed = false;
        bool majorRead = false;
        uint8_t n = 0;
        for (size_t i = 0; i < numValueBytes; i++) {
            char c = stringValue[i];
            switch (c) {
                case '.': {
                    if (!separatorAllowed) {
                        HAPLogError(&logObject, "Found malformed %s value (separator at unexpected position).", key);
                        RETURN_ERROR();
                    }
                    if (majorRead) {
                        HAPLogError(&logObject, "Found malformed %s value (too many separators).", key);
                        RETURN_ERROR();
                    }
                    majorRead = true;
                    arguments->serverInfo->protocolVersion.major = n;
                    n = 0;
                } break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    separatorAllowed = true;
                    if (n > UINT8_MAX / 10) {
                        HAPLogError(&logObject, "Found malformed %s value (version too large).", key);
                        RETURN_ERROR();
                    }
                    n *= 10;
                    if (n > UINT8_MAX - (c - '0')) {
                        HAPLogError(&logObject, "Found malformed %s value (version too large).", key);
                        RETURN_ERROR();
                    }
                    n += c - '0';
                } break;
                default: {
                    HAPLogError(&logObject, "Found malformed %s value (unexpected character).", key);
                }
                    RETURN_ERROR();
            }
        }
        if (!separatorAllowed) {
            HAPLogError(&logObject, "Found malformed %s value (not ending in number).", key);
            RETURN_ERROR();
        }
        if (!majorRead) {
            arguments->serverInfo->protocolVersion.major = n;
            n = 0;
        }
        arguments->serverInfo->protocolVersion.minor = n;
    } else if (HAPStringAreEqual(key, "s#")) {
        if (arguments->foundSN) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundSN = true;

        if (!HAPStringAreEqual(valueBytes, "1")) {
            HAPLogError(&logObject, "Found unexpected %s value (must be 1).", key);
            RETURN_ERROR();
        }
        arguments->serverInfo->stateNumber = 1;
    } else if (HAPStringAreEqual(key, "sf")) {
        if (arguments->foundSF) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundSF = true;

        uint64_t value;
        PARSE_INT_OR_RETURN_ERROR(&value, 0x00, 0xff);
        if ((value & 0x01u) == 0x01u) {
            arguments->serverInfo->statusFlags.isNotPaired = true;
            value &= ~0x01u;
        }
        if ((value & 0x04u) == 0x04u) {
            arguments->serverInfo->statusFlags.hasProblem = true;
            value &= ~0x04u;
        }
        if (value) {
            HAPLog(&logObject, "Ignoring unknown %s flags: 0x%02x.", key, (uint8_t) value);
        }
    } else if (HAPStringAreEqual(key, "ci")) {
        if (arguments->foundCI) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundCI = true;

        uint64_t value;
        PARSE_INT_OR_RETURN_ERROR(&value, 1, 65535);
        if (value > (HAPAccessoryCategory) -1) {
            HAPLog(&logObject, "Found unexpected %s value: %u.", key, (uint16_t) value);
            RETURN_ERROR();
        }
        arguments->serverInfo->category = 0;
        switch ((HAPAccessoryCategory) value) {
            case kHAPAccessoryCategory_BridgedAccessory: {
                HAPLog(&logObject, "Found unexpected %s value: %u.", key, (uint16_t) value);
            }
                RETURN_ERROR();
            case kHAPAccessoryCategory_Other:
            case kHAPAccessoryCategory_Bridges:
            case kHAPAccessoryCategory_Fans:
            case kHAPAccessoryCategory_GarageDoorOpeners:
            case kHAPAccessoryCategory_Lighting:
            case kHAPAccessoryCategory_Locks:
            case kHAPAccessoryCategory_Outlets:
            case kHAPAccessoryCategory_Switches:
            case kHAPAccessoryCategory_Thermostats:
            case kHAPAccessoryCategory_Sensors:
            case kHAPAccessoryCategory_SecuritySystems:
            case kHAPAccessoryCategory_Doors:
            case kHAPAccessoryCategory_Windows:
            case kHAPAccessoryCategory_WindowCoverings:
            case kHAPAccessoryCategory_ProgrammableSwitches:
            case kHAPAccessoryCategory_RangeExtenders:
            case kHAPAccessoryCategory_AirPurifiers:
            case kHAPAccessoryCategory_Heaters:
            case kHAPAccessoryCategory_AirConditioners:
            case kHAPAccessoryCategory_Humidifiers:
            case kHAPAccessoryCategory_Dehumidifiers:
            case kHAPAccessoryCategory_Sprinklers:
            case kHAPAccessoryCategory_Faucets:
            case kHAPAccessoryCategory_ShowerSystems: {
                arguments->serverInfo->category = (HAPAccessoryCategory) value;
            } break;
        }
        if (!arguments->serverInfo->category) {
            HAPLog(&logObject, "Found unexpected %s value: %u.", key, (uint16_t) value);
            RETURN_ERROR();
        }
    } else if (HAPStringAreEqual(key, "sh")) {
        if (arguments->foundSH) {
            HAPLogError(&logObject, "Found duplicate %s key.", key);
            RETURN_ERROR();
        }
        arguments->foundSH = true;

        if (numValueBytes != util_base64_encoded_len(sizeof arguments->serverInfo->setupHash.bytes + 1)) {
            HAPLogError(&logObject, "Found malformed %s value (unexpected length).", key);
            RETURN_ERROR();
        }
        size_t numDecodedBytes;
        err = util_base64_decode(
                valueBytes,
                numValueBytes,
                arguments->serverInfo->setupHash.bytes,
                sizeof arguments->serverInfo->setupHash.bytes,
                &numDecodedBytes);
        HAPAssert(err != kHAPError_OutOfResources);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            HAPLogError(&logObject, "Found malformed %s value (not in base64 format).", key);
            RETURN_ERROR();
        }
        HAPAssert(numDecodedBytes == sizeof arguments->serverInfo->setupHash.bytes);
        arguments->serverInfo->setupHash.isSet = true;
    } else {
        HAPLog(&logObject, "Ignoring unknown %s key.", key);
    }

#undef PARSE_INT_OR_RETURN_ERROR
#undef RETURN_ERROR
}

HAPError HAPDiscoverIPAccessoryServer(
        HAPPlatformServiceDiscoveryRef _Nonnull serviceDiscovery,
        HAPAccessoryServerInfo* _Nonnull serverInfo,
        HAPNetworkPort* _Nonnull serverPort) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(serverInfo);
    HAPPrecondition(serverPort);

    HAPRawBufferZero(serverInfo, sizeof *serverInfo);
    *serverPort = 0;

    if (!HAPPlatformServiceDiscoveryIsAdvertising(serviceDiscovery)) {
        HAPLog(&logObject, "IP accessory server is not advertising.");
        return kHAPError_InvalidState;
    }

    // See HomeKit Accessory Protocol Specification R14
    // Section 6.4 Discovery
    {
        const char* value = HAPPlatformServiceDiscoveryGetName(serviceDiscovery);
        size_t numValueBytes = HAPStringGetNumBytes(value);
        if (numValueBytes >= sizeof serverInfo->name) {
            HAPLogError(&logObject, "Found too long name (%zu bytes).", numValueBytes);
            return kHAPError_InvalidData;
        }
        HAPRawBufferCopyBytes(serverInfo->name, value, numValueBytes + 1);
    }
    if (!HAPStringAreEqual(HAPPlatformServiceDiscoveryGetProtocol(serviceDiscovery), "_hap._tcp")) {
        HAPLog(&logObject, "IP accessory server is not advertising HAP service.");
        return kHAPError_InvalidData;
    }
    *serverPort = HAPPlatformServiceDiscoveryGetPort(serviceDiscovery);

    // Process TXT records.
    EnumerateHAPTXTRecordsContext context = { .serverInfo = serverInfo };
    HAPPlatformServiceDiscoveryEnumerateTXTRecords(serviceDiscovery, EnumerateHAPTXTRecordsCallback, &context);
    if (context.invalidData) {
        return kHAPError_InvalidData;
    }
    if (!context.foundCN) {
        HAPLogError(&logObject, "IP accessory server is not advertising %s key.", "c#");
        return kHAPError_InvalidData;
    }
    if (!context.foundFF) {
        HAPLogError(&logObject, "IP accessory server is not advertising %s key.", "ff");
        return kHAPError_InvalidData;
    }
    if (!context.foundID) {
        HAPLogError(&logObject, "IP accessory server is not advertising %s key.", "id");
        return kHAPError_InvalidData;
    }
    if (!context.foundMD) {
        HAPLogError(&logObject, "IP accessory server is not advertising %s key.", "md");
        return kHAPError_InvalidData;
    }
    if (!context.foundPV) {
        context.serverInfo->protocolVersion.major = 1;
        context.serverInfo->protocolVersion.minor = 0;
    }
    if (!context.foundSN) {
        HAPLogError(&logObject, "IP accessory server is not advertising %s key.", "s#");
        return kHAPError_InvalidData;
    }
    if (!context.foundSF) {
        HAPLogError(&logObject, "IP accessory server is not advertising %s key.", "sf");
        return kHAPError_InvalidData;
    }
    if (!context.foundCI) {
        HAPLogError(&logObject, "IP accessory server is not advertising %s key.", "ci");
        return kHAPError_InvalidData;
    }
    if (!context.foundSH) {
        context.serverInfo->setupHash.isSet = false;
    }

    return kHAPError_None;
}

HAPError HAPDiscoverBLEAccessoryServer(
        HAPPlatformBLEPeripheralManagerRef _Nonnull blePeripheralManager,
        HAPAccessoryServerInfo* _Nonnull serverInfo,
        HAPPlatformBLEPeripheralManagerDeviceAddress* _Nonnull deviceAddress) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(serverInfo);
    HAPPrecondition(deviceAddress);

    HAPError err;

    HAPRawBufferZero(serverInfo, sizeof *serverInfo);
    HAPRawBufferZero(deviceAddress, sizeof *deviceAddress);

    if (!HAPPlatformBLEPeripheralManagerIsAdvertising(blePeripheralManager)) {
        HAPLog(&logObject, "BLE accessory server is not advertising.");
        return kHAPError_InvalidState;
    }

    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.2.1 HAP BLE Regular Advertisement Format
    uint8_t advertisingBytes[31];
    size_t numAdvertisingBytes;
    uint8_t scanResponseBytes[31];
    size_t numScanResponseBytes;
    err = HAPPlatformBLEPeripheralManagerGetAdvertisingData(
            blePeripheralManager,
            advertisingBytes,
            sizeof advertisingBytes,
            &numAdvertisingBytes,
            scanResponseBytes,
            sizeof scanResponseBytes,
            &numScanResponseBytes);
    HAPAssert(!err);

    // See Bluetooth Core Specification Version 5
    // Vol 3 Part C Section 11 Advertising and Scan Response Data Format

    // Process advertising data.
    bool foundFlags = false;
    bool foundManufacturerData = false;
    bool foundShortenedName = false;
    bool foundCompleteName = false;
    for (size_t i = 0; i < numAdvertisingBytes;) {
        uint8_t length = advertisingBytes[i++];
        if (numAdvertisingBytes < length) {
            HAPLogError(&logObject, "BLE advertising data invalid (invalid length).");
            return kHAPError_InvalidData;
        }
        if (length < 1) {
            HAPLogError(&logObject, "BLE advertising data invalid (AD type missing).");
            return kHAPError_InvalidData;
        }
        uint8_t adType = advertisingBytes[i++];
        length--;
        switch (adType) {
            case 0x01: {
                // Flags.
                // See Bluetooth Core Specification Supplement Version 7
                // Section 1.3 Flags
                if (length != 1) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Invalid %s length).", "Flags");
                    return kHAPError_InvalidData;
                }
                uint8_t flags = advertisingBytes[i];
                if ((flags & (1 << 0)) != (0 << 0)) {
                    HAPLogError(&logObject, "BLE advertising data invalid (LE Limited Discoverable Mode is set).");
                    return kHAPError_InvalidData;
                }
                if ((flags & (1 << 1)) != (1 << 1)) {
                    HAPLogError(&logObject, "BLE advertising data invalid (LE General Discoverable Mode is not set).");
                    return kHAPError_InvalidData;
                }

                if (foundFlags) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Duplicate Flags).");
                    return kHAPError_InvalidData;
                }
                foundFlags = true;
            } break;
            case 0x08: {
                // Shortened Local Name.
                // See Bluetooth Core Specification Supplement Version 7
                // Section 1.2 Local Name
                if (length >= sizeof serverInfo->name) {
                    HAPLogError(
                            &logObject, "BLE advertising data invalid (Invalid %s length).", "Shortened Local Name");
                    return kHAPError_InvalidData;
                }

                if (foundCompleteName) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Duplicate Local Name).");
                    return kHAPError_InvalidData;
                }
                if (foundShortenedName) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Duplicate Shortened Local Name).");
                    return kHAPError_InvalidData;
                }
                foundShortenedName = true;

                HAPRawBufferZero(serverInfo->name, sizeof serverInfo->name);
                HAPRawBufferCopyBytes(serverInfo->name, &advertisingBytes[i], length);
                if (length != HAPStringGetNumBytes(serverInfo->name)) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Shortened Local Name contains NULL bytes).");
                    return kHAPError_InvalidData;
                }
            } break;
            case 0x09: {
                // Complete Local Name.
                // See Bluetooth Core Specification Supplement Version 7
                // Section 1.2 Local Name
                if (length >= sizeof serverInfo->name) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Invalid %s length).", "Complete Local Name");
                    return kHAPError_InvalidData;
                }

                if (foundShortenedName) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Duplicate Local Name).");
                    return kHAPError_InvalidData;
                }
                if (foundCompleteName) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Duplicate Complete Local Name).");
                    return kHAPError_InvalidData;
                }
                foundCompleteName = true;

                if (foundShortenedName) {
                    if (HAPStringGetNumBytes(serverInfo->name) > length) {
                        HAPLogError(
                                &logObject,
                                "BLE advertising data invalid (Complete Local Name shorter than Shortened Local "
                                "Name).");
                        return kHAPError_InvalidData;
                    }
                    if (!HAPRawBufferAreEqual(
                                serverInfo->name, &advertisingBytes[i], HAPStringGetNumBytes(serverInfo->name))) {
                        HAPLogError(
                                &logObject,
                                "BLE advertising data invalid "
                                "(Complete Local Name does not start with Shortened Local Name).");
                        return kHAPError_InvalidData;
                    }
                }

                HAPRawBufferZero(serverInfo->name, sizeof serverInfo->name);
                HAPRawBufferCopyBytes(serverInfo->name, &advertisingBytes[i], length);
                if (length != HAPStringGetNumBytes(serverInfo->name)) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Complete Local Name contains NULL bytes).");
                    return kHAPError_InvalidData;
                }
            }
            case 0xFF: {
                // Manufacturer Specific Data.
                // See Bluetooth Core Specification Supplement Version 7
                // Section 1.4 Manufacturer Specific Data
                if (length < 2) {
                    HAPLogError(
                            &logObject,
                            "BLE advertising data invalid (Invalid %s length).",
                            "Manufacturer Specific Data");
                    return kHAPError_InvalidData;
                }
                uint16_t coID = HAPReadLittleUInt16(&advertisingBytes[i]);
                i += 2;
                length -= 2;
                if (coID != 0x004C) {
                    HAPLog(&logObject,
                           "Ignoring unknown Manufacturer Specific Data from company with ID 0x%04X.",
                           coID);
                    break;
                }

                if (length < 1) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Apple, Inc. Type missing).");
                    return kHAPError_InvalidData;
                }
                uint8_t type = advertisingBytes[i++];
                length--;
                if (type != 0x06) {
                    HAPLog(&logObject, "Ignoring unknown Apple, Inc. Specific Data with Type 0x%02X.", type);
                    break;
                }
                if (length < 1) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Apple, Inc. SubTypeLength missing).");
                    return kHAPError_InvalidData;
                }
                uint8_t subTypeLength = advertisingBytes[i++];
                length--;
                uint8_t subType = (uint8_t)((subTypeLength >> 5) & ((1 << 0) | (1 << 1) | (1 << 2)));
                if (subType != 1) {
                    HAPLog(&logObject,
                           "Ignoring unknown Apple, Inc. Specific Data with Type 0x%02X / SubType 0x%02X.",
                           type,
                           subType);
                    break;
                }
                if ((subTypeLength & ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4))) != length) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Unexpected Apple, Inc. SubTypeLength).");
                    return kHAPError_InvalidData;
                }

                if (foundManufacturerData) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Duplicate Manufacturer Data).");
                    return kHAPError_InvalidData;
                }
                foundManufacturerData = true;

                if (length < 1) {
                    HAPLogError(&logObject, "BLE advertising data invalid (SF missing).");
                    return kHAPError_InvalidData;
                }
                uint8_t sf = advertisingBytes[i++];
                length--;
                serverInfo->statusFlags.isNotPaired = (sf & (1 << 0)) == (1 << 0);

                if (length < sizeof deviceAddress->bytes) {
                    HAPLogError(&logObject, "BLE advertising data invalid (Device ID missing).");
                    return kHAPError_InvalidData;
                }
                HAPRawBufferCopyBytes(deviceAddress->bytes, &advertisingBytes[i], sizeof deviceAddress->bytes);
                i += sizeof deviceAddress->bytes;
                length -= sizeof deviceAddress->bytes;

                if (length < 2) {
                    HAPLogError(&logObject, "BLE advertising data invalid (ACID missing).");
                    return kHAPError_InvalidData;
                }
                uint16_t acid = HAPReadLittleUInt16(&advertisingBytes[i]);
                i += 2;
                length -= 2;
                if (acid > (HAPAccessoryCategory) -1) {
                    HAPLog(&logObject, "BLE advertising data invalid (unexpected ACID value: %u).", acid);
                    return kHAPError_InvalidData;
                }
                serverInfo->category = 0;
                switch ((HAPAccessoryCategory) acid) {
                    case kHAPAccessoryCategory_BridgedAccessory: {
                        HAPLog(&logObject, "BLE advertising data invalid (unexpected ACID value: %u).", acid);
                    }
                        return kHAPError_InvalidData;
                    case kHAPAccessoryCategory_Other:
                    case kHAPAccessoryCategory_Bridges:
                    case kHAPAccessoryCategory_Fans:
                    case kHAPAccessoryCategory_GarageDoorOpeners:
                    case kHAPAccessoryCategory_Lighting:
                    case kHAPAccessoryCategory_Locks:
                    case kHAPAccessoryCategory_Outlets:
                    case kHAPAccessoryCategory_Switches:
                    case kHAPAccessoryCategory_Thermostats:
                    case kHAPAccessoryCategory_Sensors:
                    case kHAPAccessoryCategory_SecuritySystems:
                    case kHAPAccessoryCategory_Doors:
                    case kHAPAccessoryCategory_Windows:
                    case kHAPAccessoryCategory_WindowCoverings:
                    case kHAPAccessoryCategory_ProgrammableSwitches:
                    case kHAPAccessoryCategory_RangeExtenders:
                    case kHAPAccessoryCategory_AirPurifiers:
                    case kHAPAccessoryCategory_Heaters:
                    case kHAPAccessoryCategory_AirConditioners:
                    case kHAPAccessoryCategory_Humidifiers:
                    case kHAPAccessoryCategory_Dehumidifiers:
                    case kHAPAccessoryCategory_Sprinklers:
                    case kHAPAccessoryCategory_Faucets:
                    case kHAPAccessoryCategory_ShowerSystems: {
                        serverInfo->category = (HAPAccessoryCategory) acid;
                    } break;
                }
                if (!serverInfo->category) {
                    HAPLog(&logObject, "BLE advertising data invalid (unexpected ACID value: %u).", acid);
                    return kHAPError_InvalidData;
                }

                if (length < 2) {
                    HAPLogError(&logObject, "BLE advertising data invalid (GSN missing).");
                    return kHAPError_InvalidData;
                }
                uint16_t gsn = HAPReadLittleUInt16(&advertisingBytes[i]);
                i += 2;
                length -= 2;
                if (!gsn) {
                    HAPLog(&logObject, "BLE advertising data invalid (unexpected GSN value: %u).", gsn);
                    return kHAPError_InvalidData;
                }
                serverInfo->stateNumber = gsn;

                if (length < 1) {
                    HAPLogError(&logObject, "BLE advertising data invalid (CN missing).");
                    return kHAPError_InvalidData;
                }
                uint8_t cn = advertisingBytes[i++];
                length--;
                if (!cn) {
                    HAPLog(&logObject, "BLE advertising data invalid (unexpected CN value: %u).", cn);
                    return kHAPError_InvalidData;
                }
                serverInfo->configurationNumber = cn;

                if (length < 1) {
                    HAPLogError(&logObject, "BLE advertising data invalid (CV missing).");
                    return kHAPError_InvalidData;
                }
                uint8_t cv = advertisingBytes[i++];
                length--;
                if (cv != 2) {
                    HAPLog(&logObject, "BLE advertising data invalid (unexpected CV value: %u).", cv);
                    return kHAPError_InvalidData;
                }

                if (length) {
                    if (length < sizeof serverInfo->setupHash.bytes) {
                        HAPLogError(&logObject, "BLE advertising data invalid (SH missing).");
                        return kHAPError_InvalidData;
                    }
                    HAPRawBufferCopyBytes(
                            serverInfo->setupHash.bytes, &advertisingBytes[i], sizeof serverInfo->setupHash.bytes);
                    i += sizeof serverInfo->setupHash.bytes;
                    length -= sizeof serverInfo->setupHash.bytes;
                    serverInfo->setupHash.isSet = true;
                }

                if (length) {
                    HAPLog(&logObject, "Ignoring extra data in BLE Manufacturer Data (%u bytes).", length);
                }
            } break;
            default: {
                HAPLog(&logObject, "Ignoring unknown AD type in BLE advertising data: %u.", adType);
            } break;
        }
        i += length;
    }
    if (!foundFlags) {
        HAPLogError(&logObject, "BLE advertising data invalid (Flags not found).");
        return kHAPError_InvalidData;
    }
    if (!foundManufacturerData) {
        HAPLogError(&logObject, "BLE advertising data invalid (Manufacturer Data not found).");
        return kHAPError_InvalidData;
    }
    if (!foundCompleteName && !foundShortenedName) {
        HAPLogError(&logObject, "BLE advertising data invalid (Local Name not found).");
        return kHAPError_InvalidData;
    }

    // Process scan response data.
    bool foundName = foundCompleteName;
    for (size_t i = 0; i < numScanResponseBytes;) {
        uint8_t length = scanResponseBytes[i++];
        if (numScanResponseBytes < length) {
            HAPLogError(&logObject, "BLE scan response data invalid (invalid length).");
            return kHAPError_InvalidData;
        }
        if (length < 1) {
            HAPLogError(&logObject, "BLE scan response data invalid (AD type missing).");
            return kHAPError_InvalidData;
        }
        uint8_t adType = scanResponseBytes[i++];
        length--;
        switch (adType) {
            case 0x01: {
                HAPLogError(&logObject, "BLE scan response data invalid (Contains Flags).");
            }
                return kHAPError_InvalidData;
            case 0x08:
            case 0x09: {
                // Local Name.
                // See Bluetooth Core Specification Supplement Version 7
                // Section 1.2 Local Name
                if (length >= sizeof serverInfo->name) {
                    HAPLogError(
                            &logObject, "BLE scan response data invalid (Invalid %s length).", "Complete Local Name");
                    return kHAPError_InvalidData;
                }

                if (foundName) {
                    HAPLogError(&logObject, "BLE scan response data invalid (Duplicate Complete Local Name).");
                    return kHAPError_InvalidData;
                }
                foundName = true;

                HAPAssert(foundShortenedName);
                if (HAPStringGetNumBytes(serverInfo->name) > length) {
                    HAPLogError(
                            &logObject,
                            "BLE scan response data invalid (Local Name shorter than Shortened Local Name).");
                    return kHAPError_InvalidData;
                }
                if (!HAPRawBufferAreEqual(
                            serverInfo->name, &scanResponseBytes[i], HAPStringGetNumBytes(serverInfo->name))) {
                    HAPLogError(
                            &logObject,
                            "BLE scan response data invalid "
                            "(Local Name does not start with Shortened Local Name).");
                    return kHAPError_InvalidData;
                }

                HAPRawBufferZero(serverInfo->name, sizeof serverInfo->name);
                HAPRawBufferCopyBytes(serverInfo->name, &scanResponseBytes[i], length);
                if (length != HAPStringGetNumBytes(serverInfo->name)) {
                    HAPLogError(&logObject, "BLE scan response data invalid (Local Name contains NULL bytes).");
                    return kHAPError_InvalidData;
                }
            } break;
            case 0xFF: {
                HAPLogError(&logObject, "BLE scan response data invalid (Contains Manufacturer Data).");
            }
                return kHAPError_InvalidData;
            default: {
                HAPLog(&logObject, "Ignoring unknown AD type in BLE scan response data: %u.", adType);
            } break;
        }
        i += length;
    }
    if (!foundName) {
        HAPLogError(&logObject, "BLE scan response data invalid (Local Name not found).");
        return kHAPError_InvalidData;
    }

    return kHAPError_None;
}
