// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_SYSTEM_COMMAND_H
#define HAP_PLATFORM_SYSTEM_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Synchronously runs a system command and stores the result of STDOUT in the provided buffer.
 *
 * Passes an empty environment to HAPPlatformSystemCommandRunWithEnvironment.
 *
 * @param      command              Command to be run.
 * @param[out] bytes                Buffer that will be filled with data.
 * @param      maxBytes             Maximum number of bytes that may be filled into the buffer.
 * @param[in,out] numBytes          Effective number of bytes written to the buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If the process does not exit successfully.
 * @return kHAPError_OutOfResources If the buffer was not big enough to store the result.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformSystemCommandRun(
        char* _Nullable const command[_Nonnull],
        char* bytes,
        size_t maxBytes,
        size_t* numBytes);

/**
 * Synchronously runs a system command with an environment and stores the result of STDOUT in the provided buffer.
 *
 * @param      command              Command to be run.
 * @param      environment          Environment to be passed to the command.
 * @param[out] bytes                Buffer that will be filled with data.
 * @param      maxBytes             Maximum number of bytes that may be filled into the buffer.
 * @param[in,out] numBytes          Effective number of bytes written to the buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If the process does not exit successfully.
 * @return kHAPError_OutOfResources If the buffer was not big enough to store the result.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformSystemCommandRunWithEnvironment(
        char* _Nullable const command[_Nonnull],
        char* _Nullable const environment[_Nullable],
        char* bytes,
        size_t maxBytes,
        size_t* numBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
