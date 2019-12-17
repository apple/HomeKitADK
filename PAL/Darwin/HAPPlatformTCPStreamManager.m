// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#import <Foundation/Foundation.h>
#import <Network/Network.h>

#include "HAPPlatformTCPStreamManager+Init.h"

static const HAPLogObject tcp_log = { .subsystem = kHAPPlatform_LogSubsystem, .category = "TCPStreamManager" };

@interface Connection : NSObject
@property (nonatomic, retain) nw_connection_t socket;
@property nw_connection_state_t state;
@property HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager;
@property HAPPlatformTCPStreamEvent interests;
@property HAPPlatformTCPStreamEventCallback _Nullable callback;
@property void* _Nullable context;
@property (nonatomic, retain) NSData* buffer;
@end

@implementation Connection
- (instancetype)initWithSocket:(nw_connection_t)socket manager:(HAPPlatformTCPStreamManagerRef)manager {
    if (self = [super init]) {
        _socket = socket;
        _state = nw_connection_state_invalid;
        _tcpStreamManager = manager;
        _interests = (HAPPlatformTCPStreamEvent) {
            .hasBytesAvailable = false,
            .hasSpaceAvailable = false,
        };
        _callback = NULL;
        _context = NULL;
        _buffer = [[NSMutableData alloc] init];
    }
    return self;
}
@end

static nw_listener_t listener = nil;
static NSMutableArray<Connection*>* socketsWaitingToBeAccepted = nil;
static NSMutableArray<Connection*>* connections = nil;

void HAPPlatformTCPStreamManagerCreate(
        HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager,
        const HAPPlatformTCPStreamManagerOptions* options) {
    socketsWaitingToBeAccepted = [[NSMutableArray alloc] init];
    connections = [[NSMutableArray alloc] init];
}

void HAPPlatformTCPStreamManagerRelease(HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager) {
    if (listener) {
        HAPPlatformTCPStreamManagerCloseListener(tcpStreamManager);
    }
    socketsWaitingToBeAccepted = nil;
    connections = nil;
}

HAP_RESULT_USE_CHECK
uint16_t HAPPlatformTCPStreamManagerGetListenerPort(HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(listener);

    return nw_listener_get_port(listener);
}

HAP_RESULT_USE_CHECK
bool HAPPlatformTCPStreamManagerIsListenerOpen(HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);
    return listener != nil;
}

void HAPPlatformTCPStreamManagerOpenListener(
        HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager,
        HAPPlatformTCPStreamListenerCallback _Nonnull callback,
        void* _Nullable context) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(callback);
    HAPPrecondition(!listener);
    HAPPrecondition(socketsWaitingToBeAccepted);

    nw_endpoint_t host = nw_endpoint_create_host("::", "0");
    nw_parameters_t tcp =
            nw_parameters_create_secure_tcp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION);
    nw_parameters_set_local_endpoint(tcp, host);
    listener = nw_listener_create(tcp);
    HAPAssert(listener);
    nw_listener_set_queue(listener, dispatch_get_main_queue());
    nw_listener_set_new_connection_handler(listener, ^(nw_connection_t socket) {
        Connection* connection = [[Connection alloc] initWithSocket:socket manager:tcpStreamManager];
        [socketsWaitingToBeAccepted addObject:connection];
        callback(tcpStreamManager, context);
    });
    nw_listener_start(listener);
}

void HAPPlatformTCPStreamManagerCloseListener(HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(listener);

    nw_listener_cancel(listener);
    listener = nil;
}

static void EventCallback(Connection* connection, bool incoming) {
    if (![connections containsObject:connection]) {
        return;
    }

    if ((connection.interests.hasSpaceAvailable && !incoming) || (connection.interests.hasBytesAvailable && incoming)) {
        connection.callback(
                connection.tcpStreamManager,
                (HAPPlatformTCPStreamRef)(__bridge void*) connection,
                (HAPPlatformTCPStreamEvent) {
                        .hasBytesAvailable = incoming,
                        .hasSpaceAvailable = !incoming,
                },
                connection.context);
    }
}

static void WaitForMoreDataInBackground(Connection* connection) {
    if (![connections containsObject:connection]) {
        return;
    }
    nw_connection_receive(
            connection.socket,
            1,
            4096,
            ^(dispatch_data_t data, nw_content_context_t context, bool is_complete, nw_error_t error) {
                if (error) {
                    return;
                }
                NSMutableData* newdata = [[NSMutableData alloc] init];
                [newdata appendData:connection.buffer];
                [newdata appendData:(NSData*) data];
                connection.buffer = newdata;
                EventCallback(connection, true);
                dispatch_async(dispatch_get_main_queue(), ^{
                    WaitForMoreDataInBackground(connection);
                });
            });
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamManagerAcceptTCPStream(
        HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager,
        HAPPlatformTCPStreamRef* _Nonnull stream) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(stream);
    HAPPrecondition(socketsWaitingToBeAccepted.count > 0);

    Connection* connection = socketsWaitingToBeAccepted.lastObject;
    [socketsWaitingToBeAccepted removeLastObject];
    HAPAssert(![connections containsObject:connection]);
    [connections addObject:connection];
    *stream = (HAPPlatformTCPStreamRef)(__bridge void*) connection;

    nw_connection_set_queue(connection.socket, dispatch_get_main_queue());
    nw_connection_set_state_changed_handler(connection.socket, ^(nw_connection_state_t state, nw_error_t error) {
        connection.state = state;
        if (error) {
            return;
        }
        if (state == nw_connection_state_ready) {
            EventCallback(connection, false);
        }
    });
    WaitForMoreDataInBackground(connection);
    nw_connection_start(connection.socket);

    HAPLogDebug(&tcp_log, "Accept %lx", *stream);

    return kHAPError_None;
}

void HAPPlatformTCPStreamCloseOutput(
        HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream);

    HAPLogDebug(&tcp_log, "CloseOutput %lx", tcpStream);

    Connection* connection = (__bridge Connection*) (void*) tcpStream;
    HAPAssert([connections containsObject:connection]);
}

void HAPPlatformTCPStreamClose(
        HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream);

    HAPLogDebug(&tcp_log, "Close %lx", tcpStream);

    Connection* connection = (__bridge Connection*) (void*) tcpStream;
    HAPAssert([connections containsObject:connection]);

    [connections removeObject:connection];

    nw_connection_cancel(connection.socket);
}

void HAPPlatformTCPStreamUpdateInterests(
        HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream,
        HAPPlatformTCPStreamEvent interests,
        HAPPlatformTCPStreamEventCallback _Nullable callback,
        void* _Nullable context) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(tcpStream);

    Connection* connection = (__bridge Connection*) (void*) tcpStream;
    HAPAssert([connections containsObject:connection]);

    connection.interests = interests;
    connection.callback = callback;
    connection.context = context;

    if (interests.hasSpaceAvailable && connection.state == nw_connection_state_ready) {
        dispatch_async(dispatch_get_main_queue(), ^{
            EventCallback(connection, false);
        });
    }
    if (interests.hasBytesAvailable && connection.buffer.length > 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            EventCallback(connection, true);
        });
    }

    HAPLogDebug(
            &tcp_log,
            "SetInterest %lx: %s%s",
            tcpStream,
            interests.hasBytesAvailable ? "hasBytesAvailable " : "",
            interests.hasSpaceAvailable ? "hasSpaceAvailable " : "");
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamRead(
        HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream __unused,
        void* _Nonnull bytes,
        size_t maxBytes,
        size_t* _Nonnull numBytes) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    Connection* connection = (__bridge Connection*) (void*) tcpStream;
    HAPAssert([connections containsObject:connection]);

    NSData* buffer = connection.buffer;
    if (!buffer.length) {
        *numBytes = 0;
        return kHAPError_None;
    }

    size_t n = buffer.length;
    if (n > maxBytes) {
        n = maxBytes;
    }
    memcpy(bytes, buffer.bytes, *numBytes = n);
    HAPAssert(n <= buffer.length);
    connection.buffer = [buffer subdataWithRange:NSMakeRange(n, buffer.length - n)];

    // HAPLogBufferDebug(&tcp_log, bytes, n, "Read %lx", tcpStream);

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTCPStreamWrite(
        HAPPlatformTCPStreamManagerRef _Nonnull tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream,
        const void* _Nonnull bytes,
        size_t maxBytes,
        size_t* _Nonnull numBytes) {
    HAPPrecondition(tcpStreamManager);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    Connection* connection = (__bridge Connection*) (void*) tcpStream;
    HAPAssert([connections containsObject:connection]);

    // HAPLogBufferDebug(&tcp_log, bytes, maxBytes, "Write %lx", tcpStream);

    nw_content_context_t context = nw_content_context_create("data");
    nw_connection_send(
            connection.socket,
            dispatch_data_create(bytes, maxBytes, dispatch_get_main_queue(), DISPATCH_DATA_DESTRUCTOR_DEFAULT),
            context,
            true,
            ^(nw_error_t error) {
                HAPAssert(!error);
                EventCallback(connection, false);
            });
    *numBytes = maxBytes;

    return kHAPError_None;
}
