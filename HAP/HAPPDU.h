// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PDU_H
#define HAP_PDU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Opcode of a HAP PDU Request.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-8 HAP Opcode Description
 */
HAP_ENUM_BEGIN(uint8_t, HAPPDUOpcode) { /** HAP-Characteristic-Signature-Read. */
                                        kHAPPDUOpcode_CharacteristicSignatureRead = 0x01,

                                        /** HAP-Characteristic-Write. */
                                        kHAPPDUOpcode_CharacteristicWrite = 0x02,

                                        /** HAP-Characteristic-Read. */
                                        kHAPPDUOpcode_CharacteristicRead = 0x03,

                                        /** HAP-Characteristic-Timed-Write. */
                                        kHAPPDUOpcode_CharacteristicTimedWrite = 0x04,

                                        /** HAP-Characteristic-Execute-Write. */
                                        kHAPPDUOpcode_CharacteristicExecuteWrite = 0x05,

                                        /** HAP-Service-Signature-Read. */
                                        kHAPPDUOpcode_ServiceSignatureRead = 0x06,

                                        /** HAP-Characteristic-Configuration. */
                                        kHAPPDUOpcode_CharacteristicConfiguration = 0x07,

                                        /** HAP-Protocol-Configuration. */
                                        kHAPPDUOpcode_ProtocolConfiguration = 0x08,

                                        /**
                                         * HAP-Token-Request / Response.
                                         *
                                         * @see HomeKit Accessory Protocol Specification R14
                                         *      Section 5.15 Software Authentication Procedure
                                         */
                                        kHAPPDUOpcode_Token = 0x10,

                                        /**
                                         * HAP-Token-Update-Request / Response.
                                         *
                                         * @see HomeKit Accessory Protocol Specification R14
                                         *      Section 5.15 Software Authentication Procedure
                                         */
                                        kHAPPDUOpcode_TokenUpdate = 0x11,

                                        /**
                                         * HAP-Info-Request / Response.
                                         *
                                         * @see HomeKit Accessory Protocol Specification R14
                                         *      Section 5.15 Software Authentication Procedure
                                         */
                                        kHAPPDUOpcode_Info = 0x12
} HAP_ENUM_END(uint8_t, HAPPDUOpcode);

/**
 * Checks whether a value represents a valid HAP opcode.
 *
 * @param      value                Value to check.
 *
 * @return     true                 If the value is valid.
 * @return     false                Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPPDUIsValidOpcode(uint8_t value);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
