// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "TLVWriter" };

void HAPTLVWriterCreate(HAPTLVWriterRef* writer_, void* bytes, size_t maxBytes) {
    HAPPrecondition(writer_);
    HAPTLVWriter* writer = (HAPTLVWriter*) writer_;
    HAPPrecondition(bytes);

    // Initialize writer.
    HAPRawBufferZero(writer, sizeof *writer);
    writer->bytes = bytes;
    writer->maxBytes = maxBytes;
    writer->numBytes = 0;
    writer->lastType = 0;
}

HAP_RESULT_USE_CHECK
HAPError HAPTLVWriterAppend(HAPTLVWriterRef* writer_, const HAPTLV* tlv) {
    HAPPrecondition(writer_);
    HAPTLVWriter* writer = (HAPTLVWriter*) writer_;
    HAPPrecondition(tlv);
    if (!tlv->value.bytes) {
        HAPPrecondition(!tlv->value.numBytes);
    }
    if (writer->numBytes) {
        HAPPrecondition(tlv->type != writer->lastType);
    }

    uint8_t* destinationBytes = HAPNonnullVoid(writer->bytes);
    size_t maxDestinationBytes = writer->maxBytes - writer->numBytes;

    const uint8_t* _Nullable valueBytes = tlv->value.bytes;
    size_t numValueBytes = tlv->value.numBytes;

    // Serialize TLV, fragment by fragment.
    do {
        size_t numFragmentBytes = numValueBytes > UINT8_MAX ? UINT8_MAX : numValueBytes;

        // Consume space needed for header.
        if (maxDestinationBytes < 2) {
            // TLV header does not fit into buffer.
            HAPLog(&logObject, "Not enough memory to write TLV header.");
            return kHAPError_OutOfResources;
        }
        maxDestinationBytes -= 2;

        if (valueBytes) {
            // Since the memory after serialized TLV data may have been used by the client,
            // move that data to accommodate the TLV header.
            if (maxDestinationBytes < numValueBytes) {
                // Value does not fit into buffer.
                HAPLog(&logObject, "Not enough memory to write TLV value.");
                return kHAPError_OutOfResources;
            }
            // Entire remaining value is copied, including followup fragments.
            HAPRawBufferCopyBytes(&destinationBytes[writer->numBytes + 2], HAPNonnull(valueBytes), numValueBytes);
            valueBytes = &destinationBytes[writer->numBytes + 2];
            maxDestinationBytes -= numFragmentBytes;
            numValueBytes -= numFragmentBytes;
            valueBytes += numFragmentBytes;
        } else {
            HAPAssert(!numValueBytes);
        }

        // Serialize fragment.
        destinationBytes[writer->numBytes++] = (uint8_t) tlv->type;
        destinationBytes[writer->numBytes++] = (uint8_t) numFragmentBytes;
        writer->numBytes += numFragmentBytes;
    } while (numValueBytes);

    writer->lastType = tlv->type;
    return kHAPError_None;
}

void HAPTLVWriterGetBuffer(const HAPTLVWriterRef* writer_, void* _Nonnull* _Nonnull bytes, size_t* numBytes) {
    HAPPrecondition(writer_);
    const HAPTLVWriter* writer = (const HAPTLVWriter*) writer_;
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    *bytes = writer->bytes;
    *numBytes = writer->numBytes;
}

void HAPTLVWriterGetScratchBytes(
        const HAPTLVWriterRef* writer_,
        void* _Nonnull* _Nonnull scratchBytes,
        size_t* numScratchBytes) {
    HAPPrecondition(writer_);
    const HAPTLVWriter* writer = (const HAPTLVWriter*) writer_;
    HAPPrecondition(scratchBytes);
    HAPPrecondition(numScratchBytes);

    uint8_t* bytes = writer->bytes;
    *scratchBytes = &bytes[writer->numBytes];
    *numScratchBytes = writer->maxBytes - writer->numBytes;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
static HAPError HAPTLVWriterEncodeScalar(
        void* bytes,
        size_t maxBytes,
        size_t* numBytes,
        HAPTLVType tlvType,
        const char* debugDescription,
        const HAPTLVFormat* format,
        HAPTLVValue* _Nullable value,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel);

HAP_RESULT_USE_CHECK
static HAPError HAPTLVWriterEncodeAggregate(
        HAPTLVWriterRef* writer,
        const HAPTLVFormat* format,
        HAPTLVValue* value,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel);

typedef struct {
    HAPTLVWriterRef* writer;
    const HAPSequenceTLVFormat* format;
    HAPError err;
    bool needsSeparator;
} EnumerateSequenceTLVContext;

static void EnumerateSequenceTLVCallback(void* _Nullable context_, HAPTLVValue* value, bool* shouldContinue) {
    HAPPrecondition(context_);
    EnumerateSequenceTLVContext* context = context_;
    HAPPrecondition(context->writer);
    HAPTLVWriterRef* writer = context->writer;
    HAPPrecondition(context->format);
    HAPPrecondition(HAPTLVFormatIsValid(context->format));
    const HAPSequenceTLVFormat* fmt = context->format;
    HAPPrecondition(!context->err);
    bool* needsSeparator = &context->needsSeparator;
    HAPPrecondition(value);
    HAPPrecondition(shouldContinue);
    HAPPrecondition(*shouldContinue);

    HAPError err;

    char logBytes[kHAPTLVValue_MaxLogBytes + 1];
    HAPStringBuilderRef stringBuilder;
    HAPStringBuilderCreate(&stringBuilder, logBytes, sizeof logBytes);

    if (!*needsSeparator) {
        HAPLogDebug(&logObject, "Encoding sequence TLV.");
        *needsSeparator = true;
    } else {
        void* bytes;
        size_t maxBytes;
        HAPTLVWriterGetScratchBytes(writer, &bytes, &maxBytes);

        size_t numBytes;
        err = HAPTLVWriterEncodeScalar(
                bytes,
                maxBytes,
                &numBytes,
                fmt->separator.tlvType,
                fmt->separator.debugDescription,
                fmt->separator.format,
                NULL,
                &stringBuilder,
                /* nestingLevel: */ 0);
        if (err) {
            HAPAssert(
                    err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                    err == kHAPError_Busy);
            context->err = err;
            *shouldContinue = false;
            return;
        }
        HAPAssert(numBytes <= maxBytes);

        err = HAPTLVWriterAppend(
                writer,
                &(const HAPTLV) { .type = fmt->separator.tlvType, .value = { .bytes = bytes, .numBytes = numBytes } });
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            context->err = err;
            *shouldContinue = false;
            return;
        }
    }

    {
        if (fmt->item.isFlat) {
            HAPAssert(HAPTLVFormatIsAggregate(fmt->item.format));
            HAPAssert(((const HAPBaseTLVFormat*) fmt->item.format)->type == kHAPTLVFormatType_Union);

            err = HAPTLVWriterEncodeAggregate(writer, fmt->item.format, value, &stringBuilder, /* nestingLevel: */ 0);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                HAPLogTLV(&logObject, fmt->item.tlvType, fmt->item.debugDescription, "Value encoding failed.");
                context->err = err;
                *shouldContinue = false;
                return;
            }
        } else {
            void* bytes;
            size_t maxBytes;
            HAPTLVWriterGetScratchBytes(writer, &bytes, &maxBytes);

            size_t numBytes;
            if (HAPTLVFormatIsAggregate(fmt->item.format)) {
                HAPTLVAppendToLog(
                        fmt->item.tlvType,
                        fmt->item.debugDescription,
                        fmt,
                        NULL,
                        &stringBuilder,
                        /* nestingLevel: */ 0);
                HAPTLVWriterRef subWriter;
                HAPTLVWriterCreate(&subWriter, bytes, maxBytes);
                err = HAPTLVWriterEncodeAggregate(
                        &subWriter, fmt->item.format, value, &stringBuilder, /* nestingLevel: */ 1);
                if (err) {
                    HAPAssert(
                            err == kHAPError_Unknown || err == kHAPError_InvalidState ||
                            err == kHAPError_OutOfResources || err == kHAPError_Busy);
                    HAPLogTLV(&logObject, fmt->item.tlvType, fmt->item.debugDescription, "Value encoding failed.");
                    context->err = err;
                    *shouldContinue = false;
                    return;
                }
                void* tlvBytes;
                HAPTLVWriterGetBuffer(&subWriter, &tlvBytes, &numBytes);
                HAPAssert(tlvBytes == bytes);
            } else {
                err = HAPTLVWriterEncodeScalar(
                        bytes,
                        maxBytes,
                        &numBytes,
                        fmt->item.tlvType,
                        fmt->item.debugDescription,
                        fmt->item.format,
                        value,
                        &stringBuilder,
                        /* nestingLevel: */ 0);
                if (err) {
                    HAPAssert(
                            err == kHAPError_Unknown || err == kHAPError_InvalidState ||
                            err == kHAPError_OutOfResources || err == kHAPError_Busy);
                    context->err = err;
                    *shouldContinue = false;
                    return;
                }
            }
            HAPAssert(numBytes <= maxBytes);

            err = HAPTLVWriterAppend(
                    writer,
                    &(const HAPTLV) { .type = fmt->item.tlvType, .value = { .bytes = bytes, .numBytes = numBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                context->err = err;
                *shouldContinue = false;
                return;
            }
        }
    }

    if (HAPStringBuilderDidOverflow(&stringBuilder)) {
        HAPLogError(&logObject, "Logs were truncated.");
    }
    HAPLogDebug(&logObject, "Encoded sequence TLV:%s", HAPStringBuilderGetString(&stringBuilder));
}

HAP_RESULT_USE_CHECK
static HAPError HAPTLVWriterEncodeTLV(
        HAPTLVWriterRef* writer,
        HAPTLVType tlvType,
        const char* debugDescription,
        const HAPTLVFormat* format,
        HAPTLVValue* _Nullable value,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel) {
    HAPPrecondition(writer);
    HAPPrecondition(debugDescription);
    HAPPrecondition(format);
    HAPPrecondition(HAPTLVFormatIsValid(format));
    HAPPrecondition(stringBuilder);

    HAPError err;

    void* bytes;
    size_t maxBytes;
    HAPTLVWriterGetScratchBytes(writer, &bytes, &maxBytes);

    size_t numBytes;
    if (HAPTLVFormatIsAggregate(format)) {
        HAPTLVAppendToLog(tlvType, debugDescription, format, NULL, stringBuilder, nestingLevel);
        HAPTLVWriterRef subWriter;
        HAPTLVWriterCreate(&subWriter, bytes, maxBytes);
        err = HAPTLVWriterEncodeAggregate(&subWriter, format, HAPNonnullVoid(value), stringBuilder, nestingLevel + 1);
        if (err) {
            HAPAssert(
                    err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                    err == kHAPError_Busy);
            HAPLogTLV(&logObject, tlvType, debugDescription, "Value encoding failed.");
            return err;
        }
        void* tlvBytes;
        HAPTLVWriterGetBuffer(&subWriter, &tlvBytes, &numBytes);
        HAPAssert(tlvBytes == bytes);
    } else {
        err = HAPTLVWriterEncodeScalar(
                bytes, maxBytes, &numBytes, tlvType, debugDescription, format, value, stringBuilder, nestingLevel);
        if (err) {
            HAPAssert(
                    err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                    err == kHAPError_Busy);
            return err;
        }
        HAPAssert(numBytes <= maxBytes);
    }

    err = HAPTLVWriterAppend(
            writer, &(const HAPTLV) { .type = tlvType, .value = { .bytes = bytes, .numBytes = numBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPError HAPTLVWriterEncodeScalar(
        void* bytes,
        size_t maxBytes,
        size_t* numBytes,
        HAPTLVType tlvType,
        const char* debugDescription,
        const HAPTLVFormat* format_,
        HAPTLVValue* _Nullable value_,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel) {
    HAPPrecondition(bytes);
    HAPPrecondition(debugDescription);
    HAPPrecondition(format_);
    HAPPrecondition(HAPTLVFormatIsValid(format_));
    HAPPrecondition(!HAPTLVFormatIsAggregate(format_));
    const HAPBaseTLVFormat* format = format_;
    HAPPrecondition(bytes);
    HAPPrecondition(stringBuilder);

    HAPError err;

    *numBytes = 0;
    switch (format->type) {
        case kHAPTLVFormatType_None: {
            const HAPSeparatorTLVFormat* fmt HAP_UNUSED = format_;
            HAPPrecondition(!value_);
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
            *numBytes = 0;
            HAPAssert(*numBytes <= maxBytes);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Enum: {
            const HAPEnumTLVFormat* fmt = format_;
            const uint8_t* value = HAPNonnullVoid(value_);
            HAPAssert(fmt->callbacks.isValid);
            HAPPrecondition(fmt->callbacks.isValid(*value));
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
            if (maxBytes < sizeof *value) {
                HAPLogTLV(&logObject, tlvType, debugDescription, "Not enough memory to encode enumeration value.");
                return kHAPError_OutOfResources;
            }
            ((uint8_t*) bytes)[0] = *value;
            *numBytes = sizeof *value;
            HAPAssert(*numBytes <= maxBytes);
        }
            return kHAPError_None;
#define PROCESS_INTEGER_FORMAT(formatName, typeName, printfFormat, printfTypeName) \
    do { \
        const formatName* fmt = format_; \
        typeName* value = HAPNonnullVoid(value_); \
        HAPPrecondition(*value >= fmt->constraints.minimumValue); \
        HAPPrecondition(*value <= fmt->constraints.maximumValue); \
        HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel); \
        if (maxBytes < sizeof *value) { \
            HAPLogTLV(&logObject, tlvType, debugDescription, "Not enough memory to encode integer value."); \
            return kHAPError_OutOfResources; \
        } \
        *numBytes = sizeof *value; \
        for (size_t i = 0; i < *numBytes; i++) { \
            ((uint8_t*) bytes)[i] = (uint8_t)((*value >> (i * CHAR_BIT)) & 0xFF); \
        } \
        HAPAssert(*numBytes <= maxBytes); \
    } while (0)
        case kHAPTLVFormatType_UInt8: {
            PROCESS_INTEGER_FORMAT(HAPUInt8TLVFormat, uint8_t, "u", unsigned int);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_UInt16: {
            PROCESS_INTEGER_FORMAT(HAPUInt16TLVFormat, uint16_t, "u", unsigned int);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_UInt32: {
            PROCESS_INTEGER_FORMAT(HAPUInt32TLVFormat, uint32_t, "lu", unsigned long);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_UInt64: {
            PROCESS_INTEGER_FORMAT(HAPUInt64TLVFormat, uint64_t, "llu", unsigned long long);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Int8: {
            PROCESS_INTEGER_FORMAT(HAPInt8TLVFormat, int8_t, "d", int);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Int16: {
            PROCESS_INTEGER_FORMAT(HAPInt16TLVFormat, int16_t, "d", int);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Int32: {
            PROCESS_INTEGER_FORMAT(HAPInt32TLVFormat, int32_t, "ld", long);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Int64: {
            PROCESS_INTEGER_FORMAT(HAPInt64TLVFormat, int64_t, "lld", long long);
        }
            return kHAPError_None;
#undef PROCESS_INTEGER_FORMAT
        case kHAPTLVFormatType_Data: {
            const HAPDataTLVFormat* fmt = format_;
            const HAPDataTLVValue* value = HAPNonnullVoid(value_);
            HAPPrecondition(value->numBytes >= fmt->constraints.minLength);
            HAPPrecondition(value->numBytes <= fmt->constraints.maxLength);
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
            if (maxBytes < value->numBytes) {
                HAPLogTLV(&logObject, tlvType, debugDescription, "Not enough memory to encode data value.");
                return kHAPError_OutOfResources;
            }
            HAPRawBufferCopyBytes(bytes, value->bytes, value->numBytes);
            *numBytes = value->numBytes;
            HAPAssert(*numBytes <= maxBytes);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_String: {
            const HAPStringTLVFormat* fmt = format_;
            const char** value = HAPNonnullVoid(value_);
            size_t numValueBytes = HAPStringGetNumBytes(*value);
            HAPPrecondition(!fmt->callbacks.isValid || fmt->callbacks.isValid(*value));
            HAPPrecondition(HAPUTF8IsValidData(*value, numValueBytes));
            HAPPrecondition(numValueBytes >= fmt->constraints.minLength);
            HAPPrecondition(numValueBytes <= fmt->constraints.maxLength);
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
            if (maxBytes < numValueBytes) {
                HAPLogTLV(&logObject, tlvType, debugDescription, "Not enough memory to encode string value.");
                return kHAPError_OutOfResources;
            }
            HAPRawBufferCopyBytes(bytes, *value, numValueBytes);
            *numBytes = numValueBytes;
            HAPAssert(*numBytes <= maxBytes);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Value: {
            const HAPValueTLVFormat* fmt = format_;
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
            HAPAssert(fmt->callbacks.encode);
            err = fmt->callbacks.encode(HAPNonnullVoid(value_), bytes, maxBytes, numBytes);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                HAPLogTLV(&logObject, tlvType, debugDescription, "Not enough memory to encode value.");
                return err;
            }
            HAPAssert(*numBytes <= maxBytes);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Sequence:
        case kHAPTLVFormatType_Struct:
        case kHAPTLVFormatType_Union: {
        }
            HAPFatalError();
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
static bool GetStructMemberIsSet(const HAPStructTLVMember* member, HAPTLVValue* value) {
    HAPPrecondition(member);
    HAPPrecondition(member->isOptional);
    HAPPrecondition(value);

    return *((bool*) &((char*) value)[member->isSetOffset]);
}

HAP_RESULT_USE_CHECK
static HAPTLVValue* GetStructMemberValue(const HAPStructTLVMember* member, HAPTLVValue* value) {
    HAPPrecondition(member);
    HAPPrecondition(value);

    return &((char*) value)[member->valueOffset];
}

HAP_RESULT_USE_CHECK
static HAPError HAPTLVWriterEncodeAggregate(
        HAPTLVWriterRef* writer,
        const HAPTLVFormat* format_,
        HAPTLVValue* value_,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel) {
    HAPPrecondition(writer);
    HAPPrecondition(format_);
    HAPPrecondition(HAPTLVFormatIsValid(format_));
    HAPPrecondition(HAPTLVFormatIsAggregate(format_));
    const HAPBaseTLVFormat* format = format_;
    HAPPrecondition(value_);
    HAPPrecondition(stringBuilder);

    HAPError err;

    if (format->type == kHAPTLVFormatType_Sequence) {
        const HAPSequenceTLVFormat* fmt HAP_UNUSED = format_;
        HAPSequenceTLVValue* value = value_;
        HAPPrecondition(value->enumerate);
        EnumerateSequenceTLVContext enumerateContext;
        HAPRawBufferZero(&enumerateContext, sizeof enumerateContext);
        enumerateContext.writer = writer;
        enumerateContext.format = format_;
        err = value->enumerate(&value->dataSource, EnumerateSequenceTLVCallback, &enumerateContext);
        if (!err) {
            err = enumerateContext.err;
        }
        if (err) {
            HAPAssert(
                    err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                    err == kHAPError_Busy);
            return err;
        }
    } else if (format->type == kHAPTLVFormatType_Struct) {
        const HAPStructTLVFormat* fmt = format_;
        HAPPrecondition(!fmt->callbacks.isValid || fmt->callbacks.isValid(HAPNonnullVoid(value_)));
        if (fmt->members) {
            for (size_t i = 0; fmt->members[i]; i++) {
                const HAPStructTLVMember* member = fmt->members[i];
                HAPTLVValue* memberValue = GetStructMemberValue(member, HAPNonnullVoid(value_));
                if (member->isFlat) {
                    HAPAssert(HAPTLVFormatIsAggregate(member->format));
                    HAPAssert(!member->isOptional);
                    err = HAPTLVWriterEncodeAggregate(writer, member->format, memberValue, stringBuilder, nestingLevel);
                    if (err) {
                        HAPAssert(
                                err == kHAPError_Unknown || err == kHAPError_InvalidState ||
                                err == kHAPError_OutOfResources || err == kHAPError_Busy);
                        return err;
                    }
                } else if (!member->isOptional || GetStructMemberIsSet(member, HAPNonnullVoid(value_))) {
                    err = HAPTLVWriterEncodeTLV(
                            writer,
                            member->tlvType,
                            member->debugDescription,
                            member->format,
                            memberValue,
                            stringBuilder,
                            nestingLevel);
                    if (err) {
                        HAPAssert(
                                err == kHAPError_Unknown || err == kHAPError_InvalidState ||
                                err == kHAPError_OutOfResources || err == kHAPError_Busy);
                        return err;
                    }
                }
            }
        }
    } else {
        const HAPUnionTLVFormat* fmt = format_;
        const HAPUnionTLVValue* value = value_;
        if (fmt->variants) {
            bool isValid = false;
            for (size_t i = 0; fmt->variants[i]; i++) {
                const HAPUnionTLVVariant* variant = fmt->variants[i];
                if (variant->tlvType != value->type) {
                    continue;
                }
                err = HAPTLVWriterEncodeTLV(
                        writer,
                        variant->tlvType,
                        variant->debugDescription,
                        variant->format,
                        &((char*) value_)[fmt->untaggedValueOffset],
                        stringBuilder,
                        nestingLevel);
                if (err) {
                    HAPAssert(
                            err == kHAPError_Unknown || err == kHAPError_InvalidState ||
                            err == kHAPError_OutOfResources || err == kHAPError_Busy);
                    return err;
                }
                isValid = true;
                break;
            }
            HAPPrecondition(isValid);
        }
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPTLVWriterEncodeVoid(HAPTLVWriterRef* writer, const HAPTLVFormat* format, HAPTLVValue* value) {
    HAPPrecondition(writer);
    HAPPrecondition(format);
    HAPPrecondition(HAPTLVFormatIsValid(format));
    HAPPrecondition(HAPTLVFormatIsAggregate(format));
    HAPPrecondition(value);

    HAPError err;

    char logBytes[kHAPTLVValue_MaxLogBytes + 1];
    HAPStringBuilderRef stringBuilder;
    HAPStringBuilderCreate(&stringBuilder, logBytes, sizeof logBytes);

    err = HAPTLVWriterEncodeAggregate(writer, format, value, &stringBuilder, /* nestingLevel: */ 0);
    if (err) {
        HAPAssert(
                err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                err == kHAPError_Busy);
        return err;
    }

    if (HAPStringBuilderDidOverflow(&stringBuilder)) {
        HAPLogError(&logObject, "Logs were truncated.");
    }
    HAPLogDebug(&logObject, "Encoded TLV:%s", HAPStringBuilderGetString(&stringBuilder));
    return kHAPError_None;
}
