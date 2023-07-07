#ifndef _GIL_H_
#define _GIL_H_

#include <Python.h>
#include <node_api.h>

class PythonGILState {
private:
    static PythonGILState* instance;
    PyGILState_STATE gstate;

    PythonGILState() : gstate(PyGILState_UNLOCKED) {}

public:
    static PythonGILState* getInstance() {
        if (instance == nullptr) {
            instance = new PythonGILState();
        }
        return instance;
    }

    void ensureGIL() {
        gstate = PyGILState_Ensure();
    }

    void releaseGIL() {
        PyGILState_Release(gstate);
    }
};

PythonGILState* PythonGILState::instance = nullptr;


#endif