#pragma once
#include <DataManager.h>
#include <CustomParameters.h>
#include "main.h"

#define NO_INIT -1000000

auto create_directory(const std::string &_path) -> bool;
auto get_home_directory() -> std::string;
auto is_directory(const std::string &_path) -> bool;
auto config_set(const std::string &_directory, const std::string &_filename) -> bool;
auto config_get(const std::string &_path) -> void;
auto isChanged()-> bool;

template<typename _ParameterType, typename _ValueType>
void update_parameter(_ParameterType &_parameter, const _ValueType &_value) {
        JSONNode root_node(JSON_NODE);
        root_node.push_back(JSONNode("value", _value));
        _parameter.SetValueFromJSON(root_node);
};
