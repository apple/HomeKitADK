// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_FILE_MANAGER_H
#define HAP_PLATFORM_FILE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <dirent.h>
#include <errno.h>

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Creates a directory and all parent directories that don't exist.
 *
 * @param      dirPath              Path to the directory to be created.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If the file access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerCreateDirectory(const char* dirPath);

/**
 * Writes a file atomically.
 *
 * @param      filePath             Path to the file to be created.
 * @param      bytes                Buffer with the content of the file, if exists. numBytes != 0 implies bytes.
 * @param      numBytes             Effective length of the bytes buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If the file access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerWriteFile(const char* filePath, const void* _Nullable bytes, size_t numBytes);

/**
 * Reads a file.
 *
 * @param      filePath             Path to the file to be read.
 * @param[out] bytes                Buffer for the content of the file, if exists.
 * @param      maxBytes             Capacity of the bytes buffer.
 * @param[out] numBytes             Effective length of the bytes buffer, if exists.
 * @param[out] valid                Whether the file path exists and could be read.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If the file access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerReadFile(
        const char* filePath,
        void* _Nullable bytes,
        size_t maxBytes,
        size_t* _Nullable numBytes,
        bool* valid);

/**
 * Removes a file.
 *
 * @param      filePath             Path to the file to be removed.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If the file removal failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerRemoveFile(const char* filePath);

/**
 * Normalizes a path.
 *
 * @param      path                 Path to normalize.
 * @param[out] bytes                Buffer to fill with the normalized string. Will be NULL-terminated.
 * @param      maxBytes             Maximum number of bytes that may be filled into the buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer was not large enough.
 * @return kHAPError_Unknown        On error.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformFileManagerNormalizePath(const char* path, char* bytes, size_t maxBytes);

/**
 * Closes a directory with a given path and sets dir to NULL.
 *
 * @param      dir                  Directory to close (type: DIR *).
 */
#define HAPPlatformFileManagerCloseDirFreeSafe(dir) \
    do { \
        HAPAssert(dir); \
        int _e; \
        do { \
            _e = closedir(dir); \
        } while (_e == -1 && errno == EINTR); \
        if (_e) { \
            HAPAssert(_e == -1); \
        } \
        (dir) = NULL; \
    } while (0)

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
