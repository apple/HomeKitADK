// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

#if HAP_IP

#include "util_base64.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "IPAccessoryServer" };

/** Build-time flag to disable session security. */
#define kHAPIPAccessoryServer_SessionSecurityDisabled ((bool) false)

/** US-ASCII horizontal-tab character. */
#define kHAPIPAccessoryServerCharacter_HorizontalTab ((char) 9)

/** US-ASCII space character. */
#define kHAPIPAccessoryServerCharacter_Space ((char) 32)

/**
 * HAP Status Codes.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 6-11 HAP Status Codes
 */
/**@{*/
/** This specifies a success for the request. */
#define kHAPIPAccessoryServerStatusCode_Success ((int32_t) 0)

/** Request denied due to insufficient privileges. */
#define kHAPIPAccessoryServerStatusCode_InsufficientPrivileges ((int32_t) -70401)

/** Unable to perform operation with requested service or characteristic. */
#define kHAPIPAccessoryServerStatusCode_UnableToPerformOperation ((int32_t) -70402)

/** Resource is busy, try again. */
#define kHAPIPAccessoryServerStatusCode_ResourceIsBusy ((int32_t) -70403)

/** Cannot write to read only characteristic. */
#define kHAPIPAccessoryServerStatusCode_WriteToReadOnlyCharacteristic ((int32_t) -70404)

/** Cannot read from a write only characteristic. */
#define kHAPIPAccessoryServerStatusCode_ReadFromWriteOnlyCharacteristic ((int32_t) -70405)

/** Notification is not supported for characteristic. */
#define kHAPIPAccessoryServerStatusCode_NotificationNotSupported ((int32_t) -70406)

/** Out of resources to process request. */
#define kHAPIPAccessoryServerStatusCode_OutOfResources ((int32_t) -70407)

/** Resource does not exist. */
#define kHAPIPAccessoryServerStatusCode_ResourceDoesNotExist ((int32_t) -70409)

/** Accessory received an invalid value in a write request. */
#define kHAPIPAccessoryServerStatusCode_InvalidValueInWrite ((int32_t) -70410)

/** Insufficient Authorization. */
#define kHAPIPAccessoryServerStatusCode_InsufficientAuthorization ((int32_t) -70411)

/**@}*/

/**
 * Predefined HTTP/1.1 response indicating successful request completion with an empty response body.
 */
#define kHAPIPAccessoryServerResponse_NoContent ("HTTP/1.1 204 No Content\r\n\r\n")

/**
 * Predefined HTTP/1.1 response indicating a malformed request.
 */
#define kHAPIPAccessoryServerResponse_BadRequest \
    ("HTTP/1.1 400 Bad Request\r\n" \
     "Content-Length: 0\r\n\r\n")

/**
 * Predefined HTTP/1.1 response indicating that the client has insufficient privileges to request the corresponding
 * operation.
 */
#define kHAPIPAccessoryServerResponse_InsufficientPrivileges \
    ("HTTP/1.1 400 Bad Request\r\n" \
     "Content-Type: application/hap+json\r\n" \
     "Content-Length: 17\r\n\r\n" \
     "{\"status\":-70401}")

/**
 * Predefined HTTP/1.1 response indicating that the requested resource is not available.
 */
#define kHAPIPAccessoryServerResponse_ResourceNotFound \
    ("HTTP/1.1 404 Not Found\r\n" \
     "Content-Length: 0\r\n\r\n")

/**
 * Predefined HTTP/1.1 response indicating that the requested operation is not supported for the requested resource.
 */
#define kHAPIPAccessoryServerResponse_MethodNotAllowed \
    ("HTTP/1.1 405 Method Not Allowed\r\n" \
     "Content-Length: 0\r\n\r\n")

/**
 * Predefined HTTP/1.1 response indicating that the connection is not authorized to request the corresponding operation.
 */
#define kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired \
    ("HTTP/1.1 470 Connection Authorization Required\r\n" \
     "Content-Length: 0\r\n\r\n")

/**
 * Predefined HTTP/1.1 response indicating that the connection is not authorized to request the corresponding operation,
 * including a HAP status code.
 */
#define kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequiredWithStatus \
    ("HTTP/1.1 470 Connection Authorization Required\r\n" \
     "Content-Type: application/hap+json\r\n" \
     "Content-Length: 17\r\n\r\n" \
     "{\"status\":-70411}")

/**
 * Predefined HTTP/1.1 response indicating that the server encountered an unexpected condition which prevented it from
 * successfully processing the request.
 */
#define kHAPIPAccessoryServerResponse_InternalServerError \
    ("HTTP/1.1 500 Internal Server Error\r\n" \
     "Content-Length: 0\r\n\r\n")

/**
 * Predefined HTTP/1.1 response indicating that the server did not have enough resources to process request.
 */
#define kHAPIPAccessoryServerResponse_OutOfResources \
    ("HTTP/1.1 500 Internal Server Error\r\n" \
     "Content-Type: application/hap+json\r\n" \
     "Content-Length: 17\r\n\r\n" \
     "{\"status\":-70407}")

/**
 * Maximum time an IP session can stay idle before it will be closed by the accessory server.
 *
 * - Maximum idle time will on be enforced during shutdown of the accessory server or at maximum capacity.
 */
#define kHAPIPSession_MaxIdleTime ((HAPTime)(60 * HAPSecond))

/**
 * Maximum delay during which event notifications will be coalesced into a single message.
 */
#define kHAPIPAccessoryServer_MaxEventNotificationDelay ((HAPTime)(1 * HAPSecond))

static void log_result(HAPLogType type, char* msg, int result, const char* function, const char* file, int line) {
    HAPAssert(msg);
    HAPAssert(function);
    HAPAssert(file);

    HAPLogWithType(&logObject, type, "%s:%d - %s @ %s:%d", msg, result, function, file, line);
}

static void log_protocol_error(
        HAPLogType type,
        char* msg,
        HAPIPByteBuffer* b,
        const char* function,
        const char* file,
        int line) {
    HAPAssert(msg);
    HAPAssert(b);
    HAPAssert(function);
    HAPAssert(file);

    HAPLogBufferWithType(
            &logObject,
            b->data,
            b->position,
            type,
            "%s:%lu - %s @ %s:%d",
            msg,
            (unsigned long) b->position,
            function,
            file,
            line);
}

static void get_db_ctx(
        HAPAccessoryServerRef* server_,
        uint64_t aid,
        uint64_t iid,
        const HAPCharacteristic** chr,
        const HAPService** svc,
        const HAPAccessory** acc) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(chr);
    HAPPrecondition(svc);
    HAPPrecondition(acc);

    *chr = NULL;
    *svc = NULL;
    *acc = NULL;

    const HAPAccessory* accessory = NULL;

    if (server->primaryAccessory->aid == aid) {
        accessory = server->primaryAccessory;
    } else if (server->ip.bridgedAccessories) {
        for (size_t i = 0; server->ip.bridgedAccessories[i]; i++) {
            if (server->ip.bridgedAccessories[i]->aid == aid) {
                accessory = server->ip.bridgedAccessories[i];
                break;
            }
        }
    }

    if (accessory) {
        size_t i = 0;
        while (accessory->services[i] && !*chr) {
            const HAPService* service = accessory->services[i];
            if (HAPAccessoryServerSupportsService(server_, kHAPTransportType_IP, service)) {
                size_t j = 0;
                while (service->characteristics[j] && !*chr) {
                    const HAPBaseCharacteristic* characteristic = service->characteristics[j];
                    if (HAPIPCharacteristicIsSupported(characteristic)) {
                        if (characteristic->iid == iid) {
                            *chr = characteristic;
                            *svc = service;
                            *acc = accessory;
                        } else {
                            j++;
                        }
                    } else {
                        j++;
                    }
                }
                if (!*chr) {
                    i++;
                }
            } else {
                i++;
            }
        }
    }
}

static void publish_homeKit_service(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPAssert(!server->ip.isServiceDiscoverable);
    HAPAssert(HAPPlatformTCPStreamManagerIsListenerOpen(HAPNonnull(server->platform.ip.tcpStreamManager)));

    HAPIPServiceDiscoverySetHAPService(server_);
    server->ip.isServiceDiscoverable = true;
}

static void HandlePendingTCPStream(HAPPlatformTCPStreamManagerRef tcpStreamManager, void* _Nullable context);

static void schedule_max_idle_time_timer(HAPAccessoryServerRef* server_);

static void HAPIPSessionDestroy(HAPIPSession* ipSession) {
    HAPPrecondition(ipSession);

    HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) &ipSession->descriptor;
    if (!session->server) {
        return;
    }

    HAPLogDebug(&logObject, "session:%p:releasing session", (const void*) session);

    HAPRawBufferZero(&ipSession->descriptor, sizeof ipSession->descriptor);
    HAPRawBufferZero(ipSession->inboundBuffer.bytes, ipSession->inboundBuffer.numBytes);
    HAPRawBufferZero(ipSession->outboundBuffer.bytes, ipSession->outboundBuffer.numBytes);
    HAPRawBufferZero(
            ipSession->eventNotifications, ipSession->numEventNotifications * sizeof *ipSession->eventNotifications);
}

static void collect_garbage(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (server->ip.garbageCollectionTimer) {
        HAPPlatformTimerDeregister(server->ip.garbageCollectionTimer);
        server->ip.garbageCollectionTimer = 0;
    }

    size_t n = 0;
    for (size_t i = 0; i < server->ip.storage->numSessions; i++) {
        HAPIPSession* ipSession = &server->ip.storage->sessions[i];
        HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) &ipSession->descriptor;
        if (!session->server) {
            continue;
        }

        if (session->state == kHAPIPSessionState_Idle) {
            HAPIPSessionDestroy(ipSession);
            HAPAssert(server->ip.numSessions > 0);
            server->ip.numSessions--;
        } else {
            n++;
        }
    }
    HAPAssert(n == server->ip.numSessions);

    // If there are open sessions, wait until they are closed before continuing.
    if (HAPPlatformTCPStreamManagerIsListenerOpen(HAPNonnull(server->platform.ip.tcpStreamManager)) ||
        (server->ip.numSessions != 0)) {
        return;
    }

    // Finalize server state transition after last session closed.
    HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Stopping);
    if (server->ip.stateTransitionTimer) {
        HAPPlatformTimerDeregister(server->ip.stateTransitionTimer);
        server->ip.stateTransitionTimer = 0;
    }
    if (server->ip.maxIdleTimeTimer) {
        HAPPlatformTimerDeregister(server->ip.maxIdleTimeTimer);
        server->ip.maxIdleTimeTimer = 0;
    }
    HAPLogDebug(&logObject, "Completing accessory server state transition.");
    if (server->ip.nextState == kHAPIPAccessoryServerState_Running) {
        server->ip.state = kHAPIPAccessoryServerState_Running;
        server->ip.nextState = kHAPIPAccessoryServerState_Undefined;
        HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);
    } else {
        HAPAssert(server->ip.nextState == kHAPIPAccessoryServerState_Idle);

        // HAPAccessoryServerStop.

        if (server->ip.isServiceDiscoverable) {
            HAPIPServiceDiscoveryStop(server_);
            server->ip.isServiceDiscoverable = false;
        }

        // Stop service discovery.
        if (server->ip.discoverableService) {
            HAPAssert(!server->ip.isServiceDiscoverable);
            HAPAssert(server->ip.discoverableService == kHAPIPServiceDiscoveryType_HAP);
            HAPIPServiceDiscoveryStop(server_);
        }

        HAPAssert(!server->ip.discoverableService);
        HAPAssert(!server->ip.isServiceDiscoverable);

        server->ip.state = kHAPIPAccessoryServerState_Idle;
        server->ip.nextState = kHAPIPAccessoryServerState_Undefined;
        HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);
    }
}

static void handle_garbage_collection_timer(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    (void) server;
    HAPPrecondition(timer == server->ip.garbageCollectionTimer);
    server->ip.garbageCollectionTimer = 0;

    collect_garbage(server_);
}

static void handle_max_idle_time_timer(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    (void) server;
    HAPPrecondition(timer == server->ip.maxIdleTimeTimer);
    server->ip.maxIdleTimeTimer = 0;

    HAPLogDebug(&logObject, "Session idle timer expired.");
    schedule_max_idle_time_timer(server_);
}

static void CloseSession(HAPIPSessionDescriptor* session);

static void schedule_max_idle_time_timer(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (server->ip.maxIdleTimeTimer) {
        HAPPlatformTimerDeregister(server->ip.maxIdleTimeTimer);
        server->ip.maxIdleTimeTimer = 0;
    }

    HAPError err;

    HAPTime clock_now_ms = HAPPlatformClockGetCurrent();

    int64_t timeout_ms = -1;

    if ((server->ip.state == kHAPIPAccessoryServerState_Stopping) &&
        HAPPlatformTCPStreamManagerIsListenerOpen(HAPNonnull(server->platform.ip.tcpStreamManager))) {
        HAPPlatformTCPStreamManagerCloseListener(HAPNonnull(server->platform.ip.tcpStreamManager));
    }

    for (size_t i = 0; i < server->ip.storage->numSessions; i++) {
        HAPIPSession* ipSession = &server->ip.storage->sessions[i];
        HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) &ipSession->descriptor;
        if (!session->server) {
            continue;
        }

        if ((session->state == kHAPIPSessionState_Reading) && (session->inboundBuffer.position == 0) &&
            (server->ip.state == kHAPIPAccessoryServerState_Stopping)) {
            CloseSession(session);
        } else if (
                ((session->state == kHAPIPSessionState_Reading) || (session->state == kHAPIPSessionState_Writing)) &&
                ((server->ip.numSessions == server->ip.storage->numSessions) ||
                 (server->ip.state == kHAPIPAccessoryServerState_Stopping))) {
            HAPAssert(clock_now_ms >= session->stamp);
            HAPTime dt_ms = clock_now_ms - session->stamp;
            if (dt_ms < kHAPIPSession_MaxIdleTime) {
                HAPAssert(kHAPIPSession_MaxIdleTime <= INT64_MAX);
                int64_t t_ms = (int64_t)(kHAPIPSession_MaxIdleTime - dt_ms);
                if ((timeout_ms == -1) || (t_ms < timeout_ms)) {
                    timeout_ms = t_ms;
                }
            } else {
                HAPLogInfo(&logObject, "Connection timeout.");
                CloseSession(session);
            }
        }
    }

    if (timeout_ms >= 0) {
        HAPTime deadline_ms;

        if (UINT64_MAX - clock_now_ms < (HAPTime) timeout_ms) {
            HAPLog(&logObject, "Clipping maximum idle time timer to avoid clock overflow.");
            deadline_ms = UINT64_MAX;
        } else {
            deadline_ms = clock_now_ms + (HAPTime) timeout_ms;
        }
        HAPAssert(deadline_ms >= clock_now_ms);

        err = HAPPlatformTimerRegister(&server->ip.maxIdleTimeTimer, deadline_ms, handle_max_idle_time_timer, server_);
        if (err) {
            HAPLog(&logObject, "Not enough resources to schedule maximum idle time timer!");
            HAPFatalError();
        }
        HAPAssert(server->ip.maxIdleTimeTimer);
    }

    if (!server->ip.garbageCollectionTimer) {
        err = HAPPlatformTimerRegister(&server->ip.garbageCollectionTimer, 0, handle_garbage_collection_timer, server_);
        if (err) {
            HAPLog(&logObject, "Not enough resources to schedule garbage collection!");
            HAPFatalError();
        }
        HAPAssert(server->ip.garbageCollectionTimer);
    }
}

static void RegisterSession(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(server->ip.numSessions < server->ip.storage->numSessions);

    server->ip.numSessions++;
    if (server->ip.numSessions == server->ip.storage->numSessions) {
        schedule_max_idle_time_timer(session->server);
    }
}

static void handle_characteristic_unsubscribe_request(
        HAPIPSessionDescriptor* session,
        const HAPCharacteristic* chr,
        const HAPService* svc,
        const HAPAccessory* acc);

static void CloseSession(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;

    HAPAssert(session->state != kHAPIPSessionState_Idle);

    HAPError err;

    HAPLogDebug(&logObject, "session:%p:closing", (const void*) session);

    while (session->numEventNotifications) {
        HAPIPEventNotification* eventNotification =
                (HAPIPEventNotification*) &session->eventNotifications[session->numEventNotifications - 1];
        const HAPCharacteristic* characteristic;
        const HAPService* service;
        const HAPAccessory* accessory;
        get_db_ctx(
                session->server, eventNotification->aid, eventNotification->iid, &characteristic, &service, &accessory);
        if (eventNotification->flag) {
            HAPAssert(session->numEventNotificationFlags);
            session->numEventNotificationFlags--;
        }
        session->numEventNotifications--;
        handle_characteristic_unsubscribe_request(session, characteristic, service, accessory);
    }
    if (session->securitySession.isOpen) {
        HAPLogDebug(&logObject, "session:%p:closing security context", (const void*) session);
        switch (session->securitySession.type) {
            case kHAPIPSecuritySessionType_HAP: {
                HAPLogDebug(&logObject, "Closing HAP session.");
                HAPSessionRelease(HAPNonnull(session->server), &session->securitySession._.hap);
                HAPRawBufferZero(&session->securitySession, sizeof session->securitySession);
            } break;
            case kHAPIPSecuritySessionType_MFiSAP: {
                HAPLogDebug(&logObject, "Closing MFi SAP session.");
                HAPRawBufferZero(&session->securitySession, sizeof session->securitySession);
            } break;
        }
        HAPAssert(!session->securitySession.type);
        HAPAssert(!session->securitySession.isSecured);
        HAPAssert(!session->securitySession.isOpen);
    }
    if (session->tcpStreamIsOpen) {
        HAPLogDebug(&logObject, "session:%p:closing TCP stream", (const void*) session);
        HAPPlatformTCPStreamClose(HAPNonnull(server->platform.ip.tcpStreamManager), session->tcpStream);
        session->tcpStreamIsOpen = false;
    }
    session->state = kHAPIPSessionState_Idle;
    if (!server->ip.garbageCollectionTimer) {
        err = HAPPlatformTimerRegister(
                &server->ip.garbageCollectionTimer, 0, handle_garbage_collection_timer, session->server);
        if (err) {
            HAPLog(&logObject, "Not enough resources to schedule garbage collection!");
            HAPFatalError();
        }
        HAPAssert(server->ip.garbageCollectionTimer);
    }

    HAPLogDebug(&logObject, "session:%p:closed", (const void*) session);
}

static void OpenSecuritySession(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(!session->securitySession.isOpen);
    HAPPrecondition(!session->securitySession.isSecured);

    HAPLogDebug(&logObject, "Opening HAP session.");
    session->securitySession.type = kHAPIPSecuritySessionType_HAP;
    HAPSessionCreate(HAPNonnull(session->server), &session->securitySession._.hap, kHAPTransportType_IP);

    session->securitySession.isOpen = true;
}

static void write_msg(HAPIPByteBuffer* b, const char* msg) {
    HAPError err;

    err = HAPIPByteBufferAppendStringWithFormat(b, "%s", msg);
    HAPAssert(!err);
}

static void prepare_reading_request(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);

    util_http_reader_init(&session->httpReader, util_HTTP_READER_TYPE_REQUEST);
    session->httpReaderPosition = 0;
    session->httpParserError = false;
    session->httpMethod.bytes = NULL;
    session->httpURI.bytes = NULL;
    session->httpHeaderFieldName.bytes = NULL;
    session->httpHeaderFieldValue.bytes = NULL;
    session->httpContentLength.isDefined = false;
    session->httpContentType = kHAPIPAccessoryServerContentType_Unknown;
}

static void handle_input(HAPIPSessionDescriptor* session);

static void post_resource(HAPIPSessionDescriptor* session) {
}

static void put_prepare(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));

    HAPError err;
    uint64_t ttl, pid;

    HAPAssert(session->inboundBuffer.data);
    HAPAssert(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPAssert(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPAssert(session->httpReaderPosition <= session->inboundBuffer.position);
    if (session->httpContentLength.isDefined) {
        HAPAssert(session->httpContentLength.value <= session->inboundBuffer.position - session->httpReaderPosition);
        err = HAPIPAccessoryProtocolGetCharacteristicWritePreparation(
                &session->inboundBuffer.data[session->httpReaderPosition],
                session->httpContentLength.value,
                &ttl,
                &pid);
        if (!err) {
            HAPLogDebug(&logObject, "Prepare Write Request - TTL = %lu ms.", (unsigned long) ttl);

            // If the accessory receives consecutive Prepare Write Requests in the same session, the accessory must
            // reset the timed write transaction with the TTL specified by the latest request.
            // See HomeKit Accessory Protocol Specification R14
            // Section 6.7.2.4 Timed Write Procedures
            // Assumption: Same behavior for PID.

            // TTL.
            HAPTime clock_now_ms = HAPPlatformClockGetCurrent();
            if (UINT64_MAX - clock_now_ms < ttl) {
                HAPLog(&logObject, "Clipping TTL to avoid clock overflow.");
                session->timedWriteExpirationTime = UINT64_MAX;
            } else {
                session->timedWriteExpirationTime = clock_now_ms + ttl;
            }
            HAPAssert(session->timedWriteExpirationTime >= clock_now_ms);

            // PID.
            session->timedWritePID = pid;

            // The accessory must respond with a 200 OK HTTP Status Code and include a HAP status code indicating if
            // timed write procedure can be executed or not.
            // See HomeKit Accessory Protocol Specification R14
            // Section 6.7.2.4 Timed Write Procedures
            // It is not documented under what conditions this should fail.
            write_msg(
                    &session->outboundBuffer,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/hap+json\r\n"
                    "Content-Length: 12\r\n\r\n"
                    "{\"status\":0}");
        } else {
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
        }
    } else {
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
    }
}

static void write_characteristic_write_response(
        HAPIPSessionDescriptor* session,
        HAPIPWriteContextRef* contexts,
        size_t contexts_count) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));

    HAPError err;
    size_t content_length, mark;

    HAPAssert(contexts);
    HAPAssert(session->outboundBuffer.data);
    HAPAssert(session->outboundBuffer.position <= session->outboundBuffer.limit);
    HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);
    content_length = HAPIPAccessoryProtocolGetNumCharacteristicWriteResponseBytes(
            HAPNonnull(session->server), contexts, contexts_count);
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pa084)
    if (content_length <= UINT32_MAX) {
        mark = session->outboundBuffer.position;
        err = HAPIPByteBufferAppendStringWithFormat(
                &session->outboundBuffer,
                "HTTP/1.1 207 Multi-Status\r\n"
                "Content-Type: application/hap+json\r\n"
                "Content-Length: %lu\r\n\r\n",
                (unsigned long) content_length);
        HAPAssert(!err);
        if (content_length <= session->outboundBuffer.limit - session->outboundBuffer.position) {
            mark = session->outboundBuffer.position;
            err = HAPIPAccessoryProtocolGetCharacteristicWriteResponseBytes(
                    HAPNonnull(session->server), contexts, contexts_count, &session->outboundBuffer);
            HAPAssert(!err && (session->outboundBuffer.position - mark == content_length));
        } else {
            HAPLog(&logObject, "Out of resources (outbound buffer too small).");
            session->outboundBuffer.position = mark;
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
        }
    } else {
        HAPLog(&logObject, "Content length exceeding UINT32_MAX.");
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
    }
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pa084)
}

static void schedule_event_notifications(HAPAccessoryServerRef* server_);

static void handle_event_notification_timer(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(timer == server->ip.eventNotificationTimer);
    server->ip.eventNotificationTimer = 0;

    HAPLogDebug(&logObject, "Event notification timer expired.");
    schedule_event_notifications(server_);
}

static void write_event_notifications(HAPIPSessionDescriptor* session);

static void schedule_event_notifications(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (server->ip.eventNotificationTimer) {
        HAPPlatformTimerDeregister(server->ip.eventNotificationTimer);
        server->ip.eventNotificationTimer = 0;
    }

    HAPError err;

    for (size_t i = 0; i < server->ip.storage->numSessions; i++) {
        HAPIPSession* ipSession = &server->ip.storage->sessions[i];
        HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) &ipSession->descriptor;
        if (!session->server) {
            continue;
        }

        if ((session->state == kHAPIPSessionState_Reading) && (session->inboundBuffer.position == 0) &&
            (session->numEventNotificationFlags > 0)) {
            write_event_notifications(session);
        }
    }

    HAPTime clock_now_ms = HAPPlatformClockGetCurrent();
    int64_t timeout_ms = -1;

    for (size_t i = 0; i < server->ip.storage->numSessions; i++) {
        HAPIPSession* ipSession = &server->ip.storage->sessions[i];
        HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) &ipSession->descriptor;
        if (!session->server) {
            continue;
        }

        if ((session->state == kHAPIPSessionState_Reading) && (session->inboundBuffer.position == 0) &&
            (session->numEventNotificationFlags > 0)) {
            HAPAssert(clock_now_ms >= session->eventNotificationStamp);
            HAPTime dt_ms = clock_now_ms - session->eventNotificationStamp;
            HAP_DIAGNOSTIC_PUSH
            HAP_DIAGNOSTIC_IGNORED_ARMCC(186)
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wtype-limits")
            if (dt_ms < kHAPIPAccessoryServer_MaxEventNotificationDelay) {
                HAPAssert(kHAPIPAccessoryServer_MaxEventNotificationDelay <= INT64_MAX);
                int64_t t_ms = (int64_t)(kHAPIPAccessoryServer_MaxEventNotificationDelay - dt_ms);
                if ((timeout_ms == -1) || (t_ms < timeout_ms)) {
                    timeout_ms = t_ms;
                }
            } else {
                timeout_ms = 0;
            }
            HAP_DIAGNOSTIC_POP
        }
    }

    if (timeout_ms >= 0) {
        HAPTime deadline_ms;

        if (UINT64_MAX - clock_now_ms < (HAPTime) timeout_ms) {
            HAPLog(&logObject, "Clipping event notification timer to avoid clock overflow.");
            deadline_ms = UINT64_MAX;
        } else {
            deadline_ms = clock_now_ms + (HAPTime) timeout_ms;
        }
        HAPAssert(deadline_ms >= clock_now_ms);

        err = HAPPlatformTimerRegister(
                &server->ip.eventNotificationTimer, deadline_ms, handle_event_notification_timer, server_);
        if (err) {
            HAPLog(&logObject, "Not enough resources to schedule event notification timer!");
            HAPFatalError();
        }
        HAPAssert(server->ip.eventNotificationTimer);
    }
}

static void handle_characteristic_subscribe_request(
        HAPIPSessionDescriptor* session,
        const HAPCharacteristic* chr,
        const HAPService* svc,
        const HAPAccessory* acc) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));
    HAPPrecondition(chr);
    HAPPrecondition(svc);
    HAPPrecondition(acc);

    HAPAccessoryServerHandleSubscribe(HAPNonnull(session->server), &session->securitySession._.hap, chr, svc, acc);
}

static void handle_characteristic_unsubscribe_request(
        HAPIPSessionDescriptor* session,
        const HAPCharacteristic* chr,
        const HAPService* svc,
        const HAPAccessory* acc) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));
    HAPPrecondition(chr);
    HAPPrecondition(svc);
    HAPPrecondition(acc);

    HAPAccessoryServerHandleUnsubscribe(HAPNonnull(session->server), &session->securitySession._.hap, chr, svc, acc);
}

static void handle_characteristic_read_request(
        HAPIPSessionDescriptor* session,
        const HAPCharacteristic* chr,
        const HAPService* svc,
        const HAPAccessory* acc,
        HAPIPReadContextRef* ctx,
        HAPIPByteBuffer* data_buffer);

/**
 * Converts a characteristic write request error to the corresponding HAP status code.
 *
 * @param      error                Write request error.
 *
 * @return HAP write request status code.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 6-11 HAP Status Codes
 */
static int32_t ConvertCharacteristicWriteErrorToStatusCode(HAPError error) {
    switch (error) {
        case kHAPError_None: {
            return kHAPIPAccessoryServerStatusCode_Success;
        }
        case kHAPError_Unknown: {
            return kHAPIPAccessoryServerStatusCode_UnableToPerformOperation;
        }
        case kHAPError_InvalidState: {
            return kHAPIPAccessoryServerStatusCode_UnableToPerformOperation;
        }
        case kHAPError_InvalidData: {
            return kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
        }
        case kHAPError_OutOfResources: {
            return kHAPIPAccessoryServerStatusCode_OutOfResources;
        }
        case kHAPError_NotAuthorized: {
            return kHAPIPAccessoryServerStatusCode_InsufficientAuthorization;
        }
        case kHAPError_Busy: {
            return kHAPIPAccessoryServerStatusCode_ResourceIsBusy;
        }
    }
    HAPFatalError();
}

static void handle_characteristic_write_request(
        HAPIPSessionDescriptor* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPIPWriteContextRef* context,
        HAPIPByteBuffer* dataBuffer) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(context);
    HAPPrecondition(dataBuffer);

    HAPError err;

    const HAPBaseCharacteristic* baseCharacteristic = characteristic;

    HAPIPWriteContext* writeContext = (HAPIPWriteContext*) context;
    HAPAssert(baseCharacteristic->iid == writeContext->iid);

    if ((writeContext->type == kHAPIPWriteValueType_None) &&
        (writeContext->ev == kHAPIPEventNotificationState_Undefined)) {
        writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
        return;
    }

    if (writeContext->ev != kHAPIPEventNotificationState_Undefined) {
        if (HAPCharacteristicReadRequiresAdminPermissions(baseCharacteristic) &&
            !HAPSessionControllerIsAdmin(&session->securitySession._.hap)) {
            writeContext->status = kHAPIPAccessoryServerStatusCode_InsufficientPrivileges;
        } else if (!baseCharacteristic->properties.supportsEventNotification) {
            writeContext->status = kHAPIPAccessoryServerStatusCode_NotificationNotSupported;
        } else {
            writeContext->status = kHAPIPAccessoryServerStatusCode_Success;
            HAPAssert(session->numEventNotifications <= session->maxEventNotifications);
            size_t i = 0;
            while ((i < session->numEventNotifications) &&
                   ((((HAPIPEventNotification*) &session->eventNotifications[i])->aid != writeContext->aid) ||
                    (((HAPIPEventNotification*) &session->eventNotifications[i])->iid != writeContext->iid))) {
                i++;
            }
            HAPAssert(
                    (i == session->numEventNotifications) ||
                    ((i < session->numEventNotifications) &&
                     (((HAPIPEventNotification*) &session->eventNotifications[i])->aid == writeContext->aid) &&
                     (((HAPIPEventNotification*) &session->eventNotifications[i])->iid == writeContext->iid)));
            if (i == session->numEventNotifications) {
                if (writeContext->ev == kHAPIPEventNotificationState_Enabled) {
                    if (i == session->maxEventNotifications) {
                        writeContext->status = kHAPIPAccessoryServerStatusCode_OutOfResources;
                    } else {
                        ((HAPIPEventNotification*) &session->eventNotifications[i])->aid = writeContext->aid;
                        ((HAPIPEventNotification*) &session->eventNotifications[i])->iid = writeContext->iid;
                        ((HAPIPEventNotification*) &session->eventNotifications[i])->flag = false;
                        session->numEventNotifications++;
                        handle_characteristic_subscribe_request(session, characteristic, service, accessory);
                    }
                }
            } else if (writeContext->ev == kHAPIPEventNotificationState_Disabled) {
                session->numEventNotifications--;
                if (((HAPIPEventNotification*) &session->eventNotifications[i])->flag) {
                    HAPAssert(session->numEventNotificationFlags > 0);
                    session->numEventNotificationFlags--;
                }
                while (i < session->numEventNotifications) {
                    HAPRawBufferCopyBytes(
                            &session->eventNotifications[i],
                            &session->eventNotifications[i + 1],
                            sizeof session->eventNotifications[i]);
                    i++;
                }
                HAPAssert(i == session->numEventNotifications);
                handle_characteristic_unsubscribe_request(session, characteristic, service, accessory);
            }
        }
    }

    if (writeContext->type != kHAPIPWriteValueType_None) {
        if (HAPCharacteristicWriteRequiresAdminPermissions(baseCharacteristic) &&
            !HAPSessionControllerIsAdmin(&session->securitySession._.hap)) {
            writeContext->status = kHAPIPAccessoryServerStatusCode_InsufficientPrivileges;
            return;
        }
        if ((baseCharacteristic->properties.ip.supportsWriteResponse || writeContext->response) &&
            HAPCharacteristicReadRequiresAdminPermissions(baseCharacteristic) &&
            !HAPSessionControllerIsAdmin(&session->securitySession._.hap)) {
            writeContext->status = kHAPIPAccessoryServerStatusCode_InsufficientPrivileges;
            return;
        }
        if (baseCharacteristic->properties.writable) {
            writeContext->status = kHAPIPAccessoryServerStatusCode_Success;
            const void* authorizationDataBytes = NULL;
            size_t numAuthorizationDataBytes = 0;
            if (writeContext->authorizationData.bytes) {
                int r = util_base64_decode(
                        writeContext->authorizationData.bytes,
                        writeContext->authorizationData.numBytes,
                        writeContext->authorizationData.bytes,
                        writeContext->authorizationData.numBytes,
                        &writeContext->authorizationData.numBytes);
                if (r == 0) {
                    authorizationDataBytes = writeContext->authorizationData.bytes;
                    numAuthorizationDataBytes = writeContext->authorizationData.numBytes;
                } else {
                    writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                }
            }
            if (writeContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                switch (baseCharacteristic->format) {
                    case kHAPCharacteristicFormat_Data: {
                        if (writeContext->type == kHAPIPWriteValueType_String) {
                            HAPAssert(writeContext->value.stringValue.bytes);
                            int r = util_base64_decode(
                                    writeContext->value.stringValue.bytes,
                                    writeContext->value.stringValue.numBytes,
                                    writeContext->value.stringValue.bytes,
                                    writeContext->value.stringValue.numBytes,
                                    &writeContext->value.stringValue.numBytes);
                            if (r == 0) {
                                HAPAssert(writeContext->value.stringValue.bytes);
                                err = HAPDataCharacteristicHandleWrite(
                                        HAPNonnull(session->server),
                                        &(const HAPDataCharacteristicWriteRequest) {
                                                .transportType = kHAPTransportType_IP,
                                                .session = &session->securitySession._.hap,
                                                .characteristic = (const HAPDataCharacteristic*) baseCharacteristic,
                                                .service = service,
                                                .accessory = accessory,
                                                .remote = writeContext->remote,
                                                .authorizationData = { .bytes = authorizationDataBytes,
                                                                       .numBytes = numAuthorizationDataBytes } },
                                        HAPNonnull(writeContext->value.stringValue.bytes),
                                        writeContext->value.stringValue.numBytes,
                                        HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                                writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                            } else {
                                writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                            }
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_Bool: {
                        if ((writeContext->type == kHAPIPWriteValueType_UInt) &&
                            (writeContext->value.unsignedIntValue <= 1)) {
                            err = HAPBoolCharacteristicHandleWrite(
                                    HAPNonnull(session->server),
                                    &(const HAPBoolCharacteristicWriteRequest) {
                                            .transportType = kHAPTransportType_IP,
                                            .session = &session->securitySession._.hap,
                                            .characteristic = (const HAPBoolCharacteristic*) baseCharacteristic,
                                            .service = service,
                                            .accessory = accessory,
                                            .remote = writeContext->remote,
                                            .authorizationData = { .bytes = authorizationDataBytes,
                                                                   .numBytes = numAuthorizationDataBytes } },
                                    (bool) writeContext->value.unsignedIntValue,
                                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                            writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_UInt8: {
                        if ((writeContext->type == kHAPIPWriteValueType_UInt) &&
                            (writeContext->value.unsignedIntValue <= UINT8_MAX)) {
                            err = HAPUInt8CharacteristicHandleWrite(
                                    HAPNonnull(session->server),
                                    &(const HAPUInt8CharacteristicWriteRequest) {
                                            .transportType = kHAPTransportType_IP,
                                            .session = &session->securitySession._.hap,
                                            .characteristic = (const HAPUInt8Characteristic*) baseCharacteristic,
                                            .service = service,
                                            .accessory = accessory,
                                            .remote = writeContext->remote,
                                            .authorizationData = { .bytes = authorizationDataBytes,
                                                                   .numBytes = numAuthorizationDataBytes } },
                                    (uint8_t) writeContext->value.unsignedIntValue,
                                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                            writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_UInt16: {
                        if ((writeContext->type == kHAPIPWriteValueType_UInt) &&
                            (writeContext->value.unsignedIntValue <= UINT16_MAX)) {
                            err = HAPUInt16CharacteristicHandleWrite(
                                    HAPNonnull(session->server),
                                    &(const HAPUInt16CharacteristicWriteRequest) {
                                            .transportType = kHAPTransportType_IP,
                                            .session = &session->securitySession._.hap,
                                            .characteristic = (const HAPUInt16Characteristic*) baseCharacteristic,
                                            .service = service,
                                            .accessory = accessory,
                                            .remote = writeContext->remote,
                                            .authorizationData = { .bytes = authorizationDataBytes,
                                                                   .numBytes = numAuthorizationDataBytes } },
                                    (uint16_t) writeContext->value.unsignedIntValue,
                                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                            writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_UInt32: {
                        if ((writeContext->type == kHAPIPWriteValueType_UInt) &&
                            (writeContext->value.unsignedIntValue <= UINT32_MAX)) {
                            err = HAPUInt32CharacteristicHandleWrite(
                                    HAPNonnull(session->server),
                                    &(const HAPUInt32CharacteristicWriteRequest) {
                                            .transportType = kHAPTransportType_IP,
                                            .session = &session->securitySession._.hap,
                                            .characteristic = (const HAPUInt32Characteristic*) baseCharacteristic,
                                            .service = service,
                                            .accessory = accessory,
                                            .remote = writeContext->remote,
                                            .authorizationData = { .bytes = authorizationDataBytes,
                                                                   .numBytes = numAuthorizationDataBytes } },
                                    (uint32_t) writeContext->value.unsignedIntValue,
                                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                            writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_UInt64: {
                        if (writeContext->type == kHAPIPWriteValueType_UInt) {
                            err = HAPUInt64CharacteristicHandleWrite(
                                    HAPNonnull(session->server),
                                    &(const HAPUInt64CharacteristicWriteRequest) {
                                            .transportType = kHAPTransportType_IP,
                                            .session = &session->securitySession._.hap,
                                            .characteristic = (const HAPUInt64Characteristic*) baseCharacteristic,
                                            .service = service,
                                            .accessory = accessory,
                                            .remote = writeContext->remote,
                                            .authorizationData = { .bytes = authorizationDataBytes,
                                                                   .numBytes = numAuthorizationDataBytes } },
                                    writeContext->value.unsignedIntValue,
                                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                            writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_Int: {
                        if ((writeContext->type == kHAPIPWriteValueType_UInt) &&
                            (writeContext->value.unsignedIntValue <= INT32_MAX)) {
                            writeContext->value.intValue = (int32_t) writeContext->value.unsignedIntValue;
                            writeContext->type = kHAPIPWriteValueType_Int;
                        }
                        if (writeContext->type == kHAPIPWriteValueType_Int) {
                            err = HAPIntCharacteristicHandleWrite(
                                    HAPNonnull(session->server),
                                    &(const HAPIntCharacteristicWriteRequest) {
                                            .transportType = kHAPTransportType_IP,
                                            .session = &session->securitySession._.hap,
                                            .characteristic = (const HAPIntCharacteristic*) baseCharacteristic,
                                            .service = service,
                                            .accessory = accessory,
                                            .remote = writeContext->remote,
                                            .authorizationData = { .bytes = authorizationDataBytes,
                                                                   .numBytes = numAuthorizationDataBytes } },
                                    writeContext->value.intValue,
                                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                            writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_Float: {
                        if ((writeContext->type == kHAPIPWriteValueType_Int) &&
                            (writeContext->value.intValue >= -FLT_MAX) && (writeContext->value.intValue <= FLT_MAX)) {
                            writeContext->value.floatValue = (float) writeContext->value.intValue;
                            writeContext->type = kHAPIPWriteValueType_Float;
                        }
                        if ((writeContext->type == kHAPIPWriteValueType_UInt) &&
                            (writeContext->value.unsignedIntValue <= FLT_MAX)) {
                            writeContext->value.floatValue = (float) writeContext->value.unsignedIntValue;
                            writeContext->type = kHAPIPWriteValueType_Float;
                        }
                        if (writeContext->type == kHAPIPWriteValueType_Float) {
                            err = HAPFloatCharacteristicHandleWrite(
                                    HAPNonnull(session->server),
                                    &(const HAPFloatCharacteristicWriteRequest) {
                                            .transportType = kHAPTransportType_IP,
                                            .session = &session->securitySession._.hap,
                                            .characteristic = (const HAPFloatCharacteristic*) baseCharacteristic,
                                            .service = service,
                                            .accessory = accessory,
                                            .remote = writeContext->remote,
                                            .authorizationData = { .bytes = authorizationDataBytes,
                                                                   .numBytes = numAuthorizationDataBytes } },
                                    writeContext->value.floatValue,
                                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                            writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_String: {
                        if ((writeContext->type == kHAPIPWriteValueType_String) &&
                            (writeContext->value.stringValue.numBytes <= 256)) {
                            HAPAssert(writeContext->value.stringValue.bytes);
                            HAPAssert(dataBuffer->data);
                            HAPAssert(dataBuffer->position <= dataBuffer->limit);
                            HAPAssert(dataBuffer->limit <= dataBuffer->capacity);
                            if (writeContext->value.stringValue.numBytes >= dataBuffer->limit - dataBuffer->position) {
                                writeContext->status = kHAPIPAccessoryServerStatusCode_OutOfResources;
                            } else {
                                HAPRawBufferCopyBytes(
                                        &dataBuffer->data[dataBuffer->position],
                                        HAPNonnull(writeContext->value.stringValue.bytes),
                                        writeContext->value.stringValue.numBytes);
                                dataBuffer->data[dataBuffer->position + writeContext->value.stringValue.numBytes] =
                                        '\0';
                                err = HAPStringCharacteristicHandleWrite(
                                        HAPNonnull(session->server),
                                        &(const HAPStringCharacteristicWriteRequest) {
                                                .transportType = kHAPTransportType_IP,
                                                .session = &session->securitySession._.hap,
                                                .characteristic = (const HAPStringCharacteristic*) baseCharacteristic,
                                                .service = service,
                                                .accessory = accessory,
                                                .remote = writeContext->remote,
                                                .authorizationData = { .bytes = authorizationDataBytes,
                                                                       .numBytes = numAuthorizationDataBytes } },
                                        &dataBuffer->data[dataBuffer->position],
                                        HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                                writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                            }
                        } else {
                            writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                        }
                    } break;
                    case kHAPCharacteristicFormat_TLV8: {
                        if (writeContext->type == kHAPIPWriteValueType_String) {
                            HAPAssert(writeContext->value.stringValue.bytes);
                            int r = util_base64_decode(
                                    writeContext->value.stringValue.bytes,
                                    writeContext->value.stringValue.numBytes,
                                    writeContext->value.stringValue.bytes,
                                    writeContext->value.stringValue.numBytes,
                                    &writeContext->value.stringValue.numBytes);
                            if (r == 0) {
                                HAPTLVReaderRef tlvReader;
                                HAPTLVReaderCreate(
                                        &tlvReader,
                                        writeContext->value.stringValue.bytes,
                                        writeContext->value.stringValue.numBytes);
                                err = HAPTLV8CharacteristicHandleWrite(
                                        HAPNonnull(session->server),
                                        &(const HAPTLV8CharacteristicWriteRequest) {
                                                .transportType = kHAPTransportType_IP,
                                                .session = &session->securitySession._.hap,
                                                .characteristic = (const HAPTLV8Characteristic*) baseCharacteristic,
                                                .service = service,
                                                .accessory = accessory,
                                                .remote = writeContext->remote,
                                                .authorizationData = { .bytes = authorizationDataBytes,
                                                                       .numBytes = numAuthorizationDataBytes } },
                                        &tlvReader,
                                        HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
                                writeContext->status = ConvertCharacteristicWriteErrorToStatusCode(err);
                            } else {
                                writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                            }
                        }
                    } break;
                }
                if (writeContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                    if (baseCharacteristic->properties.ip.supportsWriteResponse) {
                        HAPIPByteBuffer dataBufferSnapshot;
                        HAPRawBufferCopyBytes(&dataBufferSnapshot, dataBuffer, sizeof dataBufferSnapshot);
                        HAPIPReadContext readContext;
                        HAPRawBufferZero(&readContext, sizeof readContext);
                        readContext.aid = writeContext->aid;
                        readContext.iid = writeContext->iid;
                        handle_characteristic_read_request(
                                session,
                                characteristic,
                                service,
                                accessory,
                                (HAPIPReadContextRef*) &readContext,
                                dataBuffer);
                        writeContext->status = readContext.status;
                        if (writeContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                            if (writeContext->response) {
                                switch (baseCharacteristic->format) {
                                    case kHAPCharacteristicFormat_Bool:
                                    case kHAPCharacteristicFormat_UInt8:
                                    case kHAPCharacteristicFormat_UInt16:
                                    case kHAPCharacteristicFormat_UInt32:
                                    case kHAPCharacteristicFormat_UInt64: {
                                        writeContext->value.unsignedIntValue = readContext.value.unsignedIntValue;
                                    } break;
                                    case kHAPCharacteristicFormat_Int: {
                                        writeContext->value.intValue = readContext.value.intValue;
                                    } break;
                                    case kHAPCharacteristicFormat_Float: {
                                        writeContext->value.floatValue = readContext.value.floatValue;
                                    } break;
                                    case kHAPCharacteristicFormat_Data:
                                    case kHAPCharacteristicFormat_String:
                                    case kHAPCharacteristicFormat_TLV8: {
                                        writeContext->value.stringValue.bytes = readContext.value.stringValue.bytes;
                                        writeContext->value.stringValue.numBytes =
                                                readContext.value.stringValue.numBytes;
                                    } break;
                                }
                            } else {
                                // Ignore value of read operation and revert possible changes to data buffer.
                                HAPRawBufferCopyBytes(dataBuffer, &dataBufferSnapshot, sizeof *dataBuffer);
                            }
                        }
                    } else if (writeContext->response) {
                        writeContext->status = kHAPIPAccessoryServerStatusCode_ReadFromWriteOnlyCharacteristic;
                    }
                }
            }
        } else {
            writeContext->status = kHAPIPAccessoryServerStatusCode_WriteToReadOnlyCharacteristic;
        }
    }
}

/**
 * Handles a set of characteristic write requests.
 *
 * @param      session              IP session descriptor.
 * @param      contexts             Request contexts.
 * @param      numContexts          Length of @p contexts.
 * @param      dataBuffer           Buffer for values of type data, string or TLV8.
 * @param      timedWrite           Whether the request was a valid Execute Write Request or a regular Write Request.
 *
 * @return 0                        If all writes could be handled successfully.
 * @return -1                       Otherwise (Multi-Status).
 */
HAP_RESULT_USE_CHECK
static int handle_characteristic_write_requests(
        HAPIPSessionDescriptor* session,
        HAPIPWriteContextRef* contexts,
        size_t numContexts,
        HAPIPByteBuffer* dataBuffer,
        bool timedWrite) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));
    HAPPrecondition(contexts);
    HAPPrecondition(dataBuffer);

    int r = 0;

    for (size_t i = 0; i < numContexts; i++) {
        HAPIPWriteContext* writeContext = (HAPIPWriteContext*) &contexts[i];
        const HAPCharacteristic* characteristic;
        const HAPService* service;
        const HAPAccessory* accessory;
        get_db_ctx(session->server, writeContext->aid, writeContext->iid, &characteristic, &service, &accessory);
        if (characteristic) {
            HAPAssert(service);
            HAPAssert(accessory);
            server->ip.characteristicWriteRequestContext.ipSession = NULL;
            for (size_t j = 0; j < server->ip.storage->numSessions; j++) {
                HAPIPSession* ipSession = &server->ip.storage->sessions[j];
                HAPIPSessionDescriptor* t = (HAPIPSessionDescriptor*) &ipSession->descriptor;
                if (t->server && (t == session)) {
                    HAPAssert(!server->ip.characteristicWriteRequestContext.ipSession);
                    server->ip.characteristicWriteRequestContext.ipSession = ipSession;
                }
            }
            HAPAssert(server->ip.characteristicWriteRequestContext.ipSession);
            server->ip.characteristicWriteRequestContext.characteristic = characteristic;
            server->ip.characteristicWriteRequestContext.service = service;
            server->ip.characteristicWriteRequestContext.accessory = accessory;
            const HAPBaseCharacteristic* baseCharacteristic = characteristic;
            if ((writeContext->type != kHAPIPWriteValueType_None) &&
                baseCharacteristic->properties.requiresTimedWrite && !timedWrite) {
                // If the accessory receives a standard write request on a characteristic which requires timed write,
                // the accessory must respond with HAP status error code -70410 (HAPIPStatusErrorCodeInvalidWrite).
                // See HomeKit Accessory Protocol Specification R14
                // Section 6.7.2.4 Timed Write Procedures
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected write: Only timed writes are supported.");
                writeContext->status = kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
            } else {
                handle_characteristic_write_request(
                        session, characteristic, service, accessory, &contexts[i], dataBuffer);
            }
            server->ip.characteristicWriteRequestContext.ipSession = NULL;
            server->ip.characteristicWriteRequestContext.characteristic = NULL;
            server->ip.characteristicWriteRequestContext.service = NULL;
            server->ip.characteristicWriteRequestContext.accessory = NULL;
        } else {
            writeContext->status = kHAPIPAccessoryServerStatusCode_ResourceDoesNotExist;
        }
        if ((r == 0) && (writeContext->status != kHAPIPAccessoryServerStatusCode_Success)) {
            r = -1;
        }
        if ((r == 0) && writeContext->response) {
            r = -1;
        }
    }

    return r;
}

static void put_characteristics(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));

    HAPError err;
    int r;
    size_t i, contexts_count;
    bool pid_valid;
    uint64_t pid;
    HAPIPByteBuffer data_buffer;

    HAPAssert(session->inboundBuffer.data);
    HAPAssert(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPAssert(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPAssert(session->httpReaderPosition <= session->inboundBuffer.position);
    if (session->httpContentLength.isDefined) {
        HAPAssert(session->httpContentLength.value <= session->inboundBuffer.position - session->httpReaderPosition);
        err = HAPIPAccessoryProtocolGetCharacteristicWriteRequests(
                &session->inboundBuffer.data[session->httpReaderPosition],
                session->httpContentLength.value,
                server->ip.storage->writeContexts,
                server->ip.storage->numWriteContexts,
                &contexts_count,
                &pid_valid,
                &pid);
        if (!err) {
            if ((session->timedWriteExpirationTime && pid_valid &&
                 session->timedWriteExpirationTime < HAPPlatformClockGetCurrent()) ||
                (session->timedWriteExpirationTime && pid_valid && session->timedWritePID != pid) ||
                (!session->timedWriteExpirationTime && pid_valid)) {
                // If the accessory receives an Execute Write Request after the TTL has expired it must ignore the
                // request and respond with HAP status error code -70410 (HAPIPStatusErrorCodeInvalidWrite).
                // See HomeKit Accessory Protocol Specification R14
                // Section 6.7.2.4 Timed Write Procedures
                HAPLog(&logObject, "Rejecting expired Execute Write Request.");
                for (i = 0; i < contexts_count; i++) {
                    ((HAPIPWriteContext*) &server->ip.storage->writeContexts[i])->status =
                            kHAPIPAccessoryServerStatusCode_InvalidValueInWrite;
                }
                HAPAssert(i == contexts_count);
                write_characteristic_write_response(session, server->ip.storage->writeContexts, contexts_count);
            } else if (contexts_count == 0) {
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_NoContent);
            } else {
                data_buffer.data = server->ip.storage->scratchBuffer.bytes;
                data_buffer.capacity = server->ip.storage->scratchBuffer.numBytes;
                data_buffer.limit = server->ip.storage->scratchBuffer.numBytes;
                data_buffer.position = 0;
                HAPAssert(data_buffer.data);
                HAPAssert(data_buffer.position <= data_buffer.limit);
                HAPAssert(data_buffer.limit <= data_buffer.capacity);
                r = handle_characteristic_write_requests(
                        session, server->ip.storage->writeContexts, contexts_count, &data_buffer, pid_valid);
                if (r == 0) {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_NoContent);
                } else {
                    write_characteristic_write_response(session, server->ip.storage->writeContexts, contexts_count);
                }
            }
            // Reset timed write transaction.
            if (session->timedWriteExpirationTime && pid_valid) {
                session->timedWriteExpirationTime = 0;
                session->timedWritePID = 0;
            }
        } else if (err == kHAPError_OutOfResources) {
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
        } else {
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
        }
    } else {
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
    }
}

/**
 * Converts a characteristic read request error to the corresponding HAP status code.
 *
 * @param      error                Read request error.
 *
 * @return HAP read request status code.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 6-11 HAP Status Codes
 */
static int32_t ConvertCharacteristicReadErrorToStatusCode(HAPError error) {
    switch (error) {
        case kHAPError_None: {
            return kHAPIPAccessoryServerStatusCode_Success;
        }
        case kHAPError_Unknown: {
            return kHAPIPAccessoryServerStatusCode_UnableToPerformOperation;
        }
        case kHAPError_InvalidState: {
            return kHAPIPAccessoryServerStatusCode_UnableToPerformOperation;
        }
        case kHAPError_InvalidData: {
            HAPFatalError();
        }
        case kHAPError_OutOfResources: {
            return kHAPIPAccessoryServerStatusCode_OutOfResources;
        }
        case kHAPError_NotAuthorized: {
            return kHAPIPAccessoryServerStatusCode_InsufficientAuthorization;
        }
        case kHAPError_Busy: {
            return kHAPIPAccessoryServerStatusCode_ResourceIsBusy;
        }
    }
    HAPFatalError();
}

static void handle_characteristic_read_request(
        HAPIPSessionDescriptor* session,
        const HAPCharacteristic* chr_,
        const HAPService* svc,
        const HAPAccessory* acc,
        HAPIPReadContextRef* ctx,
        HAPIPByteBuffer* data_buffer) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));

    HAPError err;

    size_t n, sval_length;
    bool bool_val;
    int32_t int_val;
    uint8_t uint8_val;
    uint16_t uint16_val;
    uint32_t uint32_val;
    uint64_t uint64_val;
    float float_val;
    HAPTLVWriterRef tlv8_writer;
    HAPAssert(chr_);
    const HAPBaseCharacteristic* chr = chr_;
    HAPAssert(svc);
    HAPAssert(acc);
    HAPAssert(ctx);
    HAPAssert(data_buffer);
    HAPAssert(data_buffer->data);
    HAPAssert(data_buffer->position <= data_buffer->limit);
    HAPAssert(data_buffer->limit <= data_buffer->capacity);
    HAPIPReadContext* readContext = (HAPIPReadContext*) ctx;
    readContext->status = kHAPIPAccessoryServerStatusCode_Success;
    switch (chr->format) {
        case kHAPCharacteristicFormat_Data: {
            err = HAPDataCharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPDataCharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                .session = &session->securitySession._.hap,
                                                                .characteristic = (const HAPDataCharacteristic*) chr,
                                                                .service = svc,
                                                                .accessory = acc },
                    &data_buffer->data[data_buffer->position],
                    data_buffer->limit - data_buffer->position,
                    &sval_length,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                if (sval_length <= data_buffer->limit - data_buffer->position) {
                    util_base64_encode(
                            &data_buffer->data[data_buffer->position],
                            sval_length,
                            &data_buffer->data[data_buffer->position],
                            data_buffer->limit - data_buffer->position,
                            &sval_length);
                    if (sval_length < data_buffer->limit - data_buffer->position) {
                        data_buffer->data[data_buffer->position + sval_length] = 0;
                        readContext->value.stringValue.bytes = &data_buffer->data[data_buffer->position];
                        readContext->value.stringValue.numBytes = sval_length;
                        data_buffer->position += sval_length + 1;
                        HAPAssert(data_buffer->position <= data_buffer->limit);
                        HAPAssert(data_buffer->limit <= data_buffer->capacity);
                    } else {
                        readContext->status = kHAPIPAccessoryServerStatusCode_OutOfResources;
                    }
                } else {
                    readContext->status = kHAPIPAccessoryServerStatusCode_OutOfResources;
                }
            }
        } break;
        case kHAPCharacteristicFormat_Bool: {
            err = HAPBoolCharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPBoolCharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                .session = &session->securitySession._.hap,
                                                                .characteristic = (const HAPBoolCharacteristic*) chr,
                                                                .service = svc,
                                                                .accessory = acc },
                    &bool_val,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                readContext->value.unsignedIntValue = bool_val ? 1 : 0;
            }
        } break;
        case kHAPCharacteristicFormat_UInt8: {
            err = HAPUInt8CharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPUInt8CharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                 .session = &session->securitySession._.hap,
                                                                 .characteristic = (const HAPUInt8Characteristic*) chr,
                                                                 .service = svc,
                                                                 .accessory = acc },
                    &uint8_val,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                readContext->value.unsignedIntValue = uint8_val;
            }
        } break;
        case kHAPCharacteristicFormat_UInt16: {
            err = HAPUInt16CharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPUInt16CharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                  .session = &session->securitySession._.hap,
                                                                  .characteristic =
                                                                          (const HAPUInt16Characteristic*) chr,
                                                                  .service = svc,
                                                                  .accessory = acc },
                    &uint16_val,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                readContext->value.unsignedIntValue = uint16_val;
            }
        } break;
        case kHAPCharacteristicFormat_UInt32: {
            err = HAPUInt32CharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPUInt32CharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                  .session = &session->securitySession._.hap,
                                                                  .characteristic =
                                                                          (const HAPUInt32Characteristic*) chr,
                                                                  .service = svc,
                                                                  .accessory = acc },
                    &uint32_val,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                readContext->value.unsignedIntValue = uint32_val;
            }
        } break;
        case kHAPCharacteristicFormat_UInt64: {
            err = HAPUInt64CharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPUInt64CharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                  .session = &session->securitySession._.hap,
                                                                  .characteristic =
                                                                          (const HAPUInt64Characteristic*) chr,
                                                                  .service = svc,
                                                                  .accessory = acc },
                    &uint64_val,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                readContext->value.unsignedIntValue = uint64_val;
            }
        } break;
        case kHAPCharacteristicFormat_Int: {
            err = HAPIntCharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPIntCharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                               .session = &session->securitySession._.hap,
                                                               .characteristic = (const HAPIntCharacteristic*) chr,
                                                               .service = svc,
                                                               .accessory = acc },
                    &int_val,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                readContext->value.intValue = int_val;
            }
        } break;
        case kHAPCharacteristicFormat_Float: {
            err = HAPFloatCharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPFloatCharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                 .session = &session->securitySession._.hap,
                                                                 .characteristic = (const HAPFloatCharacteristic*) chr,
                                                                 .service = svc,
                                                                 .accessory = acc },
                    &float_val,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                readContext->value.floatValue = float_val;
            }
        } break;
        case kHAPCharacteristicFormat_String: {
            err = HAPStringCharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPStringCharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                  .session = &session->securitySession._.hap,
                                                                  .characteristic =
                                                                          (const HAPStringCharacteristic*) chr,
                                                                  .service = svc,
                                                                  .accessory = acc },
                    &data_buffer->data[data_buffer->position],
                    data_buffer->limit - data_buffer->position,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                sval_length = HAPStringGetNumBytes(&data_buffer->data[data_buffer->position]);
                if (sval_length < data_buffer->limit - data_buffer->position) {
                    data_buffer->data[data_buffer->position + sval_length] = 0;
                    readContext->value.stringValue.bytes = &data_buffer->data[data_buffer->position];
                    readContext->value.stringValue.numBytes = sval_length;
                    data_buffer->position += sval_length + 1;
                    HAPAssert(data_buffer->position <= data_buffer->limit);
                    HAPAssert(data_buffer->limit <= data_buffer->capacity);
                } else {
                    readContext->status = kHAPIPAccessoryServerStatusCode_OutOfResources;
                }
            }
        } break;
        case kHAPCharacteristicFormat_TLV8: {
            n = data_buffer->limit - data_buffer->position;
            HAPTLVWriterCreate(&tlv8_writer, &data_buffer->data[data_buffer->position], n);
            err = HAPTLV8CharacteristicHandleRead(
                    HAPNonnull(session->server),
                    &(const HAPTLV8CharacteristicReadRequest) { .transportType = kHAPTransportType_IP,
                                                                .session = &session->securitySession._.hap,
                                                                .characteristic = (const HAPTLV8Characteristic*) chr,
                                                                .service = svc,
                                                                .accessory = acc },
                    &tlv8_writer,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            readContext->status = ConvertCharacteristicReadErrorToStatusCode(err);
            if (readContext->status == kHAPIPAccessoryServerStatusCode_Success) {
                if (((HAPTLVWriter*) &tlv8_writer)->numBytes <= data_buffer->limit - data_buffer->position) {
                    util_base64_encode(
                            &data_buffer->data[data_buffer->position],
                            ((HAPTLVWriter*) &tlv8_writer)->numBytes,
                            &data_buffer->data[data_buffer->position],
                            data_buffer->limit - data_buffer->position,
                            &sval_length);
                    if (sval_length < data_buffer->limit - data_buffer->position) {
                        data_buffer->data[data_buffer->position + sval_length] = 0;
                        readContext->value.stringValue.bytes = &data_buffer->data[data_buffer->position];
                        readContext->value.stringValue.numBytes = sval_length;
                        data_buffer->position += sval_length + 1;
                        HAPAssert(data_buffer->position <= data_buffer->limit);
                        HAPAssert(data_buffer->limit <= data_buffer->capacity);
                    } else {
                        readContext->status = kHAPIPAccessoryServerStatusCode_OutOfResources;
                    }
                } else {
                    readContext->status = kHAPIPAccessoryServerStatusCode_OutOfResources;
                }
            }
        } break;
    }
}

HAP_RESULT_USE_CHECK
static int handle_characteristic_read_requests(
        HAPIPSessionDescriptor* session,
        HAPIPSessionContext session_context,
        HAPIPReadContextRef* contexts,
        size_t contexts_count,
        HAPIPByteBuffer* data_buffer) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));

    int r;
    size_t i, j;
    const HAPCharacteristic* c;
    const HAPService* svc;
    const HAPAccessory* acc;
    HAPAssert(contexts);
    r = 0;
    for (i = 0; i < contexts_count; i++) {
        HAPIPReadContext* readContext = (HAPIPReadContext*) &contexts[i];

        get_db_ctx(session->server, readContext->aid, readContext->iid, &c, &svc, &acc);
        if (c) {
            const HAPBaseCharacteristic* chr = c;
            HAPAssert(chr->iid == readContext->iid);
            HAPAssert(session->numEventNotifications <= session->maxEventNotifications);
            j = 0;
            while ((j < session->numEventNotifications) &&
                   ((((HAPIPEventNotification*) &session->eventNotifications[j])->aid != readContext->aid) ||
                    (((HAPIPEventNotification*) &session->eventNotifications[j])->iid != readContext->iid))) {
                j++;
            }
            HAPAssert(
                    (j == session->numEventNotifications) ||
                    ((j < session->numEventNotifications) &&
                     (((HAPIPEventNotification*) &session->eventNotifications[j])->aid == readContext->aid) &&
                     (((HAPIPEventNotification*) &session->eventNotifications[j])->iid == readContext->iid)));
            readContext->ev = j < session->numEventNotifications;
            if (!HAPCharacteristicReadRequiresAdminPermissions(chr) ||
                HAPSessionControllerIsAdmin(&session->securitySession._.hap)) {
                if (chr->properties.readable) {
                    if ((session_context != kHAPIPSessionContext_EventNotification) &&
                        HAPUUIDAreEqual(chr->characteristicType, &kHAPCharacteristicType_ProgrammableSwitchEvent)) {
                        // A read of this characteristic must always return a null value for IP accessories.
                        // See HomeKit Accessory Protocol Specification R14
                        // Section 9.75 Programmable Switch Event
                        readContext->status = kHAPIPAccessoryServerStatusCode_Success;
                        readContext->value.unsignedIntValue = 0;
                    } else if (
                            (session_context == kHAPIPSessionContext_GetAccessories) &&
                            chr->properties.ip.controlPoint) {
                        readContext->status = kHAPIPAccessoryServerStatusCode_UnableToPerformOperation;
                    } else {
                        handle_characteristic_read_request(session, chr, svc, acc, &contexts[i], data_buffer);
                    }
                } else {
                    readContext->status = kHAPIPAccessoryServerStatusCode_ReadFromWriteOnlyCharacteristic;
                }
            } else {
                readContext->status = kHAPIPAccessoryServerStatusCode_InsufficientPrivileges;
            }
        } else {
            readContext->status = kHAPIPAccessoryServerStatusCode_ResourceDoesNotExist;
        }
        if ((r == 0) && (readContext->status != kHAPIPAccessoryServerStatusCode_Success)) {
            r = -1;
        }
    }
    HAPAssert(i == contexts_count);
    return r;
}

static void get_characteristics(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));

    HAPError err;

    int r;
    size_t contexts_count, content_length, mark;
    HAPIPReadRequestParameters parameters;
    HAPIPByteBuffer data_buffer;

    HAPAssert(
            (session->httpURI.numBytes >= 16) &&
            HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/characteristics", 16));
    if ((session->httpURI.numBytes >= 17) && (session->httpURI.bytes[16] == '?')) {
        err = HAPIPAccessoryProtocolGetCharacteristicReadRequests(
                &session->httpURI.bytes[17],
                session->httpURI.numBytes - 17,
                server->ip.storage->readContexts,
                server->ip.storage->numReadContexts,
                &contexts_count,
                &parameters);
        if (!err) {
            if (contexts_count == 0) {
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_NoContent);
            } else {
                data_buffer.data = server->ip.storage->scratchBuffer.bytes;
                data_buffer.capacity = server->ip.storage->scratchBuffer.numBytes;
                data_buffer.limit = server->ip.storage->scratchBuffer.numBytes;
                data_buffer.position = 0;
                HAPAssert(data_buffer.data);
                HAPAssert(data_buffer.position <= data_buffer.limit);
                HAPAssert(data_buffer.limit <= data_buffer.capacity);
                r = handle_characteristic_read_requests(
                        session,
                        kHAPIPSessionContext_GetCharacteristics,
                        server->ip.storage->readContexts,
                        contexts_count,
                        &data_buffer);
                content_length = HAPIPAccessoryProtocolGetNumCharacteristicReadResponseBytes(
                        HAPNonnull(session->server), server->ip.storage->readContexts, contexts_count, &parameters);
                HAPAssert(session->outboundBuffer.data);
                HAPAssert(session->outboundBuffer.position <= session->outboundBuffer.limit);
                HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);
                mark = session->outboundBuffer.position;
                if (r == 0) {
                    err = HAPIPByteBufferAppendStringWithFormat(&session->outboundBuffer, "HTTP/1.1 200 OK\r\n");
                } else {
                    err = HAPIPByteBufferAppendStringWithFormat(
                            &session->outboundBuffer, "HTTP/1.1 207 Multi-Status\r\n");
                }
                HAPAssert(!err);
                HAP_DIAGNOSTIC_IGNORED_ICCARM(Pa084)
                if (content_length <= UINT32_MAX) {
                    err = HAPIPByteBufferAppendStringWithFormat(
                            &session->outboundBuffer,
                            "Content-Type: application/hap+json\r\n"
                            "Content-Length: %lu\r\n\r\n",
                            (unsigned long) content_length);
                    HAPAssert(!err);
                    if (content_length <= session->outboundBuffer.limit - session->outboundBuffer.position) {
                        mark = session->outboundBuffer.position;
                        err = HAPIPAccessoryProtocolGetCharacteristicReadResponseBytes(
                                HAPNonnull(session->server),
                                server->ip.storage->readContexts,
                                contexts_count,
                                &parameters,
                                &session->outboundBuffer);
                        HAPAssert(!err && (session->outboundBuffer.position - mark == content_length));
                    } else {
                        HAPLog(&logObject, "Out of resources (outbound buffer too small).");
                        session->outboundBuffer.position = mark;
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
                    }
                } else {
                    HAPLog(&logObject, "Content length exceeding UINT32_MAX.");
                    session->outboundBuffer.position = mark;
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
                }
                HAP_DIAGNOSTIC_RESTORE_ICCARM(Pa084)
            }
        } else if (err == kHAPError_OutOfResources) {
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
        } else {
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
        }
    } else {
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
    }
}

static void handle_accessory_serialization(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));

    HAPError err;

    HAPAssert(session->outboundBuffer.data);
    HAPAssert(session->outboundBuffer.capacity);

    if (session->accessorySerializationIsInProgress) {
        HAPAssert(session->outboundBuffer.position == session->outboundBuffer.limit);
        if (session->securitySession.isSecured) {
            HAPAssert(session->outboundBuffer.limit <= session->outboundBufferMark);
            HAPAssert(session->outboundBufferMark <= session->outboundBuffer.capacity);
            HAPRawBufferCopyBytes(
                    &session->outboundBuffer.data[0],
                    &session->outboundBuffer.data[session->outboundBuffer.limit],
                    session->outboundBufferMark - session->outboundBuffer.limit);
            session->outboundBuffer.position = session->outboundBufferMark - session->outboundBuffer.limit;
            session->outboundBuffer.limit = session->outboundBuffer.capacity;
            session->outboundBufferMark = 0;
        } else {
            HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);
            session->outboundBuffer.position = 0;
            session->outboundBuffer.limit = session->outboundBuffer.capacity;
        }
    }

    HAPAssert(session->outboundBuffer.position <= session->outboundBuffer.limit);
    HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);

    if ((session->outboundBuffer.position < session->outboundBuffer.limit) &&
        (session->outboundBuffer.position < kHAPIPSecurityProtocol_MaxFrameBytes) &&
        !HAPIPAccessorySerializationIsComplete(&session->accessorySerializationContext)) {
        size_t numBytesSerialized;
        size_t maxBytes = session->outboundBuffer.limit - session->outboundBuffer.position;
        size_t minBytes =
                kHAPIPSecurityProtocol_MaxFrameBytes < maxBytes ? kHAPIPSecurityProtocol_MaxFrameBytes : maxBytes;
        err = HAPIPAccessorySerializeReadResponse(
                &session->accessorySerializationContext,
                HAPNonnull(session->server),
                (HAPIPSessionDescriptorRef*) session,
                &session->outboundBuffer.data[session->outboundBuffer.position],
                minBytes,
                maxBytes,
                &numBytesSerialized);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPLogError(&logObject, "Invalid configuration (outbound buffer too small).");
            HAPFatalError();
        }
        HAPAssert(numBytesSerialized > 0);
        HAPAssert(numBytesSerialized <= maxBytes);
        HAPAssert(
                (numBytesSerialized >= minBytes) ||
                HAPIPAccessorySerializationIsComplete(&session->accessorySerializationContext));

        // maxProtocolBytes = max(8, size_t represented in HEX + '\r' + '\n' + '\0')
        char protocolBytes[HAPMax(8, sizeof(size_t) * 2 + 2 + 1)];

        err = HAPStringWithFormat(protocolBytes, sizeof protocolBytes, "%zX\r\n", numBytesSerialized);
        HAPAssert(!err);
        size_t numProtocolBytes = HAPStringGetNumBytes(protocolBytes);

        if (numProtocolBytes > session->outboundBuffer.limit - session->outboundBuffer.position) {
            HAPLogError(&logObject, "Invalid configuration (outbound buffer too small).");
            HAPFatalError();
        }
        if (numBytesSerialized > session->outboundBuffer.limit - session->outboundBuffer.position - numProtocolBytes) {
            HAPLogError(&logObject, "Invalid configuration (outbound buffer too small).");
            HAPFatalError();
        }

        HAPRawBufferCopyBytes(
                &session->outboundBuffer.data[session->outboundBuffer.position + numProtocolBytes],
                &session->outboundBuffer.data[session->outboundBuffer.position],
                numBytesSerialized);
        HAPRawBufferCopyBytes(
                &session->outboundBuffer.data[session->outboundBuffer.position], protocolBytes, numProtocolBytes);
        session->outboundBuffer.position += numProtocolBytes + numBytesSerialized;

        if (HAPIPAccessorySerializationIsComplete(&session->accessorySerializationContext)) {
            err = HAPStringWithFormat(protocolBytes, sizeof protocolBytes, "\r\n0\r\n\r\n");
        } else {
            err = HAPStringWithFormat(protocolBytes, sizeof protocolBytes, "\r\n");
        }
        HAPAssert(!err);
        numProtocolBytes = HAPStringGetNumBytes(protocolBytes);

        if (numProtocolBytes > session->outboundBuffer.limit - session->outboundBuffer.position) {
            HAPLogError(&logObject, "Invalid configuration (outbound buffer too small).");
            HAPFatalError();
        }

        HAPRawBufferCopyBytes(
                &session->outboundBuffer.data[session->outboundBuffer.position], protocolBytes, numProtocolBytes);
        session->outboundBuffer.position += numProtocolBytes;
    }

    if (session->outboundBuffer.position > 0) {
        HAPIPByteBufferFlip(&session->outboundBuffer);

        if (session->securitySession.isSecured) {
            size_t numFrameBytes = kHAPIPSecurityProtocol_MaxFrameBytes <
                                                   session->outboundBuffer.limit - session->outboundBuffer.position ?
                                           kHAPIPSecurityProtocol_MaxFrameBytes :
                                           session->outboundBuffer.limit - session->outboundBuffer.position;

            HAPLogBufferDebug(
                    &logObject,
                    &session->outboundBuffer.data[session->outboundBuffer.position],
                    numFrameBytes,
                    "session:%p:<",
                    (const void*) session);

            size_t numUnencryptedBytes =
                    session->outboundBuffer.limit - session->outboundBuffer.position - numFrameBytes;

            size_t numEncryptedBytes = HAPIPSecurityProtocolGetNumEncryptedBytes(numFrameBytes);
            if (numEncryptedBytes >
                session->outboundBuffer.capacity - session->outboundBuffer.position - numUnencryptedBytes) {
                HAPLogError(&logObject, "Invalid configuration (outbound buffer too small).");
                HAPFatalError();
            }

            HAPRawBufferCopyBytes(
                    &session->outboundBuffer.data[session->outboundBuffer.position + numEncryptedBytes],
                    &session->outboundBuffer.data[session->outboundBuffer.position + numFrameBytes],
                    numUnencryptedBytes);

            session->outboundBuffer.limit = session->outboundBuffer.position + numFrameBytes;

            HAPIPSecurityProtocolEncryptData(
                    HAPNonnull(session->server), &session->securitySession._.hap, &session->outboundBuffer);
            HAPAssert(numEncryptedBytes == session->outboundBuffer.limit - session->outboundBuffer.position);

            session->outboundBufferMark = session->outboundBuffer.limit + numUnencryptedBytes;
        } else {
            HAPLogBufferDebug(
                    &logObject,
                    &session->outboundBuffer.data[session->outboundBuffer.position],
                    session->outboundBuffer.limit - session->outboundBuffer.position,
                    "session:%p:<",
                    (const void*) session);
        }

        session->state = kHAPIPSessionState_Writing;

        session->accessorySerializationIsInProgress = true;
    } else {
        session->accessorySerializationIsInProgress = false;

        session->state = kHAPIPSessionState_Reading;
        prepare_reading_request(session);
        if (session->inboundBuffer.position != 0) {
            handle_input(session);
        }
    }
}

static void get_accessories(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));
    HAPPrecondition(!session->accessorySerializationIsInProgress);

    HAPError err;

    HAPAssert(session->outboundBuffer.data);
    HAPAssert(session->outboundBuffer.position <= session->outboundBuffer.limit);
    HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);
    err = HAPIPByteBufferAppendStringWithFormat(
            &session->outboundBuffer,
            "HTTP/1.1 200 OK\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: application/hap+json\r\n\r\n");
    HAPAssert(!err);

    HAPIPAccessoryCreateSerializationContext(&session->accessorySerializationContext);
    handle_accessory_serialization(session);
}

static void handle_pairing_data(
        HAPIPSessionDescriptor* session,
        HAPError (*write_hap_pairing_data)(
                HAPAccessoryServerRef* p_acc,
                HAPSessionRef* p_sess,
                HAPTLVReaderRef* p_reader),
        HAPError (*read_hap_pairing_data)(
                HAPAccessoryServerRef* p_acc,
                HAPSessionRef* p_sess,
                HAPTLVWriterRef* p_writer)) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);

    HAPError err;

    int r;
    bool pairing_status;
    uint8_t* p_tlv8_buffer;
    size_t tlv8_length, mark;
    HAPTLVReaderOptions tlv8_reader_init;
    HAPTLVReaderRef tlv8_reader;
    HAPTLVWriterRef tlv8_writer;

    char* scratchBuffer = server->ip.storage->scratchBuffer.bytes;
    size_t maxScratchBufferBytes = server->ip.storage->scratchBuffer.numBytes;

    HAPAssert(session->inboundBuffer.data);
    HAPAssert(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPAssert(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPAssert(session->httpReaderPosition <= session->inboundBuffer.position);
    HAPAssert(write_hap_pairing_data);
    HAPAssert(read_hap_pairing_data);
    pairing_status = HAPAccessoryServerIsPaired(HAPNonnull(session->server));
    if (session->httpContentLength.isDefined) {
        HAPAssert(session->httpContentLength.value <= session->inboundBuffer.position - session->httpReaderPosition);
        if (session->httpContentLength.value <= maxScratchBufferBytes) {
            HAPRawBufferCopyBytes(
                    scratchBuffer,
                    &session->inboundBuffer.data[session->httpReaderPosition],
                    session->httpContentLength.value);
            tlv8_reader_init.bytes = scratchBuffer;
            tlv8_reader_init.numBytes = session->httpContentLength.value;
            tlv8_reader_init.maxBytes = maxScratchBufferBytes;
            HAPTLVReaderCreateWithOptions(&tlv8_reader, &tlv8_reader_init);
            r = write_hap_pairing_data(HAPNonnull(session->server), &session->securitySession._.hap, &tlv8_reader);
            if (r == 0) {
                HAPTLVWriterCreate(&tlv8_writer, scratchBuffer, maxScratchBufferBytes);
                r = read_hap_pairing_data(HAPNonnull(session->server), &session->securitySession._.hap, &tlv8_writer);
                if (r == 0) {
                    HAPTLVWriterGetBuffer(&tlv8_writer, (void*) &p_tlv8_buffer, &tlv8_length);
                    if (HAPAccessoryServerIsPaired(HAPNonnull(session->server)) != pairing_status) {
                        HAPIPServiceDiscoverySetHAPService(HAPNonnull(session->server));
                    }
                    HAPAssert(session->outboundBuffer.data);
                    HAPAssert(session->outboundBuffer.position <= session->outboundBuffer.limit);
                    HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);
                    mark = session->outboundBuffer.position;
                    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pa084)
                    if (tlv8_length <= UINT32_MAX) {
                        err = HAPIPByteBufferAppendStringWithFormat(
                                &session->outboundBuffer,
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: application/pairing+tlv8\r\n"
                                "Content-Length: %lu\r\n\r\n",
                                (unsigned long) tlv8_length);
                        HAPAssert(!err);
                        if (tlv8_length <= session->outboundBuffer.limit - session->outboundBuffer.position) {
                            HAPRawBufferCopyBytes(
                                    &session->outboundBuffer.data[session->outboundBuffer.position],
                                    p_tlv8_buffer,
                                    tlv8_length);
                            session->outboundBuffer.position += tlv8_length;
                            for (size_t i = 0; i < server->ip.storage->numSessions; i++) {
                                HAPIPSession* ipSession = &server->ip.storage->sessions[i];
                                HAPIPSessionDescriptor* t = (HAPIPSessionDescriptor*) &ipSession->descriptor;
                                if (!t->server) {
                                    continue;
                                }

                                // Other sessions whose pairing has been removed during the pairing session
                                // need to be closed as soon as possible.
                                if (t != session && t->state == kHAPIPSessionState_Reading &&
                                    t->securitySession.type == kHAPIPSecuritySessionType_HAP &&
                                    t->securitySession.isSecured && !HAPSessionIsSecured(&t->securitySession._.hap)) {
                                    HAPLogInfo(&logObject, "Closing other session whose pairing has been removed.");
                                    CloseSession(t);
                                }
                            }
                        } else {
                            HAPLog(&logObject, "Invalid configuration (outbound buffer too small).");
                            session->outboundBuffer.position = mark;
                            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_InternalServerError);
                        }
                        HAP_DIAGNOSTIC_RESTORE_ICCARM(Pa084)
                    } else {
                        HAPLog(&logObject, "Content length exceeding UINT32_MAX.");
                        session->outboundBuffer.position = mark;
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
                    }
                } else {
                    log_result(
                            kHAPLogType_Error,
                            "error:Function 'read_hap_pairing_data' failed.",
                            r,
                            __func__,
                            HAP_FILE,
                            __LINE__);
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_InternalServerError);
                }
            } else {
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
            }
        } else {
            HAPLog(&logObject, "Invalid configuration (inbound buffer too small).");
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_InternalServerError);
        }
    } else {
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
    }
}

/**
 * Handles a POST request on the /secure-message endpoint.
 *
 * - Session has already been validated to be secured.
 *
 * @param      session              IP session descriptor.
 */
static void handle_secure_message(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(server->primaryAccessory);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(session->inboundBuffer.data);
    HAPPrecondition(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPPrecondition(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPPrecondition(session->httpReaderPosition <= session->inboundBuffer.position);

    HAPError err;

    // Validate request.
    // Requests use the HAP PDU format.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.3 HAP PDU Format
    if (session->httpContentType != kHAPIPAccessoryServerContentType_Application_OctetStream) {
        HAPLog(&logObject, "Received unexpected Content-Type in /secure-message request.");
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
        return;
    }
    if (!session->httpContentLength.isDefined) {
        HAPLog(&logObject, "Received malformed /secure-message request (no content length).");
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
        return;
    }
    HAPAssert(session->httpContentLength.value <= session->inboundBuffer.position - session->httpReaderPosition);
    uint8_t* requestBytes = (uint8_t*) &session->inboundBuffer.data[session->httpReaderPosition];
    size_t numRequestBytes = session->httpContentLength.value;
    if (numRequestBytes < 5) {
        HAPLog(&logObject, "Received too short /secure-message request.");
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
        return;
    }
    if (requestBytes[0] != ((0 << 7) | (0 << 4) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0))) {
        HAPLog(&logObject, "Received malformed /secure-message request (control field: 0x%02x).", requestBytes[0]);
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
        return;
    }
    uint8_t opcode = requestBytes[1];
    uint8_t tid = (uint8_t) requestBytes[2];
    uint16_t iid = HAPReadLittleUInt16(&requestBytes[3]);
    HAPTLVReaderRef requestBodyReader;
    if (numRequestBytes <= 5) {
        HAPAssert(numRequestBytes == 5);
        HAPTLVReaderCreate(&requestBodyReader, NULL, 0);
    } else {
        if (numRequestBytes < 7) {
            HAPLog(&logObject, "Received malformed /secure-message request (malformed body length).");
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
            return;
        }
        uint16_t numRequestBodyBytes = HAPReadLittleUInt16(&requestBytes[5]);
        if (numRequestBytes - 7 != numRequestBodyBytes) {
            HAPLog(&logObject, "Received malformed /secure-message request (incorrect body length).");
            write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
            return;
        }
        HAPTLVReaderCreate(&requestBodyReader, &requestBytes[7], numRequestBodyBytes);
    }

    // Response variables.
    HAPBLEPDUStatus status;
    void* _Nullable responseBodyBytes = NULL;
    size_t numResponseBodyBytes = 0;

    // Validate opcode.
    if (!HAPPDUIsValidOpcode(opcode)) {
        // If an accessory receives a HAP PDU with an opcode that it does not support it shall reject the PDU and
        // respond with a status code Unsupported PDU in its HAP response.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.3.3.2 HAP Request Format
        HAPLogAccessory(
                &logObject,
                server->primaryAccessory,
                "Rejected /secure-message request with unsupported opcode: 0x%02x.",
                opcode);
        status = kHAPBLEPDUStatus_UnsupportedPDU;
        goto SendResponse;
    }

    // Validate iid.
    // For IP accessories instance ID in the request shall be set to 0.
    // See HomeKit Accessory Protocol Specification R14
    // Section 5.15 Software Authentication Procedure
    if (iid) {
        HAPLogAccessory(
                &logObject,
                server->primaryAccessory,
                "Request's IID [00000000%08X] does not match the addressed IID.",
                iid);
        status = kHAPBLEPDUStatus_InvalidInstanceID;
        goto SendResponse;
    }

#define DestroyRequestBodyAndCreateResponseBodyWriter(responseWriter) \
    do { \
        size_t numBytes = server->ip.storage->scratchBuffer.numBytes; \
        if (numBytes > UINT16_MAX) { \
            /* Maximum for HAP-BLE PDU. */ \
            numBytes = UINT16_MAX; \
        } \
        HAPTLVWriterCreate(responseWriter, server->ip.storage->scratchBuffer.bytes, numBytes); \
    } while (0)

    // Handle request.
    HAPAssert(sizeof opcode == sizeof(HAPPDUOpcode));
    switch ((HAPPDUOpcode) opcode) {
        case kHAPPDUOpcode_ServiceSignatureRead:
        case kHAPPDUOpcode_CharacteristicSignatureRead:
        case kHAPPDUOpcode_CharacteristicConfiguration:
        case kHAPPDUOpcode_ProtocolConfiguration:
        case kHAPPDUOpcode_CharacteristicTimedWrite:
        case kHAPPDUOpcode_CharacteristicExecuteWrite:
        case kHAPPDUOpcode_CharacteristicWrite:
        case kHAPPDUOpcode_CharacteristicRead: {
            HAPLogAccessory(
                    &logObject,
                    server->primaryAccessory,
                    "Rejected /secure-message request with opcode that is not supported by IP: 0x%02x.",
                    opcode);
            status = kHAPBLEPDUStatus_UnsupportedPDU;
        }
            goto SendResponse;
        case kHAPPDUOpcode_Token: {
            // See HomeKit Accessory Protocol Specification R14
            // Section 5.15.1 HAP-Token-Request
            HAPAssert(!iid);
            HAPAssert(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);

            // HAP-Token-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(&writer);

            // Serialize HAP-Token-Response.
            err = HAPMFiTokenAuthGetTokenResponse(
                    HAPNonnull(session->server),
                    &session->securitySession._.hap,
                    HAPNonnull(server->primaryAccessory),
                    &writer);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
                HAPLogAccessory(
                        &logObject,
                        server->primaryAccessory,
                        "Rejected token request: Request handling failed with error %u.",
                        err);
                status = kHAPBLEPDUStatus_InvalidRequest;
                goto SendResponse;
            }
            HAPTLVWriterGetBuffer(&writer, &responseBodyBytes, &numResponseBodyBytes);
            status = kHAPBLEPDUStatus_Success;
        }
            goto SendResponse;
        case kHAPPDUOpcode_TokenUpdate: {
            // See HomeKit Accessory Protocol Specification R14
            // Section 5.15.3 HAP-Token-Update-Request
            HAPAssert(!iid);
            HAPAssert(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);

            // Handle HAP-Token-Update-Request.
            err = HAPMFiTokenAuthHandleTokenUpdateRequest(
                    HAPNonnull(session->server),
                    &session->securitySession._.hap,
                    HAPNonnull(server->primaryAccessory),
                    &requestBodyReader);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_InvalidData);
                HAPLogAccessory(
                        &logObject,
                        server->primaryAccessory,
                        "Rejected token update request: Request handling failed with error %u.",
                        err);
                status = kHAPBLEPDUStatus_InvalidRequest;
                goto SendResponse;
            }

            // Send HAP-Token-Update-Response.
            status = kHAPBLEPDUStatus_Success;
        }
            goto SendResponse;
        case kHAPPDUOpcode_Info: {
            // See HomeKit Accessory Protocol Specification R14
            // Section 5.15.5 HAP-Info-Request
            HAPAssert(!iid);
            HAPAssert(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);

            // HAP-Info-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(&writer);

            // Serialize HAP-Info-Response.
            err = HAPAccessoryGetInfoResponse(
                    HAPNonnull(session->server),
                    &session->securitySession._.hap,
                    HAPNonnull(server->primaryAccessory),
                    &writer);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_OutOfResources);
                HAPLogAccessory(
                        &logObject,
                        server->primaryAccessory,
                        "Rejected info request: Request handling failed with error %u.",
                        err);
                status = kHAPBLEPDUStatus_InvalidRequest;
                goto SendResponse;
            }
            HAPTLVWriterGetBuffer(&writer, &responseBodyBytes, &numResponseBodyBytes);
            status = kHAPBLEPDUStatus_Success;
        }
            goto SendResponse;
    }

#undef DestroyRequestBodyAndCreateResponseBodyWriter

    HAPFatalError();
SendResponse : {
    // Serialize response.
    // Responses use the HAP PDU format.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.3 HAP PDU Format
    size_t mark = session->outboundBuffer.position;
    size_t numResponseBytes = 3;
    if (responseBodyBytes) {
        numResponseBytes += 2;
        numResponseBytes += numResponseBodyBytes;
    }
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pa084)
    if (numResponseBytes > UINT32_MAX) {
        HAPLog(&logObject, "/secure-message response: Content length exceeds UINT32_MAX.");
        session->outboundBuffer.position = mark;
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
        return;
    }
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pa084)
    const char* contentType = "application/octet-stream";
    err = HAPIPByteBufferAppendStringWithFormat(
            &session->outboundBuffer,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %lu\r\n\r\n",
            contentType,
            (unsigned long) numResponseBytes);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        session->outboundBuffer.position = mark;
        HAPLog(&logObject, "/secure-message response: Invalid configuration (outbound buffer too small for headers).");
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_InternalServerError);
        return;
    }
    if (numResponseBytes > session->outboundBuffer.limit - session->outboundBuffer.position) {
        HAPAssert(err == kHAPError_OutOfResources);
        session->outboundBuffer.position = mark;
        HAPLog(&logObject, "/secure-message response: Invalid configuration (outbound buffer too small for body).");
        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_InternalServerError);
        return;
    }
    session->outboundBuffer.data[session->outboundBuffer.position++] =
            (0 << 7) | (0 << 4) | (0 << 3) | (0 << 2) | (1 << 1) | (0 << 0);
    session->outboundBuffer.data[session->outboundBuffer.position++] = (char) tid;
    session->outboundBuffer.data[session->outboundBuffer.position++] = (char) status;
    if (responseBodyBytes) {
        HAPWriteLittleUInt16(&session->outboundBuffer.data[session->outboundBuffer.position], numResponseBodyBytes);
        session->outboundBuffer.position += 2;

        HAPRawBufferCopyBytes(
                &session->outboundBuffer.data[session->outboundBuffer.position],
                HAPNonnullVoid(responseBodyBytes),
                numResponseBodyBytes);
        session->outboundBuffer.position += numResponseBodyBytes;
    }
    HAPAssert(session->outboundBuffer.limit >= session->outboundBuffer.position);
}
}

static void identify_primary_accessory(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(server->primaryAccessory);
    HAPPrecondition(server->primaryAccessory->aid == kHAPIPAccessoryProtocolAID_PrimaryAccessory);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(!session->securitySession.isSecured);

    HAPError err;

    const HAPService* service = NULL;
    for (size_t i = 0; server->primaryAccessory->services[i]; i++) {
        const HAPService* s = server->primaryAccessory->services[i];
        if ((s->iid == kHAPIPAccessoryProtocolIID_AccessoryInformation) &&
            HAPUUIDAreEqual(s->serviceType, &kHAPServiceType_AccessoryInformation)) {
            service = s;
            break;
        }
    }
    if (service) {
        const HAPBaseCharacteristic* characteristic = NULL;
        for (size_t i = 0; service->characteristics[i]; i++) {
            const HAPBaseCharacteristic* c = service->characteristics[i];
            if (HAPUUIDAreEqual(c->characteristicType, &kHAPCharacteristicType_Identify) &&
                (c->format == kHAPCharacteristicFormat_Bool) && c->properties.writable) {
                characteristic = c;
                break;
            }
        }
        if (characteristic) {
            err = HAPBoolCharacteristicHandleWrite(
                    HAPNonnull(session->server),
                    &(const HAPBoolCharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_IP,
                            .session = &session->securitySession._.hap,
                            .characteristic = (const HAPBoolCharacteristic*) characteristic,
                            .service = service,
                            .accessory = HAPNonnull(server->primaryAccessory),
                            .remote = false,
                            .authorizationData = { .bytes = NULL, .numBytes = 0 } },
                    true,
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                HAPLog(&logObject, "Identify failed: %u.", err);
            }
        }
    }

    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_NoContent);
}

static void handle_http_request(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(session->securitySession.isOpen);

    HAPAssert(session->httpReader.state == util_HTTP_READER_STATE_DONE);
    HAPAssert(!session->httpParserError);

    {
        HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);

        if ((session->httpURI.numBytes == 9) &&
            HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/identify", 9)) {
            if ((session->httpMethod.numBytes == 4) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "POST", 4)) {
                if (!HAPAccessoryServerIsPaired(HAPNonnull(session->server))) {
                    identify_primary_accessory(session);
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_InsufficientPrivileges);
                }
            } else {
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
            }
        } else if (
                (session->httpURI.numBytes == 11) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/pair-setup", 11)) {
            if ((session->httpMethod.numBytes == 4) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "POST", 4)) {
                if (!session->securitySession.isSecured) {
                    // Close existing transient session.
                    for (size_t i = 0; i < server->ip.storage->numSessions; i++) {
                        HAPIPSession* ipSession = &server->ip.storage->sessions[i];
                        HAPIPSessionDescriptor* t = (HAPIPSessionDescriptor*) &ipSession->descriptor;
                        if (!t->server) {
                            continue;
                        }
                        // TODO Make this finish writing ongoing responses. Similar to Remove Pairing.
                        if (t != session && t->securitySession.type == kHAPIPSecuritySessionType_HAP &&
                            HAPSessionIsTransient(&t->securitySession._.hap)) {
                            HAPLog(&logObject,
                                   "Closing transient session "
                                   "due to /pair-setup while transient session is active.");
                            CloseSession(t);
                        }
                    }

                    // Handle message.
                    handle_pairing_data(session, HAPSessionHandlePairSetupWrite, HAPSessionHandlePairSetupRead);
                } else {
                    HAPLog(&logObject, "Rejected POST /pair-setup: Only non-secure access is supported.");
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                }
            } else {
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
            }
        } else if (
                (session->httpURI.numBytes == 12) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/pair-verify", 12)) {
            if ((session->httpMethod.numBytes == 4) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "POST", 4)) {
                if (!session->securitySession.isSecured) {
                    handle_pairing_data(session, HAPSessionHandlePairVerifyWrite, HAPSessionHandlePairVerifyRead);
                } else {
                    HAPLog(&logObject, "Rejected POST /pair-verify: Only non-secure access is supported.");
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                }
            } else {
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
            }
        } else if (
                (session->httpURI.numBytes == 9) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/pairings", 9)) {
            if ((session->httpMethod.numBytes == 4) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "POST", 4)) {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        handle_pairing_data(session, HAPSessionHandlePairingsWrite, HAPSessionHandlePairingsRead);
                    } else {
                        HAPLog(&logObject, "Rejected POST /pairings: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            } else {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
                    } else {
                        HAPLog(&logObject, "Rejected request for /pairings: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            }
        } else if (
                (session->httpURI.numBytes == 15) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/secure-message", 15)) {
            if ((session->httpMethod.numBytes == 4) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "POST", 4)) {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    handle_secure_message(session);
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            } else {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            }
        } else if (
                (session->httpURI.numBytes == 7) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/config", 7)) {
            if ((session->httpMethod.numBytes == 4) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "POST", 4)) {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        HAPLog(&logObject, "Rejected POST /config: Session is not transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ResourceNotFound);
                    } else {
                        HAPLog(&logObject, "Rejected POST /config: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            } else {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
                    } else {
                        HAPLog(&logObject, "Rejected request for /config: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            }
        } else if (
                (session->httpURI.numBytes == 11) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/configured", 11)) {
            if ((session->httpMethod.numBytes == 4) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "POST", 4)) {
                HAPLog(&logObject, "Received unexpected /configured on _hap._tcp endpoint. Replying with success.");
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_NoContent);
            } else {
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
            }
        } else if (
                (session->httpURI.numBytes == 12) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/accessories", 12)) {
            if ((session->httpMethod.numBytes == 3) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "GET", 3)) {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        get_accessories(session);
                    } else {
                        HAPLog(&logObject, "Rejected GET /accessories: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(
                            &session->outboundBuffer,
                            kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequiredWithStatus);
                }
            } else {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
                    } else {
                        HAPLog(&logObject, "Rejected request for /accessories: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            }
        } else if (
                (session->httpURI.numBytes >= 16) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/characteristics", 16)) {
            if ((session->httpMethod.numBytes == 3) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "GET", 3)) {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        get_characteristics(session);
                    } else {
                        HAPLog(&logObject, "Rejected GET /characteristics: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(
                            &session->outboundBuffer,
                            kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequiredWithStatus);
                }
            } else if (
                    (session->httpMethod.numBytes == 3) &&
                    HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "PUT", 3)) {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        put_characteristics(session);
                    } else {
                        HAPLog(&logObject, "Rejected PUT /characteristics: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(
                            &session->outboundBuffer,
                            kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequiredWithStatus);
                }
            } else {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
                    } else {
                        HAPLog(&logObject, "Rejected request for /characteristics: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            }
        } else if (
                (session->httpURI.numBytes == 8) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/prepare", 8)) {
            if ((session->httpMethod.numBytes == 3) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "PUT", 3)) {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        put_prepare(session);
                    } else {
                        HAPLog(&logObject, "Rejected PUT /prepare: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(
                            &session->outboundBuffer,
                            kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequiredWithStatus);
                }
            } else {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
                    } else {
                        HAPLog(&logObject, "Rejected request for /prepare: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            }
        } else if (
                (session->httpURI.numBytes == 9) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpURI.bytes), "/resource", 9)) {
            if ((session->httpMethod.numBytes == 4) &&
                HAPRawBufferAreEqual(HAPNonnull(session->httpMethod.bytes), "POST", 4)) {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        post_resource(session);
                    } else {
                        HAPLog(&logObject, "Rejected POST /resource: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(
                            &session->outboundBuffer,
                            kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequiredWithStatus);
                }
            } else {
                if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                    if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_MethodNotAllowed);
                    } else {
                        HAPLog(&logObject, "Rejected request for /resource: Session is transient.");
                        write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                    }
                } else {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
                }
            }
        } else {
            HAPLogBuffer(
                    &logObject,
                    session->httpURI.bytes,
                    HAPStringGetNumBytes(HAPNonnull(session->httpURI.bytes)),
                    "Unknown endpoint accessed.");
            if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
                if (!HAPSessionIsTransient(&session->securitySession._.hap)) {
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ResourceNotFound);
                } else {
                    HAPLog(&logObject, "Rejected request for unknown endpoint: Session is transient.");
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_BadRequest);
                }
            } else {
                write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_ConnectionAuthorizationRequired);
            }
        }
    }
}

static void handle_http(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.isOpen);

    size_t content_length, encrypted_length;
    HAPAssert(session->inboundBuffer.data);
    HAPAssert(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPAssert(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPAssert(session->httpReaderPosition <= session->inboundBuffer.position);
    HAPAssert(session->httpReader.state == util_HTTP_READER_STATE_DONE);
    HAPAssert(!session->httpParserError);
    if (session->httpContentLength.isDefined) {
        content_length = session->httpContentLength.value;
    } else {
        content_length = 0;
    }
    if ((content_length <= session->inboundBuffer.position) &&
        (session->httpReaderPosition <= session->inboundBuffer.position - content_length)) {
        HAPLogBufferDebug(
                &logObject,
                session->inboundBuffer.data,
                session->httpReaderPosition + content_length,
                "session:%p:>",
                (const void*) session);
        handle_http_request(session);
        HAPIPByteBufferShiftLeft(&session->inboundBuffer, session->httpReaderPosition + content_length);
        if (session->accessorySerializationIsInProgress) {
            // Session is already prepared for writing
            HAPAssert(session->outboundBuffer.data);
            HAPAssert(session->outboundBuffer.position <= session->outboundBuffer.limit);
            HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);
            HAPAssert(session->state == kHAPIPSessionState_Writing);
        } else {
            HAPAssert(session->outboundBuffer.data);
            HAPAssert(session->outboundBuffer.position <= session->outboundBuffer.limit);
            HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);
            HAPIPByteBufferFlip(&session->outboundBuffer);
            HAPLogBufferDebug(
                    &logObject,
                    session->outboundBuffer.data,
                    session->outboundBuffer.limit,
                    "session:%p:<",
                    (const void*) session);

            if (session->securitySession.type == kHAPIPSecuritySessionType_HAP && session->securitySession.isSecured) {
                encrypted_length = HAPIPSecurityProtocolGetNumEncryptedBytes(
                        session->outboundBuffer.limit - session->outboundBuffer.position);
                if (encrypted_length > session->outboundBuffer.capacity - session->outboundBuffer.position) {
                    HAPLog(&logObject, "Out of resources (outbound buffer too small).");
                    session->outboundBuffer.limit = session->outboundBuffer.capacity;
                    write_msg(&session->outboundBuffer, kHAPIPAccessoryServerResponse_OutOfResources);
                    HAPIPByteBufferFlip(&session->outboundBuffer);
                    encrypted_length = HAPIPSecurityProtocolGetNumEncryptedBytes(
                            session->outboundBuffer.limit - session->outboundBuffer.position);
                    HAPAssert(encrypted_length <= session->outboundBuffer.capacity - session->outboundBuffer.position);
                }
                HAPIPSecurityProtocolEncryptData(
                        HAPNonnull(session->server), &session->securitySession._.hap, &session->outboundBuffer);
                HAPAssert(encrypted_length == session->outboundBuffer.limit - session->outboundBuffer.position);
            }
            session->state = kHAPIPSessionState_Writing;
        }
    }
}

static void update_token(struct util_http_reader* r, char** token, size_t* length) {
    HAPAssert(r);
    HAPAssert(token);
    HAPAssert(length);

    if (!*token) {
        *token = r->result_token;
        *length = r->result_length;
    } else if (r->result_token) {
        HAPAssert(&(*token)[*length] == r->result_token);
        *length += r->result_length;
    }
}

static void read_http_content_length(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);

    size_t i;
    int overflow;
    unsigned int v;

    HAPAssert(session->inboundBuffer.data);
    HAPAssert(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPAssert(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPAssert(session->httpReaderPosition <= session->inboundBuffer.position);
    HAPAssert(session->httpReader.state == util_HTTP_READER_STATE_COMPLETED_HEADER_VALUE);
    HAPAssert(!session->httpParserError);
    i = 0;
    while ((i < session->httpHeaderFieldValue.numBytes) &&
           ((session->httpHeaderFieldValue.bytes[i] == kHAPIPAccessoryServerCharacter_Space) ||
            (session->httpHeaderFieldValue.bytes[i] == kHAPIPAccessoryServerCharacter_HorizontalTab))) {
        // Skip whitespace.
        i++;
    }
    HAPAssert(
            (i == session->httpHeaderFieldValue.numBytes) ||
            ((i < session->httpHeaderFieldValue.numBytes) &&
             (session->httpHeaderFieldValue.bytes[i] != kHAPIPAccessoryServerCharacter_Space) &&
             (session->httpHeaderFieldValue.bytes[i] != kHAPIPAccessoryServerCharacter_HorizontalTab)));
    if ((i < session->httpHeaderFieldValue.numBytes) && ('0' <= session->httpHeaderFieldValue.bytes[i]) &&
        (session->httpHeaderFieldValue.bytes[i] <= '9') && !session->httpContentLength.isDefined) {
        overflow = 0;
        session->httpContentLength.value = 0;
        do {
            v = (unsigned int) (session->httpHeaderFieldValue.bytes[i] - '0');
            if (session->httpContentLength.value <= (SIZE_MAX - v) / 10) {
                session->httpContentLength.value = session->httpContentLength.value * 10 + v;
                i++;
            } else {
                overflow = 1;
            }
        } while (!overflow && (i < session->httpHeaderFieldValue.numBytes) &&
                 ('0' <= session->httpHeaderFieldValue.bytes[i]) && (session->httpHeaderFieldValue.bytes[i] <= '9'));
        HAPAssert(
                overflow || (i == session->httpHeaderFieldValue.numBytes) ||
                ((i < session->httpHeaderFieldValue.numBytes) &&
                 ((session->httpHeaderFieldValue.bytes[i] < '0') || (session->httpHeaderFieldValue.bytes[i] > '9'))));
        if (!overflow) {
            while ((i < session->httpHeaderFieldValue.numBytes) &&
                   ((session->httpHeaderFieldValue.bytes[i] == kHAPIPAccessoryServerCharacter_Space) ||
                    (session->httpHeaderFieldValue.bytes[i] == kHAPIPAccessoryServerCharacter_HorizontalTab))) {
                i++;
            }
            HAPAssert(
                    (i == session->httpHeaderFieldValue.numBytes) ||
                    ((i < session->httpHeaderFieldValue.numBytes) &&
                     (session->httpHeaderFieldValue.bytes[i] != kHAPIPAccessoryServerCharacter_Space) &&
                     (session->httpHeaderFieldValue.bytes[i] != kHAPIPAccessoryServerCharacter_HorizontalTab)));
            if (i == session->httpHeaderFieldValue.numBytes) {
                session->httpContentLength.isDefined = true;
            } else {
                session->httpParserError = true;
            }
        } else {
            session->httpParserError = true;
        }
    } else {
        session->httpParserError = true;
    }
}

static void read_http_content_type(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);

    HAPAssert(session->inboundBuffer.data);
    HAPAssert(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPAssert(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPAssert(session->httpReaderPosition <= session->inboundBuffer.position);
    HAPAssert(session->httpReader.state == util_HTTP_READER_STATE_COMPLETED_HEADER_VALUE);
    HAPAssert(!session->httpParserError);

    size_t i = 0;
    while ((i < session->httpHeaderFieldValue.numBytes) &&
           ((session->httpHeaderFieldValue.bytes[i] == kHAPIPAccessoryServerCharacter_Space) ||
            (session->httpHeaderFieldValue.bytes[i] == kHAPIPAccessoryServerCharacter_HorizontalTab))) {
        // Skip whitespace.
        i++;
    }
    HAPAssert(
            (i == session->httpHeaderFieldValue.numBytes) ||
            ((i < session->httpHeaderFieldValue.numBytes) &&
             (session->httpHeaderFieldValue.bytes[i] != kHAPIPAccessoryServerCharacter_Space) &&
             (session->httpHeaderFieldValue.bytes[i] != kHAPIPAccessoryServerCharacter_HorizontalTab)));
    if ((i < session->httpHeaderFieldValue.numBytes)) {
        session->httpContentType = kHAPIPAccessoryServerContentType_Unknown;

#define TryAssignContentType(contentType, contentTypeString) \
    do { \
        size_t numContentTypeStringBytes = sizeof(contentTypeString) - 1; \
        if (session->httpHeaderFieldValue.numBytes - i >= numContentTypeStringBytes && \
            HAPRawBufferAreEqual( \
                    &session->httpHeaderFieldValue.bytes[i], (contentTypeString), numContentTypeStringBytes)) { \
            session->httpContentType = (contentType); \
            i += numContentTypeStringBytes; \
        } \
    } while (0)

        // Check longer header values first if multiple have the same prefix.
        TryAssignContentType(kHAPIPAccessoryServerContentType_Application_HAPJSON, "application/hap+json");
        TryAssignContentType(kHAPIPAccessoryServerContentType_Application_OctetStream, "application/octet-stream");
        TryAssignContentType(kHAPIPAccessoryServerContentType_Application_PairingTLV8, "application/pairing+tlv8");

#undef TryAssignContentType

        while ((i < session->httpHeaderFieldValue.numBytes) &&
               ((session->httpHeaderFieldValue.bytes[i] == kHAPIPAccessoryServerCharacter_Space) ||
                (session->httpHeaderFieldValue.bytes[i] == kHAPIPAccessoryServerCharacter_HorizontalTab))) {
            i++;
        }
        HAPAssert(
                (i == session->httpHeaderFieldValue.numBytes) ||
                ((i < session->httpHeaderFieldValue.numBytes) &&
                 (session->httpHeaderFieldValue.bytes[i] != kHAPIPAccessoryServerCharacter_Space) &&
                 (session->httpHeaderFieldValue.bytes[i] != kHAPIPAccessoryServerCharacter_HorizontalTab)));
        if (i != session->httpHeaderFieldValue.numBytes) {
            HAPLogBuffer(
                    &logObject,
                    session->httpHeaderFieldValue.bytes,
                    session->httpHeaderFieldValue.numBytes,
                    "Unknown Content-Type.");
            session->httpContentType = kHAPIPAccessoryServerContentType_Unknown;
        }
    } else {
        session->httpParserError = true;
    }
}

static void read_http(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);

    struct util_http_reader* r;

    HAPAssert(session->inboundBuffer.data);
    HAPAssert(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPAssert(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPAssert(session->httpReaderPosition <= session->inboundBuffer.position);
    HAPAssert(!session->httpParserError);
    r = &session->httpReader;
    bool hasContentLength = false;
    bool hasContentType = false;
    do {
        session->httpReaderPosition += util_http_reader_read(
                r,
                &session->inboundBuffer.data[session->httpReaderPosition],
                session->inboundBuffer.position - session->httpReaderPosition);
        switch (r->state) {
            case util_HTTP_READER_STATE_READING_METHOD:
            case util_HTTP_READER_STATE_COMPLETED_METHOD: {
                update_token(r, &session->httpMethod.bytes, &session->httpMethod.numBytes);
            } break;
            case util_HTTP_READER_STATE_READING_URI:
            case util_HTTP_READER_STATE_COMPLETED_URI: {
                update_token(r, &session->httpURI.bytes, &session->httpURI.numBytes);
            } break;
            case util_HTTP_READER_STATE_READING_HEADER_NAME:
            case util_HTTP_READER_STATE_COMPLETED_HEADER_NAME: {
                update_token(r, &session->httpHeaderFieldName.bytes, &session->httpHeaderFieldName.numBytes);
            } break;
            case util_HTTP_READER_STATE_READING_HEADER_VALUE: {
                update_token(r, &session->httpHeaderFieldValue.bytes, &session->httpHeaderFieldValue.numBytes);
            } break;
            case util_HTTP_READER_STATE_COMPLETED_HEADER_VALUE: {
                update_token(r, &session->httpHeaderFieldValue.bytes, &session->httpHeaderFieldValue.numBytes);
                HAPAssert(session->httpHeaderFieldName.bytes);
                if ((session->httpHeaderFieldName.numBytes == 14) &&
                    (session->httpHeaderFieldName.bytes[0] == 'C' || session->httpHeaderFieldName.bytes[0] == 'c') &&
                    (session->httpHeaderFieldName.bytes[1] == 'O' || session->httpHeaderFieldName.bytes[1] == 'o') &&
                    (session->httpHeaderFieldName.bytes[2] == 'N' || session->httpHeaderFieldName.bytes[2] == 'n') &&
                    (session->httpHeaderFieldName.bytes[3] == 'T' || session->httpHeaderFieldName.bytes[3] == 't') &&
                    (session->httpHeaderFieldName.bytes[4] == 'E' || session->httpHeaderFieldName.bytes[4] == 'e') &&
                    (session->httpHeaderFieldName.bytes[5] == 'N' || session->httpHeaderFieldName.bytes[5] == 'n') &&
                    (session->httpHeaderFieldName.bytes[6] == 'T' || session->httpHeaderFieldName.bytes[6] == 't') &&
                    (session->httpHeaderFieldName.bytes[7] == '-') &&
                    (session->httpHeaderFieldName.bytes[8] == 'L' || session->httpHeaderFieldName.bytes[8] == 'l') &&
                    (session->httpHeaderFieldName.bytes[9] == 'E' || session->httpHeaderFieldName.bytes[9] == 'e') &&
                    (session->httpHeaderFieldName.bytes[10] == 'N' || session->httpHeaderFieldName.bytes[10] == 'n') &&
                    (session->httpHeaderFieldName.bytes[11] == 'G' || session->httpHeaderFieldName.bytes[11] == 'g') &&
                    (session->httpHeaderFieldName.bytes[12] == 'T' || session->httpHeaderFieldName.bytes[12] == 't') &&
                    (session->httpHeaderFieldName.bytes[13] == 'H' || session->httpHeaderFieldName.bytes[13] == 'h')) {
                    if (hasContentLength) {
                        HAPLog(&logObject, "Request has multiple Content-Length headers.");
                        session->httpParserError = true;
                    } else {
                        hasContentLength = true;
                        read_http_content_length(session);
                    }
                } else if (
                        (session->httpHeaderFieldName.numBytes == 12) &&
                        (session->httpHeaderFieldName.bytes[0] == 'C' ||
                         session->httpHeaderFieldName.bytes[0] == 'c') &&
                        (session->httpHeaderFieldName.bytes[1] == 'O' ||
                         session->httpHeaderFieldName.bytes[1] == 'o') &&
                        (session->httpHeaderFieldName.bytes[2] == 'N' ||
                         session->httpHeaderFieldName.bytes[2] == 'n') &&
                        (session->httpHeaderFieldName.bytes[3] == 'T' ||
                         session->httpHeaderFieldName.bytes[3] == 't') &&
                        (session->httpHeaderFieldName.bytes[4] == 'E' ||
                         session->httpHeaderFieldName.bytes[4] == 'e') &&
                        (session->httpHeaderFieldName.bytes[5] == 'N' ||
                         session->httpHeaderFieldName.bytes[5] == 'n') &&
                        (session->httpHeaderFieldName.bytes[6] == 'T' ||
                         session->httpHeaderFieldName.bytes[6] == 't') &&
                        (session->httpHeaderFieldName.bytes[7] == '-') &&
                        (session->httpHeaderFieldName.bytes[8] == 'T' ||
                         session->httpHeaderFieldName.bytes[8] == 't') &&
                        (session->httpHeaderFieldName.bytes[9] == 'Y' ||
                         session->httpHeaderFieldName.bytes[9] == 'y') &&
                        (session->httpHeaderFieldName.bytes[10] == 'P' ||
                         session->httpHeaderFieldName.bytes[10] == 'p') &&
                        (session->httpHeaderFieldName.bytes[11] == 'E' ||
                         session->httpHeaderFieldName.bytes[11] == 'e')) {
                    if (hasContentType) {
                        HAPLog(&logObject, "Request has multiple Content-Type headers.");
                        session->httpParserError = true;
                    } else {
                        hasContentType = true;
                        read_http_content_type(session);
                    }
                }
                session->httpHeaderFieldName.bytes = NULL;
                session->httpHeaderFieldValue.bytes = NULL;
            } break;
            default: {
            } break;
        }
    } while ((session->httpReaderPosition < session->inboundBuffer.position) &&
             (r->state != util_HTTP_READER_STATE_DONE) && (r->state != util_HTTP_READER_STATE_ERROR) &&
             !session->httpParserError);
    HAPAssert(
            (session->httpReaderPosition == session->inboundBuffer.position) ||
            ((session->httpReaderPosition < session->inboundBuffer.position) &&
             ((r->state == util_HTTP_READER_STATE_DONE) || (r->state == util_HTTP_READER_STATE_ERROR) ||
              session->httpParserError)));
}

static void handle_input(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.isOpen);

    int r;

    HAPAssert(session->inboundBuffer.data);
    HAPAssert(session->inboundBuffer.position <= session->inboundBuffer.limit);
    HAPAssert(session->inboundBuffer.limit <= session->inboundBuffer.capacity);
    HAPAssert(session->inboundBufferMark <= session->inboundBuffer.position);
    session->inboundBuffer.limit = session->inboundBuffer.position;
    if (session->securitySession.type == kHAPIPSecuritySessionType_HAP &&
        HAPSessionIsSecured(&session->securitySession._.hap)) {
        // TODO Should be moved to handle_completed_output, maybe.
        if (!session->securitySession.isSecured) {
            HAPLogDebug(&logObject, "Established HAP security session.");
            session->securitySession.isSecured = true;
        }
        session->inboundBuffer.position = session->inboundBufferMark;
        r = HAPIPSecurityProtocolDecryptData(
                HAPNonnull(session->server), &session->securitySession._.hap, &session->inboundBuffer);
    } else {
        HAPAssert(
                session->securitySession.type != kHAPIPSecuritySessionType_HAP || !session->securitySession.isSecured);
        r = 0;
    }
    if (r == 0) {
        read_http(session);
        if ((session->httpReader.state == util_HTTP_READER_STATE_ERROR) || session->httpParserError) {
            log_protocol_error(
                    kHAPLogType_Info, "Unexpected request.", &session->inboundBuffer, __func__, HAP_FILE, __LINE__);
            CloseSession(session);
        } else {
            if (session->httpReader.state == util_HTTP_READER_STATE_DONE) {
                handle_http(session);
            }
            session->inboundBufferMark = session->inboundBuffer.position;
            session->inboundBuffer.position = session->inboundBuffer.limit;
            session->inboundBuffer.limit = session->inboundBuffer.capacity;
            if ((session->state == kHAPIPSessionState_Reading) &&
                (session->inboundBuffer.position == session->inboundBuffer.limit)) {
                log_protocol_error(
                        kHAPLogType_Info,
                        "Unexpected request. Closing connection (inbound buffer too small).",
                        &session->inboundBuffer,
                        __func__,
                        HAP_FILE,
                        __LINE__);
                CloseSession(session);
            }
        }
    } else {
        HAPAssert(r == -1);
        HAPLog(&logObject, "Decryption error.");
        CloseSession(session);
    }
}

static void HandleTCPStreamEvent(
        HAPPlatformTCPStreamManagerRef tcpStreamManager,
        HAPPlatformTCPStreamRef tcpStream,
        HAPPlatformTCPStreamEvent event,
        void* _Nullable context);

static void write_event_notifications(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));
    HAPPrecondition(session->state == kHAPIPSessionState_Reading);
    HAPPrecondition(session->inboundBuffer.position == 0);
    HAPPrecondition(session->numEventNotificationFlags > 0);
    HAPPrecondition(session->numEventNotificationFlags <= session->numEventNotifications);
    HAPPrecondition(session->numEventNotifications <= session->maxEventNotifications);

    HAPError err;

    if (session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled) {
        HAPTime clock_now_ms = HAPPlatformClockGetCurrent();
        HAPAssert(clock_now_ms >= session->eventNotificationStamp);
        HAPTime dt_ms = clock_now_ms - session->eventNotificationStamp;

        size_t numReadContexts = 0;

        for (size_t i = 0; i < session->numEventNotifications; i++) {
            HAPIPEventNotification* eventNotification = (HAPIPEventNotification*) &session->eventNotifications[i];
            if (eventNotification->flag) {
                bool notifyNow;
                if (dt_ms >= kHAPIPAccessoryServer_MaxEventNotificationDelay) {
                    notifyNow = true;
                    session->eventNotificationStamp = clock_now_ms;
                } else {
                    // Network-based notifications must be coalesced by the accessory using a delay of no less than
                    // 1 second. The exception to this rule includes notifications for the following characteristics
                    // which must be delivered immediately.
                    // See HomeKit Accessory Protocol Specification R14
                    // Section 6.8 Notifications
                    const HAPCharacteristic* characteristic_;
                    const HAPService* service;
                    const HAPAccessory* accessory;
                    get_db_ctx(
                            session->server,
                            eventNotification->aid,
                            eventNotification->iid,
                            &characteristic_,
                            &service,
                            &accessory);
                    HAPAssert(accessory);
                    HAPAssert(service);
                    HAPAssert(characteristic_);
                    const HAPBaseCharacteristic* characteristic = characteristic_;
                    notifyNow = HAPUUIDAreEqual(
                            characteristic->characteristicType, &kHAPCharacteristicType_ProgrammableSwitchEvent);
                    if (notifyNow) {
                        HAPLogCharacteristicDebug(
                                &logObject,
                                characteristic_,
                                service,
                                accessory,
                                "Characteristic whitelisted to bypassing notification coalescing requirement.");
                    }
                }
                if (notifyNow) {
                    HAPAssert(numReadContexts < server->ip.storage->numReadContexts);
                    HAPIPReadContext* readContext =
                            (HAPIPReadContext*) &server->ip.storage->readContexts[numReadContexts];
                    HAPRawBufferZero(readContext, sizeof *readContext);
                    readContext->aid = eventNotification->aid;
                    readContext->iid = eventNotification->iid;
                    numReadContexts++;
                    eventNotification->flag = false;
                    HAPAssert(session->numEventNotificationFlags > 0);
                    session->numEventNotificationFlags--;
                }
            }
        }

        if (numReadContexts > 0) {
            HAPIPByteBuffer data_buffer;
            data_buffer.data = server->ip.storage->scratchBuffer.bytes;
            data_buffer.capacity = server->ip.storage->scratchBuffer.numBytes;
            data_buffer.limit = server->ip.storage->scratchBuffer.numBytes;
            data_buffer.position = 0;
            HAPAssert(data_buffer.data);
            HAPAssert(data_buffer.position <= data_buffer.limit);
            HAPAssert(data_buffer.limit <= data_buffer.capacity);
            int r = handle_characteristic_read_requests(
                    session,
                    kHAPIPSessionContext_EventNotification,
                    server->ip.storage->readContexts,
                    numReadContexts,
                    &data_buffer);
            (void) r;

            size_t content_length = HAPIPAccessoryProtocolGetNumEventNotificationBytes(
                    HAPNonnull(session->server), server->ip.storage->readContexts, numReadContexts);

            HAPAssert(session->outboundBuffer.data);
            HAPAssert(session->outboundBuffer.position <= session->outboundBuffer.limit);
            HAPAssert(session->outboundBuffer.limit <= session->outboundBuffer.capacity);
            size_t mark = session->outboundBuffer.position;
            err = HAPIPByteBufferAppendStringWithFormat(
                    &session->outboundBuffer,
                    "EVENT/1.0 200 OK\r\n"
                    "Content-Type: application/hap+json\r\n"
                    "Content-Length: %zu\r\n\r\n",
                    content_length);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                HAPLog(&logObject, "Invalid configuration (outbound buffer too small).");
                HAPFatalError();
            }
            if (content_length <= session->outboundBuffer.limit - session->outboundBuffer.position) {
                mark = session->outboundBuffer.position;
                err = HAPIPAccessoryProtocolGetEventNotificationBytes(
                        HAPNonnull(session->server),
                        server->ip.storage->readContexts,
                        numReadContexts,
                        &session->outboundBuffer);
                HAPAssert(!err && (session->outboundBuffer.position - mark == content_length));
                HAPIPByteBufferFlip(&session->outboundBuffer);
                HAPLogBufferDebug(
                        &logObject,
                        session->outboundBuffer.data,
                        session->outboundBuffer.limit,
                        "session:%p:<",
                        (const void*) session);
                if (session->securitySession.isSecured) {
                    size_t encrypted_length = HAPIPSecurityProtocolGetNumEncryptedBytes(
                            session->outboundBuffer.limit - session->outboundBuffer.position);
                    if (encrypted_length <= session->outboundBuffer.capacity - session->outboundBuffer.position) {
                        HAPIPSecurityProtocolEncryptData(
                                HAPNonnull(session->server), &session->securitySession._.hap, &session->outboundBuffer);
                        HAPAssert(encrypted_length == session->outboundBuffer.limit - session->outboundBuffer.position);
                        session->state = kHAPIPSessionState_Writing;
                    } else {
                        HAPLog(&logObject, "Skipping event notifications (outbound buffer too small).");
                        HAPIPByteBufferClear(&session->outboundBuffer);
                    }
                } else {
                    HAPAssert(kHAPIPAccessoryServer_SessionSecurityDisabled);
                    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe111)
                    session->state = kHAPIPSessionState_Writing;
                    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe111)
                }
                if (session->state == kHAPIPSessionState_Writing) {
                    HAPPlatformTCPStreamEvent interests = { .hasBytesAvailable = false, .hasSpaceAvailable = true };
                    HAPPlatformTCPStreamUpdateInterests(
                            HAPNonnull(server->platform.ip.tcpStreamManager),
                            session->tcpStream,
                            interests,
                            HandleTCPStreamEvent,
                            session);
                }
            } else {
                HAPLog(&logObject, "Skipping event notifications (outbound buffer too small).");
                session->outboundBuffer.position = mark;
            }
        }
    } else {
        for (size_t i = 0; i < session->numEventNotifications; i++) {
            HAPIPEventNotification* eventNotification = (HAPIPEventNotification*) &session->eventNotifications[i];
            if (eventNotification->flag) {
                eventNotification->flag = false;
                HAPAssert(session->numEventNotificationFlags > 0);
                session->numEventNotificationFlags--;
            }
        }
        HAPAssert(session->numEventNotificationFlags == 0);
        session->eventNotificationStamp = HAPPlatformClockGetCurrent();
    }
}

static void handle_io_progression(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;

    if ((session->state == kHAPIPSessionState_Reading) && (session->inboundBuffer.position == 0)) {
        if (server->ip.state == kHAPIPAccessoryServerState_Stopping) {
            CloseSession(session);
        } else {
            HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Running);
            if (session->numEventNotificationFlags > 0) {
                HAPAssert(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
                schedule_event_notifications(session->server);
            }
        }
    }
    if (session->tcpStreamIsOpen) {
        HAPPlatformTCPStreamEvent interests = { .hasBytesAvailable = (session->state == kHAPIPSessionState_Reading),
                                                .hasSpaceAvailable = (session->state == kHAPIPSessionState_Writing) };
        if ((session->state == kHAPIPSessionState_Reading) || (session->state == kHAPIPSessionState_Writing)) {
            HAPPlatformTCPStreamUpdateInterests(
                    HAPNonnull(server->platform.ip.tcpStreamManager),
                    session->tcpStream,
                    interests,
                    HandleTCPStreamEvent,
                    session);
        } else {
            HAPPlatformTCPStreamUpdateInterests(
                    HAPNonnull(server->platform.ip.tcpStreamManager), session->tcpStream, interests, NULL, session);
        }
    } else {
        HAPAssert(server->ip.garbageCollectionTimer);
    }
}

static void handle_output_completion(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;

    HAPAssert(session->state == kHAPIPSessionState_Writing);
    if (session->securitySession.isOpen && session->securitySession.receivedConfig) {
        HAPLogDebug(&logObject, "Completed sending of Wi-Fi configuration response.");

        HAPAssert(session->tcpStreamIsOpen);
        HAPPlatformTCPStreamCloseOutput(HAPNonnull(server->platform.ip.tcpStreamManager), session->tcpStream);
    } else if (
            session->securitySession.type == kHAPIPSecuritySessionType_MFiSAP && session->securitySession.isOpen &&
            session->securitySession._.mfiSAP.receivedConfigured) {
        HAPLogDebug(&logObject, "Completed sending of /configured response.");
        session->securitySession._.mfiSAP.receivedConfigured = false;
        HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Stopping);
    }
    session->state = kHAPIPSessionState_Reading;
    prepare_reading_request(session);
    if (session->inboundBuffer.position != 0) {
        handle_input(session);
    }
}

static void WriteOutboundData(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(session->tcpStreamIsOpen);

    HAPError err;

    HAPIPByteBuffer* b;
    b = &session->outboundBuffer;
    HAPAssert(b->data);
    HAPAssert(b->position <= b->limit);
    HAPAssert(b->limit <= b->capacity);

    size_t numBytes;
    err = HAPPlatformTCPStreamWrite(
            HAPNonnull(server->platform.ip.tcpStreamManager),
            session->tcpStream,
            /* bytes: */ &b->data[b->position],
            /* maxBytes: */ b->limit - b->position,
            &numBytes);

    if (err == kHAPError_Unknown) {
        log_result(
                kHAPLogType_Error,
                "error:Function 'HAPPlatformTCPStreamWrite' failed.",
                err,
                __func__,
                HAP_FILE,
                __LINE__);
        CloseSession(session);
        return;
    } else if (err == kHAPError_Busy) {
        return;
    }

    HAPAssert(!err);
    if (numBytes == 0) {
        HAPLogDebug(&logObject, "error:Function 'HAPPlatformTCPStreamWrite' failed: 0 bytes written.");
        CloseSession(session);
        return;
    } else {
        HAPAssert(numBytes <= b->limit - b->position);
        b->position += numBytes;
        if (b->position == b->limit) {
            if (session->securitySession.type == kHAPIPSecuritySessionType_HAP && session->securitySession.isSecured &&
                !HAPSessionIsSecured(&session->securitySession._.hap)) {
                HAPLogDebug(&logObject, "Pairing removed, closing session.");
                CloseSession(session);
            } else if (session->accessorySerializationIsInProgress) {
                handle_accessory_serialization(session);
            } else {
                HAPIPByteBufferClear(b);
                handle_output_completion(session);
            }
        }
    }
}

static void handle_input_closed(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;

    HAPLogDebug(&logObject, "session:%p:input closed", (const void*) session);

    if (session->securitySession.isOpen && session->securitySession.receivedConfig) {
        HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Stopping);
    } else {
        CloseSession(session);
    }
}

static void ReadInboundData(HAPIPSessionDescriptor* session) {
    HAPPrecondition(session);
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPAssert(session->tcpStreamIsOpen);

    HAPError err;

    HAPIPByteBuffer* b;
    b = &session->inboundBuffer;
    HAPAssert(b->data);
    HAPAssert(b->position <= b->limit);
    HAPAssert(b->limit <= b->capacity);

    size_t numBytes;
    err = HAPPlatformTCPStreamRead(
            HAPNonnull(server->platform.ip.tcpStreamManager),
            session->tcpStream,
            /* bytes: */ &b->data[b->position],
            /* maxBytes: */ b->limit - b->position,
            &numBytes);

    if (err == kHAPError_Unknown) {
        log_result(
                kHAPLogType_Error,
                "error:Function 'HAPPlatformTCPStreamRead' failed.",
                err,
                __func__,
                HAP_FILE,
                __LINE__);
        CloseSession(session);
        return;
    } else if (err == kHAPError_Busy) {
        return;
    }

    HAPAssert(!err);
    if (numBytes == 0) {
        handle_input_closed(session);
    } else {
        HAPAssert(numBytes <= b->limit - b->position);
        b->position += numBytes;
        handle_input(session);
    }
}

static void HandleTCPStreamEvent(
        HAPPlatformTCPStreamManagerRef tcpStreamManager_,
        HAPPlatformTCPStreamRef tcpStream,
        HAPPlatformTCPStreamEvent event,
        void* _Nullable context) {
    HAPAssert(context);
    HAPIPSessionDescriptor* session = context;
    HAPAssert(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPAssert(tcpStreamManager_ == server->platform.ip.tcpStreamManager);
    HAPAssert(session->tcpStream == tcpStream);
    HAPAssert(session->tcpStreamIsOpen);

    HAPTime clock_now_ms = HAPPlatformClockGetCurrent();

    if (event.hasBytesAvailable) {
        HAPAssert(!event.hasSpaceAvailable);
        HAPAssert(session->state == kHAPIPSessionState_Reading);
        session->stamp = clock_now_ms;
        ReadInboundData(session);
        handle_io_progression(session);
    }

    if (event.hasSpaceAvailable) {
        HAPAssert(!event.hasBytesAvailable);
        HAPAssert(session->state == kHAPIPSessionState_Writing);
        session->stamp = clock_now_ms;
        WriteOutboundData(session);
        handle_io_progression(session);
    }
}

static void HandlePendingTCPStream(HAPPlatformTCPStreamManagerRef tcpStreamManager, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPAssert(tcpStreamManager == server->platform.ip.tcpStreamManager);

    HAPError err;

    HAPPlatformTCPStreamRef tcpStream;
    err = HAPPlatformTCPStreamManagerAcceptTCPStream(HAPNonnull(server->platform.ip.tcpStreamManager), &tcpStream);
    if (err) {
        log_result(
                kHAPLogType_Error,
                "error:Function 'HAPPlatformTCPStreamManagerAcceptTCPStream' failed.",
                err,
                __func__,
                HAP_FILE,
                __LINE__);
        return;
    }

    // Find free IP session.
    HAPIPSession* ipSession = NULL;
    for (size_t i = 0; i < server->ip.storage->numSessions; i++) {
        HAPIPSessionDescriptor* descriptor = (HAPIPSessionDescriptor*) &server->ip.storage->sessions[i].descriptor;
        if (!descriptor->server) {
            ipSession = &server->ip.storage->sessions[i];
            break;
        }
    }
    if (!ipSession) {
        HAPLog(&logObject,
               "Failed to allocate session."
               " (Number of supported accessory server sessions should be consistent with"
               " the maximum number of concurrent streams supported by TCP stream manager.)");
        HAPPlatformTCPStreamClose(HAPNonnull(server->platform.ip.tcpStreamManager), tcpStream);
        return;
    }

    HAPIPSessionDescriptor* t = (HAPIPSessionDescriptor*) &ipSession->descriptor;
    HAPRawBufferZero(t, sizeof *t);
    t->server = server_;
    t->tcpStream = tcpStream;
    t->tcpStreamIsOpen = true;
    t->state = kHAPIPSessionState_Idle;
    t->stamp = HAPPlatformClockGetCurrent();
    t->securitySession.isOpen = false;
    t->securitySession.isSecured = false;
    t->inboundBuffer.position = 0;
    t->inboundBuffer.limit = ipSession->inboundBuffer.numBytes;
    t->inboundBuffer.capacity = ipSession->inboundBuffer.numBytes;
    t->inboundBuffer.data = ipSession->inboundBuffer.bytes;
    t->inboundBufferMark = 0;
    t->outboundBuffer.position = 0;
    t->outboundBuffer.limit = ipSession->outboundBuffer.numBytes;
    t->outboundBuffer.capacity = ipSession->outboundBuffer.numBytes;
    t->outboundBuffer.data = ipSession->outboundBuffer.bytes;
    t->eventNotifications = ipSession->eventNotifications;
    t->maxEventNotifications = ipSession->numEventNotifications;
    t->numEventNotifications = 0;
    t->numEventNotificationFlags = 0;
    t->eventNotificationStamp = 0;
    t->timedWriteExpirationTime = 0;
    t->timedWritePID = 0;
    OpenSecuritySession(t);
    t->state = kHAPIPSessionState_Reading;
    prepare_reading_request(t);
    HAPAssert(t->tcpStreamIsOpen);
    HAPPlatformTCPStreamEvent interests = { .hasBytesAvailable = true, .hasSpaceAvailable = false };
    HAPPlatformTCPStreamUpdateInterests(
            HAPNonnull(server->platform.ip.tcpStreamManager), t->tcpStream, interests, HandleTCPStreamEvent, t);

    RegisterSession(t);

    HAPLogDebug(&logObject, "session:%p:accepted", (const void*) t);
}

static void engine_init(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPLogDebug(
            &logObject,
            "Storage configuration: ipAccessoryServerStorage = %lu",
            (unsigned long) sizeof *server->ip.storage);
    HAPLogDebug(
            &logObject, "Storage configuration: numSessions = %lu", (unsigned long) server->ip.storage->numSessions);
    HAPLogDebug(
            &logObject,
            "Storage configuration: sessions = %lu",
            (unsigned long) (server->ip.storage->numSessions * sizeof(HAPIPSession)));
    for (size_t i = 0; i < server->ip.storage->numSessions;) {
        size_t j;
        for (j = i + 1; j < server->ip.storage->numSessions; j++) {
            if (server->ip.storage->sessions[j].inboundBuffer.numBytes !=
                        server->ip.storage->sessions[i].inboundBuffer.numBytes ||
                server->ip.storage->sessions[j].outboundBuffer.numBytes !=
                        server->ip.storage->sessions[i].outboundBuffer.numBytes ||
                server->ip.storage->sessions[j].numEventNotifications !=
                        server->ip.storage->sessions[i].numEventNotifications) {
                break;
            }
        }
        if (i == j - 1) {
            HAPLogDebug(
                    &logObject,
                    "Storage configuration: sessions[%lu].inboundBuffer.numBytes = %lu",
                    (unsigned long) i,
                    (unsigned long) server->ip.storage->sessions[i].inboundBuffer.numBytes);
            HAPLogDebug(
                    &logObject,
                    "Storage configuration: sessions[%lu].outboundBuffer.numBytes = %lu",
                    (unsigned long) i,
                    (unsigned long) server->ip.storage->sessions[i].outboundBuffer.numBytes);
            HAPLogDebug(
                    &logObject,
                    "Storage configuration: sessions[%lu].numEventNotifications = %lu",
                    (unsigned long) i,
                    (unsigned long) server->ip.storage->sessions[i].numEventNotifications);
            HAPLogDebug(
                    &logObject,
                    "Storage configuration: sessions[%lu].eventNotifications = %lu",
                    (unsigned long) i,
                    (unsigned long) (server->ip.storage->sessions[i].numEventNotifications * sizeof(HAPIPEventNotificationRef)));
        } else {
            HAPLogDebug(
                    &logObject,
                    "Storage configuration: sessions[%lu...%lu].inboundBuffer.numBytes = %lu",
                    (unsigned long) i,
                    (unsigned long) j - 1,
                    (unsigned long) server->ip.storage->sessions[i].inboundBuffer.numBytes);
            HAPLogDebug(
                    &logObject,
                    "Storage configuration: sessions[%lu...%lu].outboundBuffer.numBytes = %lu",
                    (unsigned long) i,
                    (unsigned long) j - 1,
                    (unsigned long) server->ip.storage->sessions[i].outboundBuffer.numBytes);
            HAPLogDebug(
                    &logObject,
                    "Storage configuration: sessions[%lu...%lu].numEventNotifications = %lu",
                    (unsigned long) i,
                    (unsigned long) j - 1,
                    (unsigned long) server->ip.storage->sessions[i].numEventNotifications);
            HAPLogDebug(
                    &logObject,
                    "Storage configuration: sessions[%lu...%lu].eventNotifications = %lu",
                    (unsigned long) i,
                    (unsigned long) j - 1,
                    (unsigned long) (server->ip.storage->sessions[i].numEventNotifications * sizeof(HAPIPEventNotificationRef)));
        }
        i = j;
    }
    HAPLogDebug(
            &logObject,
            "Storage configuration: numReadContexts = %lu",
            (unsigned long) server->ip.storage->numReadContexts);
    HAPLogDebug(
            &logObject,
            "Storage configuration: readContexts = %lu",
            (unsigned long) (server->ip.storage->numReadContexts * sizeof(HAPIPReadContextRef)));
    HAPLogDebug(
            &logObject,
            "Storage configuration: numWriteContexts = %lu",
            (unsigned long) server->ip.storage->numWriteContexts);
    HAPLogDebug(
            &logObject,
            "Storage configuration: writeContexts = %lu",
            (unsigned long) (server->ip.storage->numWriteContexts * sizeof(HAPIPWriteContextRef)));
    HAPLogDebug(
            &logObject,
            "Storage configuration: scratchBuffer.numBytes = %lu",
            (unsigned long) server->ip.storage->scratchBuffer.numBytes);

    HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Undefined);

    server->ip.state = kHAPIPAccessoryServerState_Idle;
    server->ip.nextState = kHAPIPAccessoryServerState_Undefined;
}

HAP_RESULT_USE_CHECK
static HAPError engine_deinit(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Idle);

    HAPIPAccessoryServerStorage* storage = HAPNonnull(server->ip.storage);

    HAPAssert(storage->readContexts);
    HAPRawBufferZero(storage->readContexts, storage->numReadContexts * sizeof *storage->readContexts);

    HAPAssert(storage->writeContexts);
    HAPRawBufferZero(storage->writeContexts, storage->numWriteContexts * sizeof *storage->writeContexts);

    HAPAssert(storage->scratchBuffer.bytes);
    HAPRawBufferZero(storage->scratchBuffer.bytes, storage->scratchBuffer.numBytes);

    server->ip.state = kHAPIPAccessoryServerState_Undefined;

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPAccessoryServerState engine_get_state(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    switch (server->ip.state) {
        case kHAPIPAccessoryServerState_Undefined: {
            HAPPrecondition(false);
        } break;
        case kHAPIPAccessoryServerState_Idle: {
            return kHAPAccessoryServerState_Idle;
        }
        case kHAPIPAccessoryServerState_Running: {
            return kHAPAccessoryServerState_Running;
        }
        case kHAPIPAccessoryServerState_Stopping: {
            if (server->ip.nextState == kHAPIPAccessoryServerState_Running) {
                return kHAPAccessoryServerState_Running;
            } else {
                HAPAssert(server->ip.nextState == kHAPIPAccessoryServerState_Idle);
                return kHAPAccessoryServerState_Stopping;
            }
        }
    }

    HAPFatalError();
}

static void handle_server_state_transition_timer(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    (void) server;
    HAPPrecondition(timer == server->ip.stateTransitionTimer);
    server->ip.stateTransitionTimer = 0;

    HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Stopping);
    schedule_max_idle_time_timer(server_);
}

static void schedule_server_state_transition(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    (void) server;

    HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Stopping);

    HAPError err;

    if (!server->ip.stateTransitionTimer) {
        err = HAPPlatformTimerRegister(
                &server->ip.stateTransitionTimer, 0, handle_server_state_transition_timer, server_);
        if (err) {
            HAPLog(&logObject, "Not enough resources to schedule accessory server state transition!");
            HAPFatalError();
        }
        HAPAssert(server->ip.stateTransitionTimer);
    }
}

static void engine_start(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPAssert(server->ip.state == kHAPIPAccessoryServerState_Idle);

    HAPLogDebug(&logObject, "Starting server engine.");

    server->ip.state = kHAPIPAccessoryServerState_Running;
    HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);

    HAPAssert(!HAPPlatformTCPStreamManagerIsListenerOpen(HAPNonnull(server->platform.ip.tcpStreamManager)));

    HAPPlatformTCPStreamManagerOpenListener(
            HAPNonnull(server->platform.ip.tcpStreamManager), HandlePendingTCPStream, server_);
    HAPAssert(HAPPlatformTCPStreamManagerIsListenerOpen(HAPNonnull(server->platform.ip.tcpStreamManager)));
    publish_homeKit_service(server_);
}

HAP_RESULT_USE_CHECK
static HAPError engine_stop(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPLogDebug(&logObject, "Stopping server engine.");

    if (server->ip.state == kHAPIPAccessoryServerState_Running) {
        HAPAssert(server->ip.nextState == kHAPIPAccessoryServerState_Undefined);
        server->ip.state = kHAPIPAccessoryServerState_Stopping;
        server->ip.nextState = kHAPIPAccessoryServerState_Idle;
        HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);
        schedule_server_state_transition(server_);
    } else if (server->ip.state == kHAPIPAccessoryServerState_Stopping) {
        if (server->ip.nextState == kHAPIPAccessoryServerState_Running) {
            server->ip.nextState = kHAPIPAccessoryServerState_Idle;
        } else {
            HAPAssert(server->ip.nextState == kHAPIPAccessoryServerState_Idle);
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPError engine_raise_event_on_session_(
        HAPAccessoryServerRef* server_,
        const HAPCharacteristic* characteristic_,
        const HAPService* service_,
        const HAPAccessory* accessory_,
        const HAPSessionRef* securitySession_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPPrecondition(characteristic_);
    HAPPrecondition(service_);
    HAPPrecondition(accessory_);

    HAPError err;

    size_t events_raised = 0;

    uint64_t aid = accessory_->aid;
    uint64_t iid = ((const HAPBaseCharacteristic*) characteristic_)->iid;

    for (size_t i = 0; i < server->ip.storage->numSessions; i++) {
        HAPIPSession* ipSession = &server->ip.storage->sessions[i];
        HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) &ipSession->descriptor;
        if (!session->server) {
            continue;
        }
        if (session->securitySession.type != kHAPIPSecuritySessionType_HAP) {
            if (!securitySession_) {
                HAPLogDebug(&logObject, "Not flagging event pending on non-HAP session.");
            }
            continue;
        }
        if (securitySession_ && (securitySession_ != &session->securitySession._.hap)) {
            continue;
        }
        if (HAPSessionIsTransient(&session->securitySession._.hap)) {
            HAPLogDebug(&logObject, "Not flagging event pending on transient session.");
            continue;
        }

        if ((ipSession != server->ip.characteristicWriteRequestContext.ipSession) ||
            (characteristic_ != server->ip.characteristicWriteRequestContext.characteristic) ||
            (service_ != server->ip.characteristicWriteRequestContext.service) ||
            (accessory_ != server->ip.characteristicWriteRequestContext.accessory)) {
            HAPAssert(session->numEventNotifications <= session->maxEventNotifications);
            size_t j = 0;
            while ((j < session->numEventNotifications) &&
                   ((((HAPIPEventNotification*) &session->eventNotifications[j])->aid != aid) ||
                    (((HAPIPEventNotification*) &session->eventNotifications[j])->iid != iid))) {
                j++;
            }
            HAPAssert(
                    (j == session->numEventNotifications) ||
                    ((j < session->numEventNotifications) &&
                     (((HAPIPEventNotification*) &session->eventNotifications[j])->aid == aid) &&
                     (((HAPIPEventNotification*) &session->eventNotifications[j])->iid == iid)));
            if ((j < session->numEventNotifications) &&
                !((HAPIPEventNotification*) &session->eventNotifications[j])->flag) {
                ((HAPIPEventNotification*) &session->eventNotifications[j])->flag = true;
                session->numEventNotificationFlags++;
                events_raised++;
            }
        }
    }

    if (events_raised) {
        if (server->ip.eventNotificationTimer) {
            HAPPlatformTimerDeregister(server->ip.eventNotificationTimer);
            server->ip.eventNotificationTimer = 0;
        }
        err = HAPPlatformTimerRegister(&server->ip.eventNotificationTimer, 0, handle_event_notification_timer, server_);
        if (err) {
            HAPLog(&logObject, "Not enough resources to schedule event notification timer!");
            HAPFatalError();
        }
        HAPAssert(server->ip.eventNotificationTimer);
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPError engine_raise_event(
        HAPAccessoryServerRef* server,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(server);
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    return engine_raise_event_on_session_(server, characteristic, service, accessory, /* session: */ NULL);
}

HAP_RESULT_USE_CHECK
static HAPError engine_raise_event_on_session(
        HAPAccessoryServerRef* server,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        const HAPSessionRef* session) {
    HAPPrecondition(server);
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(session);

    return engine_raise_event_on_session_(server, characteristic, service, accessory, session);
}

static void Create(HAPAccessoryServerRef* server_, const HAPAccessoryServerOptions* options) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPPrecondition(server->platform.ip.tcpStreamManager);
    HAPPrecondition(server->platform.ip.serviceDiscovery);

    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_ARMCC(2570)
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe1444)

    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe1444)
    HAP_DIAGNOSTIC_POP

    // Initialize IP storage.
    HAPPrecondition(options->ip.accessoryServerStorage);
    HAPIPAccessoryServerStorage* storage = options->ip.accessoryServerStorage;
    HAPPrecondition(storage->readContexts);
    HAPPrecondition(storage->writeContexts);
    HAPPrecondition(storage->scratchBuffer.bytes);
    HAPPrecondition(storage->sessions);
    HAPPrecondition(storage->numSessions);
    for (size_t i = 0; i < storage->numSessions; i++) {
        HAPIPSession* session = &storage->sessions[i];
        HAPPrecondition(session->inboundBuffer.bytes);
        HAPPrecondition(session->outboundBuffer.bytes);
        HAPPrecondition(session->eventNotifications);
    }
    HAPRawBufferZero(storage->readContexts, storage->numReadContexts * sizeof *storage->readContexts);
    HAPRawBufferZero(storage->writeContexts, storage->numWriteContexts * sizeof *storage->writeContexts);
    HAPRawBufferZero(storage->scratchBuffer.bytes, storage->scratchBuffer.numBytes);
    for (size_t i = 0; i < storage->numSessions; i++) {
        HAPIPSession* ipSession = &storage->sessions[i];
        HAPRawBufferZero(&ipSession->descriptor, sizeof ipSession->descriptor);
        HAPRawBufferZero(ipSession->inboundBuffer.bytes, ipSession->inboundBuffer.numBytes);
        HAPRawBufferZero(ipSession->outboundBuffer.bytes, ipSession->outboundBuffer.numBytes);
        HAPRawBufferZero(
                ipSession->eventNotifications,
                ipSession->numEventNotifications * sizeof *ipSession->eventNotifications);
    }
    server->ip.storage = options->ip.accessoryServerStorage;

    // Install server engine.
    HAPNonnull(server->transports.ip)->serverEngine.install();
}

static void PrepareStart(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPIPAccessoryServerStorage* storage = HAPNonnull(server->ip.storage);
    HAPRawBufferZero(storage->readContexts, storage->numReadContexts * sizeof *storage->readContexts);
    HAPRawBufferZero(storage->writeContexts, storage->numWriteContexts * sizeof *storage->writeContexts);
    HAPRawBufferZero(storage->scratchBuffer.bytes, storage->scratchBuffer.numBytes);
    for (size_t i = 0; i < storage->numSessions; i++) {
        HAPIPSession* ipSession = &storage->sessions[i];
        HAPRawBufferZero(&ipSession->descriptor, sizeof ipSession->descriptor);
        HAPRawBufferZero(ipSession->inboundBuffer.bytes, ipSession->inboundBuffer.numBytes);
        HAPRawBufferZero(ipSession->outboundBuffer.bytes, ipSession->outboundBuffer.numBytes);
        HAPRawBufferZero(
                ipSession->eventNotifications,
                ipSession->numEventNotifications * sizeof *ipSession->eventNotifications);
    }
}

static void WillStart(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
}

static void PrepareStop(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
}

static void HAPSessionInvalidateDependentIPState(HAPAccessoryServerRef* server_, HAPSessionRef* session) {
    HAPPrecondition(server_);
    HAPPrecondition(session);
}

static const HAPAccessoryServerServerEngine* _Nullable _serverEngine;

static void HAPAccessoryServerInstallServerEngine(void) {
    HAPPrecondition(!_serverEngine);

    _serverEngine = &HAPIPAccessoryServerServerEngine;
}

static void HAPAccessoryServerUninstallServerEngine(void) {
    _serverEngine = NULL;
}

static const HAPAccessoryServerServerEngine* _Nullable HAPAccessoryServerGetServerEngine(void) {
    return _serverEngine;
}

const HAPIPAccessoryServerTransport kHAPAccessoryServerTransport_IP = {
    .create = Create,
    .prepareStart = PrepareStart,
    .willStart = WillStart,
    .prepareStop = PrepareStop,
    .session = { .invalidateDependentIPState = HAPSessionInvalidateDependentIPState },
    .serverEngine = { .install = HAPAccessoryServerInstallServerEngine,
                      .uninstall = HAPAccessoryServerUninstallServerEngine,
                      .get = HAPAccessoryServerGetServerEngine }
};

const HAPAccessoryServerServerEngine HAPIPAccessoryServerServerEngine = { .init = engine_init,
                                                                          .deinit = engine_deinit,
                                                                          .get_state = engine_get_state,
                                                                          .start = engine_start,
                                                                          .stop = engine_stop,
                                                                          .raise_event = engine_raise_event,
                                                                          .raise_event_on_session =
                                                                                  engine_raise_event_on_session };

HAP_RESULT_USE_CHECK
size_t HAPAccessoryServerGetIPSessionIndex(const HAPAccessoryServerRef* server_, const HAPSessionRef* session) {
    HAPPrecondition(server_);
    const HAPAccessoryServer* server = (const HAPAccessoryServer*) server_;
    HAPPrecondition(session);

    const HAPIPAccessoryServerStorage* storage = HAPNonnull(server->ip.storage);

    for (size_t i = 0; i < storage->numSessions; i++) {
        HAPIPSessionDescriptor* t = (HAPIPSessionDescriptor*) &storage->sessions[i].descriptor;
        if (!t->server) {
            continue;
        }
        if (t->securitySession.type != kHAPIPSecuritySessionType_HAP) {
            continue;
        }
        if (&t->securitySession._.hap == session) {
            return i;
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
bool HAPIPSessionAreEventNotificationsEnabled(
        HAPIPSessionDescriptorRef* session_,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(session_);
    HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) session_;
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    uint64_t aid = accessory->aid;
    uint64_t iid = ((const HAPBaseCharacteristic*) characteristic)->iid;

    size_t i = 0;
    while ((i < session->numEventNotifications) &&
           ((((HAPIPEventNotification*) &session->eventNotifications[i])->aid != aid) ||
            (((HAPIPEventNotification*) &session->eventNotifications[i])->iid != iid))) {
        i++;
    }
    HAPAssert(
            (i == session->numEventNotifications) ||
            ((i < session->numEventNotifications) &&
             (((HAPIPEventNotification*) &session->eventNotifications[i])->aid == aid) &&
             (((HAPIPEventNotification*) &session->eventNotifications[i])->iid == iid)));

    return i < session->numEventNotifications;
}

void HAPIPSessionHandleReadRequest(
        HAPIPSessionDescriptorRef* session_,
        HAPIPSessionContext sessionContext,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPIPSessionReadResult* readResult,
        HAPIPByteBuffer* dataBuffer) {
    HAPPrecondition(session_);
    HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) session_;
    HAPPrecondition(session->server);
    HAPPrecondition(session->securitySession.type == kHAPIPSecuritySessionType_HAP);
    HAPPrecondition(session->securitySession.isOpen);
    HAPPrecondition(session->securitySession.isSecured || kHAPIPAccessoryServer_SessionSecurityDisabled);
    HAPPrecondition(!HAPSessionIsTransient(&session->securitySession._.hap));
    HAPPrecondition(readResult);
    HAPPrecondition(dataBuffer);

    HAPRawBufferZero(readResult, sizeof *readResult);

    const HAPBaseCharacteristic* baseCharacteristic = (const HAPBaseCharacteristic*) characteristic;

    HAPIPReadContext readContext;
    HAPRawBufferZero(&readContext, sizeof readContext);

    readContext.aid = accessory->aid;
    readContext.iid = baseCharacteristic->iid;

    if (!HAPCharacteristicReadRequiresAdminPermissions(baseCharacteristic) ||
        HAPSessionControllerIsAdmin(&session->securitySession._.hap)) {
        if (baseCharacteristic->properties.readable) {
            if ((sessionContext != kHAPIPSessionContext_EventNotification) &&
                HAPUUIDAreEqual(
                        baseCharacteristic->characteristicType, &kHAPCharacteristicType_ProgrammableSwitchEvent)) {
                // A read of this characteristic must always return a null value for IP accessories.
                // See HomeKit Accessory Protocol Specification R14
                // Section 9.75 Programmable Switch Event
                readResult->status = kHAPIPAccessoryServerStatusCode_Success;
                readResult->value.unsignedIntValue = 0;
            } else if (
                    (sessionContext == kHAPIPSessionContext_GetAccessories) &&
                    baseCharacteristic->properties.ip.controlPoint) {
                readResult->status = kHAPIPAccessoryServerStatusCode_UnableToPerformOperation;
            } else {
                handle_characteristic_read_request(
                        session, characteristic, service, accessory, (HAPIPReadContextRef*) &readContext, dataBuffer);
                readResult->status = readContext.status;
                switch (baseCharacteristic->format) {
                    case kHAPCharacteristicFormat_Bool:
                    case kHAPCharacteristicFormat_UInt8:
                    case kHAPCharacteristicFormat_UInt16:
                    case kHAPCharacteristicFormat_UInt32:
                    case kHAPCharacteristicFormat_UInt64: {
                        readResult->value.unsignedIntValue = readContext.value.unsignedIntValue;
                    } break;
                    case kHAPCharacteristicFormat_Int: {
                        readResult->value.intValue = readContext.value.intValue;
                    } break;
                    case kHAPCharacteristicFormat_Float: {
                        readResult->value.floatValue = readContext.value.floatValue;
                    } break;
                    case kHAPCharacteristicFormat_Data:
                    case kHAPCharacteristicFormat_String:
                    case kHAPCharacteristicFormat_TLV8: {
                        readResult->value.stringValue.bytes = readContext.value.stringValue.bytes;
                        readResult->value.stringValue.numBytes = readContext.value.stringValue.numBytes;
                    } break;
                }
            }
        } else {
            readResult->status = kHAPIPAccessoryServerStatusCode_ReadFromWriteOnlyCharacteristic;
        }
    } else {
        readResult->status = kHAPIPAccessoryServerStatusCode_InsufficientPrivileges;
    }
}

#endif
