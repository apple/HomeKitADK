// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
bool HAPBLECharacteristicSupportsServiceProcedures(const HAPCharacteristic* characteristic_) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;

    return HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_ServiceSignature);
}

bool HAPBLECharacteristicDropsSecuritySession(const HAPCharacteristic* characteristic_) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;

    return HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_PairSetup) ||
           HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_PairVerify) ||
           HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_PairingFeatures);
}
