// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatformTimer.h"

#import <Foundation/Foundation.h>

static NSTimer* scheduleTimer(HAPTime deadline, HAPPlatformTimerCallback callback, void* _Nullable context) {
    NSTimeInterval interval = 0;
    HAPTime currentTime = HAPPlatformClockGetCurrent();

    if (deadline > currentTime) {
        interval = ((NSTimeInterval)(deadline - currentTime)) / 1000.0;
    }

    NSTimer* t = [NSTimer
            scheduledTimerWithTimeInterval:interval
                                   repeats:NO
                                     block:^(NSTimer* timer) {
                                         dispatch_async(dispatch_get_main_queue(), ^{
                                             NSLog(@"Timer fired %@ timer %@", [NSThread currentThread], timer);
                                             callback((HAPPlatformTimerRef)(__bridge void*) timer, context);
                                             NSLog(@"callback completed");
                                         });
                                     }];

    return t;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTimerRegister(
        HAPPlatformTimerRef* timer,
        HAPTime deadline,
        HAPPlatformTimerCallback callback,
        void* _Nullable context) {
    __block NSTimer* t;

    if (![NSThread isMainThread]) {
        // needs to be sync since timer has to be updated, but can only
        // do this if not already on the main thread
        dispatch_sync(dispatch_get_main_queue(), ^{
            t = scheduleTimer(deadline, callback, context);

            printf("timer dispatch register %p\n", t);
        });
    } else {
        t = scheduleTimer(deadline, callback, context);
        printf("timer register %p\n", t);
    }

    *timer = (HAPPlatformTimerRef)(__bridge_retained void*) t;

    return kHAPError_None;
}

void HAPPlatformTimerDeregister(HAPPlatformTimerRef timer_) {
    NSTimer* timer = (__bridge_transfer NSTimer*) (void*) timer_;
    [timer invalidate];
    timer = nil;
}
