#pragma once
#ifdef __cplusplus
extern "C"{
#endif

typedef void		(*ws_set_params_interval_func)(int);
typedef void		(*ws_set_signals_interval_func)(int);
typedef int		(*ws_get_params_interval_func)(void);
typedef int		(*ws_get_signals_interval_func)(void);
typedef const char     *(*ws_get_params_func)(void);
typedef const char     *(*ws_get_signals_func)(void);
typedef int		(*ws_set_params_func)(const char *_params);
typedef int		(*ws_set_signals_func)(const char *_signals);
typedef void	(*ws_gzip_func)(const char *_in, void* _out, size_t* _size);

// The following struct can be used to define specific parameters
struct server_parameters {
  	ws_set_params_interval_func set_params_interval_func;
	ws_set_signals_interval_func set_signals_interval_func;
	ws_get_params_interval_func get_params_interval_func;
	ws_get_signals_interval_func get_signals_interval_func;
	ws_get_params_func get_params_func;
	ws_get_signals_func get_signals_func;
	ws_set_params_func set_params_func;
	ws_set_signals_func set_signals_func;
	ws_gzip_func gzip_func;
	int signal_interval; // in ms
	int param_interval; // in ms
	int port;
};

void start_ws_server(const struct server_parameters* _params);

void stop_ws_server();

struct server_parameters* load_params();
#ifdef __cplusplus
}
#endif
