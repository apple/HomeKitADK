// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "Characteristic" };

bool HAPCharacteristicReadRequiresAdminPermissions(const HAPCharacteristic* characteristic_) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;

    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_ARMCC(2570)
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe1444)
    bool readRequiresAdminPermissions = characteristic->properties.requiresAdminPermissions ||
                                        characteristic->properties.readRequiresAdminPermissions;
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe1444)
    HAP_DIAGNOSTIC_POP

    return readRequiresAdminPermissions;
}

bool HAPCharacteristicWriteRequiresAdminPermissions(const HAPCharacteristic* characteristic_) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;

    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_ARMCC(2570)
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe1444)
    bool writeRequiresAdminPermissions = characteristic->properties.requiresAdminPermissions ||
                                         characteristic->properties.writeRequiresAdminPermissions;
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe1444)
    HAP_DIAGNOSTIC_POP

    return writeRequiresAdminPermissions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define IS_VALUE_IN_RANGE(value, constraints) \
    ((value) >= (constraints).minimumValue && (value) <= (constraints).maximumValue && \
     (!(constraints).stepValue || ((value) - (constraints).minimumValue) % (constraints).stepValue == 0))

#define IS_VALUE_IN_RANGE_WITH_TOLERANCE(value, constraints, tolerance) \
    ((value) >= (constraints).minimumValue && (value) <= (constraints).maximumValue && \
     (HAPFloatIsZero((constraints).stepValue) || \
      HAPFloatGetAbsoluteValue( \
              HAPFloatGetFraction(((value) - (constraints).minimumValue) / (constraints).stepValue + 0.5f) - 0.5f) <= \
              (tolerance)))

#define IS_LENGTH_IN_RANGE(length, constraints) ((length) <= (constraints).maxLength)

#define ROUND_VALUE_TO_STEP(value, constraints) \
    ((value) - (HAPFloatGetFraction(((value) - (constraints).minimumValue) / (constraints).stepValue + 0.5f) - 0.5f) * \
                       (constraints).stepValue)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool HAPDataCharacteristicIsValueFulfillingConstraints(
        const HAPDataCharacteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        size_t numValueBytes) {
    if (!IS_LENGTH_IN_RANGE(numValueBytes, characteristic->constraints)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Value too long: %zu bytes (constraints: maxLength = %lu bytes).",
                numValueBytes,
                (unsigned long) characteristic->constraints.maxLength);
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPDataCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPDataCharacteristicReadRequest* request,
        void* valueBytes,
        size_t maxValueBytes,
        size_t* numValueBytes,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Data);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(valueBytes);
    HAPPrecondition(numValueBytes);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(
            server, request, valueBytes, maxValueBytes, numValueBytes, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }
    if (*numValueBytes > maxValueBytes) {
        HAPLogCharacteristicError(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read data value exceeds available buffer space (%zu bytes / available %zu bytes).",
                *numValueBytes,
                maxValueBytes);
        HAPFatalError();
    }

    // Validate constraints.
    HAPAssert(HAPDataCharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, *numValueBytes));

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPDataCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPDataCharacteristicWriteRequest* request,
        const void* valueBytes,
        size_t numValueBytes,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Data);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);
    HAPPrecondition(valueBytes);

    HAPError err;

    // Validate constraints.
    if (!HAPDataCharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, numValueBytes)) {
        return kHAPError_InvalidData;
    }

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, valueBytes, numValueBytes, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPDataCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPDataCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Data);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPDataCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPDataCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Data);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool HAPBoolCharacteristicIsValueFulfillingConstraints(
        const HAPBoolCharacteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        bool value) {
    if (value != false && value != true) {
        HAPLogCharacteristic(&logObject, characteristic, service, accessory, "Value invalid: %u.", value);
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPBoolCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicReadRequest* request,
        bool* value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Bool);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    // Validate constraints.
    HAPAssert(HAPBoolCharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, *value));

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBoolCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicWriteRequest* request,
        bool value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Bool);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);

    HAPError err;

    // Validate constraints.
    if (!HAPBoolCharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, value)) {
        return kHAPError_InvalidData;
    }

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPBoolCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Bool);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPBoolCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Bool);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool IsInValidValues(uint8_t value, const uint8_t* const* validValues) {
    if (validValues != NULL) {
        size_t i = 0;
        while (validValues[i] && *validValues[i] != value) {
            i++;
        }
        if (validValues[i] == NULL) {
            return false;
        }
    }
    return true;
}

HAP_RESULT_USE_CHECK
static bool
        IsInValidValuesRanges(uint8_t value, const HAPUInt8CharacteristicValidValuesRange* const* validValuesRanges) {
    if (validValuesRanges != NULL) {
        size_t i = 0;
        while (validValuesRanges[i] && (value < validValuesRanges[i]->start || value > validValuesRanges[i]->end)) {
            i++;
        }
        if (validValuesRanges[i] == NULL) {
            return false;
        }
    }
    return true;
}

HAP_RESULT_USE_CHECK
static bool HAPUInt8CharacteristicIsValueFulfillingConstraints(
        const HAPUInt8Characteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        uint8_t value) {
    if (!IS_VALUE_IN_RANGE(value, characteristic->constraints)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Value out of range: %u (constraints: minimumValue = %u / maximumValue = %u / stepValue = %u).",
                value,
                characteristic->constraints.minimumValue,
                characteristic->constraints.maximumValue,
                characteristic->constraints.stepValue);
        return false;
    }
    if (HAPUUIDIsAppleDefined(characteristic->characteristicType)) {
        if (!characteristic->constraints.validValues && !characteristic->constraints.validValuesRanges) {
            return true;
        }
        if (characteristic->constraints.validValues &&
            IsInValidValues(value, characteristic->constraints.validValues)) {
            return true;
        }
        if (characteristic->constraints.validValuesRanges &&
            IsInValidValuesRanges(value, characteristic->constraints.validValuesRanges)) {
            return true;
        }
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                request->service,
                accessory,
                "Value not supported: %u (constraints: validValues / validValuesRanges).",
                value);
        return false;
    } else {
        HAPAssert(!characteristic->constraints.validValues);
        HAPAssert(!characteristic->constraints.validValuesRanges);
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt8CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicReadRequest* request,
        uint8_t* value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt8);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    // Validate constraints.
    HAPAssert(HAPUInt8CharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, *value));

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt8CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicWriteRequest* request,
        uint8_t value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt8);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);

    HAPError err;

    // Validate constraints.
    if (!HAPUInt8CharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, value)) {
        return kHAPError_InvalidData;
    }

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPUInt8CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt8);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPUInt8CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt8);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool HAPUInt16CharacteristicIsValueFulfillingConstraints(
        const HAPUInt16Characteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        uint16_t value) {
    if (!IS_VALUE_IN_RANGE(value, characteristic->constraints)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Value out of range: %u (constraints: minimumValue = %u / maximumValue = %u / stepValue = %u).",
                value,
                characteristic->constraints.minimumValue,
                characteristic->constraints.maximumValue,
                characteristic->constraints.stepValue);
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt16CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPUInt16CharacteristicReadRequest* request,
        uint16_t* value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt16);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    // Validate constraints.
    HAPAssert(HAPUInt16CharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, *value));

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt16CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt16CharacteristicWriteRequest* request,
        uint16_t value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt16);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);

    HAPError err;

    // Validate constraints.
    if (!HAPUInt16CharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, value)) {
        return kHAPError_InvalidData;
    }

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPUInt16CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt16CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt16);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPUInt16CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt16CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt16);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool HAPUInt32CharacteristicIsValueFulfillingConstraints(
        const HAPUInt32Characteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        uint32_t value) {
    if (!IS_VALUE_IN_RANGE(value, characteristic->constraints)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Value out of range: %lu (constraints: minimumValue = %lu / maximumValue = %lu / stepValue = %lu).",
                (unsigned long) value,
                (unsigned long) characteristic->constraints.minimumValue,
                (unsigned long) characteristic->constraints.maximumValue,
                (unsigned long) characteristic->constraints.stepValue);
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt32CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPUInt32CharacteristicReadRequest* request,
        uint32_t* value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt32);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    // Validate constraints.
    HAPAssert(HAPUInt32CharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, *value));

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt32CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt32CharacteristicWriteRequest* request,
        uint32_t value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt32);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);

    HAPError err;

    // Validate constraints.
    if (!HAPUInt32CharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, value)) {
        return kHAPError_InvalidData;
    }

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPUInt32CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt32CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt32);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPUInt32CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt32CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt32);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool HAPUInt64CharacteristicIsValueFulfillingConstraints(
        const HAPUInt64Characteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        uint64_t value) {
    if (!IS_VALUE_IN_RANGE(value, characteristic->constraints)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Value out of range: %llu (constraints: minimumValue = %llu / maximumValue = %llu / stepValue = %llu).",
                (unsigned long long) value,
                (unsigned long long) characteristic->constraints.minimumValue,
                (unsigned long long) characteristic->constraints.maximumValue,
                (unsigned long long) characteristic->constraints.stepValue);
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt64CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPUInt64CharacteristicReadRequest* request,
        uint64_t* value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt64);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    // Validate constraints.
    HAPAssert(HAPUInt64CharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, *value));

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt64CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt64CharacteristicWriteRequest* request,
        uint64_t value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt64);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);

    HAPError err;

    // Validate constraints.
    if (!HAPUInt64CharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, value)) {
        return kHAPError_InvalidData;
    }

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPUInt64CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt64CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt64);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPUInt64CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt64CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_UInt64);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool HAPIntCharacteristicIsValueFulfillingConstraints(
        const HAPIntCharacteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        int32_t value) {
    if (!IS_VALUE_IN_RANGE(value, characteristic->constraints)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Value out of range: %ld (constraints: minimumValue = %ld / maximumValue = %ld / stepValue = %ld).",
                (long) value,
                (long) characteristic->constraints.minimumValue,
                (long) characteristic->constraints.maximumValue,
                (long) characteristic->constraints.stepValue);
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPIntCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicReadRequest* request,
        int32_t* value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Int);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    // Validate constraints.
    HAPAssert(HAPIntCharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, *value));

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPIntCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicWriteRequest* request,
        int32_t value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Int);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);

    HAPError err;

    // Validate constraints.
    if (!HAPIntCharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, value)) {
        return kHAPError_InvalidData;
    }

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPIntCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Int);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPIntCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Int);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool HAPFloatCharacteristicIsValueFulfillingConstraints(
        const HAPFloatCharacteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        float value) {
    if (HAPFloatIsInfinite(value)) {
        if (value > 0) {
            if (!HAPFloatIsInfinite(characteristic->constraints.maximumValue) ||
                characteristic->constraints.maximumValue < 0) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Value out of range: %g (constraints: minimumValue = %g / maximumValue = %g / stepValue = %g).",
                        (double) value,
                        (double) characteristic->constraints.minimumValue,
                        (double) characteristic->constraints.maximumValue,
                        (double) characteristic->constraints.stepValue);
                return false;
            }
        } else {
            if (!HAPFloatIsInfinite(characteristic->constraints.minimumValue) ||
                characteristic->constraints.minimumValue > 0) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Value out of range: %g (constraints: minimumValue = %g / maximumValue = %g / stepValue = %g).",
                        (double) value,
                        (double) characteristic->constraints.minimumValue,
                        (double) characteristic->constraints.maximumValue,
                        (double) characteristic->constraints.stepValue);
                return false;
            }
        }
    } else {
        if (!HAPFloatIsFinite(value)) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Value out of range: %g (constraints: minimumValue = %g / maximumValue = %g / stepValue = %g).",
                    (double) value,
                    (double) characteristic->constraints.minimumValue,
                    (double) characteristic->constraints.maximumValue,
                    (double) characteristic->constraints.stepValue);
            return false;
        }
        if (!IS_VALUE_IN_RANGE_WITH_TOLERANCE(value, characteristic->constraints, /* tolerance: */ 0.1f)) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Value out of range: %g (constraints: minimumValue = %g / maximumValue = %g / stepValue = %g).",
                    (double) value,
                    (double) characteristic->constraints.minimumValue,
                    (double) characteristic->constraints.maximumValue,
                    (double) characteristic->constraints.stepValue);
            return false;
        }
    }

    return true;
}

HAP_RESULT_USE_CHECK
static float HAPFloatCharacteristicRoundValueToStep(const HAPFloatCharacteristic* characteristic, float value) {
    if (!HAPFloatIsZero(characteristic->constraints.stepValue)) {
        value = ROUND_VALUE_TO_STEP(value, characteristic->constraints);
    }

    return value;
}

HAP_RESULT_USE_CHECK
HAPError HAPFloatCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPFloatCharacteristicReadRequest* request,
        float* value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Float);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    // Validate constraints.
    HAPAssert(HAPFloatCharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, *value));

    // Round to step.
    *value = HAPFloatCharacteristicRoundValueToStep(request->characteristic, *value);

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPFloatCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPFloatCharacteristicWriteRequest* request,
        float value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Float);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);

    HAPError err;

    // Validate constraints.
    if (!HAPFloatCharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, value)) {
        return kHAPError_InvalidData;
    }

    // Round to step.
    value = HAPFloatCharacteristicRoundValueToStep(request->characteristic, value);

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPFloatCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPFloatCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Float);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPFloatCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPFloatCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_Float);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static bool HAPStringCharacteristicIsValueFulfillingConstraints(
        const HAPStringCharacteristic* characteristic,
        const HAPService* service HAP_UNUSED,
        const HAPAccessory* accessory,
        const char* value) {
    size_t numValueBytes = HAPStringGetNumBytes(value);
    if (!IS_LENGTH_IN_RANGE(HAPStringGetNumBytes(value), characteristic->constraints)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Value too long: %zu bytes (constraints: maxLength = %u bytes).",
                numValueBytes,
                characteristic->constraints.maxLength);
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
HAPError HAPStringCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_String);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // We require at least 1 byte for the NULL-terminator.
    if (!maxValueBytes) {
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Not enough space to store value: Need 1 byte for NULL-terminator.");
        return kHAPError_OutOfResources;
    }

    // Set NULL-terminator.
    value[maxValueBytes - 1] = '\0';

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, value, maxValueBytes, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    // Validate that NULL-terminator is still present.
    if (value[maxValueBytes - 1] != '\0') {
        HAPLogCharacteristicError(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read string value exceeds available buffer space (available %zu bytes, including NULL-terminator).",
                maxValueBytes);
        HAPFatalError();
    }

    // Validate UTF-8 encoding.
    if (!HAPUTF8IsValidData(value, HAPStringGetNumBytes(value))) {
        HAPLogCharacteristicError(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read string value is not valid UTF-8.");
        HAPFatalError();
    }

    // Validate constraints.
    HAPAssert(HAPStringCharacteristicIsValueFulfillingConstraints(
            request->characteristic, request->service, request->accessory, value));

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPStringCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicWriteRequest* request,
        const char* value,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_String);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);
    HAPPrecondition(value);

    HAPError err;

    // Validate constraints.
    if (!HAPStringCharacteristicIsValueFulfillingConstraints(
                request->characteristic, request->service, request->accessory, value)) {
        return kHAPError_InvalidData;
    }

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, value, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPStringCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_String);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPStringCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_String);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
HAPError HAPTLV8CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_TLV8);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleRead);
    HAPPrecondition(request->accessory);
    HAPPrecondition(responseWriter);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling read handler.");
    err = request->characteristic->callbacks.handleRead(server, request, responseWriter, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Read handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPTLV8CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_TLV8);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->characteristic->callbacks.handleWrite);
    HAPPrecondition(request->accessory);
    HAPPrecondition(requestReader);

    HAPError err;

    // Call handler.
    HAPLogCharacteristicInfo(
            &logObject, request->characteristic, request->service, request->accessory, "Calling write handler.");
    err = request->characteristic->callbacks.handleWrite(server, request, requestReader, context);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Write handler failed with error %u.",
                err);
        return err;
    }

    return kHAPError_None;
}

void HAPTLV8CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_TLV8);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleSubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling subscribe handler.");
        request->characteristic->callbacks.handleSubscribe(server, request, context);
    }
}

void HAPTLV8CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicSubscriptionRequest* request,
        void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(request);
    HAPPrecondition(request->characteristic);
    HAPPrecondition(request->characteristic->format == kHAPCharacteristicFormat_TLV8);
    HAPPrecondition(request->characteristic->debugDescription);
    HAPPrecondition(request->accessory);

    if (request->characteristic->callbacks.handleUnsubscribe) {
        // Call handler.
        HAPLogCharacteristicInfo(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Calling unsubscribe handler.");
        request->characteristic->callbacks.handleUnsubscribe(server, request, context);
    }
}
