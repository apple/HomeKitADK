// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "RunLoop" };

// This platform module must be implemented in any case.

void HAPPlatformRunLoopRun(void) {
    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}

void HAPPlatformRunLoopStop(void) {
    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformRunLoopScheduleCallback(
        HAPPlatformRunLoopCallback callback,
        void* _Nullable context,
        size_t contextSize) {
    HAPPrecondition(callback);
    HAPPrecondition(!contextSize || context);

    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}
