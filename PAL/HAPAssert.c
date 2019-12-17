// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#undef HAP_DISALLOW_USE_IGNORED
#define HAP_DISALLOW_USE_IGNORED 1

#include "HAPPlatform.h"

HAP_NORETURN
void HAPAssertAbortInternal(void) {
    HAPPlatformAbort();
}

HAP_NORETURN
void HAPAssertInternal(const char* callerFunction, const char* callerFile, int callerLine) {
    HAPLogFault(&kHAPLog_Default, "assertion failed - %s @ %s:%d", callerFunction, callerFile, callerLine);
    HAPPlatformAbort();
}

HAP_NORETURN
void HAPAssertionFailureInternal(const char* callerFunction, const char* callerFile, int callerLine) {
    HAPLogFault(&kHAPLog_Default, "assertion failed - %s @ %s:%d", callerFunction, callerFile, callerLine);
    HAPPlatformAbort();
}

HAP_NORETURN
void HAPPreconditionInternal(const char* condition, const char* callerFunction) {
    HAPLogFault(&kHAPLog_Default, "precondition failed: %s - %s", condition, callerFunction);
    HAPPlatformAbort();
}

HAP_NORETURN
void HAPPreconditionFailureInternal(const char* callerFunction) {
    HAPLogFault(&kHAPLog_Default, "precondition failed - %s", callerFunction);
    HAPPlatformAbort();
}

HAP_NORETURN
void HAPFatalErrorInternal(const char* callerFunction, const char* callerFile, int callerLine) {
    HAPLogFault(&kHAPLog_Default, "fatal error - %s @ %s:%d", callerFunction, callerFile, callerLine);
    HAPPlatformAbort();
}
