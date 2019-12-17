// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_ACCESSORY_VALIDATION_H
#define HAP_ACCESSORY_VALIDATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Validates a regular (Bluetooth LE / IP) accessory definition.
 *
 * @param      server               Accessory server.
 * @param      accessory            Accessory to validate.
 *
 * @return true                     If the accessory definition is valid.
 * @return false                    Otherwise.
 */
bool HAPRegularAccessoryIsValid(HAPAccessoryServerRef* server, const HAPAccessory* accessory);

/**
 * Validates a bridged accessory definition.
 *
 * @param      bridgedAccessory     Accessory to validate.
 *
 * @return true                     If the accessory definition is valid.
 * @return false                    Otherwise.
 */
bool HAPBridgedAccessoryIsValid(const HAPAccessory* bridgedAccessory);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
