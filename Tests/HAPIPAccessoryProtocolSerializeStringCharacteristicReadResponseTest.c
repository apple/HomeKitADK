// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPFloatCharacteristic testCharacteristic = { .iid = 3,
                                                           .format = kHAPCharacteristicFormat_String,
                                                           .characteristicType = &kHAPCharacteristicType_Name,
                                                           .properties = { .readable = true } };

static const HAPService testService = { .iid = 2,
                                        .serviceType = &kHAPServiceType_AccessoryInformation,
                                        .characteristics =
                                                (const HAPCharacteristic* const[]) { &testCharacteristic, NULL } };

static const HAPAccessory testAccessory = { .aid = 1, .services = (const HAPService* const[]) { &testService, NULL } };

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
                    HAPAssert(jsonReader.state == util_JSON_READER_STATE_BEGINNING_STRING);
                    i = k;
                    HAPAssert(k <= numDataBytes);
                    k += util_json_reader_read(&jsonReader, &data[k], numDataBytes - k);
                    HAPAssert(jsonReader.state == util_JSON_READER_STATE_COMPLETED_STRING);
                    j = k;
                    HAPAssert(i <= j);
                    HAPAssert(j <= k);
                    HAPAssert(k <= numDataBytes);
                    HAPAssert(j - i >= 2);
                    HAPAssert(data[i] == '\"');
                    i++;
                    size_t v = 0;
                    while (j - i > 1) {
                        HAPAssert(v < maxValueBytes);
                        value[v] = data[i];
                        v++;
                        i++;
                    }
                    HAPAssert(data[i] == '\"');
                    i++;
                    HAPAssert(v < maxValueBytes);
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

    static char value[128];

    {
        buffer.position = 0;
        readContext->value.stringValue.bytes = "Home";
        readContext->value.stringValue.numBytes =
                HAPStringGetNumBytes(HAPNonnull(readContext->value.stringValue.bytes));

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);

        HAPAssert(HAPStringAreEqual(value, "Home"));
    }
    {
        buffer.position = 0;
        readContext->value.stringValue.bytes = "Home \"A\"";
        readContext->value.stringValue.numBytes =
                HAPStringGetNumBytes(HAPNonnull(readContext->value.stringValue.bytes));

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);

        HAPAssert(HAPStringAreEqual(value, "Home \\\"A\\\""));
    }
    {
        buffer.position = 0;
        char testString[] = { (char) 'a', (char) 'b', (char) 'c', (char) 0x1b, (char) 0x0 };
        readContext->value.stringValue.bytes = testString;
        readContext->value.stringValue.numBytes =
                HAPStringGetNumBytes(HAPNonnull(readContext->value.stringValue.bytes));

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);

        HAPAssert(HAPStringAreEqual(value, "abc\\u001b"));
    }
    {
        buffer.position = 0;
        char emoji0[] = { (char) 0xf0, (char) 0x9f, (char) 0x98, (char) 0x81, (char) 0x0 };
        readContext->value.stringValue.bytes = emoji0;
        readContext->value.stringValue.numBytes =
                HAPStringGetNumBytes(HAPNonnull(readContext->value.stringValue.bytes));

        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters, &buffer);
        HAPAssert(!err);

        HAPAssert(
                buffer.position ==
                HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        testAccessoryServer, readContexts, HAPArrayCount(readContexts), &parameters));

        ReadSingleCharacteristicReadResponseValue(buffer.data, buffer.position, value, sizeof value);

        char emoji1[] = { (char) 0xf0, (char) 0x9f, (char) 0x98, (char) 0x81, (char) 0x0 };
        HAPAssert(HAPStringAreEqual(value, emoji1));
    }

    return 0;
}
