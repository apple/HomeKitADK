// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_TRANSACTION_H
#define HAP_BLE_TRANSACTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**@cond */
/**
 * Transaction state.
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLETransactionState) { /**
                                                   * Waiting for initial write.
                                                   */
                                                  kHAPBLETransactionState_WaitingForInitialWrite,

                                                  /**
                                                   * Reading request.
                                                   */
                                                  kHAPBLETransactionState_ReadingRequest,

                                                  /**
                                                   * Request has been retrieved. Waiting for response to be set.
                                                   */
                                                  kHAPBLETransactionState_HandlingRequest,

                                                  /**
                                                   * Waiting for initial read.
                                                   */
                                                  kHAPBLETransactionState_WaitingForInitialRead,

                                                  /**
                                                   * Writing response.
                                                   */
                                                  kHAPBLETransactionState_WritingResponse
} HAP_ENUM_END(uint8_t, HAPBLETransactionState);

/**
 * Transaction.
 */
typedef struct {
    HAPBLETransactionState state; /**< Transaction State. */
    union {
        struct {
            HAPPDUOpcode opcode; /**< HAP Opcode. */
            uint8_t tid;         /**< TID. Transaction Identifier. */
            uint16_t iid;        /**< CID. Characteristic / service instance ID. */

            void* _Nullable bodyBytes; /**< Combined body. */
            size_t maxBodyBytes;       /**< Combined body capacity. */
            size_t totalBodyBytes;     /**< Combined body length. */
            size_t bodyOffset;         /**< Combined body offset. */
        } request;
        struct {
            uint8_t tid;            /**< TID. Transaction Identifier. */
            HAPBLEPDUStatus status; /**< Status. */

            void* _Nullable bodyBytes; /**< Combined body. */
            size_t totalBodyBytes;     /**< Combined body length. */
            size_t bodyOffset;         /**< Combined body offset. */
        } response;
    } _;
} HAPBLETransaction;
/**@endcond */

/**
 * Initializes a transaction with a body buffer.
 *
 * - Only one transaction can be processed. Reinitialization is required to process the next transaction.
 *
 * @param[out] bleTransaction       Transaction.
 * @param      bodyBytes            Buffer that may be used to store body data.
 * @param      numBodyBytes         Capacity of body buffer.
 */
void HAPBLETransactionCreate(HAPBLETransaction* bleTransaction, void* _Nullable bodyBytes, size_t numBodyBytes)
        HAP_DIAGNOSE_ERROR(!bodyBytes && numBodyBytes, "empty buffer cannot have a length");

/**
 * Processes incoming data. Once all data fragments have been received,
 * #HAPBLETransactionIsRequestAvailable returns true, and
 * #HAPBLETransactionGetRequest may be used to get the request data.
 *
 * @param      bleTransaction       Transaction.
 * @param      bytes                Buffer with data to process.
 * @param      numBytes             Length of buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed or unexpected request.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLETransactionHandleWrite(HAPBLETransaction* bleTransaction, const void* bytes, size_t numBytes);

/**
 * Returns whether a complete request has been received and is ready to be fetched with #HAPBLETransactionGetRequest.
 *
 * @param      bleTransaction       Transaction.
 *
 * @return true  If a complete request has been received.
 * @return false Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPBLETransactionIsRequestAvailable(const HAPBLETransaction* bleTransaction);

/**
 * Request.
 */
typedef struct {
    /**
     * HAP Opcode.
     */
    HAPPDUOpcode opcode;

    /**
     * CID. Characteristic / service instance ID.
     *
     * - For Bluetooth LE, instance IDs cannot exceed UINT16_MAX.
     */
    uint16_t iid;

    /**
     * Reader that may be used to query the request's body.
     *
     * If the body did not fit into the buffer supplied to #HAPBLETransactionCreate,
     * the body is skipped and the reader returns no data.
     */
    HAPTLVReaderRef bodyReader;
} HAPBLETransactionRequest;

/**
 * After #HAPBLETransactionHandleWrite indicates that a complete request has been received,
 * this function may be used to retrieve the most recent request. The function may only be called once per request.
 *
 * @param      bleTransaction       Transaction.
 * @param[out] request              Request.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the request did not fit into the receive buffer.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLETransactionGetRequest(HAPBLETransaction* bleTransaction, HAPBLETransactionRequest* request);

/**
 * Sets the response for sending with future #HAPBLETransactionHandleRead commands.
 *
 * @param      bleTransaction       Transaction.
 * @param      status               Response Status.
 * @param      bodyWriter           Writer containing the serialized response body. Optional. Maximum UINT16_MAX length.
 */
void HAPBLETransactionSetResponse(
        HAPBLETransaction* bleTransaction,
        HAPBLEPDUStatus status,
        const HAPTLVWriterRef* _Nullable bodyWriter);

/**
 * Fills a buffer with the next response fragment to be sent.
 *
 * @param      bleTransaction       Transaction.
 * @param[out] bytes                Buffer to put fragment data into.
 * @param      maxBytes             Capacity of buffer.
 * @param[out] numBytes             Length of fragment put into buffer.
 * @param[out] isFinalFragment      true If all data fragments have been produced; false Otherwise.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If buffer not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLETransactionHandleRead(
        HAPBLETransaction* bleTransaction,
        void* bytes,
        size_t maxBytes,
        size_t* numBytes,
        bool* isFinalFragment);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
