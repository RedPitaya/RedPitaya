#include "scpi/ieee488.h"
#include "scpi/error.h"
#include "error.h"
#include <queue>
#include <map>

std::map<scpi_t*, queue<string>> g_errorList;


void rp_resetErrorList(scpi_t * context){
    g_errorList[context] = queue<string>();
}

void rp_addError(scpi_t * context,string &str){
    g_errorList[context].push(str);
}

string rp_popError(scpi_t * context){
    if (g_errorList[context].empty()) return "";
    auto err = g_errorList[context].front();
    g_errorList[context].pop();
    return err;
}

size_t rp_errorCount(scpi_t * context){
    return g_errorList[context].size();
}

void rp_errorPush(scpi_t * context, string err) {
    rp_addError(context,err);

    SCPI_RegSetBits(context, SCPI_REG_ESR, ESR_EER);

    SCPI_RegSetBits(context, SCPI_REG_STB, STB_QMA);

    if (context) {
        context->cmd_error = TRUE;
    }
}