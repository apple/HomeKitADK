// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_IP_ACCESSORY_SERVER_H
#define HAP_IP_ACCESSORY_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

typedef struct {
    void (*init)(HAPAccessoryServerRef* p_srv);
    HAP_RESULT_USE_CHECK
    HAPError (*deinit)(HAPAccessoryServerRef* p_srv);
    HAP_RESULT_USE_CHECK
    HAPAccessoryServerState (*get_state)(HAPAccessoryServerRef* p_srv);
    void (*start)(HAPAccessoryServerRef* p_srv);
    HAP_RESULT_USE_CHECK
    HAPError (*stop)(HAPAccessoryServerRef* p_srv);
    HAP_RESULT_USE_CHECK
    HAPError (*raise_event)(
            HAPAccessoryServerRef* server,
            const HAPCharacteristic* characteristic,
            const HAPService* service,
            const HAPAccessory* accessory);
    HAP_RESULT_USE_CHECK
    HAPError (*raise_event_on_session)(
            HAPAccessoryServerRef* server,
            const HAPCharacteristic* characteristic,
            const HAPService* service,
            const HAPAccessory* accessory,
            const HAPSessionRef* session);
} HAPAccessoryServerServerEngine;

extern const HAPAccessoryServerServerEngine HAPIPAccessoryServerServerEngine;

struct HAPIPAccessoryServerTransport {
    void (*create)(HAPAccessoryServerRef* server, const HAPAccessoryServerOptions* options);

    void (*prepareStart)(HAPAccessoryServerRef* server);

    void (*willStart)(HAPAccessoryServerRef* server);

    void (*prepareStop)(HAPAccessoryServerRef* server);

    struct {
        void (*invalidateDependentIPState)(HAPAccessoryServerRef* server_, HAPSessionRef* session);
    } session;

    struct {
        void (*install)(void);

        void (*uninstall)(void);

        const HAPAccessoryServerServerEngine* _Nullable (*_Nonnull get)(void);
    } serverEngine;
};

/**
 * Session type.
 */
HAP_ENUM_BEGIN(uint8_t, HAPIPSecuritySessionType) { /** HAP session. */
                                                    kHAPIPSecuritySessionType_HAP = 1,

                                                    /** MFiSAP session. */
                                                    kHAPIPSecuritySessionType_MFiSAP
} HAP_ENUM_END(uint8_t, HAPIPSecuritySessionType);

/**
 * Session.
 */
typedef struct {
    /** Session type. */
    HAPIPSecuritySessionType type;

    /** Whether or not the session is open. */
    bool isOpen : 1;

    /** Whether or not a security session has been established. */
    bool isSecured : 1;

    /**
     * Whether or not the /config message has been received.
     *
     * - This sends FIN after the next response, and restarts the IP server after receiving FIN from controller.
     */
    bool receivedConfig : 1;

    /** Session. */
    union {
        /** HAP session. */
        HAPSessionRef hap;

        /** MFi SAP session. */
        struct {
            /** AES master context. */
            HAP_aes_ctr_ctx aesMasterContext;

            /** Whether or not the /configured message has been received. */
            bool receivedConfigured : 1;
        } mfiSAP;
    } _;
} HAPIPSecuritySession;

//----------------------------------------------------------------------------------------------------------------------

/**
 * Accessory server session state.
 */
HAP_ENUM_BEGIN(uint8_t, HAPIPSessionState) { /** Accessory server session is idle. */
                                             kHAPIPSessionState_Idle,

                                             /** Accessory server session is reading. */
                                             kHAPIPSessionState_Reading,

                                             /** Accessory server session is writing. */
                                             kHAPIPSessionState_Writing
} HAP_ENUM_END(uint8_t, HAPIPSessionState);

/**
 * HTTP/1.1 Content Type.
 */
HAP_ENUM_BEGIN(uint8_t, HAPIPAccessoryServerContentType) { /** Unknown HTTP/1.1 content type. */
                                                           kHAPIPAccessoryServerContentType_Unknown,

                                                           /** application/hap+json. */
                                                           kHAPIPAccessoryServerContentType_Application_HAPJSON,

                                                           /** application/octet-stream. */
                                                           kHAPIPAccessoryServerContentType_Application_OctetStream,

                                                           /** application/pairing+tlv8. */
                                                           kHAPIPAccessoryServerContentType_Application_PairingTLV8
} HAP_ENUM_END(uint8_t, HAPIPAccessoryServerContentType);

/**
 * IP specific event notification state.
 */
typedef struct {
    /** Accessory instance ID. */
    uint64_t aid;

    /** Characteristic instance ID. */
    uint64_t iid;

    /** Flag indicating whether an event has been raised for the given characteristic in the given accessory. */
    bool flag;
} HAPIPEventNotification;
HAP_STATIC_ASSERT(sizeof(HAPIPEventNotificationRef) >= sizeof(HAPIPEventNotification), event_notification);

/**
 * IP specific accessory server session descriptor.
 */
typedef struct {
    /** Accessory server serving this session. */
    HAPAccessoryServerRef* _Nullable server;

    /** TCP stream. */
    HAPPlatformTCPStreamRef tcpStream;

    /** Flag indicating whether the TCP stream is open. */
    bool tcpStreamIsOpen;

    /** IP session state. */
    HAPIPSessionState state;

    /** Time stamp of last activity on this session. */
    HAPTime stamp;

    /** Security session. */
    HAPIPSecuritySession securitySession;

    /** Inbound buffer. */
    HAPIPByteBuffer inboundBuffer;

    /** Marked inbound buffer position indicating the position until which the buffer has been decrypted. */
    size_t inboundBufferMark;

    /** Outbound buffer. */
    HAPIPByteBuffer outboundBuffer;

    /**
     * Marked outbound buffer position indicating the position until which the buffer has not yet been encrypted
     * (starting from outboundBuffer->limit).
     */
    size_t outboundBufferMark;

    /** HTTP reader. */
    struct util_http_reader httpReader;

    /** Current position of the HTTP reader in the inbound buffer. */
    size_t httpReaderPosition;

    /** Flag indication whether an error has been encountered while parsing a HTTP message. */
    bool httpParserError;

    /**
     * HTTP/1.1 Method.
     */
    struct {
        /**
         * Pointer to the HTTP/1.1 method in the inbound buffer.
         */
        char* _Nullable bytes;

        /**
         * Length of the HTTP/1.1 method in the inbound buffer.
         */
        size_t numBytes;
    } httpMethod;

    /**
     * HTTP/1.1 URI.
     */
    struct {
        /**
         * Pointer to the HTTP/1.1 URI in the inbound buffer.
         */
        char* _Nullable bytes;

        /**
         * Length of the HTTP/1.1 URI in the inbound buffer.
         */
        size_t numBytes;
    } httpURI;

    /**
     * HTTP/1.1 Header Field Name.
     */
    struct {
        /**
         * Pointer to the current HTTP/1.1 header field name in the inbound buffer.
         */
        char* _Nullable bytes;

        /**
         * Length of the current HTTP/1.1 header field name in the inbound buffer.
         */
        size_t numBytes;
    } httpHeaderFieldName;

    /**
     * HTTP/1.1 Header Field Value.
     */
    struct {
        /**
         * Pointer to the current HTTP/1.1 header value in the inbound buffer.
         */
        char* _Nullable bytes;

        /**
         * Length of the current HTTP/1.1 header value in the inbound buffer.
         */
        size_t numBytes;
    } httpHeaderFieldValue;

    /**
     * HTTP/1.1 Content Length.
     */
    struct {
        /**
         * Flag indicating whether a HTTP/1.1 content length is defined.
         */
        bool isDefined;

        /**
         * HTTP/1.1 content length.
         */
        size_t value;
    } httpContentLength;

    /**
     * HTTP/1.1 Content Type.
     */
    HAPIPAccessoryServerContentType httpContentType;

    /**
     * Array of event notification contexts on this session.
     */
    HAPIPEventNotificationRef* _Nullable eventNotifications;

    /**
     * The maximum number of events this session can handle.
     */
    size_t maxEventNotifications;

    /**
     * The number of subscribed events on this session.
     */
    size_t numEventNotifications;

    /**
     * The number of raised events on this session.
     */
    size_t numEventNotificationFlags;

    /**
     * Time stamp of last event notification on this session.
     */
    HAPTime eventNotificationStamp;

    /**
     * Time when the request expires. 0 if no timed write in progress.
     */
    HAPTime timedWriteExpirationTime;

    /**
     * PID of timed write. Must match "pid" of next PUT /characteristics.
     */
    uint64_t timedWritePID;

    /**
     * Serialization context for incremental accessory attribute database serialization.
     */
    HAPIPAccessorySerializationContext accessorySerializationContext;

    /**
     * Flag indicating whether incremental serialization of accessory attribute database is in progress.
     */
    bool accessorySerializationIsInProgress;
} HAPIPSessionDescriptor;
HAP_STATIC_ASSERT(sizeof(HAPIPSessionDescriptorRef) >= sizeof(HAPIPSessionDescriptor), HAPIPSessionDescriptor);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
