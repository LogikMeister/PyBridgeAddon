#ifndef _NODE_CALL_PYTHON_FUNCTION_ASYNC_H_
#define _NODE_CALL_PYTHON_FUNCTION_ASYNC_H_

#include <node_api.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "../gil.hpp"
#include "../transform.hpp"

namespace py = pybind11;

struct WorkData {
    napi_async_work work;
    napi_deferred deferred;
    std::string module_name;
    std::string function_name;
    Variant function_args;
    Variant result;
    std::string error_str;
};

inline void ExecuteWork(napi_env env, void* data) {
    WorkData* work_data = static_cast<WorkData*>(data);
    PythonGILState::getInstance()->ensureGIL();
    try {
        // Transform the Variant to py::object
        py::object arguments = transform_variant_2_p(work_data->function_args);
        py::object module = py::module::import(work_data->module_name.c_str());
        py::object function = module.attr(work_data->function_name.c_str());
        py::object result_obj;
        if (py::isinstance<py::dict>(arguments)) {
            result_obj = function.attr("__call__")(*py::tuple(), **arguments);
        } else {
            result_obj = function(arguments);
        }
        // Transform py::object to Variant
        Variant result;
        transform_p_2_variant(result_obj, result);
        work_data->result = std::move(result);
    } catch (py::error_already_set & e) {
        // a Python exception occurred
        work_data->error_str = std::string("Python exception: ") + e.what();
    } catch (std::exception & e) {
        // a C++ exception occurred
        work_data->error_str = std::string("C++ exception: ") + e.what();
    } catch (...) {
        // an unknown exception occurred
        work_data->error_str = std::string("Unknown exception occurred.");
    }
    PythonGILState::getInstance()->releaseGIL();
}

inline void CompleteWork(napi_env env, napi_status status, void* data) {
    WorkData* work_data = static_cast<WorkData*>(data);
    napi_value result_value;
    if (work_data->error_str.empty()) {
        // Transform Variant to napi_value
        try {
            result_value = transform_variant_2_n(env, work_data->result);
            napi_resolve_deferred(env, work_data->deferred, result_value);
        } catch (std::exception & e) {
            // a C++ exception occurred
            napi_create_string_utf8(env, (std::string("C++ exception: ") + e.what()).c_str(), NAPI_AUTO_LENGTH, &result_value);
            napi_reject_deferred(env, work_data->deferred, result_value);
        } catch (...) {
            // an unknown exception occurred
            napi_create_string_utf8(env, "Unknown exception", NAPI_AUTO_LENGTH, &result_value);
        }
    } else {
        napi_create_string_utf8(env, work_data->error_str.c_str(), NAPI_AUTO_LENGTH, &result_value);
        napi_reject_deferred(env, work_data->deferred, result_value);
    }
    napi_delete_async_work(env, work_data->work);
    delete work_data;
}

// Call Python function
inline napi_value CallPythonFunctionAsync(napi_env env, napi_callback_info info) {
    
    napi_status status;
    size_t argc = 3;
    napi_value argv[3];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // Check parameters
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }
    
    napi_valuetype arg_types[3];
    for (int i=0; i<3; i++) {
        napi_typeof(env, argv[i], &arg_types[i]);
    }
    if (arg_types[0] != napi_string || arg_types[1] != napi_string || arg_types[2] != napi_string) {
        napi_throw_type_error(env, nullptr, "Wrong arguments");
    }

    // Get parameters
    size_t result;
    char module_name[1024];
    napi_get_value_string_utf8(env, argv[0], module_name, sizeof(module_name), &result);
    char function_name[1024];
    napi_get_value_string_utf8(env, argv[1], function_name, sizeof(function_name), &result);
    // Transform the napi_value to Variant
    Variant function_args;
    transform_n_2_variant(env, argv[2], function_args);

    // Call function
    // Create promise
    napi_value promise;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);
    // Create work
    napi_async_work work;
    WorkData* workData = new WorkData{ work, deferred, module_name, function_name, function_args };
    napi_value resource_name;
    napi_create_string_utf8(env, "call_python_function", NAPI_AUTO_LENGTH, &resource_name);
    napi_create_async_work(env, NULL, resource_name, ExecuteWork, CompleteWork, workData, &work);
    workData->work = work;
    napi_queue_async_work(env, workData->work);

    return promise;
}
#endif
