// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_STRING_BUILDER_H
#define HAP_STRING_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * String builder.
 */
typedef HAP_OPAQUE(32) HAPStringBuilderRef;

/**
 * Initializes a string builder.
 *
 * @param[out] stringBuilder        String builder to initialize.
 * @param      bytes                Buffer to fill with the combined strings. Will be NULL-terminated.
 * @param      maxBytes             Capacity of buffer.
 */
void HAPStringBuilderCreate(HAPStringBuilderRef* stringBuilder, char* bytes, size_t maxBytes);

/**
 * Indicates whether the capacity of a string builder was not sufficient to hold all appended values.
 *
 * @param      stringBuilder        String builder.
 *
 * @return true                     If string builder capacity was insufficient. Combined string is truncated.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPStringBuilderDidOverflow(HAPStringBuilderRef* stringBuilder);

/**
 * Returns the combined string of a string builder.
 *
 * @param      stringBuilder        String builder.
 *
 * @return The combined string of a string builder.
 */
HAP_RESULT_USE_CHECK
const char* HAPStringBuilderGetString(HAPStringBuilderRef* stringBuilder);

/**
 * Returns the length of the current combined strings of a string builder.
 *
 * @param      stringBuilder        String builder.
 *
 * @return Total length of all appended strings.
 */
HAP_RESULT_USE_CHECK
size_t HAPStringBuilderGetNumBytes(HAPStringBuilderRef* stringBuilder);

/**
 * Appends a formatted string to the combined strings of a string builder.
 *
 * - If the string builder's buffer is not large enough the appended string is truncated.
 *
 * - The supported conversion specifiers follow the IEEE printf specification.
 *   See http://pubs.opengroup.org/onlinepubs/009695399/functions/printf.html
 *
 * @param      stringBuilder        String builder.
 * @param      format               A format string.
 * @param      ...                  Arguments for the format string.
 *
 * Currently the following options are supported:
 * flags:  0, +, ' '
 * width:  number
 * length: l, ll, z
 * types:  %, d, i, u, x, X, p, s, c
 */
HAP_PRINTFLIKE(2, 3)
void HAPStringBuilderAppend(HAPStringBuilderRef* stringBuilder, const char* format, ...);

/**
 * Appends a formatted string to the combined strings of a string builder.
 *
 * - If the string builder's buffer is not large enough the appended string is truncated.
 *
 * - The supported conversion specifiers follow the IEEE printf specification.
 *   See http://pubs.opengroup.org/onlinepubs/009695399/functions/printf.html
 *
 * @param      stringBuilder        String builder.
 * @param      format               A format string.
 * @param      arguments            Arguments for the format string.
 *
 * Currently the following options are supported:
 * flags:  0, +, ' '
 * width:  number
 * length: l, ll, z
 * types:  %, d, i, u, x, X, p, s, c
 */
HAP_PRINTFLIKE(2, 0)
void HAPStringBuilderAppendWithArguments(HAPStringBuilderRef* stringBuilder, const char* format, va_list arguments);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
