// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef UTIL_JSON_READER_H
#define UTIL_JSON_READER_H

#include "HAPPlatform.h"

#define util_JSON_READER_STATE_READING_WHITESPACE 0

#define util_JSON_READER_STATE_BEGINNING_OBJECT 1
#define util_JSON_READER_STATE_COMPLETED_OBJECT 2

#define util_JSON_READER_STATE_BEGINNING_ARRAY 3
#define util_JSON_READER_STATE_COMPLETED_ARRAY 4

#define util_JSON_READER_STATE_BEGINNING_NUMBER 5
#define util_JSON_READER_STATE_READING_NUMBER   6
#define util_JSON_READER_STATE_COMPLETED_NUMBER 7

#define util_JSON_READER_STATE_BEGINNING_STRING 8
#define util_JSON_READER_STATE_READING_STRING   9
#define util_JSON_READER_STATE_COMPLETED_STRING 10

#define util_JSON_READER_STATE_BEGINNING_FALSE 11
#define util_JSON_READER_STATE_READING_FALSE   12
#define util_JSON_READER_STATE_COMPLETED_FALSE 13

#define util_JSON_READER_STATE_BEGINNING_TRUE 14
#define util_JSON_READER_STATE_READING_TRUE   15
#define util_JSON_READER_STATE_COMPLETED_TRUE 16

#define util_JSON_READER_STATE_BEGINNING_NULL 17
#define util_JSON_READER_STATE_READING_NULL   18
#define util_JSON_READER_STATE_COMPLETED_NULL 19

#define util_JSON_READER_STATE_AFTER_NAME_SEPARATOR  20
#define util_JSON_READER_STATE_AFTER_VALUE_SEPARATOR 21

#define util_JSON_READER_STATE_ERROR 22

struct util_json_reader {
    int state;
    int substate;
};

void util_json_reader_init(struct util_json_reader* r);
HAP_RESULT_USE_CHECK
size_t util_json_reader_read(struct util_json_reader* r, const char* buffer, size_t length);

#endif
