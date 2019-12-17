// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLEPDU" };

/**
 * Checks whether a value represents a valid PDU type.
 *
 * @param      value                Value to check.
 *
 * @return true                     If the value is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool HAPBLEPDUIsValidType(uint8_t value) {
    switch (value) {
        case kHAPBLEPDUType_Request:
        case kHAPBLEPDUType_Response: {
            return true;
        }
        default: {
            return false;
        }
    }
}

/**
 * Returns description of a PDU type.
 *
 * @param      type                 Value of which to get description.
 *
 * @return Description of the type.
 */
HAP_RESULT_USE_CHECK
static const char* HAPBLEPDUTypeDescription(HAPBLEPDUType type) {
    HAPPrecondition(HAPBLEPDUIsValidType(type));

    switch (type) {
        case kHAPBLEPDUType_Request: {
            return "Request";
        }
        case kHAPBLEPDUType_Response: {
            return "Response";
        }
    }
    HAPFatalError();
}

/**
 * Returns description of a HAP Opcode.
 *
 * @param      opcode               Value of which to get description.
 *
 * @return Description of the opcode.
 */
HAP_RESULT_USE_CHECK
static const char* HAPBLEPDUOpcodeDescription(HAPPDUOpcode opcode) {
    HAPPrecondition(HAPPDUIsValidOpcode(opcode));

    switch (opcode) {
        case kHAPPDUOpcode_CharacteristicSignatureRead: {
            return "HAP-Characteristic-Signature-Read";
        }
        case kHAPPDUOpcode_CharacteristicWrite: {
            return "HAP-Characteristic-Write";
        }
        case kHAPPDUOpcode_CharacteristicRead: {
            return "HAP-Characteristic-Read";
        }
        case kHAPPDUOpcode_CharacteristicTimedWrite: {
            return "HAP-Characteristic-Timed-Write";
        }
        case kHAPPDUOpcode_CharacteristicExecuteWrite: {
            return "HAP-Characteristic-Execute-Write";
        }
        case kHAPPDUOpcode_ServiceSignatureRead: {
            return "HAP-Service-Signature-Read";
        }
        case kHAPPDUOpcode_CharacteristicConfiguration: {
            return "HAP-Characteristic-Configuration";
        }
        case kHAPPDUOpcode_ProtocolConfiguration: {
            return "HAP-Protocol-Configuration";
        }
        case kHAPPDUOpcode_Token: {
            return "HAP-Token";
        }
        case kHAPPDUOpcode_TokenUpdate: {
            return "HAP-Token-Update";
        }
        case kHAPPDUOpcode_Info: {
            return "HAP-Info";
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
bool HAPBLEPDUOpcodeIsServiceOperation(HAPPDUOpcode opcode) {
    HAPPrecondition(HAPPDUIsValidOpcode(opcode));

    switch (opcode) {
        case kHAPPDUOpcode_ServiceSignatureRead:
        case kHAPPDUOpcode_ProtocolConfiguration: {
            return true;
        }
        case kHAPPDUOpcode_CharacteristicSignatureRead:
        case kHAPPDUOpcode_CharacteristicWrite:
        case kHAPPDUOpcode_CharacteristicRead:
        case kHAPPDUOpcode_CharacteristicTimedWrite:
        case kHAPPDUOpcode_CharacteristicExecuteWrite:
        case kHAPPDUOpcode_CharacteristicConfiguration:
        case kHAPPDUOpcode_Token:
        case kHAPPDUOpcode_TokenUpdate:
        case kHAPPDUOpcode_Info: {
            return false;
        }
    }
    HAPFatalError();
}

/**
 * Checks whether a value represents a valid HAP Status Code.
 *
 * @param      value                Value to check.
 *
 * @return true                     If the value is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool HAPBLEPDUIsValidStatus(uint8_t value) {
    switch (value) {
        case kHAPBLEPDUStatus_Success:
        case kHAPBLEPDUStatus_UnsupportedPDU:
        case kHAPBLEPDUStatus_MaxProcedures:
        case kHAPBLEPDUStatus_InsufficientAuthorization:
        case kHAPBLEPDUStatus_InvalidInstanceID:
        case kHAPBLEPDUStatus_InsufficientAuthentication:
        case kHAPBLEPDUStatus_InvalidRequest: {
            return true;
        }
        default: {
            return false;
        }
    }
}

/**
 * Returns description of a HAP Status Code.
 *
 * @param      status               Value of which to get description.
 *
 * @return Description of the status.
 */
HAP_RESULT_USE_CHECK
static const char* HAPBLEPDUStatusDescription(HAPBLEPDUStatus status) {
    HAPPrecondition(HAPBLEPDUIsValidStatus(status));

    switch (status) {
        case kHAPBLEPDUStatus_Success: {
            return "Success";
        }
        case kHAPBLEPDUStatus_UnsupportedPDU: {
            return "Unsupported-PDU";
        }
        case kHAPBLEPDUStatus_MaxProcedures: {
            return "Max-Procedures";
        }
        case kHAPBLEPDUStatus_InsufficientAuthorization: {
            return "Insufficient Authorization";
        }
        case kHAPBLEPDUStatus_InvalidInstanceID: {
            return "Invalid Instance ID";
        }
        case kHAPBLEPDUStatus_InsufficientAuthentication: {
            return "Insufficient Authentication";
        }
        case kHAPBLEPDUStatus_InvalidRequest: {
            return "Invalid Request";
        }
    }
    HAPFatalError();
}

/**
 * Logs a HAP-BLE PDU.
 *
 * @param      pdu                  PDU to log.
 */
static void LogPDU(const HAPBLEPDU* pdu) {
    HAPPrecondition(pdu);
    HAPBLEPDUType type = pdu->controlField.type;
    HAPPrecondition(HAPBLEPDUIsValidType(type));

    switch (pdu->controlField.fragmentationStatus) {
        case kHAPBLEPDUFragmentationStatus_FirstFragment: {
            switch (type) {
                case kHAPBLEPDUType_Request: {
                    HAPPDUOpcode opcode = pdu->fixedParams.request.opcode;
                    HAPLogBufferDebug(
                            &logObject,
                            pdu->body.bytes,
                            pdu->body.numBytes,
                            "%s-%s (0x%02x):\n"
                            "    TID: 0x%02x\n"
                            "    IID: %u",
                            HAPPDUIsValidOpcode(opcode) ? HAPBLEPDUOpcodeDescription(opcode) : "Unknown",
                            HAPBLEPDUTypeDescription(type),
                            opcode,
                            pdu->fixedParams.request.tid,
                            pdu->fixedParams.request.iid);
                } break;
                case kHAPBLEPDUType_Response: {
                    HAPBLEPDUStatus status = pdu->fixedParams.response.status;
                    HAPLogBufferDebug(
                            &logObject,
                            pdu->body.bytes,
                            pdu->body.numBytes,
                            "%s:\n"
                            "    TID: 0x%02x\n"
                            "    Status: %s (0x%02x)",
                            HAPBLEPDUTypeDescription(type),
                            pdu->fixedParams.response.tid,
                            HAPBLEPDUIsValidStatus(status) ? HAPBLEPDUStatusDescription(status) : "Unknown",
                            pdu->fixedParams.response.status);
                } break;
            }
        } break;
        case kHAPBLEPDUFragmentationStatus_Continuation: {
            HAPLogBufferDebug(
                    &logObject,
                    pdu->body.bytes,
                    pdu->body.numBytes,
                    "%s (Continuation):\n"
                    "    TID: 0x%02x",
                    HAPBLEPDUTypeDescription(type),
                    pdu->fixedParams.response.tid);
        } break;
    }
}

/**
 * Attempts to deserialize the Control Field into a PDU structure.
 *
 * @param[out] pdu                  PDU.
 * @param      controlField         Serialized Control Field.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 */
HAP_RESULT_USE_CHECK
static HAPError DeserializeControlField(HAPBLEPDU* pdu, const uint8_t* controlField) {
    HAPPrecondition(pdu);
    HAPPrecondition(controlField);

    // Check that reserved bits are 0.
    if (controlField[0] & (1 << 6 | 1 << 5 | 1 << 4)) {
        HAPLog(&logObject, "Invalid reserved bits in Control Field 0x%02x.", controlField[0]);
        return kHAPError_InvalidData;
    }

    // Fragmentation status.
    switch (controlField[0] & 1 << 7) {
        case 0 << 7: {
            pdu->controlField.fragmentationStatus = kHAPBLEPDUFragmentationStatus_FirstFragment;
        } break;
        case 1 << 7: {
            pdu->controlField.fragmentationStatus = kHAPBLEPDUFragmentationStatus_Continuation;
        } break;
        default: {
            HAPLog(&logObject, "Invalid fragmentation status in Control Field 0x%02x.", controlField[0]);
            return kHAPError_InvalidData;
        }
    }

    // PDU Type.
    switch (controlField[0] & (1 << 3 | 1 << 2 | 1 << 1)) {
        case 0 << 3 | 0 << 2 | 0 << 1: {
            pdu->controlField.type = kHAPBLEPDUType_Request;
        } break;
        case 0 << 3 | 0 << 2 | 1 << 1: {
            pdu->controlField.type = kHAPBLEPDUType_Response;
        } break;
        default: {
            HAPLog(&logObject, "Invalid PDU Type in Control Field 0x%02x.", controlField[0]);
            return kHAPError_InvalidData;
        }
    }

    // Length.
    switch (controlField[0] & 1 << 0) {
        case 0 << 0: {
            pdu->controlField.length = kHAPBLEPDUControlFieldLength_1Byte;
        } break;
        default: {
            HAPLog(&logObject, "Invalid length in Control Field 0x%02x.", controlField[0]);
            return kHAPError_InvalidData;
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUDeserialize(HAPBLEPDU* pdu, const void* bytes, size_t numBytes) {
    HAPPrecondition(pdu);
    HAPPrecondition(bytes);

    HAPError err;

    const uint8_t* b = bytes;
    size_t remainingBytes = numBytes;

    // PDU Header - Control Field.
    if (remainingBytes < 1) {
        HAPLog(&logObject, "PDU not long enough to contain Control Field.");
        return kHAPError_InvalidData;
    }
    err = DeserializeControlField(pdu, b);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }
    if (pdu->controlField.fragmentationStatus != kHAPBLEPDUFragmentationStatus_FirstFragment) {
        HAPLog(&logObject, "Unexpected PDU fragmentation status (expected: First fragment (or no fragmentation)).");
        return kHAPError_InvalidData;
    }
    b += 1;
    remainingBytes -= 1;

    // PDU Fixed Params.
    switch (pdu->controlField.type) {
        case kHAPBLEPDUType_Request: {
            if (remainingBytes < 4) {
                HAPLog(&logObject, "Request PDU not long enough to contain Fixed Params.");
                return kHAPError_InvalidData;
            }
            pdu->fixedParams.request.opcode = (HAPPDUOpcode) b[0];
            pdu->fixedParams.request.tid = b[1];
            pdu->fixedParams.request.iid = HAPReadLittleUInt16(&b[2]);
            b += 4;
            remainingBytes -= 4;
        }
            goto deserialize_body;
        case kHAPBLEPDUType_Response: {
            if (remainingBytes < 2) {
                HAPLog(&logObject, "Response PDU not long enough to contain Fixed Params.");
                return kHAPError_InvalidData;
            }
            pdu->fixedParams.response.tid = b[0];
            pdu->fixedParams.response.status = (HAPBLEPDUStatus) b[1];
            b += 2;
            remainingBytes -= 2;
        }
            goto deserialize_body;
    }
    HAPFatalError();

deserialize_body:

    // PDU Body (Optional).
    if (!remainingBytes) {
        pdu->body.totalBodyBytes = 0;
        pdu->body.bytes = NULL;
        pdu->body.numBytes = 0;
    } else {
        if (remainingBytes < 2) {
            HAPLog(&logObject, "PDU not long enough to contain body length.");
            return kHAPError_InvalidData;
        }
        pdu->body.totalBodyBytes = HAPReadLittleUInt16(b);
        b += 2;
        remainingBytes -= 2;

        if (remainingBytes < pdu->body.totalBodyBytes) {
            // First fragment.
            pdu->body.numBytes = (uint16_t) remainingBytes;
        } else {
            // Complete body available.
            pdu->body.numBytes = pdu->body.totalBodyBytes;
        }
        pdu->body.bytes = b;
        b += pdu->body.numBytes;
        remainingBytes -= pdu->body.numBytes;
    }

    // All data read.
    if (remainingBytes) {
        HAPLog(&logObject, "Excess data after PDU.");
        return kHAPError_InvalidData;
    }
    (void) b;

    LogPDU(pdu);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUDeserializeContinuation(
        HAPBLEPDU* pdu,
        const void* bytes,
        size_t numBytes,
        HAPBLEPDUType typeOfFirstFragment,
        size_t remainingBodyBytes,
        size_t totalBodyBytesSoFar) {
    HAPPrecondition(pdu);
    HAPPrecondition(bytes);
    HAPPrecondition(remainingBodyBytes >= totalBodyBytesSoFar);
    HAPPrecondition(remainingBodyBytes <= UINT16_MAX);

    HAPError err;

    const uint8_t* b = bytes;
    size_t remainingBytes = numBytes;

    // PDU Header - Control Field.
    if (remainingBytes < 1) {
        HAPLog(&logObject, "PDU not long enough to contain Control Field.");
        return kHAPError_InvalidData;
    }
    err = DeserializeControlField(pdu, b);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }
    if (pdu->controlField.fragmentationStatus != kHAPBLEPDUFragmentationStatus_Continuation) {
        HAPLog(&logObject, "Unexpected PDU fragmentation status (expected: Continuation of fragmented PDU).");
        return kHAPError_InvalidData;
    }
    if (pdu->controlField.type != typeOfFirstFragment) {
        HAPLog(&logObject,
               "Unexpected PDU type (Continuation type: 0x%02x, First Fragment type: 0x%02x).",
               pdu->controlField.type,
               typeOfFirstFragment);
        return kHAPError_InvalidData;
    }
    b += 1;
    remainingBytes -= 1;

    // PDU Fixed Params.
    if (remainingBytes < 1) {
        HAPLog(&logObject, "Continuation PDU not long enough to contain Fixed Params.");
        return kHAPError_InvalidData;
    }
    pdu->fixedParams.continuation.tid = b[0];
    b += 1;
    remainingBytes -= 1;

    // PDU Body (Optional).
    pdu->body.totalBodyBytes = (uint16_t) remainingBodyBytes;
    if (!remainingBytes) {
        pdu->body.bytes = NULL;
        pdu->body.numBytes = 0;
    } else if (remainingBytes <= remainingBodyBytes - totalBodyBytesSoFar) {
        pdu->body.numBytes = (uint16_t) remainingBytes;
        pdu->body.bytes = b;
        b += pdu->body.numBytes;
        remainingBytes -= pdu->body.numBytes;
    }

    // All data read.
    if (remainingBytes) {
        HAPLog(&logObject, "Excess data after PDU.");
        return kHAPError_InvalidData;
    }
    (void) b;

    LogPDU(pdu);
    return kHAPError_None;
}

/**
 * Checks whether a value represents a valid fragmentation status.
 *
 * @param      value                Value to check.
 *
 * @return true                     If the value is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool HAPBLEPDUIsValidFragmentStatus(uint8_t value) {
    switch (value) {
        case kHAPBLEPDUFragmentationStatus_FirstFragment:
        case kHAPBLEPDUFragmentationStatus_Continuation: {
            return true;
        }
        default: {
            return false;
        }
    }
}

/**
 * Checks whether a value represents a valid Control Field length.
 *
 * @param      value                Value to check.
 *
 * @return true                     If the value is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool HAPBLEPDUIsValidControlFieldLength(uint8_t value) {
    switch (value) {
        case kHAPBLEPDUControlFieldLength_1Byte: {
            return true;
        }
        default: {
            return false;
        }
    }
}

/**
 * Checks whether the Control Field of a PDU structure is valid.
 *
 * @param      pdu                  PDU with Control Field to check.
 *
 * @return true                     If the Control Field is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool HAPBLEPDUHasValidControlField(const HAPBLEPDU* pdu) {
    HAPPrecondition(pdu);

    return HAPBLEPDUIsValidFragmentStatus(pdu->controlField.fragmentationStatus) &&
           HAPBLEPDUIsValidType(pdu->controlField.type) && HAPBLEPDUIsValidControlFieldLength(pdu->controlField.length);
}

/**
 * Serializes the Control Field of a PDU structure.
 *
 * @param      pdu                  PDU with Control Field to serialize.
 * @param[out] controlFieldByte     Serialized Control Field.
 */
static void SerializeControlField(const HAPBLEPDU* pdu, uint8_t* controlFieldByte) {
    HAPPrecondition(pdu);
    HAPPrecondition(HAPBLEPDUHasValidControlField(pdu));
    HAPPrecondition(controlFieldByte);

    // Clear all bits.
    controlFieldByte[0] = 0;

    // Fragmentation status.
    switch (pdu->controlField.fragmentationStatus) {
        case kHAPBLEPDUFragmentationStatus_FirstFragment: {
            controlFieldByte[0] |= 0 << 7;
        }
            goto serialize_pdu_type;
        case kHAPBLEPDUFragmentationStatus_Continuation: {
            controlFieldByte[0] |= 1 << 7;
        }
            goto serialize_pdu_type;
    }
    HAPFatalError();

serialize_pdu_type:
    // PDU Type.
    switch (pdu->controlField.type) {
        case kHAPBLEPDUType_Request: {
            controlFieldByte[0] |= 0 << 3 | 0 << 2 | 0 << 1;
        }
            goto serialize_length;
        case kHAPBLEPDUType_Response: {
            controlFieldByte[0] |= 0 << 3 | 0 << 2 | 1 << 1;
        }
            goto serialize_length;
    }
    HAPFatalError();

serialize_length:
    // Length.
    switch (pdu->controlField.length) {
        case kHAPBLEPDUControlFieldLength_1Byte: {
            controlFieldByte[0] |= 0 << 0;
            return;
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUSerialize(const HAPBLEPDU* pdu, void* bytes, size_t maxBytes, size_t* numBytes) {
    HAPPrecondition(pdu);
    HAPPrecondition(HAPBLEPDUHasValidControlField(pdu));
    HAPPrecondition(pdu->body.numBytes <= pdu->body.totalBodyBytes);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    LogPDU(pdu);

    uint8_t* b = bytes;
    size_t remainingBytes = maxBytes;

    // PDU Header - Control Field.
    if (remainingBytes < 1) {
        HAPLog(&logObject, "Not enough capacity to serialize Control Field.");
        return kHAPError_OutOfResources;
    }
    SerializeControlField(pdu, b);
    b += 1;
    remainingBytes -= 1;

    // PDU Header - PDU Fixed Params.
    switch (pdu->controlField.fragmentationStatus) {
        case kHAPBLEPDUFragmentationStatus_FirstFragment: {
            switch (pdu->controlField.type) {
                case kHAPBLEPDUType_Request: {
                    if (remainingBytes < 4) {
                        HAPLog(&logObject, "Not enough capacity to serialize Request PDU Fixed Params.");
                        return kHAPError_OutOfResources;
                    }
                    b[0] = pdu->fixedParams.request.opcode;
                    b[1] = pdu->fixedParams.request.tid;
                    HAPWriteLittleUInt16(&b[2], pdu->fixedParams.request.iid);
                    b += 4;
                    remainingBytes -= 4;
                    goto serialize_body;
                }
                case kHAPBLEPDUType_Response: {
                    if (remainingBytes < 2) {
                        HAPLog(&logObject, "Not enough capacity to serialize Response PDU Fixed Params.");
                        return kHAPError_OutOfResources;
                    }
                    b[0] = pdu->fixedParams.response.tid;
                    b[1] = pdu->fixedParams.response.status;
                    b += 2;
                    remainingBytes -= 2;
                    goto serialize_body;
                }
            }
            HAPFatalError();

        serialize_body:
            // PDU Body (Optional).
            if (pdu->body.bytes) {
                if (remainingBytes < 2) {
                    HAPLog(&logObject, "Not enough capacity to serialize PDU Body length.");
                    return kHAPError_OutOfResources;
                }
                HAPWriteLittleUInt16(&b[0], pdu->body.totalBodyBytes);
                b += 2;
                remainingBytes -= 2;

                if (remainingBytes < pdu->body.numBytes) {
                    HAPLog(&logObject, "Not enough capacity to serialize PDU Body.");
                    return kHAPError_OutOfResources;
                }
                HAPRawBufferCopyBytes(b, HAPNonnullVoid(pdu->body.bytes), pdu->body.numBytes);
                b += pdu->body.numBytes;
                remainingBytes -= pdu->body.numBytes;
            }
            goto done;
        }
        case kHAPBLEPDUFragmentationStatus_Continuation: {
            if (remainingBytes < 1) {
                HAPLog(&logObject, "Not enough capacity to serialize Continuation PDU Fixed Params.");
                return kHAPError_OutOfResources;
            }
            b[0] = pdu->fixedParams.continuation.tid;
            b += 1;
            remainingBytes -= 1;

            if (remainingBytes < pdu->body.numBytes) {
                HAPLog(&logObject, "Not enough capacity to serialize PDU Body.");
                return kHAPError_OutOfResources;
            }
            if (!pdu->body.numBytes) {
                HAPLog(&logObject, "Received empty continuation fragment.");
            } else {
                HAPAssert(pdu->body.bytes);
                HAPRawBufferCopyBytes(b, HAPNonnullVoid(pdu->body.bytes), pdu->body.numBytes);
                b += pdu->body.numBytes;
                remainingBytes -= pdu->body.numBytes;
            }
            goto done;
        }
    }
    HAPAssertionFailure();

done:
    // All data written.
    *numBytes = maxBytes - remainingBytes;
    (void) b;
    return kHAPError_None;
}
