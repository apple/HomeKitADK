// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <stdlib.h>

#include "HAPPlatformTCPStreamManager+Init.h"
#include "HAPPlatformTCPStreamManager+Test.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "TCPStreamManager" };

static HAPNetworkPort _port = 1024;

void HAPPlatformTCPStreamManagerCreate(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        const HAPPlatformTCPStreamManagerOptions* options) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(options);
    HAPPrecondition(options->tcpStreams);

    HAPRawBufferZero(options->tcpStreams, options->numTCPStreams * sizeof *options->tcpStreams);

    HAPRawBufferZero(tcpStreamManager, sizeof *tcpStreamManager);
    tcpStreamManager->tcpStreams = options->tcpStreams;
    tcpStreamManager->numTCPStreams = options->numTCPStreams;
    tcpStreamManager->numBufferBytes =
            options->numBufferBytes ? options->numBufferBytes : kHAPPlatformTCPStreamManager_NumBufferBytes;

    tcpStreamManager->port = _port++;

    for (size_t i = 0; i < tcpStreamManager->numTCPStreams; i++) {
        HAPPlatformTCPStream* tcpStream = &tcpStreamManager->tcpStreams[i];
        tcpStream->tcpStreamManager = tcpStreamManager;
    }
}

HAP_RESULT_USE_CHECK
HAPNetworkPort HAPPlatformTCPStreamManagerGetListenerPort(HAPPlatformTCPStreamManagerRef tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);

    return tcpStreamManager->port;
}

HAP_RESULT_USE_CHECK
bool HAPPlatformTCPStreamManagerIsListenerOpen(HAPPlatformTCPStreamManagerRef tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);

    return tcpStreamManager->callback != NULL;
}

void HAPPlatformTCPStreamManagerOpenListener(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamListenerCallback callback,
        void* _Nullable context) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(callback);

    HAPLogInfo(&logObject, "%s(%u).", __func__, tcpStreamManager->port);
    HAPPrecondition(!tcpStreamManager->callback);
    HAPPrecondition(!tcpStreamManager->context);
    tcpStreamManager->callback = callback;
    tcpStreamManager->context = context;
}

void HAPPlatformTCPStreamManagerCloseListener(HAPPlatformTCPStreamManagerRef tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);

    HAPLogInfo(&logObject, "%s(%u).", __func__, tcpStreamManager->port);
    tcpStreamManager->callback = NULL;
    tcpStreamManager->context = NULL;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamManagerConnectToListener(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef* tcpStream) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(HAPPlatformTCPStreamManagerIsListenerOpen(tcpStreamManager));
    HAPPrecondition(tcpStream);

    for (size_t i = 0; i < tcpStreamManager->numTCPStreams; i++) {
        HAPPlatformTCPStream* stream = &tcpStreamManager->tcpStreams[i];
        if (stream->isActive) {
            continue;
        }

        // Open connection.
        HAPLogInfo(&logObject, "Opened connection: %p.", (const void*) stream);
        stream->isActive = true;
        stream->rx.maxBytes = tcpStreamManager->numBufferBytes;
        stream->rx.bytes = calloc(1, stream->rx.maxBytes);
        HAPAssert(stream->rx.bytes);
        stream->tx.maxBytes = tcpStreamManager->numBufferBytes;
        stream->tx.bytes = calloc(1, stream->tx.maxBytes);
        HAPAssert(stream->tx.bytes);
        *tcpStream = (HAPPlatformTCPStreamRef) stream;

        // Inform delegate.
        HAPPrecondition(tcpStreamManager->callback);
        tcpStreamManager->callback(tcpStreamManager, tcpStreamManager->context);

        return kHAPError_None;
    }

    HAPLogError(&logObject, "TCP stream manager cannot accept more connections.");
    return kHAPError_OutOfResources;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamManagerAcceptTCPStream(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef* tcpStream) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream);

    for (size_t i = 0; i < tcpStreamManager->numTCPStreams; i++) {
        HAPPlatformTCPStream* stream = &tcpStreamManager->tcpStreams[i];
        if (!stream->isActive) {
            continue;
        }
        if (stream->isConnected) {
            continue;
        }

        // Open connection.
        HAPLogInfo(&logObject, "Accepted connection: %p.", (const void*) stream);
        stream->isConnected = true;
        *tcpStream = (HAPPlatformTCPStreamRef) stream;

        return kHAPError_None;
    }

    HAPLog(&logObject, "No acceptable connections found.");
    return kHAPError_Unknown;
}

static void Invalidate(HAPPlatformTCPStreamManagerRef tcpStreamManager, HAPPlatformTCPStreamRef tcpStream_) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;

    if (tcpStream->invokeCallbackTimer) {
        HAPPlatformTimerDeregister(tcpStream->invokeCallbackTimer);
    }
    free(tcpStream->rx.bytes);
    free(tcpStream->tx.bytes);
    HAPRawBufferZero(tcpStream, sizeof *tcpStream);
}

void HAPPlatformTCPStreamCloseOutput(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream HAP_UNUSED) {
    HAPPrecondition(tcpStreamManager);

    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}

void HAPPlatformTCPStreamClose(HAPPlatformTCPStreamManagerRef tcpStreamManager, HAPPlatformTCPStreamRef tcpStream_) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;
    HAPAssert(tcpStream->isActive);

    HAPAssert(!tcpStream->rx.isClosed);
    tcpStream->rx.isClosed = true;
    tcpStream->tx.isClosed = true;

    // Release resources when both sides closed.
    if (tcpStream->rx.isClosed && tcpStream->rx.isClientClosed) {
        HAPLogDebug(&logObject, "[%p] Closing.", (const void*) tcpStream);
        HAPAssert(tcpStream->tx.isClosed);
        Invalidate(tcpStreamManager, tcpStream_);
    }
}

static void InvokeCallback(HAPPlatformTCPStreamManagerRef tcpStreamManager, HAPPlatformTCPStreamRef tcpStream_) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;
    HAPAssert(tcpStream->isActive);

    if (!tcpStream->callback) {
        return;
    }

    HAPPlatformTCPStreamEvent event = { .hasBytesAvailable = tcpStream->rx.numBytes || tcpStream->rx.isClientClosed,
                                        .hasSpaceAvailable = tcpStream->tx.numBytes < tcpStream->tx.maxBytes };

    if (tcpStream->rx.isClosed || !tcpStream->interests.hasBytesAvailable) {
        event.hasBytesAvailable = false;
    }
    if (tcpStream->tx.isClosed || !tcpStream->interests.hasSpaceAvailable) {
        event.hasSpaceAvailable = false;
    }
    if (event.hasBytesAvailable || event.hasSpaceAvailable) {
        tcpStream->callback(tcpStreamManager, tcpStream_, event, tcpStream->context);
    }
}

static void InvokeCallbackExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPPlatformTCPStream* tcpStream = context;
    HAPPlatformTCPStreamRef tcpStream_ = (HAPPlatformTCPStreamRef) tcpStream;
    HAPPrecondition(timer == tcpStream->invokeCallbackTimer);
    tcpStream->invokeCallbackTimer = 0;

    HAPPlatformTCPStreamManagerRef tcpStreamManager = tcpStream->tcpStreamManager;
    InvokeCallback(tcpStreamManager, tcpStream_);
}

void HAPPlatformTCPStreamManagerClientClose(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;
    HAPAssert(tcpStream->isActive);

    HAPAssert(!tcpStream->rx.isClientClosed);
    tcpStream->rx.isClientClosed = true;
    InvokeCallback(tcpStreamManager, tcpStream_);

    // Release resources when both sides closed.
    if (tcpStream->rx.isClosed && tcpStream->rx.isClientClosed) {
        HAPLogDebug(&logObject, "[%p] Closing (Client closed).", (const void*) tcpStream);
        HAPAssert(tcpStream->tx.isClosed);
        Invalidate(tcpStreamManager, tcpStream_);
    }
}

void HAPPlatformTCPStreamUpdateInterests(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_,
        HAPPlatformTCPStreamEvent interests,
        HAPPlatformTCPStreamEventCallback _Nullable callback,
        void* _Nullable context) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;
    HAPPrecondition(tcpStream->isActive);
    HAPPrecondition(tcpStream->isConnected);
    HAPPrecondition(!(interests.hasBytesAvailable || interests.hasSpaceAvailable) || callback != NULL);

    HAPError err;

    tcpStream->interests = interests;
    tcpStream->callback = callback;
    tcpStream->context = context;

    if (!tcpStream->invokeCallbackTimer) {
        err = HAPPlatformTimerRegister(&tcpStream->invokeCallbackTimer, 0, InvokeCallbackExpired, tcpStream);
        HAPAssert(!err);
    }
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamRead(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_,
        void* bytes,
        size_t maxBytes,
        size_t* numBytes) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;
    HAPAssert(tcpStream->isActive);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    *numBytes = HAPMin(tcpStream->rx.numBytes, maxBytes);
    HAPRawBufferCopyBytes(bytes, HAPNonnullVoid(tcpStream->rx.bytes), *numBytes);
    HAPRawBufferCopyBytes(
            HAPNonnullVoid(tcpStream->rx.bytes),
            &((uint8_t*) tcpStream->rx.bytes)[*numBytes],
            tcpStream->rx.numBytes - *numBytes);
    tcpStream->rx.numBytes -= *numBytes;

    if (!*numBytes && !tcpStream->rx.isClosed) {
        return kHAPError_Busy;
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamClientRead(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_,
        void* bytes,
        size_t maxBytes,
        size_t* numBytes) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;
    HAPAssert(tcpStream->isActive);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    *numBytes = 0;
    size_t n;
    InvokeCallback(tcpStreamManager, tcpStream_);
    do {
        n = HAPMin(tcpStream->tx.numBytes, maxBytes - *numBytes);
        HAPRawBufferCopyBytes(&((uint8_t*) bytes)[*numBytes], HAPNonnullVoid(tcpStream->tx.bytes), n);
        HAPRawBufferCopyBytes(
                HAPNonnullVoid(tcpStream->tx.bytes), &((uint8_t*) tcpStream->tx.bytes)[n], tcpStream->tx.numBytes - n);
        tcpStream->tx.numBytes -= n;
        *numBytes += n;
        InvokeCallback(tcpStreamManager, tcpStream_);
    } while (*numBytes < maxBytes && n && !tcpStream->tx.isClosed);
    HAPAssert(*numBytes <= maxBytes);

    if (!*numBytes && !tcpStream->tx.isClosed) {
        return kHAPError_Busy;
    }
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
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;
    HAPAssert(tcpStream->isActive);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    if (tcpStream->tx.isClosed) {
        return kHAPError_Unknown;
    }

    *numBytes = HAPMin(tcpStream->tx.maxBytes - tcpStream->tx.numBytes, maxBytes);
    HAPRawBufferCopyBytes(&((uint8_t*) tcpStream->tx.bytes)[tcpStream->tx.numBytes], bytes, *numBytes);
    tcpStream->tx.numBytes += *numBytes;

    if (!*numBytes) {
        return kHAPError_Busy;
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamClientWrite(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream_,
        const void* bytes,
        size_t maxBytes,
        size_t* numBytes) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream_);
    HAPPlatformTCPStream* tcpStream = (HAPPlatformTCPStream*) tcpStream_;
    HAPAssert(tcpStream->isActive);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    *numBytes = 0;
    size_t n;
    InvokeCallback(tcpStreamManager, tcpStream_);
    do {
        n = HAPMin(tcpStream->rx.maxBytes - tcpStream->rx.numBytes, maxBytes - *numBytes);
        HAPRawBufferCopyBytes(
                &((uint8_t*) tcpStream->rx.bytes)[tcpStream->rx.numBytes], &((const uint8_t*) bytes)[*numBytes], n);
        tcpStream->rx.numBytes += n;
        *numBytes += n;
        InvokeCallback(tcpStreamManager, tcpStream_);
    } while (*numBytes < maxBytes && n && !tcpStream->rx.isClosed);
    HAPAssert(*numBytes <= maxBytes);

    if (!*numBytes && !tcpStream->rx.isClosed) {
        return kHAPError_Busy;
    }
    return kHAPError_None;
}
