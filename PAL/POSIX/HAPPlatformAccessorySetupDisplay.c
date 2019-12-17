// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

#include "HAPPlatform.h"
#include "HAPPlatformAccessorySetupDisplay+Init.h"
#include "HAPPlatformSystemCommand.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "AccessorySetupDisplay" };

static void DisplayQRCode(HAPPlatformAccessorySetupDisplayRef setupDisplay) {
    HAPPrecondition(setupDisplay);
    HAPPrecondition(setupDisplay->setupPayloadIsSet);
    HAPPrecondition(setupDisplay->setupCodeIsSet);

    HAPError err;

    HAPLogInfo(
            &logObject,
            "%s: Launching 'qrencode' to display QR code with setup code: %s.",
            __func__,
            setupDisplay->setupCode.stringValue);

    HAPSetupPayload setupPayload;
    HAPRawBufferCopyBytes(&setupPayload, &setupDisplay->setupPayload, sizeof setupPayload);

    char* const cmd[] = { "/usr/bin/env", "qrencode", "-t", "ANSI256", setupPayload.stringValue, NULL };

    char bytes[4800];
    size_t numBytes;
    extern char** environ; // declared by unistd.h

    err = HAPPlatformSystemCommandRunWithEnvironment(cmd, environ, bytes, sizeof bytes - 1, &numBytes);
    bytes[numBytes] = '\0';

    if (err == kHAPError_OutOfResources) {
        HAPLogError(&logObject, "%s: Displaying QR code failed: Buffer too small.", __func__);
        return;
    } else if (err) {
        printf("%s\n", bytes);
        HAPLogError(&logObject, "%s: Displaying QR code failed: 'qrencode' not installed.", __func__);
        return;
    }
    printf("\n%s\n", bytes);
}

void HAPPlatformAccessorySetupDisplayCreate(HAPPlatformAccessorySetupDisplayRef setupDisplay) {
    HAPPrecondition(HAVE_DISPLAY);
    HAPPrecondition(setupDisplay);

    HAPRawBufferZero(setupDisplay, sizeof *setupDisplay);
}

void HAPPlatformAccessorySetupDisplayUpdateSetupPayload(
        HAPPlatformAccessorySetupDisplayRef setupDisplay,
        const HAPSetupPayload* _Nullable setupPayload,
        const HAPSetupCode* _Nullable setupCode) {
    HAPPrecondition(HAVE_DISPLAY);
    HAPPrecondition(setupDisplay);

    if (setupCode) {
        HAPLogInfo(&logObject, "##### Setup code for display: %s", setupCode->stringValue);
        HAPRawBufferCopyBytes(&setupDisplay->setupCode, HAPNonnull(setupCode), sizeof setupDisplay->setupCode);
        setupDisplay->setupCodeIsSet = true;
    } else {
        HAPLogInfo(&logObject, "##### Setup code for display invalidated.");
        HAPRawBufferZero(&setupDisplay->setupCode, sizeof setupDisplay->setupCode);
        setupDisplay->setupCodeIsSet = false;
    }
    if (setupPayload) {
        HAPLogInfo(&logObject, "##### Setup payload for QR code display: %s", setupPayload->stringValue);
        HAPRawBufferCopyBytes(&setupDisplay->setupPayload, HAPNonnull(setupPayload), sizeof setupDisplay->setupPayload);
        setupDisplay->setupPayloadIsSet = true;
    } else {
        HAPRawBufferZero(&setupDisplay->setupPayload, sizeof setupDisplay->setupPayload);
        setupDisplay->setupPayloadIsSet = false;
    }

    if (setupDisplay->setupPayloadIsSet) {
        DisplayQRCode(setupDisplay);
    }
}

void HAPPlatformAccessorySetupDisplayHandleStartPairing(HAPPlatformAccessorySetupDisplayRef setupDisplay) {
    HAPPrecondition(HAVE_DISPLAY);
    HAPPrecondition(setupDisplay);
    HAPPrecondition(setupDisplay->setupCodeIsSet);

    HAPLogInfo(
            &logObject, "##### Pairing attempt has started with setup code: %s.", setupDisplay->setupCode.stringValue);

    if (setupDisplay->setupPayloadIsSet) {
        DisplayQRCode(setupDisplay);
    }
}

void HAPPlatformAccessorySetupDisplayHandleStopPairing(HAPPlatformAccessorySetupDisplayRef setupDisplay) {
    HAPPrecondition(HAVE_DISPLAY);
    HAPPrecondition(setupDisplay);

    HAPLogInfo(&logObject, "##### Pairing attempt has completed or has been canceled.");
}
