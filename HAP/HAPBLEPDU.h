// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_PDU_H
#define HAP_BLE_PDU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Fragmentation status of a HAP PDU.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-3 Control Field Bit 7 Values
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEPDUFragmentationStatus) { /** First fragment (or no fragmentation). */
                                                        kHAPBLEPDUFragmentationStatus_FirstFragment = 0x00,

                                                        /** Continuation of fragmented PDU. */
                                                        kHAPBLEPDUFragmentationStatus_Continuation = 0x01
} HAP_ENUM_END(uint8_t, HAPBLEPDUFragmentationStatus);

/**
 * Instance ID size of a HAP PDU.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-4 Control Field Bit 4 Values
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEPDUInstanceIDSize) { /** 16-bit IIDs (or IID = 0). */
                                                   kHAPBLEPDUInstanceIDSize_16Bit = 0x00,

                                                   /** 64-bit IIDs. */
                                                   kHAPBLEPDUInstanceIDSize_64Bit = 0x01
} HAP_ENUM_END(uint8_t, HAPBLEPDUInstanceIDSize);

/**
 * Type of a HAP PDU.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-5 Control Field Bit 1-3 Values
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEPDUType) { /** Request. */
                                         kHAPBLEPDUType_Request,

                                         /** Response. */
                                         kHAPBLEPDUType_Response
} HAP_ENUM_END(uint8_t, HAPBLEPDUType);

/**
 * Length of a HAP PDU Control Field.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-6 Control Field Bit 0 Values
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEPDUControlFieldLength) { /** 1 Byte Control Field. */
                                                       kHAPBLEPDUControlFieldLength_1Byte
} HAP_ENUM_END(uint8_t, HAPBLEPDUControlFieldLength);

/**
 * Returns whether an opcode is a Service operation.
 *
 * @param      opcode               Operation.
 *
 * @return true                     If opcode is a Service operation.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPBLEPDUOpcodeIsServiceOperation(HAPPDUOpcode opcode);

/**
 * HAP Status Code.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-37 HAP Status Codes Description
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEPDUStatus) { /** Success. */
                                           kHAPBLEPDUStatus_Success = 0x00,

                                           /** Unsupported-PDU. */
                                           kHAPBLEPDUStatus_UnsupportedPDU = 0x01,

                                           /** Max-Procedures. */
                                           kHAPBLEPDUStatus_MaxProcedures = 0x02,

                                           /** Insufficient Authorization. */
                                           kHAPBLEPDUStatus_InsufficientAuthorization = 0x03,

                                           /** Invalid instance ID. */
                                           kHAPBLEPDUStatus_InvalidInstanceID = 0x04,

                                           /** Insufficient Authentication. */
                                           kHAPBLEPDUStatus_InsufficientAuthentication = 0x05,

                                           /** Invalid Request. */
                                           kHAPBLEPDUStatus_InvalidRequest = 0x06
} HAP_ENUM_END(uint8_t, HAPBLEPDUStatus);

/**
 * Header length of a HAP-BLE Request.
 */
#define kHAPBLEPDU_NumRequestHeaderBytes ((size_t)(1 + 4))

/**
 * Header length of a HAP-BLE Response.
 */
#define kHAPBLEPDU_NumResponseHeaderBytes ((size_t)(1 + 2))

/**
 * Header length of a continuation of a fragmented HAP-BLE PDU.
 */
#define kHAPBLEPDU_NumContinuationHeaderBytes ((size_t)(1 + 1))

/**
 * Additional header length of a PDU with a body.
 * Only applies to the first fragment of a HAP-BLE PDU.
 */
#define kHAPBLEPDU_NumBodyHeaderBytes ((size_t)(2))

/**
 * HAP PDU.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.3 HAP PDU Format
 */
typedef struct {
    /**
     * Control Field.
     *
     * Defines how the PDU and the rest of the bytes in the PDU are interpreted.
     *
     * @see HomeKit Accessory Protocol Specification R14
     *      Section 7.3.3.1 HAP PDU Header - Control Field
     */
    struct {
        /** Fragmentation status. */
        HAPBLEPDUFragmentationStatus fragmentationStatus;

        /** PDU type. */
        HAPBLEPDUType type;

        /** Control Field length. */
        HAPBLEPDUControlFieldLength length;
    } controlField;

    /**
     * PDU Fixed Params.
     *
     * Contains fixed params depending on the Control Field.
     */
    union {
        /**
         * HAP Request.
         *
         * @see HomeKit Accessory Protocol Specification R14
         *      Section 7.3.3.2 HAP Request Format
         */
        struct {
            HAPPDUOpcode opcode; /**< HAP Opcode. */
            uint8_t tid;         /**< TID. Transaction Identifier. */
            uint16_t iid;        /**< CharID / SvcID. Characteristic / service instance ID. */
        } request;

        /**
         * HAP Response.
         *
         * @see HomeKit Accessory Protocol Specification R14
         *      Section 7.3.3.3 HAP Response Format
         */
        struct {
            uint8_t tid;            /**< TID. Transaction Identifier. */
            HAPBLEPDUStatus status; /**< Status. */
        } response;

        /**
         * Continuation of fragmented PDU.
         *
         * @see HomeKit Accessory Protocol Specification R14
         *      Section 7.3.3.5 HAP PDU Fragmentation Scheme
         */
        struct {
            uint8_t tid; /**< TID. Transaction Identifier. */
        } continuation;
    } fixedParams;

    /**
     * HAP-BLE PDU Body.
     *
     * - For PDUs without a body:
     *   - @c totalBodyBytes is 0.
     *   - @c bytes is NULL.
     *   - @c numBytes is 0.
     * - For PDUs that include a body:
     *   - @c totalBodyBytes contains the total PDU Body Length across all
     *     potential fragments.
     *   - @c bytes points to the start of the current body fragment.
     *   - @c numBytes contains the length of the current body fragment.
     *
     * @see HomeKit Accessory Protocol Specification R14
     *      Section 7.3.3.4 HAP PDU Body
     *
     * @see #HAPBLEPDUTLVType
     */
    struct {
        uint16_t totalBodyBytes;     /**< PDU Body Length. */
        const void* _Nullable bytes; /**< Additional Params and Values in TLV8s. */
        uint16_t numBytes;           /**< Length of the body fragment. */
    } body;
} HAPBLEPDU;

/**
 * Deserialize the content of a buffer into a HAP-BLE PDU structure.
 * The buffer should contain the complete serialized PDU, or its first fragment.
 *
 * @param[out] pdu                  Deserialized PDU.
 * @param      bytes                Start of the memory region to deserialize.
 * @param      numBytes             Length of the memory region to deserialize.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 *
 * @remark To deserialize continuations of fragmented PDUs, use #HAPBLEPDUDeserializeContinuation.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUDeserialize(HAPBLEPDU* pdu, const void* bytes, size_t numBytes);

/**
 * Deserialize the content of a buffer into a HAP-BLE PDU structure.
 * The buffer should contain the serialized continuation of a fragmented PDU. Otherwise, an error is returned.
 *
 * - To deserialize complete PDUs or their first fragment, use #HAPBLEPDUDeserialize.
 *
 * @param[out] pdu                  Deserialized PDU.
 * @param      bytes                Start of the memory region to deserialize.
 * @param      numBytes             Length of the memory region to deserialize.
 * @param      typeOfFirstFragment  Type of the first fragment of the PDU.
 * @param      remainingBodyBytes   Remaining body length.
 * @param      totalBodyBytesSoFar  Combined length of preceding body fragments.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.3.5 HAP PDU Fragmentation Scheme
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUDeserializeContinuation(
        HAPBLEPDU* pdu,
        const void* bytes,
        size_t numBytes,
        HAPBLEPDUType typeOfFirstFragment,
        size_t remainingBodyBytes,
        size_t totalBodyBytesSoFar);

/**
 * Serialize a HAP-BLE PDU structure.
 *
 * - For continuations of fragmented PDUs, @c pdu->body.totalBodyBytes is not validated.
 *
 * @param      pdu                  PDU to serialize.
 * @param      bytes                Start of the memory region to serialize into.
 * @param      maxBytes             Capacity of the memory region to serialize into.
 * @param[out] numBytes             Effective length of the serialized PDU.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the buffer is not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUSerialize(const HAPBLEPDU* pdu, void* bytes, size_t maxBytes, size_t* numBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
