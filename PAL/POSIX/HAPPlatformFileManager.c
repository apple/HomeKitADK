// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <wordexp.h>

#include "HAPPlatform+Init.h"
#include "HAPPlatformFileManager.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "FileManager" };

HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerCreateDirectory(const char* dirPath) {
    HAPPrecondition(dirPath);

    HAPError err;

    char path[PATH_MAX];

    // Duplicate string, as each path component needs to be modified to be NULL-terminated.
    // Duplicate is necessary, as input may reside in read-only memory.
    err = HAPStringWithFormat(path, sizeof path, "%s", dirPath);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLogError(&logObject, "Directory path too long: %s", dirPath);
        return kHAPError_Unknown;
    }

    // Create parent directories.
    for (char *start = path, *end = strchr(start, '/'); end; start = end + 1, end = strchr(start, '/')) {
        if (start == end) {
            // Root, or double separator.
            continue;
        }

        // Replace separator with \0 temporarily. Create directory. Restore back.
        *end = '\0';
        int e = mkdir(path, S_IRWXU);
        *end = '/';
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            if (_errno == EEXIST) {
                continue;
            }
            *end = '\0';
            HAPLogError(&logObject, "mkdir %s failed: %d.", path, _errno);
            *end = '/';
            return kHAPError_Unknown;
        }
    }

    // Create directory.
    int e = mkdir(path, S_IRWXU);
    if (e) {
        int _errno = errno;
        HAPAssert(e == -1);
        if (_errno != EEXIST) {
            HAPLogError(&logObject, "mkdir %s failed: %d.", path, _errno);
            return kHAPError_Unknown;
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerWriteFile(const char* filePath, const void* _Nullable bytes, size_t numBytes)
        HAP_DIAGNOSE_ERROR(!bytes && numBytes, "empty buffer cannot have a length") {
    HAPPrecondition(filePath);
    HAPAssert(bytes || numBytes); // bytes ==> numBytes > 0.

    HAPError err;

    char targetDirPath[PATH_MAX];
    err = HAPStringWithFormat(targetDirPath, sizeof targetDirPath, "%s", filePath);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLogError(&logObject, "Not enough resources to copy string: %s", filePath);
        return kHAPError_Unknown;
    }

    // Get split relative file path and dir path
    const char* relativeFilePath = filePath;
    {
        const size_t filePathLength = HAPStringGetNumBytes(filePath);
        HAPPrecondition(filePathLength);

        size_t dirPathLength = 0;
        for (size_t i = 0; i < filePathLength; ++i) {
            if (filePath[i] == '/') {
                dirPathLength = i;
                relativeFilePath = &filePath[i + 1];
            }
        }
        HAPAssert(dirPathLength < filePathLength);
        targetDirPath[dirPathLength] = '\0';

        HAPAssert(dirPathLength == HAPStringGetNumBytes(targetDirPath));
        HAPAssert(dirPathLength + HAPStringGetNumBytes(relativeFilePath) + 1 == filePathLength);
    }

    // Create directory.
    err = HAPPlatformFileManagerCreateDirectory(targetDirPath);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPLogError(&logObject, "Create directory %s failed.", targetDirPath);
        return err;
    }

    // Open the target directory.
    DIR* targetDirRef = opendir(targetDirPath);
    if (!targetDirRef) {
        int _errno = errno;
        HAPLogError(&logObject, "opendir %s failed: %d.", targetDirPath, _errno);
        return kHAPError_Unknown;
    }
    int targetDirFD = dirfd(targetDirRef);
    if (targetDirFD < 0) {
        int _errno = errno;
        HAPAssert(targetDirFD == -1);
        HAPLogError(&logObject, "dirfd %s failed: %d.", targetDirPath, _errno);
        HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
        return kHAPError_Unknown;
    }

    // Create the filename of the temporary file.
    char tmpPath[PATH_MAX];
    err = HAPStringWithFormat(tmpPath, sizeof tmpPath, "%s-tmp", relativeFilePath);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLogError(&logObject, "Not enough resources to get path: %s-tmp", relativeFilePath);
        HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
        return kHAPError_Unknown;
    }

    // Open the tempfile
    int tmpPathFD;
    do {
        tmpPathFD = openat(targetDirFD, tmpPath, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR);
    } while (tmpPathFD == -1 && errno == EINTR);
    if (tmpPathFD < 0) {
        int _errno = errno;
        HAPAssert(tmpPathFD == -1);
        HAPLogError(&logObject, "open %s in %s failed: %d.", tmpPath, targetDirPath, _errno);
        HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
        return kHAPError_Unknown;
    }

    // Write to temporary file.
    if (bytes) {
        size_t o = 0;
        while (o < numBytes) {
            size_t c = numBytes - o;
            if (c > SSIZE_MAX) {
                c = SSIZE_MAX;
            }

            ssize_t n;
            do {
                n = write(tmpPathFD, &((const uint8_t*) bytes)[o], c);
            } while (n == -1 && errno == EINTR);
            if (n < 0) {
                int _errno = errno;
                HAPAssert(n == -1);
                HAPLogError(&logObject, "write to temporary file %s failed: %d.", tmpPath, _errno);
                break;
            }
            if (n == 0) {
                HAPLogError(&logObject, "write to temporary file %s returned EOF.", tmpPath);
                break;
            }

            HAPAssert((size_t) n <= c);
            o += (size_t) n;
        }
        if (o != numBytes) {
            (void) close(tmpPathFD);
            int e = remove(tmpPath);
            if (e) {
                int _errno = errno;
                HAPAssert(e == -1);
                HAPLogError(&logObject, "remove of temporary file %s failed: %d.", tmpPath, _errno);
                HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
                return kHAPError_Unknown;
            }
            HAPLogError(&logObject, "Error writing temporary file %s in %s.", tmpPath, targetDirPath);
            HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
            return kHAPError_Unknown;
        }
    }

    // Try to synchronize and close the temporary file.
    {
        int e;
        do {
            e = fsync(tmpPathFD);
        } while (e == -1 && errno == EINTR);
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            HAPLogError(&logObject, "fsync of temporary file %s failed: %d.", tmpPath, _errno);
        }
        (void) close(tmpPathFD);
    }

    // Fsync dir
    {
        int e;
        do {
            e = fsync(targetDirFD);
        } while (e == -1 && errno == EINTR);
        if (e < 0) {
            int _errno = errno;
            HAPAssert(e == -1);
            HAPLogError(&logObject, "fsync of the target directory %s failed: %d", targetDirPath, _errno);
            HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
            return kHAPError_Unknown;
        }
    }

    // Rename file
    {
        int e = renameat(targetDirFD, tmpPath, targetDirFD, relativeFilePath);
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            HAPLogError(&logObject, "rename of temporary file %s to %s failed: %d.", tmpPath, filePath, _errno);
            HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
            return kHAPError_Unknown;
        }
    }

    // Fsync dir
    {
        int e;
        do {
            e = fsync(targetDirFD);
        } while (e == -1 && errno == EINTR);
        if (e < 0) {
            int _errno = errno;
            HAPAssert(e == -1);
            HAPLogError(&logObject, "fsync of the target directory %s failed: %d", targetDirPath, _errno);
            HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
            return kHAPError_Unknown;
        }
    }

    HAPPlatformFileManagerCloseDirFreeSafe(targetDirRef);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerReadFile(
        const char* filePath,
        void* _Nullable bytes,
        size_t maxBytes,
        size_t* _Nullable numBytes,
        bool* valid) {
    HAPPrecondition(filePath);
    HAPPrecondition(!maxBytes || bytes);
    HAPPrecondition((bytes == NULL) == (numBytes == NULL));
    HAPPrecondition(valid);

    *valid = false;

    int fd;
    do {
        fd = open(filePath, O_RDONLY);
    } while (fd == -1 && errno == EINTR);

    if (fd < 0) {
        int _errno = errno;
        if (_errno == ENOENT) {
            return kHAPError_None;
        }
        HAPAssert(fd == -1);
        HAPLogError(&logObject, "open %s failed: %d.", filePath, _errno);
        return kHAPError_Unknown;
    }
    *valid = true;

    if (bytes) {
        HAPAssert(numBytes);

        size_t o = 0;
        while (o < maxBytes) {
            size_t c = maxBytes - o;
            if (c > SSIZE_MAX) {
                c = SSIZE_MAX;
            }

            ssize_t n;
            do {
                n = read(fd, &((uint8_t*) bytes)[o], c);
            } while (n == -1 && errno == EINTR);
            if (n < 0) {
                int _errno = errno;
                HAPAssert(n == -1);
                HAPLogError(&logObject, "read %s failed: %d.", filePath, _errno);
                (void) close(fd);
                return kHAPError_Unknown;
            }

            HAPAssert((size_t) n <= c);
            o += (size_t) n;

            if (n == 0) {
                break;
            }
        }
        *numBytes = o;
    }
    (void) close(fd);

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerRemoveFile(const char* filePath) {
    HAPPrecondition(filePath);

    HAPError err;

    // Test for a regular file or a symbolic link.
    {
        struct stat statBuffer;
        int e = stat(filePath, &statBuffer);
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            if (_errno == ENOENT) {
                // File does not exist.
                return kHAPError_None;
            }
            HAPLogError(&logObject, "stat file %s failed: %d.", filePath, _errno);
            HAPFatalError();
        }
        if (!S_ISREG(statBuffer.st_mode) && !S_ISLNK(statBuffer.st_mode)) {
            HAPLogError(&logObject, "file %s is neither a regular file nor a symbolic link.", filePath);
            HAPFatalError();
        }
    }

    // Remove file.
    {
        int e = unlink(filePath);
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            if (_errno == ENOENT) {
                // File does not exist.
                return kHAPError_None;
            }
            HAPLogError(&logObject, "unlink file %s failed: %d.", filePath, _errno);
            return kHAPError_Unknown;
        }
    }

    // Try to synchronize the directory containing the removed file.
    {
        char targetDirPath[PATH_MAX];
        err = HAPStringWithFormat(targetDirPath, sizeof targetDirPath, "%s", filePath);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPLogError(&logObject, "Not enough resources to copy string: %s", filePath);
            return kHAPError_None;
        }
        char* lastSeparator = NULL;
        for (char *start = targetDirPath, *end = strchr(start, '/'); end; start = end + 1, end = strchr(start, '/')) {
            lastSeparator = end;
        }
        if (!lastSeparator) {
            // No directory in path - treat as current working directory.
            HAPAssert(sizeof targetDirPath > 2);
            targetDirPath[0] = '.';
            targetDirPath[1] = '\0';
        } else {
            // Replace final separator with '\0' to cut off the file name.
            HAPAssert(lastSeparator < &targetDirPath[sizeof targetDirPath - 1]);
            HAPAssert(*lastSeparator == '/');
            HAPAssert(*(lastSeparator + 1));
            *lastSeparator = '\0';
        }

        int targetDirFd;
        do {
            targetDirFd = open(targetDirPath, O_RDONLY);
        } while (targetDirFd == -1 && errno == EINTR);
        if (targetDirFd < 0) {
            int _errno = errno;
            HAPAssert(targetDirFd == -1);
            HAPLogError(&logObject, "open target directory %s failed: %d.", targetDirPath, _errno);
            return kHAPError_None;
        }

        int e;
        do {
            e = fsync(targetDirFd);
        } while (e == -1 && errno == EINTR);
        if (e) {
            int _errno = errno;
            HAPAssert(e == -1);
            HAPLogError(&logObject, "fsync of target directory file %s failed: %d.", targetDirPath, _errno);
        }
        (void) close(targetDirFd);
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerNormalizePath(const char* path, char* bytes, size_t maxBytes) {
    HAPPrecondition(path);
    HAPPrecondition(bytes);

    HAPError err;

    HAPLogDebug(&logObject, "%s: Expanding '%s'", __func__, path);
    int e;

    // Use wordexp to expand the path.
    wordexp_t wordexpResult;
    e = wordexp(path, &wordexpResult, WRDE_UNDEF | WRDE_NOCMD);
    if (e != 0) {
        HAPLogError(&logObject, "%s: wordexp expansion failed: %d.", __func__, e);
        err = kHAPError_Unknown;
        goto exit;
    }

    const char* expandedPath = wordexpResult.we_wordv[0];
    size_t len = HAPStringGetNumBytes(expandedPath);
    if (len >= maxBytes) {
        HAPLogError(&logObject, "%s: Target buffer too small (got: %zu, has: %zu).", __func__, maxBytes, len);
        err = kHAPError_OutOfResources;
        goto exit;
    }
    HAPRawBufferCopyBytes(bytes, expandedPath, len + 1);
    err = kHAPError_None;

    wordfree(&wordexpResult);
exit:
    return err;
}
