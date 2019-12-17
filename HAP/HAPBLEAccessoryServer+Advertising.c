// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLEAccessoryServer" };

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerGetGSN(HAPPlatformKeyValueStoreRef keyValueStore, HAPBLEAccessoryServerGSN* gsn) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(gsn);

    HAPError err;

    bool found;
    size_t numBytes;
    uint8_t gsnBytes[sizeof(uint16_t) + sizeof(uint8_t)];
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_BLEGSN,
            gsnBytes,
            sizeof gsnBytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        HAPWriteLittleUInt16(&gsnBytes[0], 1U);
        gsnBytes[2] = 0x00;
    } else if (numBytes != sizeof gsnBytes) {
        HAPLog(&logObject, "Invalid GSN length %lu.", (unsigned long) numBytes);
        return kHAPError_Unknown;
    }

    HAPRawBufferZero(gsn, sizeof *gsn);
    gsn->gsn = HAPReadLittleUInt16(&gsnBytes[0]);
    gsn->didIncrement = (gsnBytes[2] & 0x01u) == 0x01;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerGetAdvertisingParameters(
        HAPAccessoryServerRef* server_,
        bool* isActive,
        uint16_t* advertisingInterval,
        void* advertisingBytes,
        size_t maxAdvertisingBytes,
        size_t* numAdvertisingBytes,
        void* scanResponseBytes,
        size_t maxScanResponseBytes,
        size_t* numScanResponseBytes)
        HAP_DIAGNOSE_ERROR(maxAdvertisingBytes < 31, "maxAdvertisingBytes must be at least 31")
                HAP_DIAGNOSE_WARNING(maxScanResponseBytes < 2, "maxScanResponseBytes should be at least 2") {
    HAPPrecondition(server_);
    const HAPAccessoryServer* server = (const HAPAccessoryServer*) server_;
    HAPPrecondition(isActive);
    HAPPrecondition(advertisingInterval);
    HAPPrecondition(advertisingBytes);
    HAPPrecondition(maxAdvertisingBytes >= 31);
    HAPPrecondition(numAdvertisingBytes);
    HAPPrecondition(scanResponseBytes);
    HAPPrecondition(numScanResponseBytes);

    HAPError err;

    *isActive = true;
    *advertisingInterval = 0;
    *numAdvertisingBytes = 0;
    *numScanResponseBytes = 0;

    // The accessory shall not advertise while it is connected to a HomeKit controller.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.1.4 Advertising Interval
    if (server->ble.adv.connected) {
        *isActive = false;
        return kHAPError_None;
    }

    if (server->ble.adv.broadcastedEvent.iid) {
        // HAP BLE Encrypted Notification Advertisement Format.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.2.2 HAP BLE Encrypted Notification Advertisement Format

        uint16_t keyExpirationGSN;
        HAPBLEAccessoryServerBroadcastEncryptionKey broadcastKey;
        HAPDeviceID advertisingID;
        err = HAPBLEAccessoryServerBroadcastGetParameters(
                server->platform.keyValueStore, &keyExpirationGSN, &broadcastKey, &advertisingID);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
        if (!keyExpirationGSN) {
            HAPLog(&logObject, "Started broadcasted event without valid key. Corrupted data?");
            return kHAPError_Unknown;
        }
        HAPBLEAccessoryServerGSN gsn;
        err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }

        // Interval.
        *advertisingInterval = 0;
        switch (server->ble.adv.broadcastedEvent.interval) {
            case kHAPBLECharacteristicBroadcastInterval_20Ms: {
                *advertisingInterval = HAPBLEAdvertisingIntervalCreateFromMilliseconds(20);
            } break;
            case kHAPBLECharacteristicBroadcastInterval_1280Ms: {
                *advertisingInterval = HAPBLEAdvertisingIntervalCreateFromMilliseconds(1280);
            } break;
            case kHAPBLECharacteristicBroadcastInterval_2560Ms: {
                *advertisingInterval = HAPBLEAdvertisingIntervalCreateFromMilliseconds(2560);
            } break;
        }
        HAPAssert(*advertisingInterval);

        uint8_t* adv = advertisingBytes;

        // Flags.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.2.2.1 Flags
        /* 0x00   LEN */ *adv++ = 0x02;
        /* 0x01   ADT */ *adv++ = 0x01;
        /* 0x02 Flags */ *adv++ = 0U << 0U | //  NO: LE Limited Discoverable Mode.
                                  1U << 1U | // YES: LE General Discoverable Mode.
                                  1U << 2U | // YES: BR/EDR Not Supported.
                                  0U << 3U | //  NO: Simultaneous LE and BR/EDR to Same Device Capable (Controller).
                                  0U << 4U | //  NO: Simultaneous LE and BR/EDR to Same Device Capable (Host).
                                  0U << 5U | 0U << 6U | 0U << 7U; // Reserved.

        // Manufacturer data.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.2.2.2 Manufacturer Data
        /* 0x00   LEN */ *adv++ = 0x1B;
        /* 0x01   ADT */ *adv++ = 0xFF;
        /* 0x02  CoID */ HAPWriteLittleUInt16(adv, 0x004CU);
        adv += 2;
        /* 0x04    TY */ *adv++ = 0x11;
        /* 0x05   STL */ *adv++ = 0x36;
        /* 0x06 AdvID */ {
            HAPAssert(sizeof advertisingID.bytes == 6);
            HAPRawBufferCopyBytes(adv, advertisingID.bytes, sizeof advertisingID.bytes);
            adv += sizeof advertisingID.bytes;
        }
        uint8_t* encryptedBytes = adv;
        /* 0x0C   GSN */ HAPWriteLittleUInt16(adv, gsn.gsn);
        adv += 2;
        /* 0x0E   IID */ HAPWriteLittleUInt16(adv, server->ble.adv.broadcastedEvent.iid);
        adv += 2;
        /* 0x10 Value */ {
            HAPRawBufferCopyBytes(
                    adv, server->ble.adv.broadcastedEvent.value, sizeof server->ble.adv.broadcastedEvent.value);
            adv += sizeof server->ble.adv.broadcastedEvent.value;
        }
        /* 0x18   Tag */ {
            // See HomeKit Accessory Protocol Specification R14
            // Section 5.9 AEAD Algorithm.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.4.7.3 Broadcast Encryption Key Generation
            uint8_t tagBytes[CHACHA20_POLY1305_TAG_BYTES];
            uint8_t nonceBytes[] = { HAPExpandLittleUInt64((uint64_t) gsn.gsn) };
            HAP_chacha20_poly1305_encrypt_aad(
                    tagBytes,
                    encryptedBytes,
                    encryptedBytes,
                    (size_t)(adv - encryptedBytes),
                    advertisingID.bytes,
                    sizeof advertisingID.bytes,
                    nonceBytes,
                    sizeof nonceBytes,
                    broadcastKey.value);
            HAPRawBufferCopyBytes(adv, tagBytes, 4);
            adv += 4;
        }

        *numAdvertisingBytes = (size_t)(adv - (uint8_t*) advertisingBytes);
        HAPAssert(*numAdvertisingBytes <= maxAdvertisingBytes);

        // Log.
        adv = advertisingBytes;
        adv += 3;
        HAPLogInfo(
                &logObject,
                "HAP BLE Encrypted Notification Advertisement Format (Manufacturer Data).\n"
                "-   LEN = 0x%02X\n"
                "-   ADT = 0x%02X\n"
                "-  CoID = 0x%04X\n"
                "-    TY = 0x%02X\n"
                "-   STL = 0x%02X\n"
                "- AdvID = %02X:%02X:%02X:%02X:%02X:%02X\n"
                "-    Ev = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n"
                "          -   GSN = %u\n"
                "          -   IID = 0x%04X\n"
                "          - Value = 0x%02X%02X%02X%02X%02X%02X%02X%02X\n"
                "-   Tag = 0x%02X%02X%02X%02X",
                adv[0x00],
                adv[0x01],
                HAPReadLittleUInt16(&adv[0x02]),
                adv[0x04],
                adv[0x05],
                adv[0x06],
                adv[0x07],
                adv[0x08],
                adv[0x09],
                adv[0x0A],
                adv[0x0B],
                adv[0x0C],
                adv[0x0D],
                adv[0x0E],
                adv[0x0F],
                adv[0x10],
                adv[0x11],
                adv[0x12],
                adv[0x13],
                adv[0x14],
                adv[0x15],
                adv[0x16],
                adv[0x17],
                gsn.gsn,
                server->ble.adv.broadcastedEvent.iid,
                server->ble.adv.broadcastedEvent.value[0],
                server->ble.adv.broadcastedEvent.value[1],
                server->ble.adv.broadcastedEvent.value[2],
                server->ble.adv.broadcastedEvent.value[3],
                server->ble.adv.broadcastedEvent.value[4],
                server->ble.adv.broadcastedEvent.value[5],
                server->ble.adv.broadcastedEvent.value[6],
                server->ble.adv.broadcastedEvent.value[7],
                adv[0x18],
                adv[0x19],
                adv[0x1A],
                adv[0x1B]);
    } else {
        // HAP BLE Regular Advertisement Format.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.2.1 HAP BLE Regular Advertisement Format

        // Interval.
        // - 20 ms for first 30 seconds after boot.
        //   See Accessory Design Guidelines for Apple Devices R7
        //   Section 11.5 Advertising Interval
        // - 20 ms for first 3 seconds after Disconnected Event.
        //   See HomeKit Accessory Protocol Specification R14
        //   Section 7.4.6.3 Disconnected Events
        // - Regular advertising interval, otherwise.
        *advertisingInterval = (server->ble.adv.timer || !server->ble.adv.fast_started || server->ble.adv.fast_timer) ?
                                       HAPBLEAdvertisingIntervalCreateFromMilliseconds(20) :
                                       server->ble.adv.interval;

        uint8_t* adv = advertisingBytes;

        // Get setup ID.
        HAPSetupID setupID;
        bool hasSetupID = false;
        HAPPlatformAccessorySetupLoadSetupID(server->platform.accessorySetup, &hasSetupID, &setupID);

        // Flags.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.2.1.1 Flags
        /* 0x00   LEN */ *adv++ = 0x02;
        /* 0x01   ADT */ *adv++ = 0x01;
        /* 0x02 Flags */ *adv++ = 0U << 0U | //  NO: LE Limited Discoverable Mode.
                                  1U << 1U | // YES: LE General Discoverable Mode.
                                  1U << 2U | // YES: BR/EDR Not Supported.
                                  0U << 3U | //  NO: Simultaneous LE and BR/EDR to Same Device Capable (Controller).
                                  0U << 4U | //  NO: Simultaneous LE and BR/EDR to Same Device Capable (Host).
                                  0U << 5U | 0U << 6U | 0U << 7U; // Reserved.

        // Manufacturer data.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.2.1.2 Manufacturer Data
        /* 0x00   LEN */ *adv++ = (uint8_t)(0x12 + (hasSetupID ? 4 : 0));
        /* 0x01   ADT */ *adv++ = 0xFF;
        /* 0x02  CoID */ HAPWriteLittleUInt16(adv, 0x004CU);
        adv += 2;
        /* 0x04    TY */ *adv++ = 0x06;
        /* 0x05   STL */ *adv++ = (uint8_t)(0x2D + (hasSetupID ? 4 : 0));
        /* 0x06    SF */ *adv++ = (uint8_t)(HAPAccessoryServerIsPaired(server_) ? 0U << 0U : 1U << 0U);
        /* 0x07 DevID */ {
            HAPDeviceID deviceID;
            err = HAPDeviceIDGet(server->platform.keyValueStore, &deviceID);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
            HAPAssert(sizeof deviceID.bytes == 6);
            HAPRawBufferCopyBytes(adv, deviceID.bytes, sizeof deviceID.bytes);
            adv += sizeof deviceID.bytes;
        }
        /* 0x0D  ACID */ HAPWriteLittleUInt16(adv, (uint16_t) server->primaryAccessory->category);
        adv += 2;
        /* 0x0F   GSN */ {
            HAPBLEAccessoryServerGSN gsn;
            err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
            HAPWriteLittleUInt16(adv, gsn.gsn);
            adv += 2;
        }
        /* 0x11    CN */ {
            uint16_t cn;
            err = HAPAccessoryServerGetCN(server->platform.keyValueStore, &cn);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
            *adv++ = (uint8_t)((cn - 1) % UINT8_MAX + 1);
        }
        /* 0x12    CV */ *adv++ = 0x02;
        /* 0x13    SH */ {
            if (hasSetupID) {
                // Get Device ID string.
                HAPDeviceIDString deviceIDString;
                err = HAPDeviceIDGetAsString(server->platform.keyValueStore, &deviceIDString);
                if (err) {
                    HAPAssert(err == kHAPError_Unknown);
                    return err;
                }

                // Get setup hash.
                HAPAccessorySetupSetupHash setupHash;
                HAPAccessorySetupGetSetupHash(&setupHash, &setupID, &deviceIDString);

                // Append.
                HAPAssert(sizeof setupHash.bytes == 4);
                HAPRawBufferCopyBytes(adv, setupHash.bytes, sizeof setupHash.bytes);
                adv += sizeof setupHash.bytes;
            }
        }

        // Name.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.2.1.3 Local Name
        size_t numNameBytes = HAPStringGetNumBytes(server->primaryAccessory->name);
        HAPAssert(numNameBytes < UINT8_MAX);
        size_t maxNameBytesInAdvertisingData = maxAdvertisingBytes - (size_t)(adv - (uint8_t*) advertisingBytes) - 2;
        if (numNameBytes > maxNameBytesInAdvertisingData) {
            // When the advertisement includes the shortened local name
            // the accessory should include the complete local name in the Scan Response.
            if (maxScanResponseBytes >= 2) {
                uint8_t* sr = scanResponseBytes;

                size_t maxNameBytesInScanResponseData = maxScanResponseBytes - 2;
                if (numNameBytes > maxNameBytesInScanResponseData) {
                    numNameBytes = maxNameBytesInScanResponseData;
                    /* 0x00   LEN */ *sr++ = (uint8_t)(numNameBytes + 1);
                    /* 0x01   ADT */ *sr++ = 0x08;
                } else {
                    /* 0x00   LEN */ *sr++ = (uint8_t)(numNameBytes + 1);
                    /* 0x01   ADT */ *sr++ = 0x09;
                }
                /* 0x02  Name */ {
                    HAPRawBufferCopyBytes(sr, server->primaryAccessory->name, numNameBytes);
                    sr += numNameBytes;
                }

                *numScanResponseBytes = (size_t)(sr - (uint8_t*) scanResponseBytes);
                HAPAssert(*numScanResponseBytes <= maxScanResponseBytes);
            }

            numNameBytes = maxNameBytesInAdvertisingData;
            /* 0x00   LEN */ *adv++ = (uint8_t)(numNameBytes + 1);
            /* 0x01   ADT */ *adv++ = 0x08;
        } else {
            /* 0x00   LEN */ *adv++ = (uint8_t)(numNameBytes + 1);
            /* 0x01   ADT */ *adv++ = 0x09;
        }
        /* 0x02  Name */ {
            HAPRawBufferCopyBytes(adv, server->primaryAccessory->name, numNameBytes);
            adv += numNameBytes;
        }

        *numAdvertisingBytes = (size_t)(adv - (uint8_t*) advertisingBytes);
        HAPAssert(*numAdvertisingBytes <= maxAdvertisingBytes);

        // Log.
        adv = advertisingBytes;
        adv += 3;
        char setupHashLog[12 + 4 * 2 + 1 + 1];
        if (hasSetupID) {
            err = HAPStringWithFormat(
                    setupHashLog,
                    sizeof setupHashLog,
                    "\n-    SH = 0x%02X%02X%02X%02X",
                    adv[0x13],
                    adv[0x14],
                    adv[0x15],
                    adv[0x16]);
            HAPAssert(!err);
        }
        HAPLogInfo(
                &logObject,
                "HAP BLE Regular Advertisement Format (Manufacturer Data).\n"
                "-   LEN = 0x%02X\n"
                "-   ADT = 0x%02X\n"
                "-  CoID = 0x%04X\n"
                "-    TY = 0x%02X\n"
                "-   STL = 0x%02X\n"
                "-    SF = 0x%02X\n"
                "- DevID = %02X:%02X:%02X:%02X:%02X:%02X\n"
                "-  ACID = %u\n"
                "-   GSN = %u\n"
                "-    CN = %u\n"
                "-    CV = 0x%02X"
                "%s",
                adv[0x00],
                adv[0x01],
                HAPReadLittleUInt16(&adv[0x02]),
                adv[0x04],
                adv[0x05],
                adv[0x06],
                adv[0x07],
                adv[0x08],
                adv[0x09],
                adv[0x0A],
                adv[0x0B],
                adv[0x0C],
                HAPReadLittleUInt16(&adv[0x0D]),
                HAPReadLittleUInt16(&adv[0x0F]),
                adv[0x11],
                adv[0x12],
                hasSetupID ? setupHashLog : "");
    }

    float advertisingIntervalMilliseconds = HAPBLEAdvertisingIntervalGetMilliseconds(*advertisingInterval);
    HAPLogBufferInfo(
            &logObject,
            advertisingBytes,
            *numAdvertisingBytes,
            "ADV data: Active = %d, Interval = %u.%03u ms.",
            *isActive,
            (uint16_t) advertisingIntervalMilliseconds,
            (uint16_t)((uint32_t)(advertisingIntervalMilliseconds * 1000) % 1000));
    if (scanResponseBytes && *numScanResponseBytes) {
        HAPLogBufferInfo(&logObject, scanResponseBytes, *numScanResponseBytes, "SR data.");
    }

    return kHAPError_None;
}

static void AdvertisingTimerExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (timer == server->ble.adv.fast_timer) {
        HAPLogDebug(&logObject, "Fast advertisement timer expired.");
        server->ble.adv.fast_timer = 0;
    } else if (timer == server->ble.adv.timer) {
        HAPLogDebug(&logObject, "Advertisement timer expired.");
        server->ble.adv.timer = 0;
    } else {
        HAPPreconditionFailure();
    }

    HAPError err;

    HAPAssert(!server->ble.adv.connected);

    // If no controller connects to the accessory within the 3 second broadcast period then the accessory must fall
    // back to the Disconnected Events advertisement rule with its current GSN as specified in `Disconnected Events`.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.6.2 Broadcasted Events
    if (server->ble.adv.broadcastedEvent.iid && !server->ble.adv.timer) {
        HAPRawBufferZero(&server->ble.adv.broadcastedEvent, sizeof server->ble.adv.broadcastedEvent);

        // After updating the GSN as specified in Section `HAP BLE Regular Advertisement Format` in the disconnected
        // state the accessory must use a 20 ms advertising interval for at least 3 seconds.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.6.3 Disconnected Events
        err = HAPPlatformTimerRegister(
                &server->ble.adv.timer,
                HAPPlatformClockGetCurrent() + server->ble.adv.ev_duration,
                AdvertisingTimerExpired,
                server_);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPLog(&logObject, "Not enough resources to start disconnected event timer!");
        }
    }

    // Update advertisement parameters.
    HAPAccessoryServerUpdateAdvertisingData(server_);
}

void HAPBLEAccessoryServerDidStartAdvertising(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    // For first 30 seconds after boot, use 20ms as regular advertisement interval.
    if (!server->ble.adv.fast_started) {
        HAPAssert(!server->ble.adv.fast_timer);
        server->ble.adv.fast_started = true;
        err = HAPPlatformTimerRegister(
                &server->ble.adv.fast_timer,
                HAPPlatformClockGetCurrent() + 30 * HAPSecond,
                AdvertisingTimerExpired,
                server_);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPLog(&logObject,
                   "Not enough resources to start fast initial advertisement timer. Using regular interval!");
        }
    }
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerDidConnect(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(!server->ble.adv.connected);

    HAPError err;

    server->ble.adv.connected = true;

    // Stop fast advertisement timer.
    if (server->ble.adv.fast_timer) {
        HAPPlatformTimerDeregister(server->ble.adv.fast_timer);
        server->ble.adv.fast_timer = 0;
    }

    // Stop timer for disconnected and broadcasted events.
    // If a controller connects to the accessory before the completion of the 3 second advertising period the accessory
    // should abort the encrypted advertisement and continue with its regular advertisement at the regular advertising
    // period after the controller disconnects.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.6.2 Broadcasted Events
    if (server->ble.adv.timer) {
        HAPPlatformTimerDeregister(server->ble.adv.timer);
        server->ble.adv.timer = 0;
    }

    // Reset disconnected events coalescing.
    HAPBLEAccessoryServerGSN gsn;
    err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    gsn.didIncrement = false;
    uint8_t gsnBytes[] = { HAPExpandLittleUInt16(gsn.gsn), gsn.didIncrement ? (uint8_t) 0x01 : (uint8_t) 0x00 };
    err = HAPPlatformKeyValueStoreSet(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_BLEGSN,
            gsnBytes,
            sizeof gsnBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // Reset broadcasted events.
    HAPRawBufferZero(&server->ble.adv.broadcastedEvent, sizeof server->ble.adv.broadcastedEvent);

    // Update advertisement parameters.
    HAPAccessoryServerUpdateAdvertisingData(server_);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerDidDisconnect(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->ble.adv.connected);

    HAPError err;

    server->ble.adv.connected = false;

    HAPAssert(server->ble.adv.fast_started);
    HAPAssert(!server->ble.adv.fast_timer);
    HAPAssert(!server->ble.adv.timer);

    // Allow quick reconnection.
    err = HAPPlatformTimerRegister(
            &server->ble.adv.fast_timer,
            HAPPlatformClockGetCurrent() + server->ble.adv.ev_duration,
            AdvertisingTimerExpired,
            server_);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLog(&logObject, "Not enough resources to start quick reconnection timer. Using regular interval!");
    }

    // Reset GSN update coalescing.
    HAPBLEAccessoryServerGSN gsn;
    err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    gsn.didIncrement = false;
    uint8_t gsnBytes[] = { HAPExpandLittleUInt16(gsn.gsn), gsn.didIncrement ? (uint8_t) 0x01 : (uint8_t) 0x00 };
    err = HAPPlatformKeyValueStoreSet(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_BLEGSN,
            gsnBytes,
            sizeof gsnBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    HAPAssert(!server->ble.adv.broadcastedEvent.iid);

    // Update advertisement parameters.
    HAPAccessoryServerUpdateAdvertisingData(server_);

    // Proceed with shutdown, if requested.
    if (server->state != kHAPAccessoryServerState_Running) {
        HAPLogInfo(&logObject, "BLE connection disconnected. Proceeding with shutdown.");
        HAPAccessoryServerStop(server_);
    }
    return kHAPError_None;
}

/**
 * Increments GSN, invalidating broadcast encryption key if necessary.
 *
 * @param      server_              Accessory server.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPBLEAccessoryServerIncrementGSN(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    // Get key expiration GSN.
    uint16_t keyExpirationGSN;
    err = HAPBLEAccessoryServerBroadcastGetParameters(server->platform.keyValueStore, &keyExpirationGSN, NULL, NULL);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // Get GSN.
    HAPBLEAccessoryServerGSN gsn;
    err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // Expire broadcast encryption key if necessary.
    if (gsn.gsn == keyExpirationGSN) {
        err = HAPBLEAccessoryServerBroadcastExpireKey(server->platform.keyValueStore);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }

    // Increment GSN.
    if (gsn.gsn == UINT16_MAX) {
        gsn.gsn = 1;
    } else {
        gsn.gsn++;
    }
    gsn.didIncrement = true;
    HAPLogInfo(&logObject, "New GSN: %u.", gsn.gsn);

    // Save GSN state.
    uint8_t gsnBytes[] = { HAPExpandLittleUInt16(gsn.gsn), gsn.didIncrement ? (uint8_t) 0x01 : (uint8_t) 0x00 };
    err = HAPPlatformKeyValueStoreSet(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_BLEGSN,
            gsnBytes,
            sizeof gsnBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerDidRaiseEvent(
        HAPAccessoryServerRef* server_,
        const HAPCharacteristic* characteristic_,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPSessionRef* _Nullable session) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(accessory->aid == 1);

    HAPError err;

    if (characteristic->properties.supportsEventNotification) {
        // Connected event.
        if (server->ble.connection.connected && (!session || session == server->ble.storage->session)) {
            const HAPCharacteristic* writtenCharacteristic = server->ble.connection.write.characteristic;
            const HAPService* writtenService = server->ble.connection.write.service;
            const HAPAccessory* writtenAccessory = server->ble.connection.write.accessory;
            if (characteristic_ == writtenCharacteristic && service == writtenService &&
                accessory == writtenAccessory) {
                HAPLogCharacteristicInfo(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Suppressing notification as the characteristic is currently being written.");
            } else {
                HAPBLEPeripheralManagerRaiseEvent(server_, characteristic_, service, accessory);
            }
        }
    }

    if (characteristic->properties.ble.supportsBroadcastNotification) {
        // Broadcasted event.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.6.2 Broadcasted Events

        // If a controller connects to the accessory before the completion of the 3 second advertising period the
        // accessory should abort the encrypted advertisement and continue with its regular advertisement at the regular
        // advertising period after the controller disconnects.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.6.2 Broadcasted Events
        if (!server->ble.adv.connected) {
            uint16_t keyExpirationGSN;
            err = HAPBLEAccessoryServerBroadcastGetParameters(
                    server->platform.keyValueStore, &keyExpirationGSN, NULL, NULL);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
            HAPBLEAccessoryServerGSN gsn;
            err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }

            // Characteristic changes while in a broadcast encryption key expired state shall not use broadcasted events
            // and must fall back to disconnected/connected events until the controller has re-generated a new broadcast
            // encryption key and re-registered characteristics for broadcasted notification.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.4.7.4 Broadcast Encryption Key expiration and refresh
            if (keyExpirationGSN && keyExpirationGSN != gsn.gsn) {
                HAPBLECharacteristicBroadcastInterval interval;
                bool enabled;
                err = HAPBLECharacteristicGetBroadcastConfiguration(
                        characteristic, service, accessory, &enabled, &interval, server->platform.keyValueStore);
                if (err) {
                    HAPAssert(err == kHAPError_Unknown);
                    return err;
                }

                if (enabled) {
                    // For additional characteristic changes before the completion of the 3 second period and before
                    // a controller connection, the GSN should be updated again and the accessory must reflect the
                    // latest changed characteristic value in its encrypted advertisement and continue to broadcast
                    // for an additional 3 seconds from the last change.
                    // See HomeKit Accessory Protocol Specification R14
                    // Section 7.4.6.2 Broadcasted Events
                    if (server->ble.adv.timer) {
                        HAPPlatformTimerDeregister(server->ble.adv.timer);
                        server->ble.adv.timer = 0;
                    }

                    // Cancel current broadcast.
                    server->ble.adv.broadcastedEvent.iid = 0;
                    HAPRawBufferZero(
                            server->ble.adv.broadcastedEvent.value, sizeof server->ble.adv.broadcastedEvent.value);
                    HAPAccessoryServerUpdateAdvertisingData(server_);

                    // Fetch characteristic value.
                    // When the characteristic value is less than 8 bytes the remaining bytes shall be set to 0.
                    // See HomeKit Accessory Protocol Specification R14
                    // Section 7.4.2.2.2 Manufacturer Data
                    err = kHAPError_Unknown;
                    uint8_t* bytes = server->ble.adv.broadcastedEvent.value;
                    HAPAssert(sizeof server->ble.adv.broadcastedEvent.value == 8);
                    switch (characteristic->format) {
                        case kHAPCharacteristicFormat_Data: {
                            // Characteristics with format of string or data/tlv8 cannot be used
                            // in broadcast notifications
                            // See HomeKit Accessory Protocol Specification R14
                            // Section 7.4.2.2.2 Manufacturer Data
                            HAPLogCharacteristicError(
                                    &logObject,
                                    characteristic,
                                    service,
                                    accessory,
                                    "%s characteristic cannot be used in broadcast notifications.",
                                    "Data");
                        } break;
                        case kHAPCharacteristicFormat_Bool: {
                            bool value;
                            err = HAPBoolCharacteristicHandleRead(
                                    server_,
                                    &(const HAPBoolCharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                                .session = NULL,
                                                                                .characteristic = characteristic_,
                                                                                .service = service,
                                                                                .accessory = accessory },
                                    &value,
                                    server->context);
                            if (err) {
                                break;
                            }
                            bytes[0] = (uint8_t)(value ? 1 : 0);
                        } break;
                        case kHAPCharacteristicFormat_UInt8: {
                            uint8_t value;
                            err = HAPUInt8CharacteristicHandleRead(
                                    server_,
                                    &(const HAPUInt8CharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                                 .session = NULL,
                                                                                 .characteristic = characteristic_,
                                                                                 .service = service,
                                                                                 .accessory = accessory },
                                    &value,
                                    server->context);
                            if (err) {
                                break;
                            }
                            bytes[0] = value;
                        } break;
                        case kHAPCharacteristicFormat_UInt16: {
                            uint16_t value;
                            err = HAPUInt16CharacteristicHandleRead(
                                    server_,
                                    &(const HAPUInt16CharacteristicReadRequest) { .transportType =
                                                                                          kHAPTransportType_BLE,
                                                                                  .session = NULL,
                                                                                  .characteristic = characteristic_,
                                                                                  .service = service,
                                                                                  .accessory = accessory },
                                    &value,
                                    server->context);
                            if (err) {
                                break;
                            }
                            HAPWriteLittleUInt16(bytes, value);
                        } break;
                        case kHAPCharacteristicFormat_UInt32: {
                            uint32_t value;
                            err = HAPUInt32CharacteristicHandleRead(
                                    server_,
                                    &(const HAPUInt32CharacteristicReadRequest) { .transportType =
                                                                                          kHAPTransportType_BLE,
                                                                                  .session = NULL,
                                                                                  .characteristic = characteristic_,
                                                                                  .service = service,
                                                                                  .accessory = accessory },
                                    &value,
                                    server->context);
                            if (err) {
                                break;
                            }
                            HAPWriteLittleUInt32(bytes, value);
                        } break;
                        case kHAPCharacteristicFormat_UInt64: {
                            uint64_t value;
                            err = HAPUInt64CharacteristicHandleRead(
                                    server_,
                                    &(const HAPUInt64CharacteristicReadRequest) { .transportType =
                                                                                          kHAPTransportType_BLE,
                                                                                  .session = NULL,
                                                                                  .characteristic = characteristic_,
                                                                                  .service = service,
                                                                                  .accessory = accessory },
                                    &value,
                                    server->context);
                            if (err) {
                                break;
                            }
                            HAPWriteLittleUInt64(bytes, value);
                        } break;
                        case kHAPCharacteristicFormat_Int: {
                            int32_t value;
                            err = HAPIntCharacteristicHandleRead(
                                    server_,
                                    &(const HAPIntCharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                               .session = NULL,
                                                                               .characteristic = characteristic_,
                                                                               .service = service,
                                                                               .accessory = accessory },
                                    &value,
                                    server->context);
                            if (err) {
                                break;
                            }
                            HAPWriteLittleInt32(bytes, value);
                        } break;
                        case kHAPCharacteristicFormat_Float: {
                            float value;
                            err = HAPFloatCharacteristicHandleRead(
                                    server_,
                                    &(const HAPFloatCharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                                 .session = NULL,
                                                                                 .characteristic = characteristic_,
                                                                                 .service = service,
                                                                                 .accessory = accessory },
                                    &value,
                                    server->context);
                            if (err) {
                                break;
                            }
                            uint32_t bitPattern = HAPFloatGetBitPattern(value);
                            HAPWriteLittleUInt32(bytes, bitPattern);
                        } break;
                        case kHAPCharacteristicFormat_String: {
                            // Characteristics with format of string or data/tlv8 cannot be used
                            // in broadcast notifications.
                            // See HomeKit Accessory Protocol Specification R14
                            // Section 7.4.2.2.2 Manufacturer Data
                            HAPLogCharacteristicError(
                                    &logObject,
                                    characteristic,
                                    service,
                                    accessory,
                                    "%s characteristic cannot be used in broadcast notifications.",
                                    "String");
                        }
                            HAPFatalError();
                        case kHAPCharacteristicFormat_TLV8: {
                            // Characteristics with format of string or data/tlv8 cannot be used
                            // in broadcast notifications".
                            // See HomeKit Accessory Protocol Specification R14
                            // Section 7.4.2.2.2 Manufacturer Data
                            HAPLogCharacteristicError(
                                    &logObject,
                                    characteristic,
                                    service,
                                    accessory,
                                    "%s characteristic cannot be used in broadcast notifications.",
                                    "TLV8");
                        }
                            HAPFatalError();
                    }
                    if (err) {
                        HAPAssert(
                                err == kHAPError_Unknown || err == kHAPError_InvalidState ||
                                err == kHAPError_OutOfResources || err == kHAPError_Busy);
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "Value for broadcast notification could not be received. Skipping event!");
                    } else {
                        // Increment GSN.
                        err = HAPBLEAccessoryServerIncrementGSN(server_);
                        if (err) {
                            HAPAssert(err == kHAPError_Unknown);
                            return err;
                        }

                        err = HAPPlatformTimerRegister(
                                &server->ble.adv.timer,
                                HAPPlatformClockGetCurrent() + server->ble.adv.ev_duration,
                                AdvertisingTimerExpired,
                                server_);
                        if (err) {
                            HAPAssert(err == kHAPError_OutOfResources);
                            HAPLogCharacteristicError(
                                    &logObject,
                                    characteristic,
                                    service,
                                    accessory,
                                    "Not enough resources to start broadcast event timer. Skipping event!");
                        } else {
                            // Initialize broadcasted event.
                            server->ble.adv.broadcastedEvent.interval = interval;
                            HAPAssert(characteristic->iid <= UINT16_MAX);
                            server->ble.adv.broadcastedEvent.iid = (uint16_t) characteristic->iid;
                            HAPLogCharacteristicInfo(
                                    &logObject, characteristic, service, accessory, "Broadcasted Event.");
                        }
                    }

                    // Update advertisement parameters.
                    HAPAccessoryServerUpdateAdvertisingData(server_);
                    return kHAPError_None;
                } else {
                    HAPLogCharacteristicInfo(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Broadcasted Event - Skipping: Broadcasts disabled.");
                }
            } else {
                HAPLogCharacteristicInfo(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Broadcasted Event - Skipping: Broadcast Key expired.");
            }
        } else {
            HAPLogCharacteristicInfo(
                    &logObject, characteristic, service, accessory, "Broadcasted Event - Skipping: Connected.");
        }
    }

    if (characteristic->properties.ble.supportsDisconnectedNotification) {
        // Disconnected event.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.6.3 Disconnected Events

        HAPBLEAccessoryServerGSN gsn;
        err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }

        // The GSN should increment only once for multiple characteristic value changes while in in disconnected state
        // until the accessory state changes from disconnected to connected.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.6.3 Disconnected Events
        if (!gsn.didIncrement) {
            HAPAssert(!server->ble.adv.broadcastedEvent.iid);
            HAPAssert(!server->ble.adv.timer);

            err = HAPBLEAccessoryServerIncrementGSN(server_);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }

            if (!server->ble.adv.connected) {
                HAPLogCharacteristicInfo(&logObject, characteristic, service, accessory, "Disconnected Event.");

                // After updating the GSN as specified in Section HAP BLE Regular Advertisement Format in the
                // disconnected state the accessory must use a 20 ms advertising interval for at least 3 seconds.
                // See HomeKit Accessory Protocol Specification R14
                // Section 7.4.6.3 Disconnected Events
                err = HAPPlatformTimerRegister(
                        &server->ble.adv.timer,
                        HAPPlatformClockGetCurrent() + server->ble.adv.ev_duration,
                        AdvertisingTimerExpired,
                        server_);
                if (err) {
                    HAPAssert(err == kHAPError_OutOfResources);
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Not enough resources to start disconnected event timer!");
                }

                // Update advertisement parameters.
                HAPAccessoryServerUpdateAdvertisingData(server_);
                return kHAPError_None;
            } else {
                HAPLogCharacteristicInfo(
                        &logObject, characteristic, service, accessory, "Disconnected Event - Connected (no adv).");
            }
        } else {
            HAPLogCharacteristicInfo(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Disconnected Event - Skipping: GSN already incremented.");
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerDidSendEventNotification(
        HAPAccessoryServerRef* server_,
        const HAPCharacteristic* characteristic_,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->ble.adv.connected);
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(accessory->aid == 1);

    HAPError err;

    // After the first characteristic change on characteristics that are registered for Bluetooth LE indications in the
    // current connected state, the GSN shall also be incremented by 1 and reflected in the subsequent advertisements
    // after the current connection is disconnected. The GSN must increment only once for multiple characteristic
    // changes while in the current connected state.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.6.1 Connected Events
    HAPBLEAccessoryServerGSN gsn;
    err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // The GSN should increment only once for multiple characteristic value changes while in in disconnected state
    // until the accessory state changes from disconnected to connected.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.6.3 Disconnected Events
    if (!gsn.didIncrement) {
        HAPAssert(!server->ble.adv.broadcastedEvent.iid);
        HAPAssert(!server->ble.adv.timer);

        err = HAPBLEAccessoryServerIncrementGSN(server_);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }

        // Not connected, so no advertisement parameter update necessary.
        HAPLogCharacteristicInfo(&logObject, characteristic, service, accessory, "Connected Event - GSN incremented.");
    } else {
        HAPLogCharacteristicInfo(
                &logObject, characteristic, service, accessory, "Connected Event - Skipping: GSN already incremented.");
    }

    return kHAPError_None;
}
