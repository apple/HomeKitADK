// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "IPAccessoryProtocol" };

typedef struct {
    struct {
        bool isDefined;
        uint64_t value;
    } aid;
    struct {
        bool isDefined;
        uint64_t value;
    } iid;
    HAPIPWriteValueType type;
    union {
        int32_t intValue;
        uint64_t unsignedIntValue;
        float floatValue;
        struct {
            char* _Nullable bytes;
            size_t numBytes;
        } stringValue;
    } value;
    struct {
        char* _Nullable bytes;
        size_t numBytes;
    } authorizationData;
    HAPIPEventNotificationState ev;
    bool remote;
    bool response;
} HAPIPWriteRequestParameters;

HAP_RESULT_USE_CHECK
static unsigned long long int uintval(uint64_t ui) {
    return (unsigned long long int) ui;
}

HAP_RESULT_USE_CHECK
static size_t try_read_uint(const char* buffer, size_t length, unsigned int* r) {
    size_t k;
    HAPAssert(buffer != NULL);
    HAPAssert(r != NULL);
    *r = 0;
    k = 0;
    HAPAssert(k <= length);
    while ((k < length) && ('0' <= buffer[k]) && (buffer[k] <= '9') &&
           (*r <= (UINT_MAX - (unsigned int) (buffer[k] - '0')) / 10)) {
        *r = *r * 10 + (unsigned int) (buffer[k] - '0');
        k++;
    }
    HAPAssert(
            (k == length) || ((k < length) && ((buffer[k] < '0') || (buffer[k] > '9') ||
                                               (*r > (UINT_MAX - (unsigned int) (buffer[k] - '0')) / 10))));
    return k;
}

HAP_RESULT_USE_CHECK
static size_t try_read_uint64(const char* buffer, size_t length, uint64_t* r) {
    size_t k;
    HAPAssert(buffer != NULL);
    HAPAssert(r != NULL);
    *r = 0;
    k = 0;
    HAPAssert(k <= length);
    while ((k < length) && ('0' <= buffer[k]) && (buffer[k] <= '9') &&
           (*r <= (UINT64_MAX - (uint64_t)(buffer[k] - '0')) / 10)) {
        *r = *r * 10 + (uint64_t)(buffer[k] - '0');
        k++;
    }
    HAPAssert(
            (k == length) || ((k < length) && ((buffer[k] < '0') || (buffer[k] > '9') ||
                                               (*r > (UINT64_MAX - (uint64_t)(buffer[k] - '0')) / 10))));
    return k;
}

/**
 * Finds the corresponding accessory object for the provided accessory instance ID and characteristic instance ID.
 *
 * @param      server_              Accessory server.
 * @param      aid                  Accessory instance ID.
 *
 * @return The accessory object for the provided accessory instance ID or NULL, if
 *         no corresponding accessory object was found.
 */
HAP_RESULT_USE_CHECK
static const HAPAccessory* _Nullable GetAccessory(HAPAccessoryServerRef* server_, uint64_t aid) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->primaryAccessory);

    const HAPAccessory* accessory = NULL;

    if (server->primaryAccessory->aid == aid) {
        accessory = server->primaryAccessory;
    } else if (server->ip.bridgedAccessories) {
        for (size_t i = 0; server->ip.bridgedAccessories[i]; i++) {
            if (server->ip.bridgedAccessories[i]->aid == aid) {
                accessory = server->ip.bridgedAccessories[i];
                break;
            }
        }
    }

    return accessory;
}

/**
 * Finds the corresponding characteristic object for the provided accessory instance ID and characteristic instance ID.
 *
 * @param      server               Accessory server.
 * @param      aid                  Accessory instance ID.
 * @param      iid                  Characteristic instance ID.
 *
 * @return The characteristic object for the provided accessory instance ID and characteristic instance ID or NULL, if
 *         no corresponding characteristic object was found.
 */
HAP_RESULT_USE_CHECK
static const HAPCharacteristic* _Nullable GetCharacteristic(HAPAccessoryServerRef* server, uint64_t aid, uint64_t iid) {
    HAPPrecondition(server);

    const HAPAccessory* accessory = GetAccessory(server, aid);

    if (accessory) {
        for (size_t serviceIndex = 0; accessory->services[serviceIndex]; serviceIndex++) {
            const HAPService* service = accessory->services[serviceIndex];
            if (!HAPAccessoryServerSupportsService(server, kHAPTransportType_IP, service)) {
                continue;
            }
            for (size_t characteristicIndex = 0; service->characteristics[characteristicIndex]; characteristicIndex++) {
                const HAPBaseCharacteristic* characteristic_ = service->characteristics[characteristicIndex];
                if (!HAPIPCharacteristicIsSupported(characteristic_)) {
                    continue;
                }
                if (characteristic_->iid != iid) {
                    continue;
                }
                return characteristic_;
            }
        }
    }

    return NULL;
}

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicReadRequests(
        char* bytes,
        size_t numBytes,
        HAPIPReadContextRef* readContexts,
        size_t maxReadContexts,
        size_t* numReadContexts,
        HAPIPReadRequestParameters* parameters) {
    HAPError err;
    bool done;
    unsigned int x;
    uint64_t aid, iid;
    size_t i, k, n;
    HAPAssert(bytes != NULL);
    HAPAssert(readContexts != NULL);
    HAPAssert(numReadContexts != NULL);
    HAPAssert(parameters != NULL);
    *numReadContexts = 0;
    parameters->meta = false;
    parameters->perms = false;
    parameters->type = false;
    parameters->ev = false;
    err = kHAPError_None;
    i = 0;
    HAPAssert(i <= numBytes);
    while (!err && (i < numBytes)) {
        if ((numBytes - i >= 3) && (bytes[i] == 'i') && (bytes[i + 1] == 'd') && (bytes[i + 2] == '=')) {
            i += 3;
            HAPAssert(i <= numBytes);
            if (i < numBytes) {
                done = false;
                do {
                    HAPAssert(i <= numBytes);
                    k = i;
                    i += try_read_uint64(&bytes[i], numBytes - i, &aid);
                    HAPAssert(k <= i);
                    HAPAssert(i <= numBytes);
                    if ((k < i) && (i < numBytes) && (bytes[i] == '.')) {
                        i++;
                        k = i;
                        i += try_read_uint64(&bytes[i], numBytes - i, &iid);
                        HAPAssert(k <= i);
                        HAPAssert(i <= numBytes);
                        if ((k < i) && ((i == numBytes) || ((bytes[i] < '0') || (bytes[i] > '9')))) {
                            if (*numReadContexts < maxReadContexts) {
                                HAPIPReadContext* readContext = (HAPIPReadContext*) &readContexts[*numReadContexts];
                                HAPRawBufferZero(readContext, sizeof *readContext);
                                readContext->aid = aid;
                                readContext->iid = iid;
                                (*numReadContexts)++;
                            } else {
                                HAPAssert(*numReadContexts == maxReadContexts);
                                err = kHAPError_OutOfResources;
                            }
                            HAPAssert(i <= numBytes);
                            if ((i == numBytes) || (bytes[i] != ',')) {
                                done = true;
                            } else {
                                i++;
                            }
                        } else {
                            err = kHAPError_InvalidData;
                        }
                    } else {
                        err = kHAPError_InvalidData;
                    }
                } while (!err && !done);
            }
        } else if (
                (numBytes - i >= 5) && (bytes[i] == 'm') && (bytes[i + 1] == 'e') && (bytes[i + 2] == 't') &&
                (bytes[i + 3] == 'a') && (bytes[i + 4] == '=')) {
            i += 5;
            n = try_read_uint(&bytes[i], numBytes - i, &x);
            if ((n == 1) && ((x == 0) || (x == 1))) {
                parameters->meta = x != 0;
                i++;
            } else {
                err = kHAPError_InvalidData;
            }
        } else if (
                (numBytes - i >= 6) && (bytes[i] == 'p') && (bytes[i + 1] == 'e') && (bytes[i + 2] == 'r') &&
                (bytes[i + 3] == 'm') && (bytes[i + 4] == 's') && (bytes[i + 5] == '=')) {
            i += 6;
            n = try_read_uint(&bytes[i], numBytes - i, &x);
            if ((n == 1) && ((x == 0) || (x == 1))) {
                parameters->perms = x != 0;
                i++;
            } else {
                err = kHAPError_InvalidData;
            }
        } else if (
                (numBytes - i >= 5) && (bytes[i] == 't') && (bytes[i + 1] == 'y') && (bytes[i + 2] == 'p') &&
                (bytes[i + 3] == 'e') && (bytes[i + 4] == '=')) {
            i += 5;
            n = try_read_uint(&bytes[i], numBytes - i, &x);
            if ((n == 1) && ((x == 0) || (x == 1))) {
                parameters->type = x != 0;
                i++;
            } else {
                err = kHAPError_InvalidData;
            }
        } else if ((numBytes - i >= 3) && (bytes[i] == 'e') && (bytes[i + 1] == 'v') && (bytes[i + 2] == '=')) {
            i += 3;
            n = try_read_uint(&bytes[i], numBytes - i, &x);
            if ((n == 1) && ((x == 0) || (x == 1))) {
                parameters->ev = x != 0;
                i++;
            } else {
                err = kHAPError_InvalidData;
            }
        } else {
            err = kHAPError_InvalidData;
        }
        HAPAssert(i <= numBytes);
        if (!err && (i < numBytes)) {
            switch (bytes[i]) {
                case '&':
                    i++;
                    break;
                case '#':
                    numBytes = i;
                    break;
                default:
                    err = kHAPError_InvalidData;
                    break;
            }
        }
    }
    HAPAssert(err || (i == numBytes));
    return err;
}

HAP_RESULT_USE_CHECK
size_t HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
        HAPAccessoryServerRef* server,
        HAPIPReadContextRef* readContexts,
        size_t numReadContexts,
        HAPIPReadRequestParameters* parameters) {
    HAPPrecondition(server);
    HAPPrecondition(readContexts);
    HAPPrecondition(parameters);

    size_t r, i, n;
    int success;

    r = 22;
    i = 0;
    HAPAssert(i <= numReadContexts);
    while ((i < numReadContexts) && (((HAPIPReadContext*) &readContexts[i])->status == 0)) {
        i++;
    }
    HAPIPReadContext* readContext = (HAPIPReadContext*) &readContexts[i];
    HAPAssert((i == numReadContexts) || ((i < numReadContexts) && (readContext->status != 0)));
    success = i == numReadContexts;
    for (i = 0; i < numReadContexts; i++) {
        readContext = (HAPIPReadContext*) &readContexts[i];

        const HAPBaseCharacteristic* chr_ = GetCharacteristic(server, readContext->aid, readContext->iid);
        HAPAssert(chr_ || (readContext->status != 0));
        r += (i == 0 ? 15 : 16) + HAPUInt64GetNumDescriptionBytes(readContext->aid) +
             HAPUInt64GetNumDescriptionBytes(readContext->iid);
        if (parameters->type && chr_) {
            r += 10 + HAPUUIDGetNumDescriptionBytes(chr_->characteristicType);
        }
        if (parameters->meta && chr_) {
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                    r += 16;
                } break;
                case kHAPCharacteristicFormat_UInt8: {
                    r += 17;
                } break;
                case kHAPCharacteristicFormat_UInt16: {
                    r += 18;
                } break;
                case kHAPCharacteristicFormat_UInt32: {
                    r += 18;
                } break;
                case kHAPCharacteristicFormat_UInt64: {
                    r += 18;
                } break;
                case kHAPCharacteristicFormat_Int: {
                    r += 15;
                } break;
                case kHAPCharacteristicFormat_Float: {
                    r += 17;
                } break;
                case kHAPCharacteristicFormat_String: {
                    r += 18;
                } break;
                case kHAPCharacteristicFormat_TLV8: {
                    r += 16;
                } break;
                case kHAPCharacteristicFormat_Data: {
                    r += 16;
                } break;
            }
        }
        if (readContext->status == 0) {
            HAPAssert(chr_);
            r += success ? 9 : 20;
            if (HAPUUIDAreEqual(chr_->characteristicType, &kHAPCharacteristicType_ProgrammableSwitchEvent)) {
                r += 4;
            } else {
                switch (chr_->format) {
                    case kHAPCharacteristicFormat_Bool: {
                        r += 1;
                    } break;
                    case kHAPCharacteristicFormat_UInt8:
                    case kHAPCharacteristicFormat_UInt16:
                    case kHAPCharacteristicFormat_UInt32:
                    case kHAPCharacteristicFormat_UInt64: {
                        r += HAPUInt64GetNumDescriptionBytes(readContext->value.unsignedIntValue);
                    } break;
                    case kHAPCharacteristicFormat_Int: {
                        r += HAPInt32GetNumDescriptionBytes(readContext->value.intValue);
                    } break;
                    case kHAPCharacteristicFormat_Float: {
                        r += HAPJSONUtilsGetFloatNumDescriptionBytes(readContext->value.floatValue);
                    } break;
                    case kHAPCharacteristicFormat_String:
                    case kHAPCharacteristicFormat_TLV8:
                    case kHAPCharacteristicFormat_Data: {
                        r += 2 + HAPJSONUtilsGetNumEscapedStringDataBytes(
                                         HAPNonnull(readContext->value.stringValue.bytes),
                                         readContext->value.stringValue.numBytes);
                    } break;
                }
            }
        } else {
            r += 10 + HAPInt32GetNumDescriptionBytes(readContext->status);
        }
        if (parameters->perms && chr_) {
            n = HAPCharacteristicGetNumEnabledProperties(chr_);
            r += 11 + (unsigned int) (n == 0 ? 0 : n * 4 + n - 1);
        }
        if (parameters->ev && chr_) {
            r += 6 + (readContext->ev ? 4 : 5);
        }
        if (parameters->meta && chr_) {
            HAPCharacteristicUnits unit = kHAPCharacteristicUnits_None;
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                } break;
                case kHAPCharacteristicFormat_UInt8: {
                    unit = ((const HAPUInt8Characteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_UInt16: {
                    unit = ((const HAPUInt16Characteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_UInt32: {
                    unit = ((const HAPUInt32Characteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_UInt64: {
                    unit = ((const HAPUInt64Characteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_Int: {
                    unit = ((const HAPIntCharacteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_Float: {
                    unit = ((const HAPFloatCharacteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_String:
                case kHAPCharacteristicFormat_TLV8:
                case kHAPCharacteristicFormat_Data: {
                } break;
            }
            switch (unit) {
                case kHAPCharacteristicUnits_None: {
                    r += 0;
                } break;
                case kHAPCharacteristicUnits_Celsius: {
                    r += 17;
                } break;
                case kHAPCharacteristicUnits_ArcDegrees: {
                    r += 20;
                } break;
                case kHAPCharacteristicUnits_Percentage: {
                    r += 20;
                } break;
                case kHAPCharacteristicUnits_Lux: {
                    r += 13;
                } break;
                case kHAPCharacteristicUnits_Seconds: {
                    r += 17;
                } break;
            }
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                } break;
                case kHAPCharacteristicFormat_UInt8: {
                    const HAPUInt8Characteristic* chr = (const HAPUInt8Characteristic*) chr_;
                    uint8_t minimumValue = chr->constraints.minimumValue;
                    uint8_t maximumValue = chr->constraints.maximumValue;
                    uint8_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);

                    if (minimumValue || maximumValue != UINT8_MAX) {
                        r += 35 + HAPUInt64GetNumDescriptionBytes(minimumValue) +
                             HAPUInt64GetNumDescriptionBytes(maximumValue) + HAPUInt64GetNumDescriptionBytes(stepValue);
                    }
                } break;
                case kHAPCharacteristicFormat_UInt16: {
                    const HAPUInt16Characteristic* chr = (const HAPUInt16Characteristic*) chr_;
                    uint16_t minimumValue = chr->constraints.minimumValue;
                    uint16_t maximumValue = chr->constraints.maximumValue;
                    uint16_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);

                    if (minimumValue || maximumValue != UINT16_MAX) {
                        r += 35 + HAPUInt64GetNumDescriptionBytes(minimumValue) +
                             HAPUInt64GetNumDescriptionBytes(maximumValue) + HAPUInt64GetNumDescriptionBytes(stepValue);
                    }
                } break;
                case kHAPCharacteristicFormat_UInt32: {
                    const HAPUInt32Characteristic* chr = (const HAPUInt32Characteristic*) chr_;
                    uint32_t minimumValue = chr->constraints.minimumValue;
                    uint32_t maximumValue = chr->constraints.maximumValue;
                    uint32_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);

                    if (minimumValue || maximumValue != UINT32_MAX) {
                        r += 35 + HAPUInt64GetNumDescriptionBytes(minimumValue) +
                             HAPUInt64GetNumDescriptionBytes(maximumValue) + HAPUInt64GetNumDescriptionBytes(stepValue);
                    }
                } break;
                case kHAPCharacteristicFormat_UInt64: {
                    const HAPUInt64Characteristic* chr = (const HAPUInt64Characteristic*) chr_;
                    uint64_t minimumValue = chr->constraints.minimumValue;
                    uint64_t maximumValue = chr->constraints.maximumValue;
                    uint64_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);

                    if (minimumValue || maximumValue != UINT64_MAX) {
                        r += 35 + HAPUInt64GetNumDescriptionBytes(minimumValue) +
                             HAPUInt64GetNumDescriptionBytes(maximumValue) + HAPUInt64GetNumDescriptionBytes(stepValue);
                    }
                } break;
                case kHAPCharacteristicFormat_Int: {
                    const HAPIntCharacteristic* chr = (const HAPIntCharacteristic*) chr_;
                    int32_t minimumValue = chr->constraints.minimumValue;
                    int32_t maximumValue = chr->constraints.maximumValue;
                    int32_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);
                    HAPAssert(stepValue >= 0);

                    if (minimumValue != INT32_MIN || maximumValue != INT32_MAX) {
                        r += 35 + HAPInt32GetNumDescriptionBytes(minimumValue) +
                             HAPInt32GetNumDescriptionBytes(maximumValue) + HAPInt32GetNumDescriptionBytes(stepValue);
                    }
                } break;
                case kHAPCharacteristicFormat_Float: {
                    const HAPFloatCharacteristic* chr = (const HAPFloatCharacteristic*) chr_;
                    float minimumValue = chr->constraints.minimumValue;
                    float maximumValue = chr->constraints.maximumValue;
                    float stepValue = chr->constraints.stepValue;
                    HAPAssert(HAPFloatIsFinite(minimumValue) || HAPFloatIsInfinite(minimumValue));
                    HAPAssert(HAPFloatIsFinite(maximumValue) || HAPFloatIsInfinite(maximumValue));
                    HAPAssert(minimumValue <= maximumValue);
                    HAPAssert(stepValue >= 0);

                    if (!(HAPFloatIsInfinite(minimumValue) && minimumValue < 0) ||
                        !(HAPFloatIsInfinite(maximumValue) && maximumValue > 0)) {
                        r += 35 + HAPJSONUtilsGetFloatNumDescriptionBytes(minimumValue) +
                             HAPJSONUtilsGetFloatNumDescriptionBytes(maximumValue) +
                             HAPJSONUtilsGetFloatNumDescriptionBytes(stepValue);
                    }
                } break;
                case kHAPCharacteristicFormat_String: {
                    const HAPStringCharacteristic* chr = (const HAPStringCharacteristic*) chr_;
                    uint16_t maxLength = chr->constraints.maxLength;

                    if (maxLength != 64) {
                        r += 10 + HAPUInt64GetNumDescriptionBytes(maxLength);
                    }
                } break;
                case kHAPCharacteristicFormat_TLV8: {
                } break;
                case kHAPCharacteristicFormat_Data: {
                    const HAPDataCharacteristic* chr = (const HAPDataCharacteristic*) chr_;
                    uint32_t maxLength = chr->constraints.maxLength;

                    if (maxLength != 2097152) {
                        r += 14 + HAPUInt64GetNumDescriptionBytes(maxLength);
                    }
                } break;
            }
        }
    }
    HAPAssert(i == numReadContexts);
    return r;
}

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
        HAPAccessoryServerRef* server,
        HAPIPReadContextRef* readContexts,
        size_t numReadContexts,
        HAPIPReadRequestParameters* parameters,
        HAPIPByteBuffer* buffer) {
    HAPPrecondition(server);
    HAPPrecondition(parameters);
    HAPPrecondition(buffer);

    HAPError err;

    int n, success;
    size_t i;
    char scratch_string[64];

    i = 0;
    HAPAssert(i <= numReadContexts);
    while ((i < numReadContexts) && (((HAPIPReadContext*) &readContexts[i])->status == 0)) {
        i++;
    }
    HAPIPReadContext* readContext = (HAPIPReadContext*) &readContexts[i];
    HAPAssert((i == numReadContexts) || ((i < numReadContexts) && (readContext->status != 0)));
    success = i == numReadContexts;
    err = HAPIPByteBufferAppendStringWithFormat(buffer, "{\"characteristics\":[");
    if (err) {
        goto error;
    }
    for (i = 0; i < numReadContexts; i++) {
        readContext = (HAPIPReadContext*) &readContexts[i];

        const HAPBaseCharacteristic* chr_ = GetCharacteristic(server, readContext->aid, readContext->iid);
        HAPAssert(chr_ || (readContext->status != 0));
        err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s{\"aid\":", i == 0 ? "" : ",");
        if (err) {
            goto error;
        }
        err = HAPUInt64GetDescription(uintval(readContext->aid), scratch_string, sizeof scratch_string);
        HAPAssert(!err);
        err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", scratch_string);
        if (err) {
            goto error;
        }
        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"iid\":");
        if (err) {
            goto error;
        }
        err = HAPUInt64GetDescription(uintval(readContext->iid), scratch_string, sizeof scratch_string);
        HAPAssert(!err);
        err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", scratch_string);
        if (err) {
            goto error;
        }

        if (parameters->type && chr_) {
            err = HAPUUIDGetDescription(chr_->characteristicType, scratch_string, sizeof scratch_string);
            HAPAssert(!err);
            err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"type\":\"%s\"", scratch_string);
            if (err) {
                goto error;
            }
        }
        if (parameters->meta && chr_) {
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"bool\"");
                } break;
                case kHAPCharacteristicFormat_UInt8: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"uint8\"");
                } break;
                case kHAPCharacteristicFormat_UInt16: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"uint16\"");
                } break;
                case kHAPCharacteristicFormat_UInt32: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"uint32\"");
                } break;
                case kHAPCharacteristicFormat_UInt64: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"uint64\"");
                } break;
                case kHAPCharacteristicFormat_Int: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"int\"");
                } break;
                case kHAPCharacteristicFormat_Float: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"float\"");
                } break;
                case kHAPCharacteristicFormat_String: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"string\"");
                } break;
                case kHAPCharacteristicFormat_TLV8: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"tlv8\"");
                } break;
                case kHAPCharacteristicFormat_Data: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"format\":\"data\"");
                } break;
            }
            if (err) {
                goto error;
            }
        }
        if (readContext->status == 0) {
            HAPAssert(chr_);
            if (!success) {
                err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"status\":0");
                if (err) {
                    goto error;
                }
            }
            if (HAPUUIDAreEqual(chr_->characteristicType, &kHAPCharacteristicType_ProgrammableSwitchEvent)) {
                // A read of this characteristic must always return a null value for IP accessories.
                // See HomeKit Accessory Protocol Specification R14
                // Section 9.75 Programmable Switch Event
                const HAPAccessory* accessory = HAPNonnull(GetAccessory(server, readContext->aid));
                HAPLogCharacteristicInfo(
                        &logObject,
                        chr_,
                        service,
                        accessory,
                        "Sending null value (readHandler callback is only called for HAP events).");
                err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":null}");
            } else {
                switch (chr_->format) {
                    case kHAPCharacteristicFormat_Bool: {
                        err = HAPIPByteBufferAppendStringWithFormat(
                                buffer, ",\"value\":%s}", readContext->value.unsignedIntValue ? "1" : "0");
                    } break;
                    case kHAPCharacteristicFormat_UInt8:
                    case kHAPCharacteristicFormat_UInt16:
                    case kHAPCharacteristicFormat_UInt32:
                    case kHAPCharacteristicFormat_UInt64: {
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":");
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(
                                uintval(readContext->value.unsignedIntValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, "}");
                    } break;
                    case kHAPCharacteristicFormat_Int: {
                        err = HAPIPByteBufferAppendStringWithFormat(
                                buffer, ",\"value\":%ld}", (long) readContext->value.intValue);
                    } break;
                    case kHAPCharacteristicFormat_Float: {
                        err = HAPJSONUtilsGetFloatDescription(
                                readContext->value.floatValue, scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":%s}", scratch_string);
                    } break;
                    case kHAPCharacteristicFormat_String:
                    case kHAPCharacteristicFormat_TLV8:
                    case kHAPCharacteristicFormat_Data: {
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":\"");
                        if (err) {
                            goto error;
                        }
                        size_t bufferMark = buffer->position;
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", readContext->value.stringValue.bytes);
                        if (err) {
                            goto error;
                        }
                        size_t numStringDataBytes = readContext->value.stringValue.numBytes;
                        err = HAPJSONUtilsEscapeStringData(
                                &buffer->data[bufferMark], buffer->limit - bufferMark, &numStringDataBytes);
                        if (err) {
                            goto error;
                        }
                        buffer->position = bufferMark + numStringDataBytes;
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, "\"}");
                    } break;
                }
            }
            if (err) {
                goto error;
            }
        } else {
            err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"status\":%ld}", (long) readContext->status);
            if (err) {
                goto error;
            }
        }
        if (parameters->perms && chr_) {
            // See HomeKit Accessory Protocol Specification R14
            // Section 6.3.3 Characteristic Objects
            err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"perms\":[");
            if (err) {
                goto error;
            }
            n = 0;
            if (chr_->properties.readable) {
                err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s\"pr\"", n == 0 ? "" : ",");
                if (err) {
                    goto error;
                }
                n++;
            }
            if (chr_->properties.writable) {
                err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s\"pw\"", n == 0 ? "" : ",");
                if (err) {
                    goto error;
                }
                n++;
            }
            if (chr_->properties.supportsEventNotification) {
                err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s\"ev\"", n == 0 ? "" : ",");
                if (err) {
                    goto error;
                }
                n++;
            }
            if (chr_->properties.supportsAuthorizationData) {
                err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s\"aa\"", n == 0 ? "" : ",");
                if (err) {
                    goto error;
                }
                n++;
            }
            if (chr_->properties.requiresTimedWrite) {
                err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s\"tw\"", n == 0 ? "" : ",");
                if (err) {
                    goto error;
                }
                n++;
            }
            if (chr_->properties.ip.supportsWriteResponse) {
                err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s\"wr\"", n == 0 ? "" : ",");
                if (err) {
                    goto error;
                }
                n++;
            }
            if (chr_->properties.hidden) {
                err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s\"hd\"", n == 0 ? "" : ",");
                if (err) {
                    goto error;
                }
                n++;
            }
            err = HAPIPByteBufferAppendStringWithFormat(buffer, "]");
            if (err) {
                goto error;
            }
        }
        if (parameters->ev && chr_) {
            err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"ev\":%s", readContext->ev ? "true" : "false");
            if (err) {
                goto error;
            }
        }
        if (parameters->meta && chr_) {
            HAPCharacteristicUnits unit = kHAPCharacteristicUnits_None;
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                } break;
                case kHAPCharacteristicFormat_UInt8: {
                    unit = ((const HAPUInt8Characteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_UInt16: {
                    unit = ((const HAPUInt16Characteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_UInt32: {
                    unit = ((const HAPUInt32Characteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_UInt64: {
                    unit = ((const HAPUInt64Characteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_Int: {
                    unit = ((const HAPIntCharacteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_Float: {
                    unit = ((const HAPFloatCharacteristic*) chr_)->units;
                } break;
                case kHAPCharacteristicFormat_String:
                case kHAPCharacteristicFormat_TLV8:
                case kHAPCharacteristicFormat_Data: {
                } break;
            }
            switch (unit) {
                case kHAPCharacteristicUnits_None: {
                } break;
                case kHAPCharacteristicUnits_Celsius: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"unit\":\"celsius\"");
                } break;
                case kHAPCharacteristicUnits_ArcDegrees: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"unit\":\"arcdegrees\"");
                } break;
                case kHAPCharacteristicUnits_Percentage: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"unit\":\"percentage\"");
                } break;
                case kHAPCharacteristicUnits_Lux: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"unit\":\"lux\"");
                } break;
                case kHAPCharacteristicUnits_Seconds: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"unit\":\"seconds\"");
                } break;
            }
            if (err) {
                goto error;
            }
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                } break;
                case kHAPCharacteristicFormat_UInt8: {
                    const HAPUInt8Characteristic* chr = (const HAPUInt8Characteristic*) chr_;
                    uint8_t minimumValue = chr->constraints.minimumValue;
                    uint8_t maximumValue = chr->constraints.maximumValue;
                    uint8_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);

                    if (minimumValue || maximumValue != UINT8_MAX) {
                        err = HAPUInt64GetDescription(uintval(minimumValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(uintval(maximumValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"maxValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(uintval(stepValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minStep\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                    }
                } break;
                case kHAPCharacteristicFormat_UInt16: {
                    const HAPUInt16Characteristic* chr = (const HAPUInt16Characteristic*) chr_;
                    uint16_t minimumValue = chr->constraints.minimumValue;
                    uint16_t maximumValue = chr->constraints.maximumValue;
                    uint16_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);

                    if (minimumValue || maximumValue != UINT16_MAX) {
                        err = HAPUInt64GetDescription(uintval(minimumValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(uintval(maximumValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"maxValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(uintval(stepValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minStep\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                    }
                } break;
                case kHAPCharacteristicFormat_UInt32: {
                    const HAPUInt32Characteristic* chr = (const HAPUInt32Characteristic*) chr_;
                    uint32_t minimumValue = chr->constraints.minimumValue;
                    uint32_t maximumValue = chr->constraints.maximumValue;
                    uint32_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);

                    if (minimumValue || maximumValue != UINT32_MAX) {
                        err = HAPUInt64GetDescription(uintval(minimumValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(uintval(maximumValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"maxValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(uintval(stepValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minStep\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                    }
                } break;
                case kHAPCharacteristicFormat_UInt64: {
                    const HAPUInt64Characteristic* chr = (const HAPUInt64Characteristic*) chr_;
                    uint64_t minimumValue = chr->constraints.minimumValue;
                    uint64_t maximumValue = chr->constraints.maximumValue;
                    uint64_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);

                    if (minimumValue || maximumValue != UINT64_MAX) {
                        err = HAPUInt64GetDescription(uintval(minimumValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(uintval(maximumValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"maxValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPUInt64GetDescription(uintval(stepValue), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minStep\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                    }
                } break;
                case kHAPCharacteristicFormat_Int: {
                    const HAPIntCharacteristic* chr = (const HAPIntCharacteristic*) chr_;
                    int32_t minimumValue = chr->constraints.minimumValue;
                    int32_t maximumValue = chr->constraints.maximumValue;
                    int32_t stepValue = chr->constraints.stepValue;
                    HAPAssert(minimumValue <= maximumValue);
                    HAPAssert(stepValue >= 0);

                    if (minimumValue != INT32_MIN || maximumValue != INT32_MAX) {
                        err = HAPIPByteBufferAppendStringWithFormat(
                                buffer,
                                ",\"minValue\":%ld"
                                ",\"maxValue\":%ld"
                                ",\"minStep\":%ld",
                                (long) minimumValue,
                                (long) maximumValue,
                                (long) stepValue);
                        if (err) {
                            goto error;
                        }
                    }
                } break;
                case kHAPCharacteristicFormat_Float: {
                    const HAPFloatCharacteristic* chr = (const HAPFloatCharacteristic*) chr_;
                    float minimumValue = chr->constraints.minimumValue;
                    float maximumValue = chr->constraints.maximumValue;
                    float stepValue = chr->constraints.stepValue;
                    HAPAssert(HAPFloatIsFinite(minimumValue) || HAPFloatIsInfinite(minimumValue));
                    HAPAssert(HAPFloatIsFinite(maximumValue) || HAPFloatIsInfinite(maximumValue));
                    HAPAssert(minimumValue <= maximumValue);
                    HAPAssert(stepValue >= 0);

                    if (!(HAPFloatIsInfinite(minimumValue) && minimumValue < 0) ||
                        !(HAPFloatIsInfinite(maximumValue) && maximumValue > 0)) {
                        err = HAPJSONUtilsGetFloatDescription(minimumValue, scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPJSONUtilsGetFloatDescription(maximumValue, scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"maxValue\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                        err = HAPJSONUtilsGetFloatDescription(stepValue, scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"minStep\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                    }
                } break;
                case kHAPCharacteristicFormat_String: {
                    const HAPStringCharacteristic* chr = (const HAPStringCharacteristic*) chr_;
                    uint16_t maxLength = chr->constraints.maxLength;

                    if (maxLength != 64) {
                        err = HAPUInt64GetDescription(uintval(maxLength), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"maxLen\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                    }
                } break;
                case kHAPCharacteristicFormat_TLV8: {
                } break;
                case kHAPCharacteristicFormat_Data: {
                    const HAPDataCharacteristic* chr = (const HAPDataCharacteristic*) chr_;
                    uint32_t maxLength = chr->constraints.maxLength;

                    if (maxLength != 2097152) {
                        err = HAPUInt64GetDescription(uintval(maxLength), scratch_string, sizeof scratch_string);
                        HAPAssert(!err);
                        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"maxDataLen\":%s", scratch_string);
                        if (err) {
                            goto error;
                        }
                    }
                } break;
            }
        }
    }
    HAPAssert(i == numReadContexts);
    err = HAPIPByteBufferAppendStringWithFormat(buffer, "]}");
    if (err) {
        goto error;
    }
    return kHAPError_None;
error:
    return kHAPError_OutOfResources;
}

HAP_RESULT_USE_CHECK
static size_t read_characteristic_write_request_parameters(
        struct util_json_reader* r,
        char* buffer,
        size_t length,
        HAPIPWriteRequestParameters* parameters,
        HAPError* err) {
    size_t i, j, k, n;
    int frac;
    uint64_t aid, iid;
    unsigned int ev, remote;
    float fval;
    int64_t llval;
    uint64_t ullval;
    char number[64];
    unsigned int response;

    HAPAssert(r != NULL);
    HAPAssert(buffer != NULL);
    HAPAssert(parameters != NULL);
    HAPAssert(err != NULL);

    *err = kHAPError_None;
    k = util_json_reader_read(r, buffer, length);
    if (r->state != util_JSON_READER_STATE_BEGINNING_STRING) {
        *err = kHAPError_InvalidData;
        goto exit;
    }
    HAPAssert(k <= length);
    i = k;
    k += util_json_reader_read(r, &buffer[k], length - k);
    if (r->state != util_JSON_READER_STATE_COMPLETED_STRING) {
        *err = kHAPError_InvalidData;
        goto exit;
    }
    HAPAssert(k <= length);
    j = k;
    k += util_json_reader_read(r, &buffer[k], length - k);
    if (r->state != util_JSON_READER_STATE_AFTER_NAME_SEPARATOR) {
        *err = kHAPError_InvalidData;
        goto exit;
    }
    HAPAssert(i <= j);
    HAPAssert(j <= k);
    HAPAssert(k <= length);
    if (j - i == 5) {
        if (HAPRawBufferAreEqual(&buffer[i], "\"aid\"", 5)) {
            k += util_json_reader_read(r, &buffer[k], length - k);
            if (r->state != util_JSON_READER_STATE_BEGINNING_NUMBER) {
                *err = kHAPError_InvalidData;
                goto exit;
            }
            HAPAssert(k <= length);
            i = k;
            k += util_json_reader_read(r, &buffer[k], length - k);
            if (r->state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                *err = kHAPError_InvalidData;
                goto exit;
            }
            HAPAssert(i <= k);
            HAPAssert(k <= length);
            n = try_read_uint64(&buffer[i], k - i, &aid);
            if (n == k - i) {
                parameters->aid.isDefined = true;
                parameters->aid.value = aid;
            } else {
                *err = kHAPError_InvalidData;
                goto exit;
            }
        } else if (HAPRawBufferAreEqual(&buffer[i], "\"iid\"", 5)) {
            k += util_json_reader_read(r, &buffer[k], length - k);
            if (r->state != util_JSON_READER_STATE_BEGINNING_NUMBER) {
                *err = kHAPError_InvalidData;
                goto exit;
            }
            HAPAssert(k <= length);
            i = k;
            k += util_json_reader_read(r, &buffer[k], length - k);
            if (r->state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                *err = kHAPError_InvalidData;
                goto exit;
            }
            HAPAssert(i <= k);
            HAPAssert(k <= length);
            n = try_read_uint64(&buffer[i], k - i, &iid);
            if (n == k - i) {
                parameters->iid.isDefined = true;
                parameters->iid.value = iid;
            } else {
                *err = kHAPError_InvalidData;
                goto exit;
            }
        } else {
            size_t skippedBytes;
            *err = HAPJSONUtilsSkipValue(r, &buffer[k], length - k, &skippedBytes);
            if (*err) {
                HAPAssert((*err == kHAPError_InvalidData) || (*err == kHAPError_OutOfResources));
                goto exit;
            }
            k += skippedBytes;
        }
    } else if ((j - i == 7) && HAPRawBufferAreEqual(&buffer[i], "\"value\"", 7)) {
        k += util_json_reader_read(r, &buffer[k], length - k);
        switch (r->state) {
            case util_JSON_READER_STATE_BEGINNING_NUMBER: {
                HAPAssert(k <= length);
                i = k;
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                HAPAssert(i <= k);
                HAPAssert(k <= length);
                n = 0;
                HAPAssert(n <= sizeof number);
                frac = 0;
                while ((i < k) && (n < sizeof number)) {
                    if (!frac && (buffer[i] == '.')) {
                        frac = 1;
                    }
                    number[n] = buffer[i];
                    n++;
                    i++;
                }
                if (n < sizeof number) {
                    HAPAssert(i == k);
                    number[n] = '\0';
                    if (frac) {
                        *err = HAPFloatFromString(number, &fval);
                        if (*err) {
                            HAPAssert(*err == kHAPError_InvalidData);
                            goto exit;
                        }
                        parameters->value.floatValue = fval;
                        parameters->type = kHAPIPWriteValueType_Float;
                    } else {
                        *err = HAPInt64FromString(number, &llval);
                        if (!*err) {
                            if (llval < 0) {
                                if (llval >= INT32_MIN) {
                                    parameters->value.intValue = (int32_t) llval;
                                    parameters->type = kHAPIPWriteValueType_Int;
                                } else {
                                    *err = kHAPError_InvalidData;
                                    goto exit;
                                }
                            } else {
                                parameters->value.unsignedIntValue = (uint64_t) llval;
                                parameters->type = kHAPIPWriteValueType_UInt;
                            }
                        } else {
                            HAPAssert(*err == kHAPError_InvalidData);
                            *err = HAPUInt64FromString(number, &ullval);
                            if (*err) {
                                HAPAssert(*err == kHAPError_InvalidData);
                                goto exit;
                            }
                            parameters->value.unsignedIntValue = ullval;
                            parameters->type = kHAPIPWriteValueType_UInt;
                        }
                    }
                } else {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
            } break;
            case util_JSON_READER_STATE_BEGINNING_STRING: {
                HAPAssert(k <= length);
                i = k;
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_STRING) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                HAPAssert(i <= k);
                HAPAssert(k <= length);
                HAPAssert(k - i >= 2);
                parameters->value.stringValue.bytes = &buffer[i + 1];
                parameters->value.stringValue.numBytes = k - i - 2;
                if (!HAPUTF8IsValidData(
                            HAPNonnull(parameters->value.stringValue.bytes), parameters->value.stringValue.numBytes)) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                *err = HAPJSONUtilsUnescapeStringData(
                        HAPNonnull(parameters->value.stringValue.bytes), &parameters->value.stringValue.numBytes);
                if (*err) {
                    HAPAssert(*err == kHAPError_InvalidData);
                    goto exit;
                }
                parameters->type = kHAPIPWriteValueType_String;
            } break;
            case util_JSON_READER_STATE_BEGINNING_FALSE: {
                HAPAssert(k <= length);
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_FALSE) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                parameters->value.unsignedIntValue = 0;
                parameters->type = kHAPIPWriteValueType_UInt;
            } break;
            case util_JSON_READER_STATE_BEGINNING_TRUE: {
                HAPAssert(k <= length);
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_TRUE) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                parameters->value.unsignedIntValue = 1;
                parameters->type = kHAPIPWriteValueType_UInt;
            } break;
            default: {
                *err = kHAPError_InvalidData;
            }
                goto exit;
        }
    } else if ((j - i == 4) && HAPRawBufferAreEqual(&buffer[i], "\"ev\"", 4)) {
        k += util_json_reader_read(r, &buffer[k], length - k);
        switch (r->state) {
            case util_JSON_READER_STATE_BEGINNING_NUMBER:
                HAPAssert(k <= length);
                i = k;
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                HAPAssert(i <= k);
                HAPAssert(k <= length);
                n = try_read_uint(&buffer[i], k - i, &ev);
                if (n == k - i) {
                    if (ev == 0) {
                        parameters->ev = kHAPIPEventNotificationState_Disabled;
                    } else if (ev == 1) {
                        parameters->ev = kHAPIPEventNotificationState_Enabled;
                    } else {
                        *err = kHAPError_InvalidData;
                        goto exit;
                    }
                } else {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                break;
            case util_JSON_READER_STATE_BEGINNING_FALSE:
                HAPAssert(k <= length);
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_FALSE) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                parameters->ev = kHAPIPEventNotificationState_Disabled;
                break;
            case util_JSON_READER_STATE_BEGINNING_TRUE:
                HAPAssert(k <= length);
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_TRUE) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                parameters->ev = kHAPIPEventNotificationState_Enabled;
                break;
            default:
                *err = kHAPError_InvalidData;
                goto exit;
        }
    } else if ((j - i == 10) && HAPRawBufferAreEqual(&buffer[i], "\"authData\"", 10)) {
        k += util_json_reader_read(r, &buffer[k], length - k);
        if (r->state != util_JSON_READER_STATE_BEGINNING_STRING) {
            *err = kHAPError_InvalidData;
            goto exit;
        }
        HAPAssert(k <= length);
        i = k;
        k += util_json_reader_read(r, &buffer[k], length - k);
        if (r->state != util_JSON_READER_STATE_COMPLETED_STRING) {
            *err = kHAPError_InvalidData;
            goto exit;
        }
        HAPAssert(i <= k);
        HAPAssert(k <= length);
        HAPAssert(k - i >= 2);
        parameters->authorizationData.bytes = &buffer[i + 1];
        parameters->authorizationData.numBytes = k - i - 2;
        if (!HAPUTF8IsValidData(
                    HAPNonnull(parameters->authorizationData.bytes), parameters->authorizationData.numBytes)) {
            *err = kHAPError_InvalidData;
            goto exit;
        }
        *err = HAPJSONUtilsUnescapeStringData(
                HAPNonnull(parameters->authorizationData.bytes), &parameters->authorizationData.numBytes);
        if (*err) {
            HAPAssert(*err == kHAPError_InvalidData);
            goto exit;
        }
    } else if ((j - i == 8) && HAPRawBufferAreEqual(&buffer[i], "\"remote\"", 8)) {
        k += util_json_reader_read(r, &buffer[k], length - k);
        switch (r->state) {
            case util_JSON_READER_STATE_BEGINNING_NUMBER:
                HAPAssert(k <= length);
                i = k;
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                HAPAssert(i <= k);
                HAPAssert(k <= length);
                n = try_read_uint(&buffer[i], k - i, &remote);
                if (n == k - i) {
                    if (remote == 0) {
                        parameters->remote = false;
                    } else if (remote == 1) {
                        parameters->remote = true;
                    } else {
                        *err = kHAPError_InvalidData;
                        goto exit;
                    }
                } else {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                break;
            case util_JSON_READER_STATE_BEGINNING_FALSE:
                HAPAssert(k <= length);
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_FALSE) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                parameters->remote = false;
                break;
            case util_JSON_READER_STATE_BEGINNING_TRUE:
                HAPAssert(k <= length);
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_TRUE) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                parameters->remote = true;
                break;
            default:
                *err = kHAPError_InvalidData;
                goto exit;
        }
    } else if ((j - i == 3) && HAPRawBufferAreEqual(&buffer[i], "\"r\"", 3)) {
        k += util_json_reader_read(r, &buffer[k], length - k);
        switch (r->state) {
            case util_JSON_READER_STATE_BEGINNING_NUMBER:
                HAPAssert(k <= length);
                i = k;
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                HAPAssert(i <= k);
                HAPAssert(k <= length);
                n = try_read_uint(&buffer[i], k - i, &response);
                if (n == k - i) {
                    if (response == 0) {
                        parameters->response = false;
                    } else if (response == 1) {
                        parameters->response = true;
                    } else {
                        *err = kHAPError_InvalidData;
                        goto exit;
                    }
                } else {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                break;
            case util_JSON_READER_STATE_BEGINNING_FALSE:
                HAPAssert(k <= length);
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_FALSE) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                parameters->response = false;
                break;
            case util_JSON_READER_STATE_BEGINNING_TRUE:
                HAPAssert(k <= length);
                k += util_json_reader_read(r, &buffer[k], length - k);
                if (r->state != util_JSON_READER_STATE_COMPLETED_TRUE) {
                    *err = kHAPError_InvalidData;
                    goto exit;
                }
                parameters->response = true;
                break;
            default:
                *err = kHAPError_InvalidData;
                goto exit;
        }
    } else {
        size_t skippedBytes;
        *err = HAPJSONUtilsSkipValue(r, &buffer[k], length - k, &skippedBytes);
        if (*err) {
            HAPAssert((*err == kHAPError_InvalidData) || (*err == kHAPError_OutOfResources));
            goto exit;
        }
        k += skippedBytes;
    }
exit:
    HAPAssert((r->state != util_JSON_READER_STATE_ERROR) || *err);
    HAPAssert(k <= length);
    return k;
}

HAP_RESULT_USE_CHECK
static size_t read_characteristic_write_request(
        struct util_json_reader* r,
        char* buffer,
        size_t length,
        HAPIPWriteContextRef* contexts,
        size_t max_contexts,
        size_t* numReadContexts,
        HAPError* err) {
    size_t k;
    HAPIPWriteRequestParameters parameters;
    HAPAssert(r != NULL);
    HAPAssert(buffer != NULL);
    HAPAssert(contexts != NULL);
    HAPAssert(numReadContexts != NULL);
    HAPAssert(err != NULL);
    *err = kHAPError_None;
    HAPRawBufferZero(&parameters, sizeof parameters);
    k = util_json_reader_read(r, buffer, length);
    if (r->state != util_JSON_READER_STATE_BEGINNING_OBJECT) {
        *err = kHAPError_InvalidData;
        goto exit;
    }
    HAPAssert(k <= length);
    do {
        k += read_characteristic_write_request_parameters(r, &buffer[k], length - k, &parameters, err);
        if (*err) {
            goto exit;
        }
        HAPAssert(k <= length);
        k += util_json_reader_read(r, &buffer[k], length - k);
    } while ((k < length) && (r->state == util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR));
    HAPAssert((k == length) || ((k < length) && (r->state != util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR)));
    if (r->state != util_JSON_READER_STATE_COMPLETED_OBJECT) {
        *err = kHAPError_InvalidData;
        goto exit;
    }
    if ((parameters.aid.isDefined) && (parameters.iid.isDefined)) {
        if (*numReadContexts < max_contexts) {
            HAPIPWriteContext* writeContext = (HAPIPWriteContext*) &contexts[*numReadContexts];
            HAPRawBufferZero(writeContext, sizeof *writeContext);
            writeContext->aid = parameters.aid.value;
            writeContext->iid = parameters.iid.value;
            writeContext->type = parameters.type;
            switch (parameters.type) {
                case kHAPIPWriteValueType_None: {
                } break;
                case kHAPIPWriteValueType_Int: {
                    writeContext->value.intValue = parameters.value.intValue;
                } break;
                case kHAPIPWriteValueType_UInt: {
                    writeContext->value.unsignedIntValue = parameters.value.unsignedIntValue;
                } break;
                case kHAPIPWriteValueType_Float: {
                    writeContext->value.floatValue = parameters.value.floatValue;
                } break;
                case kHAPIPWriteValueType_String: {
                    writeContext->value.stringValue.bytes = parameters.value.stringValue.bytes;
                    writeContext->value.stringValue.numBytes = parameters.value.stringValue.numBytes;
                } break;
            }
            writeContext->ev = parameters.ev;
            writeContext->authorizationData.bytes = parameters.authorizationData.bytes;
            writeContext->authorizationData.numBytes = parameters.authorizationData.numBytes;
            writeContext->remote = parameters.remote;
            writeContext->response = parameters.response;
            (*numReadContexts)++;
        } else {
            HAPAssert(*numReadContexts == max_contexts);
            *err = kHAPError_OutOfResources;
        }
    } else {
        *err = kHAPError_InvalidData;
    }
exit:
    HAPAssert((r->state != util_JSON_READER_STATE_ERROR) || *err);
    HAPAssert(k <= length);
    return k;
}

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicWriteRequests(
        char* bytes,
        size_t numBytes,
        HAPIPWriteContextRef* writeContexts,
        size_t maxWriteContexts,
        size_t* numWriteContexts,
        bool* hasPID,
        uint64_t* pid) {
    // See HomeKit Accessory Protocol Specification R14
    // Section 6.7.2 Writing Characteristics
    struct util_json_reader json_reader;
    size_t i, j, k, n;
    uint64_t x;

    HAPAssert(bytes != NULL);
    HAPAssert(writeContexts != NULL);
    HAPAssert(numWriteContexts != NULL);
    HAPAssert(hasPID != NULL);
    HAPAssert(pid != NULL);

    HAPError err;

    util_json_reader_init(&json_reader);
    *numWriteContexts = 0;
    *hasPID = false;
    *pid = 0;
    k = util_json_reader_read(&json_reader, bytes, numBytes);
    if (json_reader.state != util_JSON_READER_STATE_BEGINNING_OBJECT) {
        return kHAPError_InvalidData;
    }
    HAPAssert(k <= numBytes);
    do {
        k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
        if (json_reader.state != util_JSON_READER_STATE_BEGINNING_STRING) {
            return kHAPError_InvalidData;
        }
        HAPAssert(k <= numBytes);
        i = k;
        k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
        if (json_reader.state != util_JSON_READER_STATE_COMPLETED_STRING) {
            return kHAPError_InvalidData;
        }
        HAPAssert(k <= numBytes);
        j = k;
        k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
        if (json_reader.state != util_JSON_READER_STATE_AFTER_NAME_SEPARATOR) {
            return kHAPError_InvalidData;
        }
        HAPAssert(i <= j);
        HAPAssert(j <= k);
        HAPAssert(k <= numBytes);
        if ((j - i == 17) && HAPRawBufferAreEqual(&bytes[i], "\"characteristics\"", 17)) {
            k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
            if (json_reader.state != util_JSON_READER_STATE_BEGINNING_ARRAY) {
                return kHAPError_InvalidData;
            }
            HAPAssert(k <= numBytes);
            do {
                k += read_characteristic_write_request(
                        &json_reader, &bytes[k], numBytes - k, writeContexts, maxWriteContexts, numWriteContexts, &err);
                if (err) {
                    return err;
                }
                HAPAssert(k <= numBytes);
                k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
            } while ((k < numBytes) && (json_reader.state == util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR));
            HAPAssert(
                    (k == numBytes) ||
                    ((k < numBytes) && (json_reader.state != util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR)));
            if (json_reader.state != util_JSON_READER_STATE_COMPLETED_ARRAY) {
                return kHAPError_InvalidData;
            }
        } else if ((j - i == 5) && HAPRawBufferAreEqual(&bytes[i], "\"pid\"", 5)) {
            if (*hasPID) {
                HAPLog(&logObject, "Multiple PID entries detected.");
                return kHAPError_InvalidData;
            }
            k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
            if (json_reader.state == util_JSON_READER_STATE_BEGINNING_NUMBER) {
                HAPAssert(k <= numBytes);
                i = k;
                k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
                if (json_reader.state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                    return kHAPError_InvalidData;
                }
                HAPAssert(i <= k);
                HAPAssert(k <= numBytes);
                n = try_read_uint64(&bytes[i], k - i, &x);
                if (n == k - i) {
                    *pid = x;
                    *hasPID = true;
                } else {
                    HAPLogBuffer(&logObject, &bytes[i], k - i, "Invalid PID requested.");
                    return kHAPError_InvalidData;
                }
            } else {
                return kHAPError_InvalidData;
            }
        } else {
            size_t skippedBytes;
            err = HAPJSONUtilsSkipValue(&json_reader, &bytes[k], numBytes - k, &skippedBytes);
            if (err) {
                HAPAssert((err == kHAPError_InvalidData) || (err == kHAPError_OutOfResources));
                return kHAPError_InvalidData;
            }
            k += skippedBytes;
        }
        HAPAssert(k <= numBytes);
        k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
    } while ((k < numBytes) && (json_reader.state == util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR));
    HAPAssert(
            (k == numBytes) || ((k < numBytes) && (json_reader.state != util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR)));
    if (json_reader.state != util_JSON_READER_STATE_COMPLETED_OBJECT) {
        return kHAPError_InvalidData;
    }
    k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
    if (k < numBytes) {
        return kHAPError_InvalidData;
    } else {
        HAPAssert(k == numBytes);
        HAPAssert(
                (json_reader.state == util_JSON_READER_STATE_COMPLETED_OBJECT) ||
                (json_reader.state == util_JSON_READER_STATE_READING_WHITESPACE));
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
size_t HAPIPAccessoryProtocolGetNumCharacteristicWriteResponseBytes(
        HAPAccessoryServerRef* server,
        HAPIPWriteContextRef* writeContexts,
        size_t numWriteContexts) {
    HAPPrecondition(server);
    HAPPrecondition(writeContexts);

    size_t r, i;

    r = 22;
    for (i = 0; i < numWriteContexts; i++) {
        HAPIPWriteContext* writeContext = (HAPIPWriteContext*) &writeContexts[i];
        r += (i == 0 ? 25 : 26) + HAPUInt64GetNumDescriptionBytes(writeContext->aid) +
             HAPUInt64GetNumDescriptionBytes(writeContext->iid) + HAPInt32GetNumDescriptionBytes(writeContext->status);
        if ((writeContext->status == 0) && writeContext->response) {
            r += 9;
            const HAPBaseCharacteristic* chr_ = GetCharacteristic(server, writeContext->aid, writeContext->iid);
            HAPAssert(chr_);
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                    r += 1;
                } break;
                case kHAPCharacteristicFormat_UInt8:
                case kHAPCharacteristicFormat_UInt16:
                case kHAPCharacteristicFormat_UInt32:
                case kHAPCharacteristicFormat_UInt64: {
                    r += HAPUInt64GetNumDescriptionBytes(writeContext->value.unsignedIntValue);
                } break;
                case kHAPCharacteristicFormat_Int: {
                    r += HAPInt32GetNumDescriptionBytes(writeContext->value.intValue);
                } break;
                case kHAPCharacteristicFormat_Float: {
                    r += HAPJSONUtilsGetFloatNumDescriptionBytes(writeContext->value.floatValue);
                } break;
                case kHAPCharacteristicFormat_String:
                case kHAPCharacteristicFormat_TLV8:
                case kHAPCharacteristicFormat_Data: {
                    r += 2 + HAPJSONUtilsGetNumEscapedStringDataBytes(
                                     HAPNonnull(writeContext->value.stringValue.bytes),
                                     writeContext->value.stringValue.numBytes);
                } break;
            }
        }
    }
    HAPAssert(i == numWriteContexts);
    return r;
}

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicWriteResponseBytes(
        HAPAccessoryServerRef* server,
        HAPIPWriteContextRef* writeContexts,
        size_t numWriteContexts,
        HAPIPByteBuffer* buffer) {
    HAPPrecondition(server);
    HAPPrecondition(writeContexts);
    HAPPrecondition(buffer);

    HAPError err;

    size_t i;
    char scratch_string[64];

    err = HAPIPByteBufferAppendStringWithFormat(buffer, "{\"characteristics\":[");
    if (err) {
        goto error;
    }
    for (i = 0; i < numWriteContexts; i++) {
        HAPIPWriteContext* writeContext = (HAPIPWriteContext*) &writeContexts[i];
        char aidDescription[64];
        err = HAPUInt64GetDescription(uintval(writeContext->aid), aidDescription, sizeof aidDescription);
        HAPAssert(!err);
        char iidDescription[64];
        err = HAPUInt64GetDescription(uintval(writeContext->iid), iidDescription, sizeof iidDescription);
        HAPAssert(!err);
        err = HAPIPByteBufferAppendStringWithFormat(
                buffer,
                "%s{\"aid\":%s,\"iid\":%s,\"status\":%ld",
                i == 0 ? "" : ",",
                aidDescription,
                iidDescription,
                (long) writeContext->status);
        if (err) {
            goto error;
        }
        if ((writeContext->status == 0) && writeContext->response) {
            const HAPBaseCharacteristic* chr_ = GetCharacteristic(server, writeContext->aid, writeContext->iid);
            HAPAssert(chr_);
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                    err = HAPIPByteBufferAppendStringWithFormat(
                            buffer, ",\"value\":%s", writeContext->value.unsignedIntValue ? "1" : "0");
                } break;
                case kHAPCharacteristicFormat_UInt8:
                case kHAPCharacteristicFormat_UInt16:
                case kHAPCharacteristicFormat_UInt32:
                case kHAPCharacteristicFormat_UInt64: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":");
                    if (err) {
                        goto error;
                    }
                    err = HAPUInt64GetDescription(
                            uintval(writeContext->value.unsignedIntValue), scratch_string, sizeof scratch_string);
                    HAPAssert(!err);
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", scratch_string);
                } break;
                case kHAPCharacteristicFormat_Int: {
                    err = HAPIPByteBufferAppendStringWithFormat(
                            buffer, ",\"value\":%ld", (long) writeContext->value.intValue);
                } break;
                case kHAPCharacteristicFormat_Float: {
                    err = HAPJSONUtilsGetFloatDescription(
                            writeContext->value.floatValue, scratch_string, sizeof scratch_string);
                    HAPAssert(!err);
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":%s", scratch_string);
                } break;
                case kHAPCharacteristicFormat_String:
                case kHAPCharacteristicFormat_TLV8:
                case kHAPCharacteristicFormat_Data: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":\"");
                    if (err) {
                        goto error;
                    }
                    size_t bufferMark = buffer->position;
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", writeContext->value.stringValue.bytes);
                    if (err) {
                        goto error;
                    }
                    size_t numStringDataBytes = writeContext->value.stringValue.numBytes;
                    err = HAPJSONUtilsEscapeStringData(
                            &buffer->data[bufferMark], buffer->limit - bufferMark, &numStringDataBytes);
                    if (err) {
                        goto error;
                    }
                    buffer->position = bufferMark + numStringDataBytes;
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, "\"");
                } break;
            }
            if (err) {
                goto error;
            }
        }
        err = HAPIPByteBufferAppendStringWithFormat(buffer, "}");
        if (err) {
            goto error;
        }
    }
    HAPAssert(i == numWriteContexts);
    err = HAPIPByteBufferAppendStringWithFormat(buffer, "]}");
    if (err) {
        goto error;
    }
    return kHAPError_None;
error:
    return kHAPError_OutOfResources;
}

HAP_RESULT_USE_CHECK
size_t HAPIPAccessoryProtocolGetNumEventNotificationBytes(
        HAPAccessoryServerRef* server,
        HAPIPReadContextRef* readContexts,
        size_t numReadContexts) {
    HAPPrecondition(server);
    HAPPrecondition(readContexts);

    size_t r, i;

    r = 22;
    for (i = 0; i < numReadContexts; i++) {
        HAPIPReadContext* readContext = (HAPIPReadContext*) &readContexts[i];

        r += (i == 0 ? 24 : 25) + HAPUInt64GetNumDescriptionBytes(readContext->aid) +
             HAPUInt64GetNumDescriptionBytes(readContext->iid);
        if (readContext->status == 0) {
            const HAPBaseCharacteristic* chr_ = GetCharacteristic(server, readContext->aid, readContext->iid);
            HAPAssert(chr_);
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                    r += 1;
                } break;
                case kHAPCharacteristicFormat_UInt8:
                case kHAPCharacteristicFormat_UInt16:
                case kHAPCharacteristicFormat_UInt32:
                case kHAPCharacteristicFormat_UInt64: {
                    r += HAPUInt64GetNumDescriptionBytes(readContext->value.unsignedIntValue);
                } break;
                case kHAPCharacteristicFormat_Int: {
                    r += HAPInt32GetNumDescriptionBytes(readContext->value.intValue);
                } break;
                case kHAPCharacteristicFormat_Float: {
                    r += HAPJSONUtilsGetFloatNumDescriptionBytes(readContext->value.floatValue);
                } break;
                case kHAPCharacteristicFormat_String:
                case kHAPCharacteristicFormat_TLV8:
                case kHAPCharacteristicFormat_Data: {
                    r += 2 + HAPJSONUtilsGetNumEscapedStringDataBytes(
                                     HAPNonnull(readContext->value.stringValue.bytes),
                                     readContext->value.stringValue.numBytes);
                } break;
            }
        } else {
            r += 4;
        }
    }
    HAPAssert(i == numReadContexts);
    return r;
}

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetEventNotificationBytes(
        HAPAccessoryServerRef* server,
        HAPIPReadContextRef* readContexts,
        size_t numReadContexts,
        HAPIPByteBuffer* buffer) {
    HAPPrecondition(server);
    HAPPrecondition(readContexts);
    HAPPrecondition(buffer);

    HAPError err;

    size_t i;
    char scratch_string[64];

    err = HAPIPByteBufferAppendStringWithFormat(buffer, "{\"characteristics\":[");
    if (err) {
        goto error;
    }
    for (i = 0; i < numReadContexts; i++) {
        HAPIPReadContext* readContext = (HAPIPReadContext*) &readContexts[i];

        err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s{\"aid\":", i == 0 ? "" : ",");
        if (err) {
            goto error;
        }
        err = HAPUInt64GetDescription(uintval(readContext->aid), scratch_string, sizeof scratch_string);
        HAPAssert(!err);
        err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", scratch_string);
        if (err) {
            goto error;
        }
        err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"iid\":");
        if (err) {
            goto error;
        }
        err = HAPUInt64GetDescription(uintval(readContext->iid), scratch_string, sizeof scratch_string);
        HAPAssert(!err);
        err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", scratch_string);
        if (err) {
            goto error;
        }

        if (readContext->status == 0) {
            const HAPBaseCharacteristic* chr_ = GetCharacteristic(server, readContext->aid, readContext->iid);
            HAPAssert(chr_);
            switch (chr_->format) {
                case kHAPCharacteristicFormat_Bool: {
                    err = HAPIPByteBufferAppendStringWithFormat(
                            buffer, ",\"value\":%s}", readContext->value.unsignedIntValue ? "1" : "0");
                } break;
                case kHAPCharacteristicFormat_UInt8:
                case kHAPCharacteristicFormat_UInt16:
                case kHAPCharacteristicFormat_UInt32:
                case kHAPCharacteristicFormat_UInt64: {
                    err = HAPUInt64GetDescription(
                            uintval(readContext->value.unsignedIntValue), scratch_string, sizeof scratch_string);
                    HAPAssert(!err);
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":%s}", scratch_string);
                } break;
                case kHAPCharacteristicFormat_Int: {
                    err = HAPIPByteBufferAppendStringWithFormat(
                            buffer, ",\"value\":%ld}", (long) readContext->value.intValue);
                } break;
                case kHAPCharacteristicFormat_Float: {
                    err = HAPJSONUtilsGetFloatDescription(
                            readContext->value.floatValue, scratch_string, sizeof scratch_string);
                    HAPAssert(!err);
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":%s}", scratch_string);
                } break;
                case kHAPCharacteristicFormat_String:
                case kHAPCharacteristicFormat_TLV8:
                case kHAPCharacteristicFormat_Data: {
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":\"");
                    if (err) {
                        goto error;
                    }
                    size_t bufferMark = buffer->position;
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, "%s", readContext->value.stringValue.bytes);
                    if (err) {
                        goto error;
                    }
                    size_t numStringDataBytes = readContext->value.stringValue.numBytes;
                    err = HAPJSONUtilsEscapeStringData(
                            &buffer->data[bufferMark], buffer->limit - bufferMark, &numStringDataBytes);
                    if (err) {
                        goto error;
                    }
                    buffer->position = bufferMark + numStringDataBytes;
                    err = HAPIPByteBufferAppendStringWithFormat(buffer, "\"}");
                } break;
            }
        } else {
            err = HAPIPByteBufferAppendStringWithFormat(buffer, ",\"value\":null}");
        }
        if (err) {
            goto error;
        }
    }
    HAPAssert(i == numReadContexts);
    err = HAPIPByteBufferAppendStringWithFormat(buffer, "]}");
    if (err) {
        goto error;
    }
    return kHAPError_None;
error:
    return kHAPError_OutOfResources;
}

HAP_RESULT_USE_CHECK
HAPError HAPIPAccessoryProtocolGetCharacteristicWritePreparation(
        const char* bytes,
        size_t numBytes,
        uint64_t* ttl,
        uint64_t* pid) {
    // See HomeKit Accessory Protocol Specification R14
    // Section 6.7.2.4 Timed Write Procedures
    bool hasTTL, hasPID;
    struct util_json_reader json_reader;
    size_t i, j, k, n;
    uint64_t x;

    HAPAssert(bytes != NULL);
    HAPAssert(ttl != NULL);
    HAPAssert(pid != NULL);

    HAPError err;

    hasTTL = false;
    hasPID = false;
    util_json_reader_init(&json_reader);
    k = util_json_reader_read(&json_reader, bytes, numBytes);
    if (json_reader.state != util_JSON_READER_STATE_BEGINNING_OBJECT) {
        goto error;
    }
    HAPAssert(k <= numBytes);
    do {
        k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
        if (json_reader.state != util_JSON_READER_STATE_BEGINNING_STRING) {
            goto error;
        }
        HAPAssert(k <= numBytes);
        i = k;
        k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
        if (json_reader.state != util_JSON_READER_STATE_COMPLETED_STRING) {
            goto error;
        }
        HAPAssert(k <= numBytes);
        j = k;
        k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
        if (json_reader.state != util_JSON_READER_STATE_AFTER_NAME_SEPARATOR) {
            goto error;
        }
        HAPAssert(i <= j);
        HAPAssert(j <= k);
        HAPAssert(k <= numBytes);
        if ((j - i == 5) && HAPRawBufferAreEqual(&bytes[i], "\"ttl\"", 5)) {
            if (hasTTL) {
                HAPLog(&logObject, "Multiple TTL entries detected.");
                goto error;
            }
            k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
            if (json_reader.state == util_JSON_READER_STATE_BEGINNING_NUMBER) {
                HAPAssert(k <= numBytes);
                i = k;
                k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
                if (json_reader.state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                    goto error;
                }
                HAPAssert(i <= k);
                HAPAssert(k <= numBytes);
                n = try_read_uint64(&bytes[i], k - i, &x);
                // Specified TTL in milliseconds the controller requests the accessory to securely execute a write
                // command. Maximum value of this is 9007199254740991.
                // See HomeKit Accessory Protocol Specification R14
                // Table 6-3 Properties of Characteristic Objects in JSON
                if ((n == k - i) && (x <= 9007199254740991)) {
                    *ttl = x;
                    hasTTL = true;
                } else {
                    HAPLogBuffer(&logObject, &bytes[i], k - i, "Invalid TTL requested.");
                    goto error;
                }
            } else {
                goto error;
            }
        } else if ((j - i == 5) && HAPRawBufferAreEqual(&bytes[i], "\"pid\"", 5)) {
            if (hasPID) {
                HAPLog(&logObject, "Multiple PID entries detected.");
                goto error;
            }
            k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
            if (json_reader.state == util_JSON_READER_STATE_BEGINNING_NUMBER) {
                HAPAssert(k <= numBytes);
                i = k;
                k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
                if (json_reader.state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                    goto error;
                }
                HAPAssert(i <= k);
                HAPAssert(k <= numBytes);
                n = try_read_uint64(&bytes[i], k - i, &x);
                // 64-bit unsigned integer assigned by the controller to uniquely identify the timed write
                // transaction.
                // See HomeKit Accessory Protocol Specification R14
                // Table 6-3 Properties of Characteristic Objects in JSON
                if (n == k - i) {
                    *pid = x;
                    hasPID = true;
                } else {
                    HAPLogBuffer(&logObject, &bytes[i], k - i, "Invalid PID requested.");
                    goto error;
                }
            } else {
                goto error;
            }
        } else {
            size_t skippedBytes;
            err = HAPJSONUtilsSkipValue(&json_reader, &bytes[k], numBytes - k, &skippedBytes);
            if (err) {
                HAPAssert((err == kHAPError_InvalidData) || (err == kHAPError_OutOfResources));
                goto error;
            }
            k += skippedBytes;
        }
        HAPAssert(k <= numBytes);
        k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
    } while ((k < numBytes) && (json_reader.state == util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR));
    HAPAssert(
            (k == numBytes) || ((k < numBytes) && (json_reader.state != util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR)));
    if (json_reader.state != util_JSON_READER_STATE_COMPLETED_OBJECT) {
        goto error;
    }
    k += util_json_reader_read(&json_reader, &bytes[k], numBytes - k);
    if (k < numBytes) {
        goto error;
    } else {
        HAPAssert(k == numBytes);
        HAPAssert(
                (json_reader.state == util_JSON_READER_STATE_COMPLETED_OBJECT) ||
                (json_reader.state == util_JSON_READER_STATE_READING_WHITESPACE));
    }
    if (!hasTTL || !hasPID) {
        HAPLog(&logObject, "TTL or PID missing in request.");
        goto error;
    }
    return kHAPError_None;
error:
    return kHAPError_InvalidData;
}
