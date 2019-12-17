// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"
#include "HAPCrypto.h"

void HAPSHA1Checksum(uint8_t checksum[kHAPSHA1Checksum_NumBytes], const void* bytes, size_t numBytes) {
    HAPAssert(checksum);
    HAPAssert(bytes);

    HAP_sha1(checksum, bytes, numBytes);
}
