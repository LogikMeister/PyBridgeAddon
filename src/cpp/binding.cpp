#include <node_api.h>
#include <cassert>
#include "./node_function/node_function.hpp"
#include "./utils.hpp"

napi_value Init(napi_env env, napi_value exports) {
    napi_status status;

    napi_value fn_is_initialized;
    status = napi_create_function(env, nullptr, 0, IsInitialized, nullptr, &fn_is_initialized);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "isInitialized", fn_is_initialized);
    assert(status == napi_ok);

    napi_value fn_initialize_python;
    status = napi_create_function(env, nullptr, 0, InitializePython, nullptr, &fn_initialize_python);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "initializePython", fn_initialize_python);
    assert(status == napi_ok);

    napi_value fn_finalize_python;
    status = napi_create_function(env, nullptr, 0, FinalizePython, nullptr, &fn_finalize_python);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "finalizePython", fn_finalize_python);
    assert(status == napi_ok);

    napi_value fn_call_python_function_sync;
    status = napi_create_function(env, nullptr, 0, CallPythonFunctionSync, nullptr, &fn_call_python_function_sync);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "callPythonFunctionSync", fn_call_python_function_sync);
    assert(status == napi_ok);

    napi_value fn_call_python_function_async;
    status = napi_create_function(env, nullptr, 0, CallPythonFunctionAsync, nullptr, &fn_call_python_function_async);
    assert(status == napi_ok);
    status = napi_set_named_property(env, exports, "callPythonFunctionAsync", fn_call_python_function_async);
    assert(status == napi_ok);

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)