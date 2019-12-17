// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_TLV_INTERNAL_H
#define HAP_TLV_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * TLV reader.
 */
typedef struct {
    /**@cond */
    void* _Nullable bytes; /**< Buffer containing TLV data. Modified while reading. */
    size_t numBytes;       /**< Length of data in buffer. */
    size_t maxBytes;       /**< Capacity of buffer. */

    /**
     * Non-sequential access to TLVs requires reservation of a few TLV types to keep track of state.
     * Those TLV types should be picked carefully to not conflict with TLV types that are of interest.
     * TLVs using those types will be skipped and ignored. They will not be available for reading.
     *
     * When non-sequential access is turned on, TLV values are no longer implicitly NULL terminated
     * unless opted in explicitly when fetching each TLV. NULL terminated TLVs are not allowed
     * to contain NULL values themselves.
     *
     * Even with non-sequential access turned on each TLV may only be read once.
     * However, there are no restrictions on the number of times un-read TLVs may be enumerated.
     *
     * Implementation details:
     * - Data is processed in-place. No allocations take place. This is especially important
     *   w.r.t. sub-TLV structures where no free space is available before or after the TLV.
     * - When a TLV is read, potential fragmented data is merged and the TLV value is relocated
     *   according to the provided NULL terminator choice. The original TLV type is overwritten
     *   to mark the TLV as read, and to keep track of the format in which the TLV value is encoded.
     * - The different TLV value encodings are as follows: (<field : numBytes>, NULL are 0-bytes)
     *   1. Single-fragment value that may contain NULL values, but is not NULL terminated.
     *      - <type : 1>
     *      - <numValueBytes : 1>
     *      - <value : N>
     *   2. Single-fragment value that may not contain NULL values, but is NULL terminated.
     *      - <type : 1>
     *      - <value : N>
     *      - <NULL : 1>
     *   3. Multi-fragment value that may contain NULL values, and also is NULL terminated.
     *      - <type : 1>
     *      - <numFragments - 2 : X>
     *      - <numLastFragmentBytes : 1>
     *      - <NULL : 2 * (numFragments - 2) - (X - 1)>
     *      - <value : N>
     *      - <NULL : 1>
     *      The second field <numFragments - 2 : X> is a variable-length integer format
     *      where all byte values are summed up and including the first non-255 byte.
     *      X denotes the number of bytes used for this representation.
     *      For example, 1000 would be encoded as <255> <255> <255> <235> and X is 4,
     *      or 510 would be encoded as <255> <255> <0> and X is 3.
     * - When non-sequential access is turned on all TLV items are enumerated,
     *   and if the data contains TLVs with a TLV type that has a special meaning,
     *   those TLVs are implicitly read without NULL terminators and discarded.
     *   This ensures that after this initial enumeration it can be relied on that TLVs
     *   with types that have a special meaning always denote data that has already been processed.
     * - When a TLV with a type that has a special meaning is found after the initial enumeration
     *   the TLV is skipped as it has already been processed without modifying its value.
     *   This means that once a TLV has been read the client can assume that its value is stable
     *   and won't be moved to other memory locations even as other TLVs are accessed.
     */
    /**@{*/
    struct {
        HAPTLVType singleFragment;
        HAPTLVType nullTerminatedSingleFragment;
        HAPTLVType nullTerminatedMultiFragment;
    } tlvTypes;
    bool isNonSequentialAccessEnabled : 1;
    /**@}*/
    /**@endcond */
} HAPTLVReader;
HAP_STATIC_ASSERT(sizeof(HAPTLVReaderRef) >= sizeof(HAPTLVReader), HAPTLVReader);

/**
 * TLV reader initialization parameters.
 */
typedef struct {
    /**
     * Buffer that contains raw TLV data.
     *
     * @remark The buffer content will be modified by the reader!
     */
    void* _Nullable bytes;

    /**
     * Length of data in buffer.
     */
    size_t numBytes;

    /**
     * Capacity of buffer.
     */
    size_t maxBytes;
} HAPTLVReaderOptions;

/**
 * Initializes a TLV reader.
 *
 * @param[out] reader               Reader to initialize.
 * @param      options              Initialization parameters.
 */
void HAPTLVReaderCreateWithOptions(HAPTLVReaderRef* reader, const HAPTLVReaderOptions* options);

/**
 * Retrieves a temporary buffer of unused memory.
 *
 * @param      reader               Reader to retrieve buffer from.
 * @param[out] scratchBytes         Temporary buffer of free memory.
 * @param[out] numScratchBytes      Capacity of scratch buffer.
 */
void HAPTLVReaderGetScratchBytes(
        const HAPTLVReaderRef* reader,
        void* _Nonnull* _Nonnull scratchBytes,
        size_t* numScratchBytes);

/**
 * TLV writer.
 */
typedef struct {
    void* bytes;         /**< Buffer containing serialized TLV data. */
    size_t maxBytes;     /**< Capacity of buffer. */
    size_t numBytes;     /**< Length of serialized TLV data. */
    HAPTLVType lastType; /**< Type of previous TLV item. */
} HAPTLVWriter;
HAP_STATIC_ASSERT(sizeof(HAPTLVWriterRef) >= sizeof(HAPTLVWriter), HAPTLVWriter);

/**
 * Allocates bytes of memory inside a scratch buffer.
 *
 * - Memory is allocated with 4 byte alignment.
 *
 * @param[in,out] scratchBytes      Scratch buffer to allocate block in.
 * @param[in,out] maxScratchBytes   Capacity of scratch buffer.
 * @param      numBytes             Number of bytes to allocate.
 *
 * @return Start of allocated memory block, if successful. NULL otherwise.
 */
void* _Nullable HAPTLVScratchBufferAlloc(
        void* _Nonnull* _Nonnull scratchBytes,
        size_t* maxScratchBytes,
        size_t numBytes);

/**
 * Allocates bytes of memory inside a scratch buffer.
 *
 * - Memory is allocated with no alignment.
 *
 * @param[in,out] scratchBytes      Scratch buffer to allocate block in.
 * @param[in,out] numScratchBytes   Capacity of scratch buffer.
 * @param      numBytes             Number of bytes to allocate.
 *
 * @return Start of allocated memory block, if successful. NULL otherwise.
 */
void* _Nullable HAPTLVScratchBufferAllocUnaligned(
        void* _Nonnull* _Nonnull scratchBytes,
        size_t* numScratchBytes,
        size_t numBytes);

//----------------------------------------------------------------------------------------------------------------------

typedef void HAPTLVValue;
typedef void HAPTLVFormat;

/**
 * Decodes a TLV structure that matches a given format.
 *
 * @param      reader               TLV reader.
 * @param      format               Format.
 * @param[out] value                Decoded value. Type must match format.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If decoding failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPTLVReaderDecodeVoid(HAPTLVReaderRef* reader, const HAPTLVFormat* format, HAPTLVValue* value);

/**
 * Encodes a TLV structure based on a given format.
 *
 * @param      writer               TLV writer.
 * @param      format               Format.
 * @param      value                Value to encode. Type must match format.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an unknown error occurred while serializing.
 * @return kHAPError_InvalidState   If serialization is not possible in the current state.
 * @return kHAPError_OutOfResources If out of resources while serializing.
 * @return kHAPError_Busy           If serialization is temporarily not possible.
 */
HAP_RESULT_USE_CHECK
HAPError HAPTLVWriterEncodeVoid(HAPTLVWriterRef* writer, const HAPTLVFormat* format, HAPTLVValue* value);

#if __has_attribute(overloadable)
/**
 * Generates support functions to decode and encode a TLV structure.
 *
 * @param      typeName             Value type for which to generate support functions.
 * @param      formatName           TLV format corresponding to the value type.
 */
#define HAP_TLV_CODING_SUPPORT(typeName, formatName) \
    HAP_DIAGNOSTIC_PUSH \
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wunused-function") \
    __attribute__((always_inline)) __attribute__((overloadable)) HAP_RESULT_USE_CHECK static HAPError \
            HAPTLVReaderDecode( \
                    HAPTLVReaderRef* reader, \
                    const formatName* format, \
                    typeName* value) /* NOLINT(bugprone-macro-parentheses) */ \
    { \
        return HAPTLVReaderDecodeVoid(reader, format, value); \
    } \
\
    __attribute__((always_inline)) __attribute__((overloadable)) HAP_RESULT_USE_CHECK static HAPError \
            HAPTLVWriterEncode( \
                    HAPTLVWriterRef* writer, \
                    const formatName* format, \
                    typeName* value) /* NOLINT(bugprone-macro-parentheses) */ \
    { \
        return HAPTLVWriterEncodeVoid(writer, format, value); \
    } \
    HAP_DIAGNOSTIC_POP
#else
/**
 * Generates support functions to decode and encode a TLV structure.
 *
 * @param      typeName             Value type for which to generate support functions.
 * @param      formatName           TLV format corresponding to the value type.
 */
#define HAP_TLV_CODING_SUPPORT(typeName, formatName)

#define HAPTLVReaderDecode HAPTLVReaderDecodeVoid
#define HAPTLVWriterEncode HAPTLVWriterEncodeVoid
#endif

//----------------------------------------------------------------------------------------------------------------------

/**
 * TLV format type.
 */
HAP_ENUM_BEGIN(uint8_t, HAPTLVFormatType) {
    kHAPTLVFormatType_None = 1, /**< Separator between TLV items with same type. */
    kHAPTLVFormatType_Enum,     /**< Enumeration. */
    kHAPTLVFormatType_UInt8,    /**< UInt8. */
    kHAPTLVFormatType_UInt16,   /**< Little-endian UInt16. */
    kHAPTLVFormatType_UInt32,   /**< Little-endian UInt32. */
    kHAPTLVFormatType_UInt64,   /**< Little-endian UInt64. */
    kHAPTLVFormatType_Int8,     /**< Int8. */
    kHAPTLVFormatType_Int16,    /**< Little-endian Int16. */
    kHAPTLVFormatType_Int32,    /**< Little-endian Int32. */
    kHAPTLVFormatType_Int64,    /**< Little-endian Int64. */
    kHAPTLVFormatType_Data,     /**< Data buffer. */
    kHAPTLVFormatType_String,   /**< UTF-8 string. NULL-terminated. */
    kHAPTLVFormatType_Value,    /**< Value. */
    kHAPTLVFormatType_Sequence, /**< Sequence. */
    kHAPTLVFormatType_Struct,   /**< Struct. */
    kHAPTLVFormatType_Union     /**< Union. */
} HAP_ENUM_END(uint8_t, HAPTLVFormatType);

/**
 * Base TLV format.
 *
 * - Specifies common elements that are shared by all TLV format structures.
 */
typedef struct {
    HAPTLVFormatType type; /**< Type. */
} HAPBaseTLVFormat;
#define HAP_CHECK_HAPTLVFormat(tlvFormat) \
    HAP_STATIC_ASSERT(HAP_OFFSETOF(tlvFormat, type) == HAP_OFFSETOF(HAPBaseTLVFormat, type), tlvFormat##_type);

//----------------------------------------------------------------------------------------------------------------------

/**
 * Separator TLV format.
 */
typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_None. */
} HAPSeparatorTLVFormat;
HAP_CHECK_HAPTLVFormat(HAPSeparatorTLVFormat)

        /**
         * Enumeration TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Enum. */

    /** Callbacks. */
    struct {
        /**
         * The callback used to validate the TLV value.
         *
         * @param      value                Value.
         *
         * @return true                     If the provided value is valid.
         * @return false                    Otherwise.
         */
        HAP_RESULT_USE_CHECK
        bool (*isValid)(uint8_t value);

        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nonnull (*_Nonnull getDescription)(uint8_t value);
    } callbacks;
} HAPEnumTLVFormat;
HAP_CHECK_HAPTLVFormat(HAPEnumTLVFormat)

/**
 * Defines an enumeration TLV format.
 *
 * @param      typeName             Value type of this TLV.
 * @param      formatName           Name of the synthesized TLV format type.
 */
#define HAP_ENUM_TLV_SUPPORT(typeName, formatName) \
    HAP_STATIC_ASSERT(sizeof(typeName) == sizeof(uint8_t), formatName##_value); \
    typedef struct { \
        HAPTLVFormatType type; \
\
        struct { \
            HAP_RESULT_USE_CHECK \
            bool (*isValid)(uint8_t value); \
\
            HAP_RESULT_USE_CHECK \
            const char* _Nonnull (*_Nonnull getDescription)(typeName value); \
        } callbacks; \
    } formatName; /* NOLINT(bugprone-macro-parentheses) */ \
    HAP_CHECK_HAPTLVFormat(formatName)

        /**
         * UInt8 TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_UInt8. */

    /** Value constraints. */
    struct {
        uint8_t minimumValue; /**< Minimum value. */
        uint8_t maximumValue; /**< Maximum value. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description, if the value has one. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getDescription)(uint8_t value);

        /**
         * The callback used to get the description of a single option of an option set.
         *
         * - This callback is optional and may be set for option sets.
         *
         * @param      optionValue          Option value.
         *
         * @return Description, if the option is known. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getBitDescription)(uint8_t optionValue);
    } callbacks;
} HAPUInt8TLVFormat;
HAP_CHECK_HAPTLVFormat(HAPUInt8TLVFormat)

        /**
         * UInt16 TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_UInt16. */

    /** Value constraints. */
    struct {
        uint16_t minimumValue; /**< Minimum value. */
        uint16_t maximumValue; /**< Maximum value. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description, if the value has one. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getDescription)(uint16_t value);

        /**
         * The callback used to get the description of a single option of an option set.
         *
         * - This callback is optional and may be set for option sets.
         *
         * @param      optionValue          Option value (1 << bit index).
         *
         * @return Description, if the option is known. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getBitDescription)(uint16_t optionValue);
    } callbacks;
} HAPUInt16TLVFormat;
HAP_CHECK_HAPTLVFormat(HAPUInt16TLVFormat)

        /**
         * UInt32 TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_UInt32. */

    /** Value constraints. */
    struct {
        uint32_t minimumValue; /**< Minimum value. */
        uint32_t maximumValue; /**< Maximum value. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description, if the value has one. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getDescription)(uint32_t value);

        /**
         * The callback used to get the description of a single option of an option set.
         *
         * - This callback is optional and may be set for option sets.
         *
         * @param      optionValue          Option value.
         *
         * @return Description, if the option is known. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getBitDescription)(uint32_t optionValue);
    } callbacks;
} HAPUInt32TLVFormat;
HAP_CHECK_HAPTLVFormat(HAPUInt32TLVFormat)

        /**
         * UInt64 TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_UInt64. */

    /** Value constraints. */
    struct {
        uint64_t minimumValue; /**< Minimum value. */
        uint64_t maximumValue; /**< Maximum value. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description, if the value has one. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getDescription)(uint64_t value);

        /**
         * The callback used to get the description of a single option of an option set.
         *
         * @param      optionValue          Option value.
         *
         * @return Description, if the option is known. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getBitDescription)(uint64_t optionValue);
    } callbacks;
} HAPUInt64TLVFormat;
HAP_CHECK_HAPTLVFormat(HAPUInt64TLVFormat)

        /**
         * Int8 TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Int8. */

    /** Value constraints. */
    struct {
        int8_t minimumValue; /**< Minimum value. */
        int8_t maximumValue; /**< Maximum value. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description, if the value has one. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getDescription)(int8_t value);
    } callbacks;
} HAPInt8TLVFormat;
HAP_CHECK_HAPTLVFormat(HAPInt8TLVFormat)

        /**
         * Int16 TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Int16. */

    /** Value constraints. */
    struct {
        int16_t minimumValue; /**< Minimum value. */
        int16_t maximumValue; /**< Maximum value. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description, if the value has one. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getDescription)(int16_t value);
    } callbacks;
} HAPInt16TLVFormat;
HAP_CHECK_HAPTLVFormat(HAPInt16TLVFormat)

        /**
         * Int32 TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Int32. */

    /** Value constraints. */
    struct {
        int32_t minimumValue; /**< Minimum value. */
        int32_t maximumValue; /**< Maximum value. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description, if the value has one. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getDescription)(int32_t value);
    } callbacks;
} HAPInt32TLVFormat;
HAP_CHECK_HAPTLVFormat(HAPInt32TLVFormat)

        /**
         * Int64 TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Int64. */

    /** Value constraints. */
    struct {
        int64_t minimumValue; /**< Minimum value. */
        int64_t maximumValue; /**< Maximum value. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         *
         * @return Value description, if the value has one. NULL otherwise.
         */
        HAP_RESULT_USE_CHECK
        const char* _Nullable (*_Nullable getDescription)(int64_t value);
    } callbacks;
} HAPInt64TLVFormat;
HAP_CHECK_HAPTLVFormat(HAPInt64TLVFormat)

        /**
         * Data TLV value.
         */
        typedef struct {
    void* bytes;     /**< Value buffer. */
    size_t numBytes; /**< Length of value buffer. */
} HAPDataTLVValue;

/**
 * Data TLV format.
 */
typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Data. */

    /** Value constraints. */
    struct {
        size_t minLength; /**< Minimum length. */
        size_t maxLength; /**< Maximum length. */
    } constraints;
} HAPDataTLVFormat;
HAP_CHECK_HAPTLVFormat(HAPDataTLVFormat)

/**
 * Defines a data TLV format.
 *
 * @param      typeName             Value type of this TLV.
 * @param      formatName           Name of the synthesized TLV format type.
 */
#define HAP_DATA_TLV_SUPPORT(typeName, formatName) \
    HAP_STATIC_ASSERT(sizeof(typeName) == sizeof(HAPDataTLVValue), formatName##_value); \
    HAP_STATIC_ASSERT(HAP_OFFSETOF(typeName, bytes) == HAP_OFFSETOF(HAPDataTLVValue, bytes), formatName##_bytes); \
    HAP_STATIC_ASSERT( \
            HAP_OFFSETOF(typeName, numBytes) == HAP_OFFSETOF(HAPDataTLVValue, numBytes), formatName##_numBytes); \
    typedef struct { \
        HAPTLVFormatType type; \
\
        struct { \
            size_t minLength; \
            size_t maxLength; \
        } constraints; \
    } formatName; /* NOLINT(bugprone-macro-parentheses) */ \
    HAP_CHECK_HAPTLVFormat(formatName)

        /**
         * String TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_String. */

    /** Value constraints. */
    struct {
        size_t minLength; /**< Minimum length. */
        size_t maxLength; /**< Maximum length. */
    } constraints;

    /** Callbacks. */
    struct {
        /**
         * The callback used to validate the TLV value.
         *
         * @param      value                Value.
         *
         * @return true                     If the provided value is valid.
         * @return false                    Otherwise.
         */
        HAP_RESULT_USE_CHECK
        bool (*_Nullable isValid)(const char* value);
    } callbacks;
} HAPStringTLVFormat;
HAP_CHECK_HAPTLVFormat(HAPStringTLVFormat)

        /**
         * Value TLV format.
         */
        typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Value. */

    /** Callbacks. */
    struct {
        /**
         * The callback used to decode a value.
         *
         * @param[out] value                Decoded value.
         * @param      bytes                Encoded value buffer. May be modified.
         * @param      numBytes             Length of encoded value buffer.
         *
         * @return kHAPError_None           If successful.
         * @return kHAPError_InvalidData    If invalid data was encountered while parsing.
         */
        HAP_RESULT_USE_CHECK
        HAPError (*decode)(HAPTLVValue* value, void* bytes, size_t numBytes);

        /**
         * The callback used to encode a value.
         *
         * @param      value                Value to encode.
         * @param[out] bytes                Encoded value buffer.
         * @param      maxBytes             Capacity of encoded value buffer.
         * @param[out] numBytes             Length of encoded value buffer.
         *
         * @return kHAPError_None           If successful.
         * @return kHAPError_Unknown        If an unknown error occurred while serializing.
         * @return kHAPError_InvalidState   If serialization is not possible in the current state.
         * @return kHAPError_OutOfResources If out of resources while serializing.
         * @return kHAPError_Busy           If serialization is temporarily not possible.
         */
        HAP_RESULT_USE_CHECK
        HAPError (*encode)(HAPTLVValue* value, void* bytes, size_t maxBytes, size_t* numBytes);

        /**
         * The callback used to get the description of a value.
         *
         * @param      value                Valid value.
         * @param[out] bytes                Buffer to fill with the value's description. Will be NULL-terminated.
         * @param      maxBytes             Capacity of buffer.
         *
         * @return kHAPError_None           If successful.
         * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
         */
        HAP_RESULT_USE_CHECK
        HAPError (*getDescription)(HAPTLVValue* value, char* bytes, size_t maxBytes);
    } callbacks;
} HAPValueTLVFormat;
HAP_CHECK_HAPTLVFormat(HAPValueTLVFormat)

/**
 * Defines a value TLV format.
 *
 * @param      typeName             Value type of this TLV.
 * @param      formatName           Name of the synthesized TLV format type.
 */
#define HAP_VALUE_TLV_SUPPORT(typeName, formatName) \
    typedef struct { \
        HAPTLVFormatType type; \
\
        struct { \
            HAP_RESULT_USE_CHECK \
            HAPError (*decode)( \
                    typeName * value, /* NOLINT(bugprone-macro-parentheses) */ \
                    void* bytes, \
                    size_t numBytes); \
\
            HAP_RESULT_USE_CHECK \
            HAPError (*encode)( \
                    typeName * value, /* NOLINT(bugprone-macro-parentheses) */ \
                    void* bytes, \
                    size_t maxBytes, \
                    size_t* numBytes); \
\
            HAP_RESULT_USE_CHECK \
            HAPError (*getDescription)( \
                    typeName * value, /* NOLINT(bugprone-macro-parentheses) */ \
                    char* bytes, \
                    size_t maxBytes); \
        } callbacks; \
    } formatName /* NOLINT(bugprone-macro-parentheses) */; \
    HAP_CHECK_HAPTLVFormat(formatName)

        /**
         * Callback that should be invoked for each sequence item.
         *
         * @param      context              Context.
         * @param      value                Value.
         * @param[in,out] shouldContinue    True if enumeration shall continue, False otherwise. Is set to true on
         * input.
         */
        typedef void (
                *HAPSequenceTLVEnumerateCallback)(void* _Nullable context, HAPTLVValue* value, bool* shouldContinue);

/**
 * Opaque data source to pass to the enumerate function of a sequence TLV value.
 */
typedef HAP_OPAQUE(40) HAPSequenceTLVDataSourceRef;

/**
 * Sequence TLV value.
 */
typedef struct {
    /**
     * Enumerates all sequence items.
     *
     * - Enumeration can only be requested once.
     *
     * - When this callback is produced by a HAPTLVReader the only valid error codes are:
     *   kHAPError_None and kHAPError_InvalidData.
     *
     * - When this callback is passed to a HAPTLVWriter the only valid error codes are:
     *   kHAPError_None, kHAPError_Unknown, kHAPError_InvalidState, kHAPError_OutOfResources, and kHAPError_Busy.
     *   Notably, kHAPError_InvalidData is not allowed.
     *
     * @param      dataSource           Data source.
     * @param      callback             Function to call on each value.
     * @param      context              Context that is passed to the callback.
     *
     * @return kHAPError_None           If successful.
     * @return kHAPError_Unknown        If an unknown error occurred while serializing.
     * @return kHAPError_InvalidState   If serialization is not possible in the current state.
     * @return kHAPError_InvalidData    If invalid data was encountered while parsing.
     * @return kHAPError_OutOfResources If out of resources while serializing.
     * @return kHAPError_Busy           If serialization is temporarily not possible.
     */
    HAP_RESULT_USE_CHECK
    HAPError (*enumerate)(
            HAPSequenceTLVDataSourceRef* dataSource,
            HAPSequenceTLVEnumerateCallback callback,
            void* _Nullable context);

    /** Data source to pass to the enumerate function. */
    HAPSequenceTLVDataSourceRef dataSource;

    /**
     * Buffer for internal use by the enumerate function.
     *
     * - Type of this member must be set to the type of the sequence item.
     * - Offset of this member must be specified as the itemValueOffset in the corresponding sequence TLV format.
     */
    char _;
} HAPSequenceTLVValue;

/**
 * Sequence TLV format.
 */
typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Sequence. */

    /** Metadata about the sequence item. */
    struct {
        size_t valueOffset; /**< Offset of the _ member. */

        HAPTLVType tlvType;           /**< The type of the TLV item. Ignored if flat. */
        const char* debugDescription; /**< Description for debugging. Ignored if flat. */

        const HAPTLVFormat* format; /**< TLV format. */

        /**
         * Whether or not this value is embedded without a container TLV.
         *
         * - Only applicable for union items.
         */
        bool isFlat : 1;
    } item;

    /**
     * Metadata about the expected separator between sequence elements.
     */
    struct {
        HAPTLVType tlvType;           /**< The type of the TLV item. */
        const char* debugDescription; /**< Description for debugging. */

        const HAPTLVFormat* format; /**< TLV format. */
    } separator;
} HAPSequenceTLVFormat;
HAP_CHECK_HAPTLVFormat(HAPSequenceTLVFormat)

/**
 * Defines a sequence TLV format.
 *
 * @param      typeName             Value type of this TLV.
 * @param      itemFormatName       Sequence item TLV format type.
 * @param      formatName           Name of the synthesized TLV format type.
 */
#define HAP_SEQUENCE_TLV_SUPPORT(typeName, itemFormatName, formatName) \
    typedef struct { \
        HAPTLVFormatType type; \
\
        struct { \
            size_t valueOffset; \
            HAPTLVType tlvType; \
            const char* debugDescription; \
            const itemFormatName* format; \
            bool isFlat : 1; \
        } item; \
\
        struct { \
            HAPTLVType tlvType; \
            const char* debugDescription; \
            const HAPTLVFormat* format; \
        } separator; \
    } formatName; /* NOLINT(bugprone-macro-parentheses) */ \
    HAP_CHECK_HAPTLVFormat(formatName) HAP_TLV_CODING_SUPPORT(typeName, formatName)

        /**
         * Struct TLV member.
         */
        typedef struct {
    size_t valueOffset; /**< Offset of the member value. */
    size_t isSetOffset; /**< Offset of a bool indicating whether an optional member value is present. */

    HAPTLVType tlvType;           /**< The type of the TLV item. Ignored if flat. */
    const char* debugDescription; /**< Description for debugging. Ignored if flat. */

    const HAPTLVFormat* format; /**< TLV format. */

    bool isOptional : 1; /**< Whether this member value is optional. Only applicable if not flat. */

    /**
     * Whether or not this value is embedded without a container TLV.
     *
     * - Only applicable for sequence, structure and union items.
     */
    bool isFlat : 1;
} HAPStructTLVMember;

/**
 * Struct TLV format.
 */
typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Struct. */

    /** List of struct members. NULL-terminated. */
    const HAPStructTLVMember* _Nullable const* _Nullable members;

    /** Callbacks. */
    struct {
        /**
         * The callback used to validate the TLV value.
         *
         * @param      value                Value.
         *
         * @return true                     If the provided value is valid.
         * @return false                    Otherwise.
         */
        HAP_RESULT_USE_CHECK
        bool (*_Nullable isValid)(HAPTLVValue* value);
    } callbacks;
} HAPStructTLVFormat;
HAP_CHECK_HAPTLVFormat(HAPStructTLVFormat)

/**
 * Defines a struct TLV format.
 *
 * @param      typeName             Value type of this TLV.
 * @param      formatName           Name of the synthesized TLV format type.
 */
#define HAP_STRUCT_TLV_SUPPORT(typeName, formatName) \
    typedef struct { \
        HAPTLVFormatType type; \
        const HAPStructTLVMember* _Nullable const* _Nullable members; \
\
        struct { \
            HAP_RESULT_USE_CHECK \
            bool (*_Nullable isValid)(typeName * value); /* NOLINT(bugprone-macro-parentheses) */ \
        } callbacks; \
    } formatName; /* NOLINT(bugprone-macro-parentheses) */ \
    HAP_CHECK_HAPTLVFormat(formatName) HAP_TLV_CODING_SUPPORT(typeName, formatName)

        /**
         * Union TLV value.
         */
        typedef struct {
    /**
     * Type of the value.
     *
     * - Type of this member must be set to the type of the enumeration indicating type of the union value.
     */
    uint8_t type;

    /**
     * Type-specific value.
     *
     * - Type of this member must be set to the type of the union value.
     * - Offset of this member must be specified as the untaggedValueOffset in the corresponding union TLV format.
     */
    union {
        char _;
    } _;
} HAPUnionTLVValue;

/**
 * Union TLV variant.
 */
typedef struct {
    HAPTLVType tlvType;           /**< The type of the TLV item. */
    const char* debugDescription; /**< Description for debugging. */

    const HAPTLVFormat* format; /**< TLV format. */
} HAPUnionTLVVariant;

/**
 * Union TLV format.
 *
 * A union is a combination of a type and a type-specific value.
 * The type should be represented as an enumeration to enable switch statements to cover it.
 * For each applicable type a HAPUnionTLVVariant has to be defined and added to the variants list.
 *
 * There are two ways how union values may be represented.
 *
 * 1. A single TLV item is used with a dynamic TLV type.
 *    The TLV type contains the enumeration value representing the union value's type.
 *    The TLV value contains the union's type-specific value.
 *
 * 2. Two TLV items are used with static TLV types.
 *    One TLV contains the union's type-specific value.
 *    The other TLV contains the enumeration value representing the union value's type.
 *    This case is not supported at this time.
 */
typedef struct {
    HAPTLVFormatType type; /**< Type. Must be kHAPTLVFormatType_Union. */

    /**
     * Offset of the _ member within the corresponding union value type.
     */
    size_t untaggedValueOffset;

    /**
     * List of union variants. A variant must be defined for each applicable union value's type. NULL-terminated.
     *
     * - Union variants cannot be flattened sequences, structures or unions.
     */
    const HAPUnionTLVVariant* _Nullable const* _Nullable variants;
} HAPUnionTLVFormat;
HAP_CHECK_HAPTLVFormat(HAPUnionTLVFormat)

/**
 * Defines a union TLV format.
 *
 * @param      typeName             Value type of this TLV.
 * @param      formatName           Name of the synthesized TLV format type.
 */
#define HAP_UNION_TLV_SUPPORT(typeName, formatName) \
    typedef struct { \
        HAPTLVFormatType type; \
        size_t untaggedValueOffset; \
        const HAPUnionTLVVariant* _Nullable const* _Nullable variants; \
    } formatName; /* NOLINT(bugprone-macro-parentheses) */ \
    HAP_CHECK_HAPTLVFormat(formatName) HAP_TLV_CODING_SUPPORT(typeName, formatName)

        //----------------------------------------------------------------------------------------------------------------------

        /**
         * Indicates whether a TLV format contains nested sub-TLVs.
         *
         * @param      format               Format.
         *
         * @return true                     If the format contains nested sub-TLVs.
         * @return false                    Otherwise.
         */
        HAP_RESULT_USE_CHECK bool HAPTLVFormatIsAggregate(const HAPTLVFormat* format);

/**
 * Indicates whether a given TLV type is in use by a TLV format.
 *
 * @param      format               Format.
 *
 * @return true                     If provided TLV type is in use.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPTLVFormatUsesType(const HAPTLVFormat* format, HAPTLVType tlvType);

/**
 * Indicates whether two TLV formats use conflicting TLV types.
 *
 * @param      format               Metadata.
 * @param      otherFormat          Other metadata.
 *
 * @return true                     If the formats use conflicting TLV types.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPTLVFormatHaveConflictingTypes(const HAPTLVFormat* format, const HAPTLVFormat* otherFormat);

/**
 * Indicates whether a TLV format is valid.
 *
 * @param      format               Format.
 *
 * @return true                     If provided value is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPTLVFormatIsValid(const HAPTLVFormat* format);

/** Maximum supported length of a TLV value's log. */
#define kHAPTLVValue_MaxLogBytes ((size_t) 1023)

/** Maximum supported length of a value's description. */
#define kHAPTLVValue_MaxDescriptionBytes ((size_t) 255)

/**
 * Logs the provided value according to its metadata.
 *
 * @param      tlvType              The type of the TLV item.
 * @param      debugDescription     Description for debugging.
 * @param      format               Format.
 * @param      value                Value. Type must match format.
 * @param      stringBuilder        String builder.
 * @param      nestingLevel         Nesting level.
 */
void HAPTLVAppendToLog(
        HAPTLVType tlvType,
        const char* debugDescription,
        const HAPTLVFormat* format,
        HAPTLVValue* _Nullable value,
        HAPStringBuilderRef* stringBuilder,
        size_t nestingLevel);

// ISO C99 requires at least one argument for the "..." in a variadic macro.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC system_header
#endif

/**
 * Logs a default-level message related to a TLV.
 *
 * @param      logObject            Log object.
 * @param      tlvType              The type of the TLV item.
 * @param      debugDescription     Description for debugging.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogTLV(logObject, tlvType, debugDescription, format, ...) \
    HAPLog(logObject, "[%02X %s] " format, tlvType, debugDescription, ##__VA_ARGS__)

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
