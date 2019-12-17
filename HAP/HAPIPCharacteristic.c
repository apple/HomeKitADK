// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
bool HAPIPCharacteristicIsSupported(const HAPCharacteristic* characteristic_) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;

    return !HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_ServiceSignature);
}

HAP_RESULT_USE_CHECK
size_t HAPCharacteristicGetNumEnabledProperties(const HAPCharacteristic* characteristic_) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;

    // See HomeKit Accessory Protocol Specification R14
    // Section 6.3.3 Characteristic Objects
    return (characteristic->properties.readable ? 1 : 0) + (characteristic->properties.writable ? 1 : 0) +
           (characteristic->properties.supportsEventNotification ? 1 : 0) +
           (characteristic->properties.supportsAuthorizationData ? 1 : 0) +
           (characteristic->properties.requiresTimedWrite ? 1 : 0) +
           (characteristic->properties.ip.supportsWriteResponse ? 1 : 0) + (characteristic->properties.hidden ? 1 : 0);
}

HAP_RESULT_USE_CHECK
HAPCharacteristicUnits HAPCharacteristicGetUnit(const HAPCharacteristic* characteristic_) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;

    // See HomeKit Accessory Protocol Specification R14
    // Section 6.3.3 Characteristic Objects
    switch (characteristic->format) {
        case kHAPCharacteristicFormat_UInt8: {
            return ((const HAPUInt8Characteristic*) characteristic)->units;
        }
        case kHAPCharacteristicFormat_UInt16: {
            return ((const HAPUInt16Characteristic*) characteristic)->units;
        }
        case kHAPCharacteristicFormat_UInt32: {
            return ((const HAPUInt32Characteristic*) characteristic)->units;
        }
        case kHAPCharacteristicFormat_UInt64: {
            return ((const HAPUInt64Characteristic*) characteristic)->units;
        }
        case kHAPCharacteristicFormat_Int: {
            return ((const HAPIntCharacteristic*) characteristic)->units;
        }
        case kHAPCharacteristicFormat_Float: {
            return ((const HAPFloatCharacteristic*) characteristic)->units;
        }
        case kHAPCharacteristicFormat_Bool:
        case kHAPCharacteristicFormat_String:
        case kHAPCharacteristicFormat_TLV8:
        case kHAPCharacteristicFormat_Data: {
            return kHAPCharacteristicUnits_None;
        }
    }
    HAPFatalError();
}
