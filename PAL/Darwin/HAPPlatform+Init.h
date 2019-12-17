// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_INIT_H
#define HAP_PLATFORM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Optional features set in Makefile.
 */
/**@{*/
#ifndef HAVE_DISPLAY
#define HAVE_DISPLAY 0
#endif

#ifndef HAVE_MFI_HW_AUTH
#define HAVE_MFI_HW_AUTH 0
#endif
/**@}*/

#include <stdlib.h>

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
