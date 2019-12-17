// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

#include <stdlib.h>
#include <sys/utsname.h>

static struct utsname* sysinfo = NULL;

static struct utsname* SystemInfo() {
    if (!sysinfo) {
        sysinfo = (struct utsname*) malloc(sizeof(struct utsname));
        int ret = uname(sysinfo);
        HAPAssert(!ret);
    }
    return sysinfo;
}

HAP_RESULT_USE_CHECK
uint32_t HAPPlatformGetCompatibilityVersion(void) {
    return HAP_PLATFORM_COMPATIBILITY_VERSION;
}

HAP_RESULT_USE_CHECK
const char* HAPPlatformGetIdentification(void) {
    return SystemInfo()->sysname;
}

HAP_RESULT_USE_CHECK
const char* HAPPlatformGetVersion(void) {
    return SystemInfo()->release;
}

HAP_RESULT_USE_CHECK
const char* HAPPlatformGetBuild(void) {
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdate-time")
    const char* build = __DATE__ " " __TIME__;
    HAP_DIAGNOSTIC_POP
    return build;
}
