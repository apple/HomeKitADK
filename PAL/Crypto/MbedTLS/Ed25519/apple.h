// Copyright (c) 2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPCrypto.h"

#define SHA512_DIGEST_LENGTH SHA512_BYTES

#define mem_alloc(size)      malloc(size)
#define mem_free(ptr)        free(ptr)
#define mem_clear(ptr, size) memset(ptr, 0, size)
