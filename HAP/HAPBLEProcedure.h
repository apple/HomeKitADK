// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_PROCEDURE_H
#define HAP_BLE_PROCEDURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Procedure.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.5 HAP Procedures
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEProcedureMultiTransactionType) { /** No procedure in progress. */
                                                               kHAPBLEProcedureMultiTransactionType_None,

                                                               /** HAP Characteristic Timed Write Procedure. */
                                                               kHAPBLEProcedureMultiTransactionType_TimedWrite
} HAP_ENUM_END(uint8_t, HAPBLEProcedureMultiTransactionType);

/**
 * Procedure.
 */
typedef struct {
    /**@cond */
    /** Accessory server that the procedure is attached to. */
    HAPAccessoryServerRef* server;

    /** Session that the procedure is attached to. */
    HAPSessionRef* session;

    /** Characteristic that the procedure is attached to. */
    const HAPCharacteristic* characteristic;

    /** The service that contains the characteristic. */
    const HAPService* service;

    /** The accessory that provides the service. */
    const HAPAccessory* accessory;

    /** Transaction state. */
    HAPBLETransaction transaction;

    /** Value buffer. */
    void* scratchBytes;

    /** Value buffer length. */
    size_t numScratchBytes;

    /** Procedure timer. Starts on first GATT write. Ends on last GATT read. */
    HAPPlatformTimerRef procedureTimer;

    /** Active multi-transaction procedure. */
    HAPBLEProcedureMultiTransactionType multiTransactionType;

    /** Procedure is secure. */
    bool startedSecured;

    /**
     * Procedure specific elements.
     */
    union {
        /** No active procedure. */
        void* none;

        /**
         * HAP Characteristic Timed Write Procedure.
         */
        struct {
            HAPTime timedWriteStartTime; /**< Time when Timed Write request was received. */
            HAPTLVReaderRef bodyReader;  /**< Timed Write Body Reader. */
        } timedWrite;
    } _;
    /**@endcond */
} HAPBLEProcedure;
HAP_STATIC_ASSERT(sizeof(HAPBLEProcedureRef) >= sizeof(HAPBLEProcedure), HAPBLEProcedure);

/**
 * Attaches a procedure to a characteristic.
 *
 * @param[out] bleProcedure         Procedure.
 * @param      scratchBytes         Value buffer to use.
 * @param      numScratchBytes      Capacity of value buffer.
 * @param      server               Accessory server to attach to.
 * @param      session              Session to attach to.
 * @param      characteristic       Characteristic to attach to.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 */
void HAPBLEProcedureAttach(
        HAPBLEProcedureRef* bleProcedure,
        void* scratchBytes,
        size_t numScratchBytes,
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory);

/**
 * Deinitializes a procedure.
 *
 * @param      bleProcedure         Procedure.
 */
void HAPBLEProcedureDestroy(HAPBLEProcedureRef* bleProcedure);

/**
 * Gets the characteristic that a procedure is attached to.
 *
 * @param      bleProcedure         Procedure.
 *
 * @return Attached characteristic.
 */
const HAPCharacteristic* HAPBLEProcedureGetAttachedCharacteristic(const HAPBLEProcedureRef* bleProcedure);

/**
 * Queries a procedure to determine whether a transaction is currently in progress.
 * When no transaction is in progress, it is safe to attach the procedure to a different characteristic
 * through another #HAPBLEProcedureAttach invocation without losing data.
 *
 * - This function only works for procedures that are attached to a characteristic.
 *
 * @param      bleProcedure         Procedure.
 *
 * @return true                     If a procedure is in progress.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPBLEProcedureIsInProgress(const HAPBLEProcedureRef* bleProcedure);

/**
 * Processes a GATT Write request.
 *
 * @param      bleProcedure         Procedure.
 * @param      bytes                Body of the GATT Write request.
 * @param      numBytes             Length of body.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state. Link should be terminated.
 * @return kHAPError_InvalidData    If the controller sent a malformed request. Link should be terminated.
 * @return kHAPError_OutOfResources If there are not enough resources to handle the request. Link should be terminated.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEProcedureHandleGATTWrite(HAPBLEProcedureRef* bleProcedure, void* bytes, size_t numBytes);

/**
 * Processes a GATT Read request.
 *
 * @param      bleProcedure         Procedure.
 * @param[out] bytes                Buffer to serialize response into.
 * @param      maxBytes             Capacity of buffer.
 * @param[out] numBytes             Length of response put into buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state. Link should be terminated.
 * @return kHAPError_OutOfResources If buffer not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError
        HAPBLEProcedureHandleGATTRead(HAPBLEProcedureRef* bleProcedure, void* bytes, size_t maxBytes, size_t* numBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
