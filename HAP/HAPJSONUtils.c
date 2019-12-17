// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "JSONUtils" };
typedef struct {
    uint64_t bits;
    size_t depth;
} Stack;

static void StackCreate(Stack* _Nonnull s) {
    HAPPrecondition(s);
    s->bits = 0;
    s->depth = 0;
}

static bool StackIsEmpty(Stack* _Nonnull s) {
    HAPPrecondition(s);
    return s->depth == 0;
}

static bool StackIsFull(Stack* _Nonnull s) {
    HAPPrecondition(s);
    return s->depth == 64;
}

static bool StackTop(Stack* _Nonnull s) {
    HAPPrecondition(s);
    HAPPrecondition(s->depth > 0);
    return (s->bits & 1) == 1;
}

static void StackPush(Stack* _Nonnull s, bool bit) {
    HAPPrecondition(s);
    HAPPrecondition(s->depth < 64);
    s->bits = (s->bits << 1) | (bit ? 1 : 0);
    s->depth++;
}

static void StackPop(Stack* _Nonnull s) {
    HAPPrecondition(s);
    HAPPrecondition(s->depth > 0);
    s->bits >>= 1;
    s->depth--;
}

#define SKIPPING_OBJECT_MEMBER_VALUE (false)
#define SKIPPING_ARRAY_VALUE         (true)

HAP_RESULT_USE_CHECK
HAPError HAPJSONUtilsSkipValue(struct util_json_reader* reader, const char* bytes, size_t maxBytes, size_t* numBytes) {
    HAPPrecondition(reader != NULL);
    HAPPrecondition(bytes != NULL);
    HAPPrecondition(numBytes != NULL);

    Stack stack;
    StackCreate(&stack);

    bool skipped_value = false;

    *numBytes = util_json_reader_read(reader, &bytes[0], maxBytes);
    do {
        HAPAssert(!skipped_value);
        switch (reader->state) {
            case util_JSON_READER_STATE_BEGINNING_OBJECT: {
                HAPAssert(*numBytes <= maxBytes);
                *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                if (reader->state != util_JSON_READER_STATE_COMPLETED_OBJECT) {
                    if (reader->state != util_JSON_READER_STATE_BEGINNING_STRING) {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert(*numBytes <= maxBytes);
                    *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                    if (reader->state != util_JSON_READER_STATE_COMPLETED_STRING) {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert(*numBytes <= maxBytes);
                    *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                    if (reader->state != util_JSON_READER_STATE_AFTER_NAME_SEPARATOR) {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert(*numBytes <= maxBytes);
                    *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                    if (StackIsFull(&stack)) {
                        return kHAPError_OutOfResources;
                    }
                    StackPush(&stack, SKIPPING_OBJECT_MEMBER_VALUE);
                } else {
                    skipped_value = true;
                }
            } break;
            case util_JSON_READER_STATE_BEGINNING_ARRAY: {
                HAPAssert(*numBytes <= maxBytes);
                *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                if (reader->state != util_JSON_READER_STATE_COMPLETED_ARRAY) {
                    if (StackIsFull(&stack)) {
                        return kHAPError_OutOfResources;
                    }
                    StackPush(&stack, SKIPPING_ARRAY_VALUE);
                } else {
                    skipped_value = true;
                }
            } break;
            case util_JSON_READER_STATE_BEGINNING_NUMBER: {
                HAPAssert(*numBytes <= maxBytes);
                *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                if (reader->state != util_JSON_READER_STATE_COMPLETED_NUMBER) {
                    return kHAPError_InvalidData;
                }
                skipped_value = true;
            } break;
            case util_JSON_READER_STATE_BEGINNING_STRING: {
                HAPAssert(*numBytes <= maxBytes);
                *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                if (reader->state != util_JSON_READER_STATE_COMPLETED_STRING) {
                    return kHAPError_InvalidData;
                }
                skipped_value = true;
            } break;
            case util_JSON_READER_STATE_BEGINNING_FALSE: {
                HAPAssert(*numBytes <= maxBytes);
                *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                if (reader->state != util_JSON_READER_STATE_COMPLETED_FALSE) {
                    return kHAPError_InvalidData;
                }
                skipped_value = true;
            } break;
            case util_JSON_READER_STATE_BEGINNING_TRUE: {
                HAPAssert(*numBytes <= maxBytes);
                *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                if (reader->state != util_JSON_READER_STATE_COMPLETED_TRUE) {
                    return kHAPError_InvalidData;
                }
                skipped_value = true;
            } break;
            case util_JSON_READER_STATE_BEGINNING_NULL: {
                HAPAssert(*numBytes <= maxBytes);
                *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                if (reader->state != util_JSON_READER_STATE_COMPLETED_NULL) {
                    return kHAPError_InvalidData;
                }
                skipped_value = true;
            } break;
            default: {
                return kHAPError_InvalidData;
            }
        }
        while (!StackIsEmpty(&stack) && skipped_value) {
            skipped_value = false;
            HAPAssert(*numBytes <= maxBytes);
            *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
            if (StackTop(&stack) == SKIPPING_OBJECT_MEMBER_VALUE) {
                if (reader->state != util_JSON_READER_STATE_COMPLETED_OBJECT) {
                    if (reader->state != util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR) {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert(*numBytes <= maxBytes);
                    *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                    if (reader->state != util_JSON_READER_STATE_BEGINNING_STRING) {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert(*numBytes <= maxBytes);
                    *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                    if (reader->state != util_JSON_READER_STATE_COMPLETED_STRING) {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert(*numBytes <= maxBytes);
                    *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                    if (reader->state != util_JSON_READER_STATE_AFTER_NAME_SEPARATOR) {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert(*numBytes <= maxBytes);
                    *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                } else {
                    StackPop(&stack);
                    skipped_value = true;
                }
            } else {
                HAPAssert(StackTop(&stack) == SKIPPING_ARRAY_VALUE);
                if (reader->state != util_JSON_READER_STATE_COMPLETED_ARRAY) {
                    if (reader->state != util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR) {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert(*numBytes <= maxBytes);
                    *numBytes += util_json_reader_read(reader, &bytes[*numBytes], maxBytes - *numBytes);
                } else {
                    StackPop(&stack);
                    skipped_value = true;
                }
            }
        }
    } while (!StackIsEmpty(&stack));
    HAPAssert(skipped_value);
    HAPAssert(*numBytes <= maxBytes);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
size_t HAPJSONUtilsGetFloatNumDescriptionBytes(float value) {
    HAPError err;

    if (HAPFloatIsFinite(value)) {
        char description[kHAPFloat_MaxDescriptionBytes + 1];
        err = HAPFloatGetDescription(description, sizeof description, value);
        HAPAssert(!err);
        return HAPStringGetNumBytes(description);
    } else {
        return sizeof "null" - 1;
    }
}

HAP_RESULT_USE_CHECK
HAPError HAPJSONUtilsGetFloatDescription(float value, char* bytes, size_t maxBytes) {
    HAPPrecondition(bytes);

    HAPError err;

    if (HAPFloatIsFinite(value)) {
        err = HAPFloatGetDescription(bytes, maxBytes, value);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            return err;
        }
    } else {
        if (maxBytes < sizeof "null") {
            HAPLog(&logObject, "Buffer not large enough to hold non-finite float value.");
            return kHAPError_OutOfResources;
        }

        bytes[0] = 'n';
        bytes[1] = 'u';
        bytes[2] = 'l';
        bytes[3] = 'l';
        bytes[4] = '\0';
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
size_t HAPJSONUtilsGetNumEscapedStringDataBytes(const char* bytes, size_t numBytes) {
    HAPPrecondition(bytes);
    HAPPrecondition(HAPUTF8IsValidData(bytes, numBytes));

    // See RFC 7159, Section 7 "Strings" (http://www.rfc-editor.org/rfc/rfc7159.txt)

    size_t numEscapedBytes = 0;

    for (size_t i = 0; i < numBytes; i++) {
        int x = bytes[i];
        if ((x == '"') || (x == '\\')) {
            HAPAssert(SIZE_MAX - numEscapedBytes >= 2);
            numEscapedBytes += 2;
        } else if ((0 <= x) && (x <= 0x1f)) {
            // control characters
            if ((x == '\b') || (x == '\f') || (x == '\n') || (x == '\r') || (x == '\t')) {
                HAPAssert(SIZE_MAX - numEscapedBytes >= 2);
                numEscapedBytes += 2;
            } else {
                HAPAssert(SIZE_MAX - numEscapedBytes >= 6);
                numEscapedBytes += 6;
            }
        } else {
            // unescaped
            HAPAssert(SIZE_MAX - numEscapedBytes >= 1);
            numEscapedBytes += 1;
        }
    }

    return numEscapedBytes;
}

HAP_RESULT_USE_CHECK
HAPError HAPJSONUtilsEscapeStringData(char* bytes, size_t maxBytes, size_t* numBytes) {
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);
    HAPPrecondition(*numBytes <= maxBytes);
    HAPPrecondition(HAPUTF8IsValidData(bytes, *numBytes));

    HAPError err;

    // See RFC 7159, Section 7 "Strings" (http://www.rfc-editor.org/rfc/rfc7159.txt)

    if (*numBytes > 0) {
        size_t i = 0;
        size_t j = maxBytes - *numBytes;

        HAPRawBufferCopyBytes(&bytes[j], &bytes[i], *numBytes);

        while (j < maxBytes) {
            int x = bytes[j];
            if ((x == '"') || (x == '\\')) {
                if (j - i < 1) {
                    return kHAPError_OutOfResources;
                }
                bytes[i] = '\\';
                i++;
                bytes[i] = (char) x;
                i++;
            } else if ((0 <= x) && (x <= 0x1f)) {
                // control characters
                if (j - i < 1) {
                    return kHAPError_OutOfResources;
                }
                bytes[i] = '\\';
                i++;
                if (x == '\b') {
                    bytes[i] = 'b';
                    i++;
                } else if (x == '\f') {
                    bytes[i] = 'f';
                    i++;
                } else if (x == '\n') {
                    bytes[i] = 'n';
                    i++;
                } else if (x == '\r') {
                    bytes[i] = 'r';
                    i++;
                } else if (x == '\t') {
                    bytes[i] = 't';
                    i++;
                } else {
                    if (j - i < 4) {
                        return kHAPError_OutOfResources;
                    }
                    char hex[5];
                    err = HAPStringWithFormat(hex, sizeof hex, "%04x", x);
                    HAPAssert(!err);
                    bytes[i] = 'u';
                    i++;
                    bytes[i] = hex[0];
                    i++;
                    bytes[i] = hex[1];
                    i++;
                    bytes[i] = hex[2];
                    i++;
                    bytes[i] = hex[3];
                    i++;
                }
            } else {
                // unescaped
                bytes[i] = (char) x;
                i++;
            }
            j++;
            HAPAssert(i <= j);
        }
        HAPAssert(j == maxBytes);

        *numBytes = i;
    }

    return kHAPError_None;
}

/**
 * Determines whether the supplied integer value is a Unicode code point according to
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - D10, page 67.
 *
 * @param      value                Integer value.
 *
 * @return true                     If the supplied integer value is a Unicode code point.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool UnicodeIsCodePoint(uint32_t value) {
    // See http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - D10, page 88.

    return value <= 0x10ffff;
}

/**
 * Determines whether the supplied integer value is a Unicode high-surrogate code point according to
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - D71, page 88.
 *
 * @param      value                Integer value.
 *
 * @return true                     If the supplied integer value is a Unicode high-surrogate code point.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool UnicodeIsHighSurrogateCodePoint(uint32_t value) {
    // See http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - D71, page 88.

    return (0xd800 <= value) && (value <= 0xdbff);
}

/**
 * Determines whether the supplied integer value is a Unicode low-surrogate code point according to
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - D73, page 88.
 *
 * @param      value                Integer value.
 *
 * @return true                     If the supplied integer value is a Unicode low-surrogate code point.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool UnicodeIsLowSurrogateCodePoint(uint32_t value) {
    // See http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - D73, page 88.

    return (0xdc00 <= value) && (value <= 0xdfff);
}

/**
 * Determines whether the supplied integer value is a Unicode scalar value according to
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - D76, page 88.
 *
 * @param      value                Integer value.
 *
 * @return true                     If the supplied integer value is a Unicode scalar value.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
static bool UnicodeIsScalarValue(uint32_t value) {
    // See http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - D76, page 88.

    return (value <= 0xd7ff) || ((0xe000 <= value) && (value <= 0x10ffff));
}

/**
 * Calculates the Unicode scalar value from a surrogate pair according to
 * http://unicode.org/versions/Unicode3.0.0/ch03.pdf - D28, page 45.
 *
 * @param      highSurrogate        Unicode high surrogate code point.
 * @param      lowSurrogate         Unicode low surrogate code point.
 *
 * @return Unicode scalar value calculated from the supplied surrogate pair.
 */
HAP_RESULT_USE_CHECK
static uint32_t UnicodeGetScalarValueFromSurrogatePair(uint32_t highSurrogate, uint32_t lowSurrogate) {
    HAPPrecondition(UnicodeIsHighSurrogateCodePoint(highSurrogate));
    HAPPrecondition(UnicodeIsLowSurrogateCodePoint(lowSurrogate));

    // See http://unicode.org/versions/Unicode3.0.0/ch03.pdf - D28, page 45.
    //
    // Example: G clef character (U+1D11E) may be represented in JSON as "\ud834\udd1e".
    //          With highSurrogate == 0xd834 and lowSurrogate == 0xdd1e we get
    //          ((0xd834 - 0xd800) * 0x400) + (0xdd1e - 0xdc00) + 0x10000 == 0x1d11e.

    uint32_t unicodeScalar = ((highSurrogate - 0xd800) * 0x400) + (lowSurrogate - 0xdc00) + 0x10000;

    HAPAssert((0x10000 <= unicodeScalar) && (unicodeScalar <= 0x10ffff));

    return unicodeScalar;
}

/**
 * Encodes a Unicode scalar value into a UTF-8 byte sequence of one to four bytes in length according to
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - Table 3-6, page 94.
 *
 * @param      unicodeScalar        Unicode scalar value to encode.
 * @param[out] bytes                Buffer to fill with the UTF-8 byte sequence.
 * @param      maxBytes             Capacity of buffer.
 * @param[out] numBytes             Length of the UTF-8 byte sequence.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_RESULT_USE_CHECK
static HAPError UTF8EncodeCodePoint(uint32_t unicodeScalar, char* bytes, size_t maxBytes, size_t* numBytes) {
    HAPPrecondition(UnicodeIsScalarValue(unicodeScalar));
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    // See http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - Table 3-6, page 94.

    if (unicodeScalar <= 0x7f) {
        if (maxBytes < 1) {
            return kHAPError_OutOfResources;
        }
        // 00000000 0xxxxxxx
        bytes[0] = (char) unicodeScalar; // 0xxxxxxx
        *numBytes = 1;
    } else if (unicodeScalar <= 0x7ff) {
        if (maxBytes < 2) {
            return kHAPError_OutOfResources;
        }
        // 00000yyy yyxxxxxx
        bytes[0] = (char) (0xc0 | (unicodeScalar >> 6));   // 110yyyyy
        bytes[1] = (char) (0x80 | (unicodeScalar & 0x3f)); // 10xxxxxx
        *numBytes = 2;
    } else if (unicodeScalar <= 0xffff) {
        if (maxBytes < 3) {
            return kHAPError_OutOfResources;
        }
        // zzzzyyyy yyxxxxxx
        bytes[0] = (char) (0xe0 | (unicodeScalar >> 12));         // 1110zzzz
        bytes[1] = (char) (0x80 | ((unicodeScalar >> 6) & 0x3f)); // 10yyyyyy
        bytes[2] = (char) (0x80 | (unicodeScalar & 0x3f));        // 10xxxxxx
        *numBytes = 3;
    } else {
        HAPAssert(unicodeScalar <= 0x10ffff);
        if (maxBytes < 4) {
            return kHAPError_OutOfResources;
        }
        // 000uuuuu zzzzyyyy yyxxxxxx
        bytes[0] = (char) (0xf0 | (unicodeScalar >> 18));          // 11110uuu
        bytes[1] = (char) (0x80 | ((unicodeScalar >> 12) & 0x3f)); // 10uuzzzz
        bytes[2] = (char) (0x80 | ((unicodeScalar >> 6) & 0x3f));  // 10yyyyyy
        bytes[3] = (char) (0x80 | (unicodeScalar & 0x3f));         // 10xxxxxx
        *numBytes = 4;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPJSONUtilsUnescapeStringData(char* bytes, size_t* numBytes) {
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);
    HAPPrecondition(HAPUTF8IsValidData(bytes, *numBytes));

    HAPError err;

    // See RFC 7159, Section 7 "Strings" (http://www.rfc-editor.org/rfc/rfc7159.txt)

    size_t i = 0;
    size_t j = 0;

    while (j < *numBytes) {
        int x = bytes[j];
        if (x != '\\') {
            j++;
            bytes[i] = (char) x;
            i++;
        } else {
            j++;
            if (*numBytes - j < 1) {
                return kHAPError_InvalidData;
            }
            x = bytes[j];
            if ((x == '"') || (x == '\\') || (x == '/')) {
                j++;
                bytes[i] = (char) x;
                i++;
            } else if (x == 'b') {
                j++;
                bytes[i] = '\b';
                i++;
            } else if (x == 'f') {
                j++;
                bytes[i] = '\f';
                i++;
            } else if (x == 'n') {
                j++;
                bytes[i] = '\n';
                i++;
            } else if (x == 'r') {
                j++;
                bytes[i] = '\r';
                i++;
            } else if (x == 't') {
                j++;
                bytes[i] = '\t';
                i++;
            } else if (x == 'u') {
                j++;
                if (*numBytes - j < 4) {
                    return kHAPError_InvalidData;
                }
                uint32_t codePoint = 0;
                size_t n = 0;
                do {
                    x = bytes[j];
                    if (('0' <= x) && (x <= '9')) {
                        j++;
                        x = x - '0';
                    } else if (('A' <= x) && (x <= 'F')) {
                        j++;
                        x = x - 'A' + 10;
                    } else if (('a' <= x) && (x <= 'f')) {
                        j++;
                        x = x - 'a' + 10;
                    } else {
                        return kHAPError_InvalidData;
                    }
                    HAPAssert((0 <= x) && (x <= 0xf));
                    codePoint = (codePoint << 4) | ((uint32_t) x);
                    n++;
                } while (n < 4);
                HAPAssert(UnicodeIsCodePoint(codePoint));
                if (UnicodeIsLowSurrogateCodePoint(codePoint)) {
                    return kHAPError_InvalidData;
                } else if (UnicodeIsHighSurrogateCodePoint(codePoint)) {
                    // surrogate pair
                    uint32_t highSurrogate = codePoint;
                    if (*numBytes - j < 6) {
                        return kHAPError_InvalidData;
                    }
                    x = bytes[j];
                    if (x != '\\') {
                        return kHAPError_InvalidData;
                    }
                    j++;
                    x = bytes[j];
                    if (x != 'u') {
                        return kHAPError_InvalidData;
                    }
                    j++;
                    codePoint = 0;
                    n = 0;
                    do {
                        x = bytes[j];
                        if (('0' <= x) && (x <= '9')) {
                            j++;
                            x = x - '0';
                        } else if (('A' <= x) && (x <= 'F')) {
                            j++;
                            x = x - 'A' + 10;
                        } else if (('a' <= x) && (x <= 'f')) {
                            j++;
                            x = x - 'a' + 10;
                        } else {
                            return kHAPError_InvalidData;
                        }
                        HAPAssert((0 <= x) && (x <= 0xf));
                        codePoint = (codePoint << 4) | ((uint32_t) x);
                        n++;
                    } while (n < 4);
                    HAPAssert(UnicodeIsCodePoint(codePoint));
                    if (!UnicodeIsLowSurrogateCodePoint(codePoint)) {
                        return kHAPError_InvalidData;
                    }
                    uint32_t lowSurrogate = codePoint;
                    codePoint = UnicodeGetScalarValueFromSurrogatePair(highSurrogate, lowSurrogate);
                } else {
                    HAPAssert(UnicodeIsScalarValue(codePoint));
                }
                size_t numUTF8Bytes = 0;
                err = UTF8EncodeCodePoint(codePoint, &bytes[i], j - i, &numUTF8Bytes);
                HAPAssert(!err);
                i += numUTF8Bytes;
            } else {
                return kHAPError_InvalidData;
            }
        }
        HAPAssert(i <= j);
    }
    HAPAssert(j == *numBytes);

    *numBytes = i;
    return kHAPError_None;
}
