// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_JSON_UTILS_H
#define HAP_JSON_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#include "util_json_reader.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Skips over a JSON value (object, array, string, number, 'true', 'false', or 'null').
 *
 * @param      reader               Reader used to skip over a JSON value.
 * @param      bytes                Buffer to read from.
 * @param      maxBytes             Maximum number of bytes to skip over.
 * @param[out] numBytes             Number of bytes skipped.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If a JSON syntax error was encountered.
 * @return kHAPError_OutOfResources If the JSON value is nested too deeply.
 */
HAP_RESULT_USE_CHECK
HAPError HAPJSONUtilsSkipValue(struct util_json_reader* reader, const char* bytes, size_t maxBytes, size_t* numBytes);

/**
 * Determines the space needed by the string representation of a float in JSON format.
 *
 * @param      value                Numeric value.
 *
 * @return Number of bytes that the value's string representation needs (excluding NULL-terminator).
 */
HAP_RESULT_USE_CHECK
size_t HAPJSONUtilsGetFloatNumDescriptionBytes(float value);

/**
 * Gets the string representation of a float value in JSON format.
 *
 * @param      value                Numeric value.
 * @param[out] bytes                Buffer to fill with the UUID's string representation. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPJSONUtilsGetFloatDescription(float value, char* bytes, size_t maxBytes);

/**
 * Returns the number of bytes of the provided UTF-8 encoded string data after escaping according to RFC 7159, Section 7
 * "Strings" (http://www.rfc-editor.org/rfc/rfc7159.txt).
 *
 * @param      bytes                Buffer with UTF-8 encoded string data bytes.
 * @param      numBytes             Number of string data bytes to escape.
 *
 * @return Number of bytes of the provided string data after escaping.
 */
HAP_RESULT_USE_CHECK
size_t HAPJSONUtilsGetNumEscapedStringDataBytes(const char* bytes, size_t numBytes);

/**
 * Escapes UTF-8 encoded string data according to RFC 7159, Section 7 "Strings"
 * (http://www.rfc-editor.org/rfc/rfc7159.txt).
 *
 * @param[in,out] bytes             Buffer with UTF-8 encoded string data bytes.
 * @param      maxBytes             Maximum number of string data bytes that may be filled into the buffer.
 * @param[in,out] numBytes          Number of string data bytes to escape on input,
 *                                  number of escaped string data bytes on output.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the buffer is too small for the escaped string data bytes.
 */
HAP_RESULT_USE_CHECK
HAPError HAPJSONUtilsEscapeStringData(char* bytes, size_t maxBytes, size_t* numBytes);

/**
 * Unescapes UTF-8 encoded string data according to RFC 7159, Section 7 "Strings"
 * (http://www.rfc-editor.org/rfc/rfc7159.txt).
 *
 * @param[in,out] bytes             Buffer with UTF-8 encoded string data bytes.
 * @param[in,out] numBytes          Number of string data bytes to unescape on input,
 *                                  number of unescaped string data bytes on output.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If a JSON syntax error was encountered.
 */
HAP_RESULT_USE_CHECK
HAPError HAPJSONUtilsUnescapeStringData(char* bytes, size_t* numBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
