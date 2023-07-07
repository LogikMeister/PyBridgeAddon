#ifndef _VARIANT_H_
#define _VARIANT_H_

#include <string>
#include <unordered_map>
#include <vector>

enum VariantType {
    V_NULL,
    V_NUMBER,
    V_STRING,
    V_BOOLEAN,
    V_ARRAY,
    V_OBJECT,
};

struct Variant {
    VariantType type;

    double number_value;
    std::string string_value;
    bool boolean_value;
    std::vector<Variant> array_value;
    std::unordered_map<std::string, Variant> object_value;

    // Constructors for initializing different types of Variant
    Variant(): type(V_NULL) {}
    Variant(double value) : type(V_NUMBER), number_value(value) {}
    Variant(int value) : type(V_NUMBER), number_value(value) {}
    Variant(const std::string& value) : type(V_STRING), string_value(value) {}
    Variant(bool value) : type(V_BOOLEAN), boolean_value(value) {}
    Variant(const std::vector<Variant>& value) : type(V_ARRAY), array_value(value) {}
    Variant(const std::unordered_map<std::string, Variant>& value) : type(V_OBJECT), object_value(value) {}
};

#endif