// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <stdio.h>
#include <stdlib.h>

#include "HAP+Internal.h"

int main(int argc, char* argv[]) {
    HAPError err;

    if (argc <= 1) {

#ifndef _WIN32
#define B(stringLiteral) "\x1B[1m" stringLiteral "\x1B[0m"
#define U(stringLiteral) "\x1B[4m" stringLiteral "\x1B[0m"
#else
#define B(stringLiteral) stringLiteral
#define U(stringLiteral) stringLiteral
#endif

        const char* categoryDescriptions =
                "           1  Other.\n"
                "           2  Bridges.\n"
                "           3  Fans.\n"
                "           4  Garage Door Openers.\n"
                "           5  Lighting.\n"
                "           6  Locks.\n"
                "           7  Outlets.\n"
                "           8  Switches.\n"
                "           9  Thermostats.\n"
                "          10  Sensors.\n"
                "          11  Security Systems.\n"
                "          12  Doors.\n"
                "          13  Windows.\n"
                "          14  Window Coverings.\n"
                "          15  Programmable Switches.\n"
                "          16  Range Extenders.\n"
                "          19  Air Purifiers.\n"
                "          20  Heaters.\n"
                "          21  Air Conditioners.\n"
                "          22  Humidifiers.\n"
                "          23  Dehumidifiers.\n"
                "          28  Sprinklers.\n"
                "          29  Faucets.\n"
                "          30  Shower Systems.\n";

        const char* exampleOutput =
                "     1\n"
                "     518-08-582\n"
                "     263FEA64889756A8E25FD53DD5FA1022\n"
                "     D0BE3DFCC3B28A4D612943215AD71005CA4E240A5672EFF427F30EEAC173756167AC4D73779\n"
                "        3AF18937B1770E173ED346AB790E428B2771ACA62FE11C1A0FC8E01169824632BB914863\n"
                "        760918841CB3F263D5D71C431A2141C51797A91022C5BCD30D7BC9259A2037C4BDEE8F74\n"
                "        8D65B15AEA33DF2F00193FBAAC603C921820D2E4FE5747F965F31F3DD16D8A7228FE8FC8\n"
                "        5AD70138C797CB91B47488283C568D1CDAFCF6E950A1D117BD4E42FB0B90FF97992BCCE0\n"
                "        C86F62F866489BC2F556D342F4C20AC26B12A48299C642BE86270F0D3F1E6E86E84115A7\n"
                "        12931F7FE1D53E6230FB14C29AD2E23B16E0B8F6AFD4D709B562DC4921F550450AC8FD09\n"
                "        73DD80DAE629CB399DD6E3E96695E2E8060196D5FFFD292A1246AD76219E998FDD0E690B\n"
                "        405A0D2AD9C9CADF905520C4E6B66952E0DA27E523060DE310A539F6BF30E48B69A5F26D\n"
                "        5E283DE6EE8F51AFB920E00D1B1AE3BA423041A63BA788B6F6BCBA2AD7C89946EEE79D72\n"
                "        6649BCEAB43BB920F11260F8017C9921A60C169B28569\n"
                "     7OSX\n"
                "     X-HM://007JNU5AE7OSX\n";
        printf(
            B("HomeKit Accessory Setup Generator")" - Version %s (%s)\n"
            "\n"
            B("USAGE")"\n"
            "     "B("AccessorySetupGenerator")" [OPTION]...\n"
            "\n"
            B("DESCRIPTION")"\n"
            "     This tool generates information for provisioning of a HomeKit accessory,\n"
            "     namely a setup code, a corresponding SRP salt and verifier, and a setup ID.\n"
            "     The setup code is used by the controller to set up an encrypted link with\n"
            "     the accessory during HomeKit pairing. The setup ID is used to identify\n"
            "     the accessory to which a scanned label belongs.\n"
            "     \n"
            "     "B("Each accessory needs to be provisioned with unique accessory setup")"\n"
            "     "B("information before it may be used.")"\n"
            "\n"
            B("OPTIONS")"\n"
            "     The following options are available:\n"
            "     \n"
            "     "B("--ip")"\n"
            "        Accessory supports HAP over IP transport; \n"
            "     \n"
            "     "B("--ble")"\n"
            "        Accessory supports HAP over BLE transport.\n"
            "     \n"
            "     "B("--category")" "U("Category")"\n"
            "        The accessory category.\n"
            "        \n"
            "        An accessory with support for multiple categories should advertise the\n"
            "        primary category. An accessory for which a primary category cannot be\n"
            "        determined or the primary category isn't among the well defined\n"
            "        categories falls in the `Other` category.\n"
            "        \n"
            "        Well defined categories:\n"
            "%s\n"
            "     "B("--setup-code")" "U("Setup code")"\n"
            "        Generates accessory setup information that allows pairing using the\n"
            "        specified setup code (e.g. for development).\n"
            "        Format is `XXX-XX-XXX` with X being a digit from 0-9.\n"
            "        - Setup codes that only consist of a repeating digit are not allowed.\n"
            "        - `123-45-678` and `876-54-321` are not allowed.\n"
            "        If this option is not present, a random setup code is generated.\n"
            "     \n"
            "     "B("--setup-id")" "U("Setup ID")"\n"
            "        Provisions accessory setup information using a specific setup ID.\n"
            "        Format is `XXXX` with X being a digit from 0-9 or a character from A-Z.\n"
            "        - Lowercase characters are not allowed.\n"
            "        If this option is not present, a random setup ID is generated.\n"
            "\n"
            B("OUTPUT")"\n"
            "     Output consists of a series of lines in a machine-readable format.\n"
            "     Lines are terminated with a \\n character.\n"
            "     \n"
            "     1. "B("Output format version")" which is `1` for this version.\n"
            "     \n"
            "     2. "B("Setup code")" in format `XXX-XX-XXX` with X being a digit from 0-9.\n"
            "        - Must be deployed to the accessory if it has a programmable NFC tag but\n"
            "          is not connected to a display.\n"
            "        - Must be printed on labels affixed to the accessory and its packaging\n"
            "          if the accessory is not connected to a display.\n"
            "     \n"
            "     3. "B("SRP salt")" as a hexadecimal string.\n"
            "        - Must be deployed to the accessory if it is not connected to a display.\n"
            "     \n"
            "     4. "B("SRP verifier")" as a hexadecimal string.\n"
            "        - Must be deployed to the accessory if it is not connected to a display.\n"
            "     \n"
            "     5. "B("Setup ID")" in format `XXX` with X being a digit from 0-9 or a\n"
            "        character from A-Z.\n"
            "        - Must be deployed to the accessory.\n"
            "     \n"
            "     6. "B("Setup payload")" as a string.\n"
            "        - Must be printed on labels affixed to the accessory and its packaging\n"
            "          if the accessory is not connected to a display.\n"
            "\n"
            B("EXAMPLE")"\n"
            "     Example output for an "B("Outlet")" (category identifier "B("7")") accessory supporting\n"
            "     "B("HAP over IP")" and "B("Wi-Fi Accessory Configuration")" with setup code `"B("518-08-582")"`\n"
            "     and setup ID `"B("7OSX")"`.\n"
            "     \n"
            "%s",
            HAPGetVersion(), HAPGetBuild(), categoryDescriptions, exampleOutput);
#undef B
#undef U
        exit(EXIT_SUCCESS);
    }

    // Parse arguments.
    const char* fixedSetupCode = NULL;
    const char* fixedSetupID = NULL;
    HAPAccessorySetupSetupPayloadFlags flags = { .isPaired = false, .ipSupported = false, .bleSupported = false };
    HAPAccessoryCategory category = (HAPAccessoryCategory) 0;
    for (int i = 1; i < argc; i++) {
        if (HAPStringAreEqual(argv[i], "--ip")) {
            if (flags.ipSupported) {
                fprintf(stderr, "--ip specified multiple times.");
                exit(EXIT_FAILURE);
            }
            flags.ipSupported = true;
        } else if (HAPStringAreEqual(argv[i], "--ble")) {
            if (flags.bleSupported) {
                fprintf(stderr, "--ble specified multiple times.");
                exit(EXIT_FAILURE);
            }
            flags.bleSupported = true;
        } else if (HAPStringAreEqual(argv[i], "--category")) {
            if (i == argc) {
                fprintf(stderr, "--category specified without accessory category identifier.");
                exit(EXIT_FAILURE);
            }
            i++;
            uint64_t categoryID;
            err = HAPUInt64FromString(argv[i], &categoryID);
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
                fprintf(stderr, "--category specified with malformed accessory category identifier.");
                exit(EXIT_FAILURE);
            }
            if (!categoryID || categoryID > UINT8_MAX) {
                fprintf(stderr, "--category specified with out-of-range accessory category identifier.");
                exit(EXIT_FAILURE);
            }
            if (category) {
                fprintf(stderr, "--category specified multiple times.");
                exit(EXIT_FAILURE);
            }
            category = (HAPAccessoryCategory) categoryID;
        } else if (HAPStringAreEqual(argv[i], "--setup-code")) {
            if (i == argc) {
                fprintf(stderr, "--setup-code specified without setup code.");
                exit(EXIT_FAILURE);
            }
            i++;
            if (!HAPAccessorySetupIsValidSetupCode(argv[i])) {
                fprintf(stderr, "--setup-code specified with invalid setup code.");
                exit(EXIT_FAILURE);
            }
            if (fixedSetupCode) {
                fprintf(stderr, "--setup-code specified multiple times.");
                exit(EXIT_FAILURE);
            }
            fixedSetupCode = argv[i];
        } else if (HAPStringAreEqual(argv[i], "--setup-id")) {
            if (i == argc) {
                fprintf(stderr, "--setup-id specified without setup ID.");
                exit(EXIT_FAILURE);
            }
            i++;
            if (!HAPAccessorySetupIsValidSetupID(argv[i])) {
                fprintf(stderr, "--setup-id specified with invalid setup ID.");
                exit(EXIT_FAILURE);
            }
            if (fixedSetupID) {
                fprintf(stderr, "--setup-id specified multiple times.");
                exit(EXIT_FAILURE);
            }
            fixedSetupID = argv[i];
        } else {
            fprintf(stderr, "Too many arguments specified.");
            exit(EXIT_FAILURE);
        }
    }
    if (!category) {
        fprintf(stderr, "No accessory category identifier specified.");
        exit(EXIT_FAILURE);
    }
    if (!flags.ipSupported && !flags.bleSupported) {
        fprintf(stderr, "No transport specified.");
        exit(EXIT_FAILURE);
    }

    // Setup code.
    HAPSetupCode setupCode;
    if (fixedSetupCode) {
        HAPRawBufferCopyBytes(setupCode.stringValue, fixedSetupCode, sizeof setupCode.stringValue);
    } else {
        HAPAccessorySetupGenerateRandomSetupCode(&setupCode);
    }

    // Setup info.
    HAPSetupInfo setupInfo;
    HAPPlatformRandomNumberFill(setupInfo.salt, sizeof setupInfo.salt);
    const uint8_t srpUserName[] = "Pair-Setup";
    HAP_srp_verifier(
            setupInfo.verifier,
            setupInfo.salt,
            srpUserName,
            sizeof srpUserName - 1,
            (const uint8_t*) setupCode.stringValue,
            sizeof setupCode.stringValue - 1);

    // Setup ID.
    HAPSetupID setupID;
    if (fixedSetupID) {
        HAPRawBufferCopyBytes(setupID.stringValue, fixedSetupID, sizeof setupID.stringValue);
    } else {
        HAPAccessorySetupGenerateRandomSetupID(&setupID);
    }

    // Setup payload.
    HAPSetupPayload setupPayload;
    HAPAccessorySetupGetSetupPayload(&setupPayload, &setupCode, &setupID, flags, category);

    // Output.
    printf("1\n");
    printf("%s\n", setupCode.stringValue);
    for (size_t i = 0; i < sizeof setupInfo.salt; i++) {
        printf("%02X", setupInfo.salt[i]);
    }
    printf("\n");
    for (size_t i = 0; i < sizeof setupInfo.verifier; i++) {
        printf("%02X", setupInfo.verifier[i]);
    }
    printf("\n");
    printf("%s\n", setupID.stringValue);
    printf("%s\n", setupPayload.stringValue);

    return 0;
}
