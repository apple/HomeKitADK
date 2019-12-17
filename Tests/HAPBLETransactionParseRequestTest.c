// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <stdlib.h>

#include "HAP+Internal.h"

const char* TestArgs[][7] = {
    { NULL, "2048", "25", "0x01", "0x42", "0x0001", NULL },   { NULL, "2048", "25", "0x01", "0x42", "0x0001", "8" },
    { NULL, "2048", "25", "0x01", "0x42", "0x0001", "64" },   { NULL, "2048", "25", "0x01", "0x42", "0x0001", "1024" },
    { NULL, "2048", "25", "0x01", "0x42", "0x0001", "2048" }, { NULL, "2048", "25", "0x01", "0x42", "0x0001", "2049" },
    { NULL, "2048", "25", "0x01", "0x42", "0x0001", "4096" },
};

static unsigned long ParseInt(const char* stringValue, int base) {
    HAPPrecondition(stringValue);

    char* end;
    unsigned long intValue = strtoul(stringValue, &end, base);
    HAPAssert(end == &stringValue[HAPStringGetNumBytes(stringValue)]);
    return intValue;
}

static int Test(int argc, const char* argv[]) {
    // Input arguments:
    // argv[1] - Transaction body buffer size. 0 for a NULL buffer.
    // argv[2] - MTU. Must be >= 7.
    // argv[3] - HAP Opcode (hex).
    // argv[4] - TID (hex).
    // argv[5] - IID (hex).
    // argv[6] - Body length. If no body should be included, omit this arg.
    HAPPrecondition(argc == 6 || argc == 7);

    HAPError err;

    // Process arguments.
    uint64_t maxBodyBytes;
    err = HAPUInt64FromString(argv[1], &maxBodyBytes);
    HAPPrecondition(!err);
    uint64_t mtu;
    err = HAPUInt64FromString(argv[2], &mtu);
    HAPPrecondition(!err);
    HAPPrecondition(mtu >= 7);
    unsigned long opcode = ParseInt(argv[3], 16);
    HAPPrecondition(opcode <= UINT8_MAX);
    unsigned long tid = ParseInt(argv[4], 16);
    HAPPrecondition(tid <= UINT8_MAX);
    unsigned long iid = ParseInt(argv[5], 16);
    HAPPrecondition(iid <= UINT16_MAX);
    HAPPrecondition(iid);
    bool hasBody = argc == 7;
    uint64_t numBodyBytes = 0;
    if (hasBody) {
        err = HAPUInt64FromString(argv[6], &mtu);
        HAPPrecondition(!err);
        HAPPrecondition(numBodyBytes <= UINT16_MAX);
    }

    // Allocate body buffer
    static uint8_t bodyBytes[4096];
    HAPAssert(maxBodyBytes <= sizeof bodyBytes);

    // Initialize tx.
    HAPBLETransaction transaction;
    HAPBLETransactionCreate(&transaction, bodyBytes, maxBodyBytes);

    // Write data.
    static uint8_t fragmentBytes[4096];
    HAPAssert(mtu <= sizeof fragmentBytes);
    bool first = true;
    for (size_t remainingBodyBytes = numBodyBytes; first || remainingBodyBytes;) {
        size_t o = 0;

        // Write header.
        if (first) {
            first = false;
            fragmentBytes[o++] = 0x00; // First Fragment, Request, 1 Byte Control Field.
            fragmentBytes[o++] = (uint8_t) opcode;
            fragmentBytes[o++] = (uint8_t) tid;
            HAPWriteLittleUInt16(&fragmentBytes[o], iid);
            o += 2;
            if (hasBody) {
                HAPWriteLittleUInt16(&fragmentBytes[o], numBodyBytes);
                o += 2;
            }
            HAPAssert(o <= mtu);
        } else {
            fragmentBytes[o++] = 0x80; // Continuation, Request, 1 Byte Control Field.
            fragmentBytes[o++] = (uint8_t) tid;
            HAPAssert(o <= mtu);
        }

        // Synthesize body.
        while (o < mtu && remainingBodyBytes) {
            fragmentBytes[o++] = (uint8_t)((numBodyBytes - remainingBodyBytes) & 0xFF);
            remainingBodyBytes--;
        }
        HAPAssert(o <= mtu);

        // Process fragment.
        HAPAssert(!HAPBLETransactionIsRequestAvailable(&transaction));
        err = HAPBLETransactionHandleWrite(&transaction, fragmentBytes, o);
        HAPAssert(!err);
    }

    // Get request.
    HAPAssert(HAPBLETransactionIsRequestAvailable(&transaction));
    HAPBLETransactionRequest request;
    err = HAPBLETransactionGetRequest(&transaction, &request);
    if (numBodyBytes > maxBodyBytes) {
        HAPAssert(err == kHAPError_OutOfResources);
        return 0;
    }
    HAPAssert(!err);

    // Verify request.
    HAPAssert(request.opcode == opcode);
    HAPAssert(request.iid == iid);
    for (size_t i = 0; i < ((HAPTLVReader*) &request.bodyReader)->numBytes; i++) {
        uint8_t* b = ((HAPTLVReader*) &request.bodyReader)->bytes;
        HAPAssert(b[i] == (i & 0xFF));
    }

    return 0;
}

int main(int argc, char* argv[]) {
    for (size_t i = 0; i < HAPArrayCount(TestArgs); ++i) {
        HAPAssert(Test(TestArgs[i][6] ? 7 : 6, TestArgs[i]) == 0);
    }

    return 0;
}
