// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef UTIL_HTTP_READER_H
#define UTIL_HTTP_READER_H

#include "HAPPlatform.h"

#define util_HTTP_READER_TYPE_REQUEST  0
#define util_HTTP_READER_TYPE_RESPONSE 1

#define util_HTTP_READER_STATE_EXPECTING_METHOD 0
#define util_HTTP_READER_STATE_READING_METHOD   1
#define util_HTTP_READER_STATE_COMPLETED_METHOD 2

#define util_HTTP_READER_STATE_EXPECTING_URI 3
#define util_HTTP_READER_STATE_READING_URI   4
#define util_HTTP_READER_STATE_COMPLETED_URI 5

#define util_HTTP_READER_STATE_EXPECTING_VERSION 6
#define util_HTTP_READER_STATE_READING_VERSION   7
#define util_HTTP_READER_STATE_COMPLETED_VERSION 8

#define util_HTTP_READER_STATE_EXPECTING_STATUS 9
#define util_HTTP_READER_STATE_READING_STATUS   10
#define util_HTTP_READER_STATE_COMPLETED_STATUS 11

#define util_HTTP_READER_STATE_EXPECTING_REASON 12
#define util_HTTP_READER_STATE_READING_REASON   13
#define util_HTTP_READER_STATE_COMPLETED_REASON 14

#define util_HTTP_READER_STATE_EXPECTING_HEADER_NAME 15
#define util_HTTP_READER_STATE_READING_HEADER_NAME   16
#define util_HTTP_READER_STATE_COMPLETED_HEADER_NAME 17

#define util_HTTP_READER_STATE_EXPECTING_HEADER_VALUE 18
#define util_HTTP_READER_STATE_READING_HEADER_VALUE   19
#define util_HTTP_READER_STATE_COMPLETED_HEADER_VALUE 20

#define util_HTTP_READER_STATE_ENDING_HEADER_LINE  21
#define util_HTTP_READER_STATE_ENDING_HEADER_LINES 22

#define util_HTTP_READER_STATE_DONE  23
#define util_HTTP_READER_STATE_ERROR 24

struct util_http_reader {
    int type;
    int state;
    int substate;
    int in_quoted_pair;
    int in_quoted_string;
    char* result_token;
    size_t result_length;
};

void util_http_reader_init(struct util_http_reader* r, int type);
HAP_RESULT_USE_CHECK
size_t util_http_reader_read(struct util_http_reader* r, char* buffer, size_t length);

#endif
