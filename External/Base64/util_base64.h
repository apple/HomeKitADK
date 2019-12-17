// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef UTIL_BASE64_H
#define UTIL_BASE64_H

#include "HAPPlatform.h"

/**
 * Returns the number of bytes necessary to store the base64 encoded data for a given data length.
 * NULL terminator excluded.
 *
 * @param      data_len             Length of data before encoding.
 *
 * @return Number of bytes necessary to store the base64 encoded data for the given data length.
 */
#define util_base64_encoded_len(data_len) (((data_len) + 2) / 3 * 4)

/**
 * Base64 encodes data @p p_data with length @p data_len into @p p_encoded with capacity @p encoded_capacity.
 * Length of encoded data is put into @p p_encoded_len.
 *
 * @param      p_data               Data to encode.
 * @param      data_len             Length of @p p_data.
 * @param[out] p_encoded            Base64 encoded data.
 * @param      encoded_capacity     Capacity of @p p_encoded. Must be at least 4/3 * @p data_len.
 * @param[out] p_encoded_len        Effective length of @p p_encoded.
 */
void util_base64_encode(
        const void* p_data,
        size_t data_len,
        char* p_encoded,
        size_t encoded_capacity,
        size_t* p_encoded_len);

/**
 * Base64 decodes data @p p_encoded with length @p encoded_len into @p p_data with capacity @p data_capacity.
 * Length of decoded data is put into @p p_data_len.
 *
 * @param      p_encoded            Data to decode.
 * @param      encoded_len          Length of @p p_encoded.
 * @param[out] p_data               Base64 decoded data.
 * @param      data_capacity        Capacity of @p p_decoded. Maximum 3/4 * @p data_len is required for decoding.
 * @param[out] p_data_len           Effective length of @p data_capacity.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If @p p_encoded contains malformed data.
 * @return kHAPError_OutOfResources If @p data_capacity is not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError util_base64_decode(
        const char* p_encoded,
        size_t encoded_len,
        void* p_data,
        size_t data_capacity,
        size_t* p_data_len);

#endif
