#include <stdio.h>
#include <cstring>
#include "DataManager.h"
#include "CustomParameters.h"
#include "misc.h"

#ifdef ENABLE_LICENSING
#include "licverify/LicenseVerificator.h"
#endif

#include "gziping.h"

CBooleanParameter IsDemoParam("is_demo", CBaseParameter::RO, false, 1);
CStringParameter InCommandParam("in_command", CBaseParameter::WO, "", 1);
CStringParameter OutCommandParam("out_command", CBaseParameter::RO, "", 1);

int dbg_printf(const char * format, ...)
{
	static FILE* log = fopen("/var/log/nginx/rp_sdk.log", "wt");
	if(log)
	{
		va_list va;
		va_start(va, format);
		vfprintf(log, format, va);
		va_end(va);
		fflush(log);
	}

	return 0;
}

inline bool CDataManager::NeedSend(const CBaseParameter& param) const
{
	CBaseParameter::AccessMode mode = param.GetAccessMode();
	return ((mode != CBaseParameter::AccessMode::WO) && (param.IsValueChanged() || m_send_all_params))
			|| (mode == CBaseParameter::AccessMode::ROSA)
			|| (mode == CBaseParameter::AccessMode::RWSA);
}

CDataManager::CDataManager()
	: m_params()
	, m_signals()
	, m_param_interval(20)
	, m_signal_interval(20)
	, m_send_all_params(true)
{
}

CDataManager* CDataManager::GetInstance()
{
	static CDataManager instance;
	return &instance;
}

void CDataManager::RegisterParam(CBaseParameter * _param)
{
	dbg_printf("RegisterParam: %s\n", _param->GetName());
	m_params.push_back(_param);
	dbg_printf("Registered params: %d\n", m_params.size());
}

void CDataManager::RegisterSignal(CBaseParameter * _signal)
{
	dbg_printf("RegisterSignal: %s\n", _signal->GetName());
	m_signals.push_back(_signal);
	dbg_printf("Registered signals: %d\n", m_signals.size());
}

void CDataManager::UnRegisterParam(const char * _name)
{
	for (std::vector<CBaseParameter *>::iterator it =  m_params.begin() ; it !=  m_params.end(); ++it)
	{
		if(strcmp((*it)->GetName(),_name)==0)
		{
			m_params.erase(it);
			dbg_printf("UnRegisterParam: %s\n", _name);
			return;
		}
	}
}

void CDataManager::UnRegisterSignal(const char * _name)
{
	for (std::vector<CBaseParameter *>::iterator it =  m_signals.begin() ; it !=  m_params.end(); ++it)
	{
		if(strcmp((*it)->GetName(),_name)==0)
		{
			m_signals.erase(it);
			dbg_printf("UnRegisterSignal: %s\n", _name);
			return;
		}
	}
}

void CDataManager::UpdateAllParams()
{
	for(size_t i=0; i < m_params.size(); i++) {
		m_params[i]->Update();
       }
}

void CDataManager::UpdateAllSignals()
{
	for(size_t i=0; i < m_signals.size(); i++) {
		m_signals[i]->Update();
        }
}

std::string CDataManager::GetParamsJson()
{
	UpdateParams();
	JSONNode params(JSON_NODE);
	params.set_name("parameters");
	for(size_t i=0; i < m_params.size(); i++) {
		if(NeedSend(*m_params[i])) {
			JSONNode n(JSON_NODE);
			n = m_params[i]->GetJSONObject();
			params.push_back(n);
		}
	}

	JSONNode data_node(JSON_NODE);
	data_node.set_name("data");
	data_node.push_back(params);
	m_send_all_params = false;
	return data_node.write();
}

std::string CDataManager::GetSignalsJson()
{
	UpdateSignals();
	JSONNode signals(JSON_NODE);
	signals.set_name("signals");
	for(size_t i=0; i < m_signals.size(); i++) {
		if(NeedSend(*m_signals[i])) {
			JSONNode n(JSON_NODE);
			n = m_signals[i]->GetJSONObject();
			signals.push_back(n);
		}
	}

	JSONNode data_node(JSON_NODE);
	data_node.set_name("data");
	data_node.push_back(signals);
	return data_node.write();
}

void CDataManager::OnNewParams(std::string _params)
{
	JSONNode n(JSON_NODE);
	n = libjson::parse(_params);
	JSONNode m(JSON_NODE);

	for(size_t i=0; i < m_params.size(); i++)
		m_params[i]->ClearNewValue();

	for(size_t i=0; i < n.size(); i++) {
		m = n.at(i);
		const char* name = m.name().c_str();

		for(size_t j=0; j < m_params.size(); j++) {
			if(m_params[j]->GetAccessMode() != CBaseParameter::AccessMode::RO)
			{
				const char* param_name = m_params[j]->GetName();
				if(!strcmp(param_name, name))
					m_params[j]->SetValueFromJSON(m);
			}
		}
	}

	if(InCommandParam.IsNewValue())
		m_send_all_params |= InCommandParam.NewValue() == "send_all_params";

	::OnNewParams();
}

void CDataManager::OnNewSignals(std::string _signals)
{
	dbg_printf("OnNewSignals\n");
	JSONNode n(JSON_NODE);
	n = libjson::parse(_signals);
	JSONNode m(JSON_NODE);

	for(size_t i=0; i < m_signals.size(); i++)
		m_signals[i]->ClearNewValue();

	for(size_t i=0; i < n.size(); i++) {
		m = n.at(i);
		const char* name = m.name().c_str();

		for(size_t j=0; j < m_signals.size(); j++) {
			if(m_signals[j]->GetAccessMode() != CBaseParameter::AccessMode::RO)
			{
				const char* param_name = m_signals[j]->GetName();
				if(!strcmp(param_name, name))
					m_signals[j]->SetValueFromJSON(m);
			}
		}
	}

	::OnNewSignals();
}

int CDataManager::GetParamInterval()
{
	return m_param_interval;
}

int CDataManager::GetSignalInterval()
{
	return m_signal_interval;
}

void CDataManager::SetParamInterval(int _interval)
{
	m_param_interval = _interval;
}

void CDataManager::SetSignalInterval(int _interval)
{
	m_signal_interval = _interval;
}

void CDataManager::SendAllParams()
{
	m_send_all_params = true;
}

extern "C" int ws_set_params(const char *_params)
{
	CDataManager * man = CDataManager::GetInstance();
	if(man)
	{
		man->OnNewParams(_params);
		dbg_printf("Set params\n");
		return 1;
	}
	dbg_printf("Params were not set\n");
	return 0;
}

extern "C" const char * ws_get_params(void)
{
	CDataManager * man = CDataManager::GetInstance();
	static std::string res = "";
	if(man)
	{
		res = man->GetParamsJson();
		return res.c_str();
	}
	return res.c_str();
}

extern "C" int ws_set_signals(const char *_signals)
{
	CDataManager * man = CDataManager::GetInstance();
	if(man)
	{
		man->OnNewSignals(_signals);
		dbg_printf("Set signals\n");
		return 1;
	}

	dbg_printf("Signals were not set\n");
	return 0;
}

extern "C" const char* ws_get_signals(void)
{
	CDataManager * man = CDataManager::GetInstance();
	static std::string res = "";
	if(man)
	{
		res = man->GetSignalsJson();
		return res.c_str();
	}
	return res.c_str();
}

extern "C" void ws_set_params_interval(int _interval)
{
	CDataManager * man = CDataManager::GetInstance();
	if(man)
	{
		man->SetParamInterval(_interval);
		dbg_printf("Set params send interval\n");
	}
}

extern "C" int ws_get_params_interval(void)
{
	CDataManager * man = CDataManager::GetInstance();

	if(man)
	{
		int res = man->GetParamInterval();
		return res;
	}
	return 0;
}

extern "C" void ws_set_signals_interval(int _interval)
{
	CDataManager * man = CDataManager::GetInstance();
	if(man)
	{
		man->SetSignalInterval(_interval);
		dbg_printf("Set signals send interval\n");
	}
}

extern "C" int ws_get_signals_interval(void)
{
	CDataManager * man = CDataManager::GetInstance();
	if(man)
	{
		int res = man->GetSignalInterval();
		return res;
	}
	return 0;
}

extern "C" int ws_set_demo_mode(int a)
{
	dbg_printf("Set demo mode\n");
	IsDemoParam.Set(true);
	return 0;
}

extern "C" int verify_app_license(const char* app_id)
{
#ifdef ENABLE_LICENSING
	return verify_app_license_impl(app_id);
#else
	return 0;
#endif
}

extern "C" void ws_gzip(const char* _in, void* _out, size_t* _size)
{
	std::string out;
	Gziping(_in, out);
	memcpy(_out, out.data(), out.size());
	*_size = out.size();
}
