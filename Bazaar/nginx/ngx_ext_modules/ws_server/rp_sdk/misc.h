#pragma once

#include <vector>
#include <stdio.h>

extern int dbg_printf(const char * format, ...);

template <typename BaseT, typename ValueT>
struct TParam
{
	std::string name;
	ValueT value;
	BaseT min;
	BaseT max;
	int access_mode;
	int fpga_update;
};

//To get value from JSON object
template <typename T>
inline T GetValueFromJSON(JSONNode _node, const char* _at)
{
}

//int specialization of function
template <>
inline int GetValueFromJSON<int>(JSONNode _node, const char* _at)
{
	int res = _node.at(_at).as_int();
	return res;
}

//float specialization of function
template <>
inline float GetValueFromJSON<float>(JSONNode _node, const char* _at)
{
	float res = _node.at(_at).as_float();
	return res;
}

//float specialization of function
template <>
inline double GetValueFromJSON<double>(JSONNode _node, const char* _at)
{
    double tmp = 0.f;
    sscanf(_node.at(_at).as_string().c_str(), "%lf", &tmp);
    return tmp;
}

//bool specialization of function
template <>
inline bool GetValueFromJSON<bool>(JSONNode _node, const char* _at)
{
	bool res = _node.at(_at).as_bool();
	return res;
}

//std::string specialization of function
template <>
inline std::string GetValueFromJSON<std::string>(JSONNode _node, const char* _at)
{
	std::string res = _node.at(_at).as_string();
	return res;
}

//std::vector<int> specialization of function
template <>
inline std::vector<int> GetValueFromJSON<std::vector<int> >(JSONNode _node, const char* _at)
{
	JSONNode n = _node.at(_at);
	std::vector<int> res;
	JSONNode::const_iterator i = n.begin();
    	while (i != n.end()){
        	res.push_back(i->as_int());
        	++i;
    	}

	return res;
}

//std::vector<float> specialization of function
template <>
inline std::vector<float> GetValueFromJSON<std::vector<float> >(JSONNode _node, const char* _at)
{
	JSONNode n = _node.at(_at);
	std::vector<float> res;
	JSONNode::const_iterator i = n.begin();
    	while (i != n.end()){
        	res.push_back(i->as_float());
        	++i;
    	}

	return res;
}

//std::vector<double> specialization of function
template <>
inline std::vector<double> GetValueFromJSON<std::vector<double> >(JSONNode _node, const char* _at)
{
	JSONNode n = _node.at(_at);
	std::vector<double> res;
	JSONNode::const_iterator i = n.begin();
    	while (i != n.end()){
        	res.push_back(i->as_float());
        	++i;
    	}

	return res;
}
