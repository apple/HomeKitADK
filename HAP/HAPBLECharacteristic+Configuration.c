// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLECharacteristic" };

/**
 * Characteristic configuration parameter types.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-28 Characteristic configuration parameter types
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLECharacteristicConfigurationTLVType) {
    /** HAP-Characteristic-Configuration-Param-Properties. */
    kHAPBLECharacteristicConfigurationTLVType_Properties = 0x01,

    /** HAP-Characteristic-Configuration-Param-Broadcast-Interval. */
    kHAPBLECharacteristicConfigurationTLVType_BroadcastInterval = 0x02
} HAP_ENUM_END(uint8_t, HAPBLECharacteristicConfigurationTLVType);

/**
 * Characteristic configuration properties.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-29 Characteristic configuration properties
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLECharacteristicConfigurationProperty) {
    /** Enable/Disable Broadcast Notification. */
    kHAPBLECharacteristicConfigurationProperty_EnableBroadcasts = 0x0001
} HAP_ENUM_END(uint8_t, HAPBLECharacteristicConfigurationProperty);

HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicHandleConfigurationRequest(
        const HAPCharacteristic* characteristic_,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVReaderRef* requestReader,
        HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(requestReader);
    HAPPrecondition(keyValueStore);

    HAPError err;

    HAPTLV propertiesTLV, broadcastIntervalTLV;
    propertiesTLV.type = kHAPBLECharacteristicConfigurationTLVType_Properties;
    broadcastIntervalTLV.type = kHAPBLECharacteristicConfigurationTLVType_BroadcastInterval;
    err = HAPTLVReaderGetAll(requestReader, (HAPTLV* const[]) { &propertiesTLV, &broadcastIntervalTLV, NULL });
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }

    // HAP-Characteristic-Configuration-Param-Properties.
    if (propertiesTLV.value.bytes) {
        if (propertiesTLV.value.numBytes != 2) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "HAP-Characteristic-Configuration-Param-Properties has invalid length (%lu).",
                    (unsigned long) propertiesTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
        uint16_t properties = HAPReadLittleUInt16(propertiesTLV.value.bytes);
        uint16_t allProperties = kHAPBLECharacteristicConfigurationProperty_EnableBroadcasts;
        if (properties & (uint16_t) ~allProperties) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "HAP-Characteristic-Configuration-Param-Properties invalid: %u.",
                    properties);
            return kHAPError_InvalidData;
        }

        // Broadcast notifications.
        if (properties & kHAPBLECharacteristicConfigurationProperty_EnableBroadcasts) {
            // HAP-Characteristic-Configuration-Param-Broadcast-Interval.
            HAPBLECharacteristicBroadcastInterval broadcastInterval = kHAPBLECharacteristicBroadcastInterval_20Ms;
            if (broadcastIntervalTLV.value.bytes) {
                if (broadcastIntervalTLV.value.numBytes != 1) {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "HAP-Characteristic-Configuration-Param-Broadcast-Interval has invalid length (%lu).",
                            (unsigned long) broadcastIntervalTLV.value.numBytes);
                    return kHAPError_InvalidData;
                }
                uint8_t value = ((const uint8_t*) broadcastIntervalTLV.value.bytes)[0];
                if (!HAPBLECharacteristicIsValidBroadcastInterval(value)) {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "HAP-Characteristic-Configuration-Param-Broadcast-Interval invalid: %u.",
                            broadcastInterval);
                    return kHAPError_InvalidData;
                }
                broadcastInterval = (HAPBLECharacteristicBroadcastInterval) value;
            }

            // Check that characteristic supports broadcasts.
            if (!characteristic->properties.ble.supportsBroadcastNotification) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Controller requested enabling broadcasts on characteristic that does not support it.");
                return kHAPError_InvalidData;
            }

            // Enable broadcasts.
            err = HAPBLECharacteristicEnableBroadcastNotifications(
                    characteristic, service, accessory, broadcastInterval, keyValueStore);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
        } else {
            // HAP-Characteristic-Configuration-Param-Broadcast-Interval.
            if (broadcastIntervalTLV.value.bytes) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Excess HAP-Characteristic-Configuration-Param-Broadcast-Interval (disabling broadcasts).");
                return kHAPError_InvalidData;
            }

            // Disable broadcasts if characteristic supports broadcasts.
            if (characteristic->properties.ble.supportsBroadcastNotification) {
                err = HAPBLECharacteristicDisableBroadcastNotifications(
                        characteristic, service, accessory, keyValueStore);
                if (err) {
                    HAPAssert(err == kHAPError_Unknown);
                    return err;
                }
            }
        }
    } else {
        // HAP-Characteristic-Configuration-Param-Broadcast-Interval.
        if (broadcastIntervalTLV.value.bytes) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Excess HAP-Characteristic-Configuration-Param-Broadcast-Interval (no properties present).");
            return kHAPError_InvalidData;
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicGetConfigurationResponse(
        const HAPCharacteristic* characteristic_,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVWriterRef* responseWriter,
        HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(responseWriter);
    HAPPrecondition(keyValueStore);

    HAPError err;
    uint16_t properties = 0;
    if (characteristic->properties.ble.supportsBroadcastNotification) {
        HAPBLECharacteristicBroadcastInterval broadcastInterval;
        bool broadcastsEnabled;
        err = HAPBLECharacteristicGetBroadcastConfiguration(
                characteristic, service, accessory, &broadcastsEnabled, &broadcastInterval, keyValueStore);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }

        if (broadcastsEnabled) {
            properties |= kHAPBLECharacteristicConfigurationProperty_EnableBroadcasts;

            // HAP-Characteristic-Configuration-Param-Broadcast-Interval.
            // The accessory must include all parameters in the response even if the default Broadcast Interval is used.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.4.15 HAP-Characteristic-Configuration-Response
            uint8_t broadcastIntervalBytes[] = { broadcastInterval };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) {
                            .type = kHAPBLECharacteristicConfigurationTLVType_BroadcastInterval,
                            .value = { .bytes = broadcastIntervalBytes, .numBytes = sizeof broadcastIntervalBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
    }

    // HAP-Characteristic-Configuration-Param-Properties.
    uint8_t propertiesBytes[] = { HAPExpandLittleUInt16(properties) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLECharacteristicConfigurationTLVType_Properties,
                              .value = { .bytes = propertiesBytes, .numBytes = sizeof propertiesBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}
