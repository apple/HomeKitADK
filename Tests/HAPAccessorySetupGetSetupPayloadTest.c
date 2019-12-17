// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

const char* TestArgs[][8] = {
    { NULL, "518-08-582", "7OSX", "0", "1", "0", "7", "X-HM://0071WK4SM7OSX" },
    { NULL, "000-00-000", "0000", "0", "1", "0", "7", "X-HM://00711PP1C0000" },
    { NULL, "000-00-000", "0000", "1", "1", "0", "7", "X-HM://00739MG3K0000" },
    { NULL, "518-08-582", "7OSX", "0", "0", "1", "7", "X-HM://0076CDMX27OSX" },
};

int Test(int argc, const char* argv[]) {
    // Input arguments:
    // argv[1] - Setup code. Format XXX-XX-XXX. 000-00-000 for NULL.
    // argv[2] - Setup ID. Format XXXX. 0000 together with NULL setup code for NULL.
    // argv[3] - Paired (1 or 0).
    // argv[4] - Supports HAP over IP (1 or 0).
    // argv[5] - Supports HAP over BLE (1 or 0).
    // argv[6] - Category.
    // argv[7] - Expected setup payload.
    HAPPrecondition(argc == 8);

    HAPError err;

    // Process arguments.
    const HAPSetupCode* setupCode = NULL;
    if (!HAPStringAreEqual(argv[1], "000-00-000")) {
        HAPPrecondition(HAPAccessorySetupIsValidSetupCode(argv[1]));
        setupCode = (const HAPSetupCode*) argv[1];
    }
    const HAPSetupID* setupID = NULL;
    if (!setupCode) {
        HAPPrecondition(HAPStringAreEqual(argv[2], "0000"));
    } else {
        HAPPrecondition(HAPAccessorySetupIsValidSetupID(argv[2]));
        setupID = (const HAPSetupID*) argv[2];
    }
    uint64_t isPaired;
    err = HAPUInt64FromString(argv[3], &isPaired);
    HAPPrecondition(!err);
    HAPPrecondition(isPaired == false || isPaired == true);
    uint64_t isSupported;
    err = HAPUInt64FromString(argv[4], &isSupported);
    HAPPrecondition(!err);
    uint64_t bleSupported;
    err = HAPUInt64FromString(argv[5], &bleSupported);
    HAPPrecondition(!err);
    HAPPrecondition(bleSupported == false || bleSupported == true);
    uint64_t category;
    err = HAPUInt64FromString(argv[6], &category);
    HAPPrecondition(!err);
    HAPPrecondition(category > 0 && category <= UINT16_MAX);

    // Derive setup payload.
    HAPSetupPayload setupPayload;
    HAPAccessorySetupSetupPayloadFlags flags = { .isPaired = (bool) isPaired,
                                                 .ipSupported = (bool) isSupported,
                                                 .bleSupported = (bool) bleSupported };
    HAPAccessorySetupGetSetupPayload(&setupPayload, setupCode, setupID, flags, (HAPAccessoryCategory) category);

    // Compare with expectation.
    HAPAssert(HAPStringAreEqual(setupPayload.stringValue, argv[7]));

    return 0;
}

int main(int argc, char* argv[]) {
    for (size_t i = 0; i < HAPArrayCount(TestArgs); ++i) {
        HAPAssert(Test(8, TestArgs[i]) == 0);
    }

    return 0;
}
