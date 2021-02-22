// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

// Basic lock database example. This header file, and the corresponding DB.c implementation in the ADK, is
// platform-independent.

#ifndef DB_H
#define DB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Total number of services and characteristics contained in the accessory.
 */
#define kAttributeCount ((size_t) 26)

/**
 * HomeKit Accessory Information service.
 */
extern const HAPService accessoryInformationService;

/**
 * Characteristics to expose accessory information and configuration (associated with Accessory Information service).
 */
extern const HAPBoolCharacteristic accessoryInformationIdentifyCharacteristic;
extern const HAPStringCharacteristic accessoryInformationManufacturerCharacteristic;
extern const HAPStringCharacteristic accessoryInformationModelCharacteristic;
extern const HAPStringCharacteristic accessoryInformationNameCharacteristic;
extern const HAPStringCharacteristic accessoryInformationSerialNumberCharacteristic;
extern const HAPStringCharacteristic accessoryInformationFirmwareRevisionCharacteristic;
extern const HAPStringCharacteristic accessoryInformationHardwareRevisionCharacteristic;
extern const HAPStringCharacteristic accessoryInformationADKVersionCharacteristic;

/**
 * HAP Protocol Information service.
 */
extern const HAPService hapProtocolInformationService;

/**
 * Pairing service.
 */
extern const HAPService pairingService;

/**
 * Lock Mechanism service.
 */
extern const HAPService lockMechanismService;

/**
 * The 'Current State' characteristic of the Lock Mechanism service.
 */
extern const HAPUInt8Characteristic lockMechanismLockCurrentStateCharacteristic;

/**
 * The 'Target State' characteristic of the Lock Mechanism service.
 */
extern const HAPUInt8Characteristic lockMechanismLockTargetStateCharacteristic;

/**
 * Lock Management service.
 */
extern const HAPService lockManagementService;

/**
 * The 'Control Point' characteristic of the Lock Management service.
 */
extern const HAPTLV8Characteristic lockManagementLockControlPointCharacteristic;

/**
 * The 'Version' characteristic of the Lock Management service.
 */
extern const HAPStringCharacteristic lockManagementVersionCharacteristic;

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
