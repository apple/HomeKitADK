// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_INTERNAL_H
#define HAP_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

// Core features.
#ifndef HAP_IP
#define HAP_IP 1
#endif
#ifndef HAP_BLE
#define HAP_BLE 1
#endif

#include "HAP.h"

#include "HAPCrypto.h"

#include "util_http_reader.h"

#include "HAPStringBuilder.h"

#include "HAPDeviceID.h"
#include "HAPPDU.h"

#include "HAPMFiAuth.h"
#include "HAPMFiHWAuth+Types.h"
#include "HAPMFiHWAuth.h"

#include "HAPPairing.h"
#include "HAPPairingBLESessionCache.h"
#include "HAPPairingPairSetup.h"
#include "HAPPairingPairVerify.h"
#include "HAPPairingPairings.h"

#include "HAPSession.h"

#include "HAPBLEPDU+TLV.h"
#include "HAPBLEPDU.h"
#include "HAPBLETransaction.h"

#include "HAPBLEAccessoryServer+Advertising.h"
#include "HAPBLEAccessoryServer+Broadcast.h"
#include "HAPBLEAccessoryServer.h"
#include "HAPBLECharacteristic+Broadcast.h"
#include "HAPBLECharacteristic+Configuration.h"
#include "HAPBLECharacteristic+Signature.h"
#include "HAPBLECharacteristic+Value.h"
#include "HAPBLECharacteristic.h"

#include "HAPBLEPeripheralManager.h"
#include "HAPBLEProcedure.h"
#include "HAPBLEProtocol+Configuration.h"
#include "HAPBLEService+Signature.h"
#include "HAPBLESession.h"

#include "HAPIP+ByteBuffer.h"
#include "HAPIPAccessory.h"
#include "HAPIPAccessoryProtocol.h"
#include "HAPIPCharacteristic.h"
#include "HAPIPSecurityProtocol.h"
#include "HAPIPSession.h"

#include "HAPIPAccessoryServer.h"
#include "HAPIPServiceDiscovery.h"

#include "HAP+KeyValueStoreDomains.h"
#include "HAPAccessory+Info.h"
#include "HAPAccessoryServer+Internal.h"
#include "HAPAccessorySetup.h"
#include "HAPAccessorySetupInfo.h"
#include "HAPAccessoryValidation.h"
#include "HAPCharacteristic.h"

#include "HAPJSONUtils.h"
#include "HAPLog+Attributes.h"
#include "HAPMACAddress.h"
#include "HAPMFiTokenAuth.h"
#include "HAPTLV+Internal.h"
#include "HAPUUID.h"

#include "HAPCharacteristicTypes+TLV.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

#define kHAP_LogSubsystem "com.apple.mfi.HomeKit.Core"

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
