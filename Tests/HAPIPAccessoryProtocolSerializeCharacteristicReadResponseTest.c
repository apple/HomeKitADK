// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPFloatCharacteristic testCharacteristic = { .iid = 3,
                                                           .format = kHAPCharacteristicFormat_Float,
                                                           .characteristicType =
                                                                   &kHAPCharacteristicType_CurrentTemperature,
                                                           .properties = { .readable = true } };

static const HAPService testService = { .iid = 2,
                                        .serviceType = &kHAPServiceType_Thermostat,
                                        .characteristics =
                                                (const HAPCharacteristic* const[]) { &testCharacteristic, NULL } };

static const HAPAccessory testAccessory = {
    .aid = 1,
    .services = (const HAPService* const[]) { &testService, NULL },
};

static HAPAccessoryServerRef* testAccessoryServer =
        (HAPAccessoryServerRef*) &(HAPAccessoryServer) { .primaryAccessory = &testAccessory };

static void
        ReadSingleCharacteristicReadResponseValue(char* data, size_t numDataBytes, char* value, size_t maxValueBytes) {
    HAPPrecondition(data);
    HAPPrecondition(value);

    HAPError err;

    struct util_json_reader jsonReader;
    util_json_reader_init(&jsonReader);

    size_t k = util_json_reader_read(&jsonReader, data, numDataBytes);
    HAPAssert(jsonReader.state == util_JSON_READER_STATE_BEGINNING_OBJECT);
    do {
        HAPAssert(k <= numDataBytes);
        k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
        HAPAssert(jsonReader.state == util_JSON_READER_STATE_BEGINNING_STRING);
        size_t i = k;
        HAPAssert(k <= numDataBytes);
        k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
        HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_STRING);
        size_t j = k;
        HAPAssert(k <= numDataBytes);
        k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
        HAPAssert(jsonReader.state == util_JSON_READER_STATE_AFTER_NAME_SEPARATOR);
        HAPAssert(i <= j);
        HAPAssert(j <= k);
        HAPAssert(k <= numDataBytes);
        if ((j - i == 17) && HAPRawBufferAreEqual(&data[i], "\"characteristics\"", 17)) {
            HAPAssert(k <= numDataBytes);
            k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
            HAPAssert(jsonReader.state == util_JSON_READER_STATE_BEGINNING_ARRAY);
            HAPAssert(k <= numDataBytes);
            k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
            HAPAssert(jsonReader.state == util_JSON_READER_STATE_BEGINNING_OBJECT);
            do {
                HAPAssert(k <= numDataBytes);
                k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
                HAPAssert(jsonReader.state == util_JSON_READER_STATE_BEGINNING_STRING);
                i = k;
                HAPAssert(k <= numDataBytes);
                k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
                HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_STRING);
                j = k;
                HAPAssert(k <= numDataBytes);
                k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
                HAPAssert(jsonReader.state == util_JSON_READER_STATE_AFTER_NAME_SEPARATOR);
                HAPAssert(i <= j);
                HAPAssert(j <= k);
                HAPAssert(k <= numDataBytes);
                if ((j - i == 7) && HAPRawBufferAreEqual(&data[i], "\"value\"", 7)) {
                    HAPAssert(k <= numDataBytes);
                    k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
                    if (jsonReader.state == util_JSON_READER_STATE_BEGINNING_NUMBER) {
                        i = k;
                        HAPAssert(k <= numDataBytes);
                        k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
                        HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_NUMBER);
                        j = k;
                    } else {
                        HAPAssert(jsonReader.state == util_JSON_READER_STATE_BEGINNING_NULL);
                        i = k;
                        HAPAssert(k <= numDataBytes);
                        k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
                        HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_NULL);
                        j = k;
                    }
                    HAPAssert(i <= j);
                    HAPAssert(j <= k);
                    HAPAssert(k <= numDataBytes);
                    HAPAssert(j - i < maxValueBytes);
                    size_t v = 0;
                    while (i < j) {
                        value[v] = data[i];
                        v++;
                        i++;
                    }
                    value[v] = '\0';
                } else {
                    size_t skippedBytes;
                    err = HAPJSONUtilsSkipValue(&jsonReader, &data[k], numDataBytes - k, &skippedBytes);
                    HAPAssert(!err);
                    k += skippedBytes;
                }
                HAPAssert(k <= numDataBytes);
                k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
            } while ((k < numDataBytes) && (jsonReader.state == util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR));
            HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_OBJECT);
            HAPAssert(k <= numDataBytes);
            k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
            HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_ARRAY);
        } else {
            size_t skippedBytes;
            err = HAPJSONUtilsSkipValue(&jsonReader, &data[k], numDataBytes - k, &skippedBytes);
            HAPAssert(!err);
            k += skippedBytes;
        }
        HAPAssert(k <= numDataBytes);
        k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
    } while ((k < numDataBytes) && (jsonReader.state == util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR));
    HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_OBJECT);
    k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
    HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_OBJECT);
    HAPAssert(k == numDataBytes);
}

int main() {
    HAPError err;

    static HAPIPReadContextRef readContexts[1];

    static HAPIPReadContext* readContext = (HAPIPReadContext*) &readContexts[0];
    readContext->aid = testAccessory.aid;
    readContext->iid = testCharacteristic.iid;

    static HAPIPReadRequestParameters parameters;

    static char data[256];
    static HAPIPByteBuffer buffer = { .data = (char*) data, .capacity = sizeof data, .limit = sizeof data };

    static char value[16];

    {
        buffer.position = 0;
        readContext->value.floatValue = -1.0f / 0.0f;

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);
        HAPAssert(HAPRawBufferAreEqual(value, "null", 4));
    }
    {
        buffer.position = 0;
        readContext->value.floatValue = 1.0f / 0.0f;

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);
        HAPAssert(HAPRawBufferAreEqual(value, "null", 4));
    }
    {
        buffer.position = 0;
        readContext->value.floatValue = 0.0f / 0.0f;

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);
        HAPAssert(HAPRawBufferAreEqual(value, "null", 4));
    }
    {
        buffer.position = 0;
        readContext->value.floatValue = 0.0f;

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);
        HAPAssert(HAPRawBufferAreEqual(value, "0", 1));
    }
    {
        buffer.position = 0;
        readContext->value.floatValue = -0.0f;

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);
        HAPAssert(HAPRawBufferAreEqual(value, "-0", 2));
    }

    return 0;
}
