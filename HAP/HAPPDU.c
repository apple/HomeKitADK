// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
bool HAPPDUIsValidOpcode(uint8_t value) {
    HAPAssert(sizeof(HAPPDUOpcode) == sizeof value);
    switch ((HAPPDUOpcode) value) {
        case kHAPPDUOpcode_CharacteristicSignatureRead:
        case kHAPPDUOpcode_CharacteristicWrite:
        case kHAPPDUOpcode_CharacteristicRead:
        case kHAPPDUOpcode_CharacteristicTimedWrite:
        case kHAPPDUOpcode_CharacteristicExecuteWrite:
        case kHAPPDUOpcode_ServiceSignatureRead:
        case kHAPPDUOpcode_CharacteristicConfiguration:
        case kHAPPDUOpcode_ProtocolConfiguration:
        case kHAPPDUOpcode_Token:
        case kHAPPDUOpcode_TokenUpdate:
        case kHAPPDUOpcode_Info: {
            return true;
        }
    }
    return false;
}
