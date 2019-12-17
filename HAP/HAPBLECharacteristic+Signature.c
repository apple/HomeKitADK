// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicGetSignatureReadResponse(
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(responseWriter);

    HAPError err;

    err = HAPBLEPDUTLVSerializeCharacteristicType(characteristic, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeServiceInstanceID(service, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeServiceType(service, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeHAPCharacteristicPropertiesDescriptor(characteristic, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeGATTUserDescriptionDescriptor(characteristic, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeGATTPresentationFormatDescriptor(characteristic, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeGATTValidRange(characteristic, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeHAPStepValueDescriptor(characteristic, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeHAPValidValuesDescriptor(characteristic, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeHAPValidValuesRangeDescriptor(characteristic, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}
