// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

int main() {
    HAPError err;

    {
        // See HomeKit Accessory Protocol Specification R14
        // Section 6.7.2.4 Timed Write Procedures
        const char request[] =
                "{\n"
                "    \"ttl\" : 2500,\n"
                "    \"pid\": 11122333\n"
                "    \n"
                "}\n";
        uint64_t ttl;
        uint64_t pid;
        err = HAPIPAccessoryProtocolGetCharacteristicWritePreparation(request, sizeof request - 1, &ttl, &pid);
        HAPAssert(!err);
        HAPAssert(ttl == 2500);
        HAPAssert(pid == 11122333);
    }
    {
        // Detect duplicate TTL.
        const char request[] =
                "{\n"
                "    \"ttl\" : 2500,\n"
                "    \"pid\": 11122333,\n"
                "    \n"
                "    \"ttl\" : 2500\n"
                "}\n";
        uint64_t ttl;
        uint64_t pid;
        err = HAPIPAccessoryProtocolGetCharacteristicWritePreparation(request, sizeof request - 1, &ttl, &pid);
        HAPAssert(err == kHAPError_InvalidData);
    }
    {
        // Detect duplicate PID.
        const char request[] =
                "{\n"
                "    \"ttl\" : 2500,\n"
                "    \"pid\": 11122333,\n"
                "    \n"
                "    \"pid\": 11122333\n"
                "}\n";
        uint64_t ttl;
        uint64_t pid;
        err = HAPIPAccessoryProtocolGetCharacteristicWritePreparation(request, sizeof request - 1, &ttl, &pid);
        HAPAssert(err == kHAPError_InvalidData);
    }

    return 0;
}
