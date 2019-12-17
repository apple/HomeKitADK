// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

const char* TestArgs[][4] = {
    { "", "7OSX", "E1:91:1A:70:85:AA", "C9FE1BCF" },
    { "", "7OSX", "C8:D8:58:C6:63:F5", "EF5D8E9B" },
};

int Test(int argc, const char* argv[]) {
    HAPError err;

    // Input arguments:
    // argv[1] - Setup ID. Format XXXX.
    // argv[2] - Device ID. Format XX:XX:XX:XX:XX:XX, uppercase.
    // argv[3] - Expected setup hash (hex, uppercase).
    HAPPrecondition(argc == 4);

    // Process arguments.
    HAPPrecondition(HAPAccessorySetupIsValidSetupID(argv[1]));
    const HAPSetupID* setupID = (const HAPSetupID*) argv[1];
    HAPPrecondition(HAPStringGetNumBytes(argv[2]) == sizeof(HAPDeviceIDString) - 1);
    const HAPDeviceIDString* deviceIDString = (const HAPDeviceIDString*) argv[2];

    // Derive setup hash.
    HAPAccessorySetupSetupHash setupHash;
    HAPAccessorySetupGetSetupHash(&setupHash, setupID, deviceIDString);

    // Compare with expectation.
    HAPPrecondition(HAPStringGetNumBytes(argv[3]) == 2 * sizeof setupHash.bytes);
    for (size_t i = 0; i < sizeof setupHash.bytes; i++) {
        char setupHashHexString[3];
        err = HAPStringWithFormat(setupHashHexString, sizeof setupHashHexString, "%02X", setupHash.bytes[i]);
        HAPAssert(!err);
        HAPAssert(HAPStringGetNumBytes(setupHashHexString) == sizeof setupHashHexString - 1);
        HAPAssert(HAPRawBufferAreEqual(setupHashHexString, &argv[3][2 * i], 2));
    }

    return 0;
}

int main(int argc, char* argv[]) {
    for (size_t i = 0; i < HAPArrayCount(TestArgs); ++i) {
        HAPAssert(Test(4, TestArgs[i]) == 0);
    }
    return 0;
}
