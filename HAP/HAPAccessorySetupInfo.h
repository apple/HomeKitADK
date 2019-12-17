// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_ACCESSORY_SETUP_INFO_H
#define HAP_ACCESSORY_SETUP_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Duration after which a dynamic setup code expires.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 4.4.2.1 Requirements
 */
#define kHAPAccessorySetupInfo_DynamicRefreshInterval ((HAPTime)(5 * HAPMinute))

/**
 * Fetches the currently active setup info.
 *
 * - This may only be called while a pairing attempt is in progress.
 *
 * @param      server               Accessory server.
 * @param      restorePrevious      Whether or not a setup info from a previous pairing attempt should be reused.
 *
 * @return Setup info, if available. NULL otherwise. Always non-NULL if restorePrevious is false.
 */
HAPSetupInfo* _Nullable HAPAccessorySetupInfoGetSetupInfo(HAPAccessoryServerRef* server, bool restorePrevious);

/**
 * Handles accessory server start.
 *
 * - This starts periodically generating setup codes for displays.
 *
 * @param      server               Accessory server.
 */
void HAPAccessorySetupInfoHandleAccessoryServerStart(HAPAccessoryServerRef* server);

/**
 * Handles accessory server stop.
 *
 * - This invalidates generated setup codes for displays and exits NFC pairing mode.
 *
 * - Must not be called if there are still active connections.
 *
 * @param      server               Accessory server.
 */
void HAPAccessorySetupInfoHandleAccessoryServerStop(HAPAccessoryServerRef* server);

/**
 * Handles accessory server state update.
 *
 * - This starts periodically generating setup codes for displays if the accessory server was unpaired.
 *
 * @param      server               Accessory server.
 */
void HAPAccessorySetupInfoHandleAccessoryServerStateUpdate(HAPAccessoryServerRef* server);

/**
 * Handles start of a pairing attempt.
 *
 * - This locks the current setup code so it does not change during the pairing attempt.
 *
 * @param      server               Accessory server.
 */
void HAPAccessorySetupInfoHandlePairingStart(HAPAccessoryServerRef* server);

/**
 * Handles completion of a pairing attempt.
 *
 * - If pairing is successful, this stops advertising a setup code.
 *   Otherwise for displays a new setup code is generated.
 *
 * @param      server               Accessory server.
 * @param      keepSetupInfo        Whether or not the current setup info should be preserved for next pairing attempt.
 */
void HAPAccessorySetupInfoHandlePairingStop(HAPAccessoryServerRef* server, bool keepSetupInfo);

/**
 * Refreshes the setup payload.
 *
 * @param      server               Accessory server.
 */
void HAPAccessorySetupInfoRefreshSetupPayload(HAPAccessoryServerRef* server);

/**
 * Enters NFC pairing mode.
 *
 * - NFC pairing mode exits automatically after 5 minutes or when pairing completes.
 *
 * @param      server               Accessory server.
 */
void HAPAccessorySetupInfoEnterNFCPairingMode(HAPAccessoryServerRef* server);

/**
 * Exits NFC pairing mode.
 *
 * @param      server               Accessory server.
 */
void HAPAccessorySetupInfoExitNFCPairingMode(HAPAccessoryServerRef* server);

/**
 * Enters legacy pairing mode.
 *
 * - Legacy pairing mode exits automatically after 5 minutes or when pairing completes or is cancelled.
 *   In legacy pairing mode, displays are not refreshed automatically.
 *
 * @param      server               Accessory server.
 */
void HAPAccessorySetupInfoEnterLegacyPairingMode(HAPAccessoryServerRef* server);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
