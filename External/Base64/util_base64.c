// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

#include "util_base64.h"

// See https://tools.ietf.org/html/rfc4648

// GROUP and INDEX are in range 0 ... 7F.
// The range is shifted so that if GROUP >= INDEX, then shifted GROUP >= 0x80.
// The 0x80 bit is then extracted to get the result of this comparison in constant time.
#define GTE_MASK(GROUP, INDEX) ((int8_t)((GROUP) + 0x80 - (INDEX)) >> 7)

void util_base64_encode(
        const void* p_data,
        size_t data_len,
        char* p_encoded,
        size_t encoded_capacity,
        size_t* p_encoded_len) {
    HAPPrecondition(p_data);
    HAPPrecondition(p_encoded);
    HAPPrecondition(p_encoded_len);

    // Check buffer size.
    // Per 3-byte group (rounded up), a 4-byte group is produced.
    *p_encoded_len = (data_len + 2) / 3 * 4;
    HAPPrecondition(encoded_capacity >= *p_encoded_len);

    // Each block is processed individually.
    // Since the output is larger than the input, we need to ensure that the input is not overwritten
    // when the same pointer is passed as both input and output.
    // Since the next input chunk is always read completely before writing an output chunk,
    // it is safe to overwrite the current input chunk.
    //
    // Worst case example with just enough buffer size for a successful encoding:
    //                   Input: 111222333
    //         Buffer at start: 111222333###
    //   Move to end of buffer: ###111222333
    //     Process first block: AAAA11222333 - 1 gets overwritten, but is read before the overwrite.
    //    Process second block: AAAABBBB2333 - 2 gets overwritten, but is read before the overwrite.
    //     Process third block: AAAABBBBCCCC
    //
    // Padding doesn't have an influence on this behaviour.

    // Move input to end of output buffer.
    uint8_t* p_data_new = (uint8_t*) p_encoded + encoded_capacity - data_len;
    HAPRawBufferCopyBytes(p_data_new, p_data, data_len);

    const uint8_t* p_in = p_data_new;
    char* p_out = p_encoded;

    // Encode data.
    while (data_len) {
        // Concatenate 24-bit group.
        uint32_t group24 = 0;
        int padding = 0;
        for (int i = 0; i < 3; i++) {
            group24 <<= 8;
            if (data_len) {
                group24 |= *(p_in++);
                data_len--;
            } else {
                padding++;
            }
        }

        // Split into 6-bit groups.
        for (int i = 0; i < 4 - padding; i++) {
            // group24: xxxxxxxx xxxxxxxx xxxxxxxx
            //  group6: xxxxxx xxxxxx xxxxxx xxxxxx
            uint8_t group6 = (uint8_t)((group24 >> 18) & 0x3F);
            group24 <<= 6;

            // Constant time transformation to avoid leaking secret data through side channels.

            //    Index: 0                          26                         52         62 63
            // Alphabet: ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789 +  /
            //    ASCII: 65                      90 97                     122 48      57 43 47

            // Transform alphabet index into ASCII.
            int8_t offset = 0;
            offset += GTE_MASK(group6, 63) & ('/' - '+' - 1); // Skip gap between + and /.
            offset += GTE_MASK(group6, 62) & ('+' - '9' - 1); // Skip gap between 9 and +.
            offset += GTE_MASK(group6, 52) & ('0' - 'z' - 1); // Skip gap between z and 0.
            offset += GTE_MASK(group6, 26) & ('a' - 'Z' - 1); // Skip gap between Z and a.
            offset += 'A';                                    // Shift base.
            group6 += offset;

            *p_out++ = (char) group6;
        }

        // Add padding.
        for (int i = 0; i < padding; i++) {
            *p_out++ = '=';
        }
    }

    HAPAssert((size_t)(p_out - p_encoded) == *p_encoded_len);
}

HAP_RESULT_USE_CHECK
HAPError util_base64_decode(
        const char* p_encoded,
        size_t encoded_len,
        void* p_data,
        size_t data_capacity,
        size_t* p_data_len) {
    HAPPrecondition(p_encoded);
    HAPPrecondition(p_data);
    HAPPrecondition(p_data_len);

    const char* p_in = p_encoded;
    uint8_t* p_out = p_data;

    // Each block is processed individually.
    // Since output is smaller than input, it is safe to pass the same pointer as both input and output.

    while (encoded_len) {
        // Concatenate 6-bit groups into 24-bit group.
        uint32_t group24 = 0;
        int padding = 0;
        for (int i = 0; i < 4; i++) {
            group24 <<= 6;

            if (!encoded_len) {
                HAPLog(&kHAPLog_Default, "Incomplete 24-bit group.");
                return kHAPError_InvalidData;
            }
            encoded_len--;
            uint8_t group6 = (uint8_t) *p_in++;

            // Handle padding.
            if (group6 == '=') {
                group6 = 0;
                padding++;
                if (padding > 2) {
                    HAPLog(&kHAPLog_Default, "More than two padding characters.");
                    return kHAPError_InvalidData;
                }
            } else if (padding) {
                // Non-padding after padding.
                HAPLog(&kHAPLog_Default, "Non-padding after padding.");
                return kHAPError_InvalidData;
            } else {
                // Constant time transformation to avoid leaking secret data through side channels.

                //    ASCII: 43 47 48      57 65                      90 97                     122
                // Alphabet: +  /  0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz
                // Squashed: 0  1  2       11 12                      37 38                      63
                //    Index: 62 63 52      61 0                       25 26                      51

                // Transform into alphabet index. Map illegal characters to values >= 64.

                if (group6 & 0x80) {
                    // illegal character
                    HAPLog(&kHAPLog_Default, "Illegal character (before transform): 0x%02x", group6);
                    return kHAPError_InvalidData;
                }
                uint8_t offset = 64;                                             // 0     -> 64
                offset += GTE_MASK(group6, '+') & (62 - 64 - '+');               // '+'   -> 62
                offset += GTE_MASK(group6, '+' + 1) & (64 - 62 - 1);             // '+'+1 -> 64
                offset += GTE_MASK(group6, '/') & (63 - 64 - '/' + '+' + 1);     // '/'   -> 63
                offset += GTE_MASK(group6, '0') & (52 - 63 - '0' + '/');         // '0'   -> 52
                offset += GTE_MASK(group6, '9' + 1) & (64 - 52 - '9' - 1 + '0'); // '9'+1 -> 64
                offset += GTE_MASK(group6, 'A') & (0 - 64 - 'A' + '9' + 1);      // 'A'   ->  0
                offset += GTE_MASK(group6, 'Z' + 1) & (64 - 0 - 'Z' - 1 + 'A');  // 'Z'+1 -> 64
                offset += GTE_MASK(group6, 'a') & (26 - 64 - 'a' + 'Z' + 1);     // 'a'   -> 26
                offset += GTE_MASK(group6, 'z' + 1) & (64 - 26 - 'z' - 1 + 'a'); // 'z'+1 -> 64
                group6 += offset;
                if (group6 & 0xC0) {
                    // illegal character
                    HAPLog(&kHAPLog_Default, "Illegal character (after transform): 0x%02x", group6 - offset);
                    return kHAPError_InvalidData;
                }
            }

            // Add to group.
            //  group6: xxxxxx xxxxxx xxxxxx xxxxxx
            // group24: xxxxxxxx xxxxxxxx xxxxxxxx
            group24 |= group6;
        }

        // Write 24-bit group.
        for (int i = 0; i < 3 - padding; i++) {
            if (data_capacity < 1) {
                return kHAPError_OutOfResources;
            }
            data_capacity -= 1;
            *p_out++ = (uint8_t)((group24 >> 16) & 0xFF);
            group24 <<= 8;
        }

        // If there was padding, this must be the last group.
        if (padding && encoded_len) {
            HAPLog(&kHAPLog_Default, "Additional group after a group that contained padding.");
            return kHAPError_InvalidData;
        }
    }

    *p_data_len = (size_t)(p_out - (uint8_t*) p_data);
    return kHAPError_None;
}
