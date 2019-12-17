// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

#if defined(HAP_IDENTIFICATION) || defined(HAP_VERSION) || defined(HAP_BUILD)
#ifdef _MSC_VER
#ifdef armv7
// Windows CE 7.0 workaround to serialize the string "/ARM/MSVC/armv7-wince7/".
#undef armv7
#endif
#define COMMAND_LINE_STRINGIFY_INNER(X) #X
#define COMMAND_LINE_STRINGIFY(X)       COMMAND_LINE_STRINGIFY_INNER(X)
#else
#define COMMAND_LINE_STRINGIFY(X) X
#endif
#endif

HAP_RESULT_USE_CHECK
uint32_t HAPGetCompatibilityVersion(void) {
    return HAP_COMPATIBILITY_VERSION;
}

const char* HAPGetIdentification(void) {
#ifdef HAP_IDENTIFICATION
    const char* identification = COMMAND_LINE_STRINGIFY(HAP_IDENTIFICATION);
#else
    const char* identification = "Unknown";
#endif

    return identification;
}

HAP_RESULT_USE_CHECK
const char* HAPGetVersion(void) {
#ifdef HAP_VERSION
    const char* version = COMMAND_LINE_STRINGIFY(HAP_VERSION);
#else
    const char* version = "Internal";
#endif

    return version;
}

HAP_RESULT_USE_CHECK
const char* HAPGetBuild(void) {
#ifdef HAP_BUILD
    const char* build = COMMAND_LINE_STRINGIFY(HAP_BUILD);
#else
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdate-time")
    const char* build = __DATE__ " " __TIME__;
    HAP_DIAGNOSTIC_POP
#endif

    return build;
}
