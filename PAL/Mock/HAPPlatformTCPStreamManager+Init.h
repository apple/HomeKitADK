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

// Opaque type. Do not use directly.
/**@cond */
typedef struct {
    HAPPlatformTCPStreamManagerRef tcpStreamManager;

    bool isActive : 1;
    bool isConnected : 1;

    HAPPlatformTCPStreamEvent interests;
    HAPPlatformTCPStreamEventCallback _Nullable callback;
    void* _Nullable context;
    HAPPlatformTimerRef invokeCallbackTimer;

    struct {
        void* _Nullable bytes;
        size_t maxBytes;
        size_t numBytes;
        bool isClosed : 1;
        bool isClientClosed : 1;
    } rx;
    struct {
        void* _Nullable bytes;
        size_t maxBytes;
        size_t numBytes;
        bool isClosed : 1;
    } tx;
} HAPPlatformTCPStream;
/**@endcond */

/**
 * TCP stream manager.
 */
struct HAPPlatformTCPStreamManager {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    HAPPlatformTCPStream* tcpStreams;
    size_t numTCPStreams;
    size_t numBufferBytes;

    HAPPlatformTCPStreamListenerCallback _Nullable callback;
    void* _Nullable context;
    HAPNetworkPort port;
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
     * Buffer to store TCP streams. Must remain valid.
     */
    HAPPlatformTCPStream* tcpStreams;

    /**
     * Number of TCP streams.
     */
    size_t numTCPStreams;

    /**
     * Buffer size. If 0, defaults to kHAPPlatformTCPStreamManager_NumBufferBytes.
     */
    size_t numBufferBytes;
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

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
