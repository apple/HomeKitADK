// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_TCP_STREAM_MANAGER_INIT_H
#define HAP_PLATFORM_TCP_STREAM_MANAGER_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

typedef struct {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    char _;
    /**@endcond */
} HAPPlatformTCPStream;

/**
 * TCP stream manager.
 */
struct HAPPlatformTCPStreamManager {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    char _;
    /**@endcond */
};

/**
 * Default buffer size.
 */
#define kHAPPlatformTCPStreamManager_NumBufferBytes ((size_t) 4 * 1024)

/**
 * Key-value store initialization options.
 */
typedef struct {
    /**
     * Local network interface name on which to bind the TCP stream manager.
     * A value of NULL will use all available network interfaces.
     */
    const char* _Nullable interfaceName;

    /**
     * Local port number on which to bind the TCP stream manager.
     * A value of 0 will use an unused port number from the ephemeral port range.
     */
    uint16_t port;

    /**
     * Maximum number of concurrent TCP streams.
     */
    size_t maxConcurrentTCPStreams;
} HAPPlatformTCPStreamManagerOptions;

/**
 * Initializes a TCP stream manager.
 *
 * @param[out] tcpStreamManager     Pointer to an allocated but uninitialized HAPPlatformTCPStreamManager structure.
 * @param      options              Initialization options.
 */
void HAPPlatformTCPStreamManagerCreate(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        const HAPPlatformTCPStreamManagerOptions* options);

/**
 * Releases resources associated with an initialized TCP stream manager instance.
 *
 * - IMPORTANT: Do not use this method on TCP stream manager structures that are not initialized!
 *
 * @param      tcpStreamManager     TCP stream manager.
 */
void HAPPlatformTCPStreamManagerRelease(HAPPlatformTCPStreamManagerRef tcpStreamManager);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
