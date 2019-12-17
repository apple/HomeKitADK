// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_TCP_STREAM_MANAGER_TEST_H
#define HAP_PLATFORM_TCP_STREAM_MANAGER_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Creates a connection to the TCP stream manager's listener.
 *
 * - The TCP stream must be closed using HAPPlatformTCPStreamManagerClientClose after it is no longer used.
 *
 * @param      tcpStreamManager     TCP stream manager.
 * @param[out] tcpStream            TCP stream that has been put into the TCP stream manager's accept queue.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOFResources If not enough resources are available to create the connection.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamManagerConnectToListener(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef* tcpStream);

/**
 * Closes a TCP stream for reading and writing.
 *
 * - /!\ WARNING: The TCP stream must no longer be used by the client after this function returns.
 *
 * @param      tcpStreamManager     TCP stream manager.
 * @param      tcpStream            TCP stream that has been put into the TCP stream manager's accept queue.
 */
void HAPPlatformTCPStreamManagerClientClose(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream);

/**
 * Reads from a TCP stream as client.
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
HAPError HAPPlatformTCPStreamClientRead(
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
HAPError HAPPlatformTCPStreamClientWrite(
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
