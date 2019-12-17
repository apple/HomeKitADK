// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

HAP_RESULT_USE_CHECK
double HAPDoubleFromBitPattern(uint64_t bitPattern) {
    double value;
    HAPAssert(sizeof value == sizeof bitPattern);
    HAPRawBufferCopyBytes(&value, &bitPattern, sizeof value);
    return value;
}

HAP_RESULT_USE_CHECK
uint64_t HAPDoubleGetBitPattern(double value) {
    uint64_t bitPattern;
    HAPAssert(sizeof bitPattern == sizeof value);
    HAPRawBufferCopyBytes(&bitPattern, &value, sizeof bitPattern);
    return bitPattern;
}
