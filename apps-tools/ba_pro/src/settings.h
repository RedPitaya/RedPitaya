#pragma once
#include <DataManager.h>
#include <CustomParameters.h>
#include "main.h"

#define NO_INIT -1000000

auto createDirectory(const std::string &_path) -> bool;
auto getHomeDirectory() -> std::string;
auto isDirectory(const std::string &_path) -> bool;
auto configSet(const std::string &_directory, const std::string &_filename) -> bool;
auto configGet(const std::string &_path) -> void;
auto configSetWithList(const std::string &_directory, const std::string &_filename,const std::vector<std::string> &_skipParameters) -> bool;

auto isChanged()-> bool;
auto deleteConfig(const std::string &_path) -> bool;

template<typename _ParameterType, typename _ValueType>
void update_parameter(_ParameterType &_parameter, const _ValueType &_value) {
        JSONNode root_node(JSON_NODE);
        root_node.push_back(JSONNode("value", _value));
        _parameter.SetValueFromJSON(root_node);
};
