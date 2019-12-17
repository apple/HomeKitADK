// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_IP_INIT_H
#define HAP_PLATFORM_IP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <net/if.h>

#include "HAPPlatform.h"
#include "HAPPlatformFileHandle.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**@file
 * TCP stream manager implementation for POSIX.
 *
 * The following limitations apply if this code is not modified:
 * - Non-null values for the option interfaceName are ignored on platforms without support for the socket option
 *   SO_BINDTODEVICE which binds the socket to a particular network interface.
 *
 * **Example**

   @code{.c}
   // Allocate TCP stream manager object.
   static HAPPlatformTCPStreamManager tcpStreamManager;

   // Initialize TCP stream manager object.
   HAPPlatformTCPStreamManagerCreate(&platform.tcpStreamManager,
       &(const HAPPlatformTCPStreamManagerOptions) {
           // Listen on all available network interfaces.
           .interfaceName = NULL,

           // Listen on an unused port number from the ephemeral port range.
           .port = kHAPNetworkPort_Any,

           // Allocate enough concurrent TCP streams to support the IP accessory.
           .maxConcurrentTCPStreams = kHAPIPSessionStorage_DefaultNumElements
   });

   @endcode
 */

/**
 * TCP stream manager initialization options.
 */
typedef struct {
    /**
     * Local network interface name on which to bind the TCP stream manager.
     *
     * - A value of NULL will use all available network interfaces.
     */
    const char* _Nullable interfaceName;

    /**
     * Local port number on which to bind the TCP stream manager.
     *
     * - A value of kHAPNetworkPort_Any will use an unused port number from the ephemeral port range.
     */
    HAPNetworkPort port;

    /**
     * Maximum number of concurrent TCP streams.
     */
    size_t maxConcurrentTCPStreams;
} HAPPlatformTCPStreamManagerOptions;

// Opaque type. Do not use directly.
/**@cond */
typedef struct {
    HAPPlatformTCPStreamManagerRef tcpStreamManager;

    uint32_t interfaceIndex;
    HAPNetworkPort port;

    int fileDescriptor;
    HAPPlatformFileHandleRef fileHandle;
    HAPPlatformTCPStreamListenerCallback _Nullable callback;
    void* _Nullable context;
} HAPPlatformTCPStreamListener;
/**@endcond */

// Opaque type. Do not use directly.
/**@cond */
typedef struct {
    HAPPlatformTCPStreamManagerRef tcpStreamManager;

    int fileDescriptor;
    HAPPlatformFileHandleRef fileHandle;
    HAPPlatformTCPStreamEvent interests;
    HAPPlatformTCPStreamEventCallback _Nullable callback;
    void* _Nullable context;
} HAPPlatformTCPStream;
/**@endcond */

/**
 * TCP stream manager.
 */
struct HAPPlatformTCPStreamManager {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    size_t numTCPStreams;
    size_t maxTCPStreams;

    struct {
        char interfaceName[IFNAMSIZ];
        HAPNetworkPort port;
    } tcpStreamListenerConfiguration;

    HAPPlatformTCPStreamListener tcpStreamListener;
    HAPPlatformTCPStream* _Nullable tcpStreams;
    /**@endcond */
};

/**
 * Initializes TCP stream manager.
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
