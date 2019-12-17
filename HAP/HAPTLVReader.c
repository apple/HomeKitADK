// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "TLVReader" };

void HAPTLVReaderCreateWithOptions(HAPTLVReaderRef* reader_, const HAPTLVReaderOptions* options) {
    HAPPrecondition(reader_);
    HAPTLVReader* reader = (HAPTLVReader*) reader_;
    HAPPrecondition(options);
    if (options->numBytes || options->maxBytes) {
        HAPPrecondition(options->bytes);
    }
    HAPPrecondition(options->numBytes <= options->maxBytes);

    // Initialize reader.
    HAPRawBufferZero(reader, sizeof *reader);
    reader->bytes = options->bytes;
    reader->numBytes = options->numBytes;
    reader->maxBytes = options->maxBytes;
}

void HAPTLVReaderCreate(HAPTLVReaderRef* reader, void* _Nullable bytes, size_t numBytes) {
    HAPPrecondition(reader);
    HAPPrecondition(!numBytes || bytes);

    HAPTLVReaderCreateWithOptions(
            reader, &(const HAPTLVReaderOptions) { .bytes = bytes, .numBytes = numBytes, .maxBytes = numBytes });
}

HAP_RESULT_USE_CHECK
HAPError HAPTLVReaderGetNext(HAPTLVReaderRef* reader_, bool* found, HAPTLV* tlv) {
    HAPPrecondition(reader_);
    HAPTLVReader* reader = (HAPTLVReader*) reader_;
    HAPPrecondition(found);
    HAPPrecondition(tlv);

    *found = false;

    uint8_t* bytes = reader->bytes;
    size_t maxBytes = reader->numBytes;
    size_t o = 0;

    if (!maxBytes) {
        return kHAPError_None;
    }
    HAPAssert(bytes);

    // Read TLV header.
    if (maxBytes < 2) {
        HAPLog(&logObject, "Found incomplete TLV fragment header with length %zu.", maxBytes);
        return kHAPError_InvalidData;
    }
    tlv->type = bytes[o];
    tlv->value.numBytes = 0;
    tlv->value.bytes = &bytes[o];
    size_t numFragmentBytes = bytes[o + 1];
    maxBytes -= 2;

    size_t numFragments = 0;

    // Read TLV body.
    if (maxBytes < numFragmentBytes) {
        HAPLog(&logObject, "Found incomplete TLV fragment body with length %zu.", maxBytes);
        return kHAPError_InvalidData;
    }
    HAPRawBufferCopyBytes(&bytes[o - 2 * numFragments], &bytes[o + 2], numFragmentBytes);
    numFragments++;
    tlv->value.numBytes += numFragmentBytes;
    o += numFragmentBytes;
    maxBytes -= numFragmentBytes;
    o += 2;
    HAPRawBufferZero(&bytes[o - 2 * numFragments], 2 * numFragments);

    // Read additional chunks (long TLV).
    while (maxBytes && bytes[o] == tlv->type) {
        // Read TLV header.
        if (maxBytes < 2) {
            HAPLog(&logObject, "Found incomplete TLV fragment header with length %zu.", maxBytes);
            return kHAPError_InvalidData;
        }

        // Only the last TLV fragment item in series of contiguous TLV fragment items may have non-255 byte length.
        if (tlv->value.numBytes != numFragments * UINT8_MAX) {
            HAPLog(&logObject, "Found additional TLV fragment after previous fragment with non-255 byte length.");
            return kHAPError_InvalidData;
        }

        // Each TLV fragment item must have a non-0 length.
        numFragmentBytes = bytes[o + 1];
        if (!numFragmentBytes) {
            HAPLog(&logObject, "Found TLV fragment item with 0 length.");
            return kHAPError_InvalidData;
        }

        maxBytes -= 2;

        // Merge TLV body.
        if (maxBytes < numFragmentBytes) {
            HAPLog(&logObject, "Found incomplete TLV fragment body with length %zu.", maxBytes);
            return kHAPError_InvalidData;
        }
        HAPRawBufferCopyBytes(&bytes[o - 2 * numFragments], &bytes[o + 2], numFragmentBytes);
        numFragments++;
        tlv->value.numBytes += numFragmentBytes;
        o += numFragmentBytes;
        maxBytes -= numFragmentBytes;
        o += 2;
        HAPRawBufferZero(&bytes[o - 2 * numFragments], 2 * numFragments);
    }

    // Update reader state.
    reader->bytes = &bytes[o];
    reader->numBytes -= o;
    reader->maxBytes -= o;

    *found = true;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPTLVReaderGetAll(HAPTLVReaderRef* reader, HAPTLV* _Nullable const* _Nonnull tlvs) {
    HAPPrecondition(reader);
    HAPPrecondition(tlvs);

    HAPError err;

    for (HAPTLV* const* tlvItem = tlvs; *tlvItem; tlvItem++) {
        // Check for duplicate type.
        for (HAPTLV* const* otherTLVItem = tlvs; *otherTLVItem != *tlvItem; otherTLVItem++) {
            HAPPrecondition((*tlvItem)->type != (*otherTLVItem)->type);
        }

        // Initialize TLV.
        (*tlvItem)->value.numBytes = 0;
        (*tlvItem)->value.bytes = NULL;
    }

    for (;;) {
        // Fetch TLV.
        HAPTLV tlv;
        bool valid;
        err = HAPTLVReaderGetNext(reader, &valid, &tlv);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }

        // Last TLV read?
        if (!valid) {
            break;
        }

        // Match TLV.
        HAPTLV* const* tlvItem;
        for (tlvItem = tlvs; *tlvItem; tlvItem++) {
            if ((*tlvItem)->type == tlv.type) {
                if ((*tlvItem)->value.bytes) {
                    // Duplicate TLV with same type found.
                    HAPLog(&logObject, "[%02x] Duplicate TLV.", tlv.type);
                    return kHAPError_InvalidData;
                }

                // TLV found. Save.
                **tlvItem = tlv;
                break;
            }
        }
        if (!*tlvItem) {
            HAPLog(&logObject, "[%02x] TLV item ignored.", tlv.type);
        }
    }

    return kHAPError_None;
}

void HAPTLVReaderGetScratchBytes(
        const HAPTLVReaderRef* reader_,
        void* _Nonnull* _Nonnull scratchBytes,
        size_t* numScratchBytes) {
    HAPPrecondition(reader_);
    const HAPTLVReader* reader = (const HAPTLVReader*) reader_;
    HAPPrecondition(scratchBytes);
    HAPPrecondition(numScratchBytes);

    uint8_t* bytes = reader->bytes;
    *scratchBytes = &bytes[reader->numBytes];
    *numScratchBytes = reader->maxBytes - reader->numBytes;
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * Peeks at information of the first TLV item within a buffer.
 *
 * - This function may also be called on TLV items that have already been read.
 *
 * @param      reader_              TLV reader.
 * @param      tlvBytes_            Start of buffer.
 * @param      maxTLVBytes          Size of buffer. May contain more than one TLV item.
 * @param[out] tlvType              Type of the first TLV item within the buffer.
 * @param[out] numTLVBytes          Length of the first TLV item within the buffer, including all headers.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If data within the buffer is malformed.
 */
HAP_RESULT_USE_CHECK
static HAPError GetNextTLVInfo(
        const HAPTLVReaderRef* reader_,
        const void* tlvBytes_,
        size_t maxTLVBytes,
        HAPTLVType* tlvType,
        size_t* numTLVBytes) {
    HAPPrecondition(reader_);
    const HAPTLVReader* reader = (const HAPTLVReader*) reader_;
    HAPPrecondition(tlvBytes_);
    const uint8_t* tlvBytes = tlvBytes_;
    HAPPrecondition(tlvType);
    HAPPrecondition(numTLVBytes);

    *numTLVBytes = 0;

    if (maxTLVBytes - *numTLVBytes < 1) {
        HAPLogSensitiveBuffer(&logObject, tlvBytes_, maxTLVBytes, "Malformed TLV item.");
        return kHAPError_InvalidData;
    }
    *tlvType = tlvBytes[(*numTLVBytes)++];

    if (reader->isNonSequentialAccessEnabled && *tlvType == reader->tlvTypes.singleFragment) {
        HAPAssert(maxTLVBytes - *numTLVBytes >= 1);
        uint8_t numValueBytes = tlvBytes[(*numTLVBytes)++];
        HAPAssert(maxTLVBytes - *numTLVBytes >= numValueBytes);
        *numTLVBytes += numValueBytes;
    } else if (reader->isNonSequentialAccessEnabled && *tlvType == reader->tlvTypes.nullTerminatedSingleFragment) {
        size_t numValueBytes = HAPStringGetNumBytes((const void*) &tlvBytes[*numTLVBytes]);
        HAPAssert(maxTLVBytes - *numTLVBytes >= numValueBytes);
        *numTLVBytes += numValueBytes;
        HAPAssert(maxTLVBytes - *numTLVBytes >= 1);
        uint8_t zeroByte = tlvBytes[(*numTLVBytes)++];
        HAPAssert(!zeroByte);
    } else if (reader->isNonSequentialAccessEnabled && *tlvType == reader->tlvTypes.nullTerminatedMultiFragment) {
        size_t x = 0;
        size_t numFragments = 2;
        uint8_t partialNumFragments;
        do {
            HAPAssert(maxTLVBytes - *numTLVBytes >= 1);
            partialNumFragments = tlvBytes[(*numTLVBytes)++];
            numFragments += partialNumFragments;
            x++;
        } while (partialNumFragments == UINT8_MAX);
        HAPAssert(maxTLVBytes - *numTLVBytes >= 1);

        uint8_t numLastFragmentBytes = tlvBytes[(*numTLVBytes)++];

        size_t numZeros = 2 * (numFragments - 2) - (x - 1);
        HAPAssert(maxTLVBytes - *numTLVBytes >= numZeros);
        HAPAssert(HAPRawBufferIsZero(&tlvBytes[*numTLVBytes], numZeros));
        *numTLVBytes += numZeros;

        size_t numValueBytes = (numFragments - 1) * UINT8_MAX + numLastFragmentBytes;
        HAPAssert(maxTLVBytes - *numTLVBytes >= numValueBytes);
        *numTLVBytes += numValueBytes;

        HAPAssert(maxTLVBytes - *numTLVBytes >= 1);
        uint8_t zeroByte = tlvBytes[(*numTLVBytes)++];
        HAPAssert(!zeroByte);
    } else {
        if (maxTLVBytes - *numTLVBytes < 1) {
            HAPLogSensitiveBuffer(&logObject, tlvBytes_, maxTLVBytes, "Malformed TLV item.");
            return kHAPError_InvalidData;
        }
        uint8_t numFragmentBytes = tlvBytes[(*numTLVBytes)++];

        if (maxTLVBytes - *numTLVBytes < numFragmentBytes) {
            HAPLogSensitiveBuffer(&logObject, tlvBytes_, maxTLVBytes, "Malformed TLV item.");
            return kHAPError_InvalidData;
        }
        *numTLVBytes += numFragmentBytes;

        while (maxTLVBytes - *numTLVBytes && tlvBytes[*numTLVBytes] == *tlvType) {
            if (maxTLVBytes - *numTLVBytes < 2) {
                HAPLogSensitiveBuffer(&logObject, tlvBytes_, maxTLVBytes, "Malformed TLV item.");
                return kHAPError_InvalidData;
            }
            if (numFragmentBytes != UINT8_MAX) {
                HAPLogSensitiveBuffer(&logObject, tlvBytes_, maxTLVBytes, "Malformed TLV item.");
                return kHAPError_InvalidData;
            }
            *tlvType = tlvBytes[(*numTLVBytes)++];
            numFragmentBytes = tlvBytes[(*numTLVBytes)++];

            if (maxTLVBytes - *numTLVBytes < numFragmentBytes) {
                HAPLogSensitiveBuffer(&logObject, tlvBytes_, maxTLVBytes, "Malformed TLV item.");
                return kHAPError_InvalidData;
            }
            *numTLVBytes += numFragmentBytes;
        }
    }
    HAPAssert(*numTLVBytes <= maxTLVBytes);
    return kHAPError_None;
}

/**
 * Finds the first TLV item with a given TLV type within the buffer of a TLV reader.
 *
 * @param      reader_              TLV reader.
 * @param      tlvType              Type of the TLV item.
 * @param[out] tlvBytes             Start of buffer containing the TLV item, if found. NULL otherwise.
 * @param[out] numTLVBytes          Length of the buffer containing the TLV item, including all headers, if found.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If data within the buffer is malformed.
 */
HAP_RESULT_USE_CHECK
static HAPError FindTLVInfo(
        const HAPTLVReaderRef* reader_,
        HAPTLVType tlvType,
        void* _Nullable* _Nonnull tlvBytes,
        size_t* numTLVBytes) {
    HAPPrecondition(reader_);
    const HAPTLVReader* reader = (const HAPTLVReader*) reader_;
    HAPPrecondition(reader->isNonSequentialAccessEnabled);
    HAPPrecondition(tlvBytes);
    HAPPrecondition(numTLVBytes);

    HAPError err;

    *tlvBytes = NULL;
    *numTLVBytes = 0;

    uint8_t* bytes = reader->bytes;
    size_t maxBytes = reader->numBytes;
    size_t o = 0;
    while (o < maxBytes) {
        HAPTLVType type;
        size_t numBytes;
        err = GetNextTLVInfo(reader_, &bytes[o], maxBytes - o, &type, &numBytes);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }

        if (type == tlvType) {
            *tlvBytes = &bytes[o];
            *numTLVBytes = numBytes;
            return kHAPError_None;
        }

        o += numBytes;
    }

    return kHAPError_None;
}

/**
 * TLV format properties.
 */
HAP_OPTIONS_BEGIN(uint8_t, HAPTLVFormatProperties) {
    /** TLV value may contain NULL bytes. When read, it is not guaranteed to be NULL-terminated. */
    kHAPTLVFormatProperties_MayContainNullBytes = 1 << 0
} HAP_OPTIONS_END(uint8_t, HAPTLVFormatProperties);

/**
 * Reads a TLV item, merging fragments and NULL-terminating its value by default.
 *
 * - Each TLV item may only be read once.
 *
 * @param      reader_              TLV reader.
 * @param      tlvBytes_            TLV item buffer.
 * @param      numTLVBytes          Length of TLV item buffer. Must contain exactly one TLV item.
 * @param      formatProperties     TLV format properties.
 * @param[out] tlv                  TLV item.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If data within the TLV item buffer is malformed.
 */
HAP_RESULT_USE_CHECK
static HAPError
        ReadTLV(const HAPTLVReaderRef* reader_,
                void* tlvBytes_,
                size_t numTLVBytes,
                HAPTLVFormatProperties formatProperties,
                HAPTLV* tlv) {
    HAPPrecondition(reader_);
    const HAPTLVReader* reader = (const HAPTLVReader*) reader_;
    HAPPrecondition(tlvBytes_);
    uint8_t* tlvBytes = tlvBytes_;
    HAPPrecondition(tlv);

    HAPRawBufferZero(tlv, sizeof *tlv);

    // Only the last TLV fragment item in series of contiguous TLV fragment items may have non-255 byte length.
    // See HomeKit Accessory Protocol Specification R14
    // Section 15.1.1 TLV Rules
    const size_t numFullFragmentBytes = /* type: */ 1 + /* length: */ 1 + /* value: */ UINT8_MAX;
    if (numTLVBytes <= numFullFragmentBytes) {
        if (formatProperties & kHAPTLVFormatProperties_MayContainNullBytes) {
            HAPPrecondition(numTLVBytes >= 2);
            tlv->type = tlvBytes[0];
            tlv->value.numBytes = tlvBytes[1];
            HAPPrecondition(numTLVBytes == 2 + tlv->value.numBytes);

            tlvBytes[0] = reader->tlvTypes.singleFragment;
            tlvBytes[1] = (uint8_t) tlv->value.numBytes;
            tlv->value.bytes = &tlvBytes[2];
        } else {
            HAPPrecondition(numTLVBytes >= 2);
            tlv->type = tlvBytes[0];
            tlv->value.numBytes = tlvBytes[1];
            HAPPrecondition(numTLVBytes == 2 + tlv->value.numBytes);

            tlvBytes[0] = reader->tlvTypes.nullTerminatedSingleFragment;
            HAPRawBufferCopyBytes(&tlvBytes[1], &tlvBytes[2], tlv->value.numBytes);
            tlvBytes[1 + tlv->value.numBytes] = '\0';
            tlv->value.bytes = &tlvBytes[1];

            if (HAPStringGetNumBytes(HAPNonnullVoid(tlv->value.bytes)) != tlv->value.numBytes) {
                HAPLog(&logObject, "[%02x] TLV item contains unexpected NULL bytes.", tlv->type);
                return kHAPError_InvalidData;
            }
        }
    } else {
        HAPPrecondition(numTLVBytes >= 2);
        tlv->type = tlvBytes[0];
        tlv->value.numBytes = 0;
        size_t numFragments = 0;
        size_t numLastFragmentBytes = 0;

        for (size_t i = 0; i < numTLVBytes; i += numFullFragmentBytes) {
            HAPPrecondition(tlvBytes[i] == tlv->type);
            if (i) {
                HAPPrecondition(numLastFragmentBytes == UINT8_MAX);
            }
            numFragments++;
            numLastFragmentBytes = tlvBytes[i + 1];
            tlv->value.numBytes += numLastFragmentBytes;
        }
        HAPPrecondition(numTLVBytes == (numFragments - 1) * numFullFragmentBytes + 2 + numLastFragmentBytes);

        // Merge fragments.
        for (size_t i = 0; i < numFragments; i++) {
            HAPRawBufferCopyBytes(
                    &tlvBytes[numTLVBytes - /* NULL: */ 1 - numLastFragmentBytes - i * UINT8_MAX],
                    &tlvBytes[numTLVBytes - numLastFragmentBytes - i * numFullFragmentBytes],
                    i ? UINT8_MAX : numLastFragmentBytes);
        }
        tlv->value.bytes = &tlvBytes[numTLVBytes - /* NULL: */ 1 - tlv->value.numBytes];

        // See documentation in HAPTLV+Internal.h for background information.
        size_t o = 0;

        tlvBytes[o++] = reader->tlvTypes.nullTerminatedMultiFragment;

        HAPAssert(numFragments >= 2);
        size_t x = 0;
        size_t remainingFragments = numFragments - 2;
        while (remainingFragments >= UINT8_MAX) {
            tlvBytes[o++] = UINT8_MAX;
            remainingFragments -= UINT8_MAX;
            x++;
        }
        tlvBytes[o++] = (uint8_t) remainingFragments;
        x++;

        tlvBytes[o++] = (uint8_t) numLastFragmentBytes;

        size_t numZeros = 2 * (numFragments - 2) - (x - 1);
        HAPRawBufferZero(&tlvBytes[o], numZeros);
        o += numZeros;

        HAPAssert(&tlvBytes[o] == tlv->value.bytes);
        o += tlv->value.numBytes;

        tlvBytes[o++] = '\0';

        HAPAssert(o == numTLVBytes);
    }

    if (reader->isNonSequentialAccessEnabled) {
        HAPPrecondition(tlv->type != reader->tlvTypes.singleFragment);
        HAPPrecondition(tlv->type != reader->tlvTypes.nullTerminatedSingleFragment);
        HAPPrecondition(tlv->type != reader->tlvTypes.nullTerminatedMultiFragment);
    }
    return kHAPError_None;
}

/**
 * Pre-processes the buffer of a TLV reader to enable non-sequential access to TLV items.
 *
 * - Each TLV item may still only be read once.
 *
 * - The unused TLV types passed as arguments must be three distinct values.
 *
 * @param      reader_              TLV reader.
 * @param      unusedTLVType1       Unused TLV type that is not expected in the buffer. If it is, TLV item is skipped.
 * @param      unusedTLVType2       Unused TLV type that is not expected in the buffer. If it is, TLV item is skipped.
 * @param      unusedTLVType3       Unused TLV type that is not expected in the buffer. If it is, TLV item is skipped.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If data within the TLV item buffer is malformed.
 */
HAP_RESULT_USE_CHECK
static HAPError EnableNonSequentialAccess(
        HAPTLVReaderRef* reader_,
        HAPTLVType unusedTLVType1,
        HAPTLVType unusedTLVType2,
        HAPTLVType unusedTLVType3) {
    HAPPrecondition(reader_);
    HAPTLVReader* reader = (HAPTLVReader*) reader_;
    HAPPrecondition(!reader->isNonSequentialAccessEnabled);
    HAPPrecondition(unusedTLVType2 != unusedTLVType1);
    HAPPrecondition(unusedTLVType3 != unusedTLVType1 && unusedTLVType3 != unusedTLVType2);

    HAPError err;

    reader->tlvTypes.singleFragment = unusedTLVType1;
    reader->tlvTypes.nullTerminatedSingleFragment = unusedTLVType2;
    reader->tlvTypes.nullTerminatedMultiFragment = unusedTLVType3;

    // Read all TLVs that happen to have a reserved type.
    // Other TLVs remain unprocessed to be available for later reading.
    uint8_t* tlvBytes = reader->bytes;
    size_t maxTLVBytes = reader->numBytes;
    size_t o = 0;
    while (o < maxTLVBytes) {
        HAPTLVType tlvType;
        size_t numTLVBytes;
        err = GetNextTLVInfo(reader_, &tlvBytes[o], maxTLVBytes - o, &tlvType, &numTLVBytes);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }

        if (tlvType == unusedTLVType1 || tlvType == unusedTLVType2 || tlvType == unusedTLVType3) {
            HAPLog(&logObject, "[%02x] Ignoring TLV item with reserved type.", tlvType);
            HAPTLV tlv;
            err = ReadTLV(reader_, &tlvBytes[o], numTLVBytes, kHAPTLVFormatProperties_MayContainNullBytes, &tlv);
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
                return err;
            }
            HAPLogSensitiveBuffer(&logObject, tlv.value.bytes, tlv.value.numBytes, "[%02x] Ignored TLV.", tlvType);
        }

        o += numTLVBytes;
    }

    reader->isNonSequentialAccessEnabled = true;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static bool HAPTLVReaderIsTypeReserved(const HAPTLVReaderRef* reader_, HAPTLVType tlvType) {
    HAPPrecondition(reader_);
    const HAPTLVReader* reader = (const HAPTLVReader*) reader_;

    if (!reader->isNonSequentialAccessEnabled) {
        return false;
    }

    return tlvType == reader->tlvTypes.singleFragment || tlvType == reader->tlvTypes.nullTerminatedSingleFragment ||
           tlvType == reader->tlvTypes.nullTerminatedMultiFragment;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
static HAPError EnableNonSequentialAccessWithFormat(HAPTLVReaderRef* reader, const HAPTLVFormat* format) {
    HAPPrecondition(reader);
    HAPPrecondition(format);
    HAPPrecondition(HAPTLVFormatIsValid(format));

    HAPError err;

    HAPTLVType unusedTLVTypes[3];
    size_t unusedTLVTypeIndex = 0;
    for (uint16_t tlvType = 0; tlvType <= UINT8_MAX && unusedTLVTypeIndex < HAPArrayCount(unusedTLVTypes); tlvType++) {
        if (!HAPTLVFormatUsesType(format, (HAPTLVType) tlvType)) {
            unusedTLVTypes[unusedTLVTypeIndex++] = (HAPTLVType) tlvType;
        }
    }
    if (unusedTLVTypeIndex < HAPArrayCount(unusedTLVTypes)) {
        HAPLogError(&logObject, "Can only parse up to 253 distinct TLV types.");
        return kHAPError_InvalidData;
    }

    err = EnableNonSequentialAccess(reader, unusedTLVTypes[0], unusedTLVTypes[1], unusedTLVTypes[2]);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPError SkipUnexpectedValues(HAPTLVReaderRef* reader_, const HAPTLVFormat* format) {
    HAPPrecondition(reader_);
    HAPTLVReader* reader = (HAPTLVReader*) reader_;
    HAPPrecondition(format);

    HAPError err;

    uint8_t* tlvBytes = reader->bytes;
    size_t maxTLVBytes = reader->numBytes;
    size_t o = 0;
    while (o < maxTLVBytes) {
        HAPTLVType tlvType;
        size_t numTLVBytes;
        err = GetNextTLVInfo(reader_, &tlvBytes[o], maxTLVBytes - o, &tlvType, &numTLVBytes);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }

        if (!HAPTLVReaderIsTypeReserved(reader_, tlvType) && !HAPTLVFormatUsesType(format, tlvType)) {
            HAPTLV tlv;
            err = ReadTLV(reader_, &tlvBytes[o], numTLVBytes, kHAPTLVFormatProperties_MayContainNullBytes, &tlv);
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
                return err;
            }
            HAPLogSensitiveBuffer(&logObject, tlv.value.bytes, tlv.value.numBytes, "[%02x] Ignored TLV.", tlv.type);
        }

        o += numTLVBytes;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

static HAPTLVFormatProperties HAPGetFormatProperties(const HAPTLVFormat* format) {
    HAPPrecondition(format);

    switch (((const HAPBaseTLVFormat*) format)->type) {
        case kHAPTLVFormatType_String: {
            return 0;
        }
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
        case kHAPTLVFormatType_Value:
        case kHAPTLVFormatType_Sequence:
        case kHAPTLVFormatType_Struct:
        case kHAPTLVFormatType_Union: {
            return kHAPTLVFormatProperties_MayContainNullBytes;
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
static HAPError HAPTLVReaderDecodeScalar(
        void* bytes,
        size_t numBytes,
        HAPTLVType tlvType,
        const char* debugDescription,
        const HAPTLVFormat* format,
        HAPTLVValue* _Nullable value,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel);

HAP_RESULT_USE_CHECK
static HAPError HAPTLVReaderDecodeAggregate(
        HAPTLVReaderRef* reader,
        const HAPTLVFormat* format,
        HAPTLVValue* value,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel);

typedef struct {
    const HAPSequenceTLVFormat* format;
    HAPTLVReaderRef reader;
} HAPSequenceTLVDataSource;
HAP_STATIC_ASSERT(sizeof(HAPSequenceTLVDataSource) <= sizeof(HAPSequenceTLVDataSourceRef), HAPSequenceTLVDataSource);

HAP_RESULT_USE_CHECK
static HAPError EnumerateSequenceTLV(
        HAPSequenceTLVDataSourceRef* dataSource_,
        HAPSequenceTLVEnumerateCallback callback,
        void* _Nullable context) {
    HAPPrecondition(dataSource_);
    HAPSequenceTLVDataSource* dataSource = (HAPSequenceTLVDataSource*) dataSource_;
    HAPPrecondition(dataSource->format);
    HAPPrecondition(HAPTLVFormatIsValid(dataSource->format));
    const HAPSequenceTLVFormat* fmt = dataSource->format;
    const HAPTLVReaderRef* reader_ = &dataSource->reader;
    const HAPTLVReader* reader = (const HAPTLVReader*) reader_;
    HAPPrecondition(callback);

    HAPError err;

    void* value = &((char*) dataSource_)[fmt->item.valueOffset - HAP_OFFSETOF(HAPSequenceTLVValue, dataSource)];

    HAPLogDebug(&logObject, "Decoding sequence TLV.");

    uint8_t* tlvBytes = reader->bytes;
    size_t maxTLVBytes = reader->numBytes;
    size_t o = 0;
    bool shouldContinue = true;
    while (o < maxTLVBytes) {
        char logBytes[kHAPTLVValue_MaxLogBytes + 1];
        HAPStringBuilderRef stringBuilder;
        HAPStringBuilderCreate(&stringBuilder, logBytes, sizeof logBytes);

        HAPTLVType tlvType;
        size_t numTLVBytes;
        err = GetNextTLVInfo(reader_, &tlvBytes[o], maxTLVBytes - o, &tlvType, &numTLVBytes);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }

        if (fmt->item.isFlat && HAPTLVFormatUsesType(fmt->item.format, tlvType)) {
            HAPAssert(HAPTLVFormatIsAggregate(fmt->item.format));
            HAPAssert(((const HAPBaseTLVFormat*) fmt->item.format)->type == kHAPTLVFormatType_Union);

            // Create a copy of the TLV reader that wraps just one item.
            // By creating a copy inner state regarding non-sequential access is preserved.
            HAPTLVReaderRef itemReader_;
            HAPRawBufferCopyBytes(&itemReader_, reader_, sizeof itemReader_);
            HAPTLVReader* itemReader = (HAPTLVReader*) &itemReader_;
            itemReader->bytes = &tlvBytes[o];
            itemReader->numBytes = numTLVBytes;
            itemReader->maxBytes = numTLVBytes;

            err = HAPTLVReaderDecodeAggregate(
                    &itemReader_, fmt->item.format, value, &stringBuilder, /* nestingLevel: */ 0);
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
                HAPLog(&logObject, "Invalid value.");
                return err;
            }

            if (HAPStringBuilderDidOverflow(&stringBuilder)) {
                HAPLogError(&logObject, "Logs were truncated.");
            }
            HAPLogDebug(&logObject, "Decoded sequence TLV:%s", HAPStringBuilderGetString(&stringBuilder));

            if (shouldContinue) {
                callback(context, value, &shouldContinue);
            }
        } else if (!fmt->item.isFlat && tlvType == fmt->item.tlvType) {
            HAPTLV tlv;
            err = ReadTLV(reader_, &tlvBytes[o], numTLVBytes, HAPGetFormatProperties(fmt->item.format), &tlv);
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
                return err;
            }
            HAPAssert(tlv.type == fmt->item.tlvType);

            if (HAPTLVFormatIsAggregate(fmt->item.format)) {
                HAPTLVAppendToLog(
                        fmt->item.tlvType,
                        fmt->item.debugDescription,
                        fmt,
                        NULL,
                        &stringBuilder,
                        /* nestingLevel: */ 0);
                HAPTLVReaderRef subReader;
                HAPTLVReaderCreate(&subReader, (void*) (uintptr_t) tlv.value.bytes, tlv.value.numBytes);
                err = EnableNonSequentialAccessWithFormat(&subReader, fmt->item.format);
                if (err) {
                    HAPAssert(err == kHAPError_InvalidData);
                    return err;
                }
                err = HAPTLVReaderDecodeAggregate(
                        &subReader, fmt->item.format, value, &stringBuilder, /* nestingLevel: */ 1);
                if (err) {
                    HAPAssert(err == kHAPError_InvalidData);
                    HAPLogTLV(&logObject, fmt->item.tlvType, fmt->item.debugDescription, "Invalid value.");
                    return err;
                }
            } else {
                err = HAPTLVReaderDecodeScalar(
                        (void*) (uintptr_t) tlv.value.bytes,
                        tlv.value.numBytes,
                        fmt->item.tlvType,
                        fmt->item.debugDescription,
                        fmt->item.format,
                        value,
                        &stringBuilder,
                        /* nestingLevel: */ 0);
                if (err) {
                    HAPAssert(err == kHAPError_InvalidData);
                    return err;
                }
            }

            if (HAPStringBuilderDidOverflow(&stringBuilder)) {
                HAPLogError(&logObject, "Logs were truncated.");
            }
            HAPLogDebug(&logObject, "Decoded sequence TLV:%s", HAPStringBuilderGetString(&stringBuilder));

            if (shouldContinue) {
                callback(context, value, &shouldContinue);
            }
        } else if (tlvType == fmt->separator.tlvType) {
            HAPTLV tlv;
            err = ReadTLV(reader_, &tlvBytes[o], numTLVBytes, HAPGetFormatProperties(fmt->separator.format), &tlv);
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
                return err;
            }
            HAPAssert(tlv.type == fmt->separator.tlvType);

            err = HAPTLVReaderDecodeScalar(
                    (void*) (uintptr_t) tlv.value.bytes,
                    tlv.value.numBytes,
                    fmt->separator.tlvType,
                    fmt->separator.debugDescription,
                    fmt->separator.format,
                    NULL,
                    &stringBuilder,
                    /* nestingLevel: */ 0);
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
                return err;
            }
        }

        o += numTLVBytes;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPError HAPTLVReaderFindAndDecodeTLV(
        HAPTLVReaderRef* reader,
        HAPTLVType tlvType,
        const char* debugDescription,
        const HAPTLVFormat* format,
        bool* found,
        HAPTLVValue* _Nullable value,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel) {
    HAPPrecondition(reader);
    HAPPrecondition(debugDescription);
    HAPPrecondition(format);
    HAPPrecondition(HAPTLVFormatIsValid(format));
    HAPPrecondition(found);
    HAPPrecondition(stringBuilder);

    HAPError err;

    void* tlvBytes;
    size_t numTLVBytes;
    err = FindTLVInfo(reader, tlvType, &tlvBytes, &numTLVBytes);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }
    if (!tlvBytes) {
        *found = false;
        return kHAPError_None;
    }
    *found = true;

    HAPTLV tlv;
    err = ReadTLV(reader, tlvBytes, numTLVBytes, HAPGetFormatProperties(format), &tlv);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }
    HAPAssert(tlv.type == tlvType);

    if (HAPTLVFormatIsAggregate(format)) {
        HAPTLVAppendToLog(tlvType, debugDescription, format, NULL, stringBuilder, nestingLevel);
        HAPTLVReaderRef subReader;
        HAPTLVReaderCreate(&subReader, (void*) (uintptr_t) tlv.value.bytes, tlv.value.numBytes);
        err = EnableNonSequentialAccessWithFormat(&subReader, format);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }
        err = HAPTLVReaderDecodeAggregate(&subReader, format, HAPNonnullVoid(value), stringBuilder, nestingLevel + 1);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            HAPLogTLV(&logObject, tlvType, debugDescription, "Invalid value.");
            return err;
        }
    } else {
        err = HAPTLVReaderDecodeScalar(
                (void*) (uintptr_t) tlv.value.bytes,
                tlv.value.numBytes,
                tlvType,
                debugDescription,
                format,
                value,
                stringBuilder,
                nestingLevel);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }
    }

    err = FindTLVInfo(reader, tlvType, &tlvBytes, &numTLVBytes);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }
    if (tlvBytes) {
        HAPLogTLV(&logObject, tlvType, debugDescription, "Duplicate TLV.");
        return kHAPError_InvalidData;
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPError HAPTLVReaderDecodeScalar(
        void* bytes,
        size_t numBytes,
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

    switch (format->type) {
        case kHAPTLVFormatType_None: {
            const HAPSeparatorTLVFormat* fmt HAP_UNUSED = format_;
            HAPPrecondition(!value_);
            if (numBytes) {
                HAPLogTLV(&logObject, tlvType, debugDescription, "Ignoring separator value.");
                HAPLogSensitiveBuffer(&logObject, bytes, numBytes, "Ignored value.");
            }
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Enum: {
            const HAPEnumTLVFormat* fmt = format_;
            uint8_t* value = HAPNonnullVoid(value_);
            *value = 0;
            if (numBytes != sizeof *value) {
                HAPLogTLV(
                        &logObject,
                        tlvType,
                        debugDescription,
                        "Invalid enumeration length (%zu bytes - expecting %zu bytes).",
                        numBytes,
                        sizeof *value);
                return kHAPError_InvalidData;
            }
            *value = HAPReadUInt8(bytes);
            HAPAssert(fmt->callbacks.isValid);
            if (!fmt->callbacks.isValid(*value)) {
                HAPLogTLV(&logObject, tlvType, debugDescription, "Invalid enumeration value: %u.", *value);
                return kHAPError_InvalidData;
            }
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
        }
            return kHAPError_None;
#define PROCESS_INTEGER_FORMAT(formatName, typeName, printfFormat, printfTypeName) \
    do { \
        const formatName* fmt = format_; \
        typeName* value = HAPNonnullVoid(value_); \
        *value = 0; \
        if (numBytes > sizeof *value) { \
            HAPLogTLV( \
                    &logObject, \
                    tlvType, \
                    debugDescription, \
                    "Invalid integer length (%zu bytes - expecting maximum %zu bytes).", \
                    numBytes, \
                    sizeof *value); \
            return kHAPError_InvalidData; \
        } \
        for (size_t i = 0; i < numBytes; i++) { \
            *value |= (typeName)(((const uint8_t*) bytes)[i] << (i * CHAR_BIT)); \
        } \
        if (*value < fmt->constraints.minimumValue || *value > fmt->constraints.maximumValue) { \
            HAPLogTLV( \
                    &logObject, \
                    tlvType, \
                    debugDescription, \
                    "Invalid integer value: %" printfFormat \
                    " " \
                    "(constraints: minimumValue = %" printfFormat " / maximumValue = %" printfFormat ").", \
                    (printfTypeName) *value, \
                    (printfTypeName) fmt->constraints.minimumValue, \
                    (printfTypeName) fmt->constraints.maximumValue); \
            return kHAPError_InvalidData; \
        } \
        HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel); \
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
            HAPDataTLVValue* value = HAPNonnullVoid(value_);
            HAPRawBufferZero(value, sizeof *value);
            if (numBytes < fmt->constraints.minLength || numBytes > fmt->constraints.maxLength) {
                HAPLogTLV(
                        &logObject,
                        tlvType,
                        debugDescription,
                        "Invalid data length: %zu (constraints: minLength = %zu / maxLength = %zu).",
                        numBytes,
                        fmt->constraints.minLength,
                        fmt->constraints.maxLength);
                return kHAPError_InvalidData;
            }
            value->bytes = bytes;
            value->numBytes = numBytes;
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_String: {
            const HAPStringTLVFormat* fmt = format_;
            char** value = HAPNonnullVoid(value_);
            *value = NULL;
            if (numBytes < fmt->constraints.minLength || numBytes > fmt->constraints.maxLength) {
                HAPLogTLV(
                        &logObject,
                        tlvType,
                        debugDescription,
                        "Invalid string length: %zu (constraints: minLength = %zu / maxLength = %zu).",
                        numBytes,
                        fmt->constraints.minLength,
                        fmt->constraints.maxLength);
                return kHAPError_InvalidData;
            }
            if (HAPStringGetNumBytes(HAPNonnullVoid(bytes)) != numBytes) {
                HAPLogTLV(&logObject, tlvType, debugDescription, "Invalid string value: Contains NULL characters.");
                return kHAPError_InvalidData;
            }
            if (!HAPUTF8IsValidData(HAPNonnullVoid(bytes), numBytes)) {
                HAPLogTLV(&logObject, tlvType, debugDescription, "Invalid string value: Not valid UTF-8.");
                return kHAPError_InvalidData;
            }
            if (fmt->callbacks.isValid && !fmt->callbacks.isValid(HAPNonnullVoid(bytes))) {
                HAPLogTLV(&logObject, tlvType, debugDescription, "Invalid string value.");
                return kHAPError_InvalidData;
            }
            *value = bytes;
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
        }
            return kHAPError_None;
        case kHAPTLVFormatType_Value: {
            const HAPValueTLVFormat* fmt = format_;
            HAPAssert(fmt->callbacks.decode);
            err = fmt->callbacks.decode(HAPNonnullVoid(value_), bytes, numBytes);
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
                HAPLogTLV(&logObject, tlvType, debugDescription, "Invalid value.");
                return err;
            }
            HAPTLVAppendToLog(tlvType, debugDescription, format_, value_, stringBuilder, nestingLevel);
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

static void SetStructMemberIsSet(const HAPStructTLVMember* member, HAPTLVValue* value, bool isSet) {
    HAPPrecondition(member);
    HAPPrecondition(member->isOptional);
    HAPPrecondition(value);

    *((bool*) &((char*) value)[member->isSetOffset]) = isSet;
}

HAP_RESULT_USE_CHECK
static HAPTLVValue* GetStructMemberValue(const HAPStructTLVMember* member, HAPTLVValue* value) {
    HAPPrecondition(member);
    HAPPrecondition(value);

    return &((char*) value)[member->valueOffset];
}

HAP_RESULT_USE_CHECK
static HAPError HAPTLVReaderDecodeAggregate(
        HAPTLVReaderRef* reader,
        const HAPTLVFormat* format_,
        HAPTLVValue* value_,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel) {
    HAPPrecondition(reader);
    HAPPrecondition(format_);
    HAPPrecondition(HAPTLVFormatIsValid(format_));
    HAPPrecondition(HAPTLVFormatIsAggregate(format_));
    const HAPBaseTLVFormat* format = format_;
    HAPPrecondition(value_);
    HAPPrecondition(stringBuilder);

    HAPError err;

    err = SkipUnexpectedValues(reader, format_);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }

    if (format->type == kHAPTLVFormatType_Sequence) {
        const HAPSequenceTLVFormat* fmt = format_;
        HAPSequenceTLVValue* value = value_;
        HAPRawBufferZero(value, sizeof *value);
        HAPSequenceTLVDataSourceRef* dataSource_ = &value->dataSource;
        HAPSequenceTLVDataSource* dataSource = (HAPSequenceTLVDataSource*) dataSource_;
        dataSource->format = format_;
        // Extend lifetime of reader by copying it.
        // NOTE: This only works if reader itself is not modified from here on.
        // Since only functions taking a const HAPTLVReaderRef are called, this is fine.
        // When a function is called through either reader, they operate on the same buffer,
        // and because we enabled non-sequential access they won't get confused.
        HAPRawBufferCopyBytes(&dataSource->reader, reader, sizeof dataSource->reader);
        value->enumerate = EnumerateSequenceTLV;
        if (!fmt->item.isFlat) {
            HAPTLVAppendToLog(fmt->item.tlvType, fmt->item.debugDescription, fmt, NULL, stringBuilder, nestingLevel);
        }
    } else if (format->type == kHAPTLVFormatType_Struct) {
        const HAPStructTLVFormat* fmt = format_;
        if (fmt->members) {
            for (size_t i = 0; fmt->members[i]; i++) {
                const HAPStructTLVMember* member = fmt->members[i];
                HAPTLVValue* memberValue = GetStructMemberValue(member, HAPNonnullVoid(value_));
                if (member->isFlat) {
                    HAPAssert(HAPTLVFormatIsAggregate(member->format));
                    HAPAssert(!member->isOptional);
                    err = HAPTLVReaderDecodeAggregate(reader, member->format, memberValue, stringBuilder, nestingLevel);
                    if (err) {
                        HAPAssert(err == kHAPError_InvalidData);
                        return err;
                    }
                } else {
                    bool found;
                    err = HAPTLVReaderFindAndDecodeTLV(
                            reader,
                            member->tlvType,
                            member->debugDescription,
                            member->format,
                            &found,
                            memberValue,
                            stringBuilder,
                            nestingLevel);
                    if (err) {
                        HAPAssert(err == kHAPError_InvalidData);
                        return err;
                    }
                    if (member->isOptional) {
                        SetStructMemberIsSet(member, HAPNonnullVoid(value_), found);
                        if (!found) {
                            continue;
                        }
                    } else if (!found) {
                        HAPLogTLV(&logObject, member->tlvType, member->debugDescription, "TLV missing.");
                        return kHAPError_InvalidData;
                    }
                }
            }
        }
        if (fmt->callbacks.isValid && !fmt->callbacks.isValid(HAPNonnullVoid(value_))) {
            return kHAPError_InvalidData;
        }
    } else {
        HAPAssert(format->type == kHAPTLVFormatType_Union);
        const HAPUnionTLVFormat* fmt = format_;
        HAPUnionTLVValue* value = value_;
        if (fmt->variants) {
            bool isValid = false;
            for (size_t i = 0; fmt->variants[i]; i++) {
                const HAPUnionTLVVariant* variant = fmt->variants[i];
                bool found;
                err = HAPTLVReaderFindAndDecodeTLV(
                        reader,
                        variant->tlvType,
                        variant->debugDescription,
                        variant->format,
                        &found,
                        &((char*) value_)[fmt->untaggedValueOffset],
                        stringBuilder,
                        nestingLevel);
                if (err) {
                    HAPAssert(err == kHAPError_InvalidData);
                    return err;
                }
                if (!found) {
                    continue;
                }
                if (isValid) {
                    HAPLogTLV(
                            &logObject,
                            variant->tlvType,
                            variant->debugDescription,
                            "TLV not allowed when [%02X] TLV is present.",
                            value->type);
                    return kHAPError_InvalidData;
                }
                value->type = variant->tlvType;
                isValid = true;
            }
            if (!isValid) {
                for (size_t i = 0; fmt->variants[i]; i++) {
                    const HAPUnionTLVVariant* variant = fmt->variants[i];
                    HAPLogTLV(&logObject, variant->tlvType, variant->debugDescription, "TLV missing.");
                }
                return kHAPError_InvalidData;
            }
        }
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPTLVReaderDecodeVoid(HAPTLVReaderRef* reader, const HAPTLVFormat* format, HAPTLVValue* value) {
    HAPPrecondition(reader);
    HAPPrecondition(format);
    HAPPrecondition(HAPTLVFormatIsValid(format));
    HAPPrecondition(HAPTLVFormatIsAggregate(format));
    HAPPrecondition(value);

    HAPError err;

    char logBytes[kHAPTLVValue_MaxLogBytes + 1];
    HAPStringBuilderRef stringBuilder;
    HAPStringBuilderCreate(&stringBuilder, logBytes, sizeof logBytes);

    err = EnableNonSequentialAccessWithFormat(reader, format);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }
    err = HAPTLVReaderDecodeAggregate(reader, format, value, &stringBuilder, /* nestingLevel: */ 0);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        HAPLog(&logObject, "Invalid value.");
        return err;
    }

    if (HAPStringBuilderDidOverflow(&stringBuilder)) {
        HAPLogError(&logObject, "Logs were truncated.");
    }
    HAPLogDebug(&logObject, "Decoded TLV:%s", HAPStringBuilderGetString(&stringBuilder));
    return kHAPError_None;
}
