#include "main.h"

#include <string>
#include <regex>
#include <fstream>
#include <iterator>

#include <stdio.h>
#include <unistd.h>
#include "version.h"
#include <sys/types.h>
#include <sys/sysinfo.h>

CBooleanParameter inRun("OSC_RUN", CBaseParameter::RW, false, 0);
CStringParameter listDownload("LIST_DOWNLOAD", CBaseParameter::RW, "", 0);
CStringParameter buildURL("BUILD_URL", CBaseParameter::RW, "", 0);
CIntParameter stage("STAGE", CBaseParameter::RW, 0, 0, 0, 100);

const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya Updater\n";
}

extern "C" int rp_app_init(void) {
    fprintf(stderr, "Loading SCPI server version %s-%s.\n", VERSION_STR, REVISION_STR);
    CDataManager::GetInstance()->SetParamInterval(1000);

    rpApp_Init();
    rpApp_OscRun();
    return 0;
}

int rp_app_exit(void) {
    fprintf(stderr, "Unloading SCPI server version %s-%s.\n", VERSION_STR, REVISION_STR);
    rpApp_Release();
    return 0;
}

int rp_set_params(rp_app_params_t *p, int len) {
    return 0;
}

int rp_get_params(rp_app_params_t **p) {
    return 0;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len) {
    return 0;
}


void UpdateSignals(void) {

}

void UpdateParams(void) {
		system("wget -O /tmp/downloadList.html http://downloads.redpitaya.com/downloads/");

		using namespace std;
		ifstream f("/tmp/downloadList.html");
		string html((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());

		string result_url;
		float major_max = 0, minor_max = 0;
		smatch urls, vers;
		regex re(R"(ecosystem-([\.\d]+)-([\.\d]+)-[\w\d]+\.zip)");
		regex_match(html, urls, re);
		fprintf(stderr, "url1 %d\n", urls.size());
		for (const auto& url : urls) {
			fprintf(stderr, "url1 %s\n", url.str().c_str());
			if (regex_match(url.str(), vers, re) && stof(vers[1]) >= major_max && stof(vers[2]) > minor_max) {
				result_url = url.str();
				major_max = stof(vers[1]);
				minor_max = stof(vers[2]);
			}
		}
		fprintf(stderr, "URL = %s\n", result_url.c_str());
}

void OnNewParams(void) {
	inRun.Update();
	inRun.Value() = true;
	//if (inRun.IsNewValue()) {


		//system(("rm -rf /tmp/build && mkdir /tmp/build && cd /tmp/build/ && wget -O /tmp/build/build.zip " + result_url).c_str());
		//system("cd /tmp/build/ && unzip build.zip");
		//system("killall nginx && rw && rm -rf /opt/redpitaya/* && cd /tmp/build/ && cp -fr * /opt/redpitaya/ && reboot &");
	//}
}
