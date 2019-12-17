// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include "HAP.h"
#include "HAPPlatformLog+Init.h"

#ifdef __linux__
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200112L
#error "This file needs the XSI-compliant version of 'strerror_r'. Set _POSIX_C_SOURCE >= 200112L."
#endif
#if defined(_GNU_SOURCE) && _GNU_SOURCE
#error "This file needs the XSI-compliant version of 'strerror_r'. Do not set _GNU_SOURCE."
#endif
#endif

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "Log" };

void HAPPlatformLogPOSIXError(
        HAPLogType type,
        const char* _Nonnull message,
        int errorNumber,
        const char* _Nonnull function,
        const char* _Nonnull file,
        int line) {
    HAPPrecondition(message);
    HAPPrecondition(function);
    HAPPrecondition(file);

    HAPError err;

    // Get error message.
    char errorString[256];
    int e = strerror_r(errorNumber, errorString, sizeof errorString);
    if (e == EINVAL) {
        err = HAPStringWithFormat(errorString, sizeof errorString, "Unknown error %d", errorNumber);
        HAPAssert(!err);
    } else if (e) {
        HAPAssert(e == ERANGE);
        HAPLog(&logObject, "strerror_r error: ERANGE.");
        return;
    }

    // Perform logging.
    HAPLogWithType(&logObject, type, "%s:%d:%s - %s @ %s:%d", message, errorNumber, errorString, function, file, line);
}

HAP_RESULT_USE_CHECK
HAPPlatformLogEnabledTypes HAPPlatformLogGetEnabledTypes(const HAPLogObject* _Nonnull log HAP_UNUSED) {
    switch (HAP_LOG_LEVEL) {
        case 0: {
            return kHAPPlatformLogEnabledTypes_None;
        }
        case 1: {
            return kHAPPlatformLogEnabledTypes_Default;
        }
        case 2: {
            return kHAPPlatformLogEnabledTypes_Info;
        }
        case 3: {
            return kHAPPlatformLogEnabledTypes_Debug;
        }
        default: {
            HAPFatalError();
        }
    }
}

void HAPPlatformLogCapture(
        const HAPLogObject* _Nonnull log,
        HAPLogType type,
        const char* _Nonnull message,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes) HAP_DIAGNOSE_ERROR(!bufferBytes && numBufferBytes, "empty buffer cannot have a length") {
    HAPPrecondition(log);
    HAPPrecondition(message);
    HAPPrecondition(!numBufferBytes || bufferBytes);

    static volatile bool captureLock = 0;
    while (__atomic_test_and_set(&captureLock, __ATOMIC_SEQ_CST))
        ;

    // Format log message.
    bool logHandled = false;

    // Perform regular logging.
    if (!logHandled) {
        // Color.
        switch (type) {
            case kHAPLogType_Debug: {
                fprintf(stderr, "\x1B[0m");
            } break;
            case kHAPLogType_Info: {
                fprintf(stderr, "\x1B[32m");
            } break;
            case kHAPLogType_Default: {
                fprintf(stderr, "\x1B[35m");
            } break;
            case kHAPLogType_Error: {
                fprintf(stderr, "\x1B[31m");
            } break;
            case kHAPLogType_Fault: {
                fprintf(stderr, "\x1B[1m\x1B[31m");
            } break;
        }

// Time.
#ifdef _WIN32
        SYSTEMTIME now;
        GetSystemTime(&now);
        (void) fprintf(
                stderr,
                "%04d-%02d-%02d'T'%02d:%02d:%02d'Z'",
                now.wYear,
                now.wMonth,
                now.wDay,
                now.wHour,
                now.wMinute,
                now.wSecond);
#else
        struct timeval now;
        int err = gettimeofday(&now, NULL);
        if (!err) {
            struct tm g;
            struct tm* gmt = gmtime_r(&now.tv_sec, &g);
            if (gmt) {
                (void) fprintf(
                        stderr,
                        "%04d-%02d-%02d'T'%02d:%02d:%02d'Z'",
                        1900 + gmt->tm_year,
                        1 + gmt->tm_mon,
                        gmt->tm_mday,
                        gmt->tm_hour,
                        gmt->tm_min,
                        gmt->tm_sec);
            }
        }
#endif
        (void) fprintf(stderr, "\t");

        // Type.
        switch (type) {
            case kHAPLogType_Debug: {
                (void) fprintf(stderr, "Debug");
            } break;
            case kHAPLogType_Info: {
                (void) fprintf(stderr, "Info");
            } break;
            case kHAPLogType_Default: {
                (void) fprintf(stderr, "Default");
            } break;
            case kHAPLogType_Error: {
                (void) fprintf(stderr, "Error");
            } break;
            case kHAPLogType_Fault: {
                (void) fprintf(stderr, "Fault");
            } break;
        }
        (void) fprintf(stderr, "\t");

        // Subsystem / Category.
        if (log->subsystem) {
            (void) fprintf(stderr, "[%s", log->subsystem);
            if (log->category) {
                (void) fprintf(stderr, ":%s", log->category);
            }
            (void) fprintf(stderr, "] ");
        }

        // Message.
        (void) fprintf(stderr, "%s", message);
        (void) fprintf(stderr, "\n");

        // Buffer.
        if (bufferBytes) {
            size_t i, n;
            const uint8_t* b = bufferBytes;
            size_t length = numBufferBytes;
            if (length == 0) {
                (void) fprintf(stderr, "\n");
            } else {
                i = 0;
                do {
                    (void) fprintf(stderr, "    %04zx ", i);
                    for (n = 0; n != 8 * 4; n++) {
                        if (n % 4 == 0) {
                            (void) fprintf(stderr, " ");
                        }
                        if ((n <= length) && (i < length - n)) {
                            (void) fprintf(stderr, "%02x", b[i + n] & 0xff);
                        } else {
                            (void) fprintf(stderr, "  ");
                        }
                    };
                    (void) fprintf(stderr, "    ");
                    for (n = 0; n != 8 * 4; n++) {
                        if (i != length) {
                            if ((32 <= b[i]) && (b[i] < 127)) {
                                (void) fprintf(stderr, "%c", b[i]);
                            } else {
                                (void) fprintf(stderr, ".");
                            }
                            i++;
                        }
                    }
                    (void) fprintf(stderr, "\n");
                } while (i != length);
            }
        }

        // Reset color.
        fprintf(stderr, "\x1B[0m");
    }

    // Finish log.
    (void) fflush(stderr);

    __atomic_clear(&captureLock, __ATOMIC_SEQ_CST);
}
