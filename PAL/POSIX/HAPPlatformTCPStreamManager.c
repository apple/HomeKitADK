// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "HAPPlatform+Init.h"
#include "HAPPlatformLog+Init.h"
#include "HAPPlatformTCPStreamManager+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "TCPStreamManager" };

/**
 * Sets all fields of a TCP stream listener to their initial values.
 *
 * @param      tcpStreamListener    TCP stream listener.
 */
static void InitializeTCPStreamListener(HAPPlatformTCPStreamListener* tcpStreamListener) {
    HAPPrecondition(tcpStreamListener);

    tcpStreamListener->tcpStreamManager = NULL;
    tcpStreamListener->interfaceIndex = 0;
    tcpStreamListener->port = 0;
    tcpStreamListener->fileDescriptor = -1;
    tcpStreamListener->fileHandle = 0;
    tcpStreamListener->callback = NULL;
    tcpStreamListener->context = NULL;
}

/**
 * Sets all fields of a TCP stream to their initial values.
 *
 * @param      tcpStream            TCP stream.
 */
static void InitializeTCPStream(HAPPlatformTCPStream* tcpStream) {
    HAPPrecondition(tcpStream);

    tcpStream->tcpStreamManager = NULL;
    tcpStream->fileDescriptor = -1;
    tcpStream->fileHandle = 0;
    tcpStream->interests.hasBytesAvailable = false;
    tcpStream->interests.hasSpaceAvailable = false;
    tcpStream->callback = NULL;
    tcpStream->context = NULL;
}

HAP_RESULT_USE_CHECK
HAPNetworkPort HAPPlatformTCPStreamManagerGetListenerPort(HAPPlatformTCPStreamManagerRef tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.tcpStreamManager);

    return tcpStreamManager->tcpStreamListener.port;
}

/**
 * Makes a file descriptor nonblocking.
 *
 * @param      fileDescriptor       File descriptor.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If the nonblocking flag could not be set.
 */
HAP_RESULT_USE_CHECK
static HAPError SetNonblocking(int fileDescriptor) {
    int e = fcntl(fileDescriptor, F_SETFL, O_NONBLOCK);
    if (e == -1) {
        HAPPlatformLogPOSIXError(
                kHAPLogType_Error,
                "System call 'fcntl' to set file descriptor flags to 'non-blocking' failed.",
                errno,
                __func__,
                HAP_FILE,
                __LINE__);
        return kHAPError_Unknown;
    }
    return kHAPError_None;
}

/**
 * Disables coalescing of small segments on a socket.
 *
 * @param      fileDescriptor       Socket file descriptor.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while disabling coalescing of small segments.
 */
HAP_RESULT_USE_CHECK
static HAPError SetNodelay(int fileDescriptor) {
    int v = 1;
    HAPLogBufferDebug(
            &logObject, &v, sizeof v, "setsockopt(%d, %d, %d, <buffer>);", fileDescriptor, IPPROTO_TCP, TCP_NODELAY);
    int e = setsockopt(fileDescriptor, IPPROTO_TCP, TCP_NODELAY, &v, sizeof v);
    if (e != 0) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Error,
                "System call 'setsockopt' to set socket options to 'no delay' failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
        return kHAPError_Unknown;
    }
    return kHAPError_None;
}

void HAPPlatformTCPStreamManagerCreate(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        const HAPPlatformTCPStreamManagerOptions* options) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(options);
    HAPPrecondition(options->maxConcurrentTCPStreams);

    HAPRawBufferZero(tcpStreamManager, sizeof *tcpStreamManager);

    if (options->interfaceName) {
        size_t numInterfaceNameBytes = HAPStringGetNumBytes(HAPNonnull(options->interfaceName));
        if ((numInterfaceNameBytes == 0) ||
            (numInterfaceNameBytes >= sizeof tcpStreamManager->tcpStreamListenerConfiguration.interfaceName)) {
            HAPLogError(&logObject, "Invalid local network interface name.");
            HAPFatalError();
        }
        HAPRawBufferCopyBytes(
                tcpStreamManager->tcpStreamListenerConfiguration.interfaceName,
                HAPNonnull(options->interfaceName),
                numInterfaceNameBytes);
    }
    tcpStreamManager->tcpStreamListenerConfiguration.port = options->port;

    tcpStreamManager->numTCPStreams = 0;
    tcpStreamManager->maxTCPStreams = options->maxConcurrentTCPStreams;

    HAPLogDebug(&logObject, "Storage configuration: tcpStreamManager = %lu", (unsigned long) sizeof *tcpStreamManager);
    HAPLogDebug(
            &logObject, "Storage configuration: maxTCPStreams = %lu", (unsigned long) tcpStreamManager->maxTCPStreams);
    HAPLogDebug(
            &logObject,
            "Storage configuration: tcpStreams = %lu",
            (unsigned long) tcpStreamManager->maxTCPStreams * sizeof(HAPPlatformTCPStream));

    InitializeTCPStreamListener(&tcpStreamManager->tcpStreamListener);

    tcpStreamManager->tcpStreams = malloc(tcpStreamManager->maxTCPStreams * sizeof(HAPPlatformTCPStream));
    if (!tcpStreamManager->tcpStreams) {
        HAPLogError(&logObject, "Allocating new TCP stream failed: out of memory.");
        HAPFatalError();
    }
    for (size_t i = 0; i < tcpStreamManager->maxTCPStreams; i++) {
        InitializeTCPStream(&tcpStreamManager->tcpStreams[i]);
    }

    // Initialize signal handling.
    void (*h)(int);
    h = signal(SIGPIPE, SIG_IGN);
    if (h == SIG_ERR) {
        HAPPlatformLogPOSIXError(
                kHAPLogType_Error,
                "System call 'signal' to ignore signals of type 'SIGPIPE' failed.",
                errno,
                __func__,
                HAP_FILE,
                __LINE__);
        HAPFatalError();
    }
}

void HAPPlatformTCPStreamManagerRelease(HAPPlatformTCPStreamManagerRef tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);

    HAPPlatformFreeSafe(tcpStreamManager->tcpStreams);
    tcpStreamManager->tcpStreams = NULL;
}

HAP_RESULT_USE_CHECK
bool HAPPlatformTCPStreamManagerIsListenerOpen(HAPPlatformTCPStreamManagerRef tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);

    return tcpStreamManager->tcpStreamListener.tcpStreamManager != NULL;
}

static void HandleTCPStreamListenerFileHandleCallback(
        HAPPlatformFileHandleRef fileHandle,
        HAPPlatformFileHandleEvent fileHandleEvents,
        void* _Nullable context);

void HAPPlatformTCPStreamManagerOpenListener(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamListenerCallback callback,
        void* _Nullable context) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(callback);

    HAPPrecondition(!tcpStreamManager->tcpStreamListener.tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.interfaceIndex == 0);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.port == 0);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.fileDescriptor == -1);
    HAPPrecondition(!tcpStreamManager->tcpStreamListener.fileHandle);
    HAPPrecondition(!tcpStreamManager->tcpStreamListener.callback);
    HAPPrecondition(!tcpStreamManager->tcpStreamListener.context);

    HAPError err;
    int _errno;
    int e;

    uint32_t interfaceIndex;
    if (tcpStreamManager->tcpStreamListenerConfiguration.interfaceName[0]) {
        unsigned int i = if_nametoindex(tcpStreamManager->tcpStreamListenerConfiguration.interfaceName);
        if ((i == 0) || (i > UINT32_MAX)) {
            HAPLogError(&logObject, "Mapping the local network interface name to its corresponding index failed.");
            HAPFatalError();
        }
        interfaceIndex = (uint32_t) i;
    } else {
        interfaceIndex = 0;
    }
    HAPNetworkPort port = tcpStreamManager->tcpStreamListenerConfiguration.port;

    int fileDescriptor = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (fileDescriptor == -1) {
        HAPLogError(&logObject, "Failed to open TCP stream listener socket.");
        HAPFatalError();
    }

    int v = 1;
    HAPLogBufferDebug(&logObject, &v, sizeof v, "setsockopt(%d, SOL_SOCKET, SO_REUSEADDR, <buffer>);", fileDescriptor);
    e = setsockopt(fileDescriptor, SOL_SOCKET, SO_REUSEADDR, &v, sizeof v);
    if (e != 0) {
        _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Error,
                "System call 'setsockopt' with option 'SO_REUSEADDR' on TCP stream listener socket failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
        HAPFatalError();
    }

    if (interfaceIndex) {
#if defined(SO_BINDTODEVICE)
        HAPLogBufferDebug(
                &logObject,
                tcpStreamManager->tcpStreamListenerConfiguration.interfaceName,
                sizeof tcpStreamManager->tcpStreamListenerConfiguration.interfaceName,
                "setsockopt(%d, SOL_SOCKET, SO_BINDTODEVICE, <buffer>);",
                fileDescriptor);
        e = setsockopt(
                fileDescriptor,
                SOL_SOCKET,
                SO_BINDTODEVICE,
                tcpStreamManager->tcpStreamListenerConfiguration.interfaceName,
                sizeof tcpStreamManager->tcpStreamListenerConfiguration.interfaceName);
        if (e != 0) {
            _errno = errno;
            HAPAssert(e == -1);
            HAPPlatformLogPOSIXError(
                    kHAPLogType_Error,
                    "System call 'setsockopt' with option 'SO_BINDTODEVICE' on TCP stream listener socket failed.",
                    _errno,
                    __func__,
                    HAP_FILE,
                    __LINE__);
            HAPFatalError();
        }
#else
        HAPLog(&logObject, "Ignoring local network interface name on which to bind the TCP stream manager.");
        interfaceIndex = 0;
#endif
    }
    HAPLogDebug(&logObject, "TCP stream listener interface index: %u", (unsigned int) interfaceIndex);

    struct sockaddr_in6 sin6;

    HAPRawBufferZero(&sin6, sizeof sin6);
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(port);
    sin6.sin6_addr = in6addr_any;

    HAPLogBufferDebug(&logObject, (struct sockaddr*) &sin6, sizeof sin6, "bind(%d, <buffer>);", fileDescriptor);
    e = bind(fileDescriptor, (struct sockaddr*) &sin6, sizeof sin6);
    if (e != 0) {
        _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Error,
                "System call 'bind' on TCP stream listener socket failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
        HAPFatalError();
    }

    if (!port) {
        socklen_t sin6_len = sizeof sin6;
        HAPRawBufferZero(&sin6, sizeof sin6);
        e = getsockname(fileDescriptor, (struct sockaddr*) &sin6, &sin6_len);
        if (e != 0) {
            _errno = errno;
            HAPAssert(e == -1);
            HAPPlatformLogPOSIXError(
                    kHAPLogType_Error,
                    "System call 'getsockname' on TCP stream listener socket failed.",
                    _errno,
                    __func__,
                    HAP_FILE,
                    __LINE__);
            HAPFatalError();
        }
        HAPAssert(sin6.sin6_port);
        port = ntohs(sin6.sin6_port);
    }
    HAPLogDebug(&logObject, "TCP stream listener port: %u.", port);

    HAPLogDebug(&logObject, "listen(%d, 64);", fileDescriptor);
    e = listen(fileDescriptor, 64);
    if (e != 0) {
        _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Error,
                "System call 'listen' on TCP stream listener socket failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
        HAPFatalError();
    }

    HAPPlatformFileHandleRef fileHandle;
    err = HAPPlatformFileHandleRegister(
            &fileHandle,
            fileDescriptor,
            (HAPPlatformFileHandleEvent) {
                    .isReadyForReading = true, .isReadyForWriting = false, .hasErrorConditionPending = false },
            HandleTCPStreamListenerFileHandleCallback,
            &tcpStreamManager->tcpStreamListener);
    if (err) {
        HAPLogError(&logObject, "Failed to register TCP stream listener file handle.");
        HAPFatalError();
    }
    HAPAssert(fileHandle);

    tcpStreamManager->tcpStreamListener.tcpStreamManager = tcpStreamManager;
    tcpStreamManager->tcpStreamListener.port = port;
    tcpStreamManager->tcpStreamListener.interfaceIndex = interfaceIndex;
    tcpStreamManager->tcpStreamListener.fileDescriptor = fileDescriptor;
    tcpStreamManager->tcpStreamListener.fileHandle = fileHandle;
    tcpStreamManager->tcpStreamListener.callback = callback;
    tcpStreamManager->tcpStreamListener.context = context;
}

void HAPPlatformTCPStreamManagerCloseListener(HAPPlatformTCPStreamManagerRef tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.tcpStreamManager == tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.fileDescriptor != -1);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.fileHandle);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.callback);

    int e;

    HAPPlatformFileHandleDeregister(tcpStreamManager->tcpStreamListener.fileHandle);

    HAPLogDebug(&logObject, "shutdown(%d, SHUT_RDWR);", tcpStreamManager->tcpStreamListener.fileDescriptor);
    e = shutdown(tcpStreamManager->tcpStreamListener.fileDescriptor, SHUT_RDWR);
    if (e != 0) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Debug,
                "System call 'shutdown' on TCP stream listener socket failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
    }

    HAPLogDebug(&logObject, "close(%d);", tcpStreamManager->tcpStreamListener.fileDescriptor);
    e = close(tcpStreamManager->tcpStreamListener.fileDescriptor);
    if (e != 0) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Debug,
                "System call 'close' on TCP stream listener socket failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
    }

    InitializeTCPStreamListener(&tcpStreamManager->tcpStreamListener);
}

static void HandleTCPStreamFileHandleCallback(
        HAPPlatformFileHandleRef fileHandle,
        HAPPlatformFileHandleEvent fileHandleEvents,
        void* _Nullable context);

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamManagerAcceptTCPStream(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef* tcpStream_) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.tcpStreamManager == tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.fileDescriptor != -1);
    HAPPrecondition(tcpStreamManager->tcpStreamListener.fileHandle);
    HAPPrecondition(tcpStream_);

    HAPError err;

    if (tcpStreamManager->numTCPStreams == tcpStreamManager->maxTCPStreams) {
        HAPLog(&logObject, "Cannot accept more TCP streams.");
        *tcpStream_ = (HAPPlatformTCPStreamRef) NULL;
        return kHAPError_OutOfResources;
    }

    HAPAssert(tcpStreamManager->numTCPStreams < tcpStreamManager->maxTCPStreams);

    // Find free TCP stream.
    size_t i = 0;
    while ((i < tcpStreamManager->maxTCPStreams) && (tcpStreamManager->tcpStreams[i].fileDescriptor != -1)) {
        i++;
    }
    HAPAssert(i < tcpStreamManager->maxTCPStreams);

    HAPPlatformTCPStream* tcpStream = &tcpStreamManager->tcpStreams[i];

    HAPAssert(!tcpStream->tcpStreamManager);
    HAPAssert(tcpStream->fileDescriptor == -1);
    HAPAssert(!tcpStream->fileHandle);

    HAPLogDebug(&logObject, "accept(%d, NULL, NULL);", tcpStreamManager->tcpStreamListener.fileDescriptor);
    int fileDescriptor = accept(tcpStreamManager->tcpStreamListener.fileDescriptor, NULL, NULL);
    if (fileDescriptor == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR && errno != ECONNABORTED && errno != EPROTO) {
            HAPPlatformLogPOSIXError(
                    kHAPLogType_Error,
                    "System call 'accept' on TCP stream listener socket failed.",
                    errno,
                    __func__,
                    HAP_FILE,
                    __LINE__);
            *tcpStream_ = (HAPPlatformTCPStreamRef) NULL;
            return kHAPError_Unknown;
        }

        HAPLogDebug(&logObject, "System call 'accept' on TCP stream listener socket is busy.");
        *tcpStream_ = (HAPPlatformTCPStreamRef) NULL;
        return kHAPError_Busy;
    }

    // Configure socket.
    int e = SetNonblocking(fileDescriptor);
    if (e != 0) {
        HAPLogError(&logObject, "Failed to configure TCP stream socket as non-blocking.");
        HAPFatalError();
    }
    e = SetNodelay(fileDescriptor);
    if (e != 0) {
        HAPLogError(&logObject, "Failed to disable Nagle's algorithm for TCP stream socket.");
        HAPFatalError();
    }

    HAPPlatformFileHandleRef fileHandle;
    err = HAPPlatformFileHandleRegister(
            &fileHandle,
            fileDescriptor,
            (HAPPlatformFileHandleEvent) {
                    .isReadyForReading = false, .isReadyForWriting = false, .hasErrorConditionPending = false },
            HandleTCPStreamFileHandleCallback,
            tcpStream);
    if (err) {
        HAPLogError(&logObject, "Failed to register TCP stream file handle.");
        HAPFatalError();
    }
    HAPAssert(fileHandle);

    tcpStream->tcpStreamManager = tcpStreamManager;
    tcpStream->fileDescriptor = fileDescriptor;
    tcpStream->fileHandle = fileHandle;
    HAPAssert(!tcpStream->interests.hasBytesAvailable);
    HAPAssert(!tcpStream->interests.hasSpaceAvailable);
    HAPAssert(!tcpStream->callback);
    HAPAssert(!tcpStream->context);

    *tcpStream_ = (HAPPlatformTCPStreamRef) tcpStream;

    tcpStreamManager->numTCPStreams++;

    if (tcpStreamManager->maxTCPStreams - tcpStreamManager->numTCPStreams == 0) {
        HAPLogInfo(&logObject, "Suspending accepting new TCP streams on TCP stream listener socket.");
        HAPPlatformFileHandleUpdateInterests(
                tcpStreamManager->tcpStreamListener.fileHandle,
                (HAPPlatformFileHandleEvent) {
                        .isReadyForReading = false, .isReadyForWriting = false, .hasErrorConditionPending = false },
                HandleTCPStreamListenerFileHandleCallback,
                &tcpStreamManager->tcpStreamListener);
    }

    return kHAPError_None;
}

void HAPPlatformTCPStreamCloseOutput(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(tcpStream_);

    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;

    HAPPrecondition(tcpStream->tcpStreamManager == tcpStreamManager);
    HAPPrecondition(tcpStream->fileDescriptor != -1);
    HAPPrecondition(tcpStream->fileHandle);

    HAPLogDebug(&logObject, "shutdown(%d, SHUT_WR);", tcpStream->fileDescriptor);
    int e = shutdown(tcpStream->fileDescriptor, SHUT_WR);
    if (e != 0) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Error,
                "System call 'shutdown' on TCP stream listener socket failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
    }
}

void HAPPlatformTCPStreamClose(HAPPlatformTCPStreamManagerRef tcpStreamManager, HAPPlatformTCPStreamRef tcpStream_) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(tcpStream_);

    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;

    HAPPrecondition(tcpStream->tcpStreamManager == tcpStreamManager);
    HAPPrecondition(tcpStream->fileDescriptor != -1);
    HAPPrecondition(tcpStream->fileHandle);

    int e;

    HAPPlatformFileHandleDeregister(tcpStream->fileHandle);

    HAPLogDebug(&logObject, "shutdown(%d, SHUT_RDWR);", tcpStream->fileDescriptor);
    e = shutdown(tcpStream->fileDescriptor, SHUT_RDWR);
    if (e != 0) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Debug,
                "System call 'shutdown' on TCP stream socket failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
    }

    HAPLogDebug(&logObject, "close(%d);", tcpStream->fileDescriptor);
    e = close(tcpStream->fileDescriptor);
    if (e != 0) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPPlatformLogPOSIXError(
                kHAPLogType_Debug,
                "System call 'close' on TCP stream socket failed.",
                _errno,
                __func__,
                HAP_FILE,
                __LINE__);
    }

    InitializeTCPStream(tcpStream);

    HAPAssert(tcpStreamManager->numTCPStreams <= tcpStreamManager->maxTCPStreams);

    HAPAssert(tcpStreamManager->numTCPStreams > 0);

    tcpStreamManager->numTCPStreams--;

    if (tcpStreamManager->tcpStreamListener.fileDescriptor != -1) {
        HAPAssert(tcpStreamManager->tcpStreamListener.tcpStreamManager == tcpStreamManager);
        HAPAssert(tcpStreamManager->tcpStreamListener.fileHandle);
        if (tcpStreamManager->maxTCPStreams - tcpStreamManager->numTCPStreams == 1) {
            HAPLogInfo(&logObject, "Resuming accepting new TCP streams on TCP stream listener socket.");
            HAPPlatformFileHandleUpdateInterests(
                    tcpStreamManager->tcpStreamListener.fileHandle,
                    (HAPPlatformFileHandleEvent) {
                            .isReadyForReading = true, .isReadyForWriting = false, .hasErrorConditionPending = false },
                    HandleTCPStreamListenerFileHandleCallback,
                    &tcpStreamManager->tcpStreamListener);
        }
    } else {
        HAPAssert(!tcpStreamManager->tcpStreamListener.tcpStreamManager);
        HAPAssert(!tcpStreamManager->tcpStreamListener.fileHandle);
    }
}

void HAPPlatformTCPStreamUpdateInterests(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_,
        HAPPlatformTCPStreamEvent interests,
        HAPPlatformTCPStreamEventCallback _Nullable callback,
        void* _Nullable context) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(tcpStream_);
    HAPPrecondition(!(interests.hasBytesAvailable || interests.hasSpaceAvailable) || callback != NULL);

    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;

    HAPPrecondition(tcpStream->tcpStreamManager == tcpStreamManager);
    HAPPrecondition(tcpStream->fileDescriptor != -1);
    HAPPrecondition(tcpStream->fileHandle);

    tcpStream->interests.hasBytesAvailable = interests.hasBytesAvailable;
    tcpStream->interests.hasSpaceAvailable = interests.hasSpaceAvailable;
    tcpStream->callback = callback;
    tcpStream->context = context;

    HAPPlatformFileHandleUpdateInterests(
            tcpStream->fileHandle,
            (HAPPlatformFileHandleEvent) { .isReadyForReading = tcpStream->interests.hasBytesAvailable,
                                           .isReadyForWriting = tcpStream->interests.hasSpaceAvailable,
                                           .hasErrorConditionPending = false },
            HandleTCPStreamFileHandleCallback,
            tcpStream);
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamRead(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_,
        void* bytes,
        size_t maxBytes,
        size_t* numBytes) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(tcpStream_);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;

    HAPPrecondition(tcpStream->tcpStreamManager == tcpStreamManager);
    HAPPrecondition(tcpStream->fileDescriptor != -1);
    HAPPrecondition(tcpStream->fileHandle);

    ssize_t n;
    do {
        n = recv(tcpStream->fileDescriptor, bytes, maxBytes, 0);
    } while ((n == -1) && (errno == EINTR));
    if (n == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            HAPPlatformLogPOSIXError(
                    kHAPLogType_Default,
                    "System call 'recv' on TCP stream socket failed.",
                    errno,
                    __func__,
                    HAP_FILE,
                    __LINE__);
            *numBytes = 0;
            return kHAPError_Unknown;
        }

        HAPLogDebug(&logObject, "System call 'recv' on TCP stream socket is busy.");
        *numBytes = 0;
        return kHAPError_Busy;
    }

    HAPAssert(n >= 0);
    HAPAssert((size_t) n <= maxBytes);
    *numBytes = (size_t) n;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamWrite(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_,
        const void* bytes,
        size_t maxBytes,
        size_t* numBytes) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStreamManager->tcpStreams);
    HAPPrecondition(tcpStream_);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;

    HAPPrecondition(tcpStream->tcpStreamManager == tcpStreamManager);
    HAPPrecondition(tcpStream->fileDescriptor != -1);
    HAPPrecondition(tcpStream->fileHandle);

    ssize_t n;
    do {
        n = send(tcpStream->fileDescriptor, bytes, maxBytes, 0);
    } while ((n == -1) && (errno == EINTR));
    if (n == -1) {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
            HAPPlatformLogPOSIXError(
                    kHAPLogType_Default,
                    "System call 'send' on TCP stream socket failed.",
                    errno,
                    __func__,
                    HAP_FILE,
                    __LINE__);
            *numBytes = 0;
            return kHAPError_Unknown;
        }

        HAPLogDebug(&logObject, "System call 'send' on TCP stream socket is busy.");
        *numBytes = 0;
        return kHAPError_Busy;
    }

    HAPAssert(n >= 0);
    HAPAssert((size_t) n <= maxBytes);
    *numBytes = (size_t) n;
    return kHAPError_None;
}

static void HandleTCPStreamListenerFileHandleCallback(
        HAPPlatformFileHandleRef fileHandle,
        HAPPlatformFileHandleEvent fileHandleEvents,
        void* _Nullable context) {
    HAPAssert(fileHandle);
    HAPAssert(context);

    HAPPlatformTCPStreamListener* listener = (HAPPlatformTCPStreamListener*) context;

    HAPAssert(listener->tcpStreamManager);
    HAPAssert(listener->fileDescriptor != -1);
    HAPAssert(listener->fileHandle == fileHandle);
    HAPAssert(listener->callback);

    HAPAssert(fileHandleEvents.isReadyForReading);

    listener->callback(listener->tcpStreamManager, listener->context);
}

static void HandleTCPStreamFileHandleCallback(
        HAPPlatformFileHandleRef fileHandle,
        HAPPlatformFileHandleEvent fileHandleEvents,
        void* _Nullable context) {
    HAPAssert(fileHandle);
    HAPAssert(context);

    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) context;

    HAPAssert(tcpStream->tcpStreamManager);
    HAPAssert(tcpStream->fileDescriptor != -1);
    HAPAssert(tcpStream->fileHandle == fileHandle);

    HAPAssert(fileHandleEvents.isReadyForReading || fileHandleEvents.isReadyForWriting);

    HAPPlatformTCPStreamEvent tcpStreamEvents;
    tcpStreamEvents.hasBytesAvailable = tcpStream->interests.hasBytesAvailable && fileHandleEvents.isReadyForReading;
    tcpStreamEvents.hasSpaceAvailable = tcpStream->interests.hasSpaceAvailable && fileHandleEvents.isReadyForWriting;

    if (tcpStreamEvents.hasBytesAvailable || tcpStreamEvents.hasSpaceAvailable) {
        HAPAssert(tcpStream->callback);
        HAPPlatformTCPStreamRef tcpStream_ = (HAPPlatformTCPStreamRef) tcpStream;
        tcpStream->callback(tcpStream->tcpStreamManager, tcpStream_, tcpStreamEvents, tcpStream->context);
    }
}
