// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
HAPError HAPBLEServiceGetSignatureReadResponse(const HAPService* _Nullable service, HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(responseWriter);

    HAPError err;

    err = HAPBLEPDUTLVSerializeHAPServiceProperties(service, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    err = HAPBLEPDUTLVSerializeHAPLinkedServices(service, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}
