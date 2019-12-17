// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatformRunLoop+Init.h"

#import <Foundation/Foundation.h>

void HAPPlatformRunLoopCreate(const HAPPlatformRunLoopOptions* options) {
    HAPPrecondition(options);
    HAPPrecondition(options->keyValueStore);
}

void HAPPlatformRunLoopRelease(void) {
}

void HAPPlatformRunLoopRun(void) {
    CFRunLoopRun();
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformRunLoopScheduleCallback(
        HAPPlatformRunLoopCallback callback,
        void* _Nullable context,
        size_t contextSize) {
    dispatch_async(dispatch_get_main_queue(), ^{
        callback(context, contextSize);
    });
    return kHAPError_None;
}

void HAPPlatformRunLoopStop(void) {
    CFRunLoopStop(CFRunLoopGetCurrent());
}
