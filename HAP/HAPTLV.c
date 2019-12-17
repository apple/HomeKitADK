// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
bool HAPTLVFormatIsAggregate(const HAPTLVFormat* format_) {
    HAPPrecondition(format_);
    const HAPBaseTLVFormat* format = format_;

    switch (format->type) {
        case kHAPTLVFormatType_None:
        case kHAPTLVFormatType_Enum:
        case kHAPTLVFormatType_UInt8:
        case kHAPTLVFormatType_UInt16:
        case kHAPTLVFormatType_UInt32:
        case kHAPTLVFormatType_UInt64:
        case kHAPTLVFormatType_Int8:
        case kHAPTLVFormatType_Int16:
        case kHAPTLVFormatType_Int32:
        case kHAPTLVFormatType_Int64:
        case kHAPTLVFormatType_Data:
        case kHAPTLVFormatType_String:
        case kHAPTLVFormatType_Value: {
        }
            return false;
        case kHAPTLVFormatType_Sequence:
        case kHAPTLVFormatType_Struct:
        case kHAPTLVFormatType_Union: {
        }
            return true;
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
bool HAPTLVFormatUsesType(const HAPTLVFormat* format_, HAPTLVType tlvType) {
    HAPPrecondition(format_);
    const HAPBaseTLVFormat* format = format_;

    if (!HAPTLVFormatIsAggregate(format_)) {
        return false;
    }
    if (format->type == kHAPTLVFormatType_Sequence) {
        const HAPSequenceTLVFormat* fmt = format_;
        if (fmt->item.isFlat) {
            if (HAPTLVFormatUsesType(fmt->item.format, tlvType)) {
                return true;
            }
        } else {
            if (tlvType == fmt->item.tlvType) {
                return true;
            }
        }
        if (tlvType == fmt->separator.tlvType) {
            return true;
        }
    } else if (format->type == kHAPTLVFormatType_Struct) {
        const HAPStructTLVFormat* fmt = format_;
        if (fmt->members) {
            for (size_t i = 0; fmt->members[i]; i++) {
                const HAPStructTLVMember* member = fmt->members[i];
                if (member->isFlat) {
                    if (HAPTLVFormatUsesType(member->format, tlvType)) {
                        return true;
                    }
                } else {
                    if (tlvType == member->tlvType) {
                        return true;
                    }
                }
            }
        }
    } else {
        HAPAssert(format->type == kHAPTLVFormatType_Union);
        const HAPUnionTLVFormat* fmt = format_;
        if (fmt->variants) {
            for (size_t i = 0; fmt->variants[i]; i++) {
                const HAPUnionTLVVariant* variant = fmt->variants[i];
                if (tlvType == variant->tlvType) {
                    return true;
                }
            }
        }
    }
    return false;
}

HAP_RESULT_USE_CHECK
bool HAPTLVFormatHaveConflictingTypes(const HAPTLVFormat* format_, const HAPTLVFormat* otherFormat) {
    HAPPrecondition(format_);
    const HAPBaseTLVFormat* format = format_;
    HAPPrecondition(otherFormat);

    if (!HAPTLVFormatIsAggregate(format_) || !HAPTLVFormatIsAggregate(otherFormat)) {
        return false;
    }
    if (format->type == kHAPTLVFormatType_Sequence) {
        const HAPSequenceTLVFormat* fmt = format_;
        if (fmt->item.isFlat) {
            if (HAPTLVFormatUsesType(otherFormat, fmt->item.tlvType)) {
                return true;
            }
        }
    } else if (format->type == kHAPTLVFormatType_Struct) {
        const HAPStructTLVFormat* fmt = format_;
        if (fmt->members) {
            for (size_t i = 0; fmt->members[i]; i++) {
                const HAPStructTLVMember* member = fmt->members[i];
                if (member->isFlat) {
                    if (HAPTLVFormatUsesType(otherFormat, member->tlvType)) {
                        return true;
                    }
                }
            }
        }
    } else {
        HAPAssert(format->type == kHAPTLVFormatType_Union);
    }
    return false;
}

HAP_RESULT_USE_CHECK
bool HAPTLVFormatIsValid(const HAPTLVFormat* format_) {
    HAPPrecondition(format_);
    const HAPBaseTLVFormat* format = format_;

    switch (format->type) {
        case kHAPTLVFormatType_None: {
            const HAPSeparatorTLVFormat* fmt HAP_UNUSED = format_;
        }
            return true;
        case kHAPTLVFormatType_Enum: {
            const HAPEnumTLVFormat* fmt = format_;
            if (!fmt->callbacks.isValid) {
                return false;
            }
            if (!fmt->callbacks.getDescription) {
                return false;
            }
        }
            return true;
#define PROCESS_INTEGER_FORMAT(formatName, typeName, printfFormat, printfTypeName) \
    do { \
        const formatName* fmt = format_; \
        if (fmt->constraints.maximumValue < fmt->constraints.minimumValue) { \
            return false; \
        } \
    } while (0)
        case kHAPTLVFormatType_UInt8: {
            PROCESS_INTEGER_FORMAT(HAPUInt8TLVFormat, uint8_t, "u", unsigned int);
        }
            return true;
        case kHAPTLVFormatType_UInt16: {
            PROCESS_INTEGER_FORMAT(HAPUInt16TLVFormat, uint16_t, "u", unsigned int);
        }
            return true;
        case kHAPTLVFormatType_UInt32: {
            PROCESS_INTEGER_FORMAT(HAPUInt32TLVFormat, uint32_t, "lu", unsigned long);
        }
            return true;
        case kHAPTLVFormatType_UInt64: {
            PROCESS_INTEGER_FORMAT(HAPUInt64TLVFormat, uint64_t, "llu", unsigned long long);
        }
            return true;
        case kHAPTLVFormatType_Int8: {
            PROCESS_INTEGER_FORMAT(HAPInt8TLVFormat, int8_t, "d", int);
        }
            return true;
        case kHAPTLVFormatType_Int16: {
            PROCESS_INTEGER_FORMAT(HAPInt16TLVFormat, int16_t, "d", int);
        }
            return true;
        case kHAPTLVFormatType_Int32: {
            PROCESS_INTEGER_FORMAT(HAPInt32TLVFormat, int32_t, "ld", long);
        }
            return true;
        case kHAPTLVFormatType_Int64: {
            PROCESS_INTEGER_FORMAT(HAPInt64TLVFormat, int64_t, "lld", long long);
        }
            return true;
#undef PROCESS_INTEGER_FORMAT
        case kHAPTLVFormatType_Data: {
            const HAPDataTLVFormat* fmt = format_;
            if (fmt->constraints.maxLength < fmt->constraints.minLength) {
                return false;
            }
        }
            return true;
        case kHAPTLVFormatType_String: {
            const HAPStringTLVFormat* fmt = format_;
            if (fmt->constraints.maxLength < fmt->constraints.minLength) {
                return false;
            }
        }
            return true;
        case kHAPTLVFormatType_Value: {
            const HAPValueTLVFormat* fmt = format_;
            if (!fmt->callbacks.decode) {
                return false;
            }
            if (!fmt->callbacks.encode) {
                return false;
            }
            if (!fmt->callbacks.getDescription) {
                return false;
            }
        }
            return true;
        case kHAPTLVFormatType_Sequence: {
            const HAPSequenceTLVFormat* fmt = format_;
            if (!fmt->item.format) {
                return false;
            }
            if (!HAPTLVFormatIsValid(fmt->item.format)) {
                return false;
            }
            if (!fmt->separator.format) {
                return false;
            }
            if (fmt->item.isFlat) {
                if (!HAPTLVFormatIsAggregate(fmt->item.format)) {
                    return false;
                }
                if (((const HAPBaseTLVFormat*) fmt->item.format)->type != kHAPTLVFormatType_Union) {
                    return false;
                }
                if (HAPTLVFormatUsesType(fmt->item.format, fmt->separator.tlvType)) {
                    return false;
                }
            } else if (fmt->item.tlvType == fmt->separator.tlvType) {
                return false;
            }
        }
            return true;
        case kHAPTLVFormatType_Struct: {
            const HAPStructTLVFormat* fmt = format_;
            if (fmt->members) {
                for (size_t i = 0; fmt->members[i]; i++) {
                    const HAPStructTLVMember* member = fmt->members[i];
                    if (!member->format) {
                        return false;
                    }
                    if (!HAPTLVFormatIsValid(member->format)) {
                        return false;
                    }
                    for (size_t j = 0; j < i; j++) {
                        const HAPStructTLVMember* otherMember = fmt->members[j];
                        if (member->isFlat) {
                            if (!HAPTLVFormatIsAggregate(member->format)) {
                                return false;
                            }
                            if (member->isOptional) {
                                return false;
                            }
                            if (otherMember->isFlat) {
                                if (HAPTLVFormatHaveConflictingTypes(member->format, otherMember->format)) {
                                    return false;
                                }
                            } else {
                                if (HAPTLVFormatUsesType(member->format, otherMember->tlvType)) {
                                    return false;
                                }
                            }
                        } else {
                            if (otherMember->isFlat) {
                                if (HAPTLVFormatUsesType(otherMember->format, member->tlvType)) {
                                    return false;
                                }
                            } else {
                                if (member->tlvType == otherMember->tlvType) {
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
        }
            return true;
        case kHAPTLVFormatType_Union: {
            const HAPUnionTLVFormat* fmt = format_;
            if (fmt->variants) {
                for (size_t i = 0; fmt->variants[i]; i++) {
                    const HAPUnionTLVVariant* variant = fmt->variants[i];
                    if (!variant->format) {
                        return false;
                    }
                    if (!HAPTLVFormatIsValid(variant->format)) {
                        return false;
                    }
                    for (size_t j = 0; j < i; j++) {
                        const HAPUnionTLVVariant* otherVariant = fmt->variants[j];
                        if (variant->tlvType == otherVariant->tlvType) {
                            return false;
                        }
                    }
                }
            }
        }
            return true;
    }
    return false;
}

void HAPTLVAppendToLog(
        HAPTLVType tlvType,
        const char* debugDescription,
        const HAPTLVFormat* format_,
        HAPTLVValue* _Nullable value_,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel) {
    HAPPrecondition(debugDescription);
    HAPPrecondition(format_);
    const HAPBaseTLVFormat* format = format_;
    HAPPrecondition(stringBuilder);

    HAPError err;

    HAPStringBuilderAppend(stringBuilder, "\n");
    for (size_t i = 0; i < nestingLevel; i++) {
        HAPStringBuilderAppend(stringBuilder, "  ");
    }
    HAPStringBuilderAppend(stringBuilder, "- [%02X %s] ", tlvType, debugDescription);

    switch (format->type) {
        case kHAPTLVFormatType_None: {
            return;
        }
        case kHAPTLVFormatType_Enum: {
            const HAPEnumTLVFormat* fmt = format_;
            HAPPrecondition(fmt->callbacks.getDescription);
            uint8_t* value = HAPNonnullVoid(value_);
            HAPStringBuilderAppend(stringBuilder, "%s (%u)", fmt->callbacks.getDescription(*value), *value);
            return;
        }
#define PROCESS_INTEGER_FORMAT(formatName, typeName, printfFormat, printfTypeName) \
    do { \
        const formatName* fmt = format_; \
        typeName* value = HAPNonnullVoid(value_); \
        bool appended = false; \
        if (fmt->callbacks.getDescription) { \
            const char* _Nullable description = fmt->callbacks.getDescription(*value); \
            if (description) { \
                HAPStringBuilderAppend(stringBuilder, "%s (%" printfFormat ")", description, (printfTypeName) *value); \
                appended = true; \
            } \
        } \
        if (!appended && fmt->callbacks.getBitDescription) { \
            HAPStringBuilderAppend(stringBuilder, "["); \
            bool needsSeparator = false; \
            for (uint8_t i = 0; i < sizeof(typeName) * CHAR_BIT; i++) { \
                typeName optionValue = (typeName)(1 << i); \
                if (*value & optionValue) { \
                    if (needsSeparator) { \
                        HAPStringBuilderAppend(stringBuilder, ", "); \
                    } \
                    needsSeparator = true; \
                    const char* _Nullable bitDescription = fmt->callbacks.getBitDescription(optionValue); \
                    if (bitDescription) { \
                        HAPStringBuilderAppend(stringBuilder, "%s", bitDescription); \
                    } else { \
                        HAPStringBuilderAppend(stringBuilder, "<Unknown bit>"); \
                    } \
                    HAPStringBuilderAppend(stringBuilder, " (bit %u)", i); \
                } \
            } \
            HAPStringBuilderAppend(stringBuilder, "]"); \
            appended = true; \
        } \
        if (!appended) { \
            HAPStringBuilderAppend(stringBuilder, "%" printfFormat, (printfTypeName) *value); \
        } \
    } while (0)
        case kHAPTLVFormatType_UInt8: {
            PROCESS_INTEGER_FORMAT(HAPUInt8TLVFormat, uint8_t, "u", unsigned int);
            return;
        }
        case kHAPTLVFormatType_UInt16: {
            PROCESS_INTEGER_FORMAT(HAPUInt16TLVFormat, uint16_t, "u", unsigned int);
            return;
        }
        case kHAPTLVFormatType_UInt32: {
            PROCESS_INTEGER_FORMAT(HAPUInt32TLVFormat, uint32_t, "lu", unsigned long);
            return;
        }
        case kHAPTLVFormatType_UInt64: {
            PROCESS_INTEGER_FORMAT(HAPUInt64TLVFormat, uint64_t, "llu", unsigned long long);
            return;
        }
#undef PROCESS_INTEGER_FORMAT
#define PROCESS_INTEGER_FORMAT(formatName, typeName, printfFormat, printfTypeName) \
    do { \
        const formatName* fmt = format_; \
        typeName* value = HAPNonnullVoid(value_); \
        bool appended = false; \
        if (fmt->callbacks.getDescription) { \
            const char* _Nullable description = fmt->callbacks.getDescription(*value); \
            if (description) { \
                HAPStringBuilderAppend(stringBuilder, "%s (%" printfFormat ")", description, (printfTypeName) *value); \
                appended = true; \
            } \
        } \
        if (!appended) { \
            HAPStringBuilderAppend(stringBuilder, "%" printfFormat, (printfTypeName) *value); \
        } \
    } while (0)
        case kHAPTLVFormatType_Int8: {
            PROCESS_INTEGER_FORMAT(HAPInt8TLVFormat, int8_t, "d", int);
            return;
        }
        case kHAPTLVFormatType_Int16: {
            PROCESS_INTEGER_FORMAT(HAPInt16TLVFormat, int16_t, "d", int);
            return;
        }
        case kHAPTLVFormatType_Int32: {
            PROCESS_INTEGER_FORMAT(HAPInt32TLVFormat, int32_t, "ld", long);
            return;
        }
        case kHAPTLVFormatType_Int64: {
            PROCESS_INTEGER_FORMAT(HAPInt64TLVFormat, int64_t, "lld", long long);
            return;
        }
#undef PROCESS_INTEGER_FORMAT
        case kHAPTLVFormatType_Data: {
            const HAPDataTLVFormat* fmt HAP_UNUSED = format_;
            HAPDataTLVValue* value = HAPNonnullVoid(value_);
            HAPStringBuilderAppend(stringBuilder, "<");
            for (size_t i = 0; i < value->numBytes; i++) {
                const uint8_t* b = value->bytes;
                if (i && !(i % 4)) {
                    HAPStringBuilderAppend(stringBuilder, " ");
                }
                HAPStringBuilderAppend(stringBuilder, "%02X", b[i]);
            }
            HAPStringBuilderAppend(stringBuilder, ">");
            return;
        }
        case kHAPTLVFormatType_String: {
            const HAPStringTLVFormat* fmt HAP_UNUSED = format_;
            char** value = HAPNonnullVoid(value_);
            HAPStringBuilderAppend(stringBuilder, "%s", *value);
            return;
        }
        case kHAPTLVFormatType_Value: {
            const HAPValueTLVFormat* fmt = format_;
            HAPPrecondition(fmt->callbacks.getDescription);

            char descriptionBytes[kHAPTLVValue_MaxDescriptionBytes + 1];
            err = fmt->callbacks.getDescription(HAPNonnullVoid(value_), descriptionBytes, sizeof descriptionBytes);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                HAPStringBuilderAppend(stringBuilder, "<Description too long>");
            } else {
                HAPStringBuilderAppend(stringBuilder, "%s", descriptionBytes);
            }
            return;
        }
        case kHAPTLVFormatType_Sequence: {
            const HAPSequenceTLVFormat* fmt HAP_UNUSED = format_;
            HAPStringBuilderAppend(stringBuilder, "<Sequence>");
            return;
        }
        case kHAPTLVFormatType_Struct: {
            const HAPStructTLVFormat* fmt HAP_UNUSED = format_;
            return;
        }
        case kHAPTLVFormatType_Union: {
            const HAPUnionTLVFormat* fmt HAP_UNUSED = format_;
            return;
        }
    }
    HAPFatalError();
}
