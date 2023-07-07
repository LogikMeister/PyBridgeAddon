#ifndef _NODE_FINALIZE_PYTHON_H_
#define _NODE_FINALIZE_PYTHON_H_
#include <node_api.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "../gil.hpp"
#include "../thread_pool/singleton.hpp"

namespace py = pybind11;

// Finalize the Python interpreter
inline napi_value FinalizePython(napi_env env, napi_callback_info info) {
    // Destroy the Python interpreter
    PythonGILState::getInstance()->ensureGIL();
    py::finalize_interpreter();

    // Destroy the thread pool
    GlobalThreadPool::destroyInstance();

    napi_value resultValue;
    napi_get_boolean(env, true, &resultValue);
    return resultValue;
}
#endif