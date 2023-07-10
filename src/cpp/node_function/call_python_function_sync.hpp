#ifndef _NODE_CALL_PYTHON_FUNCTION_SYNC_H_
#define _NODE_CALL_PYTHON_FUNCTION_SYNC_H_

#include <node_api.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "../gil.hpp"
#include "../transform.hpp"

namespace py = pybind11;

// Call Python function
inline napi_value CallPythonFunctionSync(napi_env env, napi_callback_info info) {
    napi_status status;
    size_t argc = 3;
    napi_value argv[3];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // check parameters
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }
    
    napi_valuetype arg_types[3];
    for (int i=0; i<2; i++) {
        napi_typeof(env, argv[i], &arg_types[i]);
    }
    if (
        arg_types[0] != napi_string ||
        arg_types[1] != napi_string
    ) {
        napi_throw_type_error(env, nullptr, "Wrong arguments");
    }

    // get parameters
    size_t result;
    char module_name[1024];
    napi_get_value_string_utf8(env, argv[0], module_name, sizeof(module_name), &result);
    char function_name[1024];
    napi_get_value_string_utf8(env, argv[1], function_name, sizeof(function_name), &result);

    // Transform the napi_value to Variant
    Variant function_args;
    if (argc >= 3) {
        napi_typeof(env, argv[2], &arg_types[2]);
        if (
            arg_types[2] != napi_object &&
            arg_types[2] != napi_null &&
            arg_types[2] != napi_undefined &&
            arg_types[2] != napi_number &&
            arg_types[2] != napi_string &&
            arg_types[2] != napi_boolean
        ) {
            napi_throw_type_error(env, nullptr, "Wrong arguments");
            return nullptr;
        }
        transform_n_2_variant(env, argv[2], function_args);
    } else {
        function_args = Variant();
    }

    // Call function
    Variant variant_value;
    PythonGILState::getInstance()->ensureGIL();
    try {
        // Transform the Variant to py::object
        py::object arguments = transform_variant_2_p(function_args);
        py::object module = py::module::import(module_name);
        py::object function = module.attr(function_name);
        py::object result_obj;
        if (py::isinstance<py::dict>(arguments)) {
            result_obj = function.attr("__call__")(*py::tuple(), **arguments);
        } else if (py::isinstance<py::none>(arguments)) {
                result_obj = function();
        } else {
            result_obj = function(arguments);
        }
        // Transform py::object to Variant
        transform_p_2_variant(result_obj, variant_value);
    } catch (py::error_already_set &e) {
        napi_throw_error(env, nullptr, e.what());
    } catch (std::exception &e) {
        napi_throw_error(env, nullptr, e.what());
    } catch (...) {
        napi_throw_error(env, nullptr, "Unknown exception occurred.");
    }
    PythonGILState::getInstance()->releaseGIL();

    // Transform Variant to napi_value
    napi_value result_value;
    try {
        result_value = transform_variant_2_n(env, variant_value);
    } catch (std::exception & e) {
        // a C++ exception occurred
        napi_create_string_utf8(env, (std::string("C++ exception: ") + e.what()).c_str(), NAPI_AUTO_LENGTH, &result_value);
    } catch (...) {
        // an unknown exception occurred
        napi_create_string_utf8(env, "Unknown exception", NAPI_AUTO_LENGTH, &result_value);
    }
    return result_value;
}
#endif
