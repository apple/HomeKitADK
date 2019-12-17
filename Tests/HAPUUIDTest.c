// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

int main() {
    HAPError err;

    uint8_t bytes[16];
    size_t numBytes;

    // 00000000-0000-1000-8000-0026BB765291.
    {
        err = HAPUUIDGetShortFormBytes(&(const HAPUUID) HAPUUIDCreateAppleDefined(0x0), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == 0);
    }

    // 0000003E-0000-1000-8000-0026BB765291.
    {
        err = HAPUUIDGetShortFormBytes(
                &(const HAPUUID) HAPUUIDCreateAppleDefined(0x3E), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == 1);
        HAPAssert(bytes[0] == 0x3E);
    }

    // 00000001-0000-1000-8000-0026BB765291.
    {
        err = HAPUUIDGetShortFormBytes(&(const HAPUUID) HAPUUIDCreateAppleDefined(0x1), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == 1);
        HAPAssert(bytes[0] == 0x01);
    }

    // 00000F25-0000-1000-8000-0026BB765291.
    {
        err = HAPUUIDGetShortFormBytes(
                &(const HAPUUID) HAPUUIDCreateAppleDefined(0xF25), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == 2);
        HAPAssert(bytes[0] == 0x25);
        HAPAssert(bytes[1] == 0x0F);
    }

    // 0000BBAB-0000-1000-8000-0026BB765291.
    {
        err = HAPUUIDGetShortFormBytes(
                &(const HAPUUID) HAPUUIDCreateAppleDefined(0xBBAB), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == 2);
        HAPAssert(bytes[0] == 0xAB);
        HAPAssert(bytes[1] == 0xBB);
    }

    // 00112233-0000-1000-8000-0026BB765291.
    {
        err = HAPUUIDGetShortFormBytes(
                &(const HAPUUID) HAPUUIDCreateAppleDefined(0x112233), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == 3);
        HAPAssert(bytes[0] == 0x33);
        HAPAssert(bytes[1] == 0x22);
        HAPAssert(bytes[2] == 0x11);
    }

    // 010004FF-0000-1000-8000-0026BB765291.
    {
        err = HAPUUIDGetShortFormBytes(
                &(const HAPUUID) HAPUUIDCreateAppleDefined(0x010004FF), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == 4);
        HAPAssert(bytes[0] == 0xFF);
        HAPAssert(bytes[1] == 0x04);
        HAPAssert(bytes[2] == 0x00);
        HAPAssert(bytes[3] == 0x01);
    }

    // FF000000-0000-1000-8000-0026BB765291.
    {
        err = HAPUUIDGetShortFormBytes(
                &(const HAPUUID) HAPUUIDCreateAppleDefined(0xFF000000), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == 4);
        HAPAssert(bytes[0] == 0x00);
        HAPAssert(bytes[1] == 0x00);
        HAPAssert(bytes[2] == 0x00);
        HAPAssert(bytes[3] == 0xFF);
    }

    // 34AB8811-AC7F-4340-BAC3-FD6A85F9943B.
    {
        const HAPUUID uuid = {
            { 0x3B, 0x94, 0xF9, 0x85, 0x6A, 0xFD, 0xC3, 0xBA, 0x40, 0x43, 0x7F, 0xAC, 0x11, 0x88, 0xAB, 0x34 }
        };
        err = HAPUUIDGetShortFormBytes(&uuid, bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == sizeof uuid.bytes);
        HAPAssert(HAPRawBufferAreEqual(bytes, uuid.bytes, numBytes));
    }

    return 0;
}
