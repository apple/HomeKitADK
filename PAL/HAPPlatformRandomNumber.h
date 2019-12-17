// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_RANDOM_NUMBER_H
#define HAP_PLATFORM_RANDOM_NUMBER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Fills a buffer with cryptographically securely generated random numbers.
 *
 * @param[out] bytes                Buffer to fill with random numbers.
 * @param      numBytes             Length of buffer.
 */
void HAPPlatformRandomNumberFill(void* bytes, size_t numBytes)
        HAP_DIAGNOSE_ERROR(!bytes && numBytes, "empty buffer cannot have a length");

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
