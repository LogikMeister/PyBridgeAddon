#ifndef _NODE_INITIALIZE_PYTHON_H_
#define _NODE_INITIALIZE_PYTHON_H_
#include <node_api.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "../thread_pool/singleton.hpp"
#include "../utils.hpp"
#include "../gil.hpp"
#include <iostream>

namespace py = pybind11;

// Initialize the Python interpreter
inline napi_value InitializePython(napi_env env, napi_callback_info info) {
    napi_status status;
    size_t argc = 3;
    napi_value argv[3];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // check parameters
    if (argc < 2 || argc > 3) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }
    napi_valuetype type;
    for (uint32_t i = 0; i < 2; i++) {
        status = napi_typeof(env, argv[i], &type);
        if (status != napi_ok || type != napi_string) {
            napi_throw_type_error(env, nullptr, "Wrong arguments");
            return nullptr;
        }
    }
    napi_valuetype arg_2_type;
    if (argc == 3) {
        status = napi_typeof(env, argv[2], &arg_2_type);
        if (status != napi_ok || (arg_2_type != napi_number && arg_2_type != napi_null && arg_2_type!= napi_undefined)) {
            napi_throw_type_error(env, nullptr, "Wrong arguments");
            return nullptr;
        }
    }

    // get parameters
    size_t result;
    char pythonHome[1024];
    napi_get_value_string_utf8(env, argv[0], pythonHome, sizeof(pythonHome), &result);
    char pythonPath[1024];
    napi_get_value_string_utf8(env, argv[1], pythonPath, sizeof(pythonPath), &result);
    uint32_t threads = std::thread::hardware_concurrency();
    if (argc == 3 && arg_2_type == napi_number) {
        napi_get_value_uint32(env, argv[2], &threads);
    }

    setenv("PYTHONHOME", pythonHome, 1);
    setenv("PYTHONPATH", pythonPath, 1);

    std::cout << pythonHome << " " << pythonPath << std::endl;

    // initialize the Python interpreter
    try {
        py::initialize_interpreter();
    } catch (std::exception & e) {
        // a C++ exception occurred
        napi_throw_error(env, nullptr, e.what());
        return nullptr;
    } catch (...) {
        // an unknown exception occurred
        napi_throw_error(env, nullptr, "Initailze Python failed, Please check your environment");
        return nullptr;
    }

    // initialize the thread pool
    GlobalThreadPool::createInstance(threads);

    napi_value resultValue;
    if (!Py_IsInitialized()) {
        napi_get_boolean(env, false, &resultValue);
    } else {
        napi_get_boolean(env, true, &resultValue);
    }

    PythonGILState::getInstance()->releaseGIL();
    
    return resultValue;
}
#endif