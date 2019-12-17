// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_TCP_STREAM_MANAGER_H
#define HAP_PLATFORM_TCP_STREAM_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * TCP stream manager.
 */
typedef struct HAPPlatformTCPStreamManager HAPPlatformTCPStreamManager;
typedef struct HAPPlatformTCPStreamManager* HAPPlatformTCPStreamManagerRef;
HAP_NONNULL_SUPPORT(HAPPlatformTCPStreamManager)

/**
 * Returns the network port associated with the TCP stream listener of the given TCP stream manager instance.
 *
 * @param      tcpStreamManager     TCP stream manager.
 *
 * @return Port number associated with the TCP stream listener.
 */
HAP_RESULT_USE_CHECK
HAPNetworkPort HAPPlatformTCPStreamManagerGetListenerPort(HAPPlatformTCPStreamManagerRef tcpStreamManager);

/**
 * Returns whether the TCP stream manager is listening for client connections.
 *
 * @param      tcpStreamManager     TCP stream manager.
 *
 * @return true                     If the TCP stream manager is listening for client connections.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPPlatformTCPStreamManagerIsListenerOpen(HAPPlatformTCPStreamManagerRef tcpStreamManager);

/**
 * Callback that is invoked when a client connection is ready for being accepted.
 *
 * @param      tcpStreamManager     TCP stream manager.
 * @param      context              The context parameter given to the
 *                                  HAPPlatformTCPStreamManagerOpenListener function.
 */
typedef void (*HAPPlatformTCPStreamListenerCallback)(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        void* _Nullable context);

/**
 * Starts listening for client connections.
 *
 * - The callback is never invoked synchronously.
 *
 * @param      tcpStreamManager     TCP stream manager that is not listening for client connections.
 * @param      callback             Callback to call when a client connection is ready for being accepted.
 * @param      context              Client context pointer. Will be passed to the callback.
 */
void HAPPlatformTCPStreamManagerOpenListener(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamListenerCallback callback,
        void* _Nullable context);

/**
 * Stops listening for client connections.
 *
 * @param      tcpStreamManager     TCP stream manager.
 */
void HAPPlatformTCPStreamManagerCloseListener(HAPPlatformTCPStreamManagerRef tcpStreamManager);

/**
 * TCP stream handle.
 */
typedef uintptr_t HAPPlatformTCPStreamRef;

/**
 * Accepts a client connection from a listening TCP stream manager and opens a TCP stream.
 *
 * @param      tcpStreamManager     Listening TCP stream manager.
 * @param[out] tcpStream            Accepted client connection.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while accepting the client connection.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamManagerAcceptTCPStream(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef* tcpStream);

/**
 * Closes a TCP stream for writing. No further writes are possible.
 *
 * - Data may still be read from the TCP stream.
 *
 * @param      tcpStreamManager     TCP stream manager from which the stream was accepted.
 * @param      tcpStream            TCP stream.
 */
void HAPPlatformTCPStreamCloseOutput(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream);

/**
 * Closes a TCP stream for reading and writing.
 *
 * - The stream handle will become invalid and must no longer be used.
 *
 * @param      tcpStreamManager     TCP stream manager from which the stream was accepted.
 * @param      tcpStream            TCP stream.
 */
void HAPPlatformTCPStreamClose(HAPPlatformTCPStreamManagerRef tcpStreamManager, HAPPlatformTCPStreamRef tcpStream);

/**
 * Events that may be sent to a TCP stream callback.
 */
typedef struct {
    /**
     * The stream has bytes to be read.
     */
    bool hasBytesAvailable : 1;

    /**
     * The stream can accept bytes for writing.
     */
    bool hasSpaceAvailable : 1;
} HAPPlatformTCPStreamEvent;

/**
 * Callback that is invoked when an event occurs on a TCP stream.
 *
 * @param      tcpStreamManager     TCP stream manager from which the stream was accepted.
 * @param      tcpStream            TCP stream.
 * @param      event                The stream event that occurred.
 * @param      context              The context parameter given to the HAPPlatformTCPStreamUpdateInterests function.
 */
typedef void (*HAPPlatformTCPStreamEventCallback)(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream,
        HAPPlatformTCPStreamEvent event,
        void* _Nullable context);

/**
 * Registers a callback to be invoked when an event occurs on a TCP stream.
 *
 *  - The callback is never invoked synchronously.
 *
 * @param      tcpStreamManager     TCP stream manager from which the stream was accepted.
 * @param      tcpStream            TCP stream.
 * @param      interests            Collection of events for which the callback shall be invoked.
 * @param      callback             Callback to be invoked when a stream event for which interest was registered occurs.
 * @param      context              Client context pointer. Will be passed to the callback.
 */
void HAPPlatformTCPStreamUpdateInterests(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream,
        HAPPlatformTCPStreamEvent interests,
        HAPPlatformTCPStreamEventCallback _Nullable callback,
        void* _Nullable context);

/**
 * Reads from a TCP stream.
 *
 * @param      tcpStreamManager     TCP stream manager from which the stream was accepted.
 * @param      tcpStream            TCP stream.
 * @param[out] bytes                Buffer containing received data.
 * @param      maxBytes             Capacity of buffer.
 * @param[out] numBytes             Number of bytes put into buffer.
 *
 * @return kHAPError_None           If successful.
                                    If numBytes is 0, the peer has closed its side of the connection for writing.
 * @return kHAPError_Unknown        If a non-recoverable error occurred while reading from the TCP stream.
 * @return kHAPError_Busy           If no data is available for reading at the time. Retry later.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamRead(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream,
        void* bytes,
        size_t maxBytes,
        size_t* numBytes);

/**
 * Writes to a TCP stream.
 *
 * - Partial writes may occur.
 *
 * @param      tcpStreamManager     TCP stream manager from which the stream was accepted.
 * @param      tcpStream            TCP stream.
 * @param      bytes                Buffer containing data to send.
 * @param      maxBytes             Length of buffer.
 * @param[out] numBytes             Number of bytes that have been written.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If a non-recoverable error occurred while writing to the TCP stream.
 * @return kHAPError_Busy           If no space is available for writing at the time. Retry later.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamWrite(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream,
        const void* bytes,
        size_t maxBytes,
        size_t* numBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
