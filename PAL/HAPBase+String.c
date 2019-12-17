// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

HAP_PRINTFLIKE(3, 4)
HAP_RESULT_USE_CHECK
HAPError HAPStringWithFormat(char* bytes, size_t maxBytes, const char* format, ...) {
    va_list args;
    va_start(args, format);
    HAPError err = HAPStringWithFormatAndArguments(bytes, maxBytes, format, args);
    va_end(args);
    return err;
}

HAP_PRINTFLIKE(3, 0)
HAP_RESULT_USE_CHECK
HAPError HAPStringWithFormatAndArguments(char* bytes, size_t maxBytes, const char* format, va_list arguments) {
    HAPPrecondition(bytes);
    HAPPrecondition(format);

    char c = format[0];
    size_t i = 1;
    size_t n = 0;
    while (c) {
        if (c == '%') {
            // insert value
            c = format[i++];     // format defaults:
            char prefix = ' ';   // fill with space
            char sign = 0;       // do not us a positive sign
            uint32_t length = 0; // no long type
            uint32_t width = 0;  // minimum width
            // flags field
            while (c == '0' || c == '+' || c == ' ') {
                if (c == '0') {
                    prefix = '0';
                } else {
                    sign = c;
                }
                c = format[i++];
            }
            // width field
            while (c >= '0' && c <= '9') {
                width = width * 10 + (uint32_t)(c - '0');
                c = format[i++];
            }
            // length field
            if (c == 'l') {
                length = 1;
                c = format[i++];
                if (c == 'l') {
                    length = 2;
                    c = format[i++];
                }
            } else if (c == 'z') {
                length = 3;
                c = format[i++];
            }
            // type field
            char buffer[32]; // temporary string buffer
            char* string = NULL;
            size_t strLen = 0;
            int64_t sValue = 0;
            uint64_t uValue = 0;
            switch (c) {
                case '%': {
                    string = "%";
                } break;
                case 'd':
                case 'i': {
                    if (length == 0) {
                        sValue = (int64_t) va_arg(arguments, int);
                    } else if (length == 1) { // 'l'
                        sValue = (int64_t) va_arg(arguments, long);
                    } else if (length == 2) { // 'll'
                        sValue = (int64_t) va_arg(arguments, long long);
                    } else { // 'z'
                        sValue = (int64_t) va_arg(arguments, size_t);
                    }
                } break;
                case 'x':
                case 'X':
                case 'u': {
                    if (length == 0) {
                        uValue = (uint64_t) va_arg(arguments, unsigned int);
                    } else if (length == 1) { // 'l'
                        uValue = (uint64_t) va_arg(arguments, unsigned long);
                    } else if (length == 2) { // 'll'
                        uValue = (uint64_t) va_arg(arguments, long long);
                    } else { // 'z'
                        uValue = (uint64_t) va_arg(arguments, size_t);
                    }
                } break;
                case 'p': {
                    uValue = (uint64_t)(uintptr_t) va_arg(arguments, void*);
                } break;
                case 's': {
                    string = va_arg(arguments, char*);
                    if (string == NULL) {
                        string = "(null)";
                    }
                } break;
                case 'c': {
                    buffer[0] = (char) va_arg(arguments, int);
                    buffer[1] = 0;
                    string = buffer;
                    strLen = 1;
                } break;
                case 'g': {
                    double d = (double) va_arg(arguments, double);
                    HAPError res = HAPFloatGetDescription(buffer, sizeof buffer, (float) d);
                    if (res != kHAPError_None) {
                        return res;
                    }
                    string = buffer;
                } break;
                default: {
                    HAPLogError(&kHAPLog_Default, "Unsupported format string type specifier: %%%c", c);
                    HAPPreconditionFailure();
                }
            }
            if (!string) { // convert integer to string
                HAPError res;
                if (c == 'i' || c == 'd') { // signed
                    if (sValue < 0) {
                        sValue = -sValue;
                        sign = '-';
                    }
                    uValue = (uint64_t) sValue;
                }
                if (c == 'x') {
                    res = HAPUInt64GetHexDescription(uValue, buffer, sizeof buffer, kHAPLetterCase_Lowercase);
                } else if (c == 'X') {
                    res = HAPUInt64GetHexDescription(uValue, buffer, sizeof buffer, kHAPLetterCase_Uppercase);
                } else if (c == 'p') {
                    buffer[0] = '0';
                    buffer[1] = 'x';
                    res = HAPUInt64GetHexDescription(uValue, buffer + 2, sizeof buffer - 2, kHAPLetterCase_Lowercase);
                } else {
                    res = HAPUInt64GetDescription(uValue, buffer, sizeof buffer);
                }
                if (res != kHAPError_None) {
                    return res;
                }
                string = buffer;
            }
            if (strLen == 0) {
                strLen = HAPStringGetNumBytes(string);
            }
            size_t totLen = strLen;
            if (sign)
                totLen++;
            if (n + totLen >= maxBytes || n + width >= maxBytes) {
                return kHAPError_OutOfResources;
            }
            if (sign && prefix == '0') {
                bytes[n++] = sign; // write sign before zeros
            }
            while (totLen < width) {
                bytes[n++] = prefix;
                totLen++;
            }
            if (sign && prefix == ' ') {
                bytes[n++] = sign; // write sign after spaces
            }
            HAPRawBufferCopyBytes(&bytes[n], string, strLen);
            n += strLen;
        } else {
            // copy char to output
            if (n + 1 >= maxBytes) {
                return kHAPError_OutOfResources;
            }
            bytes[n++] = c;
        }
        c = format[i++];
    }
    bytes[n] = 0;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
size_t HAPStringGetNumBytes(const char* string) {
    HAPPrecondition(string);

    const char* stringEnd;
    for (stringEnd = string; *stringEnd; stringEnd++)
        ;

    HAPAssert(stringEnd >= string);
    return (size_t)(stringEnd - string);
}

HAP_RESULT_USE_CHECK
bool HAPStringAreEqual(const char* string, const char* otherString) {
    HAPPrecondition(string);
    HAPPrecondition(otherString);

    if (string == otherString) {
        return true;
    }

    const char* c1;
    const char* c2;
    for (c1 = string, c2 = otherString; *c1 == *c2 && *c1; c1++, c2++)
        ;

    return !*c1 && !*c2;
}
