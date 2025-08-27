#pragma once
#include <DataManager.h>
#include <CustomParameters.h>
#include "main.h"

#define NO_INIT -1000000

auto createDirectory(const std::string &_path) -> bool;
auto setHomeSettingsPath(std::string _path) -> void;
auto getHomeDirectory() -> std::string;
auto isDirectory(const std::string &_path) -> bool;
auto configSet() -> bool;
auto configGet() -> void;
auto configSetWithList(const std::vector<std::string> &_parameters) -> bool;

auto loadSettingsFromStore(const std::string &_fileName) -> bool;
auto saveCurrentSettingToStore(const std::string &_newFileName) -> bool;
auto deleteSettingInStore(const std::string &_newFileName) -> bool;
auto getListOfSettingsInStore() -> std::string;

auto isChanged()-> bool;
auto deleteConfig() -> bool;
auto deleteStoredConfig(const std::string &fileName) -> bool;

template<typename _ParameterType, typename _ValueType>
void update_parameter(_ParameterType &_parameter, const _ValueType &_value) {
        JSONNode root_node(JSON_NODE);
        root_node.push_back(JSONNode("value", _value));
        _parameter.SetValueFromJSON(root_node);
};
