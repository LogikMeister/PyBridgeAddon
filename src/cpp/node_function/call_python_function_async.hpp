#ifndef _NODE_CALL_PYTHON_FUNCTION_ASYNC_H_
#define _NODE_CALL_PYTHON_FUNCTION_ASYNC_H_

#include <node_api.h>
#include <uv.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "../gil.hpp"
#include "../thread_pool/singleton.hpp"
#include "../transform.hpp"

namespace py = pybind11;

struct WorkData {
    std::string module_name;
    std::string function_name;
    Variant function_args;
    Variant result;
    std::string error_str;
};

struct CompleteData {
    uv_async_t async;
    napi_env env;
    napi_deferred deferred;
    WorkData* work_data;
};

// Call Python function
inline napi_value CallPythonFunctionAsync(napi_env env, napi_callback_info info) {
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
    if (
        arg_types[0] != napi_string ||
        arg_types[1] != napi_string ||
        (arg_types[2] != napi_object && arg_types[2] != napi_null && arg_types[2] != napi_undefined)
    ) {
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
    WorkData* work_data = new WorkData { module_name, function_name, function_args };
    CompleteData* complete_data = new CompleteData;
    complete_data->env = env;
    complete_data->deferred = deferred;
    complete_data->work_data = work_data;

    // Listening work completed event
    uv_loop_t* loop = uv_default_loop();
    uv_async_init(loop, &complete_data->async, [](uv_async_t* handle) {
        CompleteData* complete_data = (CompleteData*)handle->data;
        WorkData* work_data = complete_data->work_data;
        // Create handle scope
        napi_handle_scope scope;
        napi_open_handle_scope(complete_data->env, &scope);

        napi_value result_value;
        if (work_data->error_str.empty()) {
            // Transform Variant to napi_value
            try {
                result_value = transform_variant_2_n(complete_data->env, work_data->result);
                napi_resolve_deferred(complete_data->env, complete_data->deferred, result_value);
            } catch (std::exception & e) {
                // a C++ exception occurred
                napi_create_string_utf8(complete_data->env, (std::string("C++ exception: ") + e.what()).c_str(), NAPI_AUTO_LENGTH, &result_value);
                napi_reject_deferred(complete_data->env, complete_data->deferred, result_value);
            } catch (...) {
                // an unknown exception occurred
                napi_create_string_utf8(complete_data->env, "Unknown exception", NAPI_AUTO_LENGTH, &result_value);
            }
        } else {
            napi_create_string_utf8(complete_data->env, work_data->error_str.c_str(), NAPI_AUTO_LENGTH, &result_value);
            napi_reject_deferred(complete_data->env, complete_data->deferred, result_value);
        }

        // Close handle scope
        napi_close_handle_scope(complete_data->env, scope);
        delete work_data;
        uv_close((uv_handle_t*)handle, [](uv_handle_t* handle) {
            delete (CompleteData*)handle->data;
        });
    });

    // Submit work to thread pool
    GlobalThreadPool::getInstance()->submit([complete_data] {
        WorkData* work_data = complete_data->work_data;
        PythonGILState::getInstance()->ensureGIL();
        try {
            // Transform Variant to py::object
            py::object arguments = transform_variant_2_p(work_data->function_args);
            // Call python function
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
        complete_data->async.data = complete_data;
        uv_async_send(&complete_data->async);
    });

    return promise;
}
#endif
