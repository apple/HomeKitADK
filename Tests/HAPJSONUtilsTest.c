// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

int main() {
    HAPError err;

    struct util_json_reader jsonReader;
    size_t jsonBytesSkipped;

    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "{}";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "{\"0\":0}";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "{\"0\":0,\"1\":1,\"2\":2}";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "[]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "[0]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "[0,1,2]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "[0]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "[\"a\"]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        static const char jsonBytes[] = "[false]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        const char jsonBytes[] = "[true]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        const char jsonBytes[] = "[null]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        const char jsonBytes[] =
                "{\"00\":{\"01\":{\"02\":{\"03\":{\"04\":{\"05\":{\"06\":{\"07\":"
                "{\"08\":{\"09\":{\"10\":{\"11\":{\"12\":{\"13\":{\"15\":{\"15\":"
                "{\"16\":{\"17\":{\"18\":{\"19\":{\"20\":{\"21\":{\"22\":{\"23\":"
                "{\"24\":{\"25\":{\"26\":{\"27\":{\"28\":{\"29\":{\"30\":{\"31\":"
                "{\"32\":{\"33\":{\"34\":{\"35\":{\"36\":{\"37\":{\"38\":{\"39\":"
                "{\"40\":{\"41\":{\"42\":{\"43\":{\"44\":{\"45\":{\"46\":{\"47\":"
                "{\"48\":{\"49\":{\"50\":{\"51\":{\"52\":{\"53\":{\"54\":{\"55\":"
                "{\"56\":{\"57\":{\"58\":{\"59\":{\"60\":{\"61\":{\"62\":{\"63\":"
                "0}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        const char jsonBytes[] =
                "{\"00\":{\"01\":{\"02\":{\"03\":{\"04\":{\"05\":{\"06\":{\"07\":"
                "{\"08\":{\"09\":{\"10\":{\"11\":{\"12\":{\"13\":{\"15\":{\"15\":"
                "{\"16\":{\"17\":{\"18\":{\"19\":{\"20\":{\"21\":{\"22\":{\"23\":"
                "{\"24\":{\"25\":{\"26\":{\"27\":{\"28\":{\"29\":{\"30\":{\"31\":"
                "{\"32\":{\"33\":{\"34\":{\"35\":{\"36\":{\"37\":{\"38\":{\"39\":"
                "{\"40\":{\"41\":{\"42\":{\"43\":{\"44\":{\"45\":{\"46\":{\"47\":"
                "{\"48\":{\"49\":{\"50\":{\"51\":{\"52\":{\"53\":{\"54\":{\"55\":"
                "{\"56\":{\"57\":{\"58\":{\"59\":{\"60\":{\"61\":{\"62\":{\"63\":"
                "}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_InvalidData);
    }
    {
        util_json_reader_init(&jsonReader);
        const char jsonBytes[] =
                "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
                "7,77,777"
                "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_None);
        HAPAssert(jsonBytesSkipped == sizeof jsonBytes - 1);
    }
    {
        util_json_reader_init(&jsonReader);

        const char jsonBytes[] =
                "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
                ",,"
                "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";

        err = HAPJSONUtilsSkipValue(&jsonReader, jsonBytes, sizeof jsonBytes - 1, &jsonBytesSkipped);
        HAPAssert(err == kHAPError_InvalidData);
    }

    return 0;
}
