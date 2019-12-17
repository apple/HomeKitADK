// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "AccessorySetupDisplay" };

void HAPPlatformAccessorySetupDisplayUpdateSetupPayload(
        HAPPlatformAccessorySetupDisplayRef _Nonnull setupDisplay,
        const HAPSetupPayload* _Nullable setupPayload,
        const HAPSetupCode* _Nullable setupCode) {
    HAPPrecondition(setupDisplay);

    (void) setupPayload;
    (void) setupCode;
    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}

void HAPPlatformAccessorySetupDisplayHandleStartPairing(HAPPlatformAccessorySetupDisplayRef _Nonnull setupDisplay) {
    HAPPrecondition(setupDisplay);

    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}

void HAPPlatformAccessorySetupDisplayHandleStopPairing(HAPPlatformAccessorySetupDisplayRef _Nonnull setupDisplay) {
    HAPPrecondition(setupDisplay);

    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}
