// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"
#include "HAPCrypto.h"

void HAPPlatformRandomNumberFill(void* bytes, size_t numBytes) {
    HAP_rand(bytes, numBytes);
}
