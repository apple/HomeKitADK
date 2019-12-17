// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

// A simple test to verify the behavior of HAPPlatformSystemCommand.
//
// This test requires the POSIX commands echo, true, false.

#ifndef ECHO_COMMAND
#define ECHO_COMMAND "/bin/echo"
#endif

#ifndef TRUE_COMMAND
#define TRUE_COMMAND "/usr/bin/env", "true"
#endif
#ifndef FALSE_COMMAND
#define FALSE_COMMAND "/usr/bin/env", "false"
#endif

#include <string.h>

#include "HAPPlatformSystemCommand.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem ".Test", .category = "SystemCommand" };

static void RunAndCheck(char* const cmd[], const char* _Nullable expectedResult, HAPError expectedError) {
    HAPPrecondition(cmd);

    HAPLogInfo(&logObject, "Testing: %s %s", cmd[0], cmd[1] ? cmd[1] : "");
    HAPError err;
    char result[6];

    HAPRawBufferZero(result, sizeof(result));
    size_t written;
    err = HAPPlatformSystemCommandRun(cmd, result, sizeof(result), &written);
    HAPAssert(err == expectedError);
    if (expectedResult) {
        HAPAssert(strcmp(expectedResult, result));
    }
}

int main() {
    {
        char* const cmd[] = { ECHO_COMMAND, "true", NULL };
        RunAndCheck(cmd, "true", kHAPError_None);
    }
    {
        char* const cmd[] = { ECHO_COMMAND, "false", NULL };
        RunAndCheck(cmd, "false", kHAPError_None);
    }
    {
        char* const cmd[] = { TRUE_COMMAND, NULL };
        RunAndCheck(cmd, NULL, kHAPError_None);
    }
    {
        char* const cmd[] = { FALSE_COMMAND, NULL };
        RunAndCheck(cmd, NULL, kHAPError_Unknown);
    }
    {
        char* const cmd[] = { ECHO_COMMAND, "Extra Long string which does not fit into buffer.", NULL };
        RunAndCheck(cmd, NULL, kHAPError_OutOfResources);
    }
}
