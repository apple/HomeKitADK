// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "HAPPlatform+Init.h"
#include "HAPPlatformKeyValueStore+Init.h"
#include "HAPPlatformFileManager.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "KeyValueStore" };

/**
 * Enumerates directory @p dir, calling @p body on each directory entry.
 *
 * @param      dir                  Path to directory to scan.
 * @param      body                 Function to call on each directory entry.
 * @param      ctx                  Context that is passed to @p body.
 *
 * @return 0                        If successful.
 * @return -1                       Otherwise. @c errno is set accordingly.
 *
 * @remarks Errors:
 * - [EINVAL]       A required parameter is NULL.
 * - [EINVAL]       The implementation does not support an association of the variable @c _PC_NAME_MAX
 *                  with the specified file.
 * - [EACCES]       Search permission is denied for the component of the path prefix of @p dir
 *                  or read permission is denied for @p dir.
 * - [ELOOP]        A loop exists in symbolic links encountered during resolution of the @p dir argument.
 * - [ELOOP]        More than {SYMLOOP_MAX} symbolic links were encountered during resolution of the @p dir argument.
 * - [EMFILE]       All file descriptors available to the process are currently open.
 * - [ENAMETOOLONG] The length of a pathname exceeds {PATH_MAX}, or pathname resolution of a symbolic link
 *                  produced an intermediate result with a length that exceeds {PATH_MAX}.
 * - [ENFILE]       Too many files are currently open in the system.
 * - [ENOMEM]       Insufficient storage space is available.
 * - [ENOTDIR]      A component of @p dir names an existing file that is neither a directory
 *                  nor a symbolic link to a directory.
 * - [EOVERFLOW]    One of the values in the dirent structure of a directory entry cannot be represented correctly.
 * - Errors thrown by @p body are propagated as well.
 */
HAP_RESULT_USE_CHECK
static int
        enumdir(const char* dir,
                int (*body)(void* ctx, const char* dir, const struct dirent* ent, bool* cont),
                void* ctx) {
    HAPPrecondition(dir);
    HAPPrecondition(body);

    // Open directory.
    DIR* d = opendir(dir);
    if (!d) {
        int _errno = errno;
        if (errno == ENOENT) {
            errno = 0;
            return 0;
        }
        HAPLogError(&logObject, "opendir %s failed: %d.", dir, _errno);
        errno = _errno;
        return -1;
    }

    // Get fd.
    int fd = dirfd(d);
    if (fd < 0) {
        int _errno = errno;
        HAPAssert(fd == -1);
        HAPLogError(&logObject, "dirfd %s failed: %d.", dir, _errno);
        HAPPlatformFileManagerCloseDirFreeSafe(d);
        errno = _errno;
        return -1;
    }

    // Get dirent buffer.
    errno = 0;
    long maxNameBytes = fpathconf(fd, _PC_NAME_MAX);
    if (maxNameBytes < 0) {
        int _errno = errno;
        HAPAssert(maxNameBytes == -1);
        if (errno) {
            HAPLogError(&logObject, "fpathconf %s failed: %d.", dir, _errno);
        } else {
            HAPLogError(&logObject, "fpathconf %s returned indefinite {NAME_MAX}.", dir);
        }
        HAPPlatformFileManagerCloseDirFreeSafe(d);
        errno = _errno;
        return -1;
    }
    size_t maxDirentBytes = HAP_OFFSETOF(struct dirent, d_name) + (size_t) maxNameBytes + 1;
    if (sizeof(struct dirent) > maxDirentBytes) {
        maxDirentBytes = sizeof(struct dirent);
    }
    struct dirent* direntBuffer = malloc(maxDirentBytes);
    if (!direntBuffer) {
        int _errno = errno;
        HAPLogError(&logObject, "malloc %lu failed: %d.", (unsigned long) maxDirentBytes, _errno);
        HAPPlatformFileManagerCloseDirFreeSafe(d);
        errno = _errno;
        return -1;
    }

    // Iterate through directory.
    for (bool cont = true; cont;) {
        // Get next file.
        struct dirent* entry;

        // readdir_r is deprecated in newer glibc.
        // https://lwn.net/Articles/696474/
        HAP_DIAGNOSTIC_PUSH
        HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
        HAP_DIAGNOSTIC_IGNORED_GCC("-Wdeprecated-declarations")
        int e = readdir_r(d, direntBuffer, &entry);
        HAP_DIAGNOSTIC_POP
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            HAPLogError(&logObject, "readdir_r %s failed: %d.", dir, _errno);
            HAPPlatformFreeSafe(direntBuffer);
            HAPPlatformFileManagerCloseDirFreeSafe(d);
            errno = _errno;
            return -1;
        }
        if (!entry) {
            break;
        }

        // Invoke callback.
        errno = 0;
        e = body(ctx, dir, entry, &cont);
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            HAPPlatformFreeSafe(direntBuffer);
            HAPPlatformFileManagerCloseDirFreeSafe(d);
            errno = _errno;
            return -1;
        }
    }
    HAPPlatformFreeSafe(direntBuffer);
    HAPPlatformFileManagerCloseDirFreeSafe(d);
    errno = 0;
    return 0;
}

void HAPPlatformKeyValueStoreCreate(
        HAPPlatformKeyValueStoreRef keyValueStore,
        const HAPPlatformKeyValueStoreOptions* options) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(options);
    HAPPrecondition(options->rootDirectory);

    HAPLogDebug(&logObject, "Storage configuration: keyValueStore = %lu", (unsigned long) sizeof *keyValueStore);

    keyValueStore->rootDirectory = options->rootDirectory;
}

/**
 * Gets the file path under which data for a specified key is stored.
 *
 * @param      keyValueStore        Key-value store.
 * @param      domain               Domain.
 * @param      key                  Key.
 * @param[out] filePath             File path for the domain / key. NULL-terminated.
 * @param      maxFilePathLength    Maximum length that the filePath buffer may hold.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If @p cap not large enough.
 */
HAP_RESULT_USE_CHECK
static HAPError GetFilePath(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        char* filePath,
        size_t maxFilePathLength) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(keyValueStore->rootDirectory);
    HAPPrecondition(filePath);

    HAPError err;

    HAPAssert(sizeof domain == sizeof(uint8_t));
    HAPAssert(sizeof key == sizeof(uint8_t));
    err = HAPStringWithFormat(filePath, maxFilePathLength, "%s/%02X.%02X", keyValueStore->rootDirectory, domain, key);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLogError(
                &logObject,
                "Not enough resources to get path: %s/%02X.%02X",
                keyValueStore->rootDirectory,
                domain,
                key);
        return kHAPError_OutOfResources;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreGet(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        void* _Nullable bytes,
        size_t maxBytes,
        size_t* _Nullable numBytes,
        bool* found) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(keyValueStore->rootDirectory);
    HAPPrecondition(!maxBytes || bytes);
    HAPPrecondition((bytes == NULL) == (numBytes == NULL));
    HAPPrecondition(found);

    HAPError err;

    // Get file name.
    char filePath[PATH_MAX];
    err = GetFilePath(keyValueStore, domain, key, filePath, sizeof filePath);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return kHAPError_Unknown;
    }

    return HAPPlatformFileManagerReadFile(filePath, bytes, maxBytes, numBytes, found);
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreSet(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        const void* bytes,
        size_t numBytes) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(keyValueStore->rootDirectory);
    HAPPrecondition(bytes);

    HAPError err;

    char filePath[PATH_MAX];

    // Get file name.
    err = GetFilePath(keyValueStore, domain, key, filePath, sizeof filePath);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return kHAPError_Unknown;
    }

    // Write the KVS file.
    err = HAPPlatformFileManagerWriteFile(filePath, bytes, numBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreRemove(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(keyValueStore->rootDirectory);

    HAPError err;

    char filePath[PATH_MAX];

    // Get file name.
    err = GetFilePath(keyValueStore, domain, key, filePath, sizeof filePath);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return kHAPError_Unknown;
    }

    // Remove file.
    err = HAPPlatformFileManagerRemoveFile(filePath);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

/**
 * enumdir context.
 */
typedef struct {
    /**
     * Key-value store.
     */
    HAPPlatformKeyValueStoreRef keyValueStore;

    /**
     * Domain to enumerate.
     */
    HAPPlatformKeyValueStoreDomain domain;

    /**
     * Client callback.
     */
    HAPPlatformKeyValueStoreEnumerateCallback body;

    /**
     * Client context.
     */
    void* _Nullable context;
} EnumdirCallbackContext;

HAP_RESULT_USE_CHECK
static int EnumdirCallback(void* ctx, const char* dir, const struct dirent* ent, bool* cont) {
    EnumdirCallbackContext* arguments = ctx;
    HAPPrecondition(arguments);
    HAPPrecondition(arguments->keyValueStore);
    HAPPrecondition(arguments->body);
    HAPPrecondition(dir);
    HAPPrecondition(ent);
    HAPPrecondition(cont);

    HAPError err;

    // Parse file name.
    HAPAssert(ent->d_name);
    if (HAPStringAreEqual(ent->d_name, ".")) {
        return 0;
    }
    if (HAPStringAreEqual(ent->d_name, "..")) {
        return 0;
    }
    unsigned int domain;
    unsigned int key;
    int end;
    int n = sscanf(ent->d_name, "%2X.%2X%n", &domain, &key, &end);
    if (n != 2 || (size_t) end != HAPStringGetNumBytes(ent->d_name)) {
        HAPLog(&logObject, "Skipping unexpected file in key-value store directory: %s", ent->d_name);
        return 0;
    }
    HAPAssert(sizeof(HAPPlatformKeyValueStoreDomain) == sizeof(uint8_t));
    if (domain > UINT8_MAX) {
        HAPLog(&logObject, "Skipping file with too large domain in key-value store directory: %s", ent->d_name);
        return 0;
    }
    HAPAssert(sizeof(HAPPlatformKeyValueStoreKey) == sizeof(uint8_t));
    if (key > UINT8_MAX) {
        HAPLog(&logObject, "Skipping file with too large key in key-value store directory: %s", ent->d_name);
        return 0;
    }

    // Check domain.
    if ((HAPPlatformKeyValueStoreDomain) domain != arguments->domain) {
        return 0;
    }

    // Invoke callback.
    err = arguments->body(
            arguments->context,
            arguments->keyValueStore,
            (HAPPlatformKeyValueStoreDomain) domain,
            (HAPPlatformKeyValueStoreKey) key,
            cont);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return -1;
    }
    return 0;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStoreEnumerate(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreEnumerateCallback callback,
        void* _Nullable context) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(keyValueStore->rootDirectory);
    HAPPrecondition(callback);

    int e =
            enumdir(keyValueStore->rootDirectory,
                    EnumdirCallback,
                    &(EnumdirCallbackContext) {
                            .keyValueStore = keyValueStore,
                            .domain = domain,
                            .body = callback,
                            .context = context,
                    });
    if (e) {
        HAPAssert(e == -1);
        return kHAPError_Unknown;
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPError PurgeDomainEnumerateCallback(
        void* _Nullable context HAP_UNUSED,
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        bool* shouldContinue) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(keyValueStore->rootDirectory);
    HAPPrecondition(shouldContinue);

    HAPError err;

    err = HAPPlatformKeyValueStoreRemove(keyValueStore, domain, key);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformKeyValueStorePurgeDomain(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(keyValueStore->rootDirectory);

    HAPError err;

    err = HAPPlatformKeyValueStoreEnumerate(keyValueStore, domain, PurgeDomainEnumerateCallback, NULL);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}
