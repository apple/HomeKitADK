// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

int main() {
    // See HomeKit Accessory Protocol Specification R14
    // Section 4.2.1.2 Invalid Setup Codes
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("000-00-000"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("111-11-111"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("222-22-222"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("333-33-333"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("444-44-444"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("555-55-555"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("666-66-666"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("777-77-777"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("888-88-888"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("999-99-999"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("123-45-678"));
    HAPAssert(!HAPAccessorySetupIsValidSetupCode("876-54-321"));
}
