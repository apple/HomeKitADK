// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_FILE_HANDLE_H
#define HAP_PLATFORM_FILE_HANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * File handle type, representing the registration of a platform-specific file descriptor.
 */
typedef uintptr_t HAPPlatformFileHandleRef;

/**
 * Events that may occur on a file descriptor.
 */
typedef struct {
    /**
     * The platform-specific file descriptor is ready for reading.
     */
    bool isReadyForReading : 1;

    /**
     * The platform-specific file descriptor is ready for writing.
     */
    bool isReadyForWriting : 1;

    /**
     * The platform-specific file descriptor has an error condition pending.
     */
    bool hasErrorConditionPending : 1;
} HAPPlatformFileHandleEvent;
HAP_STATIC_ASSERT(sizeof(HAPPlatformFileHandleEvent) == 1, HAPPlatformFileHandleEvent);

/**
 * Callback that is invoked when one or more events occur on a given file descriptor.
 *
 * @param      fileHandle           Non-zero file handle.
 * @param      fileHandleEvents     The set of file handle events that occurred.
 * @param      context              The context parameter previously passed to the
 *                                  HAPPlatformFileHandleRegister function.
 */
typedef void (*HAPPlatformFileHandleCallback)(
        HAPPlatformFileHandleRef fileHandle,
        HAPPlatformFileHandleEvent fileHandleEvents,
        void* _Nullable context);

/**
 * Registers a platform-specific file descriptor for which a callback shall be invoked when one or more events occur.
 *
 * - The platform-specific file descriptor must not already be registered.
 *
 * - The callback is never invoked synchronously.
 *
 * @param[out] fileHandle           Non-zero file handle representing the registration, if successful.
 * @param      fileDescriptor       Platform-specific file descriptor.
 * @param      interests            Set of file handle events on which the callback shall be invoked.
 * @param      callback             Function to call when one or more events occur on the given file descriptor.
 * @param      context              Context that shall be passed to the callback.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If no more resources for registrations can be allocated.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileHandleRegister(
        HAPPlatformFileHandleRef* fileHandle,
        int fileDescriptor,
        HAPPlatformFileHandleEvent interests,
        HAPPlatformFileHandleCallback callback,
        void* _Nullable context);

/**
 * Updates a registration for a previously registered platform-specific file descriptor.
 *
 * @param      fileHandle           Non-zero file handle.
 * @param      interests            Set of file handle events on which the callback shall be invoked.
 * @param      callback             Function to call when one or more events occur on the given file descriptor.
 * @param      context              Context that shall be passed to the callback.
 */
void HAPPlatformFileHandleUpdateInterests(
        HAPPlatformFileHandleRef fileHandle,
        HAPPlatformFileHandleEvent interests,
        HAPPlatformFileHandleCallback callback,
        void* _Nullable context);

/**
 * Removes a registration for a previously registered platform-specific file descriptor.
 *
 * - Any use of a file handle after it has been deregistered results in undefined behavior.
 *
 * @param      fileHandle           Non-zero file handle.
 */
void HAPPlatformFileHandleDeregister(HAPPlatformFileHandleRef fileHandle);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
