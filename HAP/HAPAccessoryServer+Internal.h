// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_ACCESSORY_SERVER_INTERNAL_H
#define HAP_ACCESSORY_SERVER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * IP specific accessory server state.
 */
HAP_ENUM_BEGIN(uint8_t, HAPIPAccessoryServerState) { /** Server state is undefined. */
                                                     kHAPIPAccessoryServerState_Undefined,

                                                     /** Server is initialized but not running. */
                                                     kHAPIPAccessoryServerState_Idle,

                                                     /** Server is running. */
                                                     kHAPIPAccessoryServerState_Running,

                                                     /** Server is shutting down. */
                                                     kHAPIPAccessoryServerState_Stopping
} HAP_ENUM_END(uint8_t, HAPIPAccessoryServerState);

typedef struct {
    /** Transports. */
    struct {
        /** HAP over IP. */
        const HAPIPAccessoryServerTransport* _Nullable ip;

        /** HAP over Bluetooth LE. */
        const HAPBLEAccessoryServerTransport* _Nullable ble;
    } transports;

    /** Accessory server state. */
    HAPAccessoryServerState state;

    /** Platform. */
    HAPPlatform platform;

    /** Callbacks. */
    HAPAccessoryServerCallbacks callbacks;

    /** Timer that on expiry triggers pending callbacks. */
    HAPPlatformTimerRef callbackTimer;

    /** Maximum number of allowed pairings. */
    HAPPlatformKeyValueStoreKey maxPairings;

    /** Accessory to serve. */
    const HAPAccessory* _Nullable primaryAccessory;

    /** Apple Authentication Coprocessor manager. */
    HAPMFiHWAuth mfi;

    /** Pairing identity. */
    struct {
        /** Long-term public key. */
        uint8_t ed_LTPK[ED25519_PUBLIC_KEY_BYTES];

        /** Long-term secret key. */
        HAPAccessoryServerLongTermSecretKey ed_LTSK;
    } identity;

    /**
     * Accessory setup state.
     */
    struct {
        /** Timer until the dynamic setup info needs to be refreshed. */
        HAPPlatformTimerRef dynamicRefreshTimer;

        /** Timer until NFC pairing mode expires. 0 if NFC pairing mode is not active. */
        HAPPlatformTimerRef nfcPairingModeTimer;

        /**
         * Current setup info state.
         */
        struct {
            /**
             * Setup info.
             */
            HAPSetupInfo setupInfo;

            /** Setup code (display / programmable NFC). */
            HAPSetupCode setupCode;

            /** Whether Setup info has been loaded / generated. */
            bool setupInfoIsAvailable : 1;

            /** Whether Setup code has been loaded / generated. */
            bool setupCodeIsAvailable : 1;

            /** Whether setup info should be kept on expiration of timers. */
            bool lockSetupInfo : 1;

            /** Whether setup info should be kept across pairing attempts. */
            bool keepSetupInfo : 1;
        } state;
    } accessorySetup;

    /**
     * Pair Setup procedure state.
     */
    struct {
        /**
         * Session where the current pairing takes place.
         * NULL if no pairing in progress.
         */
        HAPSessionRef* _Nullable sessionThatIsCurrentlyPairing;

        /**
         * Time at which the current pairing operation was started.
         * Undefined if no pairing in progress.
         */
        HAPTime operationStartTime;

        uint8_t A[SRP_PUBLIC_KEY_BYTES];                 // M2, M4
        uint8_t b[SRP_SECRET_KEY_BYTES];                 // M2, M4
        uint8_t B[SRP_PUBLIC_KEY_BYTES];                 // M2, M4
        uint8_t K[SRP_SESSION_KEY_BYTES];                // SRP Session Key
        uint8_t SessionKey[CHACHA20_POLY1305_KEY_BYTES]; // SessionKey for the Pair Setup procedure

        uint8_t M1[SRP_PROOF_BYTES];
        uint8_t M2[SRP_PROOF_BYTES];

        /** Pairing Type flags. */
        uint32_t flags;

        bool flagsPresent : 1;  /**< Whether Pairing Type flags were present in Pair Setup M1. */
        bool keepSetupInfo : 1; /**< Whether setup info should be kept on disconnect. */
    } pairSetup;

    /**
     * IP specific attributes.
     */
    struct {
        /** Storage. */
        HAPIPAccessoryServerStorage* _Nullable storage;

        /** NULL-terminated array of bridged accessories for a bridge accessory. NULL otherwise. */
        const HAPAccessory* _Nullable const* _Nullable bridgedAccessories;

        /** IP specific accessory server state. */
        HAPIPAccessoryServerState state;

        /** Next IP specific accessory server state after state transition is completed. */
        HAPIPAccessoryServerState nextState;

        /** Flag indicating whether the HAP service is currently discoverable. */
        bool isServiceDiscoverable;

        /** The number of active sessions served by the accessory server. */
        size_t numSessions;

        /**
         * Characteristic write request context.
         */
        struct {
            /** The session over which the request has been received. */
            const HAPIPSession* _Nullable ipSession;

            /** The characteristic whose value is to be written. */
            const HAPCharacteristic* _Nullable characteristic;

            /** The service that contains the characteristic. */
            const HAPService* _Nullable service;

            /** The accessory that provides the service. */
            const HAPAccessory* _Nullable accessory;
        } characteristicWriteRequestContext;

        /** Timer that on expiry triggers a server state transition. */
        HAPPlatformTimerRef stateTransitionTimer;

        /** Timer that on expiry schedules pending event notifications. */
        HAPPlatformTimerRef eventNotificationTimer;

        /** Timer that on expiry runs the garbage task. */
        HAPPlatformTimerRef garbageCollectionTimer;

        /** Timer that on expiry schedules a maximum idle time check. */
        HAPPlatformTimerRef maxIdleTimeTimer;

        /** Currently registered Bonjour service. */
        HAPIPServiceDiscoveryType discoverableService;
    } ip;

    /**
     * BLE specific attributes.
     */
    struct {
        /**
         * Storage.
         *
         * - Implementation note: For now, we use only one procedure.
         */
        HAPBLEAccessoryServerStorage* _Nullable storage;

        /**
         * Connection information.
         */
        struct {
            /**
             * Information about the currently written characteristic.
             */
            struct {
                /** Characteristic being written. */
                const HAPCharacteristic* _Nullable characteristic;

                /** The service that contains the characteristic. */
                const HAPCharacteristic* _Nullable service;

                /** The accessory that provides the service. */
                const HAPAccessory* _Nullable accessory;
            } write;

            /** Connection handle of the connected controller, if applicable. */
            HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle;

            /** Whether a HomeKit controller is connected. */
            bool connected : 1;

            /** Whether the HAP-BLE procedure is attached. */
            bool procedureAttached : 1;
        } connection;

        /** Timestamp for Least Recently Used scheme in Pair Resume session cache. */
        uint32_t sessionCacheTimestamp;

        /**
         * Advertisement state.
         */
        struct {
            /** Preferred regular advertising interval. */
            HAPBLEAdvertisingInterval interval;

            /** Preferred duration of events in ms. */
            uint16_t ev_duration;

            HAPPlatformTimerRef timer; /**< Timer until advertisement parameters change. */

            HAPPlatformTimerRef fast_timer; /**< Timer until fast initial advertising completes. */
            bool fast_started : 1;          /**< Whether the fast advertising has been started. */

            bool connected : 1; /**< Whether a controller is connected. */

            /**
             * Broadcasted Events.
             *
             * @see HomeKit Accessory Protocol Specification R14
             *      Section 7.4.2.2.2 Manufacturer Data
             */
            struct {
                /** Broadcast interval. */
                HAPBLECharacteristicBroadcastInterval interval;

                /**
                 * Characteristic instance ID, if broadcasted event active. 0 otherwise.
                 *
                 * - For Bluetooth LE, instance IDs cannot exceed UINT16_MAX.
                 */
                uint16_t iid;

                /** Value. */
                uint8_t value[8];
            } broadcastedEvent;
        } adv;
    } ble;

    /** Client context pointer. */
    void* _Nullable context;
} HAPAccessoryServer;
HAP_STATIC_ASSERT(sizeof(HAPAccessoryServerRef) >= sizeof(HAPAccessoryServer), HAPAccessoryServer);

/**
 * Gets client context for the accessory server.
 *
 * @param      server               Accessory server.
 *
 * @return Client context passed to HAPAccessoryServerCreate.
 */
HAP_RESULT_USE_CHECK
void* _Nullable HAPAccessoryServerGetClientContext(HAPAccessoryServerRef* server);

/**
 * Schedules invocation of the accessory server's handleUpdatedState callback.
 *
 * @param      server               Accessory server.
 */
void HAPAccessoryServerDelegateScheduleHandleUpdatedState(HAPAccessoryServerRef* server);

/**
 * Loads the accessory server LTSK. If none exists, it is generated.
 *
 * @param      keyValueStore        Key-value store.
 * @param[out] ltsk                 Long-term secret key of the accessory server.
 */
void HAPAccessoryServerLoadLTSK(HAPPlatformKeyValueStoreRef keyValueStore, HAPAccessoryServerLongTermSecretKey* ltsk);

/**
 * IP protocol version string.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 6.6.3 IP Protocol Version
 */
#define kHAPProtocolVersion_IP "1.1.0"

/**
 * IP protocol version string (short).
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 6.6.3 IP Protocol Version
 */
#define kHAPShortProtocolVersion_IP "1.1"

/**
 * BLE protocol version string.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.4.3.1 BLE Protocol Version Characteristic
 */
#define kHAPProtocolVersion_BLE "2.2.0"

/**
 * Returns whether the accessory supports Apple Authentication Coprocessor.
 *
 * @param      server               Accessory server.
 *
 * @return true                     If the accessory supports Apple Authentication Coprocessor.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPAccessoryServerSupportsMFiHWAuth(HAPAccessoryServerRef* server);

/**
 * Returns the Pairing Feature flags.
 *
 * @param      server               Accessory server.
 *
 * @return Pairing feature flags
 */
HAP_RESULT_USE_CHECK
uint8_t HAPAccessoryServerGetPairingFeatureFlags(HAPAccessoryServerRef* server);

/**
 * Returns the Status flags.
 *
 * @param      server               Accessory server.
 *
 * @return Status flags
 */
HAP_RESULT_USE_CHECK
uint8_t HAPAccessoryServerGetStatusFlags(HAPAccessoryServerRef* server);

/**
 * Updates advertising data.
 *
 * @param      server               Accessory server.
 */
void HAPAccessoryServerUpdateAdvertisingData(HAPAccessoryServerRef* server);

/**
 * If the last remaining admin controller pairing is removed, all pairings on the accessory must be removed.
 *
 * This must be called when:
 * - the accessory server is started (to handle potential power failure during key-value store operations).
 * - a pairing is removed.
 *
 * @param      server               Accessory server.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 5.11 Remove Pairing
 */
HAP_RESULT_USE_CHECK
HAPError HAPAccessoryServerCleanupPairings(HAPAccessoryServerRef* server);

/**
 * Get configuration number.
 *
 * @param      keyValueStore        Key-value store.
 * @param[out] cn                   Configuration number.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPAccessoryServerGetCN(HAPPlatformKeyValueStoreRef keyValueStore, uint16_t* cn);

/**
 * Increments configuration number.
 *
 * - IP: Must be called when an accessory, service or characteristic is added or removed from the accessory server.
 *
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPAccessoryServerIncrementCN(HAPPlatformKeyValueStoreRef keyValueStore);

/**
 * Resets HomeKit state after a firmware update has occurred.
 *
 * - Prior to calling, make sure that the accessory server is not running.
 *
 * @param      server               Accessory server.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleFirmwareUpdate(HAPAccessoryServerRef* server);

/**
 * Informs the application that a controller subscribed to updates of characteristic value.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the subscription state was changed.
 * @param      characteristic       The characteristic whose value has changed.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 */
void HAPAccessoryServerHandleSubscribe(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory);

/**
 * Informs the application that a controller unsubscribed from updates of characteristic value.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the subscription state was changed.
 * @param      characteristic       The characteristic whose value has changed.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 */
void HAPAccessoryServerHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory);

/**
 * Returns whether a service is supported in the context of a given accessory server and over a
 * given transport type.
 *
 * - Certain services are only applicable to certain types of accessory server configurations or
 *   certain types of transports.
 *
 * @param      server               Accessory server.
 * @param      transportType        Transport type.
 * @param      service              Service.
 *
 * @return true                     If the service is supported.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPAccessoryServerSupportsService(
        HAPAccessoryServerRef* server,
        HAPTransportType transportType,
        const HAPService* service);

/**
 * Index of a service with a given type within an attribute database.
 *
 * - This is a zero-based index specific to the service type.
 *
 * - This is useful to compress a (HAPAccessory, HAPService) tuple.
 *   Use HAPAccessoryServerGetServiceFromServiceTypeIndex to fetch the (HAPAccessory, HAPService) tuple.
 *   There can be at most 150 accessories with 100 services each. 150 * 100 = 15000 -> UInt16.
 */
typedef uint16_t HAPServiceTypeIndex;

/**
 * Gets the number of service instances with a given type within an attribute database.
 *
 * @param      server               Accessory server.
 * @param      serviceType          The type of the service.
 *
 * @return Number of services with a given type within the accessory server's attribute database.
 */
HAP_RESULT_USE_CHECK
size_t HAPAccessoryServerGetNumServiceInstances(HAPAccessoryServerRef* server, const HAPUUID* serviceType);

/**
 * Gets the index of a service for later lookup.
 *
 * @param      server               Accessory server.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 *
 * @return Index of service within the accessory server's attribute database.
 */
HAP_RESULT_USE_CHECK
HAPServiceTypeIndex HAPAccessoryServerGetServiceTypeIndex(
        HAPAccessoryServerRef* server,
        const HAPService* service,
        const HAPAccessory* accessory);

/**
 * Gets a service by type and index.
 *
 * @param      server               Accessory server.
 * @param      serviceType          The type of the service.
 * @param      serviceTypeIndex     Index of the service as received through HAPAccessoryServerGetServiceTypeIndex.
 * @param[out] service              Service.
 * @param[out] accessory            The accessory that provides the service.
 */
void HAPAccessoryServerGetServiceFromServiceTypeIndex(
        HAPAccessoryServerRef* server,
        const HAPUUID* serviceType,
        HAPServiceTypeIndex serviceTypeIndex,
        const HAPService* _Nonnull* _Nonnull service,
        const HAPAccessory* _Nonnull* _Nonnull accessory);

/**
 * Callback that should be invoked for each HAP session.
 *
 * @param      context              Context.
 * @param      server               Accessory server.
 * @param      session              Session.
 * @param[in,out] shouldContinue    True if enumeration shall continue, False otherwise. Is set to true on input.
 */
typedef void (*HAPAccessoryServerEnumerateSessionsCallback)(
        void* _Nullable context,
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        bool* shouldContinue);

/**
 * Enumerates all connected HAP sessions associated with an accessory server.
 *
 * @param      server               Accessory server.
 * @param      callback             Function to call on each configured HAP session.
 * @param      context              Context that is passed to the callback.
 */
void HAPAccessoryServerEnumerateConnectedSessions(
        HAPAccessoryServerRef* server,
        HAPAccessoryServerEnumerateSessionsCallback callback,
        void* _Nullable context);

/**
 * Searches for an IP session corresponding to a given HAP session and returns the session index.
 *
 * @param      server               Accessory server.
 * @param      session              The session to search for.
 *
 * @return The index of the IP session.
 */
HAP_RESULT_USE_CHECK
size_t HAPAccessoryServerGetIPSessionIndex(const HAPAccessoryServerRef* server, const HAPSessionRef* session);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
