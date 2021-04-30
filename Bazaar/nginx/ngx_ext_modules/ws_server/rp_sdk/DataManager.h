#pragma once

#include <vector>
#include <map>
#include "BaseParameter.h"

struct Data {
	char* data;
	size_t size;
};

class CDataManager
{
private:
	CDataManager();
	CDataManager( const CDataManager&);
	CDataManager& operator=( CDataManager& );

	inline bool NeedSend(const CBaseParameter& param) const;

	std::vector<CBaseParameter*> m_params;
	std::vector<CBaseParameter*> m_signals;
	int m_param_interval; //parameters send time interval in milliseconds
	int m_signal_interval; //signals send time interval in milliseconds
	bool m_send_all_params;

public:
	static CDataManager* GetInstance();
	void UpdateAllParams(void); // involves Update function for registered parameter
	void UpdateAllSignals(void); // involves Update function for registered signal
	void RegisterParam(CBaseParameter * _param);
	void RegisterSignal(CBaseParameter * _signal);
	
	template<class T>
	T* GetByName(std::string _name){
		for (auto x : m_params) { 
			if (std::string(x->GetName()) == _name)
				return dynamic_cast<T*>(x);
		}
		for (auto x : m_signals) { 
			if (std::string(x->GetName()) == _name)
				return dynamic_cast<T*>(x);
		}
		return nullptr;
	}
	const std::vector<CBaseParameter*>* GetParametersList() {return &m_params;}
	const std::vector<CBaseParameter*>* GetSignalList(){return &m_signals;}

	void UnRegisterParam(const char * _name);
	void UnRegisterSignal(const char * _name);

	std::string GetParamsJson(); //get all parameters in JSON-formatted string
	std::string GetSignalsJson(); //get all signals in JSON-formatted string

	void OnNewParams(std::string _params); //is involved when new data received from server, data is JSON-formatted string
	void OnNewSignals(std::string _signals); //is involved when new data received from server, data is JSON-formatted string

	int GetParamInterval();
	int GetSignalInterval();

	void SetParamInterval(int _interval);
	void SetSignalInterval(int _interval);

	void SendAllParams();

	// DEPRECATED
	std::map<std::string, bool> GetFeatures(const std::string& app_id);
};

int dbg_printf(const char * format, ...);

// user callbacks
void UpdateParams(void);
void UpdateSignals(void);
void PostUpdateSignals(void);
void OnNewParams(void);
void OnNewSignals(void);

// external interface for websockets server
extern "C" void ws_set_params_interval(int _interval);
extern "C" void ws_set_signals_interval(int _interval);
extern "C" int ws_get_params_interval(void);
extern "C" int ws_get_signals_interval(void);
extern "C" const char * ws_get_params(void);
extern "C" const char * ws_get_signals(void);
extern "C" int ws_set_params(const char *_params);
extern "C" int ws_set_signals(const char *_signals);
extern "C" void ws_gzip(const char* _in, void* _out, size_t* size_);
