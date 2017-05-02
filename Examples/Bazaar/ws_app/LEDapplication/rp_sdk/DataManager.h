#pragma once

#include <vector>
#include "BaseParameter.h"

//#define EXPORT __attribute__((__visibility__("default")))


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
};

int dbg_printf(const char * format, ...);

// user callbacks
void UpdateParams(void);
void UpdateSignals(void);
void OnNewParams(void);
void OnNewSignals(void);

// external interface for websockets server
#ifdef __cplusplus
extern "C" {
#endif
void ws_set_params_interval(int _interval);
void ws_set_signals_interval(int _interval);
int ws_get_params_interval(void);
int ws_get_signals_interval(void);
const char * ws_get_params(void);
const char * ws_get_signals(void);
int ws_set_params(const char *_params);
int ws_set_signals(const char *_signals);
int ws_set_demo_mode(int a);
void ws_gzip(const char* _in, void* _out, size_t* size_);
#ifdef __cplusplus
}
#endif
