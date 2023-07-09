#ifndef _TRIGGERS_H_
#define _TRIGGERS_H_

#include <uv.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <mutex>
#include "./transform.hpp"
#include "./data.hpp"

namespace py = pybind11;

void initialize_event_trigger(CompleteData* complete_data) {
    // Create a trigger module to enable Python scripts to emit events to the Node.js main thread.
    py::module m("event_trigger");
    m.def("emit", [complete_data](py::str event_key, py::object event_data) {
        // Transform py::object to Variant
        Variant variant_data;
        transform_p_2_variant(event_data, variant_data);
        // Push event to queue
        {
            // Lock event queue
            std::lock_guard<std::mutex> lock(complete_data->event_mutex);
            complete_data->event_queue.push({py::cast<std::string>(event_key), variant_data});
        }
        complete_data->async_event.data = complete_data;
        uv_async_send(&complete_data->async_event);
    }, "Trigger an event on the main thread.");
    // Add the module to Python's sys.modules
    py::dict sys_modules = py::module::import("sys").attr("modules").cast<py::dict>();
    sys_modules["event_trigger"] = m;
}


#endif