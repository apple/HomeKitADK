// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <errno.h>
#include <linux/random.h>
#include <syscall.h>
#include <unistd.h>

#include "HAPPlatform.h"

/**
 * Linux Random Number generator.
 *
 * This Random Number generator makes use of the Linux getrandom(2) interface.
 * Please note that this interface is only supported from Linux 3.17 onwards.
 *
 * For more information see:
 *  - LWN - The long road to getrandom() in glibc: https://lwn.net/Articles/711013/
 *  - Getrandom Manpage: http://man7.org/linux/man-pages/man2/getrandom.2.html
 */

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "RandomNumber" };

void HAPPlatformRandomNumberFill(void* bytes, size_t numBytes) {
    HAPPrecondition(bytes);

    // Read random data.
    for (int i = 0; i < 5; i++) {
        size_t o = 0;
        while (o < numBytes) {
            size_t c = numBytes - o;

            // Using getrandom() to read small buffers (<= 256 bytes) from the urandom source is the preferred mode of
            // usage.
            // Source: man page of getrandom(2).
            if (c > 256) {
                c = 256;
            }

            ssize_t n;
            do {
                // Flags to call getrandom.
                const int getrandomFlags = GRND_NONBLOCK; // Use the urandom source and do not block.

                // With glibc >= 2.25 it is possible to call getrandom() directly.
                // Source: man page of getrandom(2).
                n = syscall(SYS_getrandom, &((uint8_t*) bytes)[o], c, getrandomFlags);
            } while ((n == -1) && (errno == EINTR));

            if (n < 0) {
                int _errno = errno;
                HAPAssert(n == -1);
                HAPLogError(&logObject, "Read from getrandom failed: %d.", _errno);
                HAPFatalError();
            }

            HAPAssert((size_t) n <= c);
            o += (size_t) n;
        }

        // Verify random data.
        if (numBytes < 128 / 8 || !HAPRawBufferIsZero(bytes, numBytes)) {
            return;
        }
    }
    HAPLogError(&logObject, "getrandom produced only zeros.");
    HAPFatalError();
}
