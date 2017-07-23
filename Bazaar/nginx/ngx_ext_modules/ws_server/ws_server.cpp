#include "rp_websocket_server.h"

#include "libjson/libjson.h"
#include "libjson/_internal/Source/JSONGlobals.h"
#include "libjson/JSONOptions.h"

#include <fstream>
#include <sstream>
#include <iostream>

rp_websocket_server * s = NULL;

void start_ws_server(const struct server_parameters * _params)
{
    stop_ws_server();

	fprintf(stderr, "start_ws_server()\n");

	struct server_parameters* loaded_params = load_params();
	if(_params != 0)
	{
		loaded_params->set_params_interval_func = _params->set_params_interval_func;
		loaded_params->set_signals_interval_func = _params->set_signals_interval_func;
		loaded_params->get_params_interval_func = _params->get_params_interval_func;
		loaded_params->get_signals_interval_func = _params->get_signals_interval_func;
		loaded_params->get_params_func = _params->get_params_func;
		loaded_params->set_params_func = _params->set_params_func;
		loaded_params->get_signals_func = _params->get_signals_func;
		loaded_params->set_signals_func = _params->set_signals_func;
		loaded_params->gzip_func = _params->gzip_func;
	}
	if(_params != 0 && _params->port != 0)
		loaded_params->port = _params->port;
	if(_params != 0 && _params->signal_interval != 0)
		loaded_params->signal_interval = _params->signal_interval;
	if(_params != 0 && _params->param_interval != 0)
		loaded_params->param_interval = _params->param_interval;

	int port=loaded_params->port;
	s = rp_websocket_server::create(loaded_params);
 	std::string docroot=".";
	s->start(docroot, port);
}

void stop_ws_server()
{
	fprintf(stderr, "stop_ws_server()\n");
	if(s) {
	    s->stop();
	    delete s;
	    s = NULL;
	}
}

struct server_parameters * load_params()
{
	struct stat stat_buf;
	struct server_parameters * params = (struct server_parameters*)malloc(sizeof(struct server_parameters));
	memset(params, 0, sizeof(struct server_parameters));

	if(stat("ws_server.conf", &stat_buf) < 0)
	{
        	/* Config does not exist */
		params->signal_interval = 20;
		params->param_interval = 20;
		params->port = 9002;
        	return params;
	}

	std::ifstream file("ws_server.conf");
	std::ostringstream tmp;
	tmp<<file.rdbuf();
	file.close();
	std::string s = tmp.str();
	JSONNode n = libjson::parse(s);
	params->signal_interval = n.at("s_send_interval").as_int();
	params->param_interval = n.at("p_send_interval").as_int();
	params->port = n.at("port").as_int();
	return params;
}
