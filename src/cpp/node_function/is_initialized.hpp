#ifndef _NODE_IS_INITIALIZED_H_
#define _NODE_IS_INITIALIZED_H_

#include <Python.h>
#include "node_api.h"

napi_value IsInitialized(napi_env env, napi_callback_info info) {
    napi_value result;
    napi_get_boolean(env, Py_IsInitialized(), &result);
    return result;
}

#endif