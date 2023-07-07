#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <node_api.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <stdlib.h>
#include <regex>
#include <memory>
// #include <nlohmann/json.hpp>
#include "./variant.hpp"

namespace py = pybind11;
// using json = nlohmann::json;

inline void transform_n_2_variant(napi_env env, napi_value js_value, Variant& result) {
    napi_valuetype value_type;
    napi_typeof(env, js_value, &value_type);

    if (value_type == napi_undefined || value_type == napi_null) {
        result = Variant();
    } else if (value_type == napi_number) {
        double number_value;
        napi_get_value_double(env, js_value, &number_value);
        result = Variant(number_value);
    } else if (value_type == napi_string) {
        size_t str_len;
        napi_get_value_string_utf8(env, js_value, nullptr, 0, &str_len);
        std::string str_value(str_len, '\0');
        napi_get_value_string_utf8(env, js_value, &str_value[0], str_len + 1, &str_len);
        result = Variant(std::move(str_value));
    } else if (value_type == napi_boolean) {
        bool bool_value;
        napi_get_value_bool(env, js_value, &bool_value);
        result = Variant(bool_value);
    } else if (value_type == napi_object) {
        bool is_array;
        napi_is_array(env, js_value, &is_array);
        if (is_array) {
            uint32_t array_len;
            napi_get_array_length(env, js_value, &array_len);
            std::vector<Variant> array_value(array_len);
            for (uint32_t i=0; i<array_len; i++) {
                napi_value js_array_element;
                napi_get_element(env, js_value, i, &js_array_element);
                transform_n_2_variant(env, js_array_element, array_value[i]);
            }
            result = Variant(array_value);
        } else {
            napi_value js_keys;
            napi_get_property_names(env, js_value, &js_keys);
            uint32_t num_keys;
            napi_get_array_length(env, js_keys, &num_keys);
            std::unordered_map<std::string, Variant> object_value;
            for (uint32_t i=0; i<num_keys; i++) {
                napi_value js_key, js_key_value;
                napi_get_element(env, js_keys, i, &js_key);
                napi_get_property(env, js_value, js_key, &js_key_value);

                size_t key_len;
                napi_get_value_string_utf8(env, js_key, nullptr, 0, &key_len);
                std::string key_str(key_len, '\0');
                napi_get_value_string_utf8(env, js_key, &key_str[0], key_len + 1, &key_len);

                transform_n_2_variant(env, js_key_value, object_value[key_str]);
            }
            result = Variant(object_value);
        }
    } else {
        throw std::runtime_error("Unknown or unsupport type: " + std::to_string(value_type) + "which happened in transform_n_2_variant");
    }
}

inline py::object transform_variant_2_p(const Variant& variant) {
    py::object result;

    switch(variant.type) {
        case VariantType::V_NULL: {
            result = py::none();
            break;
        }
        case VariantType::V_NUMBER:
            result = py::float_(variant.number_value);
            break;
        case VariantType::V_STRING:
            result = py::str(variant.string_value);
            break;
        case VariantType::V_BOOLEAN:
            result = py::bool_(variant.boolean_value);
            break;
        case VariantType::V_ARRAY: {
            py::list py_list;
            for (auto& item : variant.array_value) {
                py_list.append(transform_variant_2_p(item));
            }
            result = py_list;
            break;
        }
        case VariantType::V_OBJECT: {
            py::dict py_dict;
            for (auto& [key, value] : variant.object_value) {
                py_dict[py::str(key)] = transform_variant_2_p(value);
            }
            result = py_dict;
            break;
        }
        default:
            throw std::runtime_error("Unknown or unsupport type which happened in transform_variant_2_p");
            break;
    }

    return result;
}

inline void transform_p_2_variant(py::object py_value, Variant& result) {
    if (py::isinstance<py::none>(py_value)) {
        result = Variant();
    } else if (py::isinstance<py::float_>(py_value)) {
        double number_value = py::cast<double>(py_value);
        result = Variant(number_value);
    } else if (py::isinstance<py::int_>(py_value)) {
        int number_value = py::cast<int>(py_value);
        result = Variant(number_value);
    } else if (py::isinstance<py::str>(py_value)) {
        std::string str_value = py::cast<std::string>(py_value);
        result = Variant(std::move(str_value));
    } else if (py::isinstance<py::bool_>(py_value)) {
        bool bool_value = py::cast<bool>(py_value);
        result = Variant(bool_value);
    } else if (py::isinstance<py::list>(py_value)) {
        py::list py_list = py::cast<py::list>(py_value);
        uint32_t array_len = py_list.size();
        std::vector<Variant> array_value(array_len);
        for (uint32_t i=0; i<array_len; i++) {
            transform_p_2_variant(py_list[i], array_value[i]);
        }
        result = Variant(array_value);
    } else if (py::isinstance<py::dict>(py_value)) {
        py::dict py_dict = py::cast<py::dict>(py_value);
        std::unordered_map<std::string, Variant> object_value;
        for (auto item : py_dict) {
            std::string key = py::cast<std::string>(item.first);
            py::object value = py::cast<py::object>(item.second);
            transform_p_2_variant(value, object_value[key]);
        }
        result = Variant(object_value);
    } else {
        throw std::runtime_error("Unknown or unsupport type which happend in transform_p_2_variant: ");
    }
}

inline napi_value transform_variant_2_n(napi_env env, const Variant& variant) {
    napi_value js_value;
    switch(variant.type) {
        case VariantType::V_NULL:
            napi_get_null(env, &js_value);
            break;
        case VariantType::V_NUMBER:
            napi_create_double(env, variant.number_value, &js_value);
            break;
        case VariantType::V_STRING:
            napi_create_string_utf8(env, variant.string_value.c_str(), NAPI_AUTO_LENGTH, &js_value);
            break;
        case VariantType::V_BOOLEAN:
            napi_get_boolean(env, variant.boolean_value, &js_value);
            break;
        case VariantType::V_ARRAY: {
            napi_create_array_with_length(env, variant.array_value.size(), &js_value);
            for (uint32_t i=0; i<variant.array_value.size(); i++) {
                napi_value element = transform_variant_2_n(env, variant.array_value[i]);
                napi_set_element(env, js_value, i, element);
            }
            break;
        }
        case VariantType::V_OBJECT: {
            napi_create_object(env, &js_value);
            for (const auto& [key, value] : variant.object_value) {
                napi_value js_key;
                napi_create_string_utf8(env, key.c_str(), NAPI_AUTO_LENGTH, &js_key);
                napi_value js_key_value = transform_variant_2_n(env, value);
                napi_set_property(env, js_value, js_key, js_key_value);
            }
            break;
        }
        default:
            std::runtime_error("Unknown or unsupported type: " + std::to_string(variant.type) + "which happened in transform_variant_2_n");
    }
    return js_value;
}

#endif