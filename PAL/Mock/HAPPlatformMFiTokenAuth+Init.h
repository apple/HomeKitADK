// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_MFI_TOKEN_AUTH_INIT_H
#define HAP_PLATFORM_MFI_TOKEN_AUTH_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"
#include "HAPPlatformMFiTokenAuth+Init.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Software token provider initialization options.
 */
typedef struct {
    /**
     * Key-value store.
     */
    HAPPlatformKeyValueStoreRef keyValueStore;
} HAPPlatformMFiTokenAuthOptions;

/**
 * Software Token provider.
 */
struct HAPPlatformMFiTokenAuth {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    char _;
    /**@endcond */
};

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
