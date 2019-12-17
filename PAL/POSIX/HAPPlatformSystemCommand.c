// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "HAPPlatformSystemCommand.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "SystemCommand" };

HAP_RESULT_USE_CHECK
HAPError HAPPlatformSystemCommandRun(
        char* _Nullable const command[_Nonnull],
        char* bytes,
        size_t maxBytes,
        size_t* numBytes) {
    HAPPrecondition(command);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    return HAPPlatformSystemCommandRunWithEnvironment(command, /* environment: */ NULL, bytes, maxBytes, numBytes);
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformSystemCommandRunWithEnvironment(
        char* _Nullable const command[_Nonnull],
        char* _Nullable const environment[_Nullable],
        char* bytes,
        size_t maxBytes,
        size_t* numBytes) {
    HAPPrecondition(command);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);

    int e;
    int pipeFDs[2];

    e = pipe(pipeFDs);
    if (e) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPLogError(&logObject, "%s: pipe failed: %d.", __func__, _errno);
        HAPFatalError();
    }

    pid_t commandPID = fork();
    if (commandPID < 0) {
        int _errno = errno;
        HAPAssert(commandPID == -1);
        HAPLogError(&logObject, "%s: fork failed: %d.", __func__, _errno);
        HAPFatalError();
    }

    if (commandPID == 0) {
        // Forked process.

        // Remove signal handlers as they are inherited when we fork.
        if (signal(SIGTERM, SIG_DFL) == SIG_ERR) {
            int _errno = errno;
            HAPLogError(&logObject, "%s: signal TERM failed: %d.", __func__, _errno);
            HAPFatalError();
        }
        if (signal(SIGUSR1, SIG_DFL) == SIG_ERR) {
            int _errno = errno;
            HAPLogError(&logObject, "%s: signal USR1 failed: %d.", __func__, _errno);
            HAPFatalError();
        }
        if (signal(SIGUSR2, SIG_DFL) == SIG_ERR) {
            int _errno = errno;
            HAPLogError(&logObject, "%s: signal USR2 failed: %d.", __func__, _errno);
            HAPFatalError();
        }

        (void) close(pipeFDs[0]);

        do {
            e = dup2(pipeFDs[1], STDOUT_FILENO);
        } while (e == -1 && errno == EINTR);
        if (e == -1) {
            int _errno = errno;
            (void) close(pipeFDs[1]);
            HAPLogError(&logObject, "%s: dup2 STDOUT failed: %d.", __func__, _errno);
            HAPFatalError();
        }

        (void) close(pipeFDs[1]);

        e = execve(command[0], command, environment);
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            HAPLogError(&logObject, "%s: execve failed: %d.", __func__, _errno);
            HAPFatalError();
        }
        // Unreachable
        HAPFatalError();
    }

    // Main process.
    (void) close(pipeFDs[1]);

    bool bufferTooSmall = false;
    bool successfulRead = true;

    size_t o = 0;
    while (o < maxBytes) {
        size_t c = maxBytes - o;
        ssize_t n;
        do {
            n = read(pipeFDs[0], &((uint8_t*) bytes)[o], c);
        } while (n == -1 && errno == EINTR);
        if (n <= -1) {
            int _errno = errno;
            HAPAssert(n == -1);
            HAPLogError(&logObject, "%s: read failed: %d.", __func__, _errno);
            successfulRead = false;
            break;
        }

        HAPAssert((size_t) n <= c);
        o += (size_t) n;

        if (o == maxBytes) {
            // Try to read one additional byte to check if there is more data.
            char tempBuffer;
            do {
                n = read(pipeFDs[0], &tempBuffer, 1);
            } while (n == -1 && errno == EINTR);
            if (n <= -1) {
                int _errno = errno;
                HAPAssert(n == -1);
                HAPLogError(&logObject, "%s: read failed: %d.", __func__, _errno);
                successfulRead = false;
                break;
            }
            if (n == 1) {
                bufferTooSmall = true;
                break;
            }
            HAPAssert(n == 0);
        }

        if (n == 0) {
            break;
        }
    }
    *numBytes = o;

    (void) close(pipeFDs[0]);

    int status;
    pid_t pid;
    do {
        pid = waitpid(commandPID, &status, 0);
    } while (pid == -1 && errno == EINTR);
    int _errno = errno;

    if (pid < 0 && _errno != ECHILD) {
        HAPLogError(&logObject, "%s: waitpid failed: %d.", __func__, _errno);
        HAPFatalError();
    }

    // Check if read has been successful.
    if (!successfulRead) {
        return kHAPError_Unknown;
    }

    // Check exit status.
    HAPAssert(pid == commandPID || (pid == -1 && _errno == ECHILD));
    if (!WIFEXITED(status)) {
        HAPLogError(&logObject, "%s: Process did not exit: Status %d.", __func__, WIFEXITED(status));
        return kHAPError_Unknown;
    }

    int exitStatus = WEXITSTATUS(status);
    if (exitStatus != 0) {
        HAPLogInfo(&logObject, "%s: process exited with status %d.", __func__, exitStatus);
        return kHAPError_Unknown;
    }

    // Check buffer flag.
    if (bufferTooSmall) {
        HAPLogInfo(&logObject, "%s: buffer too small to store result.", __func__);
        return kHAPError_OutOfResources;
    }

    return kHAPError_None;
}
