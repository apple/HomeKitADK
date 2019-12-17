// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLEProcedure" };

// Set this flag to disable all BLE procedure timeouts.
#define DEBUG_DISABLE_TIMEOUTS (false)

void HAPBLEProcedureAttach(
        HAPBLEProcedureRef* bleProcedure_,
        void* scratchBytes,
        size_t numScratchBytes,
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(bleProcedure_);
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;
    HAPPrecondition(scratchBytes);
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    HAPRawBufferZero(bleProcedure, sizeof *bleProcedure);
    bleProcedure->scratchBytes = scratchBytes;
    bleProcedure->numScratchBytes = numScratchBytes;
    bleProcedure->server = server;
    bleProcedure->session = session_;
    bleProcedure->characteristic = characteristic;
    bleProcedure->service = service;
    bleProcedure->accessory = accessory;
    HAPBLETransactionCreate(&bleProcedure->transaction, bleProcedure->scratchBytes, bleProcedure->numScratchBytes);
    bleProcedure->procedureTimer = 0;
}

void HAPBLEProcedureDestroy(HAPBLEProcedureRef* bleProcedure_) {
    HAPPrecondition(bleProcedure_);
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;

    HAPLogDebug(&logObject, "%s", __func__);

    if (bleProcedure->procedureTimer) {
#if !DEBUG_DISABLE_TIMEOUTS
        HAPPlatformTimerDeregister(bleProcedure->procedureTimer);
#endif
        bleProcedure->procedureTimer = 0;
    }
}

static void HAPBLEProcedureReset(HAPBLEProcedureRef* bleProcedure_) {
    HAPPrecondition(bleProcedure_);
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;

    void* scratchBytes = bleProcedure->scratchBytes;
    size_t numScratchBytes = bleProcedure->numScratchBytes;
    HAPAccessoryServerRef* server = bleProcedure->server;
    HAPSessionRef* session = bleProcedure->session;
    const HAPCharacteristic* characteristic = bleProcedure->characteristic;
    const HAPService* service = bleProcedure->service;
    const HAPAccessory* accessory = bleProcedure->accessory;

    HAPBLEProcedureDestroy(bleProcedure_);
    HAPBLEProcedureAttach(
            bleProcedure_, scratchBytes, numScratchBytes, server, session, characteristic, service, accessory);
}

#if !DEBUG_DISABLE_TIMEOUTS
static void ProcedureTimerExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPBLEProcedureRef* bleProcedure_ = context;
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;
    HAPPrecondition(timer == bleProcedure->procedureTimer);
    bleProcedure->procedureTimer = 0;

    HAPLogDebug(&logObject, "%s", __func__);

    // Any procedure that times out [...] shall result in the current HAP secure session being invalidated and a new
    // session may be established by the controller.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.1 HAP Transactions and Procedures.
    //
    // 12. Accessory must reject GATT Read Requests on a HAP characteristic if it was not preceded by an
    // GATT Write Request with the same transaction ID at most 10 seconds prior.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.5 Testing Bluetooth LE Accessories
    //
    // 39. Accessories must implement a 10 second HAP procedure timeout, all HAP procedures [...] must complete within
    // 10 seconds, if a procedure fails to complete within the procedure timeout the accessory must drop the security
    // session and also drop the Bluetooth link.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.5 Testing Bluetooth LE Accessories
    //
    // ==> Having a 10 second procedure timeout that on expiry drops the security session and Bluetooth link fulfills
    // all of these requirements. Since it is not defined when a procedure starts or ends, we assume that the timeout
    // runs from the very first GATT write request until the very last GATT read request, and assume that there are no
    // excess GATT read requests with empty fragments.

    HAPBLEProcedureReset(bleProcedure_);
    HAPSessionInvalidate(bleProcedure->server, bleProcedure->session, /* terminateLink: */ true);
}
#endif

const HAPCharacteristic* HAPBLEProcedureGetAttachedCharacteristic(const HAPBLEProcedureRef* bleProcedure_) {
    HAPPrecondition(bleProcedure_);
    const HAPBLEProcedure* bleProcedure = (const HAPBLEProcedure*) bleProcedure_;

    return bleProcedure->characteristic;
}

HAP_RESULT_USE_CHECK
bool HAPBLEProcedureIsInProgress(const HAPBLEProcedureRef* bleProcedure_) {
    HAPPrecondition(bleProcedure_);
    const HAPBLEProcedure* bleProcedure = (const HAPBLEProcedure*) bleProcedure_;

    return bleProcedure->procedureTimer != 0;
}

/**
 * Destroys request body and creates response body writer.
 *
 * @param      bleProcedure_        Procedure.
 * @param[out] responseWriter       Writer.
 */
static void DestroyRequestBodyAndCreateResponseBodyWriter(
        HAPBLEProcedureRef* bleProcedure_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(bleProcedure_);
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;
    HAPPrecondition(responseWriter);

    size_t numBytes = bleProcedure->numScratchBytes;
    if (numBytes > UINT16_MAX) {
        // Maximum for HAP-BLE PDU. Note that the characteristic value TLV is even further limited by spec.
        numBytes = UINT16_MAX;
    }
    HAPTLVWriterCreate(responseWriter, bleProcedure->scratchBytes, numBytes);
}

#define SEND_ERROR_AND_RETURN(STATUS) \
    do { \
        HAPBLETransactionSetResponse(&bleProcedure->transaction, (STATUS), NULL); \
        return kHAPError_None; \
    } while (0)

#define SEND_RESPONSE_AND_RETURN(BODY) \
    do { \
        HAPBLETransactionSetResponse(&bleProcedure->transaction, kHAPBLEPDUStatus_Success, (BODY)); \
        return kHAPError_None; \
    } while (0)

/**
 * Handles a HAP-BLE transaction.
 *
 * @param      bleProcedure_        Procedure.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPBLEProcedureProcessTransaction(HAPBLEProcedureRef* bleProcedure_) {
    HAPPrecondition(bleProcedure_);
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;
    HAPPrecondition(bleProcedure->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) bleProcedure->server;
    HAPPrecondition(bleProcedure->accessory);
    const HAPAccessory* accessory = bleProcedure->accessory;
    HAPPrecondition(bleProcedure->service);
    const HAPService* service = bleProcedure->service;
    HAPPrecondition(bleProcedure->characteristic);
    const HAPBaseCharacteristic* characteristic = bleProcedure->characteristic;

    HAPError err;

    // Get request.
    HAPBLETransactionRequest request;
    err = HAPBLETransactionGetRequest(&bleProcedure->transaction, &request);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
    }

    // Validate opcode.
    if (!HAPPDUIsValidOpcode(request.opcode)) {
        // If an accessory receives a HAP PDU with an opcode that it does not support it shall reject the PDU and
        // respond with a status code Unsupported PDU in its HAP response.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.3.3.2 HAP Request Format
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Rejected request with unsupported opcode: 0x%02x.",
                request.opcode);
        SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
    }

    // Check that HAP request's characteristic / service instance ID matches the addressed characteristic's instance ID.
    HAPAssert(service->iid <= UINT16_MAX);
    HAPAssert(characteristic->iid <= UINT16_MAX);
    uint16_t iid;
    if (HAPBLEPDUOpcodeIsServiceOperation(request.opcode)) {
        iid = (uint16_t) service->iid;
    } else {
        iid = (uint16_t) characteristic->iid;
    }
    if (request.iid != iid) {
        if (HAPBLEPDUOpcodeIsServiceOperation(request.opcode)) {
            HAPLogService(
                    &logObject,
                    service,
                    accessory,
                    "Request's IID [00000000%08X] does not match the addressed IID.",
                    request.iid);
        } else {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Request's IID [00000000%08X] does not match the addressed IID.",
                    request.iid);
        }

        if (request.opcode == kHAPPDUOpcode_ServiceSignatureRead) {
            // If the accessory receives an invalid (eg., 0) service instance ID in the
            // HAP-Service-Signature-Read-Request, it must respond with a valid HAP-Service-Signature-Read-Response with
            // Svc Properties set to 0 and Linked Svc (if applicable) set to 0 length.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.4.13 HAP-Service-Signature-Read-Response
        } else {
            SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidInstanceID);
        }
    }

    // Handle request.
    bool hasReturnResponse = true;
    switch (request.opcode) {
        case kHAPPDUOpcode_ServiceSignatureRead: {
            // Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogService(
                        &logObject,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Service-Signature-Read-Request");
                return kHAPError_InvalidState;
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 7.4.4.5.4 Service Signature Characteristic

            if (!HAPBLECharacteristicSupportsServiceProcedures(characteristic)) {
                HAPLogService(
                        &logObject,
                        service,
                        accessory,
                        "Rejected %s: Not supported.",
                        "HAP-Service-Signature-Read-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // HAP-Service-Signature-Read-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(bleProcedure_, &writer);

            // Serialize HAP-Service-Signature-Read-Response.
            err = HAPBLEServiceGetSignatureReadResponse(request.iid == iid ? service : NULL, &writer);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }

            SEND_RESPONSE_AND_RETURN(&writer);
        }
        case kHAPPDUOpcode_CharacteristicSignatureRead: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Characteristic-Signature-Read-Request");
                return kHAPError_InvalidState;
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.1 HAP Characteristic Signature Read Procedure

            // The characteristics `Pair Setup`, `Pair Verify` and `Pairing Features` of `Pairing Service`
            // do not support "Paired Read" and "Paired Write" and only support the
            // `HAP Characteristic Signature Read Procedure` without a secure session.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.1 HAP Characteristic Signature Read Procedure
            if (HAPSessionIsSecured(bleProcedure->session) &&
                HAPBLECharacteristicDropsSecuritySession(characteristic)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only non-secure access is permitted.",
                        "HAP-Characteristic-Signature-Read-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // HAP-Characteristic-Signature-Read-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(bleProcedure_, &writer);

            // Serialize HAP-Characteristic-Signature-Read-Response.
            err = HAPBLECharacteristicGetSignatureReadResponse(characteristic, service, &writer);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }
            SEND_RESPONSE_AND_RETURN(&writer);
        }
        case kHAPPDUOpcode_CharacteristicConfiguration: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Characteristic-Configuration-Request");
                return kHAPError_InvalidState;
            }
            if (HAPSessionIsTransient(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Session is transient.",
                        "HAP-Characteristic-Configuration-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.8 HAP Characteristic Configuration Procedure

            if (!HAPSessionIsSecured(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only secure access is permitted.",
                        "HAP-Characteristic-Configuration-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // Handle HAP-Characteristic-Configuration-Request.
            err = HAPBLECharacteristicHandleConfigurationRequest(
                    characteristic, service, accessory, &request.bodyReader, server->platform.keyValueStore);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_InvalidData);
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Request handling failed with error %d.",
                        "HAP-Characteristic-Configuration-Request",
                        err);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }

            // HAP-Characteristic-Configuration-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(bleProcedure_, &writer);

            // Serialize HAP-Characteristic-Configuration-Response.
            err = HAPBLECharacteristicGetConfigurationResponse(
                    characteristic, service, accessory, &writer, server->platform.keyValueStore);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_OutOfResources);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }
            SEND_RESPONSE_AND_RETURN(&writer);
        }
        case kHAPPDUOpcode_ProtocolConfiguration: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogService(
                        &logObject,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Protocol-Configuration-Request");
                return kHAPError_InvalidState;
            }
            if (HAPSessionIsTransient(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Session is transient.",
                        "HAP-Protocol-Configuration-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.9 HAP Protocol Configuration Procedure

            if (!HAPBLECharacteristicSupportsServiceProcedures(characteristic)) {
                HAPLogService(
                        &logObject,
                        service,
                        accessory,
                        "Rejected %s: Not supported.",
                        "HAP-Protocol-Configuration-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            if (!service->properties.ble.supportsConfiguration) {
                HAPLogService(
                        &logObject,
                        service,
                        accessory,
                        "Rejected %s: Service does not support configuration.",
                        "HAP-Protocol-Configuration-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            if (!HAPSessionIsSecured(bleProcedure->session)) {
                HAPLogService(
                        &logObject,
                        service,
                        accessory,
                        "Rejected %s: Only secure access is permitted.",
                        "HAP-Protocol-Configuration-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // Handle HAP-Protocol-Configuration-Request.
            bool didRequestGetAll;
            err = HAPBLEProtocolHandleConfigurationRequest(
                    bleProcedure->server,
                    bleProcedure->session,
                    service,
                    accessory,
                    &request.bodyReader,
                    &didRequestGetAll,
                    server->platform.keyValueStore);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_InvalidData);
                HAPLogService(
                        &logObject,
                        service,
                        accessory,
                        "Rejected %s: Request handling failed with error %d.",
                        "HAP-Protocol-Configuration-Request",
                        err);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }
            if (!didRequestGetAll) {
                SEND_RESPONSE_AND_RETURN(NULL);
            }

            // HAP-Protocol-Configuration-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(bleProcedure_, &writer);

            // Serialize HAP-Protocol-Configuration-Response.
            err = HAPBLEProtocolGetConfigurationResponse(
                    bleProcedure->server,
                    bleProcedure->session,
                    service,
                    accessory,
                    &writer,
                    server->platform.keyValueStore);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_OutOfResources);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }
            SEND_RESPONSE_AND_RETURN(&writer);
        }
        case kHAPPDUOpcode_Token: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Token-Request");
                return kHAPError_InvalidState;
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 5.15.1 HAP-Token-Request

            if (!HAPUUIDAreEqual(service->serviceType, &kHAPServiceType_HAPProtocolInformation) ||
                !HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_ServiceSignature)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only supported on the Service Signature characteristic in the "
                        "HAP Protocol Information Service.",
                        "HAP-Token-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            if (!HAPSessionIsSecured(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only secure access is permitted.",
                        "HAP-Token-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // HAP-Token-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(bleProcedure_, &writer);

            // Serialize HAP-Token-Response.
            err = HAPMFiTokenAuthGetTokenResponse(bleProcedure->server, bleProcedure->session, accessory, &writer);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
                HAPLogAccessory(
                        &logObject,
                        accessory,
                        "Rejected %s: Request handling failed with error %u.",
                        "HAP-Token-Request",
                        err);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }
            SEND_RESPONSE_AND_RETURN(&writer);
        }
        case kHAPPDUOpcode_TokenUpdate: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Token-Update-Request");
                return kHAPError_InvalidState;
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 5.15.3 HAP-Token-Update-Request

            if (!HAPUUIDAreEqual(service->serviceType, &kHAPServiceType_HAPProtocolInformation) ||
                !HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_ServiceSignature)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only supported on the Service Signature characteristic in the "
                        "HAP Protocol Information Service.",
                        "HAP-Token-Update-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            if (!HAPSessionIsSecured(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only secure access is permitted.",
                        "HAP-Token-Update-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // Handle HAP-Token-Update-Request.
            err = HAPMFiTokenAuthHandleTokenUpdateRequest(
                    bleProcedure->server, bleProcedure->session, accessory, &request.bodyReader);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_InvalidData);
                HAPLogAccessory(
                        &logObject,
                        accessory,
                        "Rejected %s: Request handling failed with error %u.",
                        "HAP-Token-Update-Request",
                        err);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }

            // Send HAP-Token-Update-Response.
            SEND_RESPONSE_AND_RETURN(NULL);
        }
        case kHAPPDUOpcode_Info: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Info-Request");
                return kHAPError_InvalidState;
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 5.15.5 HAP-Info-Request

            if (!HAPUUIDAreEqual(service->serviceType, &kHAPServiceType_HAPProtocolInformation) ||
                !HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_ServiceSignature)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only supported on Service Signature characteristic in the "
                        "HAP Protocol Information Service.",
                        "HAP-Info-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            if (!HAPSessionIsSecured(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only secure access is permitted.",
                        "HAP-Info-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // HAP-Info-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(bleProcedure_, &writer);

            // Serialize HAP-Info-Response.
            err = HAPAccessoryGetInfoResponse(bleProcedure->server, bleProcedure->session, accessory, &writer);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_OutOfResources);
                HAPLogAccessory(
                        &logObject,
                        accessory,
                        "Rejected %s: Request handler failed with error %d.",
                        "HAP-Info-Request",
                        err);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }
            SEND_RESPONSE_AND_RETURN(&writer);
        };
        case kHAPPDUOpcode_CharacteristicTimedWrite: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Characteristic-Timed-Write-Request");
                return kHAPError_InvalidState;
            }
            if (HAPSessionIsTransient(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Session is transient.",
                        "HAP-Characteristic-Timed-Write-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.4 HAP Characteristic Timed Write Procedure

            // Cache body.
            bleProcedure->multiTransactionType = kHAPBLEProcedureMultiTransactionType_TimedWrite;
            HAPAssert(sizeof bleProcedure->_.timedWrite.bodyReader == sizeof request.bodyReader);
            HAPRawBufferCopyBytes(
                    &bleProcedure->_.timedWrite.bodyReader,
                    &request.bodyReader,
                    sizeof bleProcedure->_.timedWrite.bodyReader);

            // The accessory must start the TTL timer after sending the HAP-Characteristic-Timed-Write-Response.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.4 HAP Characteristic Timed Write Procedure
            bleProcedure->_.timedWrite.timedWriteStartTime = HAPPlatformClockGetCurrent();
            SEND_RESPONSE_AND_RETURN(NULL);
        }
        case kHAPPDUOpcode_CharacteristicExecuteWrite: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_TimedWrite) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: No timed write in progress.",
                        "HAP-Characteristic-Execute-Write-Request");
                return kHAPError_InvalidState;
            }
            HAPAssert(!HAPSessionIsTransient(bleProcedure->session));

            bleProcedure->multiTransactionType = kHAPBLEProcedureMultiTransactionType_None;

            HAPAssert(sizeof request.bodyReader == sizeof bleProcedure->_.timedWrite.bodyReader);
            HAPRawBufferCopyBytes(
                    &request.bodyReader, &bleProcedure->_.timedWrite.bodyReader, sizeof request.bodyReader);

            // Although undocumented, the pending Timed Write request may also include the Return Response flag and AAD.

            // Request body has been restored and is still available.
        } // Fallthrough.
        case kHAPPDUOpcode_CharacteristicWrite: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Characteristic-Write-Request");
                return kHAPError_InvalidState;
            }
            if (HAPSessionIsTransient(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Session is transient.",
                        "HAP-Characteristic-Write-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.2 HAP Characteristic Write Procedure

            // Fetch permissions.
            bool supportsWrite = characteristic->properties.ble.writableWithoutSecurity;
            bool supportsSecureWrite = characteristic->properties.writable;

            // Unpaired Identify must be allowed only if the accessory is unpaired, i.e. it has no paired controllers.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.4.1.9 Unpaired Identify
            if (HAPUUIDAreEqual(characteristic->characteristicType, &kHAPCharacteristicType_Identify)) {
                supportsWrite = !HAPAccessoryServerIsPaired(bleProcedure->server);
            }

            // Check permissions.
            bool sessionIsSecured = HAPSessionIsSecured(bleProcedure->session);
            if (!sessionIsSecured && !supportsWrite) {
                if (supportsSecureWrite) {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Rejected %s: Only secure writes are supported.",
                            "HAP-Characteristic-Write-Request");
                    SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InsufficientAuthentication);
                } else {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Rejected %s: Not supported.",
                            "HAP-Characteristic-Write-Request");
                    SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
                }
            }
            if (sessionIsSecured && !supportsSecureWrite) {
                if (supportsWrite) {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Rejected %s: Only non-secure writes are supported.",
                            "HAP-Characteristic-Write-Request");
                    SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
                } else {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Rejected %s: Not supported.",
                            "HAP-Characteristic-Write-Request");
                    SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
                }
            }
            if (HAPCharacteristicWriteRequiresAdminPermissions(characteristic) &&
                !HAPSessionControllerIsAdmin(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Requires controller to have admin permissions.",
                        "HAP-Characteristic-Write-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InsufficientAuthentication);
            }

            // Check for Timed Write requirement.
            bool isTimedWrite = request.opcode == kHAPPDUOpcode_CharacteristicExecuteWrite;
            bool requiresTimedWrite = characteristic->properties.requiresTimedWrite;
            if (!isTimedWrite && requiresTimedWrite) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Only timed writes are supported.",
                        "HAP-Characteristic-Write-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }

            // Destroy request body and process HAP-Characteristic-Write-Request.
            HAPAssert(server->ble.connection.connected);
            HAPAssert(!server->ble.connection.write.characteristic);
            HAPAssert(!server->ble.connection.write.service);
            HAPAssert(!server->ble.connection.write.accessory);
            server->ble.connection.write.characteristic = characteristic;
            server->ble.connection.write.service = service;
            server->ble.connection.write.accessory = accessory;
            bool hasExpired;
            err = HAPBLECharacteristicParseAndWriteValue(
                    bleProcedure->server,
                    bleProcedure->session,
                    characteristic,
                    service,
                    accessory,
                    &request.bodyReader,
                    isTimedWrite ? &bleProcedure->_.timedWrite.timedWriteStartTime : NULL,
                    &hasExpired,
                    &hasReturnResponse);
            server->ble.connection.write.characteristic = NULL;
            server->ble.connection.write.service = NULL;
            server->ble.connection.write.accessory = NULL;
            if (err == kHAPError_NotAuthorized) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Write failed due to insufficient authorization.",
                        "HAP-Characteristic-Write-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InsufficientAuthorization);
            } else if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_Busy);
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Write failed with error %d.",
                        "HAP-Characteristic-Write-Request",
                        err);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }
            if (hasExpired) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Timed Write expired.",
                        "HAP-Characteristic-Write-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }
            if (!hasReturnResponse) {
                if (characteristic->properties.ip.supportsWriteResponse) {
                    // The supportsWriteResponse characteristic property provides a guarantee to the application
                    // that the characteristic's handleRead callback is always called after a successful handleWrite.
                    // Whether the controller actually requested write response is hidden from the application.
                    // Although write response is mainly used in the HAP over IP transport it makes sense
                    // to follow the same behaviour when such a characteristic is accessed using HAP over Bluetooth LE.
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Characteristic supports write response: Calling read handler.");
                } else {
                    SEND_RESPONSE_AND_RETURN(NULL);
                }
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.5 HAP Characteristic Write-with-Response Procedure.

            // Request body has been destroyed!
        } // Fallthrough.
        case kHAPPDUOpcode_CharacteristicRead: {
            // 10. Accessory must support only one HAP procedure on a characteristic at any point in time.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.5 Testing Bluetooth LE Accessories
            if (bleProcedure->multiTransactionType != kHAPBLEProcedureMultiTransactionType_None) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Different HAP procedure in progress.",
                        "HAP-Characteristic-Read-Request");
                return kHAPError_InvalidState;
            }
            if (HAPSessionIsTransient(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Session is transient.",
                        "HAP-Characteristic-Read-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
            }

            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.5.3 HAP Characteristic Read Procedure

            // Check permissions.
            bool sessionIsSecured = HAPSessionIsSecured(bleProcedure->session);
            bool supportsRead = characteristic->properties.ble.readableWithoutSecurity;
            bool supportsSecureRead = characteristic->properties.readable;
            if (!sessionIsSecured && !supportsRead) {
                if (supportsSecureRead) {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Rejected %s: Only secure reads are supported.",
                            "HAP-Characteristic-Read-Request");
                    SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InsufficientAuthentication);
                } else {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Rejected %s: Not supported.",
                            "HAP-Characteristic-Read-Request");
                    SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
                }
            }
            if (sessionIsSecured && !supportsSecureRead) {
                if (supportsRead) {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Rejected %s: Only non-secure reads are supported.",
                            "HAP-Characteristic-Read-Request");
                    SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
                } else {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Rejected %s: Not supported.",
                            "HAP-Characteristic-Read-Request");
                    SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_UnsupportedPDU);
                }
            }
            if (HAPCharacteristicReadRequiresAdminPermissions(characteristic) &&
                !HAPSessionControllerIsAdmin(bleProcedure->session)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Requires controller to have admin permissions.",
                        "HAP-Characteristic-Read-Request");
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InsufficientAuthentication);
            }

            // HAP-Characteristic-Read-Request ok.
            HAPTLVWriterRef writer;
            DestroyRequestBodyAndCreateResponseBodyWriter(bleProcedure_, &writer);

            // Serialize HAP-Characteristic-Read-Response.
            err = HAPBLECharacteristicReadAndSerializeValue(
                    bleProcedure->server, bleProcedure->session, characteristic, service, accessory, &writer);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Rejected %s: Read failed with error %d.",
                        "HAP-Characteristic-Read-Request",
                        err);
                SEND_ERROR_AND_RETURN(kHAPBLEPDUStatus_InvalidRequest);
            }
            if (hasReturnResponse) {
                SEND_RESPONSE_AND_RETURN(&writer);
            } else {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "HAP-Param-Return-Response not set: Discarding write response.");
                SEND_RESPONSE_AND_RETURN(NULL);
            }
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEProcedureHandleGATTWrite(HAPBLEProcedureRef* bleProcedure_, void* bytes, size_t numBytes) {
    HAPPrecondition(bleProcedure_);
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;
    HAPPrecondition(bleProcedure->session);
    HAPSession* session = (HAPSession*) bleProcedure->session;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);
    HAPPrecondition(bleProcedure->characteristic);
    const HAPBaseCharacteristic* characteristic = bleProcedure->characteristic;
    const HAPService* service HAP_UNUSED = bleProcedure->service;
    const HAPAccessory* accessory = bleProcedure->accessory;
    HAPPrecondition(bytes);

    HAPError err;

    // If session is terminal, no more requests may be accepted.
    if (HAPBLESessionIsTerminal(&session->_.ble)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Rejecting GATT write: Session is terminal. No more requests are accepted.");
        return kHAPError_InvalidState;
    }

    // Start new procedure on first GATT write request.
    if (!HAPBLEProcedureIsInProgress(bleProcedure_)) {
        // If session is soon terminal, it is very unlikely to complete a full HAP-BLE transaction in time.
        // Better to disconnect earlier than to end up with ambiguity whether the transaction completed successfully.
        if (HAPBLESessionIsTerminalSoon(&session->_.ble)) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Rejecting GATT write: Session is terminal soon. No new procedures are started.");
            return kHAPError_InvalidState;
        }

        // An accessory must cancel any pending procedures when a new HAP secure session starts getting established.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.3.1 HAP Transactions and Procedures
        if (HAPSessionIsSecured(bleProcedure->session) && HAPBLECharacteristicDropsSecuritySession(characteristic)) {
            HAPLogCharacteristicInfo(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Terminating existing security session (%s).",
                    "Characteristic drops security session");
            HAPSessionInvalidate(bleProcedure->server, bleProcedure->session, /* terminateLink: */ false);
            HAPBLEProcedureReset(bleProcedure_);
        }

        // Store security state for rest of procedure.
        // This is necessary as the last Pair Verify response needs to be sent unencrypted although the link is secured.
        bleProcedure->startedSecured = HAPSessionIsSecured(bleProcedure->session);

        // Start procedure.
        HAPAssert(!bleProcedure->procedureTimer);
#if !DEBUG_DISABLE_TIMEOUTS
        err = HAPPlatformTimerRegister(
                &bleProcedure->procedureTimer,
                HAPPlatformClockGetCurrent() + 10 * HAPSecond,
                ProcedureTimerExpired,
                bleProcedure);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Not enough resources to start procedure timer. Disconnecting immediately!");
            return err;
        }
#else
        bleProcedure->procedureTimer = 1;
#endif
        HAPBLESessionDidStartBLEProcedure(bleProcedure->server, bleProcedure->session);
    }

    // Decrypt if secured.
    if (bleProcedure->startedSecured) {
        if (numBytes < CHACHA20_POLY1305_TAG_BYTES) {
            // Auth tag not present.
            HAPLogCharacteristicBuffer(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    bytes,
                    numBytes,
                    "Secure request too short, auth tag not present.");
            return kHAPError_InvalidData;
        }
        err = HAPSessionDecryptControlMessage(bleProcedure->server, bleProcedure->session, bytes, bytes, numBytes);
        if (err) {
            // Decryption failed.
            HAPAssert(err == kHAPError_InvalidState || err == kHAPError_InvalidData);
            return err;
        }

        numBytes -= CHACHA20_POLY1305_TAG_BYTES;
    }

    HAPLogCharacteristicBufferDebug(
            &logObject,
            characteristic,
            service,
            accessory,
            bytes,
            numBytes,
            "< (%s)",
            bleProcedure->startedSecured ? "encrypted" : "plaintext");

    // Process PDU.
    err = HAPBLETransactionHandleWrite(&bleProcedure->transaction, bytes, numBytes);
    if (err) {
        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_InvalidData);
        return err;
    }

    // Report response being sent.
    HAPBLESessionDidSendGATTResponse(bleProcedure->server, bleProcedure->session);

    return kHAPError_None;
}

/**
 * Completes the current transaction.
 *
 * @param      bleProcedure_        Procedure.
 */
static void CompleteTransaction(HAPBLEProcedureRef* bleProcedure_) {
    HAPPrecondition(bleProcedure_);
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;
    HAPPrecondition(bleProcedure->server);
    HAPPrecondition(bleProcedure->session);
    HAPSession* session = (HAPSession*) bleProcedure->session;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);

    switch (bleProcedure->multiTransactionType) {
        case kHAPBLEProcedureMultiTransactionType_None: {
            // Procedure complete.
            HAPAssert(bleProcedure->procedureTimer);
#if !DEBUG_DISABLE_TIMEOUTS
            HAPPlatformTimerDeregister(bleProcedure->procedureTimer);
#endif
            bleProcedure->procedureTimer = 0;

            HAPBLETransactionCreate(
                    &bleProcedure->transaction, bleProcedure->scratchBytes, bleProcedure->numScratchBytes);
            return;
        }
        case kHAPBLEProcedureMultiTransactionType_TimedWrite: {
            // The buffer still stores the pending write request. It must not be reused for new requests until cleared.
            HAPBLETransactionCreate(&bleProcedure->transaction, NULL, 0);
            return;
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEProcedureHandleGATTRead(
        HAPBLEProcedureRef* bleProcedure_,
        void* bytes,
        size_t maxBytes,
        size_t* numBytes) {
    HAPPrecondition(bleProcedure_);
    HAPBLEProcedure* bleProcedure = (HAPBLEProcedure*) bleProcedure_;
    HAPPrecondition(bleProcedure->server);
    HAPPrecondition(bleProcedure->session);
    HAPSession* session = (HAPSession*) bleProcedure->session;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);
    HAPPrecondition(bleProcedure->characteristic);
    const HAPBaseCharacteristic* characteristic = bleProcedure->characteristic;
    const HAPService* service HAP_UNUSED = bleProcedure->service;
    const HAPAccessory* accessory = bleProcedure->accessory;
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    HAPError err;

    // If session is terminal, no more requests may be accepted.
    if (HAPBLESessionIsTerminal(&session->_.ble)) {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Rejecting GATT read: Session is terminal. No more requests are accepted.");
        return kHAPError_InvalidState;
    }

    // Encrypted packets have an auth tag in the end. Available capacity is lower in that case.
    if (bleProcedure->startedSecured) {
        if (maxBytes < CHACHA20_POLY1305_TAG_BYTES) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Secure response buffer does not have enough space for auth tag.");
            return kHAPError_OutOfResources;
        }
        maxBytes -= CHACHA20_POLY1305_TAG_BYTES;
    }

    // Process pending request.
    if (HAPBLETransactionIsRequestAvailable(&bleProcedure->transaction)) {
        err = HAPBLEProcedureProcessTransaction(bleProcedure_);
        if (err) {
            HAPAssert(err == kHAPError_InvalidState);
            return err;
        }
    }

    // Prepare next response fragment.
    bool isFinalFragment;
    err = HAPBLETransactionHandleRead(&bleProcedure->transaction, bytes, maxBytes, numBytes, &isFinalFragment);
    if (err) {
        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
        return err;
    }

    HAPLogCharacteristicBufferDebug(
            &logObject,
            characteristic,
            service,
            accessory,
            bytes,
            *numBytes,
            "> (%s)",
            bleProcedure->startedSecured ? "encrypted" : "plaintext");

    // Encrypt if secured.
    if (bleProcedure->startedSecured) {
        err = HAPSessionEncryptControlMessage(bleProcedure->server, bleProcedure->session, bytes, bytes, *numBytes);
        if (err) {
            // Encryption failed.
            HAPAssert(err == kHAPError_InvalidState);
            return err;
        }

        *numBytes += CHACHA20_POLY1305_TAG_BYTES;
    }

    // If all fragments have been sent, complete the transaction.
    if (isFinalFragment) {
        CompleteTransaction(bleProcedure_);
    }

    // Report response being sent.
    HAPBLESessionDidSendGATTResponse(bleProcedure->server, bleProcedure->session);

    // Handle completed procedure.
    if (!HAPBLEProcedureIsInProgress(bleProcedure_)) {
        // If security session has been closed, invalidate session.
        if (bleProcedure->startedSecured && !HAPSessionIsSecured(bleProcedure->session)) {
            HAPSessionInvalidate(bleProcedure->server, bleProcedure->session, /* terminateLink: */ true);
        }
    }

    return kHAPError_None;
}
