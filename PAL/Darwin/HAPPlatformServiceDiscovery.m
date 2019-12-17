// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatformServiceDiscovery+Init.h"

#import <Foundation/Foundation.h>

static const HAPLogObject sd_log = { .subsystem = kHAPPlatform_LogSubsystem, .category = "ServiceDiscovery" };

static NSNetService* service = nil;

void HAPPlatformServiceDiscoveryCreate(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        const HAPPlatformServiceDiscoveryOptions* options) {
    HAPPrecondition(serviceDiscovery);
}

void HAPPlatformServiceDiscoveryRegister(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        const char* name,
        const char* protocol,
        uint16_t port,
        HAPPlatformServiceDiscoveryTXTRecord* txtRecords,
        size_t numTXTRecords) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(!service);
    HAPPrecondition(name);
    HAPPrecondition(protocol);
    HAPPrecondition(!numTXTRecords || txtRecords);
    service = [[NSNetService alloc] initWithDomain:@"" type:@(protocol) name:@(name) port:port];
    HAPPlatformServiceDiscoveryUpdateTXTRecords(serviceDiscovery, txtRecords, numTXTRecords);
    [service publish];

    HAPLog(&sd_log, "Publishing %s:%s on port %u", name, protocol, port);
}

void HAPPlatformServiceDiscoveryUpdateTXTRecords(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        HAPPlatformServiceDiscoveryTXTRecord* txtRecords,
        size_t numTXTRecords) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(service);
    HAPPrecondition(!numTXTRecords || txtRecords);
    NSMutableDictionary* txt = [NSMutableDictionary dictionaryWithCapacity:numTXTRecords];
    for (HAPPlatformServiceDiscoveryTXTRecord* p = txtRecords; p < txtRecords + numTXTRecords; ++p) {
        [txt setObject:[NSData dataWithBytes:p->value.bytes length:p->value.numBytes] forKey:@(p->key)];
        HAPLogBuffer(&sd_log, p->value.bytes, p->value.numBytes, "TXT record");
    }
    [service setTXTRecordData:[NSNetService dataFromTXTRecordDictionary:txt]];
}

void HAPPlatformServiceDiscoveryStop(HAPPlatformServiceDiscoveryRef serviceDiscovery) {
    HAPPrecondition(serviceDiscovery);
    HAPPrecondition(service);
    [service stop];
    service = nil;
}
