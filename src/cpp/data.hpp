#ifndef _DATA_H_
#define _DATA_H_

#include <node_api.h>
#include <uv.h>
#include <queue>
#include <mutex>
#include "./variant.hpp"

struct WorkData {
    std::string module_name;
    std::string function_name;
    Variant function_args;
    Variant result;
    std::string error_str;
};

struct CompleteData {
    uv_async_t async;
    uv_async_t async_event;
    napi_env env;
    napi_deferred deferred;
    std::unordered_map<std::string, napi_ref> event_callbacks;
    std::queue<std::pair<std::string, Variant>> event_queue;
    std::mutex event_mutex;
    WorkData* work_data;
};

#endif