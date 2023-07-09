#ifndef _NODE_CALL_PYTHON_FUNCTION_ASYNC_H_
#define _NODE_CALL_PYTHON_FUNCTION_ASYNC_H_

#include <node_api.h>
#include <uv.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "../gil.hpp"
#include "../thread_pool/singleton.hpp"
#include "../transform.hpp"
#include "../data.hpp"
#include "../triggers.hpp"
#include <iostream>

namespace py = pybind11;

// Call Python function
inline napi_value CallPythonFunctionAsync(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value argv[4];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // Check parameters
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }
    
    napi_valuetype arg_types[4];
    for (int i=0; i<2; i++) {
        napi_typeof(env, argv[i], &arg_types[i]);
    }
    if (
        arg_types[0] != napi_string ||
        arg_types[1] != napi_string
    ) {
        napi_throw_type_error(env, nullptr, "Wrong arguments");
    }

    if (argc >= 4) {
        napi_typeof(env, argv[3], &arg_types[3]);
    }

    // Get parameters
    size_t result;
    char module_name[1024];
    napi_get_value_string_utf8(env, argv[0], module_name, sizeof(module_name), &result);
    char function_name[1024];
    napi_get_value_string_utf8(env, argv[1], function_name, sizeof(function_name), &result);

    // Transform the napi_value to Variant
    Variant function_args;
    if (argc >= 3) {
        napi_typeof(env, argv[2], &arg_types[2]);
        if (arg_types[2] != napi_object && arg_types[2] != napi_null && arg_types[2] != napi_undefined) {
            napi_throw_type_error(env, nullptr, "Wrong arguments");
            return nullptr;
        }
        transform_n_2_variant(env, argv[2], function_args);
    } else {
        function_args = Variant();
    }
    

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

    // Get event callbacks
    if (argc >= 4 && arg_types[3] == napi_object) {
        napi_value event_callbacks_obj = argv[3];
        napi_value obj_keys;
        napi_get_property_names(env, event_callbacks_obj, &obj_keys);

        uint32_t key_len;
        napi_get_array_length(env, obj_keys, &key_len);

        for (size_t i=0; i<key_len; i++) {
            napi_value obj_key, fn_callback;
            napi_get_element(env, obj_keys, i, &obj_key);
            napi_get_property(env, event_callbacks_obj, obj_key, &fn_callback);

            napi_valuetype fn_callback_type;
            napi_typeof(env, fn_callback, &fn_callback_type);
            if (fn_callback_type == napi_function) {
                size_t obj_key_len;
                napi_get_value_string_utf8(env, obj_key, nullptr, 0, &obj_key_len);
                std::string obj_key_str(obj_key_len, '\0');
                napi_get_value_string_utf8(env, obj_key, &obj_key_str[0], obj_key_len + 1, &obj_key_len);
                // Create callback reference
                napi_ref fn_callback_ref;
                napi_create_reference(env, fn_callback, 1, &fn_callback_ref);
                complete_data->event_callbacks[obj_key_str] = fn_callback_ref;
            }
        }
    }

    // Get libuv loop;
    uv_loop_t* loop = uv_default_loop();

    // Listening for events
    uv_async_init(loop, &complete_data->async_event, [](uv_async_t* handle) {
        CompleteData* complete_data = (CompleteData*)(handle->data);
        // Create handle scope
        napi_handle_scope scope;
        napi_open_handle_scope(complete_data->env, &scope);

        while(true) {
            // Get event from queue
            std::pair<std::string, Variant> event;
            {
                // Lock event queue
                std::lock_guard<std::mutex> lock(complete_data->event_mutex);
                if (complete_data->event_queue.empty()) {
                    break;
                }
                event = std::move(complete_data->event_queue.front());
                complete_data->event_queue.pop();
            }
            // Call event callback
            auto it = complete_data->event_callbacks.find(event.first);
            if (it != complete_data->event_callbacks.end()) {
                // Transform the Variant to napi_value
                napi_value event_value = transform_variant_2_n(complete_data->env, event.second);
                // Call callback
                napi_value global;
                napi_get_global(complete_data->env, &global);
                napi_value fn_callback;
                napi_get_reference_value(complete_data->env, it->second, &fn_callback);
                napi_value fn_callback_result;
                napi_call_function(complete_data->env, global, fn_callback, 1, &event_value, &fn_callback_result);
            } else {
                std::string& module_name = complete_data->work_data->module_name;
                std::string& function_name = complete_data->work_data->function_name;
                std::cout << module_name << "." << function_name << " event callback not found: " << event.first << std::endl;
            }
        }

        // Close handle scope
        napi_close_handle_scope(complete_data->env, scope);
    });

    // Listening work completed event
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

        // Delete work data
        delete work_data;
        // Close async event
        uv_close((uv_handle_t*)&complete_data->async_event, [](uv_handle_t* handle) {
            // Empty
        });
        // Close async
        uv_close((uv_handle_t*)handle, [](uv_handle_t* handle) {
            delete (CompleteData*)handle->data;
        });
    });

    // Submit work to thread pool
    GlobalThreadPool::getInstance()->submit([complete_data] {
        WorkData* work_data = complete_data->work_data;
        PythonGILState::getInstance()->ensureGIL();
        try {
            // Initialize the event trigger
            initialize_event_trigger(complete_data);
            // Transform Variant to py::object
            py::object arguments = transform_variant_2_p(work_data->function_args);
            // Call python function
            py::object module = py::module::import(work_data->module_name.c_str());
            py::object function = module.attr(work_data->function_name.c_str());
            py::object result_obj;
            if (py::isinstance<py::dict>(arguments)) {
                result_obj = function.attr("__call__")(*py::tuple(), **arguments);
            } else if (py::isinstance<py::none>(arguments)) {
                result_obj = function();
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
